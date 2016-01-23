#ifndef _OBJ3D_H                // Prevent multiple definitions if this 
#define _OBJ3D_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Common.h"

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>
#include <vector>

#include "helper.h"

#include "Magick++.h"

using namespace std;

enum VB_TYPES 
{
	POS_VB,
	NORMAL_VB,
	TEXCOORD_VB,
	//BONE_VB,
	WEIGHT_VB,
	INDEX_VB,
	NUM_VBs            
};

struct WorldProperties
{
	glm::vec3 translation;
	glm::quat orientation;
	glm::vec3 scale;

	WorldProperties()
	{
		translation = glm::vec3(0);
		orientation = glm::quat();
		scale = glm::vec3(0);
	}
};

struct MeshEntry {
    
	MeshEntry()
    {
        NumIndices    = 0;
        BaseVertex    = 0;
        BaseIndex     = 0;
		TextureIndex = 0xFFFFFFFF;
    }
        
    unsigned int NumIndices;
    unsigned int BaseVertex;
    unsigned int BaseIndex;
	unsigned int TextureIndex; 
};

class Model
{
	private:

		GLuint vao;
		vector<int> indices;

		vector<GLuint> textures;
		
		GLuint shaderProgramID;

		int vertexCount;

	public:
		
		bool drawMe;
		bool wireframe;
		bool isColliding;

		vector<glm::vec3> vertices;

		glm::mat4 globalInverseTransform;

		Model(glm::vec3 position, glm::quat orientation, glm::vec3 scale, const char* file_name, GLuint shaderProgramID, bool serialise = true, bool wireframe = false);
		~Model();

		WorldProperties worldProperties;
		vector<MeshEntry> meshEntries;

		bool Load(const char* file_name);
		
		void Render(GLuint shader, bool bWireframe = false);

		/*void Update(double deltaTime)
		{
			
		}*/

		void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

		//Getters
		GLuint GetVAO() { return vao; }
		GLuint GetShaderProgramID() { return shaderProgramID; }
		int GetVertexCount() { return vertexCount; }
		vector<int> GetIndices() { return indices; }
		
		glm::mat4 GetModelMatrix() 
		{ 
			return 
				glm::translate(glm::mat4(1.0f), worldProperties.translation) 
				* glm::toMat4(worldProperties.orientation)
				* glm::scale(glm::mat4(1.0f), worldProperties.scale)
				* globalInverseTransform;
		}		

		glm::vec3 GetEulerAngles()
		{
			return glm::eulerAngles(worldProperties.orientation);
		}

		glm::vec3 GetForward()
		{
			return worldProperties.orientation * glm::vec3(0,0,1);
		}

		std::vector<glm::vec3> GetTransformedVertices()
		{
			std::vector<glm::vec3> transformedVertices;
			glm::mat4 modelMat = GetModelMatrix();

			for(int i = 0; i < vertices.size(); i++)
				transformedVertices.push_back(glm::vec3(glm::vec4(vertices[i], 1) * modelMat));

			return transformedVertices;
		}

		glm::vec3 GetFurthestPointInDirection(glm::vec3 dir)
		{
			glm::vec3 returnVector;

			//convert in world coordinates
			glm::vec4  m = GetModelMatrix() * glm::vec4(vertices[0].x, vertices[0].y, vertices[0].z, 1);
			glm::vec3 worldVertex = glm::vec3(m.x, m.y, m.z);

			float maxDot = glm::dot(worldVertex, dir);
			returnVector = worldVertex;

			for (int i = 1; i < vertices.size(); i++)
			{
				//convert in world coordinates
				m = GetModelMatrix() * glm::vec4(vertices[i].x, vertices[i].y, vertices[i].z, 1);
				worldVertex = glm::vec3(m.x, m.y, m.z);

				float dot = glm::dot(worldVertex, dir);

				//find biggest dot product
				if (dot > maxDot)
				{
					maxDot = dot;
					returnVector = worldVertex;

				}

			}

			return returnVector;
		}

		//Setters
		void SetShaderProgramID(GLuint p_shaderProgramID) { shaderProgramID = p_shaderProgramID; }
};

#endif