#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

#include "..\Bounce\Bounce.h"
#include "DebugDrawer.h"

class SolverDemo : public GLProgram
{

private:

	float simulationSpeed;
	std::vector<b3Body*> rigidBodies;

public:

	b3World* m_world;
	b3TimeStep m_step;

	static SolverDemo* Instance;

	DebugDrawer debugDrawer;

	SolverDemo()
	{
		Instance = this;
	}

	~SolverDemo()
	{		
		ClearBoxes();
		delete m_world;
	}

	void Init(int argc, char** argv)
	{
		GLProgram::Init(argc, argv);

		glutIdleFunc (updateCB);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", 
			"Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", 
			"Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), 
			glm::vec3(.0001), "Models/jumbo.dae",
			shaderManager.GetShaderProgramID("diffuse")));

		simulationSpeed = 1.0f;

		//BOUNCE
		m_world = new b3World();
		m_step.velocityIterations = 10;

		Stack1(NULL);
		
		//float gs = m_body->GetGravityScale();
		//m_world->SetGravityDirection(b3Vec3(0.0f, -10.0f, 0.0f));
		
		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	void AddABox(b3Vec3 position, b3BodyType bodyType, b3Vec3 scale)
	{
		//Make rigid body
		{
			b3BodyDef* bodyDef = new b3BodyDef;
			bodyDef->gravityScale = 1.0f;
			bodyDef->position = b3Vec3(position.x, 
				position.y, position.z);
			bodyDef->awake = true;
			bodyDef->type = bodyType;
			rigidBodies.push_back(m_world->CreateBody(*bodyDef));
		}
		
		//Give it a shape
		{
			scale = b3Vec3(scale.x / 2.0f, scale.y / 2.0f,
				scale.z / 2.0f);
			b3Hull* hull = new b3Hull;
			hull->SetAsBox(scale);
			b3Polyhedron* polyhedron = new b3Polyhedron;
			polyhedron->SetHull(hull);

			//polyhedron->ComputeMass(

			/*shape->m_body = this;
			shape->m_userData = def.userData;
			shape->m_local = def.local;
			shape->m_density = def.density;
			shape->m_friction = def.friction;
			shape->m_restitution = def.restitution;
			shape->m_isSensor = def.sensor;
			*/
			b3ShapeDef* shapeDef = new b3ShapeDef; 
			shapeDef->shape = polyhedron;
			shapeDef->friction = 0.5f;
			shapeDef->restitution = 0.0f;
			shapeDef->density = 0.5f;

			rigidBodies[rigidBodies.size()-1]->CreateShape(*shapeDef);
			rigidBodies[rigidBodies.size()-1]->m_scale = scale;
		}
	}

	void ClearBoxes()
	{
		//for(auto body : rigidBodies)
		for(int i = rigidBodies.size()-1; i >= 0; i--)
		{
			m_world->DestroyBody(rigidBodies[i]);
			//body->DestroyShapes();
			//delete body;
		}
		rigidBodies.clear();
	}

	static void updateCB()
	{
		Instance->update();
	}

	// GLUT CALLBACK FUNCTIONS
	void update()
	{
		GLProgram::update();

		//rigidBodyManager.Update(deltaTime * simulationSpeed);	
		//rigidBodyManager.Broadphase(broadphaseMode); //Make them always be potentially colliding for purposes of demo		
		//rigidBodyManager.Narrowphase(deltaTime);

		m_step.dt = deltaTime*.001f;
		m_world->Step(m_step);
		
		Draw();
	}

	//Draw loops through each 3d object, and switches to the correct shader for that object, and fill the uniform matrices with the up-to-date values,
	//before finally binding the VAO and drawing with verts or indices
	void Draw()
	{
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		viewMatrix = camera->GetViewMatrix();

		glm::mat4 MVP;

		DrawModels();
		DrawBounceyEnclosure(MVP);

		shaderManager.SetShaderProgram("bounding");
		m_world->Draw(&debugDrawer, b3Draw::b3DrawFlags::e_contactsFlag);
		shaderManager.SetShaderProgram(0);

		for(auto body : rigidBodies)
		{
			glm::vec3 translation = glm::vec3(body->GetTransform().translation.x,
				body->m_worldCenter.y, body->m_worldCenter.z);
			glm::quat orientation = glm::quat(body->m_orientation.d,
				body->m_orientation.a, body->m_orientation.b, 
				body->m_orientation.c);
			glm::vec3 scale = glm::vec3(body->m_scale.x,
				body->m_scale.y, body->m_scale.z);
			MVP = camera->Instance->projectionMatrix * viewMatrix
			* glm::translate(glm::mat4(1), translation)
						  * glm::toMat4(orientation)
						  * glm::scale(glm::mat4(1.0f), scale*2.0f);
			
			shaderManager.SetShaderProgram("bounding");
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), 
				"mvpMatrix", MVP);
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), 
				"boundColour", glm::vec4(1,1,1,1));
			glutWireCube(1);
			shaderManager.SetShaderProgram(0);
		}
		
		if(printText)
		{
			shaderManager.SetShaderProgram("bounding");
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), 
				"boundColour", glm::vec4(1,1,1,1));
			printouts();
			shaderManager.SetShaderProgram(0);
		}

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void DrawBounceyEnclosure(glm::mat4 MVP)
	{
		shaderManager.SetShaderProgram("bounding");
		MVP = camera->Instance->projectionMatrix * viewMatrix;
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), 
			"mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), 
			"boundColour", glm::vec4(1,0,0,1));
		glutWireCube(100);
		shaderManager.SetShaderProgram(0);
	}

	void printouts()
	{
		//shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("text"));
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

		currentLine = 0;
	}

	void HandleInput() override
	{
		GLProgram::HandleInput();

		if(Input::wasKeyPressed)
		{
			if(Input::keyPress == KEY::KEY_S ||
				Input::keyPress == KEY::KEY_s)
				Shoot();
		}
	}

	static void Shoot()
	{
		SolverDemo::Instance->AddABox(
			b3Vec3(Camera::Instance->viewProperties.position.x,
				   Camera::Instance->viewProperties.position.y, 
				   Camera::Instance->viewProperties.position.z),
			e_dynamicBody, b3Vec3(1,1,1));
		SolverDemo::Instance->rigidBodies[
			SolverDemo::Instance->rigidBodies.size()-1]->SetLinearVelocity(
				b3Vec3(
					Camera::Instance->viewProperties.forward.x*2.0f,
					Camera::Instance->viewProperties.forward.y*2.0f,
					Camera::Instance->viewProperties.forward.z*2.0f)
				);
	}

	static void TW_CALL Stack1(void *clientData)
	{
		SolverDemo::Instance->ClearBoxes();

		SolverDemo::Instance->AddABox(b3Vec3(0,0,0), e_staticBody, 
			b3Vec3(10,0.5f,10));
		SolverDemo::Instance->AddABox(b3Vec3(0,10,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0,20,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0,30,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0.5f,40,0), e_dynamicBody, 
			b3Vec3(1,1,1));
	}

	static void TW_CALL Stack2(void *clientData)
	{
		SolverDemo::Instance->ClearBoxes();

		SolverDemo::Instance->AddABox(b3Vec3(0,0,0), e_staticBody, 
			b3Vec3(10,0.5f,10));
		SolverDemo::Instance->AddABox(b3Vec3(0,10,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0,20,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0.5f,40,0), e_dynamicBody, 
			b3Vec3(1,1,1));
	}

	static void TW_CALL Stack3(void *clientData)
	{
		SolverDemo::Instance->ClearBoxes();
		SolverDemo::Instance->AddABox(b3Vec3(0,0,0), e_staticBody, 
			b3Vec3(10,0.5f,10));
		SolverDemo::Instance->AddABox(b3Vec3(0,10,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0,20,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(5.2f,30,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0.5f,40,0), e_dynamicBody, 
			b3Vec3(1,1,1));
	}

	static void TW_CALL Stack4(void *clientData)
	{
		SolverDemo::Instance->ClearBoxes();
		
		SolverDemo::Instance->AddABox(b3Vec3(-0.5,0,-0.5), e_staticBody, 
			b3Vec3(10,0.5f,10));

		for(int i = -3; i < 3; i++)
			for(int j = -3; j < 3; j++)
				for(int k = 1; k < 7; k++)
					SolverDemo::Instance->AddABox(b3Vec3(i,k*2,j), e_dynamicBody, 
						b3Vec3(1,1,1));
	
		/*SolverDemo::Instance->AddABox(b3Vec3(0.5f,30,0), e_dynamicBody, 
			b3Vec3(1,1,1));
		SolverDemo::Instance->AddABox(b3Vec3(0.5f,40,0.8f), e_dynamicBody, 
			b3Vec3(1,1,1));*/
	}

	void SetUpTweakBars()
	{
		TwBar* bar = tweakBars["main"];

		TwAddVarRW(bar, "Sleeping", TW_TYPE_BOOL8, &m_step.sleeping, "");
		TwAddVarRW(bar, "Sleep time", TW_TYPE_FLOAT, 
			&b3ExtraSettings::timeToSleep, "");
		TwAddVarRW(bar, "Damping", TW_TYPE_BOOL8, 
			&b3ExtraSettings::bApplyDamping, "");
		TwAddVarRW(bar, "Velocity Iterations", TW_TYPE_UINT32, 
			&m_step.velocityIterations, "");
		TwAddVarRW(bar, "Warm start", TW_TYPE_BOOL8, 
			&b3ExtraSettings::bWarmStart, "");
		

		/*TwAddVarRW(bar, "Friction coefficient", TW_TYPE_FLOAT, 
			&, "min=0.0 max=1.0 step=0.1");*/

		TwAddButton(bar, "Stack 1", 
			Stack1, NULL, "group='Stacks'");
		TwAddButton(bar, "Stack 2", 
			Stack2, NULL, "group='Stacks'");
		TwAddButton(bar, "Stack 3", 
			Stack3, NULL, "group='Stacks'");
		TwAddButton(bar, "Stack 4", 
			Stack4, NULL, "group='Stacks'");

		TwAddSeparator(bar, "", "");
	}
};