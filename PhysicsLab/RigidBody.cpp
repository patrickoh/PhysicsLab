#include "RigidBody.h"
#include "Inertia.h"

glm::vec3 RigidBody::force;
Model* RigidBody::impulseVisualiser;
bool RigidBody::angular; 
bool RigidBody::linear; 

RigidBody::RigidBody(Model* model)
{
	this->model = model;
	initialPosition = model->worldProperties.translation;

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

	//ApplyImpulse(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0, 0.0f, 0.0)); 

	boundingSphere = new BoundingSphere(model->vertices);
	aabb = new AABB(model->vertices);
}

RigidBody::~RigidBody()
{
}

void RigidBody::Reset()
{
	model->worldProperties.translation = initialPosition;
	model->worldProperties.orientation = glm::quat();
	
	momentum = glm::vec3(0,0,0);
	velocity = glm::vec3(0,0,0);

	angularMomentum = glm::vec3(0,0,0);
	angularVelocity = glm::vec3(0,0,0);
}

void RigidBody::StepPhysics(double deltaTime)
{
	float timestep = deltaTime / 1000;

	//POSITION UPDATE
	if(RigidBody::linear)
	{
		//P = mv
		//v = P/m

		//Momentum method
		model->worldProperties.translation += velocity * timestep;
		velocity = momentum / mass;
		momentum += forceNet * timestep;
		
		//Acceleration method
		//model->worldProperties.translation += velocity * timestep;
		//velocity += (mass * glm::vec3(0,-9.81,0))/mass * timestep;
	}

	if(RigidBody::angular)
	{
		//ROTATION UPDATE
	
		//L = Iw
		//w = L/I
		//I(t) - R(t) Ibody R(t)T 
	
		angularVelocity = angularMomentum * 
			glm::transpose(glm::toMat3(model->worldProperties.orientation))
			* glm::inverse(inertialTensor) * glm::toMat3(model->worldProperties.orientation);
		//angularVelocity = glm::vec3(0.0f, 0.0f, 1.0f);

		model->worldProperties.orientation += 
			timestep * model->worldProperties.orientation * glm::quat(setAsCrossProductMatrix(angularVelocity));
		//model->worldProperties.orientation *= glm::angleAxis(1.0f, glm::vec3(0,0,1.0f));

		//if(angularVelocity.x != 0 || angularVelocity.y != 0 || angularVelocity.z != 0)
		//{
			std::vector<glm::vec3> extents = aabb->restBBverts;
			RigidBody::transformBatch(&extents, model->worldProperties.orientation);
			aabb->Calculate(extents);
		//}
	
		angularMomentum += torqueNet * timestep;
		
		model->worldProperties.orientation
			= glm::normalize(model->worldProperties.orientation);
	}

	forceNet = glm::vec3(0);
	torqueNet = glm::vec3(0);
}

void RigidBody::Update()
{
	glm::vec3 scale = model->worldProperties.scale;
	float scaleUniform = max(max(scale.x, scale.y), scale.z);
	
	boundingSphere->Update(model->worldProperties.translation, scaleUniform);
	aabb->Update(model->worldProperties.translation, scaleUniform);
}

//Torque = (pt - com) x force
//torque is generated based on the cross product 
//of the force vector 
//and
//the point on the object relative to the center of mass of the object
void RigidBody::ApplyImpulse(glm::vec3 p, glm::vec3 force) 
{ 
	glm::vec3 x = model->worldProperties.translation + (com * model->worldProperties.scale);
	torqueNet += glm::cross(p - x, force);

	forceNet += force;
}

// Calculation of the center of mass based on paul bourke's website
// http://paulbourke.net/geometry/polygonmesh/
glm::vec3 RigidBody::CalculateCentreOfMass(Model* model)
{
	size_t N = model->vertices.size();

	glm::vec3* area = new glm::vec3[N];
	glm::vec3* R = new glm::vec3[N];
	glm::vec3 numerator;
	glm::vec3 denominator;

	for (int i = 0; i < N; i = i + 3) // for each facets --> numerator += R[i] * area[i], denominator += area[i] 
	{
		glm::vec3 v1 = model->vertices[i];
		glm::vec3 v2 = model->vertices[i + 1];
		glm::vec3 v3 = model->vertices[i + 2];
		R[i] = (v1 + v2 + v3) / 3.0f;
		area[i] = glm::abs(glm::cross(v2 - v1, v3 - v1));
		numerator += R[i] * area[i];
		denominator += area[i];
	}

	glm::vec3 com = numerator / denominator;

	if (com != com)
		com = glm::vec3(0.0f, 0.0f, 0.0f);

	delete[] area;
	delete[] R;

	return com;
}