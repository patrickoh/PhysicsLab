#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

class RigidBodyDemo : public GLProgram
{

private:

	glm::vec3 cursorWorldSpace;
	vector<RigidBody*> rigidBodies;

	float mass;

	static glm::vec3 normal[]; 
	static glm::vec3 plane[]; 
	
	bool bClickImpulse;

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

		AddADude(glm::vec3(0,0,0));

		bClickImpulse = true;

		mass = 1;

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	void AddADude(glm::vec3 position)
	{
		Model* m = new Model(position, glm::quat(), glm::vec3(.1), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"), false, false, true);
		RigidBody* rb = new RigidBody(m);
	
		rb->velocity = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));
		rb->angularMomentum = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));

		rigidBodies.push_back(rb);
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

		for (int i = 0; i < rigidBodies.size(); i++)
		{
			for(int j = 0; j < 6; j++)
			{
				if(glm::dot(rigidBodies[i]->model->worldProperties.translation - plane[j], normal[j]) < 0.01f
						&& glm::dot(normal[j], rigidBodies[i]->velocity) < 0.01f)
				{
					rigidBodies[i]->model->worldProperties.translation += -glm::dot(rigidBodies[i]->model->worldProperties.translation - plane[j], normal[j]) * normal[j]; //post processing method
					rigidBodies[i]->velocity += (1 + 1.0f/*coefficient of restitution*/) * -(rigidBodies[i]->velocity * normal[j]) * normal[j];

					//velocity.y = -velocity.y;
				}
			}

			rigidBodies[i]->StepPhysics(deltaTime); //physics update				
			rigidBodies[i]->Update(); //bookkeeping
		}

		if(Input::leftClick)
		{
			//Do Stuff
			
			glm::vec3 p1 = camera->viewProperties.position;
			glm::vec3 p2 = cursorWorldSpace;

			GLbyte color[4];
			GLfloat depth;
			GLuint index;

			float x = Input::leftClickX;
			float y = Input::leftClickY;
  
			glReadPixels(x, WINDOW_HEIGHT - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
			glReadPixels(x, WINDOW_HEIGHT - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
			glReadPixels(x, WINDOW_HEIGHT - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);

			printf("Clicked on pixel %d, %d, color %02hhx%02hhx%02hhx%02hhx, depth %f, stencil index %u\n",
					x, y, color[0], color[1], color[2], color[3], depth, index);

		}
			//rigidBodyManager[0]->ApplyImpulse(impulseVisualiser->worldProperties.translation, camera.viewProperties.forward * RigidBody::forcePush);
		
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

		DrawMouse(MVP);
		DrawBounceyEnclosure(MVP);

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void DrawBounceyEnclosure(glm::mat4 MVP)
	{
		shaderManager.SetShaderProgram("bounding");
		MVP = projectionMatrix * viewMatrix;
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		glutWireCube(10);
	}

	//ImpulseVisualiser
	void DrawMouse(glm::mat4 MVP)
	{
		cursorWorldSpace = GetOGLPos(Input::mouseX, Input::mouseY, WINDOW_WIDTH, WINDOW_HEIGHT, viewMatrix, projectionMatrix);
		shaderManager.SetShaderProgram("bounding");
		MVP = projectionMatrix * viewMatrix * glm::translate(glm::mat4(1.0f), cursorWorldSpace);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));	
		glutSolidSphere(.05, 25, 25);
	}

	void ApplyImpulse()
	{
		//rigidBodies[0]->ApplyImpulse(impulseVis->worldProperties.translation, forcePush);
	}

	static void TW_CALL ApplyImpulseCB(void *clientData)
	{
		RigidBodyDemo::Instance->ApplyImpulse();
	}

	void CalculateNewTensors()
	{
		for(RigidBody* rb : rigidBodies)
		{
			rb->mass = mass;
			rb->inertialTensor = Inertia::Compute2(rb->model, rb->mass);
		}
	}

	static void TW_CALL CalculateNewTensorsCB(void *clientData)
	{
		RigidBodyDemo::Instance->CalculateNewTensors();
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

		toStringStream(camera->viewProperties.forward, ss);
		printStream();

		toStringStream(camera->viewProperties.position, ss);
		printStream();

		toStringStream(camera->viewProperties.position, ss);
		printStream();

		toStringStream(cursorWorldSpace, ss);
		printStream();

		currentLine = 0;
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

		//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed," label='Simulation Speed' step=0.1 opened=true help='Change the simulation speed.' ");
		TwAddVarRW(bar, "Impulse Force", TW_TYPE_DIR3F, &forcePush, "");

		TwAddSeparator(bar, "", "");

		TwAddVarRW(bar, "Mass", TW_TYPE_FLOAT, &mass, "");
		TwAddButton(bar, "Recalculate TensorS", CalculateNewTensorsCB, NULL, "");


		//TO DO: show that your rigid body can update automatically based on a given linear velocity 𝒗 and angular velocity 𝝎
		//TO DO: show that your rigid body updates correctly when you manually change its position 𝒙 and orientation 𝑹


		//TwAddVarRW(bar, "Angular Velocity", TW_TYPE_DIR3F, &rigidBodies[0]->angularVelocity, "");
		//TwAddVarRW(bar, "Angular Momentum", TW_TYPE_DIR3F, &rigidBodies[0]->angularMomentum, "");
		//TwAddSeparator(bar, "", "");

		//TwAddVarRW(bar, "Centre of Mass", TW_TYPE_DIR3F, &rigidBodies[0]->com, "");

		//TwAddSeparator(bar, "", "");

		//TwAddButton(bar, "Reset", ResetRBCB, NULL, "");
	
		//Recalculate inertial tensor if mass changes
	}
};