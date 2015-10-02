#pragma once

#include <vector>
#include "Common.h"
#include "RigidBody.h"

class CollisionManager
{
	private:

		std::vector<RigidBody> rigidBodies;
		std::vector<std::pair<RigidBody, RigidBody>> activeList;

	public:

		CollisionManager()
		{
		}
		~CollisionManager()
		{
		}

		void BroadPhase()
		{
			//Sphere
			//1. Intersection test pairs
		}

		void NarrowPhase()
		{

		}
		
};

	