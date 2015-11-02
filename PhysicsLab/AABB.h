#pragma once

#include "Common.h"
#include <vector>
#include <GL/freeglut.h>

enum Axis { X = 0, Y, Z };
//enum End { min, max };

class RigidBody;

class AABB
{

private:

public:

	struct EndPoint
	{
		float value; //ooor update the value
		float global;

		bool isMin;
		Axis axis;

		AABB* owner;
		EndPoint* partner;

		EndPoint() {}
		EndPoint(float p_value, bool p_isMin, Axis axis, AABB* p_owner);

		void Update()
		{
			if(axis == Axis::X)
				global = value * owner->scale + owner->translation.x; 
			else if(axis == Axis::Y)
				global = value * owner->scale + owner->translation.y;
			else
				global = value * owner->scale + owner->translation.z;
		}

		static bool my_cmp(const EndPoint* a, const EndPoint* b)
		{
			return a->global < b->global;
		}

		/*bool operator>(const EndPoint& other) const
		{
			if(axis == Axis::X)
				return (value + owner->translation.x) > (other.value + other.owner->translation.x); 
			if(axis == Axis::Y)
				return (value + owner->translation.y) > (other.value + other.owner->translation.y);

			return (value + owner->translation.z) > (other.value + other.owner->translation.z); 
		}*/
	};

	glm::vec3 translation;
	float scale; //assumes a uniform scale

	glm::vec3 centre;
	glm::vec4 colour;

	float width;
	float depth;
	float height;

	std::vector<glm::vec3> restBBverts;

	EndPoint* min [3];
	EndPoint* max [3]; 

	RigidBody* owner;

	AABB(const std::vector<glm::vec3>& vertices, RigidBody* p_owner);

	//glm::vec3 getTranslation() { return translation; }
	//void setTranslation(glm::vec3 set) { translation = set; }

	void Create(const std::vector<glm::vec3> &vertices);
	void Calculate(const std::vector<glm::vec3> &vertices, bool firstrun  = false);
	bool collides(AABB* other);
	void Draw();

	void Update(glm::vec3 p_translation, float uniformScale)
	{
		translation = p_translation;
		scale = uniformScale;

		min[0]->Update();
		max[0]->Update();
		min[1]->Update();
		max[1]->Update();
		min[2]->Update();
		max[2]->Update();
	}
	
};

