#pragma once

#include "RigidBody.h"
#include <vector>
#include <glm\glm.hpp>
#include <map>

#include <algorithm> 

#include "QueryPerformance.h"
#include "GJK.h"
#include "EPA.h"

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
enum DebugStage { Gjk, Epa };

class RigidbodyManager
{ 
	private:
		vector<AABB::EndPoint*> Axes[3];

	public:

		ContactInfo cInfo;
		bool bVisContacts;

		GJK* gjk;
		EPA epa;

		vector<RigidBody*> rigidBodies;

		vector<AABB::EndPoint*> activeList; //List of potentially colliding pairs / active list
		std::vector<std::vector<int>> pairs; //2d container for tracking no. of axis collisions between aabbs (max 3)

		vector<RbPair> broadphasePairs; //Handed on to narrowphase stage

		bool bounceyEnclosure;
		
		bool paused;

		bool debugGJK, debugEPA, stepDebug;
		DebugStage debugStage;

		bool CR;

		std::vector<glm::vec3> currentMinkowski;

		glm::vec3 J;

		RigidbodyManager()
		{
			gjk = new GJK();
			epa = EPA();

			bounceyEnclosure = true;
			CR = false;

			paused = false;

			debugGJK = false; //Are we debug drawing GJK?
			debugEPA = false; //Are we debug drawing EPA?
			stepDebug = false; //Has the step button been pressed to do an iteration?
			debugStage = DebugStage::Gjk; //Which narrowphase stage are we at?

			bVisContacts = false;
		}

		~RigidbodyManager(){}

		/*void Add(RigidBody* rigidbody)
		{
			xAxis.push_back(rigidbody->aabb->min.x);
		}*/

		RigidBody* operator [](int i) const    { return rigidBodies[i]; }
		RigidBody* & operator [](int i) { return rigidBodies[i]; }

		//TODO - remove rigidbody

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

				/*std::vector<Axis> axes;
				axes.push_back(Axis::X);
				axes.push_back(Axis::Y);
				axes.push_back(Axis::Z);
				SAP(axes);*/
			}
	
			QueryPerformance::Finish("Broadphase");
		}

		void Narrowphase(double deltaTime)
		{
			for(int i = 0; i < broadphasePairs.size(); i++)
			{
				std::vector<SupportPoint> simplexForEPA;

				RigidBody* rb1 = broadphasePairs[i].rb1;
				RigidBody* rb2 = broadphasePairs[i].rb2;

				//For visualisation
				//currentMinkowski = gjk->MinkowskiDifference(rb1->model->GetTransformedVertices(), rb2->model->GetTransformedVertices());
				//currentMinkowski = getConvexHull(currentMinkowski);

				if(debugGJK && debugStage == DebugStage::Gjk)
				{
					paused = true;
					if(!stepDebug)
						continue;
					else
						stepDebug = false;	
				}
				
				bool gjkFinished = false;
				bool intersecting = gjk->Intersects(rb1->model, rb2->model, simplexForEPA, /* debug stuff --> */ gjkFinished, (debugGJK && debugStage == DebugStage::Gjk));

				if(gjkFinished)
					paused = false; //If it was in gjk Debug, simulation may resume
					
				if((!debugGJK || gjkFinished) && intersecting) //GJK is finished, and found an intersection
				{
					rb1->model->isColliding = rb2->model->isColliding = true; //vis
					debugStage = DebugStage::Epa;

					if(debugEPA)
					{
						paused = true;
						if(!stepDebug)
							continue;
						else
							stepDebug = false;
					}

					bool epaFinished = false;
					cInfo = epa.getContactInfo(rb1->model, rb2->model, simplexForEPA, /* debug stuff --> */ epaFinished, debugEPA);

					if(epaFinished)
					{
						debugStage = DebugStage::Gjk; //Put it back to the default
						gjk->simplex.clear(); //Can now clear the original gjk simplex which we were using for vis
						paused = false; // resume simulation (in case was paused for debugging)
							
						bVisContacts = true; //visualise contacts
					}

					//Collision Response
					if(CR)
					{
						float j = CalculateImpulse(rb2, rb1, cInfo.c1, cInfo.c2, cInfo.normal); 
							
						////𝑱=𝑗 𝒏
						J = j * cInfo.normal;

						////Δ𝑷=𝑱
						rb1->momentum -= J;
						rb2->momentum += J;

						////Δ𝑳=(𝒓×𝑱)
						rb1->angularMomentum -= glm::cross(cInfo.c1 - rb1->model->worldProperties.translation, J); //don't bother with com for the moment (As cube com is 0,0,0)
				
						rb2->angularMomentum += glm::cross(cInfo.c2 - rb2->model->worldProperties.translation, J);
					}	
				}
			}

			broadphasePairs.clear();
		}

		float CalculateImpulse(RigidBody* rb1, RigidBody* rb2, glm::vec3 c1, glm::vec3 c2, glm::vec3 normal, float e = 1.0f)
		{
			//vec3 rA = (contact pt of A) - xA (centre of mass position of A)
			glm::vec3 rA = c1 - rb1->model->worldProperties.translation; //Don't bother with com for the moment
			glm::vec3 rB = c2 - rb2->model->worldProperties.translation;

			float t1 = 1.0f / rb1->mass;
			float t2 = 1.0f / rb2->mass;
			float t3 = glm::dot(normal, glm::cross(rb1->getIntertialTensor() * glm::cross(rA, normal), rA));
			float t4 = glm::dot(normal, glm::cross(rb2->getIntertialTensor() * glm::cross(rB, normal), rB));

			glm::vec3 pA = rb1->velocity + glm::cross(rb1->angularVelocity, rA);
			glm::vec3 pB = rb2->velocity + glm::cross(rb2->angularVelocity, rB);

			float vrel = glm::dot(normal, pA - pB);
	
			float j = 0.0f;
			if(vrel < 0.0f) 
				j = std::max(0.0f, (-(1 + e) * vrel) / (t1 + t2 + t3 + t4) );

			return j;
		}

		void Update(double deltaTime)
		{
			for (int i = 0; i < rigidBodies.size(); i++)
			{
				rigidBodies[i]->model->isColliding = false;

				if(bounceyEnclosure)
				{
					glm::vec3 normal[] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
					glm::vec3 plane[] = { glm::vec3(0,-50,0), glm::vec3(0,50,0), glm::vec3(-50,0,0), glm::vec3(50,0,0), glm::vec3(0,0,50), glm::vec3(0,0,-50) };

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
				}

				if(!paused)
					rigidBodies[i]->StepPhysics(deltaTime); //physics update
					
				rigidBodies[i]->Update(); //bookkeeping
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