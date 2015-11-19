#pragma once

#include "RigidBody.h"
#include <vector>
#include <glm\glm.hpp>
#include <map>

#include <algorithm> 

#include "QueryPerformance.h"
#include "GJK.h"

struct RbPair
{
	RigidBody* rb1;
	RigidBody* rb2;

	RbPair(RigidBody* p_rb1, RigidBody* p_rb2)
	{
		rb1 = p_rb1;
		rb2 = p_rb2;
	}
};

enum BroadphaseMode { Sphere, BruteAABB, SAP1D};

class RigidbodyManager
{ 
	private:
		vector<AABB::EndPoint*> Axes[3];
		std::vector<Axis> axes;

		GJK gjk;

	public:

		//temp
		static int shaderID1;
		static int shaderID2;

		vector<RigidBody*> rigidBodies;
		vector<AABB::EndPoint*> activeList; //List of potentially colliding pairs / active list
		vector<RbPair> broadphasePairs;

		std::vector<std::vector<int>> pairs;

		RigidbodyManager()
		{
			axes.push_back(Axis::X);
			axes.push_back(Axis::Y);
			axes.push_back(Axis::Z);

			gjk = GJK();
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
			rb->id = rigidBodies.size();
			rigidBodies.push_back(rb);

			Axes[Axis::X].push_back(rb->aabb->min[Axis::X]);
			Axes[Axis::X].push_back(rb->aabb->max[Axis::X]);

			Axes[Axis::Y].push_back(rb->aabb->min[Axis::Y]);
			Axes[Axis::Y].push_back(rb->aabb->max[Axis::Y]);

			Axes[Axis::Z].push_back(rb->aabb->min[Axis::Z]);
			Axes[Axis::Z].push_back(rb->aabb->max[Axis::Z]);

			pairs.resize(rigidBodies.size());
			for(int i=0; i<rigidBodies.size(); i++)
				pairs[i].resize(rigidBodies.size());
		}

		void Broadphase(BroadphaseMode mode)
		{
			QueryPerformance::Start();
			
			if(mode == BroadphaseMode::BruteAABB)
				BruteForceCheckAABBs();
			else if (mode == BroadphaseMode::Sphere)
				SphereCollisions();
			else if (mode == BroadphaseMode::SAP1D)
			{
				SAP1D(Axis::X);
				//SAP(axes);
			}
	
			QueryPerformance::Finish("Broadphase");
		}

		void Narrowphase()
		{
			//for(int i = 0; i < broadphasePairs.size(); i++)
			//{
			//	if(gjk.Intersects(broadphasePairs[i].rb1->model->GetTransformedVertices(), 
			//		broadphasePairs[i].rb2->model->GetTransformedVertices()))
			//	{
			//		//broadphasePairs[i].rb1->aabb->colour =
			//			//broadphasePairs[i].rb2->aabb->colour = glm::vec4(1,0,0,1);

			//		broadphasePairs[i].rb1->model->SetShaderProgramID(shaderID2);//temp
			//		broadphasePairs[i].rb2->model->SetShaderProgramID(shaderID2);//
			//	}
			//}

			//broadphasePairs.clear();

			if(gjk.Intersects(rigidBodies[0]->model, rigidBodies[1]->model))
			{
				rigidBodies[0]->model->SetShaderProgramID(shaderID2);//temp
				rigidBodies[1]->model->SetShaderProgramID(shaderID2);//
			}
		}

		void Update(double deltaTime)
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				//temp change color to white
				rigidBodies[i]->model->SetShaderProgramID(shaderID1);//temp

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
			for(RigidBody* rb : rigidBodies)
				rb->boundingSphere->colour = glm::vec4(0,0,1,1);

			for (int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i + 1; j < rigidBodies.size(); j++)
				{
					if (i == j) continue; 

					BoundingSphere* sphere1 = rigidBodies[i]->boundingSphere;
					BoundingSphere* sphere2 = rigidBodies[j]->boundingSphere;

					if(sphere1->collides(sphere2))
					{
						sphere1->colour = sphere2->colour = glm::vec4(1,0,0,1);		
						broadphasePairs.push_back(RbPair(sphere1->owner, sphere2->owner));
					}
				}
			}
		}

		void BruteForceCheckAABBs()
		{
			for(RigidBody* rb : rigidBodies)
				rb->aabb->colour = glm::vec4(0,1,0,1);

			for (int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i + 1; j < rigidBodies.size(); j++)
				{
					if (i == j) continue; 

					AABB* aabb1 = rigidBodies[i]->aabb;
					AABB* aabb2 = rigidBodies[j]->aabb;

					if(aabb1->collides(aabb2))
					{
						aabb1->colour = aabb2->colour = glm::vec4(1,0,0,1);
						broadphasePairs.push_back(RbPair(aabb1->owner, aabb2->owner));
					}
				}
			}
		}

		void SAP1D(Axis a)
		{
			activeList.clear();	

			for(RigidBody* rb : rigidBodies)
				rb->aabb->colour = glm::vec4(0,1,0,1);

			//std::sort(xAxis.begin(), xAxis.end()); //O(n log (n) )
			insertionSort(Axes[a]);//Insertionsort is O(n) on nearly sorted lists
			//std::sort(xAxis.begin(), xAxis.end(), AABB::EndPoint::my_cmp); 

			for (AABB::EndPoint* ep : Axes[a])
			{
				if(ep->isMin)
				{
					for(AABB::EndPoint* ep2 : activeList)
					{
						if(ep->owner->collides(ep2->owner))
						{
							ep->owner->colour = ep2->owner->colour = glm::vec4(1,0,0,1);
							broadphasePairs.push_back(RbPair(ep->owner->owner, ep2->owner->owner));
						}
					}

					activeList.push_back(ep);
				}
				else
				{
					activeList.erase(std::remove(activeList.begin(), activeList.end(), ep->partner), activeList.end());
				}
			}
		}

		void SAP(vector<Axis> a)
		{
			activeList.clear();	

			for(RigidBody* rb : rigidBodies)
				rb->aabb->colour = glm::vec4(0,1,0,1);

			for(Axis axis : a)
			{
				//std::sort(xAxis.begin(), xAxis.end()); //O(n log (n) )
				insertionSort(Axes[axis]);//Insertionsort is O(n) on nearly sorted lists
				//std::sort(xAxis.begin(), xAxis.end(), AABB::EndPoint::my_cmp); 

				for (AABB::EndPoint* ep : Axes[axis])
				{
					if(ep->isMin)
					{
						for(AABB::EndPoint* ep2 : activeList)
						{
							int id1 = ep->owner->owner->id;
							int id2 = ep2->owner->owner->id;
							pairs[id1][id2]++;
							pairs[id2][id1]++;
						}

						activeList.push_back(ep);
					}
					else
					{
						activeList.erase(std::remove(activeList.begin(), activeList.end(), ep->partner), activeList.end());
					}
				}
			}

			for(int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i + 1; j < rigidBodies.size(); j++)
				{
					if (i == j) continue; 

					if(pairs[i][j] == 3)
					{
						rigidBodies[i]->aabb->colour = rigidBodies[j]->aabb->colour = glm::vec4(1,0,0,1);
						broadphasePairs.push_back(RbPair(rigidBodies[i], rigidBodies[j]));
					}
				}
			}

			for(int i = 0; i < rigidBodies.size(); i++)
				for (int j = 0; j < rigidBodies.size(); j++)
					pairs[i][j] = 0;
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