#include "Input.h"
#include "GLProgram.h"

bool Input::directionKeys[4] = { false }; 
bool Input::keyStates[256] = { false };
//int Input::mouseWheelDir = 0;
bool Input::mouseMoved = false;
int Input::mouseX = 0;
int Input::mouseY = 0;
unsigned char Input::keyPress = KEY::KEY_F12;
bool Input::wasKeyPressed = false;
bool Input::leftClick = false;

void Input::mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
		GLProgram::Instance->camera->Zoom(-GLProgram::Instance->deltaTime);
	else
		GLProgram::Instance->camera->Zoom(GLProgram::Instance->deltaTime);
}