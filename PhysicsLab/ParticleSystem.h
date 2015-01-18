#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm/gtx/random.hpp>

#include <vector>
#include <queue>

#include "Model.h"
#include "PhysicsHelper.h"

using namespace std;

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

struct BufferData
{
	glm::vec3 position;
	glm::vec4 colour;
};

enum IntegratorMode { Euler, RK4, None };

struct Particle
{
	bool active;

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;

	glm::vec4 colour;
	float age;

	Particle(glm::vec3 startPos)
	{
		position = startPos;
		velocity = glm::vec3(0);
		acceleration = glm::vec3(0);

		active = true;
		age = 0;
	}

	void Reset()
	{
		position = glm::vec3(0);
		velocity = glm::vec3(0);
		acceleration = glm::vec3(0);

		age = 0;
	}	
};

struct Emitter
{
	int emitRate;
	glm::vec3 centre;
	
	//TODO
	//area
	//tex support?
	//mesh object emitter.

	Emitter()
	{
		centre = glm::vec3(0);
		emitRate = 10;
	}

	void Emit(Particle* particle)
	{
		particle->position = centre;
		particle->velocity = glm::linearRand(glm::vec3(-1.0f, 3.0f, -1.0f),glm::vec3(1.0f, 5.0f, 1.0f));
	}
};

class ParticleSystem
{
	private:

		std::vector<Particle*> particles;
		std::queue<Particle*> inactiveParticles;

		GLuint vao;
		GLuint vbo;

		GLuint posBuffer;
		GLuint colBuffer;

		int size;

	public:

		IntegratorMode mode;

		Environment env;
		float windScalar;
		
		int liveParticles;
		float simulationSpeed;

		bool gravity;
		bool wind;
		bool drag;

		bool bCollisions;

		glm::vec3 plane;
		glm::vec3 normal;
		float coefficientOfRestitution;

		float radius, surfaceArea, dragCoefficient; //drag stuff

		float mass;
		float particleLife;

		Emitter emitter;

		glm::vec4 startColour;
		glm::vec4 endColour;

		ParticleSystem(int size = 1000)
		{
			plane = glm::vec3(0, 0, 0);
			normal = glm::vec3(0,1,0);

			particleLife = 5;

			coefficientOfRestitution = 0.5f;

			simulationSpeed = 0.4f;

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

			mass = 1;

			this->size = size;
			liveParticles = 0;

			for(int i = 0; i < size; i++)
			{
				glm::vec3 pos = glm::vec3(RandomFloat(-0.5, 0.5), 10, RandomFloat(-0.5, 0.5));
				
				Particle* p = new Particle(pos);
				
				p->active = false;

				inactiveParticles.push(p);
				particles.push_back(p);
			}

			bCollisions = true;

			mode = IntegratorMode::RK4;

			startColour = glm::vec4(1,1,1,1);
			endColour = glm::vec4(1,0,0,1);
		}

		~ParticleSystem()
		{

		}

		void Generate()
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, this->size * sizeof(BufferData), nullptr, GL_STREAM_DRAW);  

			glEnableVertexAttribArray(0); 
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BufferData), (GLvoid*)0);	

			glEnableVertexAttribArray(1); 
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(BufferData), (GLvoid*)offsetof(BufferData, colour));
			
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void Update(double deltaTime)
		{
			std::vector<BufferData> data;

			float timestep = deltaTime/1000;

			for(int i = 0; i < emitter.emitRate; i++)
			{
				if(inactiveParticles.size() > 0)
				{
					Particle* p = inactiveParticles.front();
					emitter.Emit(p);
					inactiveParticles.front()->active = true;
					inactiveParticles.pop();
				}
			}

			for(int i = 0; i < particles.size(); i++)
			{
				if(particles[i]->active)
				{
					//INTEGRATION
					if(mode == IntegratorMode::Euler)
						Euler(particles[i], timestep * simulationSpeed);
					else if (mode == IntegratorMode::RK4)
						RK4(particles[i], timestep * simulationSpeed);

					//CD/CR
					if(glm::dot(particles[i]->position - plane, normal) < 0.01f
						&& glm::dot(normal, particles[i]->velocity) < 0.01f)
					{
						if(bCollisions)
						{
							particles[i]->velocity += (1 + coefficientOfRestitution) * -(particles[i]->velocity * normal) * normal;
						}
						else
						{
							inactiveParticles.push(particles[i]);
							particles[i]->active = false;
							particles[i]->Reset();
						}
					}

					//ADVANCE AGE & UPDATE COLOUR
					particles[i]->age += timestep;
					particles[i]->colour = glm::mix(startColour, endColour, particles[i]->age / particleLife);
					if(particles[i]->age > particleLife)
					{
						inactiveParticles.push(particles[i]);
						particles[i]->active = false;
						particles[i]->Reset();
					}

					//ADD TO BUFFER
					BufferData bd;
					bd.position = particles[i]->position;
					bd.colour = particles[i]->colour;
					data.push_back(bd);
				}
			}

			liveParticles = data.size();

			if(liveParticles > 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glBufferSubData(GL_ARRAY_BUFFER, 0, data.size()*sizeof(data[0]), &data[0]);
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		void Render()
		{
			glBindVertexArray(vao);

			glPointSize(1.5f);              //specify size of points in pixels
			glDrawArrays(GL_POINTS, 0, size - inactiveParticles.size());
 
			glBindVertexArray(0);
		}

		void Euler(Particle* p, float timeStep)
		{
			p->position += timeStep * p->velocity;
			p->velocity += timeStep * p->acceleration;

			p->acceleration = fNet(p->position, p->velocity) / mass;
		}

		void RK4(Particle* p, float timeStep)
		{
			glm::vec3 p2 = p->position + p->velocity * (timeStep / 2);
			glm::vec3 v2 = p->velocity + p->acceleration * (timeStep / 2);
			glm::vec3 a2 = fNet(p2, v2) / mass;  

			glm::vec3 p3 = p->position + v2 * (timeStep / 2);
			glm::vec3 v3 = p->velocity + a2 * (timeStep / 2);
			glm::vec3 a3 = fNet(p3, v3) / mass;  

			glm::vec3 p4 = p->position + v3 * timeStep;
			glm::vec3 v4 = p->velocity + a3 * timeStep;
			glm::vec3 a4 = fNet(p4, v4) / mass;  

			p->position += (p->velocity + (2.0f * v2) + (2.0f * v3) + v4) * (timeStep / 6);
			p->velocity += (p->acceleration + (2.0f * a2) + (2.0f * a3) + a4) * (timeStep / 6);
			p->acceleration = fNet(p->position, p->velocity) / mass;   
		}

		glm::vec3 fNet(glm::vec3 pos, glm::vec3 vel)
		{
			glm::vec3 fNet;

			if(gravity)
				fNet += Gravity(pos, vel);
			if(drag)
				if(wind)
					fNet += PressureDrag(pos, vel, true);
				else
					fNet += PressureDrag(pos, vel, false);

			return fNet;
		}

		glm::vec3 Gravity(glm::vec3 pos, glm::vec3 vel)
		{
			return mass * env.gravity * glm::vec3(0,-1,0);
		}

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
