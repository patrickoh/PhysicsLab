#ifndef _SHADER_H                // Prevent multiple definitions if this 
#define _SHADER_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

class Shader
{
	private:
		std::string shaderText;

	public:
		Shader();
		virtual ~Shader();

		bool LoadFile(const std::string& fileName);
		void CompileShader(GLuint ShaderProgram, GLenum ShaderType);
};

#endif