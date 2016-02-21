#pragma once

#include "Common.h"
#include <vector>
#include "Support.h"
#include "Point.h"

//Gilbert Johnson Keerthi algorithm for determining whether or 
//not two convex solids intersect

//Implementation based on the mollyrocket.com/849 video
class GJK
{
	public:

		enum Mode { Draw, Step, Normal };

		static bool Intersects(Model* model1, Model* model2, std::vector<SupportPoint> &simplexForEPA)
		{
			std::vector<SupportPoint> simplex;
			simplex.clear();

			//arbitrary starting direction to get the ball rolling
			glm::vec3 dir = glm::vec3(1,0,0);

			SupportPoint sp = Support(dir, model1, model2); //Go as far as possible in search direction
			simplex.push_back(sp); 

			dir = -sp.AB; //Go in opposite direction (far side of origin)
			//= glm::normalize(dir); 

			int steps = 0;
			while(steps < 50)
			{
				SupportPoint A = Support(dir, model1, model2);

				if(glm::dot(A.AB, dir) < 0)  //Didn't reach the origin i.e. no intersection (can't enclose the origin)
					return false;

				simplex.push_back(A);

				if (ContainsOrigin(simplex, dir))  //Returns true if the new point results in a simplex which encloses the origin
				{
					simplexForEPA = simplex;
					return true;
				}

				steps++;
			}

			return false;
		}

		static bool ContainsOrigin(vector<SupportPoint>& simplex, glm::vec3& direction)
		{
			//There are always at least 2 points in the simplex
				
			if(simplex.size() == 2) //1-simplex (line)
				return Line(simplex, direction);
			else if (simplex.size() == 3) //2-simplex (triangle)
				return Triangle(simplex, direction);
			else if (simplex.size() == 4) //3-simplex (tetrahedron)		
				return Tetrahedron(simplex, direction);

			return false;
		}

		static bool Line(vector<SupportPoint>& simplex, glm::vec3 &direction)
		{
			SupportPoint A = simplex[1]; // A is the point we just added, the direction that we moved
			SupportPoint B = simplex[0]; // B is further from origin than A, because A went past the origin, so B cannot be the closest point

			glm::vec3 AO = -A.AB;		//The line from A to the origin;
			glm::vec3 AB = B.AB - A.AB;    

			if (isSameDirection(AO, AB)) //AB and AO are in the same direction, therefore the origin is in the edge region.
			{
				direction = glm::cross(glm::cross(AB, AO), AB); //cross the two vectors twice to get the vector perpendicular to AB in the direction AO
			}
			else //A is closest feature to the origin
			{	
				direction = AO;
							
				simplex.clear();
				simplex.push_back(A);
			}

			return false;
		}

		static bool Triangle(vector<SupportPoint>& simplex, glm::vec3 &direction)
		{
			SupportPoint A = simplex[2]; //A is always the last point inserted into the simplex. Because of this, assumptions can be made as to which feature is closest to the origin.
			SupportPoint B = simplex[1];
			SupportPoint C = simplex[0];

			glm::vec3 AO = -A.AB;
			glm::vec3 AB = B.AB - A.AB;
			glm::vec3 AC = C.AB - A.AB;

			glm::vec3 ABC = glm::cross(AB, AC);

			if (isSameDirection(glm::cross(ABC, AC), AO)) //Origin is in perpendicular direction of AC. This narrows down the closest feature to either AC, A, or AB
			{
				if (isSameDirection(AC, AO)) //The closest feature is AC
				{
					simplex.clear();
					simplex.push_back(C);
					simplex.push_back(A);

					direction = glm::cross(glm::cross(AC, AO), AC); //perpendicular of AC facing in direction of origin
				}
				else
				{
					//*
					if (isSameDirection(AB, AO)) //AB is the closest feature
					{
						simplex.clear();
						simplex.push_back(B);
						simplex.push_back(A);

						direction = glm::cross(glm::cross(AB, AO), AB); //perpendicular of AB facing in direction of origin
					}
					else //A is the closest feature
					{
						simplex.clear();
						simplex.push_back(A);
						
						direction = AO; //Search in direction from A to origin
					}
				}
			}
			else
			{
				if (isSameDirection(glm::cross(AB, ABC), AO)) 
				{
					//*
					if (isSameDirection(AB,AO)) //AB is the closest feature
					{
						simplex.clear();
						simplex.push_back(B);
						simplex.push_back(A);
						
						direction = glm::cross(glm::cross(AB,AO), AB); //perpendicular of AB facing in direction of origin
					}
					else //A is the closest feature
					{
						simplex.clear();
						simplex.push_back(A);
						
						direction = AO; //Search in direction from A to origin
					}
				}
				else //the origin is above or below the triangle plane
				{ 
					if (isSameDirection(ABC, AO)) //Origin is above the triangle plane (i.e. direction of triangle's normal)
					{
						simplex.clear();
						simplex.push_back(C);
						simplex.push_back(B);
						simplex.push_back(A);
						
						direction = ABC;
					}
					else //Origin is below the triangle plane (opposite direction of triangle's normal)
					{
						simplex.clear();
						simplex.push_back(B);
						simplex.push_back(C);
						simplex.push_back(A);
						
						direction = -ABC;
					}
				}
			}

			return false;
		}	

		static bool Tetrahedron(vector<SupportPoint>& simplex, glm::vec3 &direction)
		{
			SupportPoint A = simplex[3];
			SupportPoint B = simplex[2];
			SupportPoint C = simplex[1];
			SupportPoint D = simplex[0];

			glm::vec3 AO = -A.AB;
			
			glm::vec3 AB = B.AB - A.AB;
			glm::vec3 AC = C.AB - A.AB;
			glm::vec3 AD = D.AB - A.AB;

			glm::vec3 ABC = glm::cross(AB, AC);
			glm::vec3 ACD = glm::cross(AC, AD);
			glm::vec3 ADB = glm::cross(AD, AB);

			if (isSameDirection(ABC, AO)) //Origin is the the direction of the normal of ABC
			{
				simplex.clear();
				simplex.push_back(C);
				simplex.push_back(B);
				simplex.push_back(A);
					
				return Triangle(simplex, direction);
			}
			else if (isSameDirection(ADB,AO)) //Origin is the the direction of the normal of ADB
			{
				simplex.clear(); 
				simplex.push_back(B);
				simplex.push_back(D);
				simplex.push_back(A);

				return Triangle(simplex, direction);
			} 
			else if (isSameDirection(ACD,AO)) //Origin is the the direction of the normal of ACD
			{
				simplex.clear(); 
				simplex.push_back(D);
				simplex.push_back(C);
				simplex.push_back(A);

				return Triangle(simplex, direction);
			}

			//Just 3 triangle checks, as the face BCD can't possibly be the closest feature as A was the last added point to the simplex.

			return true; //The origin is INSIDE the tetrahedron
		}

		struct Debugger
		{
			bool firstrun;
			int steps;
			bool finished;

			std::vector<SupportPoint> simplex;  //a simplex is simply a list of points that form an n-dimensional construct
			glm::vec3 dir; //now a member variable for debug purposes

			Debugger()
			{
				Reset();
			}

			~Debugger()
			{

			}

			void Reset()
			{
				firstrun = true;
				steps = 0;
				finished = false;
				simplex.clear();
			}

			bool Intersects(Model* model1, Model* model2, std::vector<SupportPoint> &simplexForEPA)
			{
				if(firstrun)
				{
					simplex.clear();
					dir = glm::vec3(1,0,0); //arbitrary starting direction to get the ball rolling
					SupportPoint sp = Support(dir, model1, model2); //Go as far as possible in search direction
					simplex.push_back(sp); 

					dir = -sp.AB; //Go in opposite direction (far side of origin)
					//= glm::normalize(dir); 

					firstrun = false;
				}

				while(steps < 50)
				{
					SupportPoint A = Support(dir, model1, model2);

					if(glm::dot(A.AB, dir) < 0)  //Didn't reach the origin i.e. no intersection (can't enclose the origin)
					{
						finished = true;
						return false;
					}

					simplex.push_back(A);

					if (GJK::ContainsOrigin(simplex, dir))  //Returns true if the new point results in a simplex which encloses the origin
					{
						simplexForEPA = simplex;
						finished = true;
						return true;
					}

					steps++;
				
					finished = false;
					return false;
					
				}

				finished = true;
				return false;
			}
		};
};

