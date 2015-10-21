#pragma once

#include "Common.h"
#include <vector>
#include <GL/freeglut.h>

enum Axis { X = 0, Y, Z };
//enum End { min, max };

class AABB
{

private:

public:

	glm::vec3 translation;

	struct EndPoint
	{
		float value; //ooor update the value
		float global;

		bool isMin;
		Axis axis;

		AABB* owner;

		EndPoint() {}
		EndPoint(float p_value, bool p_isMin, Axis axis, AABB* p_owner);

		float GetGlobalValue()
		{
			if(axis == Axis::X)
				return value + owner->translation.x; 
			else if(axis == Axis::Y)
				return value + owner->translation.y;
			else
				return value + owner->translation.z;
		}

		void Update()
		{
			if(axis == Axis::X)
				global = value + owner->translation.x; 
			else if(axis == Axis::Y)
				global = value + owner->translation.y;
			else
				global = value + owner->translation.z;
		}

		static bool my_cmp(const EndPoint* a, const EndPoint* b)
		{
			if(a->axis == Axis::X)
				return (a->value + a->owner->translation.x) < (b->value + b->owner->translation.x); 
			else if(a->axis == Axis::Y)
				return (a->value + a->owner->translation.y) < (b->value + b->owner->translation.y);
			else
				return (a->value + a->owner->translation.z) < (b->value + b->owner->translation.z);
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

	float scale; //assumes a uniform scale

	glm::vec3 centre;
	glm::vec4 colour;

	float width;
	float depth;
	float height;

	std::vector<glm::vec3> restBBverts;

	EndPoint* min [3];
	EndPoint* max [3]; 

	AABB(const std::vector<glm::vec3>& vertices/*, RigidBody* p_owner*/);

	//glm::vec3 getTranslation() { return translation; }
	//void setTranslation(glm::vec3 set) { translation = set; }

	void Create(const std::vector<glm::vec3> &vertices);
	void Calculate(const std::vector<glm::vec3> &vertices, bool firstrun  = false);
	bool collides(AABB* other);
	void Draw();

	
};

