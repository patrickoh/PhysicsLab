#include "Common.h"
#include <vector>

enum Axis { X, Y, Z };
enum End { min, max };

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
	glm::vec3 position;

	glm::vec3 min;
	glm::vec3 max;

	glm::vec3 centre; //local space?

	//float width;
	//float depth;
	//float height;

public:

	AABB(const std::vector<glm::vec3>& vertices)
	{
		Create(vertices);
	}

	void Create(const std::vector<glm::vec3> &vertices)
	{
		//local space with rotation

		for (glm::vec3 point : vertices)
		{
			if (point.x < min.x) min.x = point.x;
			if (point.x > max.x) max.x = point.x;
			if (point.y < min.y) min.y = point.y;
			if (point.y > max.y) max.y = point.y;
			if (point.z < min.z) min.z = point.z;
			if (point.z > max.z) max.z = point.z;
		}
	}

	EndPoint GetEndPoint(Axis axis, End end)
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
	}
};