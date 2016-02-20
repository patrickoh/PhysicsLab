#include "ParticleDemo.h"
#include "RigidBodyDemo.h"
#include "BroadphaseDemo.h"
#include "SoftBodyDemo.h"
#include "NarrowphaseDemo.h"
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
NarrowphaseDemo* NarrowphaseDemo::Instance;

Camera* Camera::Instance;

glm::vec3 GLProgram::normal[6] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
glm::vec3 GLProgram::plane[6] = { glm::vec3(0,-5,0), glm::vec3(0,5,0), glm::vec3(-5,0,0), glm::vec3(5,0,0), glm::vec3(0,0,5), glm::vec3(0,0,-5) };

int main(int argc, char** argv)
{
	GLProgram* demo;

	int demoIndex = -1;
	do {
		std::cin >> demoIndex;
	}
	while(demoIndex < 0 || demoIndex > 4);

	if(demoIndex == 0)
		demo = new ParticleDemo();	
	else if(demoIndex == 1)
		demo = new RigidBodyDemo();
	else if(demoIndex == 2)
		demo = new BroadphaseDemo();
	else if(demoIndex == 3)
		demo = new SoftBodyDemo();
	else if(demoIndex == 4)
		demo = new NarrowphaseDemo();

	demo->Init(argc, argv);
	demo->Run();
	delete demo;

	return 0;
}