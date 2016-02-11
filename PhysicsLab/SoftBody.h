#pragma once

#include "Common.h"
#include <vector>

#include <btBulletDynamicsCommon.h>
#include "BulletSoftBody\btSoftBody.h"
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>

class SoftBody
{
	private:

		GLuint vao;
		GLuint *VBOs; 

		std::vector<int> indices;

	public:

		btSoftBody* softbody;

		SoftBody(btSoftBodyWorldInfo* softBodyWorldInfo)
		{
			softbody = btSoftBodyHelpers::CreateEllipsoid(*softBodyWorldInfo, btVector3(0,0,0), btVector3(1,1,1)*3, 512);
	
			softbody->m_materials[0]->m_kLST	=	0.1;
			softbody->m_cfg.kDF				=	1;
			softbody->m_cfg.kDP				=	0.001; // fun factor...
			softbody->m_cfg.kPR				=	2500;
			softbody->setTotalMass(30,true);
			//softbody->setMass(0,0);
			//world->addSoftBody(softBody);

			/*vertices = &softBody->m_nodes;
			edges = &softBody->m_links;
			faces = &softBody->m_faces;*/

			std::cout << softbody->m_nodes.size();

			//btSoftBody::tNodeArray& nodes(softbody->m_nodes);
			std::vector<glm::vec3> vertices;
			for(int i=0; i < softbody->m_nodes.size(); ++i)
			{
				vertices.push_back(glm::vec3(softbody->m_nodes[i].m_x.getX(), softbody->m_nodes[i].m_x.getY(), 
					softbody->m_nodes[i].m_x.getZ()));
			}
			
			/*btSoftBody::tFaceArray& faces(softbody->m_faces);

			for(int i=0; i < faces.size(); ++i)
			{
				btSoftBody::Node* node_0 = faces[i].m_n[0];
				btSoftBody::Node* node_1 = faces[i].m_n[1];
				btSoftBody::Node* node_2 = faces[i].m_n[2];

				indices.push_back(int(node_0-&softbody->m_nodes[0]));
				indices.push_back(int(node_1-&softbody->m_nodes[0]));
				indices.push_back(int(node_2-&softbody->m_nodes[0]));
			}*/

			glGenVertexArrays (1, &vao);
			VBOs = new GLuint [1];
			glGenBuffers(1, VBOs);

			glBindVertexArray( vao );
			
			glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
			glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STREAM_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[1] );
			//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), &indices[0], GL_STREAM_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		~SoftBody()
		{

		}

		void Update()
		{
			std::vector<glm::vec3> vertices;
			for(int i=0; i < softbody->m_nodes.size(); ++i)
			{
				vertices.push_back(glm::vec3(softbody->m_nodes[i].m_x.getX(), softbody->m_nodes[i].m_x.getY(), 
					softbody->m_nodes[i].m_x.getZ()));
			}

			glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
			glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STREAM_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void Render()
		{
			glBindVertexArray(vao);
			//glDrawArrays(GL_TRIANGLES, 0, softbody->m_nodes.size());

			glPointSize(2.2f);
			glDrawArrays(GL_POINTS, 0, softbody->m_nodes.size());
			glBindVertexArray(0);

			//glBindVertexArray(vao);
			//glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_BYTE, 0);
			//glBindVertexArray(0);
		}
};

/*#pragma once

#include "Model.h"
#include "AABB.h"

struct PointMass
{
	float mass;
	glm::vec3 position;
	glm::vec3 velocity;
	
	glm::vec3 forceNet;
};

class SoftBody
{
	private:

		glm::vec3 initialPosition;

		std::vector<PointMass> points;

		//glm::vec3 forceNet;

	public:

		glm::vec3 velocity;
		glm::vec3 momentum;

		float mass;
		glm::vec3 com;
		Model* model;
		AABB* aabb;

		void StepPhysics(double deltaTime);
		void Update();

		SoftBody(Model* model)
		{

		}

		~SoftBody()
		{

		}
};*/