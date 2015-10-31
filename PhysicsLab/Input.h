#pragma once

#include "Common.h"

class Input 
{

private:

public:

	static bool directionKeys[4]; 
	static bool keyStates[256];

	static bool wasKeyPressed;
	static unsigned char keyPress;

	//static int mouseWheelDir;

	static bool mouseMoved;
	static int mouseX;
	static int mouseY;

	static bool leftClick;

	static void keyPressed (unsigned char key, int x, int y) 
	{  
		TwEventKeyboardGLUT(key, x, y);

		keyStates[key] = true; // Set the state of the current key to pressed  

		wasKeyPressed = true;
		keyPress = key;
	}  
  
	static void keyUp (unsigned char key, int x, int y) 
	{  
	 	keyStates[key] = false; // Set the state of the current key to not pressed  
	}  

	//DIRECTIONAL KEYS DOWN
	static void handleSpecialKeypress(int key, int x, int y)
	{
		switch (key) 
		{
			case GLUT_KEY_LEFT:
				directionKeys[DKEY::Left] = true;
				break;

			case GLUT_KEY_RIGHT:
				directionKeys[DKEY::Right] = true;	
				break;

			case GLUT_KEY_UP:
				directionKeys[DKEY::Up] = true;
				break;

			case GLUT_KEY_DOWN:
				directionKeys[DKEY::Down] = true;
				break;
		}
	}

	//DIRECTIONAL KEYS UP
	static void handleSpecialKeyReleased(int key, int x, int y) 
	{
		TwEventSpecialGLUT(key, x, y);  // send event to AntTweakBar

		switch (key) 
		{
			case GLUT_KEY_LEFT:
				directionKeys[DKEY::Left] = false;
				break;

			case GLUT_KEY_RIGHT:
				directionKeys[DKEY::Right] = false;	
				break;

			case GLUT_KEY_UP:
				directionKeys[DKEY::Up] = false;
				break;

			case GLUT_KEY_DOWN:
				directionKeys[DKEY::Down] = false;
				break;
		}
	}

	//MOUSE FUCNTIONS
	static void passiveMouseMotion(int x, int y)  
	{
		TwEventMouseMotionGLUT(x, y); // send event to AntTweakBar

		mouseMoved = true;
		mouseX = x;
		mouseY = y;
	}

	static void mouseButton(int button, int state, int x, int y)
	{
		TwEventMouseButtonGLUT(button, state, x, y);  // send event to AntTweakBar

		if(button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				leftClick = true;
			}
			else if (state == GLUT_UP)
			{
				leftClick = false;
			}
		}
	}

	static void mouseWheel(int button, int dir, int x, int y)
	{
		//if (dir > 0)
			//camera.Zoom(-deltaTime);
		//else
			//camera.Zoom(deltaTime);
	}
};