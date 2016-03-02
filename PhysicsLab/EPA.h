#pragma once

#include "Common.h"
#include <vector>
#include "Model.h"
#include <limits>
#include <algorithm>
#include "Support.h"

#define EPA_MAX_ITERATIONS 100
#define EPA_TOLERANCE .00001

//A face comprises three vertices and a surface normal
struct TriangleEPA
{
	SupportPoint a, b, c;
	glm::vec3 n;

	TriangleEPA(SupportPoint p_v1, SupportPoint p_v2, SupportPoint p_v3) : a(p_v1), b(p_v2), c(p_v3)
	{
		//Winding needs to be CCW
		//EPA retains winding of the simplex, so this is just to make sure the initial simplex is CCW
		/*if(!isCCW(v1.AB, v2.AB))
		{
			SupportPoint tmp = v1;
			v1 = v3;
			v3 = tmp;
		}*/

		n = glm::normalize(glm::cross(b.AB-a.AB, c.AB-a.AB)); //CCW winding assumed

		if (n != n)
			n = glm::vec3(0.0f);
	}

	TriangleEPA(){}

	std::vector<glm::vec3> getPoints()
	{
		std::vector<glm::vec3> points;
		points.push_back(a.AB);
		points.push_back(b.AB);
		points.push_back(c.AB);
		return points;
	}

	float getDistance(glm::vec3 from)
	{
		return glm::distance(from, closestPointOnTriangle(getPoints(), from));
	}
};

struct Edge
{
	SupportPoint p1,p2;

	Edge(SupportPoint p1,SupportPoint p2)
		: p1(p1),p2(p2)
	{

	}

	bool operator== (const Edge& edge) const
	{
		return (edge.p1 == p1 && edge.p2 == p2) || (edge.p1 == p2 && edge.p2 == p1);
	}
};

//TODO - perhaps give them a unique id?
bool operator==(const TriangleEPA& left, const TriangleEPA& right)
{
	return left.a.AB == right.a.AB 
		&& left.b.AB == right.b.AB 
		&& left.c.AB == right.c.AB
		&& left.n == right.n;
}

//The contact info comprises two contact points, penetration depth, and penetration direction (normal)
struct ContactInfo
{
	glm::vec3 c1, c2;
	float depth; //Not using it (yet?)
	glm::vec3 normal;
	TriangleEPA f;

	ContactInfo(glm::vec3 p1, glm::vec3 p2, float p_depth, glm::vec3 norm) 
		: c1(p1), c2(p2), depth(p_depth), normal(norm) 
	{
	}

	ContactInfo()
	{
	}
};

//The EPA (Expanding Polytope Algorithm) computes the penetration depth, penetration vector, and contact points between two objects.
//The goal of EPA is to find the closest face (to the origin) on the minkowski difference boundary.
//It achieves this by iteratively expanding the polytope gained from the gjk algorithm until the expansion meets the boundary of the Minkowski Difference.
//Projecting the origin on to that face will give the penetration vector.
//...
//The distance between the origin and the closest point on the minkowski difference is the penetration depth 
//The vector from the closest point to the origin is the penetration vector
//EPA's simplex can contain any number of points, but it starts with a tetrahedron (for 3d epa)
//...
//

class EPA
{

public:

	static ContactInfo GetContactInfo(Model* shape1, Model* shape2, std::vector<SupportPoint> simplex) 
	{
		SupportPoint A = simplex[3];
		SupportPoint B = simplex[2];
		SupportPoint C = simplex[1];
		SupportPoint D = simplex[0]; 

		std::vector<TriangleEPA> polytope;
		polytope.push_back(TriangleEPA(A,B,C));
		polytope.push_back(TriangleEPA(A,C,D));
		polytope.push_back(TriangleEPA(A,D,B));
		polytope.push_back(TriangleEPA(B,D,C));

		ContactInfo cInfo(glm::vec3(0), glm::vec3(0), 0.0f, glm::vec3(0,0,1)); //tmp
		TriangleEPA closest;

		int steps = 0;
		while(steps < EPA_MAX_ITERATIONS)
		{
			// Get the closest face of the polytope to the origin
			closest = FindClosestFace(polytope);

			glm::vec3 direction = glm::normalize(closest.n);
			if (glm::dot(direction, closest.a.AB) <= 0.0f) //if direction vector projection along vector AB is negative, it's facing the wrong way
				direction = -direction;

			//Search for furthest support point in direction of the normal of closest face
			SupportPoint newSupportPoint = Support(/*closest.n*/ direction, shape1, shape2);

			//Check the distance from the origin to the edge 
			// against the distance the new support point is along the normal of the closest face
			float d = glm::dot(closest.n, newSupportPoint.AB);	
				
			//if(d - closest.getDistance(glm::vec3(0)) < EPA_TOLERANCE) //Tolerance positive close to zero
			if(glm::dot(newSupportPoint.AB - closest.a.AB, closest.n) - glm::dot(closest.a.AB, closest.n) < EPA_TOLERANCE) 
			{
				//If the distance is less than the tolerance then we can assume that the simplex cannot expand any further.
				//Slightly above zero to avoid as an approximation is okay for a CSO with a high resolution

				//calculatue barycentric coords of the closest tri with respect to proj of the origin onto the face
				glm::vec3 bary;
				
				bary = barycentric(glm::vec3(0), closest.a.AB, closest.b.AB, closest.c.AB); 
				//bary = barycentric(closest.n * closest.getDistance(glm::vec3(0)), closest.a.AB, closest.b.AB, closest.c.AB); 
				
				cInfo.depth = d; //the penetration depth is the distance between the closest minkowski facet and the origin
				cInfo.normal = closest.n; // the normal of the contact is the normal of the closest minkowski facet

				cInfo.c1 = bary.x/*u*/ * closest.a.A
					+ bary.y/*v*/ * closest.b.A
					+ bary.z/*w*/ * closest.c.A;

				cInfo.c2 = bary.x * closest.a.B
					+ bary.y * closest.b.B
					+ bary.z * closest.c.B;

				cInfo.f = closest;

				return cInfo;
			}
					
			ExpandPolytope(newSupportPoint, polytope);
		}

		return cInfo;
	}

private:

	//Remove faces from the polytope that can be "seen" by the new support point
	//Add new faces that cover up the hole. The new faces all share the new support point as a common vertex.
	static void ExpandPolytope(SupportPoint newSupportPoint, std::vector<TriangleEPA> &polytope)
	{
		std::vector<Edge> edges;

		for (int i = polytope.size() - 1; i >= 0 ; i--)
		{
			TriangleEPA triangle = polytope[i];
			
			bool canBeSeen = isSameDirection(newSupportPoint.AB - triangle.a.AB, triangle.n); //All triangles that can be seen are removed to perserve convexity
				
			if (canBeSeen) 
			{ 
				//Add the edges of the "hole"
				AddEdge(Edge(triangle.a, triangle.b), edges);
				AddEdge(Edge(triangle.b, triangle.c), edges);
				AddEdge(Edge(triangle.c, triangle.a), edges);
					
				//Remove the face
				polytope.erase(polytope.begin() + i);
			}
		}

		//Patch up the hole
		for (auto edge : edges)
		{
			polytope.push_back(TriangleEPA(newSupportPoint, edge.p1, edge.p2));
		}

		edges.clear(); 
	}

	//When an edge is added to the edge list, if the opposite edge already exists, then that edge is shared by two of the faces that are to be removed, and
	//therefore is not used for closing up the gap.
	static void AddEdge(Edge edge, std::vector<Edge> &edges)
	{
		for(auto it = edges.begin(); it != edges.end(); it++) 
		{
			if(*it == edge) 
			{
				edges.erase(it);
				return;
			}
		}

		edges.push_back(edge);
	}

	static TriangleEPA FindClosestFace(std::vector<TriangleEPA> &polytope)
	{
		TriangleEPA closest;
		float min = std::numeric_limits<float>::max();

		for(int i = 0; i < polytope.size(); i++)
		{
			float d = glm::distance(glm::vec3(0), closestPointOnTriangle(polytope[i].getPoints(), glm::vec3(0)));

			if(d < min)
			{
				min = d;
				closest = polytope[i];
			}
		}

		return closest;
	}
};



	