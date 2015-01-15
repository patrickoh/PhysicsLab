#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm/gtx/random.hpp>

#include <vector>
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
	glm::vec3 position;
	glm::vec3 velocity;

	glm::vec3 acceleration;

	float mass;
	
	float radius, surfaceArea, dragCoefficient; //drag stuff

	glm::vec3 forceAccumulator;

	Particle(glm::vec3 startPos/*, float mass*/)
	{
		position = startPos;
		velocity = glm::vec3(0);
		acceleration = glm::vec3(0);

		this->mass = 1;

		dragCoefficient = 0.47f; //sphere  
	}

	void Integrate(IntegratorMode mode, Environment env, float timeStep)
	{
		if (mode == IntegratorMode::Euler)
        {
            position += timeStep * velocity;
            velocity += timeStep * acceleration;

			forceAccumulator = GravityForce(env, *this); //+ PressureDrag(env, *this, true);
			acceleration = forceAccumulator / mass;
        }
	}

	void Draw()
	{

	}
};

class ParticleSystem
{
	private:

		std::vector<Particle*> particles;
		IntegratorMode mode;

		Environment env;

		GLuint vao;
		GLuint positionBuffer;

	public:

		ParticleSystem()
		{
			env.gravity = 9.81f; //earth
            
			env.wind = glm::vec3(3, 5, 0);
			env.fluid.density = 1.225f; //air
            env.fluid.viscosity = 18.1f; //air

			for(int i = 0; i < 1000; i++)
			{
				glm::vec3 pos = glm::vec3(RandomFloat(-5, 5), 10, RandomFloat(-5, 5));
				particles.push_back(new Particle(pos));
			}

			Generate();
		}

		~ParticleSystem()
		{
		}

		void Generate()
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &positionBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 4 * particles.size(), nullptr/*nothing*/, GL_STREAM_DRAW);
			
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, (4)*sizeof(float), (void *)((0)*sizeof(float)));//?

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);//?
		}

		void Update(double deltaTime)
		{
			for(int i = 0; i < particles.size(); i++)
				particles[i]->Integrate(IntegratorMode::Euler, env, deltaTime/1000);

			//Send new positions to buffer

			/*assert(d_system != nullptr);
				assert(d_buffer_position > 0 && d_buffer_column > 0);

				const size_t count = d_system->alive_particles_count();
				if (count > 0)
				{
					glBindBuffer(GL_ARRAY_BUFFER, d_buffer_position);
					float *ptr = (float *)(d_system->finalData()->m_position.get());
					glBufferSubData(GL_ARRAY_BUFFER, 0, count*sizeof(float)* 4, ptr);

					glBindBuffer(GL_ARRAY_BUFFER, d_buffer_column);
					ptr = (float*)(d_system->finalData()->m_col.get());
					glBufferSubData(GL_ARRAY_BUFFER, 0, count*sizeof(float)* 4, ptr);

					glBindBuffer(GL_ARRAY_BUFFER, 0);
				}*/
		}

		void Render()
		{
			//glBindVertexArray(vao);

			//glPointSize(whatever);              //specify size of points in pixels
			//glDrawArrays(GL_POINTS, 0, particles.size());
 
			//glBindVertexArray(0);
		}
};
