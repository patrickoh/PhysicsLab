#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "Inertia.h"

class RigidBodyDemo : public GLProgram
{

private:

	vector<RigidBody*> rigidBodies;
	Model* impulseVis;

public:

	glm::vec3 forcePush;

	static RigidBodyDemo* Instance;

	RigidBodyDemo()
	{
		Instance = this;
	}

	~RigidBodyDemo()
	{
	
	}

	void Init(int argc, char** argv)
	{
		GLProgram::Init(argc, argv);

		glutIdleFunc (updateCB);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

		RigidBody* rb = new RigidBody(new Model(glm::vec3(0,0,0), glm::quat(), glm::vec3(.2), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("white"), false, true));
		//rigidBody = new RigidBody(new Model(glm::vec3(0,0,0), glm::quat(), glm::vec3(.002), "Models/crate.dae", shaderManager.GetShaderProgramID("diffuse")));
		modelList.push_back(rb->model);
		rigidBodies.push_back(rb);

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	static void updateCB()
	{
		Instance->update();
	}

	// GLUT CALLBACK FUNCTIONS
	void update()
	{
		GLProgram::update();

		//rigidBody->Update(); (step physics)

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

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void ApplyImpulse()
	{
		rigidBodies[0]->ApplyImpulse(impulseVis->worldProperties.translation, forcePush);
	}

	static void TW_CALL ApplyImpulseCB(void *clientData)
	{
		RigidBodyDemo::Instance->ApplyImpulse();
	}

	/*void ResetRB()
	{
		rigidBody->Reset();
	}

	static void TW_CALL ResetRBCB(void *clientData)
	{
		RigidBodyDemo::Instance->ResetRB();
	}*/

	void CalculateNewTensors()
	{
		for(RigidBody* rb : rigidBodies)
		{
			rb->inertialTensor = Inertia::Compute2(rb->model, rb->mass);
		}
	}

	static void TW_CALL CalculateNewTensorsCB(void *clientData)
	{
		RigidBodyDemo::Instance->CalculateNewTensors();
	}

	void printouts()
	{
		GLProgram::printouts();
	}

	void SetUpTweakBars()
	{
		SetUpMainTweakBar();
	}

	void SetUpMainTweakBar()
	{
		TwBar* bar = tweakBars["main"];

		//ASSIGNMENT 2 - RIGID BODY UI
	
		TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
		TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

		TwAddSeparator(bar, "", "");

		//TwAddVarRW(bar, "Impulse Position", TW_TYPE_DIR3F, &impulseVis->worldProperties.translation, "");
		////TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, 
		//			 //" label='Simulation Speed' step=0.1 opened=true help='Change the simulation speed.' ");
		//TwAddVarRW(bar, "Impulse Force", TW_TYPE_DIR3F, &forcePush, "");
		//TwAddButton(bar, "Do Impulse", ApplyImpulseCB, NULL, "");
	
		//TwAddSeparator(bar, "", "");

		//TwAddVarRW(bar, "Angular Velocity", TW_TYPE_DIR3F, &rigidBodies[0]->angularVelocity, "");
		//TwAddVarRW(bar, "Angular Momentum", TW_TYPE_DIR3F, &rigidBodies[0]->angularMomentum, "");

		//TwAddSeparator(bar, "", "");

		//TwAddVarRW(bar, "Mass", TW_TYPE_FLOAT, &rigidBodies[0]->mass, "");
		//TwAddButton(bar, "Recalculate Tensor", CalculateNewTensorsCB, NULL, "");

		//TwAddSeparator(bar, "", "");

		//TwAddVarRW(bar, "Centre of Mass", TW_TYPE_DIR3F, &rigidBodies[0]->com, "");

		//TwAddSeparator(bar, "", "");

		//TwAddButton(bar, "Reset", ResetRBCB, NULL, "");
	
		//Recalculate inertial tensor if mass changes
	}
};