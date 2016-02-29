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

		struct Environment
		{
			float gravity;
			glm::vec3 wind;
			float windScalar;
			float fluidDensity;
		} env;

		float radius, surfaceArea, dragCoefficient; //Drag stuff
		
	public:

		bool immovable;

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

		int id;
		
		static bool angular;
		static bool linear;

		static bool gravity;
		static bool wind;
		static bool drag;

		static bool bDriftCorrection;

		void StepPhysics(double deltaTime);
		void Update();

		RigidBody(Model* model);
		~RigidBody();

		glm::vec3 FNet(glm::vec3 pos, glm::vec3 vel);
		void ApplyImpulse(glm::vec3 p, glm::vec3 force);
		void Reset();

		static glm::vec3 CalculateCentreOfMass(Model* model);

		static void transformBatch(std::vector<glm::vec3>* verts, glm::quat orientation)
		{
			for(int i = 0; i < verts->size(); i++)
				verts->at(i) = verts->at(i) * glm::toMat3(orientation);
		}

		glm::mat3 getIntertialTensor()
		{
			return glm::transpose(glm::toMat3(model->worldProperties.orientation))
				* glm::inverse(inertialTensor) * glm::toMat3(model->worldProperties.orientation);
		}

		glm::vec3 fNet(glm::vec3 vel, float mass)
		{
			glm::vec3 fNet = glm::vec3(0);

			if(gravity)
				fNet += Gravity(mass);
			if(drag)
				if(wind)
					fNet += PressureDrag(vel, true);
				else
					fNet += PressureDrag(vel, false);

			return fNet;
		}

		glm::vec3 Gravity(float mass)
		{
			return mass * env.gravity * glm::vec3(0,-1,0);
		}

		glm::vec3 PressureDrag(glm::vec3 vel, bool wind)
		{
			if(wind) 
			return 0.5f * env.fluidDensity * surfaceArea * dragCoefficient 
				* (vel - (env.wind * env.windScalar)).length() * -(vel - (env.wind * env.windScalar));
			else
			return 0.5f * env.fluidDensity * surfaceArea * dragCoefficient 
				* vel.length() * -vel;
		}
};