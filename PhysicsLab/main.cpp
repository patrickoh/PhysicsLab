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

glm::vec3 GLProgram::normal[6] = { glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,0,-1), glm::vec3(0,0,1) };
glm::vec3 GLProgram::plane[6] = { glm::vec3(0,-5,0), glm::vec3(0,5,0), glm::vec3(-5,0,0), glm::vec3(5,0,0), glm::vec3(0,0,5), glm::vec3(0,0,-5) };

int main(int argc, char** argv)
{
	GLProgram* demo;

	std::cout << "1. Particle System" << std::endl;
	std::cout << "2. Unconstrained Rigid Body" << std::endl;
	std::cout << "3. Broadphase" << std::endl;
	std::cout << "4. Soft Body (Side Project)" << std::endl;
	std::cout << "5. Narrowphase" << std::endl;
	std::cout << "6. Response" << std::endl;
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

#include "Bounce.h"

//int main()
//{
//	b3World* m_world = new b3World();
//
//	b3TimeStep m_step;
//	m_step.dt = 1.0 / 60.0f;
//	m_step.velocityIterations = 10;
//	//m_step.sleeping = true;
//	
//	b3BodyDef bodyDef;
//	bodyDef.gravityScale = 1.0f;
//	bodyDef.position = b3Vec3(0, 10, 0);
//	bodyDef.awake = true;
//	bodyDef.type = e_dynamicBody;
//	b3Body* m_body = m_world->CreateBody(bodyDef);
//
//	b3ShapeDef shapeDef; 
//	b3Hull hull;
//	hull.SetAsBox(b3Vec3(1.0f, 1.0f, 1.0f));
//	b3Polyhedron polyhedron;
//	polyhedron.SetHull(&hull);
//	shapeDef.shape = &polyhedron;
//	b3Shape* shape = m_body->CreateShape(shapeDef);
//
//	//float gs = m_body->GetGravityScale();
//	//m_world->SetGravityDirection(b3Vec3(0.0f, -10.0f, 0.0f));
//	
//	while (1)
//	{
//		m_world->Step(m_step);
//		std::cout << m_body->GetTransform().translation.x << ", "
//			<< m_body->GetTransform().translation.y << ", "
//			<< m_body->GetTransform().translation.z << std::endl;
//	}
//
//	m_body->DestroyShape(shape);
//
//	return 0;
//}