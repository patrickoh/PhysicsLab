#include "AABB.h"
#include "RigidBody.h"

AABB::EndPoint::EndPoint(float p_value, bool p_isMin, Axis p_axis, AABB* p_owner) 
{
	value = p_value;
	
	isMin = p_isMin;
	axis = p_axis;

	owner = p_owner;
}

AABB::AABB(const std::vector<glm::vec3>& vertices, RigidBody* p_owner)
{
	owner = p_owner;

	colour = glm::vec4(0,0,1,1);

	min[Axis::X] = new EndPoint(std::numeric_limits<float>::max(), true, Axis::X, this);
	min[Axis::Y] = new EndPoint(std::numeric_limits<float>::max(), true, Axis::Y, this);
	min[Axis::Z] = new EndPoint(std::numeric_limits<float>::max(), true, Axis::Z, this);

	max[Axis::X] = new EndPoint(std::numeric_limits<float>::min(), false, Axis::X, this);
	max[Axis::Y] = new EndPoint(std::numeric_limits<float>::min(), false, Axis::Y, this);
	max[Axis::Z] = new EndPoint(std::numeric_limits<float>::min(), false, Axis::Z, this);

	min[Axis::X]->partner = max[Axis::X];
	min[Axis::Y]->partner = max[Axis::Y];
	min[Axis::Z]->partner = max[Axis::Z];

	max[Axis::X]->partner = min[Axis::X];
	max[Axis::Y]->partner = min[Axis::Y];
	max[Axis::Z]->partner = min[Axis::Z];

	Create(vertices);
}

void AABB::Create(const std::vector<glm::vec3> &vertices)
{
	Calculate(vertices, true);

	restBBverts.push_back(glm::vec3(width*0.5, height*0.5, depth*0.5));
	restBBverts.push_back(glm::vec3(width*0.5, height*0.5, -depth*0.5));
	restBBverts.push_back(glm::vec3(width*0.5, -height*0.5, depth*0.5));
	restBBverts.push_back(glm::vec3(width*0.5, -height*0.5, -depth*0.5));
	restBBverts.push_back(glm::vec3(-width*0.5, height*0.5, depth*0.5));
	restBBverts.push_back(glm::vec3(-width*0.5, height*0.5, -depth*0.5));
	restBBverts.push_back(glm::vec3(-width*0.5, -height*0.5, depth*0.5));
	restBBverts.push_back(glm::vec3(-width*0.5, -height*0.5, -depth*0.5));
}

void AABB::Calculate(const std::vector<glm::vec3> &vertices, bool firstrun)
{
	min[Axis::X]->value = std::numeric_limits<float>::max();
	min[Axis::Y]->value = std::numeric_limits<float>::max();
	min[Axis::Z]->value = std::numeric_limits<float>::max();

	max[Axis::X]->value = std::numeric_limits<float>::min();
	max[Axis::Y]->value = std::numeric_limits<float>::min();
	max[Axis::Z]->value = std::numeric_limits<float>::min();
	
	for (glm::vec3 point : vertices)
	{
		if (point.x < min[Axis::X]->value) min[Axis::X]->value = point.x;
		if (point.x > max[Axis::X]->value) max[Axis::X]->value = point.x;
		if (point.y < min[Axis::Y]->value) min[Axis::Y]->value = point.y;
		if (point.y > max[Axis::Y]->value) max[Axis::Y]->value = point.y;
		if (point.z < min[Axis::Z]->value) min[Axis::Z]->value = point.z;
		if (point.z > max[Axis::Z]->value) max[Axis::Z]->value = point.z;
	}

	width = max[Axis::X]->value - min[Axis::X]->value;
	height = max[Axis::Y]->value - min[Axis::Y]->value;
	depth = max[Axis::Z]->value - min[Axis::Z]->value;

	centre = (glm::vec3(min[Axis::X]->value, min[Axis::Y]->value, min[Axis::Z]->value) +
		glm::vec3(max[Axis::X]->value, max[Axis::Y]->value, max[Axis::Z]->value)) * 0.5f;
}

bool AABB::collides(AABB* other)
{
	return ((abs((centre.x + translation.x) - (other->centre.x + other->translation.x)) * 2 < (width + other->width) * scale) && 	
			(abs((centre.y + translation.y) - (other->centre.y + other->translation.y)) * 2 < (height + other->height) * scale) &&
			(abs((centre.z + translation.z) - (other->centre.z + other->translation.z)) * 2 < (depth + other->depth) * scale ));
}

void AABB::Draw()
{
	glutWireCube(1);
}