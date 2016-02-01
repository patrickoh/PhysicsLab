#include "Model.h"

struct Fluid
{
   float density;
   //float viscosity;
};

struct Environment
{
	float gravity;
    
	glm::vec3 wind;
	float windScalar;

    Fluid fluid;
};

enum IntegratorMode { Euler, RK2, RK4, None };

class Integrator
{
	private:

		Environment env;
		IntegratorMode mode;
		Environment env;

		//float simulationSpeed;

		bool gravity;
		bool wind;
		bool drag;

		float radius, surfaceArea, dragCoefficient; //Drag stuff

	public:

		Integrator* Instance;

		Integrator()
		{
			Instance = this;

			mode = IntegratorMode::RK4;

			gravity = true;
			drag =  true;
			wind = false;

			env.gravity = 9.81f; //earth
            
			env.wind = glm::vec3(1, 0, 0);
			env.windScalar = 1000.0f;

			env.fluid.density = 1.225f; //air
            //env.fluid.viscosity = 18.1f; //air

			dragCoefficient = 0.47f; //sphere  
			radius = 0.05f;
			surfaceArea = 3.14159265 * glm::pow(radius, 2.0f); //sphere
		}

		//model->worldProperties.translation += velocity * timestep;
		//velocity += (mass * forceNet)/mass * timestep;
		void Euler(glm::vec3 &position, glm::vec3 &velocity, glm:: vec3 &acceleration, float mass, float timeStep)
		{
			position += timeStep * velocity;
			velocity += timeStep * acceleration;
			acceleration = fNet(position, velocity, mass) / mass;
		}

		//Also known as RK2, Improved Euler method, or Heun's Method
		void Midpoint(glm::vec3 &position, glm::vec3 &velocity, glm:: vec3 &acceleration, float mass, float timeStep)
		{
			glm::vec3 p2 = position + velocity * timeStep;
			glm::vec3 v2 = velocity + acceleration * timeStep;
			glm::vec3 a2 = fNet(p2, v2, mass) / mass;

			position += (velocity + v2) * (timeStep / 2);
			velocity += (acceleration + a2) * (timeStep / 2);
			acceleration = fNet(position, velocity) / mass;   
		}

		void RK4(glm::vec3 &position, glm::vec3 &velocity, glm:: vec3 &acceleration, float mass, float timeStep)
		{
			glm::vec3 p2 = position + velocity * (timeStep / 2);
			glm::vec3 v2 = velocity + acceleration * (timeStep / 2);
			glm::vec3 a2 = fNet(p2, v2, mass) / mass;  

			glm::vec3 p3 = position + v2 * (timeStep / 2);
			glm::vec3 v3 = velocity + a2 * (timeStep / 2);
			glm::vec3 a3 = fNet(p3, v3, mass) / mass;  

			glm::vec3 p4 = position + v3 * timeStep;
			glm::vec3 v4 = velocity + a3 * timeStep;
			glm::vec3 a4 = fNet(p4, v4, mass) / mass;  

			position += (velocity + (2.0f * v2) + (2.0f * v3) + v4) * (timeStep / 6);
			velocity += (acceleration + (2.0f * a2) + (2.0f * a3) + a4) * (timeStep / 6);
			acceleration = fNet(position, velocity, mass) / mass;   
		}

		glm::vec3 fNet(glm::vec3 pos, glm::vec3 vel, float mass)
		{
			glm::vec3 fNet;

			if(gravity)
				fNet += Gravity(pos, vel, mass);
			if(drag)
				if(wind)
					fNet += PressureDrag(pos, vel, true);
				else
					fNet += PressureDrag(pos, vel, false);
			//if(bUserGravity)
				//fNet += Attract(pos, vel);

			return fNet;
		}

		glm::vec3 Gravity(glm::vec3 pos, glm::vec3 vel, float mass)
		{
			return mass * env.gravity * glm::vec3(0,-1,0);
		}

		/*glm::vec3 Attract(glm::vec3 pos, glm::vec3 vel)
		{
			return (userGravity - pos) * userGravityScalar;
		}*/

		glm::vec3 PressureDrag(glm::vec3 pos, glm::vec3 vel, bool wind)
		{
			 if(wind) 
				return 0.5f * env.fluid.density * surfaceArea * dragCoefficient 
					* (vel - (env.wind * env.windScalar)).length() * -(vel - (env.wind * env.windScalar));
			 else
				return 0.5f * env.fluid.density * surfaceArea * dragCoefficient 
					* vel.length() * -vel;
		}	
};