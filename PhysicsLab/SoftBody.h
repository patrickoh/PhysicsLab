#pragma once

#include "Model.h"
#include "AABB.h"

struct SoftBodyParticle
{

};

class SoftBody
{
	private:

		glm::vec3 initialPosition;

		glm::vec3 forceNet;

	public:

		glm::vec3 velocity;
		glm::vec3 momentum;

		float mass;
		glm::vec3 com;
		Model* model;
		AABB* aabb;

		void StepPhysics(double deltaTime);
		void Update();

		SoftBody(Model* model);
		~SoftBody();
};