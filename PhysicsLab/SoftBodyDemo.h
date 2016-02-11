#pragma once
#include "GLProgram.h"

#include "RigidBody.h"
#include "RigidbodyManager.h"
#include "Inertia.h"

#include <btBulletDynamicsCommon.h>
#include "BulletSoftBody\btSoftBody.h"
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>

#define MAX_PROXIES 32766

class SoftBodyDemo : public GLProgram
{

private:

	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* broadphase;
	btSequentialImpulseConstraintSolver* solver;
	//btSoftBodyRigidBodyCollisionConfiguration* collisionConfiguration;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btSoftBodySolver* softBodySolver;
	//btSoftRigidDynamicsWorld* dynamicsWorld;
	btDiscreteDynamicsWorld* dynamicsWorld;

	//Hello sphere
	btRigidBody* groundRigidBody;
	btRigidBody* fallRigidBody;

	std::vector<RigidBody*> rigidBodies;
	float simulationSpeed;

public:

	static SoftBodyDemo* Instance;

	static glm::vec3 normal[]; 
	static glm::vec3 plane[]; 

	SoftBodyDemo()
	{
		Instance = this;
	}

	~SoftBodyDemo()
	{
		dynamicsWorld->removeRigidBody(fallRigidBody);
		delete fallRigidBody->getMotionState();
		delete fallRigidBody->getCollisionShape();
		delete fallRigidBody;

		dynamicsWorld->removeRigidBody(groundRigidBody);
		delete groundRigidBody->getMotionState();
		delete groundRigidBody->getCollisionShape();
		delete groundRigidBody;

		delete dynamicsWorld;
		delete solver;
		delete collisionConfiguration;
		delete dispatcher;
		delete broadphase;
	}

	void Init(int argc, char** argv)
	{
		GLProgram::Init(argc, argv);

		glutIdleFunc (updateCB);
		
		shaderManager.CreateShaderProgram("diffuse", "Shaders/diffuse.vs", "Shaders/diffuse.ps");
		shaderManager.CreateShaderProgram("bounding", "Shaders/diffuse.vs", "Shaders/bounding.ps");

		modelList.push_back(new Model(glm::vec3(0, 0, 10), glm::quat(), glm::vec3(.0001), "Models/jumbo.dae", shaderManager.GetShaderProgramID("diffuse")));

		AddABox(glm::vec3(0,0,0));

		simulationSpeed = 1.0f;

		//BULLET
		broadphase = new btDbvtBroadphase();
		//btVector3 worldAabbMin(-1000,-1000,-1000);
		//btVector3 worldAabbMax(1000,1000,1000);
		//broadphase = new btAxisSweep3(worldAabbMin,worldAabbMax, MAX_PROXIES);

		collisionConfiguration = new btDefaultCollisionConfiguration();
		//collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration(); //register some softbody collision algorithms on top of the default btDefaultCollisionConfiguration

		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		solver = new btSequentialImpulseConstraintSolver;
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

		//softBodySolver = new btDefaultSoftBodySolver();
		//dynamicsWorld = new btSoftRigidDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration, softBodySolver);

		dynamicsWorld->setGravity(btVector3(0, -10, 0));

		#pragma region HELLO SPHERE
		//Make a floor
		btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
		btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
		btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
		groundRigidBody = new btRigidBody(groundRigidBodyCI);

		dynamicsWorld->addRigidBody(groundRigidBody);

		
		//Make a ball 
		btCollisionShape* fallShape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));//new btSphereShape(1);
		btDefaultMotionState* fallMotionState =
				new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
		btScalar mass = 1;
		btVector3 fallInertia(0, 0, 0);
		fallShape->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
		fallRigidBody = new btRigidBody(fallRigidBodyCI);
		
		dynamicsWorld->addRigidBody(fallRigidBody);
		#pragma endregion

		#pragma region SOFT Body stuff
		//world->setInternalTickCallback(pickingPreTickCallback, this, true);
		//m_dynamicsWorld->getDispatchInfo().m_enableSPU = true;
		//m_guiHelper->createPhysicsDebugDrawer(world);

		/*btSoftBodyWorldInfo softBodyWorldInfo; //Can just make a local variable for this methinks
		softBodyWorldInfo.m_broadphase = broadphase;
		softBodyWorldInfo.m_dispatcher = dispatcher;
		softBodyWorldInfo.m_gravity.setValue(0, -10, 0);
		softBodyWorldInfo.air_density		=	(btScalar)1.2;
		softBodyWorldInfo.water_density	=	0;
		softBodyWorldInfo.water_offset		=	0;
		softBodyWorldInfo.water_normal		=	btVector3(0,0,0);*/

		//world->world

		/*btSoftBody* softBody = btSoftBodyHelpers::CreateEllipsoid(world->getWorldInfo(), btVector3(35,25,0),
			btVector3(1,1,1)*3, 512);
		softBody->m_cfg.viterations = 50;
		softBody->m_cfg.piterations = 50;
		softBody->m_cfg.kPR=1000;
		softBody->setTotalMass(3.0);
		softBody->setMass(0,0);
		world->addSoftBody(softBody);


		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0,0,0));
		btStaticPlaneShape* plane=new btStaticPlaneShape(btVector3(0,1,0),0);
		btMotionState* motion=new btDefaultMotionState(t);
		btRigidBody::btRigidBodyConstructionInfo info(0.0,motion,plane);
		btRigidBody* body=new btRigidBody(info);
		world->addRigidBody(body);
   
		//btSoftBodyHelpers::DrawFrame(

		//m_softBodyWorldInfo.m_sparsesdf.Initialize(); // what the hell is this
		//m_softBodyWorldInfo.m_sparsesdf.Reset();
		//m_softBodyWorldInfo.m_sparsesdf.GarbageCollect();

		//btCollisionShape* groundShape = 0;
		*/
		#pragma endregion

		tweakBars["main"] = TwNewBar("Main");
		TwDefine(" Main size='250 400' position='10 10' color='125 125 125' "); // change default tweak bar size and color

		SetUpTweakBars();
	}

	void AddABox(glm::vec3 position)
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

			rigidBodies[i]->StepPhysics(deltaTime * simulationSpeed); //physics update				
			rigidBodies[i]->Update(); //bookkeeping
		}

		dynamicsWorld->stepSimulation(1 / 60.f, 10);
		btTransform trans;
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
		
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

		btTransform trans;
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		float* matrix = new float[16];
		trans.getOpenGLMatrix(matrix);

		//glm::mat4 transform = glm::value_ptr(matrix);

		//cout << "something";


		/*glm::mat4 transformationMat;
		trans.getOpenGLMatrix(glm::value_ptr(trans));

		shaderManager.SetShaderProgram("bounding");
		MVP = projectionMatrix * viewMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(trans.getOrigin().x, ) 
				* glm::toMat4(worldProperties.orientation)
				* glm::scale(glm::mat4(1.0f), worldProperties.scale)
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
		ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(1,0,0,1));
		glutWireCube(1);*/

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
		SetUpMainTweakBar();
	}

	void SetUpMainTweakBar()
	{
		TwBar* bar = tweakBars["main"];
	
		TwAddVarRW(bar, "Angular", TW_TYPE_BOOL8, &RigidBody::angular, "");
		TwAddVarRW(bar, "Linear", TW_TYPE_BOOL8, &RigidBody::linear, "");

		TwAddSeparator(bar, "", "");
	}
};