#include <GL/glew.h>
#include <GL/freeglut.h>

#include <assimp/Importer.hpp>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ShaderManager.h"

#include "Common.h"
#include "Keys.h"

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <map>
#include <vector>
#include <list>

#include "ParticleSystem.h"

#include <AntTweakBar.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "RigidBody.h"
#include "Inertia.h"

#include "RigidbodyManager.h"

using namespace std;

//Callbacks
void keyPressed (unsigned char key, int x, int y); 
void keyUp (unsigned char key, int x, int y); 
void passiveMouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);
void handleSpecialKeypress(int key, int x, int y);
void handleSpecialKeyReleased(int key, int x, int y);
void mouseWheel(int, int, int, int);

void reshape(int w, int h);
void update();
void draw();

void InitTweakBar();
void TW_CALL ApplyImpulse(void *clientData);
void TW_CALL ResetRB(void *clientData);
void TW_CALL CalculateNewTesor(void *clientData);

bool directionKeys[4] = {false};

bool keyStates[256] = {false}; // Create an array of boolean values of length 256 (0-255)

void processContinuousInput();
void printouts();

Camera camera;
glm::mat4 projectionMatrix; // Store the projection matrix
bool freeMouse = false;

//int WINDOW_WIDTH = 1920;
//int WINDOW_HEIGHT = 1080;

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

//int WINDOW_WIDTH = 1680;
//int WINDOW_HEIGHT = 1050;

int oldTimeSinceStart;
double deltaTime;

int fps = 0;
int frameCounterTime = 0;
int frames = 0;

ShaderManager shaderManager;
vector<Model*> modelList;

bool printText = true;

ParticleSystem particleSystem(10000);
TwBar *bar;

Model* impulseVisualiser;

//RigidBody* rigidBody;


RigidbodyManager rigidBodyManager;

int main(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	glutInitWindowPosition (0, 0); 

    glutCreateWindow("Unconstrained Rigid Body! by Pad");
	InitTweakBar();

	//glutFullScreen();

	glutSetCursor(GLUT_CURSOR_NONE);
	
	// REGISTER GLUT CALLBACKS

	//glutDisplayFunc(draw);
	//glutReshapeFunc (reshape);
	glutKeyboardFunc(keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutKeyboardUpFunc(keyUp); // Tell GLUT to use the method "keyUp" for key up events  
	glutSpecialFunc(handleSpecialKeypress);
	glutSpecialUpFunc(handleSpecialKeyReleased);
	glutMouseFunc (mouseButton);
	glutMouseWheelFunc(mouseWheel);
	//glutMotionFunc (MouseMotion);
	glutPassiveMotionFunc(passiveMouseMotion);
	glutIdleFunc (update);

	TwGLUTModifiersFunc(glutGetModifiers);

	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);

	// A call to glewInit() must be done after glut is initialized!
	glewExperimental = GL_TRUE;
    GLenum res = glewInit();
	
	#pragma region ERROR CHECKING
	// Check for any errors
    if (res != GLEW_OK) 
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
    }
	#pragma endregion 

	glClearColor(5.0/255.0, 5.0/255.0, 5.0/255.0, 1);
	glEnable (GL_CULL_FACE); // cull face 
	glCullFace (GL_BACK); // cull back face 
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_DEPTH_TEST);

	projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

	camera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 0.0002f, 0.01f); //TODO - constructor for camera
	camera.mode = CameraMode::tp;

	shaderManager.Init(); //TODO - constructor for shader
	shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
	shaderManager.CreateShaderProgram("black", "Shaders/diffuse.vs", "Shaders/black.ps");
	shaderManager.CreateShaderProgram("white", "Shaders/diffuse.vs", "Shaders/white.ps");
	shaderManager.CreateShaderProgram("red", "Shaders/diffuse.vs", "Shaders/red.ps");
	shaderManager.CreateShaderProgram("text", "Shaders/diffuse.vs", "Shaders/white.ps");
	shaderManager.CreateShaderProgram("particle", "Shaders/particle.vs", "Shaders/particle.ps");

	//plane->wireframe = true;

	glm::quat cactuarFix = glm::angleAxis(-90.0f, glm::vec3(0,0,1));
	cactuarFix *= glm::angleAxis(-90.0f, glm::vec3(0,1,0));
	modelList.push_back(new Model(glm::vec3(0, 0, 5), cactuarFix, glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

	impulseVisualiser = new Model(glm::vec3(0,1,0), glm::quat(), glm::vec3(.01), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("red"));
	modelList.push_back(impulseVisualiser);

	RigidBody::impulseVisualiser = new Model(glm::vec3(0,1,0), glm::quat(), glm::vec3(.01), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("red"));
	RigidBody::force = glm::vec3(0,0,1);
	RigidBody::angular = true;
	RigidBody::linear = true;


	Model* m = new Model(glm::vec3(0, 0, 0), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("white"), false, true);
	rigidBodyManager.Add(new RigidBody(m));
	modelList.push_back(m);

	//ASSIGNMENT 2 - RIGID BODY UI
	
	TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
	TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

	TwAddSeparator(bar, "", "");

	TwAddVarRW(bar, "Impulse Position", TW_TYPE_DIR3F, &impulseVisualiser->worldProperties.translation, "");
	//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, 
				 //" label='Simulation Speed' step=0.1 opened=true help='Change the simulation speed.' ");
	TwAddVarRW(bar, "Impulse Force", TW_TYPE_DIR3F, &RigidBody::force, "");
	TwAddButton(bar, "Do Impulse", ApplyImpulse, NULL, "");
	
	TwAddSeparator(bar, "", "");

	TwAddVarRW(bar, "Angular Velocity", TW_TYPE_DIR3F, &rigidBodyManager[0]->angularVelocity, "");
	TwAddVarRW(bar, "Angular Momentum", TW_TYPE_DIR3F, &rigidBodyManager[0]->angularMomentum, "");

	TwAddSeparator(bar, "", "");

	TwAddVarRW(bar, "Mass", TW_TYPE_FLOAT, &rigidBodyManager[0]->mass, "");
	TwAddButton(bar, "Recalculate Tensor", CalculateNewTesor, NULL, "");

	TwAddSeparator(bar, "", "");

	TwAddVarRW(bar, "Centre of Mass", TW_TYPE_DIR3F, &rigidBodyManager[0]->com, "");

	TwAddSeparator(bar, "", "");

	TwAddButton(bar, "Reset", ResetRB, NULL, "");
	
	//Recalculate inertial tensor if mass changes

	//particleSystem.Generate();

	glutMainLoop();
    
	return 0;
}

void InitTweakBar()
{
    TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	bar = TwNewBar("TweakBar");
    
	TwDefine(" GLOBAL help='AntTweakBar.' "); // Message added to the help bar.
    TwDefine(" TweakBar size='250 700' color='125 125 125' "); // change default tweak bar size and color
}

void TW_CALL ApplyImpulse(void *clientData)
{
	rigidBodyManager[0]->ApplyImpulse(impulseVisualiser->worldProperties.translation, RigidBody::force);
}

void TW_CALL ResetRB(void *clientData)
{
	rigidBodyManager[0]->Reset();
}

void TW_CALL CalculateNewTesor(void *clientData)
{
	rigidBodyManager[0]->inertialTensor = Inertia::Compute2(rigidBodyManager[0]->model, rigidBodyManager[0]->mass);
}

// GLUT CALLBACK FUNCTIONS
void update()
{
	//Calculate deltaTime
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = timeSinceStart - oldTimeSinceStart;
    oldTimeSinceStart = timeSinceStart;

	//Calculate fps
	frames++;
	frameCounterTime += deltaTime;
	if(frameCounterTime > 1000)
	{
		fps = frames;
		frames = frameCounterTime = 0;
	}

	camera.Update(deltaTime);

	//particleSystem.Update(deltaTime);

	for (int i = 0; i < rigidBodyManager.rigidBodies.size(); i++)
		rigidBodyManager[i]->Update(deltaTime);

	//rigidBodyManager->Broadphase(BroadMode::BRUTE);

	//PHYSICS

	
	processContinuousInput();
	draw();
}

//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
//before finally binding the VAO and drawing with verts or indices
void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glm::mat4 viewMatrix = camera.GetViewMatrix();

	for (int i = 0; i < modelList.size(); i++)
	{
		if (modelList[i]->drawMe)
		{
			//Set shader
			shaderManager.SetShaderProgram(modelList[i]->GetShaderProgramID());

			//Set MVP matrix
			glm::mat4 MVP = projectionMatrix * viewMatrix * modelList.at(i)->GetModelMatrix(); //TODO - move these calculations to the graphics card?
			int mvpMatrixLocation = glGetUniformLocation(modelList[i]->GetShaderProgramID(), "mvpMatrix"); // Get the location of mvp matrix in the shader
			glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVP)); // Send updated mvp matrix 

			//Render
			modelList.at(i)->Render(shaderManager.GetCurrentShaderProgramID());
		}
	}	

	GLuint bounding = shaderManager.GetShaderProgramID("red");

	shaderManager.SetShaderProgram(bounding);
	glPolygonMode(GL_FRONT, GL_LINE);

	for (int i = 0; i < rigidBodyManager.rigidBodies.size(); i++)
	{
		glm::mat4 MVP = projectionMatrix * viewMatrix * rigidBodyManager[i]->model->GetModelMatrix() * glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->boundingSphere->centre);
		int mvpMatrixLocation = glGetUniformLocation(bounding, "mvpMatrix"); // Get the location of mvp matrix in the shader
		glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(MVP)); // Send updated mvp matrix 

		rigidBodyManager[i]->boundingSphere->draw();
	}

	//shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("particle"));
	//int viewLocation = glGetUniformLocation(shaderManager.GetCurrentShaderProgramID(), "view"); // TODO - make a function in shadermanager to do these lines
	//glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix)); 
	//int projLocation = glGetUniformLocation(shaderManager.GetCurrentShaderProgramID(), "proj"); 
	//glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	//particleSystem.Render();

	// Draw tweak bars
    TwDraw();

	shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("text"));

	glPolygonMode(GL_FRONT, GL_FILL);

	if(printText)
		printouts();

	glutSwapBuffers();
}

//KEYBOARD FUCNTIONS
void keyPressed (unsigned char key, int x, int y) 
{  
	TwEventKeyboardGLUT(key, x, y);

	keyStates[key] = true; // Set the state of the current key to pressed  
		
	camera.ProcessKeyboardOnce(key, x, y);

	if(key == KEY::KEY_h || key == KEY::KEY_H)
		printText = !printText;

	if(key == KEY::KEY_SPACE || key == KEY::KEY_ESCAPE)
	{
		if(!freeMouse)
		{
			freeMouse = true;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
		else
		{
			freeMouse = false;
			glutSetCursor(GLUT_CURSOR_NONE);
		}
	}
}  
  
void keyUp (unsigned char key, int x, int y) 
{  
	keyStates[key] = false; // Set the state of the current key to not pressed  
}  

//Process keystates
void processContinuousInput()
{
	//exit(0);

	camera.ProcessKeyboardContinuous(keyStates, deltaTime);
}

//DIRECTIONAL KEYS DOWN
void handleSpecialKeypress(int key, int x, int y)
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

	//if(editMode == levelEdit)
		//levelEditor->ProcessKeyboardOnce(key, x, y);
}

//DIRECTIONAL KEYS UP
void handleSpecialKeyReleased(int key, int x, int y) 
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
void passiveMouseMotion(int x, int y)  
{
	TwEventMouseMotionGLUT(x, y); // send event to AntTweakBar

	if(!freeMouse)
	{
		//As glutWarpPoint triggers an event callback to Mouse() we need to return to ensure it doesn't recursively call
		static bool just_warped = false;
		if(just_warped) {
			just_warped = false;
			return;
		}

		camera.MouseRotate(x, y, WINDOW_WIDTH, WINDOW_HEIGHT, deltaTime); 

		glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
		just_warped = true;
	}
}

void mouseButton(int button, int state, int x, int y)
{
	TwEventMouseButtonGLUT(button, state, x, y);  // send event to AntTweakBar

	switch(button) {
		case GLUT_LEFT_BUTTON:
			break;
    }

	//if(!freeMouse)
		//player->ProcessMouseButton(button, state, x, y);
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
		camera.Zoom(-deltaTime);
    else
		camera.Zoom(deltaTime);
}

// OTHER FUNCTIONS

void printouts()
{
	std::stringstream ss;

	//Bottom left is 0,0

	ss.str(std::string()); // clear
	ss << " Press 'spacebar' or 'esc' to toggle camera/cursor";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-40, ss.str().c_str());

	ss.str(std::string()); // clear
	ss << " Press 'c' to switch camera modes";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-60, ss.str().c_str());

	/*ss.str(std::string()); // clear
	ss << fps << " fps ";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-40, ss.str().c_str());
	*/

	ss.str(std::string()); // clear
	glm::vec3 pos = rigidBodyManager[0]->model->worldProperties.translation;
	ss << "rb.pos: (" << std::fixed << std::setprecision(PRECISION) << pos.x << ", " << pos.y << ", " << pos.z << ")";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),140, ss.str().c_str());
	ss.str(std::string()); // clear
	glm::vec3 rot = rigidBodyManager[0]->model->GetEulerAngles();
	ss << "rb.rot: (" << std::fixed << std::setprecision(PRECISION) << rot.x << ", " << rot.y << ", " << rot.z << ")";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),120, ss.str().c_str());

	//PRINT CAMERA
	ss.str(std::string()); // clear
	ss << "camera.forward: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.forward.x << ", " << camera.viewProperties.forward.y << ", " << camera.viewProperties.forward.z << ")";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),80, ss.str().c_str());

	ss.str(std::string()); // clear
	ss << "camera.pos: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.position.x << ", " << camera.viewProperties.position.y << ", " << camera.viewProperties.position.z << ")";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),60, ss.str().c_str());

	ss.str(std::string()); // clear
	ss << "camera.up: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.up.x << ", " << camera.viewProperties.up.y << ", " << camera.viewProperties.up.z << ")";
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),40, ss.str().c_str());
}


















