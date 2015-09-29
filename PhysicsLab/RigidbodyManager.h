#pragma once

#include "RigidBody.h"
#include <vector>
#include <glm\glm.hpp>

struct CollidingPair
{
	RigidBody* rb1;
	RigidBody* rb2;
};

class RigidbodyManager
{ 
	private:


	public:

		vector<RigidBody*> rigidBodies;

		vector<CollidingPair> collidingPairs; //List of potentially colliding pairs / active list

		//SAP
		vector<CollidingPair> xOverlap;	

		RigidbodyManager(){}
		~RigidbodyManager(){}

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
					{ 
						//sphere2->Change_Color(d_colliding_color);
						//sphere1->Change_Color(d_colliding_color);
					}
				}
			}
		}

		void SingleAxisSAP()
		{
			collidingPairs.clear();	
			xOverlap.clear();

			vector<EndPoint> xAxis;

			for (RigidBody* rb : rigidBodies)
			{
				xAxis.push_back(rb->aabb->GetEndPoint(Axis::X, End::min));
				xAxis.push_back(rb->aabb->GetEndPoint(Axis::X, End::max));
			}

			//O(n log (n) )
			std::sort(xAxis.begin(), xAxis.end());
		}
};