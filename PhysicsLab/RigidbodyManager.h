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

		vector<CollidingPair> collidingPairs;

		vector<CollidingPair> xOverlap;
		vector<CollidingPair> yOverlap;
		vector<CollidingPair> zOverlap;

		RigidbodyManager(){}
		~RigidbodyManager(){}

		void CheckAABBCollisions()
		{
			collidingPairs.clear();
			xOverlap.clear();
			yOverlap.clear();
			zOverlap.clear();

			vector<AABB::EndPoint> xAxis, yAxis, zAxis; 

			for (RigidBody* rb : rigidBodies)
			{
				xAxis.push_back(rb->aabb.GetEndPoint(Axis::X, End::min));
				xAxis.push_back(rb->aabb.GetEndPoint(Axis::X, End::max));
				
				yAxis.push_back(rb->aabb.GetEndPoint(Axis::Y, End::min));
				yAxis.push_back(rb->aabb.GetEndPoint(Axis::Y, End::max));

				zAxis.push_back(rb->aabb.GetEndPoint(Axis::Z, End::min));
				zAxis.push_back(rb->aabb.GetEndPoint(Axis::Z, End::max));
			}

			std::sort(xAxis.begin(),xAxis.end());
			std::sort(yAxis.begin(),yAxis.end());
			std::sort(zAxis.begin(),zAxis.end());

			CheckAxis(xAxis, xOverlap);
			CheckAxis(yAxis, yOverlap);
			CheckAxis(zAxis, zOverlap);

			for (CollidingPair pair : xOverlap)
			{
				if(std::find(yOverlap.begin(), yOverlap.end(), pair) != yOverlap.end())
					if(std::find(zOverlap.begin(), zOverlap.end(), pair) != zOverlap.end())
						collidingPairs.push_back(pair);
			}

			/*for (int i = 0; i < rigidBodies.size(); i++)
			{
				for (int j = i+1; j < rigidBodies.size(); j++)
				{
					if (rigidBodies[j]->aabb.GetEndPoint(Axis::X, End::min) > 
						rigidBodies[i]->aabb.GetEndPoint(Axis::X, End::max))
						break;

					if (rigidBodies[j]->aabb.GetEndPoint(Axis::Y, End::min) > 
						rigidBodies[i]->aabb.GetEndPoint(Axis::Y, End::max))
						break;

					if (rigidBodies[j]->aabb.GetEndPoint(Axis::Z, End::min) > 
						rigidBodies[i]->aabb.GetEndPoint(Axis::Z, End::max))
						break;

					CollidingPair pair;

					pair.rb1 = rigidBodies[i];
					pair.rb2 = rigidBodies[j];

					collidingXYZ.push_back(pair);
				}
			}*/
		}

		void CheckAxis(vector<AABB::EndPoint> axis, vector<CollidingPair> &overlap)
		{
			vector<AABB::EndPoint> activeList;

			for(AABB::EndPoint ep : axis)
			{
				if(ep.s)
				{
					for(AABB::EndPoint other : axis)
					{
						CollidingPair pair;
						pair.rb1 = ep.rb();
						pair.rb2 = other.rb();

						xOverlap.push_back(pair);
					}

					axis.push_back(ep);
				}
				else
				{
					activeList.erase(std::remove(activeList.begin(), activeList.end(), ep), activeList.end());
					//axis.erase(std::remove(activeList.begin(), activeList.end(), ep), activeList.end());
				}
			}
		}

		//TODO - exploit coherency
		template< typename T>
		static void InsertionSort(std::vector<T> &m_array)
		{
			int i, j;
			float tmp;

			for (i = 1; i < m_array.size(); i++) 
			{
				j = i;

				//while index is 1 or greater, and the value before the index is larger
				//perform swap
				while (j > 0 && m_array[j - 1] > m_array[j]) 
				{
						tmp = m_array[j];
						m_array[j] = m_array[j - 1];
						m_array[j - 1] = tmp;
						j--; //decrement index, to continue shifting the value down
				}
			}
		}
};