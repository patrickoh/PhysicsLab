#pragma once

#include "RigidBody.h"
#include <vector>
#include <glm\glm.hpp>

#include <algorithm> 

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

			//Brute();
			SingleAxisSAP();
		}

		void Update(double deltaTime)
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				rigidBodies[i]->Update(deltaTime);

				rigidBodies[i]->boundingSphere->translation = rigidBodies[i]->model->worldProperties.translation;
				rigidBodies[i]->aabb->translation = rigidBodies[i]->model->worldProperties.translation;

				glm::vec3 scale = rigidBodies[i]->model->worldProperties.scale;
				rigidBodies[i]->boundingSphere->scale = max(max(scale.x, scale.y), scale.z);
				rigidBodies[i]->aabb->scale = max(max(scale.x, scale.y), scale.z);

				rigidBodies[i]->aabb->min[0]->Update();
				rigidBodies[i]->aabb->max[0]->Update();
				rigidBodies[i]->aabb->min[1]->Update();
				rigidBodies[i]->aabb->max[1]->Update();
				rigidBodies[i]->aabb->min[2]->Update();
				rigidBodies[i]->aabb->max[2]->Update();
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

					AABB* aabb1 = rigidBodies[i]->aabb;
					AABB* aabb2 = rigidBodies[j]->aabb;

					if(aabb1->collides(aabb2))
						aabb1->colour = aabb2->colour = glm::vec4(1,0,0,1);
					else
						aabb1->colour = aabb2->colour = glm::vec4(0,1,0,1);
				}
			}
		}

		//TODO - fix scale issue
		void SingleAxisSAP()
		{
			activeList.clear();	

			//std::sort(xAxis.begin(), xAxis.end()); //O(n log (n) )
			//insertionSort(xAxis);
			std::sort(xAxis.begin(), xAxis.end(), AABB::EndPoint::my_cmp); 

			for (AABB::EndPoint* ep : xAxis)
			{
				if(ep->isMin)
				{
					for(AABB::EndPoint* ep2 : activeList)
						collidingPairs.push_back(CollidingPair(ep->owner, ep2->owner));

					activeList.push_back(ep);
				}
				else
				{
					activeList.erase(std::remove(activeList.begin(), activeList.end(), ep), activeList.end());
					activeList.erase(activeList.begin());
				}
			}

			//temp
			for(RigidBody* rb : rigidBodies)
			{
				rb->aabb->colour = glm::vec4(0,1,0,1);
			}

			for(CollidingPair cp : collidingPairs)
			{
				cp.aabb1->colour = glm::vec4(1,0,0,1);
				cp.aabb2->colour = glm::vec4(1,0,0,1);
			}

			collidingPairs.clear();
		}

		void insertionSort (vector<AABB::EndPoint*> &data) 
		{
			int i, j;
			AABB::EndPoint* tmp;

			for (i = 1; i < data.size(); i++)
			{
				j = i;

				tmp = data[i];

				while (j > 0 && tmp->GetGlobalValue() < data[j-1]->GetGlobalValue())
				{
					data[j] = data[j-1];
					j--;
				}

				data[j] = tmp;
			}
		}
};