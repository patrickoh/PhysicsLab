#include "ParticleDemo.h"
#include "RigidBodyDemo.h"
#include "BroadphaseDemo.h"
#include "SoftBodyDemo.h"
#include "NarrowphaseDemo.h"
#include "ResponseDemo.h"
#include "SolverDemo.h"
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
ResponseDemo* ResponseDemo::Instance;
SolverDemo* SolverDemo::Instance;

Camera* Camera::Instance;

glm::vec3 GLProgram::normal[6] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), 
	glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
glm::vec3 GLProgram::plane[6] = { glm::vec3(0,-5,0), glm::vec3(0,5,0), 
	glm::vec3(-5,0,0), glm::vec3(5,0,0), glm::vec3(0,0,5), glm::vec3(0,0,-5) };

int SolverDemo::stackHeight = 5;

int main(int argc, char** argv)
{
	GLProgram* demo;

	std::cout << "1. Particle System" << std::endl;
	std::cout << "2. Unconstrained Rigid Body" << std::endl;
	std::cout << "3. Broadphase - Sweep and Prune" << std::endl;
	std::cout << "4. Soft Body (Side Project)" << std::endl;
	std::cout << "5. Narrowphase - GJK" << std::endl;
	std::cout << "6. Response - EPA" << std::endl;
	std::cout << "7. Solver" << std::endl;
	std::cout << ">";

	int demoIndex = -1;
	do {
		std::cin >> demoIndex;
	}
	while(demoIndex < 1 || demoIndex > 7);

	if(demoIndex == 1)
		demo = new ParticleDemo();	
	else if(demoIndex == 2)
		demo = new RigidBodyDemo();
	else if(demoIndex == 3)
		demo = new BroadphaseDemo();
	else if(demoIndex == 4)
		demo = new SoftBodyDemo();
	else if(demoIndex == 5)
		demo = new NarrowphaseDemo();
	else if(demoIndex == 6)
		demo = new ResponseDemo();
	else if(demoIndex == 7)
		demo = new SolverDemo();

	demo->Init(argc, argv);
	demo->Run();
	delete demo;

	return 0;
}