#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm/gtx/random.hpp>

#include <vector>
#include <queue>
#include "PhysicsHelper.h"

using namespace std;

//TODO 
//	-implement recylcing
//  -implement evolution?
//	-at least 1000 particles
//	-at least two forces
//	-particle plane collision handling
//		-recycle when hit
//

//optional
//	-rk4
//	-model something real
//		-snow?

struct Fluid
{
   float density;
   float viscosity;
};

struct Environment
{
	float gravity;
    glm::vec3 wind;
    Fluid fluid;
};

enum IntegratorMode { Euler, RK4, None };

struct Particle
{
	bool active;

	glm::vec3 position;
	glm::vec3 velocity;

	glm::vec3 acceleration;

	float mass;
	
	float radius, surfaceArea, dragCoefficient; //drag stuff

	Particle(glm::vec3 startPos/*, float mass*/)
	{
		position = startPos;
		velocity = glm::vec3(0);
		acceleration = glm::vec3(0);

		this->mass = 1;

		dragCoefficient = 0.47f; //sphere  
		
		radius = 0.05f;
        surfaceArea = 3.14159265 * glm::pow(radius, 2.0f); //sphere

		active = true;
	}

	void Reset()
	{
		position = glm::vec3(0);
		velocity = glm::vec3(0);
		acceleration = glm::vec3(0);
	}

	glm::vec3 fNet(Environment env)
	{
		return Gravity(env, *this) + PressureDrag(env, *this, true);
	}

	void Euler(Environment env, float timeStep)
	{
        position += timeStep * velocity;
        velocity += timeStep * acceleration;

		acceleration = fNet(env) / mass;
	}

	void RK4(Environment env, float timeStep)
	{
		glm::vec3 p2 = position + velocity * (timeStep / 2);
		glm::vec3 v2 = velocity + acceleration * (timeStep / 2);
		glm::vec3 a2 = acceleration;//f(env) / mass;

		glm::vec3 p3 = position + v2 * (timeStep / 2);
		glm::vec3 v3 = velocity + a2 * (timeStep / 2);
		glm::vec3 a3 = a2;//f(env) / mass;

		glm::vec3 p4 = position + v3 * timeStep;
		glm::vec3 v4 = velocity + a3 * timeStep;
		glm::vec3 a4 = a3;//f(env) / mass;

		position += (velocity + (2.0f * v2) + (2.0f * v3) + v4) * (timeStep / 6);
		velocity += (acceleration + (2.0f * a2) + (2.0f * a3) + a4) * (timeStep / 6);
		acceleration = fNet(env) / mass;   
	}
};

struct Emitter
{
	//float emitRate;

	glm::vec3 centre;
	//area

	//particle lifetime
	//colour
	//animationColour()

	//tex support?

	//mesh object emitter.

	float FuzzifyValue(float value, float variance)
	{
		return RandomFloat(value - variance/2, value + variance/2);
	}

	Emitter()
	{
		centre = glm::vec3(0);
	}

	void Emit(Particle* particle)
	{
		particle->position = centre;
		particle->velocity = glm::vec3(FuzzifyValue(0, 2), FuzzifyValue(4, 2), FuzzifyValue(0, 2));
	}
};

class ParticleSystem
{
	private:

		std::vector<Particle*> particles;
		std::queue<Particle*> inactiveParticles;

		Emitter emitter;
		
		GLuint vao;
		GLuint positionBuffer;

		int size;

	public:

		IntegratorMode mode;
		Environment env;
		int liveParticles;
		float simulationSpeed;

		ParticleSystem()
		{
			simulationSpeed = 0.4f;

			env.gravity = 9.81f; //earth
            
			env.wind = glm::vec3(300, 50, 400);
			env.fluid.density = 1.225f; //air
            env.fluid.viscosity = 18.1f; //air

			size = 1000;
			liveParticles = 0;

			for(int i = 0; i < size; i++)
			{
				glm::vec3 pos = glm::vec3(RandomFloat(-0.5, 0.5), 10, RandomFloat(-0.5, 0.5));
				
				Particle* p = new Particle(pos);
				
				p->active = false;

				inactiveParticles.push(p);
				particles.push_back(p);
			}

			mode = IntegratorMode::RK4;
		}

		~ParticleSystem()
		{
			//clean up particles
		}

		void Generate()
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &positionBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 3 * this->size, nullptr/*nothing*/, GL_STREAM_DRAW);
			
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3)*sizeof(float), (void *)((0)*sizeof(float)));//?

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);//?
		}

		void Update(double deltaTime)
		{
			std::vector<glm::vec3> positions;

			while(inactiveParticles.size() > 0)
			{
				//inactiveParticles.front()->position = glm::vec3(RandomFloat(-0.5, 0.5), 10, RandomFloat(-0.5, 0.5));
				
				Particle* p = inactiveParticles.front();
				emitter.Emit(p);
				inactiveParticles.front()->active = true;
				inactiveParticles.pop();
			}

			//if(particles.size() == 0)
				//return;

			for(int i = 0; i < particles.size(); i++)
			{
				if(particles[i]->active)
				{
					if(mode == IntegratorMode::Euler)
						particles[i]->Euler(env, deltaTime/1000 * simulationSpeed);
					else if (mode == IntegratorMode::RK4)
						particles[i]->RK4(env, deltaTime/1000 * simulationSpeed);

					if(particles[i]->position.y < 0)
					{
						inactiveParticles.push(particles[i]);
						particles[i]->active = false;
					}

					positions.push_back(particles[i]->position);
				}
			}

			liveParticles = positions.size();

			//Send new positions to buffer
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size()*sizeof(positions[0]), &positions[0]);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void Render()
		{
			glBindVertexArray(vao);

			glPointSize(3.5f);              //specify size of points in pixels
			glDrawArrays(GL_POINTS, 0, size - inactiveParticles.size());
 
			glBindVertexArray(0);
		}
};
