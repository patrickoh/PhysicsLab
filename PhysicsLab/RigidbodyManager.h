#pragma once

#include "RigidBody.h"
#include <vector>
#include <glm\glm.hpp>

#include <algorithm> 

struct CollidingPair
{
	RigidBody* rb1;
	RigidBody* rb2;
};

class RigidbodyManager
{ 
	private:
		vector<EndPoint*> xAxis;

	public:

		vector<RigidBody*> rigidBodies;
		vector<CollidingPair> activeList; //List of potentially colliding pairs / active list

		RigidbodyManager()
		{
			
		}

		~RigidbodyManager(){}

		/*void Add(RigidBody* rigidbody)
		{
			xAxis.push_back(rigidbody->aabb->min.x);
		}*/

		RigidBody* operator [](int i) const    { return rigidBodies[i]; }
		RigidBody* & operator [](int i) { return rigidBodies[i]; }

		//TODO - remove

		void Add(RigidBody* rb)
		{
			rigidBodies.push_back(rb);
		}

		void Broadphase()
		{
			SphereCollisions();
		}

		void Update(double deltaTime)
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				rigidBodies[i]->Update(deltaTime);

				rigidBodies[i]->boundingSphere->scale = rigidBodies[i]->model->worldProperties.scale.x;
				rigidBodies[i]->boundingSphere->position = rigidBodies[i]->model->worldProperties.translation;
			}
		}

		//Brute force check spheres
		void SphereCollisions()
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i + 1; j < rigidBodies.size(); j++)
				{
					if (i == j) continue; 

					BoundingSphere* sphere1 = rigidBodies[i]->boundingSphere;
					BoundingSphere* sphere2 = rigidBodies[j]->boundingSphere;

					if(sphere1->collides(sphere2))
						sphere1->colour = sphere2->colour = glm::vec4(1,0,0,1);
					else
						sphere1->colour = sphere2->colour = glm::vec4(0,1,0,1);
				}
			}
		}

		void Brute()
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i + 1; j < rigidBodies.size(); j++)
				{
					if (i == j) continue; 
				}
			}
		}

		void SingleAxisSAP()
		{
			activeList.clear();	

			std::sort(xAxis.begin(), xAxis.end()); //O(n log (n) )
		}
};