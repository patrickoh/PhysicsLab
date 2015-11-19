#pragma once

#include "Common.h"
#include <vector>

class RigidBody;

class BoundingSphere
{

public:
	glm::vec3 centre;
	float radius;

	RigidBody* owner;
	
	glm::vec4 colour;

	glm::vec3 translation; 
	float scale;

	BoundingSphere(const std::vector<glm::vec3>& v, RigidBody* p_owner);

	//TODO - Fix this naive approach that doesn't even work!
	void calculate(const std::vector<glm::vec3>& v)
	{
		glm::vec3 p1, p2;
		float diameter = 0.0f;

		for(int i = 0; i < v.size(); i++)
		{
			for(int j = 0; j < v.size(); j++)
			{
				if(glm::distance(v[i], v[j]) > diameter)
				{
					diameter = glm::distance(v[i], v[j]);
					p1 = v[i];
					p2 = v[j];
				}
			}
		}

		centre = (p1 + p2) * 0.5f;
		radius = diameter * 0.5f;
	}

	bool collides(BoundingSphere* other)
	{
		return (glm::distance(centre + translation, other->centre + other->translation) < ((radius * scale) + (other->radius * other->scale)) );
	}

	void draw()
	{
		glutWireSphere(radius, 25, 25);
	}

	void Update(glm::vec3 p_translation, float uniformScale)
	{
		translation = p_translation;
		scale = uniformScale;
	}

private:

};