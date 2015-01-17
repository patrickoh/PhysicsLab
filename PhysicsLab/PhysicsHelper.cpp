#include "ParticleSystem.h"
#include "PhysicsHelper.h"

glm::vec3 Gravity(Environment env, Particle particle)
{
	return particle.mass * env.gravity * glm::vec3(0,-1,0);
}

glm::vec3 PressureDrag(Environment env, Particle particle, bool wind)
{
	 if(wind) 
		return 0.5f * env.fluid.density * particle.surfaceArea 
			* particle.dragCoefficient 
			* (particle.velocity - env.wind).length() * -(particle.velocity - env.wind);
	 else
		return 0.5f * env.fluid.density * particle.surfaceArea 
			* particle.dragCoefficient 
			* particle.velocity.length() * -particle.velocity;
}

float RandomFloat(float lo, float hi)
{
	return lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
}