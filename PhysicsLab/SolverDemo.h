#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

#include "Bounce.h"

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
		for(auto body : rigidBodies)
		{
			body->DestroyShapes();
			delete body;
		}
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
		m_step.dt = 1.0 / 60.0f;
		m_step.velocityIterations = 10;
		//m_step.sleeping = true;

		AddABox(b3Vec3(0,0,0), e_staticBody, b3Vec3(10,1,10));
		AddABox(b3Vec3(0,10,0), e_dynamicBody, b3Vec3(1,1,1));
		AddABox(b3Vec3(0,11,0), e_dynamicBody, b3Vec3(1,1,1));
		AddABox(b3Vec3(0,12,0), e_dynamicBody, b3Vec3(1,1,1));
		AddABox(b3Vec3(0.5f,13,0), e_dynamicBody, b3Vec3(1,1,1));
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
			b3Hull* hull = new b3Hull;
			hull->SetAsBox(scale);
			b3Polyhedron* polyhedron = new b3Polyhedron;
			polyhedron->SetHull(hull);
			b3ShapeDef* shapeDef = new b3ShapeDef; 
			shapeDef->shape = polyhedron;
			rigidBodies[rigidBodies.size()-1]->CreateShape(*shapeDef);
			rigidBodies[rigidBodies.size()-1]->m_scale = scale;
		}
	}

	static void updateCB()
	{
		Instance->update();
	}

	// GLUT CALLBACK FUNCTIONS
	void update()
	{
		GLProgram::update();

		//m_body->ApplyAngularImpulse(b3Vec3(1,.2,.6), false);
		//m_body->ApplyForce(b3Vec3(1,.2,.7), b3Vec3(-1,3,8), true);

		//m_body->SetAngularVelocity(b3Vec3(0,1,0));

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
			glm::vec3 translation = glm::vec3(body->m_worldCenter.x,
				body->m_worldCenter.y, body->m_worldCenter.z);
			glm::quat orientation = glm::quat(body->m_orientation.a,
				body->m_orientation.b, body->m_orientation.c, 
				body->m_orientation.d);
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

	void SetUpTweakBars()
	{
		TwBar* bar = tweakBars["main"];

		TwAddSeparator(bar, "", "");
	}
};