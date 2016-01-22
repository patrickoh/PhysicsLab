#include "ParticleDemo.h"

long long int QueryPerformance::ts = 0;
long long int QueryPerformance::tf = 0;
std::map<std::string, sint64> QueryPerformance::results;

bool Input::directionKeys[4] = { false }; 
bool Input::keyStates[256] = { false };
//int Input::mouseWheelDir = 0;
bool Input::mouseMoved = false;
int Input::mouseX = 0;
int Input::mouseY = 0;
unsigned char Input::keyPress = KEY::KEY_F12;
bool Input::wasKeyPressed = false;
bool Input::leftClick = false;

ParticleDemo* ParticleDemo::Instance;

int main(int argc, char** argv)
{
	ParticleDemo* demo = new ParticleDemo();

	demo->Init(argc, argv);
	demo->Run();

	delete demo;

	return 0;
}




















