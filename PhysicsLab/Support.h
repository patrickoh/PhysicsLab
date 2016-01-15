#pragma once

#include "Common.h"
#include <vector>
#include "Model.h"

#include "Point.h"

struct SupportPoint
{
	glm::vec3 AB; //minkowski difference
	glm::vec3 A;
	glm::vec3 B;

	SupportPoint()
	{
	}
};

//Support - furthest along in that direction 
glm::vec3 Support(glm::vec3 direction, std::vector<glm::vec3> shape)
{
	//direction = glm::normalize(direction); // just in case //Doesn't have to be normalised

	float maxDot = glm::dot(shape[0], direction);
	glm::vec3 furthest = shape[0];
	//The dot product tells you what amount of one vector goes in the direction of another, in other words,
	//what is the furtest point in said direction.

	float dot;

	for(int i = 0; i < shape.size(); i++)
	{
		dot = glm::dot(shape[i], direction);

		if(dot > maxDot)
		{
			maxDot = dot;
			furthest = shape[i];
		}
	}

	return furthest;
}

glm::vec3 Support(glm::vec3 direction, std::vector<glm::vec3> shape1, std::vector<glm::vec3> shape2)
{
	glm::vec3 p1 = Support(direction, shape1);
	glm::vec3 p2 = Support(-direction, shape2);

	return p1 - p2; //= max Dt ABij (furtest point along direction in minkowski difference)
}

//glm::vec3 Support(glm::vec3 direction, Model* shape1, Model* shape2)
//{
//	glm::vec3 p1 = shape1->GetFurthestPointInDirection(direction);
//	glm::vec3 p2 = shape2->GetFurthestPointInDirection(-direction);
//
//	return p1 - p2; //= max Dt ABij (furtest point along direction in minkowski difference)
//}

SupportPoint Support(glm::vec3 direction, Model* shape1, Model* shape2)
{
	SupportPoint sp;

	sp.A = shape1->GetFurthestPointInDirection(direction);
	sp.B = shape2->GetFurthestPointInDirection(-direction);

	sp.AB = sp.A - sp.B;

	return sp; //= max Dt ABij (furtest point along direction in minkowski difference)
}

bool operator==(const SupportPoint& left, const SupportPoint& right)
{
	return left.AB == right.AB 
		&& left.A == right.A 
		&& left.B == right.B;
}

