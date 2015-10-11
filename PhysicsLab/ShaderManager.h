#ifndef _SHADERMANAGER_H                // Prevent multiple definitions if this 
#define _SHADERMANAGER_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>

#include "Common.h"

class ShaderManager
{
	private:
		std::map <std::string, GLuint> shaderProgramList;
		std::map <GLuint, std::string> shaderProgramListReversed; //TODO - use boost multiindex
		GLuint currentShaderProgramID;

	public:

		static ShaderManager* Instance;

		void Init() { Instance = this; }

		GLuint CreateShaderProgram(std::string name, const std::string& vsFilename, const std::string& psFilename);
		
		void SetShaderProgram(std::string shaderProgramName) { SetShaderProgram(shaderProgramList[shaderProgramName]); };
		void SetShaderProgram(GLuint shaderProgramID);

		GLuint GetShaderProgramID(std::string shaderProgramName) { return shaderProgramList[shaderProgramName]; }
		std::string GetShaderProgramName(GLuint ID) { return shaderProgramListReversed[ID]; }
		
		GLuint GetCurrentShaderProgramID() { return currentShaderProgramID; }

		static void SetUniform(GLuint id, std::string name, glm::vec3 value)
		{
			glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(value));
		}

		static void SetUniform(GLuint id, std::string name, glm::vec4 value)
		{
			glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(value));
		}

		static void SetUniform(GLuint id, std::string name, glm::mat3 value)
		{
			glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
		}

		static void SetUniform(GLuint id, std::string name, glm::mat4 value)
		{
			glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
		}

		static void SetUniform(GLuint id, std::string name, GLfloat value)
		{
			glUniform1f(glGetUniformLocation(id, name.c_str()), value);
		}
};

#endif