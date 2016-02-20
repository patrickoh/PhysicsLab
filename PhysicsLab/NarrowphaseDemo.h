#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

#include "Tetrahedron.h"
#include "Line.h"
#include "Triangle.h"

class NarrowphaseDemo : public GLProgram
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
	Tetrahedron* gjkTetra;
	std::vector<Point*> minkowskiDifferencePoints;

	int rbIdx;

public:
	
	static NarrowphaseDemo* Instance;

	NarrowphaseDemo()
	{
		Instance = this;
	}

	~NarrowphaseDemo()
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

		glm::vec3 v1 = glm::vec3(0,8,0);
		glm::vec3 v2 = glm::vec3(1,0,1);
		glm::vec3 v3 = glm::vec3(0,0,1);
		glm::vec3 v4 = glm::vec3(1,0,0);
		std::vector<glm::vec3> vec;
		vec.push_back(v1);
		vec.push_back(v2);
		vec.push_back(v3);
		vec.push_back(v4);

		gjkTetra = new Tetrahedron(vec);

		rbIdx = 0;

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	void AddBox(glm::vec3 position)
	{
		Model* m = new Model(position, glm::quat(), glm::vec3(.1), "Models/cubeTri.obj", shaderManager.GetShaderProgramID("bounding"), false, false, true);
		RigidBody* rb = new RigidBody(m);
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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		viewMatrix = camera->GetViewMatrix();
		
		glm::mat4 MVP;

		DrawModels();
		DrawBounceyEnclosure(MVP);
		DrawMouse(MVP);
		DrawBoundings();

		//Draw origin
		MVP = projectionMatrix * viewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0));
		shaderManager.SetShaderProgram("bounding");
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,1,1));
		origin->Render(5.0f);
		
		DrawGJK();

		if(printText)
			printouts();

		TwDraw(); // Draw tweak bars

		glutSwapBuffers();
	}

	void DrawGJK()
	{
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,1,1,1));
		for(SupportPoint p : rigidBodyManager.gjk->simplex)
		{
			Point p(p.AB);
			p.Render(5.0f);
		}

		int simplexSize = rigidBodyManager.gjk->simplex.size();
	
		if(simplexSize == 2)
		{
			Line l(rigidBodyManager.gjk->simplex[0].AB, rigidBodyManager.gjk->simplex[1].AB);
			l.Render();
		}
		else if(simplexSize == 3)
		{	
			Triangle t(rigidBodyManager.gjk->simplex[0].AB, 
				rigidBodyManager.gjk->simplex[1].AB, rigidBodyManager.gjk->simplex[2].AB);
			
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0.1f,0.9f,0.1f,0.2f));
			t.Render();

			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1.0f,1.0f,1.0f,1.0f));
			t.Render(true);

		}
		else if(simplexSize == 4)
		{
			gjkTetra->Update(rigidBodyManager.gjk->simplex);

			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0.1f,0.9f,0.1f,0.2f));
			gjkTetra->Render(shaderManager.GetCurrentShaderProgramID(), false);
		
			ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1.0f,1.0f,1.0f,1.0f));
			gjkTetra->Render(shaderManager.GetCurrentShaderProgramID(), true);
		}

		/*ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,0,1));
		for(int i = 0; i < rigidBodyManager.currentMinkowski.size(); i++)
		{
			Point p(rigidBodyManager.currentMinkowski[i]);
			p.Render(7.5f);
		}*/
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

		/*sint64 broadphaseResult = QueryPerformance::results["Broadphase"];
		broadphaseResults += broadphaseResult;
		ss << " Query Performance - Broadphase (Avg.): " << broadphaseResults / ++broadphaseResultCounter;
		printStream();*/

		ss << "Rigid Bodies: " << rigidBodyManager.rigidBodies.size();
		printStream();

		ss << "gjk->simplex.size(): " << rigidBodyManager.gjk->simplex.size();
		printStream();

		ss << "gjk steps: " << rigidBodyManager.gjk->steps;
		printStream();

		ss << "bClickImpulse: " << bClickImpulse;
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
			if(Input::keyPress == KEY::KEY_G ||
				Input::keyPress == KEY::KEY_g)
				rigidBodyManager.stepDebug = true;

			if(Input::keyPress == KEY::KEY_TAB)
			{
				rbIdx = (rbIdx + 1) % rigidBodyManager.rigidBodies.size();
				TwRemoveVar(tweakBars["main"], "selected RB");
				TwAddVarRW(tweakBars["main"], "selected RB", TW_TYPE_DIR3F, 
					&rigidBodyManager.rigidBodies[rbIdx]->model->worldProperties.translation,
					"");

				rigidBodyManager[(rbIdx-1) % rigidBodyManager.rigidBodies.size()]->model->colour = glm::vec4(0.5,0.5,0.5,1.0);
				rigidBodyManager[rbIdx]->model->colour = glm::vec4(0.7f,0.2f, 0.2f,0.5);
			}

			if(Input::keyPress == KEY::KEY_K ||
				Input::keyPress == KEY::KEY_k)
				bClickImpulse = !bClickImpulse;

			if(Input::keyPress == KEY::KEY_P ||
				Input::keyPress == KEY::KEY_p)
				rigidBodyManager.pausedSim = !rigidBodyManager.pausedSim;
		}
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

	void CalculateNewTensors()
	{
		for(RigidBody* rb : rigidBodyManager.rigidBodies)
		{
			rb->mass = mass;
			rb->inertialTensor = Inertia::Compute2(rb->model, rb->mass);
		}
	}

	static void TW_CALL CalculateNewTensorsCB(void *clientData)
	{
		NarrowphaseDemo::Instance->CalculateNewTensors();
	}

	void ResetRB()
	{
		/*for(RigidBody* rb : rigidBodyManager.rigidBodies)
		{
			rb->Reset();
		}*/

		rigidBodyManager[rbIdx]->Reset();
	}

	static void TW_CALL ResetRBCB(void *clientData)
	{
		NarrowphaseDemo::Instance->ResetRB();
	}

	void AddBoxButton()
	{
		for(int i = 0; i < addAmount; i++)
			AddBox(glm::vec3(0,0,0));
	}

	static void TW_CALL AddBoxButtonCB(void *clientData)
	{
		NarrowphaseDemo::Instance->AddBoxButton();
	}

	void SetUpTweakBars()
	{
		TwBar* bar = tweakBars["main"];

		TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
		TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

		TwAddVarRW(bar, "Draw Bounding Spheres", TW_TYPE_BOOL8, &drawBoundingSpheres, "");
		TwAddVarRW(bar, "Draw AABBs", TW_TYPE_BOOL8, &drawBoundingBoxes, "");

		TwAddVarRW(bar, "Enclosure size", TW_TYPE_INT32, &rigidBodyManager.bounceyEnclosureSize, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		//TwAddVarRW(bar, "Simulation Speed", TW_TYPE_FLOAT, &simulationSpeed, "");
		TwAddVarRW(bar, "Pause simulation", TW_TYPE_BOOL8, &rigidBodyManager.pausedSim, "");
		TwAddVarRW(bar, "Auto-pause", TW_TYPE_BOOL8, &rigidBodyManager.autoPause, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		{
			TwEnumVal broadphaseModeEV[4] = { {BroadphaseMode::SAP1D, "SAP"}, {BroadphaseMode::BruteAABB, "BruteAABB"}, {BroadphaseMode::Sphere, "Sphere"},
				{BroadphaseMode::Skip, "Skip"} };
			TwType broadphaseType = TwDefineEnum("IntegratorType", broadphaseModeEV, 4);
			TwAddVarRW(bar, "BroadphaseMode", broadphaseType, &broadphaseMode, " keyIncr='<' keyDecr='>' help='Change broadphase mode.' ");
		}

		//TwAddButton(bar, "Reset", ResetPerformanceCounterCB, NULL, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "Mass", TW_TYPE_FLOAT, &mass, "");
		TwAddButton(bar, "Recalculate Tensors", CalculateNewTensorsCB, NULL, "");
		TwAddButton(bar, "Reset", ResetRBCB, NULL, "");
		TwAddVarRW(bar, "Impulse Force", TW_TYPE_FLOAT, &forcePush, "");
		TwAddVarRW(bar, "bClick to Impulse", TW_TYPE_BOOL8, &bClickImpulse, "");

		TwAddSeparator(bar, "", "");

		TwAddVarRW(bar, "Add Amount", TW_TYPE_INT32, &addAmount, "");
		TwAddButton(bar, "Add Box(es)", AddBoxButtonCB, NULL, "");

		TwAddSeparator(bar, "", ""); //=======================================================

		TwAddVarRW(bar, "Debug GJK", TW_TYPE_BOOL8, &rigidBodyManager.debugGJK, "");
		TwAddVarRW(bar, "selected RB", TW_TYPE_DIR3F, &rigidBodyManager.rigidBodies[rbIdx]->model->worldProperties.translation, "");
	}
};