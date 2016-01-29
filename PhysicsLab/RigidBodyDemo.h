#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "Inertia.h"

class RigidBodyDemo : public GLProgram
{

private:

	RigidBody* rigidBody;
	Model* impulseVis;

public:

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

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

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
		//rigidBody->ApplyImpulse(impulseVis->worldProperties.translation, RigidBody::foRigidBody::forcePush);
	}
	static void TW_CALL ResetPlaneCB(void *clientData)
	{
		RigidBodyDemo::Instance->ApplyImpulse();
	}

	void ResetRB()
	{
		rigidBody->Reset();
	}
	static void TW_CALL ResetRBCB(void *clientData)
	{
		RigidBodyDemo::Instance->ResetRB();
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
	}
};