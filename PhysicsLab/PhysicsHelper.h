#include <glm\glm.hpp>

class ParticleSystem;
struct Environment;
struct Particle;

glm::vec3 Gravity(Environment env, Particle particle);
glm::vec3 PressureDrag(Environment env, Particle particle, bool wind);

float RandomFloat(float lo, float hi);