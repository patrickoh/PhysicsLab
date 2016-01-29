#include "ParticleDemo.h"
#include "GLProgram.h"
#include "QueryPerformance.h"

long long int QueryPerformance::ts = 0;
long long int QueryPerformance::tf = 0;
std::map<std::string, sint64> QueryPerformance::results;

GLProgram* GLProgram::Instance;
ParticleDemo* ParticleDemo::Instance;

Camera* Camera::Instance;

int main(int argc, char** argv)
{
	ParticleDemo* demo = new ParticleDemo();

	demo->Init(argc, argv);
	demo->Run();

	delete demo;

	return 0;
}




















