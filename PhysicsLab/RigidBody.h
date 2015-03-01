#pragma once

#include "Model.h"
#include "glm\gtx\orthonormalize.hpp"
#include <AntTweakBar.h>

class RigidBody;

struct BoundingSphere
{
	glm::vec3 centre; //in world space
	float radius;

	bool collisionCheck(BoundingSphere other)
	{
		float d = glm::distance(centre, other.centre);

		if(d < (radius + other.radius))
			return true;
		else
			return false;
	}
};

enum Axis { X, Y, Z };
enum End { min, max };

struct AABB
{
	struct EndPoint
	{
		float value;
		bool s;
		AABB* owner;

		bool operator<(const EndPoint& other)
		{
			return value < other.value;
		}

		bool operator>(const EndPoint& other)
		{
			return value > other.value;
		}

		RigidBody* rb()
		{
			return owner->owner;
		}
	};

	glm::vec3 position;
	
	glm::vec3 min;
	glm::vec3 max;

	//float width;
	//float depth;
	//float height;

	RigidBody* owner;

	void Calculate(std::vector<glm::vec3> &vertices)
	{
		//local space with rotation
	}

	EndPoint GetEndPoint(Axis axis, End end)
	{
		EndPoint ep;
		ep.owner = this;

		if(axis == Axis::X)
		{	
			if(end == End::min)
			{
				ep.value = min.x;
				ep.s = true;	
			}
			else
			{
				ep.value = max.x;
				ep.s = false;
			}
		}
		else if(axis == Axis::Y)
		{
			if(end == End::min)
			{
				ep.value = min.y;
				ep.s = true;
			}
			else
			{
				ep.value = max.y;
				ep.s = false;
			}
		}
		else
		{
			if(end == End::min)
			{
				ep.value = min.z;
				ep.s = true;
			}
			else
			{
				ep.value = min.z;
				ep.s = false;
			}
		}

		return ep;
	}
};

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
		BoundingSphere boundingSphere;
		AABB aabb;

		static glm::vec3 force;
		static Model* impulseVisualiser;
		static bool angular;
		static bool linear;

		void Update(double deltaTime);

		RigidBody(Model* model);
		~RigidBody();

		void ApplyImpulse(glm::vec3 p, glm::vec3 force);
		void Reset();

		// Calculation of the center of mass based on paul bourke's website - Thanks Gio
		// http://paulbourke.net/geometry/polygonmesh/
		static glm::vec3 CalculateCentreOfMass(Model* model)
		{
			size_t N = model->vertices.size();

			glm::vec3* area  = new glm::vec3[N];
			glm::vec3* R	 = new glm::vec3[N];
			glm::vec3 numerator;
			glm::vec3 denominator;

			for (int i = 0; i < N; i = i + 3) // for each facets --> numerator += R[i] * area[i], denominator += area[i] 
			{
				glm::vec3 v1 = model->vertices[i];
				glm::vec3 v2 = model->vertices[i+1];
				glm::vec3 v3 = model->vertices[i+2];
				R[i] = (v1 + v2 + v3) / 3.0f;
				area[i] = glm::abs(glm::cross(v2 - v1,v3 - v1));
				numerator += R[i]*area[i];
				denominator +=area[i];
			}

			glm::vec3 com = numerator/denominator;

			if (com != com)
				com = glm::vec3(0.0f,0.0f,0.0f);

			delete[] area;
			delete[] R;

			return com;
		}

		//void TW_CALL TwApplyImpulse(void *clientData)
		//{
		//	ApplyImpulse(impulseVisualiser->worldProperties.translation, RigidBody::force);
		//}

		//void BuildTweakBar(TwBar* bar)
		//{
		//	TwAddVarRW(bar, "Impulse Position", TW_TYPE_DIR3F, &impulseVisualiser->worldProperties.translation, "");
		//	//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, 
		//				 //" label='Simulation Speed' step=0.1 opened=true help='Change the simulation speed.' ");
		//	TwAddVarRW(bar, "Impulse Force", TW_TYPE_DIR3F, &RigidBody::force, "");
		//	TwAddButton(bar, "Apply Torque", TwApplyImpulse, NULL, "");
		//}
};