#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <glm/gtx/random.hpp>
#include <glm\common.hpp>

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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "RigidBody.h"
#include "Inertia.h"

#include "RigidbodyManager.h"

#include "Input.h"

#include "Line.h"
#include "Point.h"

#include "Tetrahedron.h"
#include "Triangle.h"

using namespace std;

//Callbacks
void reshape(int w, int h);
void update();
void draw();

void HandleInput();

void DrawModels();
void DrawBoundings();
void DrawParticles();
void DrawDebug();

void DrawGJK();
void DrawEPA();
void DrawContacts();

void InitTweakBar();
void InitTweakBar2();
void TW_CALL ApplyImpulse(void *clientData);
void TW_CALL ResetRB(void *clientData);
void TW_CALL CalculateNewTesor(void *clientData);
void TW_CALL ResetPerformanceCounter(void *clientData);

void SetUpMainTweakBar();

//bool directionKeys[4] = {false};
//bool keyStates[256] = {false}; // Create an array of boolean values of length 256 (0-255)

void printouts();
void printStream();
std::stringstream ss;

void AddADude(glm::vec3 position, bool moving = true);

Camera camera;
glm::mat4 projectionMatrix; // Store the projection matrix
bool freeMouse = false;

glm::mat4 viewMatrix;

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

int oldTimeSinceStart;
double deltaTime;

int fps = 0;
int frameCounterTime = 0;
int frames = 0;

ShaderManager shaderManager;
vector<Model*> modelList;

bool printText = true;

ParticleSystem particleSystem(10000);

std::map<std::string, TwBar*> tweakBars;

Model* impulseVisualiser;

//RigidBody* rigidBody;

bool drawBoundingSpheres = true;
bool drawBoundingBoxes = true;

RigidbodyManager rigidBodyManager;
RigidBody* selectedRigidbody;
int selectedRigidbodyIndex;

long long int QueryPerformance::ts = 0;
long long int QueryPerformance::tf = 0;

std::map<std::string, sint64> QueryPerformance::results;

int broadphaseResultCounter = 0;
sint64 broadphaseResults = 0;

Input input;

bool Input::directionKeys[4] = { false }; 
bool Input::keyStates[256] = { false };
//int Input::mouseWheelDir = 0;

bool Input::mouseMoved = false;
int Input::mouseX = 0;
int Input::mouseY = 0;

unsigned char Input::keyPress = KEY::KEY_F12;
bool Input::wasKeyPressed = false;

bool Input::leftClick = false;

glm::vec3 cursorWorldSpace;

BroadphaseMode broadphaseMode = BroadphaseMode::SAP1D;

int currentLine = 0;

//Debug drawing variables
Point* origin;
Tetrahedron* gjkTetra;
std::vector<Point*> minkowskiDifferencePoints;

bool bRenderImpulseVis = false;

bool bClickImpulse = false;

int main(int argc, char** argv)
{
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition (0, 0); 
    glutCreateWindow("Unconstrained Rigid Body! by Pad");

	//AntTweakBar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	TwDefine(" GLOBAL help='AntTweakBar.' "); // Message added to the help bar.

	//glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);
	
	// REGISTER GLUT CALLBACKS
	glutKeyboardFunc(Input::keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
	glutKeyboardUpFunc(Input::keyUp); // Tell GLUT to use the method "keyUp" for key up events  
	glutSpecialFunc(Input::handleSpecialKeypress);
	glutSpecialUpFunc(Input::handleSpecialKeyReleased);
	glutMouseFunc (Input::mouseButton);
	glutMouseWheelFunc(Input::mouseWheel);
	glutPassiveMotionFunc(Input::passiveMouseMotion);
	//glutMotionFunc (MouseMotion);
	
	glutIdleFunc (update);
	//glutDisplayFunc(draw);
	//glutReshapeFunc (reshape);

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

	glClearColor(155.0/255.0, 155.0/255.0, 155.0/255.0, 1);
	glEnable (GL_CULL_FACE); // cull face 
	glCullFace (GL_BACK); // cull back face 
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );

	projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

	camera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 0.0002f, 0.01f); //TODO - constructor for camera
	camera.mode = CameraMode::flycam;

	shaderManager.Init(); //TODO - constructor for shader
	shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
	shaderManager.CreateShaderProgram("particle", "Shaders/particle.vs", "Shaders/particle.ps");
	shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

	origin = new Point(glm::vec3(0));

	//glm::quat cactuarFix = glm::angleAxis(-90.0f, glm::vec3(0,0,1));
	//cactuarFix *= glm::angleAxis(-90.0f, glm::vec3(0,1,0));
	modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

	impulseVisualiser = new Model(glm::vec3(0,1,0), glm::quat(), glm::vec3(.01), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding")); //TODO change to sphere
	//modelList.push_back(impulseVisualiser);

	RigidBody::impulseVisualiser = impulseVisualiser;
	RigidBody::forcePush = 1.0f;
	RigidBody::angular = true;
	RigidBody::linear = true;
	
	for(int i = 1; i <= 2; i++)
		AddADude(glm::vec3(i * 5.0f, 0, 0), false);

	tweakBars["main"] = TwNewBar("Main");
	TwDefine(" Main size='250 700' color='125 125 125' "); // change default tweak bar size and color
	SetUpMainTweakBar();

	tweakBars["main2"] = TwNewBar("TweakBar2");
	TwDefine(" TweakBar2 size='250 700' color='125 125 125' "); // change default tweak bar size and color

	selectedRigidbody = rigidBodyManager.rigidBodies[0];
	
	TwAddVarRW(tweakBars["main2"], "SelectedRB", TW_TYPE_DIR3F, &selectedRigidbody->model->worldProperties.translation, "");
	TwAddVarRW(tweakBars["main2"], "SelectedRB", TW_TYPE_INT32, &selectedRigidbodyIndex, "");

	TwAddVarRW(tweakBars["main2"], "Angular Velocity", TW_TYPE_DIR3F, &rigidBodyManager[1]->angularVelocity, "");
	TwAddVarRW(tweakBars["main2"], "Angular Momentum", TW_TYPE_DIR3F, &rigidBodyManager[1]->angularMomentum, "");
	
	//TODO? - Recalculate inertial tensor if mass changes


	glm::vec3 v1 = glm::vec3(0,8,0);
	glm::vec3 v2 = glm::vec3(1,0,1);
	glm::vec3 v3 = glm::vec3(0,0,1);
	glm::vec3 v4 = glm::vec3(1,0,0);
	std::vector<glm::vec3> vec;
	vec.push_back(v1);
	vec.push_back(v2);
	vec.push_back(v3);
	vec.push_back(v4);

	gjkTetra = new Tetrahedron(vec);

	glutMainLoop();
    
	return 0;
}

void AddADude(glm::vec3 position, bool moving)
{
	Model* m = new Model(position, glm::quat(), glm::vec3(0.1f), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"), false, true);
	RigidBody* rb = new RigidBody(m);
	
	if(moving)
	{
		rb->velocity = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));
		rb->angularMomentum = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));
	}

	rigidBodyManager.Add(rb);
	modelList.push_back(m);
}

void TW_CALL ApplyImpulse(void *clientData)
{
	rigidBodyManager[0]->ApplyImpulse(impulseVisualiser->worldProperties.translation, camera.viewProperties.forward * RigidBody::forcePush);
}

void TW_CALL ResetRB(void *clientData)
{
	rigidBodyManager[0]->Reset();
}

void TW_CALL ResetPerformanceCounter(void *clientData)
{
	broadphaseResults = 0;
	broadphaseResultCounter = 0;
}

void TW_CALL CalculateNewTesor(void *clientData)
{
	rigidBodyManager[0]->inertialTensor = Inertia::Compute2(rigidBodyManager[0]->model, rigidBodyManager[0]->mass);
}

void TW_CALL AddDudeButton(void *clientData)
{
	//AddADude();
}

void SetUpMainTweakBar()
{
	TwBar* bar = tweakBars["main"];

	TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
	TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

	TwAddVarRW(bar, "Draw Bounding Spheres", TW_TYPE_BOOL8, &drawBoundingSpheres, "");
	TwAddVarRW(bar, "Draw AABBs", TW_TYPE_BOOL8, &drawBoundingBoxes, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Pause simulation", TW_TYPE_BOOL8, &rigidBodyManager.paused, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Impulse Position", TW_TYPE_DIR3F, &impulseVisualiser->worldProperties.translation, "");
	//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, 
				 //" label='Simulation Speed' step=0.1 opened=true help='Change the simulation speed.' ");
	TwAddVarRW(bar, "Impulse Force", TW_TYPE_FLOAT, &RigidBody::forcePush, "");
	TwAddButton(bar, "Do Impulse", ApplyImpulse, NULL, "");
	
	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Angular Velocity", TW_TYPE_DIR3F, &rigidBodyManager[0]->angularVelocity, "");
	TwAddVarRW(bar, "Angular Momentum", TW_TYPE_DIR3F, &rigidBodyManager[0]->angularMomentum, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Mass", TW_TYPE_FLOAT, &rigidBodyManager[0]->mass, "");
	TwAddButton(bar, "Recalculate Tensor", CalculateNewTesor, NULL, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Centre of Mass", TW_TYPE_DIR3F, &rigidBodyManager[0]->com, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddButton(bar, "Reset", ResetRB, NULL, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	{
		TwEnumVal broadphaseModeEV[3] = { {BroadphaseMode::SAP1D, "SAP1D"}, {BroadphaseMode::BruteAABB, "BruteAABB"}, {BroadphaseMode::Sphere, "Sphere"} };
        TwType broadphaseType = TwDefineEnum("IntegratorType", broadphaseModeEV, 3);
		TwAddVarRW(bar, "BroadphaseMode", broadphaseType, &broadphaseMode, " keyIncr='<' keyDecr='>' help='Change broadphase mode.' ");
    }

	TwAddButton(bar, "Reset2", ResetPerformanceCounter, NULL, "");
	TwAddButton(bar, "Add Dude", AddDudeButton, NULL, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Debug GJK", TW_TYPE_BOOL8, &rigidBodyManager.debugGJK, "");
	TwAddVarRW(bar, "Debug EPA", TW_TYPE_BOOL8, &rigidBodyManager.debugEPA, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Collision Response", TW_TYPE_BOOL8, &rigidBodyManager.CR, "");

	TwAddSeparator(bar, "", ""); //=======================================================

	TwAddVarRW(bar, "Click impulse", TW_TYPE_BOOL8, &bClickImpulse, "");
}

// GLUT CALLBACK FUNCTIONS
void update()
{
	//TODO - make a function for this
	selectedRigidbody = rigidBodyManager.rigidBodies[0];

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

	HandleInput();
	
	camera.Update(deltaTime);

	//particleSystem.Update(deltaTime);

	rigidBodyManager.Update(deltaTime);
		
	rigidBodyManager.Broadphase(broadphaseMode);
	rigidBodyManager.Narrowphase(deltaTime);
	
	draw();
}

void HandleInput()
{
	if(!freeMouse)
	{
		if(Input::mouseMoved)
		{
			camera.MouseRotate(Input::mouseX, Input::mouseY, WINDOW_WIDTH, WINDOW_HEIGHT, deltaTime); 
			Input::mouseMoved = false;
		}

		glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
	}

	//if(Input::leftClick)
		//rigidBodyManager[0]->ApplyImpulse(impulseVisualiser->worldProperties.translation, camera.viewProperties.forward * RigidBody::forcePush);

	if(Input::wasKeyPressed)
	{
		camera.ProcessKeyboardOnce(Input::keyPress); 

		if(Input::keyPress == KEY::KEY_h || Input::keyPress == KEY::KEY_H) 
			printText = !printText;

		if(Input::keyPress == KEY::KEY_SPACE || Input::keyPress == KEY::KEY_ESCAPE) 
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

		if(Input::keyPress == KEY::KEY_P || Input::keyPress == KEY::KEY_p)
			rigidBodyManager.paused = !rigidBodyManager.paused;

		if(Input::keyPress == KEY::KEY_J || Input::keyPress == KEY::KEY_j)
			rigidBodyManager.stepDebug = true;

		Input::wasKeyPressed = false;
	}

	////TODO - actually check which object it is hitting
	if(Input::leftClick && bClickImpulse)
		rigidBodyManager[0]->ApplyImpulse(cursorWorldSpace, camera.viewProperties.forward * RigidBody::forcePush);
	
	camera.ProcessKeyboardContinuous(Input::keyStates, deltaTime);
}

//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
//before finally binding the VAO and drawing with verts or indices
void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	TwDraw(); // Draw tweak bars

	viewMatrix = camera.GetViewMatrix();

	glm::mat4 MVP;

	DrawDebug();
	DrawModels();

	cursorWorldSpace = GetOGLPos(Input::mouseX, Input::mouseY, WINDOW_WIDTH, WINDOW_HEIGHT, viewMatrix, projectionMatrix);

	//ImpulseVisualiser
	{
		RigidBody::impulseVisualiser->worldProperties.translation = cursorWorldSpace;

		shaderManager.SetShaderProgram(RigidBody::impulseVisualiser->GetShaderProgramID());
			
		MVP = projectionMatrix * viewMatrix * RigidBody::impulseVisualiser->GetModelMatrix(); //TODO - move these calculations to the graphics card?
		ShaderManager::SetUniform(RigidBody::impulseVisualiser->GetShaderProgramID(), "mvpMatrix", MVP);
			
		if(bRenderImpulseVis)
			RigidBody::impulseVisualiser->Render(shaderManager.GetCurrentShaderProgramID());
	}

	if(rigidBodyManager.bounceyEnclosure)
	{
		shaderManager.SetShaderProgram("bounding");
		MVP = projectionMatrix * viewMatrix;
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		glutWireCube(50);
	}

	DrawBoundings();
	DrawParticles();

	if(printText)
		printouts();

	glutSwapBuffers();
}

void DrawDebug()
{
	glm::mat4 MVP = projectionMatrix * viewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0));
	
	shaderManager.SetShaderProgram("bounding");
	ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
	
	ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,1,1));
	origin->Render(5.0f);
	

	if(rigidBodyManager.debugGJK)
		DrawGJK(); //todo (for each gjk...)

	if(rigidBodyManager.debugEPA)
		DrawEPA();

	if(rigidBodyManager.bVisContacts)
		DrawContacts();
}

void DrawGJK()
{
	ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));
	for(SupportPoint p : rigidBodyManager.gjk->simplex)
	{
		Point p(p.AB);
		p.Render(5.0f);
	}

	int simplexSize = rigidBodyManager.gjk->simplex.size();
	
	if(simplexSize == 2)
	{
		Line l(rigidBodyManager.gjk->simplex[0].AB, rigidBodyManager.gjk->simplex[1].AB);
		l.Render();
	}
	else if(simplexSize == 3)
	{	
		Triangle t(rigidBodyManager.gjk->simplex[0].AB, 
			rigidBodyManager.gjk->simplex[1].AB, rigidBodyManager.gjk->simplex[2].AB);
			
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0.1f,0.9f,0.1f,0.2f));
		t.Render();

		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1.0f,1.0f,1.0f,1.0f));
		t.Render(true);

	}
	else if(simplexSize == 4)
	{
		gjkTetra->Update(rigidBodyManager.gjk->simplex);

		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0.1f,0.9f,0.1f,0.2f));
		gjkTetra->Render(shaderManager.GetCurrentShaderProgramID(), false);
		
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1.0f,1.0f,1.0f,1.0f));
		gjkTetra->Render(shaderManager.GetCurrentShaderProgramID(), true);
	}

	/*ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,0,1));
	for(int i = 0; i < rigidBodyManager.currentMinkowski.size(); i++)
	{
		Point p(rigidBodyManager.currentMinkowski[i]);
		p.Render(7.5f);
	}*/
}

void DrawEPA()
{
	std::vector<TriangleEPA> polytope = rigidBodyManager.epa.polytope;
	for(int i = 0; i < polytope.size(); i++)
	{
		Triangle t(polytope[i].a.AB, polytope[i].b.AB, polytope[i].c.AB);

		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0.3f,0.3f,0.3f,0.3f));
		t.Render();

		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1.0f,1.0f,1.0f,1.0f));
		t.Render(true);

		Point p1(polytope[i].a.AB);
		p1.Render(5.0f);

		Point p2(polytope[i].b.AB);
		p2.Render(5.0f);

		Point p3(polytope[i].c.AB);
		p3.Render(5.0f);

		//ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,0,0.2f));
		//t.Render(false);
	}

	if(polytope.size() > 0)
	{
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		Point nsp(rigidBodyManager.epa.nsp);
		nsp.Render(10.0f);	
	}
}

void DrawContacts()
{
	Point c1(rigidBodyManager.cInfo.c1);
	Point c2(rigidBodyManager.cInfo.c2);

	ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));
	c1.Render(10.0f);
	c2.Render(10.0f);

	Line depth(rigidBodyManager.cInfo.c1, rigidBodyManager.cInfo.c2);
	depth.Render();

	Triangle fA(rigidBodyManager.cInfo.f.a.A,
		rigidBodyManager.cInfo.f.b.A, rigidBodyManager.cInfo.f.c.A);
	fA.Render();
		
	Triangle fB = Triangle(rigidBodyManager.cInfo.f.a.B,
		rigidBodyManager.cInfo.f.b.B, rigidBodyManager.cInfo.f.c.B);
	fB.Render();


	ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0.8f,0.2f,0.5f,1));
	Line l(rigidBodyManager.cInfo.c1, rigidBodyManager.cInfo.c1 + rigidBodyManager.J);
	l.Render();
}

void DrawModels()
{
	for (int i = 0; i < modelList.size(); i++)
	{
		if (modelList[i]->drawMe)
		{
			glm::mat4 MVP = projectionMatrix * viewMatrix * modelList.at(i)->GetModelMatrix(); //TODO - move these calculations to the graphics card?
			
			if(modelList[i]->isColliding)
			{
				shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("bounding"));
				ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
				ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
				modelList.at(i)->Render(shaderManager.GetShaderProgramID("bounding"), true);
			}
			else
			{
				shaderManager.SetShaderProgram(modelList[i]->GetShaderProgramID());
				ShaderManager::SetUniform(modelList[i]->GetShaderProgramID(), "mvpMatrix", MVP);
				modelList.at(i)->Render(shaderManager.GetCurrentShaderProgramID());
			}
				

			shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("bounding"));
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
				//if(modelList.at(i)->isColliding)
					//ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
				//else
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));
			modelList.at(i)->Render(shaderManager.GetShaderProgramID("bounding"), true);
		}
	}	
}

//GLint loc = glGetUniformLocation(bounding, "boundColour"); //check if -1
void DrawBoundings()
{
	GLuint bounding = shaderManager.GetShaderProgramID("bounding");
	shaderManager.SetShaderProgram(bounding);

	for (int i = 0; i < rigidBodyManager.rigidBodies.size(); i++)
	{
		glm::mat4 MVP;

		if(drawBoundingSpheres)
		{
			MVP = projectionMatrix * viewMatrix *  
				glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.translation) 
					* glm::scale(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.scale)
					* rigidBodyManager[i]->model->globalInverseTransform
					* glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->boundingSphere->centre);
			
			ShaderManager::SetUniform(bounding, "mvpMatrix", MVP);
			ShaderManager::SetUniform(bounding, "boundColour", rigidBodyManager[i]->boundingSphere->colour);
			
			rigidBodyManager[i]->boundingSphere->draw();
		}

		if(drawBoundingBoxes)
		{
			MVP = projectionMatrix * viewMatrix * 
			glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.translation) 
				* glm::scale(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.scale) 
				* rigidBodyManager[i]->model->globalInverseTransform 
				* glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->aabb->centre)
				* glm::scale(glm::mat4(), glm::vec3(rigidBodyManager[i]->aabb->width, rigidBodyManager[i]->aabb->height, 
													rigidBodyManager[i]->aabb->depth)); 
		
			ShaderManager::SetUniform(bounding, "mvpMatrix", MVP);
			ShaderManager::SetUniform(bounding, "boundColour", rigidBodyManager[i]->aabb->colour);

			rigidBodyManager[i]->aabb->Draw();
		}
	}
}

void DrawParticles()
{
	//shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("particle"));
	//int viewLocation = glGetUniformLocation(shaderManager.GetCurrentShaderProgramID(), "view"); // TODO - make a function in shadermanager to do these lines
	//glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix)); 
	//int projLocation = glGetUniformLocation(shaderManager.GetCurrentShaderProgramID(), "proj"); 
	//glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	//particleSystem.Render();
}

void printouts()
{
	shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("text"));

	ss << " Press 'spacebar' or 'esc' to toggle camera/cursor";
	printStream();

	ss << " Press 'c' to switch camera modes";
	printStream();

	ss << " sim paused: " << rigidBodyManager.paused;
	printStream();

	printStream();

	ss << " x: " << cursorWorldSpace.x << " y: " << cursorWorldSpace.y << "z: " << cursorWorldSpace.z;
	printStream();

	ss << " fps: " << fps;
	printStream();

	sint64 broadphaseResult = QueryPerformance::results["Broadphase"];
	broadphaseResults += broadphaseResult;
	ss << " Query Performance - Broadphase (Avg.): " << broadphaseResults / ++broadphaseResultCounter;
	printStream();

	ss << "camera.forward: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.forward.x << ", " << camera.viewProperties.forward.y << ", " << camera.viewProperties.forward.z << ")";
	printStream();

	ss << "camera.pos: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.position.x << ", " << camera.viewProperties.position.y << ", " << camera.viewProperties.position.z << ")";
	printStream();

	ss << "camera.up: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.up.x << ", " << camera.viewProperties.up.y << ", " << camera.viewProperties.up.z << ")";
	printStream();

	ss << "gjk->simplex.size(): " << rigidBodyManager.gjk->simplex.size();
	printStream();

	ss << "gjk steps: " << rigidBodyManager.gjk->steps;
	printStream();

	ss << "epa steps: " << rigidBodyManager.epa.steps;
	printStream();

	ss << "J: (" << rigidBodyManager.J.x << ", " << rigidBodyManager.J.y << ", " << rigidBodyManager.J.z << ")";
	printStream();

	currentLine = 0;
}

void printStream()
{
	currentLine += 20;
	drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-currentLine, ss.str().c_str()); //Bottom left is 0,0
	ss.str(std::string()); //reset
}




















