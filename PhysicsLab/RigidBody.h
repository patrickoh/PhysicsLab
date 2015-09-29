#pragma once

#include "Model.h"
#include "glm\gtx\orthonormalize.hpp"

#include "BoundingSphere.h"
#include "AABB.h"

class RigidBody
{
	private:

		glm::vec3 initialPosition;

		glm::vec3 torqueNet;
		glm::vec3 forceNet;

	public:

		glm::vec3 velocity;
		glm::vec3 momentum;

		float mass;

		glm::vec3 angularVelocity;
		glm::vec3 angularMomentum;

		glm::mat3 inertialTensor;

		glm::vec3 com;

		Model* model;
		BoundingSphere* boundingSphere;
		AABB* aabb;

		static glm::vec3 force;
		static Model* impulseVisualiser;
		static bool angular;
		static bool linear;

		void Update(double deltaTime);

		RigidBody(Model* model);
		~RigidBody();

		void ApplyImpulse(glm::vec3 p, glm::vec3 force);
		void Reset();

		static glm::vec3 CalculateCentreOfMass(Model* model);
};