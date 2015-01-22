#include "RigidBody.h"
#include "Inertia.h"

RigidBody::RigidBody(Model* model)
{
	this->model = model;

	momentum = glm::vec3(0,0,0); //momentum = m*v, 
	mass = 1;

	com = CalculateCentreOfMass(model);

	angularMomentum = glm::vec3(0,0,0);
	
	//inertialTensor = mass * length * length * 1.0f / 6.0f; 
	inertialTensor = Inertia::Compute2(model, mass);

	//Things that affect the tensor
	//1) increasing mass increases moment of inertia
	//2) distribution of mass from com increase moment of inertia
	//3) for most objects will depend on the axis of rotation

	ApplyImpulse(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0, 0.0f, 0.0)); 
}

RigidBody::~RigidBody()
{
}

void RigidBody::Update(double deltaTime)
{
	float timestep = deltaTime / 1000;

	//POSITION UPDATE

	//P = mv
	//v = P/m

	model->worldProperties.translation += velocity * timestep;
	//velocity = momentum / mass;
	momentum += (mass * glm::vec3(0,-9.81,0)) * timestep;

	//model->worldProperties.translation += velocity * timestep;
	//velocity += (mass * glm::vec3(0,-9.81,0))/mass * timestep;

	//=======================================================

	//ROTATION UPDATE
	
	//L = Iw
	//w = L/I
	//I(t) - R(t) Ibody R(t)T 
	
	angularVelocity = angularMomentum * 
		glm::transpose(glm::toMat3(model->worldProperties.orientation))
		* glm::inverse(inertialTensor) * glm::toMat3(model->worldProperties.orientation);
	//angularVelocity = glm::vec3(0,1.0f,0);
	
	model->worldProperties.orientation += 
		 timestep * glm::quat(setAsCrossProductMatrix(angularVelocity)) * model->worldProperties.orientation;
	
	angularMomentum += torque * timestep;
	torque = glm::vec3(0);

	model->worldProperties.orientation
		= glm::normalize(model->worldProperties.orientation);
}

//Tau = (pt - com) x force
//torque is generated based on the cross product 
//of the force vector 
//and
//the point on the object relative to the center of mass of the object
void RigidBody::ApplyImpulse(glm::vec3 p, glm::vec3 force) 
{ 
	glm::vec3 x = model->worldProperties.translation + com;
	torque += glm::cross(p - x, force);
}