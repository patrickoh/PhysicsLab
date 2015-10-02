#pragma once

#include "Common.h"

struct BoundingSphere
{
	glm::vec3 centre; //in local space
	float radius;

	//TODO - visualisation
	//bool vis;
	//glm::vec4 colour;

	BoundingSphere(const std::vector<glm::vec3>& vertices)
		: radius(0.0f)
	{
		create(vertices);
	}

	bool collides(BoundingSphere* other)
	{
		return (glm::distance(centre, other->centre) < (radius + other->radius));
	}

	void create(const std::vector<glm::vec3>& vertices)
	{
		glm::vec3 min;
		glm::vec3 max;

		for (glm::vec3 point : vertices)
		{
			if (point.x < min.x) min.x = point.x;
			if (point.x > max.x) max.x = point.x;
			if (point.y < min.y) min.y = point.y;
			if (point.y > max.y) max.y = point.y;
			if (point.z < min.z) min.z = point.z;
			if (point.z > max.z) max.z = point.z;
		}

		float diameter = glm::distance(min, max);
		radius = diameter * 0.5f;

		centre = (min + max) * 0.5f; //midpoint
	}
};