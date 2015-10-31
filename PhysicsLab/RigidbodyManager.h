#pragma once

#include "RigidBody.h"
#include <vector>
#include <glm\glm.hpp>

#include <algorithm> 

#include "QueryPerformance.h"

struct CollidingPair
{
	AABB* aabb1;
	AABB* aabb2;

	CollidingPair(AABB* p_aabb1, AABB* p_aabb2)
	{
		aabb1 = p_aabb1;
		aabb2 = p_aabb2;
	}
};

class RigidbodyManager
{ 
	private:
		vector<AABB::EndPoint*> xAxis;

	public:

		vector<RigidBody*> rigidBodies;
		vector<AABB::EndPoint*> activeList; //List of potentially colliding pairs / active list
		vector<CollidingPair> collidingPairs;

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

			xAxis.push_back(rb->aabb->min[Axis::X]);
			xAxis.push_back(rb->aabb->max[Axis::X]);
		}

		void Broadphase()
		{
			SphereCollisions();

			QueryPerformance::Start();
			//BruteForceCheckAABBs();
			SAP1D();
			QueryPerformance::Finish();

			std::cout << QueryPerformance::Result() << std::endl;
		}

		void Update(double deltaTime)
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				//if bouncy enclosure

				glm::vec3 normal[] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
				glm::vec3 plane[] = { glm::vec3(0,-5,0), glm::vec3(0,5,0), glm::vec3(-5,0,0), glm::vec3(5,0,0), glm::vec3(0,0,5), glm::vec3(0,0,-5) };

				for(int j = 0; j < 6; j++)
				{
					if(glm::dot(rigidBodies[i]->model->worldProperties.translation - plane[j], normal[j]) < 0.01f
							&& glm::dot(normal[j], rigidBodies[i]->velocity) < 0.01f)
					{
						rigidBodies[i]->model->worldProperties.translation += -glm::dot(rigidBodies[i]->model->worldProperties.translation - plane[j], normal[j]) * normal[j]; //post processing method
						rigidBodies[i]->velocity += (1 + 1.0f/*coefficient of restitution*/) * -(rigidBodies[i]->velocity * normal[j]) * normal[j];

						//velocity.y = -velocity.y;
					}
				}

				rigidBodies[i]->StepPhysics(deltaTime);
				rigidBodies[i]->Update();
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

		void BruteForceCheckAABBs()
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i + 1; j < rigidBodies.size(); j++)
				{
					if (i == j) continue; 

					AABB* aabb1 = rigidBodies[i]->aabb;
					AABB* aabb2 = rigidBodies[j]->aabb;

					if(aabb1->collides(aabb2))
						aabb1->colour = aabb2->colour = glm::vec4(1,0,0,1);
					else
						aabb1->colour = aabb2->colour = glm::vec4(0,1,0,1);
				}
			}
		}

		void SAP1D()
		{
			activeList.clear();	

			//std::sort(xAxis.begin(), xAxis.end()); //O(n log (n) )
			insertionSort(xAxis);//Insertionsort is O(n) on nearly sorted lists
			//std::sort(xAxis.begin(), xAxis.end(), AABB::EndPoint::my_cmp); 

			for(RigidBody* rb : rigidBodies)
				rb->aabb->colour = glm::vec4(0,1,0,1);

			for (AABB::EndPoint* ep : xAxis)
			{
				if(ep->isMin)
				{
					for(AABB::EndPoint* ep2 : activeList)
						if(ep->owner->collides(ep2->owner))
							ep->owner->colour = ep2->owner->colour = glm::vec4(1,0,0,1);

					activeList.push_back(ep);
				}
				else
				{
					activeList.erase(std::remove(activeList.begin(), activeList.end(), ep->partner), activeList.end());
				}
			}
		}

		void SAP3D()
		{

		}

		void insertionSort (vector<AABB::EndPoint*> &data) 
		{
			int i, j;
			AABB::EndPoint* tmp;

			for (i = 1; i < data.size(); i++)
			{
				j = i;

				tmp = data[i];

				while (j > 0 && tmp->global < data[j-1]->global)
				{
					data[j] = data[j-1];
					j--;
				}

				data[j] = tmp;
			}
		}
};