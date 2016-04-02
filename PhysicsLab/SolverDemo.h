#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"
#include "Bounce.h"

class SolverDemo : public GLProgram
{

private:

	//std::vector<RigidBody*> rigidBodies;
	float simulationSpeed;

public:

	b3World* m_world;
	b3TimeStep m_step;
	b3Body* m_body;
	b3BodyDef bodyDef;
	b3ShapeDef shapeDef; 

	static SolverDemo* Instance;

	SolverDemo()
	{
		Instance = this;
	}

	~SolverDemo()
	{
		m_body->DestroyShapes();
		delete m_body;
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

		bodyDef.gravityScale = 1.0f;
		bodyDef.position = b3Vec3(0, 10, 0);
		bodyDef.awake = true;
		bodyDef.type = e_dynamicBody;
		m_body = m_world->CreateBody(bodyDef);

		b3Hull* hull = new b3Hull();
		hull->SetAsBox(b3Vec3(1.0f, 1.0f, 1.0f));
		b3Polyhedron* polyhedron = new b3Polyhedron();
		polyhedron->SetHull(hull);
		shapeDef.shape = polyhedron;
		m_body->CreateShape(shapeDef);

		/*while (1)
		{
			m_world->Step(m_step);
			std::cout << m_body->GetTransform().translation.x << ", "
				<< m_body->GetTransform().translation.y << ", "
				<< m_body->GetTransform().translation.z << std::endl;
		}*/

		//float gs = m_body->GetGravityScale();
		//m_world->SetGravityDirection(b3Vec3(0.0f, -10.0f, 0.0f));
		
		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	void AddABox(glm::vec3 position)
	{
		/*Model* m = new Model(position, glm::quat(), glm::vec3(.1), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"), false, false, true);
		RigidBody* rb = new RigidBody(m);
	
		rb->velocity = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));
		rb->angularMomentum = glm::vec3(glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f), glm::linearRand(-1.0f,1.0f));

		rigidBodies.push_back(rb);
		modelList.push_back(m);*/
	}

	static void updateCB()
	{
		Instance->update();
	}

	// GLUT CALLBACK FUNCTIONS
	void update()
	{
		GLProgram::update();

		//static int thing = 0;
		//if(thing == 1)
		//{
			m_world->Step(m_step);
			/*std::cout << m_body->GetTransform().translation.x << ", "
				<< m_body->GetTransform().translation.y << ", "
				<< m_body->GetTransform().translation.z << std::endl;*/
		//}
		
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

		/*MVP = camera->Instance->projectionMatrix * viewMatrix;
		shaderManager.SetShaderProgram("bounding");
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));*/

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void DrawBounceyEnclosure(glm::mat4 MVP)
	{
		shaderManager.SetShaderProgram("bounding");
		MVP = camera->Instance->projectionMatrix * viewMatrix;
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		glutWireCube(10);
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

		currentLine = 0;
	}

	void SetUpTweakBars()
	{
		TwBar* bar = tweakBars["main"];

		TwAddSeparator(bar, "", "");
	}
};