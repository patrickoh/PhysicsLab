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
	glm::mat4 orientation;
	glm::vec3 scale;

	WorldProperties()
	{
		translation = glm::vec3(0);
		orientation = glm::mat4(1);
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

		glm::mat4 globalInverseTransform;

		Model(glm::vec3 position, glm::mat4 orientation, glm::vec3 scale, const char* file_name, GLuint shaderProgramID, bool serialise = true, bool wireframe = false);
		~Model();

		WorldProperties worldProperties;
		vector<MeshEntry> meshEntries;

		bool Load(const char* file_name);
		
		void Render(GLuint shader);

		void Update(double deltaTime)
		{
			
		}

		GLuint LoadTexture(const char* fileName);
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
				* worldProperties.orientation
				* glm::scale(glm::mat4(1.0f), worldProperties.scale)
				* globalInverseTransform;
		}		

		glm::vec3 GetEulerAngles()
		{
			return glm::eulerAngles(glm::toQuat(worldProperties.orientation));
		}

		glm::vec3 GetForward()
		{
			return glm::toQuat(worldProperties.orientation) * glm::vec3(0,0,1);
		}

		//Setters
		void SetShaderProgramID(GLuint p_shaderProgramID) { shaderProgramID = p_shaderProgramID; }

};

#endif