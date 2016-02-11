#include "ParticleDemo.h"
#include "RigidBodyDemo.h"
#include "BroadphaseDemo.h"
#include "SoftBodyDemo.h"
#include "GLProgram.h"
#include "QueryPerformance.h"

long long int QueryPerformance::ts = 0;
long long int QueryPerformance::tf = 0;
std::map<std::string, sint64> QueryPerformance::results;

GLProgram* GLProgram::Instance;

ParticleDemo* ParticleDemo::Instance;
RigidBodyDemo* RigidBodyDemo::Instance;
BroadphaseDemo* BroadphaseDemo::Instance;
SoftBodyDemo* SoftBodyDemo::Instance;

Camera* Camera::Instance;

glm::vec3 RigidBodyDemo::normal[6] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
glm::vec3 RigidBodyDemo::plane[6] = { glm::vec3(0,-5,0), glm::vec3(0,5,0), glm::vec3(-5,0,0), glm::vec3(5,0,0), glm::vec3(0,0,5), glm::vec3(0,0,-5) };
glm::vec3 SoftBodyDemo::normal[6] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
glm::vec3 SoftBodyDemo::plane[6] = { glm::vec3(0,-5,0), glm::vec3(0,5,0), glm::vec3(-5,0,0), glm::vec3(5,0,0), glm::vec3(0,0,5), glm::vec3(0,0,-5) };

int main(int argc, char** argv)
{
	GLProgram* demo;

	int demoIndex = -1;
	do {
		std::cin >> demoIndex;
	}
	while(demoIndex < 0 || demoIndex > 3);

	if(demoIndex == 0)
		demo = new ParticleDemo();	
	else if(demoIndex == 1)
		demo = new RigidBodyDemo();
	else if(demoIndex == 2)
		demo = new BroadphaseDemo();
	else if(demoIndex == 3)
		demo = new SoftBodyDemo();

	demo->Init(argc, argv);
	demo->Run();
	delete demo;

	return 0;
}