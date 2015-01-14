#include "ShaderManager.h"
#include "Shader.h"

ShaderManager* ShaderManager::Instance;

//Creates a program and adds it to the shader program list
GLuint ShaderManager::CreateShaderProgram(std::string name, const std::string& vsFilename, const std::string& psFilename)
{
	GLuint shaderProgramID = glCreateProgram(); //https://www.opengl.org/sdk/docs/man2/xhtml/glCreateProgram.xml
	
	#pragma region ERROR CHECKING
	if (shaderProgramID == 0) 
	{
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}
	#pragma endregion

	// Create two shader objects, one for the vertex, and one for the fragment shader
	Shader vs;
	vs.LoadFile(vsFilename);
	vs.CompileShader(shaderProgramID, GL_VERTEX_SHADER);
	//https://www.opengl.org/sdk/docs/man4/html/glCreateShader.xhtml
	//https://www.opengl.org/sdk/docs/man/html/glShaderSource.xhtml
	//https://www.opengl.org/sdk/docs/man/html/glCompileShader.xhtml
	//https://www.opengl.org/sdk/docs/man/html/glAttachShader.xhtml

	Shader ps;
	ps.LoadFile(psFilename);
	ps.CompileShader(shaderProgramID, GL_FRAGMENT_SHADER);

	shaderProgramList[name] = shaderProgramID;
	shaderProgramListReversed[shaderProgramID] = name;
	
	return shaderProgramID;
}

void ShaderManager::SetShaderProgram(GLuint shaderProgramID)
{
	if(currentShaderProgramID == shaderProgramID)
		return;

	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID); //https://www.opengl.org/sdk/docs/man/html/glUseProgram.xhtml

	currentShaderProgramID = shaderProgramID;
}