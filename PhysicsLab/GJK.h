#pragma once

#include "Common.h"
#include <vector>
#include "Support.h"

//Gilbert Johnson Keerthi algorithm for determining whether or 
//not two convex solids intersect
class GJK
{
	private:

		SupportPoint a, b, c, d;
		int nrPointsSimplex;

		bool firstrun;

	public:

		GJK()
		{
			firstrun = true;
		}

		~GJK()
		{

		}

		void Reset()
		{
			nrPointsSimplex = 0;
			
			a = SupportPoint(); 
			b = SupportPoint();
			c = SupportPoint();
			d = SupportPoint();
			
			firstrun = true;
		}

		#pragma region firsttry
		//bool Intersects(std::vector<glm::vec3> shape1, std::vector<glm::vec3> shape2)
		//{
			//std::vector<glm::vec3> simplex; //a simplex is simply a list of points that form an n-dimensional construct

			////arbitrary starting direction to get the ball rolling
			//glm::vec3 dir = glm::vec3(0,1,0);
			//simplex.push_back(Support(dir, shape1, shape2)); //Go as far as possible in search direction

			//dir = -dir; //Go in opposite direction (far side of origin)

			//for (int i = 0; i < 50; i++) //allow 50 iterations to converge
			//{
			//	glm::vec3 A = Support(dir, shape1, shape2);

			//	if(glm::dot(A, dir) < 0) //Didn't reach the origin i.e. no intersection (can't enclose the origin)
			//		return false;

			//	if(Update(simplex, dir)) //Returns true if the new point results in a simplex which encloses the origin
			//		return true;
			//}

			//return false;
		//}
		#pragma endregion

		//http://in2gpu.com/2014/05/18/gjk-algorithm-3d/
		std::pair<bool, bool> Intersects(Model* model1, Model* model2, std::vector<SupportPoint> &simplex, bool debugMode = false)
		{
			if(debugMode)
				return IntersectsStepVersion(model1, model2, simplex);

			//Reset();

			glm::vec3 dir = glm::vec3(1, 1, 1);
		
			c = Support(dir, model1, model2);
		 	
			dir = -c.AB;//negative direction

			b = Support(dir, model1, model2);

			if (glm::dot(b.AB, dir) < 0)
			{
				Reset();
				return make_pair(true, false);
			}
			dir = doubleCross(c.AB - b.AB, -b.AB);
	 
			nrPointsSimplex = 2; //begin with 2 points in simplex
	
			int steps = 0;
			while (steps<50)
			{
				a = Support(dir, model1, model2);
				if (glm::dot(a.AB, dir) < 0)
				{
					Reset();
					return make_pair(true, false);
				}
				else
				{
			 
					if (ContainsOrigin(dir))
					{
						simplex.push_back(a);
						simplex.push_back(b);
						simplex.push_back(c);
						simplex.push_back(d);

						Reset();
						return make_pair(true, true);
					}
				}
				steps++;

			}
	
			Reset();
			return make_pair(true, false);
		}

		//First bool is whether it is finished. Second bool is the actual result.
		std::pair<bool, bool> IntersectsStepVersion(Model* model1, Model* model2, std::vector<SupportPoint> &simplex)
		{
			static int steps;

			static glm::vec3 dir;
			
			if(firstrun)
			{
				steps = 0;

				dir = glm::vec3(1, 1, 1);
		
				c = Support(dir, model1, model2);
		 	
				dir = -c.AB;//negative direction

				b = Support(dir, model1, model2);

				if (glm::dot(b.AB, dir) < 0)
				{
					Reset();
					return make_pair(true, false);
				}
				dir = doubleCross(c.AB - b.AB, -b.AB);

				nrPointsSimplex = 2; //begin with 2 points in simplex

				firstrun = false;
			}

			while (steps<50)
			{
				a = Support(dir, model1, model2);
				if (glm::dot(a.AB, dir) < 0)
				{
					Reset();
					return make_pair(true, false);
				}
				else
				{
			 
					if (ContainsOrigin(dir))
					{
						simplex.push_back(a);
						simplex.push_back(b);
						simplex.push_back(c);
						simplex.push_back(d);

						Reset();
						return make_pair(true, true);
					}
				}
				steps++;

				return make_pair(false, false);
			}
	
			Reset();
			return make_pair(true, false);
		}

		//It's not neccessary to find the explicit Minkowski DIfference for GJK, however it may be handy for visualisation!
		std::vector<glm::vec3> MinkowskiDifference(std::vector<glm::vec3> shape1, std::vector<glm::vec3> shape2)
		{
	
		}

		//http://in2gpu.com/2014/05/18/gjk-algorithm-3d/
		bool ContainsOrigin(glm::vec3& dir)
		{
			if (nrPointsSimplex == 2)
			{
				return triangle(dir);
			}
			else if (nrPointsSimplex == 3)
			{
				return tetrahedron(dir);
			}
	
			return false;
		}

		//http://in2gpu.com/2014/05/18/gjk-algorithm-3d/
		bool triangle(glm::vec3& dir)
		{
			glm::vec3 ao = glm::vec3(-a.AB.x, -a.AB.y, -a.AB.z);
			glm::vec3 ab = b.AB - a.AB;
			glm::vec3 ac = c.AB - a.AB;
			glm::vec3 abc = glm::cross(ab, ac);

			//point is can't be behind/in the direction of B,C or BC

	
			glm::vec3 ab_abc = glm::cross(ab, abc);
			// is the origin away from ab edge? in the same plane
			//if a0 is in that direction than
			if (glm::dot(ab_abc, ao) > 0)
			{
				//change points
				c = b;
				b = a;

				//dir is not ab_abc because it's not point towards the origin
				dir = doubleCross(ab,ao);

				//direction change; can't build tetrahedron
				return false;
			}

	
			glm::vec3 abc_ac = glm::cross(abc, ac); 

			// is the origin away from ac edge? or it is in abc?
			//if a0 is in that direction than
			if (glm::dot(abc_ac, ao) > 0)
			{
				//keep c the same
				b = a;

				//dir is not abc_ac because it's not point towards the origin
				dir = doubleCross(ac, ao);
				
				//direction change; can't build tetrahedron
				return false;
			}

			//now can build tetrahedron; check if it's above or below
			if (glm::dot(abc, ao) > 0)
			{
				//base of tetrahedron
				d = c;
				c = b;
				b = a;

				//new direction
				dir = abc;
			}
			else
			{
				//upside down tetrahedron
				d = b;
				b = a;
				dir = -abc;
			}

			nrPointsSimplex = 3;
	
			return false;
		}

		//http://in2gpu.com/2014/05/18/gjk-algorithm-3d/
		bool tetrahedron(glm::vec3& dir)
		{
			glm::vec3 ao = -a.AB;//0-a
			glm::vec3 ab = b.AB - a.AB;
			glm::vec3 ac = c.AB - a.AB;
	 
			//build abc triangle
			glm::vec3 abc = glm::cross(ab, ac);

			//CASE 1
			if (glm::dot(abc, ao) > 0)
			{
				//in front of triangle ABC
				//we don't have to change the ao,ab,ac,abc meanings
				checkTetrahedron(ao,ab,ac,abc,dir);
			}
	 

			//CASE 2:
	 
			glm::vec3 ad = d.AB - a.AB;

			//build acd triangle
			glm::vec3 acd = glm::cross(ac, ad);

			//same direaction with ao
			if (glm::dot(acd, ao) > 0)
			{

				//in front of triangle ACD
				b = c;
				c = d;
				ab = ac;
				ac = ad;
				abc = acd;

				checkTetrahedron(ao, ab, ac, abc, dir);
			}

			//build adb triangle
			glm::vec3 adb = glm::cross(ad, ab);

			//case 3:
	 
			//same direaction with ao
			if (glm::dot(adb, ao) > 0)
			{

				//in front of triangle ADB

				c = b;
				b = d;

				ac = ab;
				ab = ad;

				abc = adb;
				checkTetrahedron(ao, ab, ac, abc, dir);
			}


			//origin in tetrahedron
			return true;

		}

		//http://in2gpu.com/2014/05/18/gjk-algorithm-3d/
		bool checkTetrahedron(const glm::vec3& ao,
										const glm::vec3& ab,
										const glm::vec3& ac,
										const glm::vec3& abc,
										glm::vec3& dir)
		{
	 
			//almost the same like triangle checks
			glm::vec3 ab_abc = glm::cross(ab, abc);

			if (glm::dot(ab_abc, ao) > 0)
			{
				c = b;
				b = a;

				//dir is not ab_abc because it's not point towards the origin;
				//ABxA0xAB direction we are looking for
				dir = doubleCross(ab, ao);
		 
				//build new triangle
				// d will be lost
				nrPointsSimplex = 2;

				return false;
			}

			glm::vec3 acp = glm::cross(abc, ac);

			if (glm::dot(acp, ao) > 0)
			{
				b = a;

				//dir is not abc_ac because it's not point towards the origin;
				//ACxA0xAC direction we are looking for
				dir = doubleCross(ac, ao);
		 
				//build new triangle
				// d will be lost
				nrPointsSimplex = 2;

				return false;
			}

			//build new tetrahedron with new base
			d = c;
			c = b;
			b = a;

			dir = abc;

			nrPointsSimplex = 3;

			return false;
		}
};

