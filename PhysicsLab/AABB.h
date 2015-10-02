#include "Common.h"
#include <vector>

//enum Axis { X, Y, Z };
//enum End { min, max };

class AABB;

struct EndPoint
{
	float value;
	bool isMin;

	AABB* owner;

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
	//RigidBody* owner;

	glm::vec3 position;

	//glm::vec3 centre; //local space?

	float width;
	float depth;
	float height;

public:

	EndPoint* min [3];
	EndPoint* max [3]; 

	AABB(const std::vector<glm::vec3>& vertices/*, RigidBody* p_owner*/)
	{
		//owner = p_owner;
		Create(vertices);
	}

	void Create(const std::vector<glm::vec3> &vertices)
	{
		//local space with rotation

		/*for (glm::vec3 point : vertices)
		{
			if (point.x < min[0]->value) min[0]->value = point.x;
			if (point.x > max[0]->value) max[0]->value = point.x;
			if (point.y < min[1]->value) min[1]->value = point.y;
			if (point.y > max[1]->value) max[1]->value = point.y;
			if (point.z < min.z) min.z = point.z;
			if (point.z > max.z) max.z = point.z;
		}

		width = max.x - min.x;
		height = max.y - min.y;*/

	}

	bool Overlaps(AABB other)
	{
		return ((abs(position.x - other.position.x) * 2 < (width + other.width)) &&
			 (abs(position.y - other.position.y) * 2 < (height + other.height)) &&
			 (abs(position.z - other.position.z) * 2 < (depth + other.depth)));
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