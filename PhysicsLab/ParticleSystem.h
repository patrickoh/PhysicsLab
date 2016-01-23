#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm/gtx/random.hpp>

#include <vector>
#include <queue>

#include "Model.h"

#include <AntTweakBar.h>

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

enum IntegratorMode { Euler, RK2, RK4, None };

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

	glm::vec3 velRangeMin, velRangeMax; 

	Emitter()
	{
		centre = glm::vec3(0);
		emitRate = 10;

		velRangeMin = glm::vec3(-1.0f, 3.0f, -1.0f);
		velRangeMax = glm::vec3(1.0f, 5.0f, 1.0f);
	}

	void Emit(Particle* particle)
	{
		particle->position = centre;
		particle->velocity = glm::linearRand(velRangeMin, velRangeMax);
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

		int maxSize;

		GLuint textureID;

	public:

		IntegratorMode mode;
		Environment env;
		
		int liveParticles;
		float simulationSpeed;

		bool gravity;
		bool wind;
		bool drag;

		bool bCollisions;
		glm::vec3 plane, normal;
		float coefficientOfRestitution; //Collision stuff

		float radius, surfaceArea, dragCoefficient; //Drag stuff

		float mass;
		
		Emitter emitter;

		glm::vec4 startColour;
		glm::vec4 endColour;

		float particleLife;

		Model* planeModel;

		ParticleSystem(int size = 1000)
		{
			textureID = loadTexture("particle.DDS");

			startColour = glm::vec4(1,1,1,1);
			endColour = glm::vec4(1,0,0,1);

			particleLife = 5;
			
			plane = glm::vec3(0, 0, 0);
			normal = glm::vec3(0,1,0);

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

			this->maxSize = size;
			liveParticles = 0;

			for(int i = 0; i < size; i++)
			{	
				Particle* p = new Particle(emitter.centre);
				
				p->active = false;

				inactiveParticles.push(p);
				particles.push_back(p);
			}

			bCollisions = true;

			mode = IntegratorMode::RK4;
		}

		~ParticleSystem()
		{
			//clean up particles
			for(auto it = particles.begin(); it != particles.end(); it++)
			{
				delete *it;
			}

			while(inactiveParticles.size() > 0)
			{
				Particle* p = inactiveParticles.front();
				inactiveParticles.pop();
				delete p;
			}
		}

		void Generate()
		{
			glGenVertexArrays(1, &vao); 
			glBindVertexArray(vao); 

			// The VBO containing the 4 vertices of the particles.
			// Thanks to instancing, they will be shared by all particles.
			/*static const GLfloat g_vertex_buffer_data[] = { 
				 -0.5f, -0.5f, 0.0f,
				  0.5f, -0.5f, 0.0f,
				 -0.5f,  0.5f, 0.0f,
				  0.5f,  0.5f, 0.0f,
			};
			GLuint billboard_vertex_buffer;
			glGenBuffers(1, &billboard_vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);*/

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, this->maxSize * sizeof(BufferData), nullptr, GL_STREAM_DRAW);  

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
					else if(mode == IntegratorMode::RK2)
						Midpoint(particles[i], timestep * simulationSpeed);
					else if (mode == IntegratorMode::RK4)
						RK4(particles[i], timestep * simulationSpeed);

					//CD/CR
					if(glm::dot(particles[i]->position - plane, normal) < 0.01f
						&& glm::dot(normal, particles[i]->velocity) < 0.01f)
					{
						if(bCollisions)
						{
							particles[i]->position += -glm::dot(particles[i]->position - plane, normal) * normal; //post processing method
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
					particles[i]->age += timestep * simulationSpeed;
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
				
				//glBufferSubData(GL_ARRAY_BUFFER, 0, data.size()*sizeof(data[0]), &data[0]);

				glBufferData(GL_ARRAY_BUFFER, maxSize*sizeof(data[0]), NULL, GL_STREAM_DRAW); // Buffer orphaning
				glBufferSubData(GL_ARRAY_BUFFER, 0, liveParticles*sizeof(data[0]), &data[0]);
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
		}

		void Render()
		{
			glBindVertexArray(vao);

			glActiveTexture(GL_TEXTURE0);
			//glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 0); //set the sampler in the shader to the correct texture 
			glBindTexture(GL_TEXTURE_2D, textureID);

			//glPointSize(1.5f);              //specify size of points in pixels
			glDrawArrays(GL_POINTS, 0, maxSize - inactiveParticles.size());

			//glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
			//glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
			//glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1
 
			glBindVertexArray(0);
		}

		void Euler(Particle* p, float timeStep)
		{
			p->position += timeStep * p->velocity;
			p->velocity += timeStep * p->acceleration;

			p->acceleration = fNet(p->position, p->velocity) / mass;
		}

		//Also known as RK2, Improved Euler method, or Heun's Method
		void Midpoint(Particle* p, float timeStep)
		{
			glm::vec3 p2 = p->position + p->velocity * timeStep;
			glm::vec3 v2 = p->velocity + p->acceleration * timeStep;
			glm::vec3 a2 = fNet(p2, v2) / mass;

			p->position += (p->velocity + v2) * (timeStep / 2);
			p->velocity += (p->acceleration + a2) * (timeStep / 2);
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
