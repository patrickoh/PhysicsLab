#pragma once

#include "Common.h"
#include <vector>
#include "Model.h"
#include <limits>
#include <algorithm>

#define EPA_MAX_ITERATIONS 100
#define EPA_TOLERANCE .00001

//The contact info comprises two contact points, penetration depth, and penetration direction (normal)
struct ContactInfo
{
	glm::vec3 c1;
	glm::vec3 c2;
	float depth;
	glm::vec3 normal;

	ContactInfo(glm::vec3 p1, glm::vec3 p2, float p_depth, glm::vec3 norm) 
		: c1(p1), c2(p2), depth(p_depth), normal(norm) 
	{
	}
};

//A face comprises three vertices and a surface normal
struct Triangle
{
	glm::vec3 v1, v2, v3;
	glm::vec3 n;

	Triangle(glm::vec3 p_v1, glm::vec3 p_v2, glm::vec3 p_v3) : v1(p_v1), v2(p_v2), v3(p_v3)
	{
		//Winding needs to be CCW
		//EPA retains winding of the simplex, so this is just to make sure the initial simplex is CCW
		if(!isCCW(v1, v2))
		{
			glm::vec3 tmp = v1;
			v1 = v3;
			v3 = tmp;
		}

		n = glm::cross(v2-v1, v3-v1); //CCW winding assumed
		n = glm::normalize(n);
	}

	Triangle(){}

	std::vector<glm::vec3> getPoints()
	{
		std::vector<glm::vec3> points;
		points.push_back(v1);
		points.push_back(v2);
		points.push_back(v3);
		return points;
	}

	float getDistance(glm::vec3 from)
	{
		return glm::distance(from, closestPointOnTriangle(getPoints(), from));
	}
};

//TODO - perhaps give them a unique id?
bool operator==(const Triangle& left, const Triangle& right)
{
	return left.v1 == right.v1 
		&& left.v2 == right.v2 
		&& left.v3 == right.v3
		&& left.n == right.n;
}

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
	private:
		std::vector<Triangle> polytope;
		//std::vector<glm::vec3> pointList;

	public:

		EPA()
		{
			
		}

		~EPA()
		{

		}

		ContactInfo getContactInfo(Model* shape1, Model* shape2, std::vector<glm::vec3> simplex) 
		{
			Reset();

			polytope.push_back(Triangle(simplex[0], simplex[1], simplex[2]));
			polytope.push_back(Triangle(simplex[0], simplex[1], simplex[3]));
			polytope.push_back(Triangle(simplex[0], simplex[2], simplex[3]));
			polytope.push_back(Triangle(simplex[1], simplex[2], simplex[3]));

			ContactInfo cInfo(glm::vec3(0), glm::vec3(0), 0.0f, glm::vec3(0,0,1)); //tmp

			Triangle closest, previousClosest;

			for(int i = 0; i < EPA_MAX_ITERATIONS; i++)
			{
				// (1) Pick closest face of polytope to origin
				closest = findClosestFace();

				// (2) Remove closest face
				polytope.erase(std::remove(polytope.begin(), polytope.end(), closest), polytope.end());

				// (3) Search for support point in direction of normal
				glm::vec3 minkowskiPoint = Support(closest.n, shape1, shape2);

				float d = closest.getDistance(glm::vec3(0));

				if(d - previousClosest.getDistance(glm::vec3(0)) < EPA_TOLERANCE)
				{
					//calculatue barycentric coords of the closest tri with respect to proj of the origin onto the face
					glm::vec3 bary = barycentric(glm::vec3(0), closest.v1, closest.v2, closest.v3);

					cInfo.depth = d; //the penetration depth is the distance between the closest minkowski facet and the origin
					cInfo.normal = closest.n; // the normal of the contact is the normal of the closest minkowski facet

					//glm::vec3 p1 = Support(direction, shape1); 
					//glm::vec3 p2 = Support(-direction, shape2);

					//SupportA(closest.n, shape1, shape2);
					//SupportB(closest.n, shape1, shape2);

					//u * support
					//v *
					//w *

					//cInfo.c1 = 
					//cInfo.c2 =

					//return cInfo;
				}

				// (4) Add new faces to connect the new point
				polytope.push_back(Triangle(minkowskiPoint, closest.v1, closest.v2));
				polytope.push_back(Triangle(minkowskiPoint, closest.v1, closest.v3));
				polytope.push_back(Triangle(minkowskiPoint, closest.v2, closest.v3));

				previousClosest = closest;
			}

			return cInfo;
		}


	private:
		void Reset()
		{
			polytope.clear();
			//pointList.clear();
		}

		Triangle findClosestFace()
		{
			Triangle closest;
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