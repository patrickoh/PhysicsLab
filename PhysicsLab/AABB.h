#include "Common.h"
#include <vector>

enum Axis { X = 0, Y, Z };
//enum End { min, max };

class AABB;

struct EndPoint
{
	float value;
	bool isMin;

	//AABB* owner;

	EndPoint() 
	{
	}

	EndPoint(float p_value, bool p_isMin) 
	{
		value = p_value;
		isMin = p_isMin;
	}

	bool operator<(const EndPoint& other)
	{
		return value < other.value;
	}

	bool operator>(const EndPoint& other)
	{
		return value > other.value;
	}
};

class AABB
{
	glm::vec3 position;
	 //local space?

public:

	glm::vec3 centre;
	glm::vec4 colour;

	float width;
	float depth;
	float height;

	std::vector<glm::vec3> restBBverts;

	EndPoint min [3];
	EndPoint max [3]; 

	AABB(const std::vector<glm::vec3>& vertices/*, RigidBody* p_owner*/)
	{
		//owner = p_owner;

		colour = glm::vec4(0,0,1,1);

		min[0] = EndPoint(std::numeric_limits<float>::max(), true);
		min[1] = EndPoint(std::numeric_limits<float>::max(), true);
		min[2] = EndPoint(std::numeric_limits<float>::max(), true);

		max[0] = EndPoint(std::numeric_limits<float>::min(), true);
		max[1] = EndPoint(std::numeric_limits<float>::min(), true);
		max[2] = EndPoint(std::numeric_limits<float>::min(), true);

		Create(vertices);
	}

	void Create(const std::vector<glm::vec3> &vertices)
	{
		Calculate(vertices);

		restBBverts.push_back(glm::vec3(width*0.5, height*0.5, depth*0.5));
		restBBverts.push_back(glm::vec3(width*0.5, height*0.5, -depth*0.5));
		restBBverts.push_back(glm::vec3(width*0.5, -height*0.5, depth*0.5));
		restBBverts.push_back(glm::vec3(width*0.5, -height*0.5, -depth*0.5));
		restBBverts.push_back(glm::vec3(-width*0.5, height*0.5, depth*0.5));
		restBBverts.push_back(glm::vec3(-width*0.5, height*0.5, -depth*0.5));
		restBBverts.push_back(glm::vec3(-width*0.5, -height*0.5, depth*0.5));
		restBBverts.push_back(glm::vec3(-width*0.5, -height*0.5, -depth*0.5));
	}

	void Calculate(const std::vector<glm::vec3> &vertices)
	{
		for (glm::vec3 point : vertices)
		{
			if (point.x < min[Axis::X].value) min[Axis::X].value = point.x;
			if (point.x > max[Axis::X].value) max[Axis::X].value = point.x;
			if (point.y < min[Axis::Y].value) min[Axis::Y].value = point.y;
			if (point.y > max[Axis::Y].value) max[Axis::Y].value = point.y;
			if (point.z < min[Axis::Z].value) min[Axis::Z].value = point.z;
			if (point.z > max[Axis::Z].value) max[Axis::Z].value = point.z;
		}

		width = max[Axis::X].value - min[Axis::X].value;
		height = max[Axis::Y].value - min[Axis::Y].value;
		depth = max[Axis::Z].value - min[Axis::Z].value;

		centre = (glm::vec3(min[Axis::X].value, min[Axis::Y].value, min[Axis::Z].value) +
			glm::vec3(max[Axis::X].value, max[Axis::Y].value, max[Axis::Z].value)) * 0.5f;
	}

	bool Overlaps(AABB other)
	{
		return ((abs(position.x - other.position.x) * 2 < (width + other.width)) &&
			 (abs(position.y - other.position.y) * 2 < (height + other.height)) &&
			 (abs(position.z - other.position.z) * 2 < (depth + other.depth)));
	}

	void Draw()
	{
		glutWireCube(1);
	}

	//Just store endpoints
	/*EndPoint GetEndPoint(Axis axis, End end)
	{
		EndPoint ep;
		
		ep.owner = this;
		ep.isMin = (end == End::min);

		switch (axis) {

			case Axis::X:
				if (end == End::min)
					ep.value = min.x;
				else
					ep.value = max.x;
				break;

			case Axis::Y:
				if (end == End::min)
					ep.value = min.y;
				else
					ep.value = max.y;
				break;

			case Axis::Z:
				if (end == End::min)
					ep.value = min.z;
				else
					ep.value = max.z;
				break;
		}
				
		return ep;
	}*/
};