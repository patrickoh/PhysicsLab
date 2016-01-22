#pragma once
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

#include "imgui.h"
#include "ImGuiImpl.h"

class ParticleDemo
{

private:

	std::stringstream ss;

	Camera camera;

	glm::mat4 projectionMatrix; // Store the projection matrix
	glm::mat4 viewMatrix;
	
	bool freeMouse;

	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;

	int oldTimeSinceStart;
	double deltaTime;

	int fps;
	int frameCounterTime;
	int frames;

	ShaderManager shaderManager;
	vector<Model*> modelList;

	bool printText; 

	ParticleSystem* particleSystem;
	Model* plane;
	std::map<std::string, TwBar*> tweakBars;
	Input input;

	int currentLine;

public:

	static ParticleDemo* Instance;

	ParticleDemo()
		: currentLine(0), oldTimeSinceStart(0), frames(0), frameCounterTime(0), fps(0),
		WINDOW_WIDTH(1280), WINDOW_HEIGHT(720), freeMouse(false), printText(true)
	{
		Instance = this;
	}

	~ParticleDemo()
	{
		delete particleSystem;
		delete plane;
	}

	void Init(int argc, char** argv)
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
	
		glutIdleFunc (updateCB);
		//glutDisplayFunc(draw);
		//glutReshapeFunc (reshape);

		TwGLUTModifiersFunc(glutGetModifiers);
		glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);

		// A call to glewInit() must be done after glut is initialized!
		glewExperimental = GL_TRUE;
		GLenum res = glewInit();

		glClearColor(155.0/255.0, 155.0/255.0, 155.0/255.0, 1);
		glEnable (GL_CULL_FACE); // cull face 
		glCullFace (GL_BACK); // cull back face 
		glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
		glEnable(GL_DEPTH_TEST);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable( GL_BLEND );

		projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

		camera = Camera();
		camera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 0.0002f, 0.01f); //TODO - constructor for camera
		camera.mode = CameraMode::flycam;

		shaderManager.Init(); //TODO - constructor for shader
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("particle", "Shaders/particle.vs", "Shaders/particle.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

		particleSystem = new ParticleSystem(100000);
		
		plane = new Model(glm::vec3(0,0,0), glm::quat(), glm::vec3(2), "Models/plane.dae", shaderManager.GetShaderProgramID("bounding"));
		plane->wireframe = true;
		modelList.push_back(plane);
		
		particleSystem->Generate();

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 700' color='125 125 125' "); // change default tweak bar size and color
		SetUpTweakBar();
	}

	void Run()
	{
		glutMainLoop();
	}

	static void updateCB()
	{
		Instance->update();
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

		HandleInput();
	
		camera.Update(deltaTime);

		//Update plane
		double cosAngle = glm::dot(glm::vec3(0,1,0), particleSystem->normal);
		if(cosAngle < 0.9999f)
		{
			float turnAngle = glm::degrees(glm::acos(cosAngle));
			glm::vec3 rotAxis = glm::normalize(glm::cross(glm::vec3(0,1,0), particleSystem->normal));
			plane->worldProperties.orientation = glm::toQuat(glm::rotate(glm::mat4(1), turnAngle, rotAxis));
		}

		particleSystem->Update(deltaTime);
	
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

			Input::wasKeyPressed = false;
		}
	
		camera.ProcessKeyboardContinuous(Input::keyStates, deltaTime);
	}

	//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
	//before finally binding the VAO and drawing with verts or indices
	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		viewMatrix = camera.GetViewMatrix();

		glm::mat4 MVP;

		DrawModels();
		DrawParticles();

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void DrawModels()
	{
		for (int i = 0; i < modelList.size(); i++)
		{
			if (modelList[i]->drawMe)
			{
				glm::mat4 MVP = projectionMatrix * viewMatrix * modelList.at(i)->GetModelMatrix(); //TODO - move these calculations to the graphics card?
			
				shaderManager.SetShaderProgram(modelList[i]->GetShaderProgramID());
				ShaderManager::SetUniform(modelList[i]->GetShaderProgramID(), "mvpMatrix", MVP);
				modelList.at(i)->Render(shaderManager.GetCurrentShaderProgramID());
	
				shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("bounding"));
				ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
				ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));
				modelList.at(i)->Render(shaderManager.GetShaderProgramID("bounding"), true);
			}
		}	
	}

	void DrawParticles()
	{
		GLuint particleShader = shaderManager.GetShaderProgramID("particle");
		shaderManager.SetShaderProgram(particleShader);

		ShaderManager::SetUniform(particleShader, "view", viewMatrix);
		ShaderManager::SetUniform(particleShader, "proj", projectionMatrix);
	
		particleSystem->Render();
	}

	void printouts()
	{
		shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("text"));

		ss << " Press 'spacebar' or 'esc' to toggle camera/cursor";
		printStream();

		ss << " Press 'c' to switch camera modes";
		printStream();

		ss << " fps: " << fps;
		printStream();

		ss << "camera.forward: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.forward.x << ", " << camera.viewProperties.forward.y << ", " << camera.viewProperties.forward.z << ")";
		printStream();

		ss << "camera.pos: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.position.x << ", " << camera.viewProperties.position.y << ", " << camera.viewProperties.position.z << ")";
		printStream();

		ss << "camera.up: (" << std::fixed << std::setprecision(PRECISION) << camera.viewProperties.up.x << ", " << camera.viewProperties.up.y << ", " << camera.viewProperties.up.z << ")";
		printStream();

		currentLine = 0;
	}

	void printStream()
	{
		currentLine += 20;
		drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-currentLine, ss.str().c_str()); //Bottom left is 0,0
		ss.str(std::string()); //reset
	}

	void ResetPlane()
	{
		particleSystem->normal = glm::vec3(0,1,0);
		plane->worldProperties.orientation = glm::quat();
	}

	static void TW_CALL ResetPlaneCB(void *clientData)
	{
		ParticleDemo::Instance->ResetPlane();
	}

	void SetUpTweakBar()
	{
		TwBar* bar = tweakBars["main"];

		TwAddVarRW(bar, "EmitRate", TW_TYPE_INT32, &particleSystem->emitter.emitRate, "");
		TwAddVarRW(bar, "Particle life", TW_TYPE_FLOAT, &particleSystem->particleLife, "min=0.0 step=0.25");
		TwAddVarRW(bar, "StartColour", TW_TYPE_COLOR3F, &particleSystem->startColour, " group='Colours' ");
		TwAddVarRW(bar, "EndColour", TW_TYPE_COLOR3F, &particleSystem->endColour, " group='Colours' ");

		TwAddVarRW(bar, "VelRangeMin", TW_TYPE_DIR3F, &particleSystem->emitter.velRangeMin, " group='Initial Velocity' ");
		TwAddVarRW(bar, "VelRangeMax", TW_TYPE_DIR3F, &particleSystem->emitter.velRangeMax, " group='Initial Velocity' ");

		TwAddSeparator(bar, "", "");
		TwAddVarRW(bar, "Gravity", TW_TYPE_BOOL8, &particleSystem->gravity, " label='Gravity'");
		TwAddVarRW(bar, "GravityStr", TW_TYPE_FLOAT, &particleSystem->env.gravity, " label='GravityStr'");

		TwAddSeparator(bar, "", "");
		TwAddVarRW(bar, "Drag", TW_TYPE_BOOL8, &particleSystem->drag, " label='Drag'");
		TwAddVarRW(bar, "Cd ", TW_TYPE_FLOAT, &particleSystem->dragCoefficient, " label='Cd' group='Drag Settings'");
		TwAddVarRW(bar, "Fluid Density", TW_TYPE_FLOAT, &particleSystem->env.fluid.density, " label='Fluid Density' group='Drag Settings'");
		TwAddVarRW(bar, "Wind ", TW_TYPE_BOOL8, &particleSystem->wind, " label='Wind' group='Drag Settings'");
		TwAddVarRW(bar, "WindDir", TW_TYPE_DIR3F, &particleSystem->env.wind, 
					" label='Wind direction' opened=false help='Change the wind direction.' group='Drag Settings' ");
		TwAddVarRW(bar, "WindScalar ", TW_TYPE_FLOAT, &particleSystem->env.windScalar, " label='WindScalar' group='Drag Settings'");

		TwAddSeparator(bar, "", "");
		{
			TwEnumVal integratorEV[4] = { {IntegratorMode::Euler, "Euler"}, {IntegratorMode::RK2, "RK2"}, {IntegratorMode::RK4, "RK4"}, {IntegratorMode::None, "None"} };
			TwType integratorType = TwDefineEnum("IntegratorType", integratorEV, 4);
			TwAddVarRW(bar, "Integrator", integratorType, &particleSystem->mode, " keyIncr='<' keyDecr='>' help='Change integrator mode.' ");
		}
		TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &particleSystem->simulationSpeed, 
				" label='Simulation Speed' step=0.1 opened=true help='Change the simulation speed.' ");

		TwAddSeparator(bar, "", "");
		TwAddVarRW(bar, "Collision Response", TW_TYPE_BOOL8, &particleSystem->bCollisions, "");
		TwAddVarRW(bar, "Normal", TW_TYPE_DIR3F, &particleSystem->normal, 
					" label='Plane Normal' opened=false help='Change the plane normal.' group='Plane Settings'");
		TwAddVarRW(bar, "Kr", TW_TYPE_FLOAT, &particleSystem->coefficientOfRestitution, "help='Coefficient of Restitution.' min=0.0 max=1.0 step=0.1 group='Plane Settings'");
		TwAddButton(bar, "Reset Plane", ResetPlaneCB, NULL, "group='Plane Settings'");

		TwAddSeparator(bar, "", "");
		TwAddVarRO(bar, "Live Particles", TW_TYPE_INT32, &particleSystem->liveParticles, " label='ParticleCount'");
		TwAddVarRW(bar, "Particle mass", TW_TYPE_FLOAT, &particleSystem->mass, "min=0.1");
	}
};