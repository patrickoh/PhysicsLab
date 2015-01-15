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

enum IntegratorMode { Euler, RK4 };

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

		active = true;
	}

	void Reset()
	{
		position = glm::vec3(0);
		velocity = glm::vec3(0);
		acceleration = glm::vec3(0);
	}

	glm::vec3 f(Environment env)
	{
		return GravityForce(env, *this); //+ PressureDrag(env, *this, true);
	}

	void Integrate(IntegratorMode mode, Environment env, float timeStep)
	{
		if (mode == IntegratorMode::Euler)
        {
            position += timeStep * velocity;
            velocity += timeStep * acceleration;

			acceleration = f(env) / mass;
        }
		else if (mode == IntegratorMode::RK4)
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
			acceleration = f(env) / mass;
        }
	}
};

struct Emitter
{
	//float emitRate;

	glm::vec3 centre;
	//area

	struct VelLimits
	{
		float ymin, ymax;
		float xmin, xmax;
		float zmin, zmax;

		VelLimits() {}

		VelLimits(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax) 
			: xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax), zmin(zmin), zmax(zmax) {}

	} velLimits;

	Emitter()
	{
		centre = glm::vec3(0);
		velLimits = VelLimits(-1.0f, 1.0f, 3.0f, 5.0f, -1.0f, 1.0f);
	}

	void Emit(Particle* particle)
	{
		particle->position = centre;
		particle->velocity = 
			glm::vec3(RandomFloat(velLimits.xmin, velLimits.xmax), 
				RandomFloat(velLimits.ymin, velLimits.ymax), RandomFloat(velLimits.zmin, velLimits.zmax));
	}
};

class ParticleSystem
{
	private:

		std::vector<Particle*> particles;
		std::queue<Particle*> inactiveParticles;

		Emitter emitter;
		
		IntegratorMode mode;

		Environment env;

		GLuint vao;
		GLuint positionBuffer;

		float speedScalar;

		int size;

	public:

		ParticleSystem()
		{
			speedScalar = 0.4f;

			env.gravity = 9.81f; //earth
            
			env.wind = glm::vec3(3, 5, 0);
			env.fluid.density = 1.225f; //air
            env.fluid.viscosity = 18.1f; //air

			size = 1000;

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

			if(particles.size() == 0)
				return;

			for(int i = 0; i < particles.size(); i++)
			{
				if(particles[i]->active)
				{
					particles[i]->Integrate(mode, env, deltaTime/1000 * speedScalar);

					if(particles[i]->position.y < 0)
					{
						inactiveParticles.push(particles[i]);
						particles[i]->active = false;
					}

					positions.push_back(particles[i]->position);
				}
			}

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
