#include "BoundingSphere.h"

BoundingSphere::BoundingSphere(const std::vector<glm::vec3>& v, RigidBody* p_owner)
{
	calculate(v);
	colour = glm::vec4(0,1,0,1);

	scale = 1;
	translation = glm::vec3(0);

	owner = p_owner;
}