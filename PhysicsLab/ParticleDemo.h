#pragma once
#include "GLProgram.h"
#include "ParticleSystem.h"

class ParticleDemo : public GLProgram
{

private:

	ParticleSystem* particleSystem;
	Model* plane;

	//Model* attractorVis;

public:

	static ParticleDemo* Instance;

	ParticleDemo()
	{
		Instance = this;
	}

	~ParticleDemo()
	{
		delete particleSystem;
		delete plane;

		// TODO - Cleanup VBO and shader
		/*glDeleteBuffers(1, &particles_color_buffer);
		glDeleteBuffers(1, &particles_position_buffer);
		glDeleteBuffers(1, &billboard_vertex_buffer);
		glDeleteProgram(programID);
		glDeleteTextures(1, &TextureID);
		glDeleteVertexArrays(1, &VertexArrayID);*/
	}

	void Init(int argc, char** argv)
	{
		GLProgram::Init(argc, argv);

		glutIdleFunc (updateCB);

		glEnable(GL_POINT_SPRITE);
		glEnable(GL_PROGRAM_POINT_SIZE);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("particle", "Shaders/particle.vs", "Shaders/particle.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

		particleSystem = new ParticleSystem(camera, 1000000);
		
		plane = new Model(glm::vec3(0,0,0), glm::quat(), glm::vec3(2), "Models/plane.dae", shaderManager.GetShaderProgramID("bounding"));
		plane->wireframe = true;
		modelList.push_back(plane);

		//attractorVis = new Model(glm::vec3(0,1,0), glm::quat(), glm::vec3(.01), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"));
		//attractorVis = new Model(glm::vec3(0,1,0), glm::quat(), glm::vec3(.01), "Models/crate.dae", shaderManager.GetShaderProgramID("diffuse"));
		//modelList.push_back(attractorVis);

		particleSystem->Generate();

		tweakBars["emitter"] = TwNewBar("Emitter");
		TwDefine(" Emitter size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		tweakBars["physics"] = TwNewBar("Physics");
		TwDefine(" Physics label='Physics TweakBar' position='1000 300' color='125 125 125' "); // change default tweak bar size and color
		
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

		//Update plane
		double cosAngle = glm::dot(glm::vec3(0,1,0), particleSystem->normal);
		if(cosAngle < 0.9999f)
		{
			float turnAngle = glm::degrees(glm::acos(cosAngle));
			glm::vec3 rotAxis = glm::normalize(glm::cross(glm::vec3(0,1,0), particleSystem->normal));
			plane->worldProperties.orientation = glm::toQuat(glm::rotate(glm::mat4(1), turnAngle, rotAxis));
		}

		particleSystem->Update(deltaTime);

		//attractorVis->worldProperties.translation = particleSystem->userGravity;

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

		{
			shaderManager.SetShaderProgram("bounding");
			
			MVP = camera->Instance->projectionMatrix * viewMatrix *  
				glm::translate(glm::mat4(1.0f), particleSystem->emitter.centre)
				* glm::scale(glm::mat4(1.0f), 2.0f * particleSystem->emitter.variance);
			
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
			glutWireCube(1);
		}

		DrawParticles();

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void DrawParticles()
	{
		GLuint particleShader = shaderManager.GetShaderProgramID("particle");
		shaderManager.SetShaderProgram(particleShader);

		ShaderManager::SetUniform(particleShader, "view", viewMatrix);
		ShaderManager::SetUniform(particleShader, "proj", camera->Instance->projectionMatrix);
		ShaderManager::SetUniform(particleShader, "size", particleSystem->pointSize);
	
		particleSystem->Render();
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

	void printouts()
	{
		shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("text"));

		ss << " Press 'spacebar' or 'esc' to toggle camera/cursor";
		printStream();

		ss << " Press 'c' to switch camera modes";
		printStream();

		ss << " fps: " << fps;
		printStream();

		ss << "camera.forward: (" << std::fixed << std::setprecision(PRECISION) << camera->viewProperties.forward.x << ", " << camera->viewProperties.forward.y << ", " << camera->viewProperties.forward.z << ")";
		printStream();

		ss << "camera.pos: (" << std::fixed << std::setprecision(PRECISION) << camera->viewProperties.position.x << ", " << camera->viewProperties.position.y << ", " << camera->viewProperties.position.z << ")";
		printStream();

		ss << "camera.up: (" << std::fixed << std::setprecision(PRECISION) << camera->viewProperties.up.x << ", " << camera->viewProperties.up.y << ", " << camera->viewProperties.up.z << ")";
		printStream();

		currentLine = 0;
	}

	void SetUpTweakBars()
	{
		SetUpEmitterTweakBar();
		SetUpPhysicsTweakBar();
	}

	void SetUpEmitterTweakBar()
	{
		TwBar* bar = tweakBars["emitter"];

		TwAddVarRW(bar, "EmitRate", TW_TYPE_INT32, &particleSystem->emitter.emitRate, "");

		TwAddVarRW(bar, "ParticleSize", TW_TYPE_FLOAT, &particleSystem->pointSize, "min=0.0");

		TwAddVarRW(bar, "Particle life", TW_TYPE_FLOAT, &particleSystem->particleLife, "min=0.0 step=0.25");
		TwAddVarRW(bar, "StartColour", TW_TYPE_COLOR3F, &particleSystem->startColour, " group='Colours' ");
		TwAddVarRW(bar, "EndColour", TW_TYPE_COLOR3F, &particleSystem->endColour, " group='Colours' ");

		TwAddVarRW(bar, "VelYMin", TW_TYPE_FLOAT, &particleSystem->emitter.velYMin, " group='Initial Velocity' ");
		TwAddVarRW(bar, "VelYMax", TW_TYPE_FLOAT, &particleSystem->emitter.velYMax, " group='Initial Velocity' ");
		TwAddVarRW(bar, "VelRadiusMin", TW_TYPE_FLOAT, &particleSystem->emitter.velRadiusMin, " group='Initial Velocity' ");
		TwAddVarRW(bar, "VelRadiusMax", TW_TYPE_FLOAT, &particleSystem->emitter.velRadiusMax, " group='Initial Velocity' ");

		TwAddSeparator(bar, "", "");

		TwAddVarRW(bar, "emitterArea", TW_TYPE_DIR3F, &particleSystem->emitter.variance, " ");

		TwAddVarRW(bar, "emitterPos", TW_TYPE_DIR3F, &particleSystem->emitter.centre, " ");

		TwAddSeparator(bar, "", "");

		TwAddVarRW(bar, "RecycleAge", TW_TYPE_BOOL8, &particleSystem->bRecycleAge, " label='RecycleAge'");
		TwAddVarRW(bar, "RecyclePlane", TW_TYPE_BOOL8, &particleSystem->bRecyclePlane, "label='RecyclePlane'");

		TwAddSeparator(bar, "", "");
		TwAddVarRO(bar, "Live Particles", TW_TYPE_INT32, &particleSystem->liveParticles, " label='ParticleCount'");
		TwAddVarRW(bar, "Particle mass", TW_TYPE_FLOAT, &particleSystem->mass, "min=0.1");

		TwAddSeparator(bar, "", "");
		TwAddVarRW(bar, "ZSort", TW_TYPE_BOOL8, &particleSystem->bZSort, "");

	}

	void SetUpPhysicsTweakBar()
	{
		TwBar* bar = tweakBars["physics"];

		/*TwAddSeparator(bar, "", "");
		TwAddVarRW(bar, "UserGravity", TW_TYPE_BOOL8, &particleSystem->, " label='UserGravity'");
		TwAddVarRW(bar, "UserGravityPos", TW_TYPE_DIR3F, &particleSystem->userGravity, " label='UserGravityPos'");
		TwAddVarRW(bar, "UserGravityStr", TW_TYPE_FLOAT, &particleSystem->userGravityScalar, " label='UserGravityStr'");*/
		
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
	}
};