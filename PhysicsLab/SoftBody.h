#pragma once

#include "Model.h"
#include "AABB.h"

struct PointMass
{
	float mass;
	glm::vec3 position;
	glm::vec3 velocity;
	
	glm::vec3 forceNet;
};

class SoftBody
{
	private:

		glm::vec3 initialPosition;

		std::vector<PointMass> points;

		//glm::vec3 forceNet;

	public:

		glm::vec3 velocity;
		glm::vec3 momentum;

		float mass;
		glm::vec3 com;
		Model* model;
		AABB* aabb;

		void StepPhysics(double deltaTime);
		void Update();

		SoftBody(Model* model)
		{

		}

		~SoftBody()
		{

		}
};