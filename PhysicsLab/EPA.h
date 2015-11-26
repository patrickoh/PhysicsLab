#pragma once

#include "Common.h"
#include <vector>
#include "Model.h"
#include <limits>

//The contact info comprises two contact points, penetration depth, and penetration direction (normal)
struct ContactInfo
{
	glm::vec3 p1;
	glm::vec3 p2;
	float depth;
	glm::vec3 normal;

	ContactInfo(glm::vec3 p_p1, glm::vec3 p_p2, float p_depth, glm::vec3 norm) 
		: p1(p_p1), p2(p_p2)
	{
	}
};

//A face comprises three vertices and a surface normal
struct Face
{
	glm::vec3 v1, v2, v3;
	glm::vec3 n;

	Face(glm::vec3 p_v1, glm::vec3 p_v2, glm::vec3 p_v3) : v1(p_v1), v2(p_v2), v3(p_v3)
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
	private:
		std::vector<Face> triangleList;
		std::vector<glm::vec3> pointList;

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

			triangleList.push_back(Face(simplex[0], simplex[1], simplex[2]));
			triangleList.push_back(Face(simplex[0], simplex[1], simplex[3]));
			triangleList.push_back(Face(simplex[0], simplex[2], simplex[3]));
			triangleList.push_back(Face(simplex[1], simplex[2], simplex[3]));

			ContactInfo cInfo(glm::vec3(0), glm::vec3(0), 0.0f, glm::vec3(0,0,1)); //tmp

			//Face closest = findClosestFace();

			//do
			//{
				//
			//}
			//while(closest);

			return cInfo;
		}


	private:
		void Reset()
		{
			triangleList.clear();
			pointList.clear();
		}

		//Face findClosestFace()
		//{
		//	//Face closest;
		//	
		//	float d;
		//	float maxD = std::numeric_limits<float>::max();

		//	for(int i = 0; i < triangleList.size(); i++)
		//	{
		//		//d = 
		//	}
		//}
};