#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

#include "Tetrahedron.h"
#include "Line.h"
#include "Triangle.h"

class ResponseDemo : public GLProgram
{

private:

	RigidbodyManager rigidBodyManager;

	BroadphaseMode broadphaseMode;

	glm::vec3 cursorWorldSpace;
	float mass;
	float forcePush;
	bool bClickImpulse;
	
	float simulationSpeed;

	int addAmount;

	bool drawBoundingSpheres; 
	bool drawBoundingBoxes;

	//Debug drawing variables
	Point* origin;
	std::vector<Point*> minkowskiDifferencePoints;

	int rbIdx;

	bool bDrawFwd;

public:
	
	static ResponseDemo* Instance;

	ResponseDemo()
	{
		Instance = this;
	}

	~ResponseDemo()
	{
	
	}

	void Init(int argc, char** argv)
	{
		GLProgram::Init(argc, argv);

		glutIdleFunc (updateCB);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), 
			glm::vec3(.0001), "Models/jumbo.dae", 
			shaderManager.GetShaderProgramID("diffuse")));

		for(int i = 0; i < 1; i++)
			AddBox(glm::vec3(0,0,0));

		bClickImpulse = false;

		mass = 1;
		forcePush = 1.0f;

		addAmount = 1;
		
		simulationSpeed = 1.0f;

		broadphaseMode = BroadphaseMode::SAP1D;

		drawBoundingSpheres = false;
		drawBoundingBoxes = true;

		origin = new Point(glm::vec3(0));

		rbIdx = 0;
		rigidBodyManager[rbIdx]->model->colour = glm::vec4(0.7f,0.2f, 0.2f,0.5);

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		tweakBars["selection"] = TwNewBar("Selection");
		TwDefine(" Selection size='250 220' position='1000 450' color='125 125 125' ");

		//rigidBodyManager.bResponse = true;
		bDrawFwd = false;

		SetUpTweakBars();
	}

	void AddBox(glm::vec3 position)
	{
		//TODO
		/*glm::vec3 scale = glm::vec3(
			glm::linearRand(0.01f, 1.0f),
			glm::linearRand(0.01f, 1.0f),
			glm::linearRand(0.01f, 1.0f));*/

		Model* m = new Model(position, glm::quat(), glm::vec3(0.1f), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"), false, false, true);
		RigidBody* rb = new RigidBody(m);

		m->colour = glm::vec4(0.5,0.5,0.5,0.5);

		glm::vec2 xz = glm::circularRand(glm::linearRand(-1.0f, 1.0f));
		rb->momentum = glm::vec3(xz.x, glm::linearRand(-1.0f, 1.0f), xz.y);
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

		if(Input::leftClick && bClickImpulse)
			rigidBodyManager.rigidBodies[rbIdx]->ApplyImpulse(cursorWorldSpace, camera->viewProperties.forward * forcePush);

		rigidBodyManager.Update(deltaTime * simulationSpeed);	
		rigidBodyManager.Broadphase(broadphaseMode); //Make them always be potentially colliding for purposes of demo		
		rigidBodyManager.Narrowphase(deltaTime);
	
		Draw();
	}

	//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
	//before finally binding the VAO and drawing with verts or indices
	void Draw()
	{
		viewMatrix = camera->GetViewMatrix();
		
		glm::mat4 MVP;

		DrawModels();
		DrawBounceyEnclosure(MVP);
		DrawMouse(MVP);
		DrawBoundings();

		//Draw origin
		MVP = camera->Instance->projectionMatrix * viewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0));
		shaderManager.SetShaderProgram("bounding");
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,1,1));
		origin->Render(5.0f);

		if(rigidBodyManager.rigidBodies.size() > 0 && bDrawFwd)
		{
			Line fwd(glm::vec3(0), glm::vec3(0,0,1));
			MVP = camera->Instance->projectionMatrix * viewMatrix * 
				glm::translate(glm::mat4(1), 
				rigidBodyManager[rbIdx]->model->worldProperties.translation);
			shaderManager.SetShaderProgram("bounding");
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));
			fwd.Render();
		}

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
				MVP = camera->Instance->projectionMatrix * viewMatrix *  
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
				MVP = camera->Instance->projectionMatrix * viewMatrix * 
				glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.translation) //translate in world space
					* glm::scale(glm::mat4(1.0f), rigidBodyManager[i]->model->worldProperties.scale * 1.001f) //scale to size of model
					* rigidBodyManager[i]->model->globalInverseTransform 
					* glm::translate(glm::mat4(1.0f), rigidBodyManager[i]->aabb->centre) //translate to centre in model space
					* glm::scale(glm::mat4(), glm::vec3(rigidBodyManager[i]->aabb->width, rigidBodyManager[i]->aabb->height, 
														rigidBodyManager[i]->aabb->depth));
		
				ShaderManager::SetUniform(bounding, "mvpMatrix", MVP);
				ShaderManager::SetUniform(bounding, "boundColour", rigidBodyManager[i]->aabb->colour);

				rigidBodyManager[i]->aabb->Draw();
			}
		}

		shaderManager.SetShaderProgram(0);
	}

	void DrawBounceyEnclosure(glm::mat4 MVP)
	{
		shaderManager.SetShaderProgram("bounding");
		MVP = camera->Instance->projectionMatrix * viewMatrix;
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		glutWireCube(rigidBodyManager.bounceyEnclosureSize * 2);
		shaderManager.SetShaderProgram(0);
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

		/*sint64 broadphaseResult = QueryPerformance::results["Broadphase"];
		broadphaseResults += broadphaseResult;
		ss << " Query Performance - Broadphase (Avg.): " << broadphaseResults / ++broadphaseResultCounter;
		printStream();*/

		ss << "Rigid Bodies: " << rigidBodyManager.rigidBodies.size();
		printStream();

		ss << "bClickImpulse: " << bClickImpulse << " (PRESS K)";
		printStream();

		ss << "rbIdx: " << rbIdx;
		printStream();

		currentLine = 0;
	}

	void HandleInput() override
	{
		GLProgram::HandleInput();

		if(Input::wasKeyPressed)
		{
			if(Input::keyPress == KEY::KEY_TAB)
			{
				rbIdx = (rbIdx + 1) % rigidBodyManager.rigidBodies.size();
				TwRemoveVar(tweakBars["selection"], "selected RB");
				TwAddVarRW(tweakBars["selection"], "selected RB", TW_TYPE_DIR3F, 
					&rigidBodyManager.rigidBodies[rbIdx]->model->worldProperties.translation,
					"");

				rigidBodyManager[(rbIdx-1) % rigidBodyManager.rigidBodies.size()]->model->colour = glm::vec4(0.5,0.5,0.5,0.5);
				rigidBodyManager[rbIdx]->model->colour = glm::vec4(0.7f,0.2f, 0.2f,0.5);
			}

			if(Input::keyPress == KEY::KEY_K ||
				Input::keyPress == KEY::KEY_k)
				bClickImpulse = !bClickImpulse;

			if(Input::keyPress == KEY::KEY_P ||
				Input::keyPress == KEY::KEY_p)
				rigidBodyManager.pausedSim = !rigidBodyManager.pausedSim;
		}

		if(rigidBodyManager.rigidBodies.size() > 0)
		{
			RigidBody* rb = rigidBodyManager.rigidBodies[rbIdx];

			if(Input::keyStates[KEY::KEY_q] || Input::keyStates[KEY::KEY_Q])
				rb->ApplyImpulse(rb->model->worldProperties.translation, glm::vec3(0,-1,0));
			if(Input::keyStates[KEY::KEY_e] || Input::keyStates[KEY::KEY_E])
				rb->ApplyImpulse(rb->model->worldProperties.translation, glm::vec3(0,1,0));

			if(Input::keyStates[KEY::KEY_w] || Input::keyStates[KEY::KEY_W])
				rb->ApplyImpulse(rb->model->worldProperties.translation, glm::vec3(0,0,1));
			if(Input::keyStates[KEY::KEY_s] || Input::keyStates[KEY::KEY_S])
				rb->ApplyImpulse(rb->model->worldProperties.translation, glm::vec3(0,0,-1));

			if(Input::keyStates[KEY::KEY_a] || Input::keyStates[KEY::KEY_A])
				rb->ApplyImpulse(rb->model->worldProperties.translation, glm::vec3(1,0,0));
			if(Input::keyStates[KEY::KEY_d] || Input::keyStates[KEY::KEY_D])
				rb->ApplyImpulse(rb->model->worldProperties.translation, glm::vec3(-1,0,0));
		}
	}

	//ImpulseVisualiser
	void DrawMouse(glm::mat4 MVP)
	{
		cursorWorldSpace = GetOGLPos(Input::mouseX, Input::mouseY, WINDOW_WIDTH, WINDOW_HEIGHT, viewMatrix, camera->Instance->projectionMatrix);
		shaderManager.SetShaderProgram("bounding");
		MVP = camera->Instance->projectionMatrix * viewMatrix * glm::translate(glm::mat4(1.0f), cursorWorldSpace);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));	
		glutSolidSphere(.05, 25, 25);
		shaderManager.SetShaderProgram(0);
	}

	void CalculateNewTensor()
	{
		RigidBody* rb = rigidBodyManager[rbIdx];
		rb->mass = mass;
		rb->inertialTensor = Inertia::Compute2(rb->model, rb->mass);
	}

	static void TW_CALL CalculateNewTensorCB(void *clientData)
	{
		ResponseDemo::Instance->CalculateNewTensor();
	}

	void ResetRB()
	{
		rigidBodyManager[rbIdx]->Reset();
	}

	static void TW_CALL ResetRBCB(void *clientData)
	{
		ResponseDemo::Instance->ResetRB();
	}

	void AddBoxButton()
	{
		for(int i = 0; i < addAmount; i++)
			AddBox(glm::vec3(0,0,0));
	}

	static void TW_CALL AddBoxButtonCB(void *clientData)
	{
		ResponseDemo::Instance->AddBoxButton();
	}

	void SetUpTweakBars()
	{
		TwBar* bar = tweakBars["main"];

		TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
		TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "Gravity", TW_TYPE_BOOL8, &RigidBody::gravity, "");
		TwAddVarRW(bar, "Wind", TW_TYPE_BOOL8, &RigidBody::wind, "");

		TwAddVarRW(bar, "Drag", TW_TYPE_BOOL8, &RigidBody::drag, "");
		TwAddVarRW(bar, "Fluid Density", TW_TYPE_FLOAT, &RigidBody::env.fluidDensity, " label='Fluid Density' group='Drag Settings'");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "Draw Bounding Spheres", TW_TYPE_BOOL8, &drawBoundingSpheres, "");
		TwAddVarRW(bar, "Draw AABBs", TW_TYPE_BOOL8, &drawBoundingBoxes, "");

		TwAddVarRW(bar, "Enclosure size", TW_TYPE_INT32, &rigidBodyManager.bounceyEnclosureSize, "");
		TwAddVarRW(bar, "bEnclosure", TW_TYPE_BOOL8, &rigidBodyManager.bounceyEnclosure, "");
		

		TwAddSeparator(bar, "", ""); //=======================================================

		//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, "");
		TwAddVarRW(bar, "(P)ause simulation", TW_TYPE_BOOL8, &rigidBodyManager.pausedSim, "");
		TwAddVarRW(bar, "Auto-pause (Broad)", TW_TYPE_BOOL8, &rigidBodyManager.bpAutoPause, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		{
			TwEnumVal broadphaseModeEV[4] = { {BroadphaseMode::SAP1D, "SAP"}, {BroadphaseMode::BruteAABB, "BruteAABB"}, {BroadphaseMode::Sphere, "Sphere"},
				{BroadphaseMode::Skip, "Skip"} };
			TwType broadphaseType = TwDefineEnum("IntegratorType", broadphaseModeEV, 4);
			TwAddVarRW(bar, "BroadphaseMode", broadphaseType, &broadphaseMode, " keyIncr='<' keyDecr='>' help='Change broadphase mode.' ");
		}

		//TwAddButton(bar, "Reset", ResetPerformanceCounterCB, NULL, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "bDrawGJK", TW_TYPE_BOOL8, &rigidBodyManager.bDrawGJK, "");
		TwAddVarRW(bar, "bDrawEPA", TW_TYPE_BOOL8, &rigidBodyManager.bDrawEPA, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "bResponse", TW_TYPE_BOOL8, &rigidBodyManager.bResponse, "");
		TwAddVarRW(bar, "bDrawFwd", TW_TYPE_BOOL8, &bDrawFwd, "");

		TwAddSeparator(bar, "", ""); //=======================================================
		
		TwAddVarRW(bar, "Impulse Force", TW_TYPE_FLOAT, &forcePush, "");

		TwAddSeparator(bar, "", "");

		TwAddVarRW(bar, "Add Amount", TW_TYPE_INT32, &addAmount, "");
		TwAddButton(bar, "Add Box(es)", AddBoxButtonCB, NULL, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		bar = tweakBars["selection"];
		TwAddVarRW(bar, "selected RB", TW_TYPE_DIR3F, &rigidBodyManager.rigidBodies[rbIdx]->model->worldProperties.translation, "");
		TwAddButton(bar, "Reset", ResetRBCB, NULL, "");
		TwAddVarRW(bar, "New Mass", TW_TYPE_FLOAT, &mass, "");
		TwAddButton(bar, "Recalculate Tensor", CalculateNewTensorCB, NULL, "");


	}
};