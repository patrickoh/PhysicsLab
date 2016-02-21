#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

class BroadphaseDemo : public GLProgram
{

private:

	RigidbodyManager rigidBodyManager;

	BroadphaseMode broadphaseMode;

	int broadphaseResultCounter;
	sint64 broadphaseResults;
	float simulationSpeed;

	int addAmount;

	bool drawBoundingSpheres; 
	bool drawBoundingBoxes;

public:
	
	static BroadphaseDemo* Instance;

	BroadphaseDemo()
	{
		Instance = this;
	}

	~BroadphaseDemo()
	{
	
	}

	void Init(int argc, char** argv)
	{
		GLProgram::Init(argc, argv);

		glutIdleFunc (updateCB);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

		for(int i = 0; i < 5; i++)
			AddBox(glm::vec3(0,0,0));
		addAmount = 1;
		
		simulationSpeed = 1.0f;

		broadphaseMode = BroadphaseMode::SAP1D;
		broadphaseResultCounter = 0;
		broadphaseResults = 0;
		drawBoundingSpheres = false;
		drawBoundingBoxes = true;

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	void AddBox(glm::vec3 position)
	{
		Model* m = new Model(position, glm::quat(), glm::vec3(.1), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"), false, false, true);
		RigidBody* rb = new RigidBody(m);

		glm::vec2 xz = glm::circularRand(glm::linearRand(-1.0f, 1.0f));
		rb->velocity = glm::vec3(xz.x, glm::linearRand(-1.0f, 1.0f), xz.y);
		//rb->velocity = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));
		rb->angularMomentum = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));

		rigidBodyManager.Add(rb);
		modelList.push_back(m);
	}

	static void updateCB()
	{
		Instance->update();
	}

	// GLUT CALLBACK FUNCTIONS
	void update()
	{
		GLProgram::update();

		//if(!pausedSim)
		//{
			rigidBodyManager.Update(deltaTime * simulationSpeed);
			rigidBodyManager.Broadphase(broadphaseMode);
		//}
	
		Draw();
	}

	//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
	//before finally binding the VAO and drawing with verts or indices
	void Draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		viewMatrix = camera->GetViewMatrix();

		glm::mat4 MVP;

		DrawModels();
		DrawBounceyEnclosure(MVP);
		DrawBoundings();

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
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
				glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.translation) //translate in world space
					* glm::scale(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.scale) //scale to size of model
					* rigidBodyManager[i]->model->globalInverseTransform 
					* glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->aabb->centre) //translate to centre in model space
					* glm::scale(glm::mat4(), glm::vec3(rigidBodyManager[i]->aabb->width, rigidBodyManager[i]->aabb->height, 
														rigidBodyManager[i]->aabb->depth));
		
				ShaderManager::SetUniform(bounding, "mvpMatrix", MVP);
				ShaderManager::SetUniform(bounding, "boundColour", rigidBodyManager[i]->aabb->colour);

				rigidBodyManager[i]->aabb->Draw();
			}
		}
	}

	void DrawBounceyEnclosure(glm::mat4 MVP)
	{
		shaderManager.SetShaderProgram("bounding");
		MVP = projectionMatrix * viewMatrix;
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		glutWireCube(rigidBodyManager.bounceyEnclosureSize * 2);
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

		ss << "camera.forward: ";
		toStringStream(camera->viewProperties.forward, ss);
		printStream();

		ss << "camera.position: ";
		toStringStream(camera->viewProperties.position, ss);
		printStream();

		ss << "camera.up: ";
		toStringStream(camera->viewProperties.up, ss);
		printStream();
		printStream();

		sint64 broadphaseResult = QueryPerformance::results["Broadphase"];
		broadphaseResults += broadphaseResult;
		ss << " Query Performance - Broadphase (Avg.): " << broadphaseResults / ++broadphaseResultCounter;
		printStream();

		ss << "Rigid Bodies: " << rigidBodyManager.rigidBodies.size();
		printStream();

		currentLine = 0;
	}

	static void TW_CALL ResetPerformanceCounterCB(void *clientData)
	{
		BroadphaseDemo::Instance->broadphaseResults = 0;
		BroadphaseDemo::Instance->broadphaseResultCounter = 0;
	}

	void AddBoxButton()
	{
		for(int i = 0; i < addAmount; i++)
			AddBox(glm::vec3(0,0,0));
	}

	static void TW_CALL AddBoxButtonCB(void *clientData)
	{
		BroadphaseDemo::Instance->AddBoxButton();
	}

	void SetUpTweakBars()
	{
		SetUpMainTweakBar();
	}

	void SetUpMainTweakBar()
	{
		TwBar* bar = tweakBars["main"];

		//

		TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
		TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

		TwAddVarRW(bar, "Draw Bounding Spheres", TW_TYPE_BOOL8, &drawBoundingSpheres, "");
		TwAddVarRW(bar, "Draw AABBs", TW_TYPE_BOOL8, &drawBoundingBoxes, "");

		TwAddVarRW(bar, "Enclosure size", TW_TYPE_INT32, &rigidBodyManager.bounceyEnclosureSize, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, "");
		TwAddVarRW(bar, "Pause simulation", TW_TYPE_BOOL8, &rigidBodyManager.pausedSim, "");
		TwAddVarRW(bar, "Auto-pause", TW_TYPE_BOOL8, &rigidBodyManager.bpAutoPause, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		{
			TwEnumVal broadphaseModeEV[3] = { {BroadphaseMode::SAP1D, "SAP"}, {BroadphaseMode::BruteAABB, "BruteAABB"}, {BroadphaseMode::Sphere, "Sphere"} };
			TwType broadphaseType = TwDefineEnum("IntegratorType", broadphaseModeEV, 3);
			TwAddVarRW(bar, "BroadphaseMode", broadphaseType, &broadphaseMode, " keyIncr='<' keyDecr='>' help='Change broadphase mode.' ");
		}

		TwAddButton(bar, "Reset", ResetPerformanceCounterCB, NULL, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "Add Amount", TW_TYPE_INT32, &addAmount, "");
		TwAddButton(bar, "Add Box(es)", AddBoxButtonCB, NULL, "");
	}
};