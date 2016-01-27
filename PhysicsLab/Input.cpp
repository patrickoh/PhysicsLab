#include "Input.h"
#include "GLProgram.h"

void Input::mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
		GLProgram::Instance->camera->Zoom(-GLProgram::Instance->deltaTime);
	else
		GLProgram::Instance->camera->Zoom(GLProgram::Instance->deltaTime);
}