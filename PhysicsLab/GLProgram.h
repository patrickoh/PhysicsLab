#pragma once
#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include <assert.h>

#include <glm/gtx/random.hpp>
#include <glm\common.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "ShaderManager.h"

#include "Common.h"
#include "Keys.h"

#include <map>
#include <vector>
#include <list>

#include "Input.h"

class GLProgram
{ 
public:
	std::stringstream ss;

	Camera* camera;

	glm::mat4 projectionMatrix; // Store the projection matrix
	glm::mat4 viewMatrix;

	bool freeMouse;

	int WINDOW_WIDTH;
	int WINDOW_HEIGHT;

	int oldTimeSinceStart;
	double deltaTime;

	int fps;
	int frameCounterTime;
	int frames;

	ShaderManager shaderManager;
	vector<Model*> modelList;

	bool printText; 

	std::map<std::string, TwBar*> tweakBars;
	Input input;

	int currentLine;

	static GLProgram* Instance;

	static glm::vec3 normal[]; 
	static glm::vec3 plane[]; 

	GLProgram()
	{
		currentLine = 0;
		oldTimeSinceStart = 0;
		frames = 0;
		frameCounterTime = 0;
		fps = 0;
		WINDOW_WIDTH = 1280;
		WINDOW_HEIGHT = 720;
		freeMouse = false;
		printText = true;

		Instance = this;
	}

	virtual ~GLProgram() = 0;

	virtual void Init(int argc, char* argv[])
	{
		// Set up the window
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		glutInitWindowPosition (0, 0); 
		glutCreateWindow("All of the Physics");

		//AntTweakBar
		TwInit(TW_OPENGL_CORE, NULL);
		TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
		TwDefine(" GLOBAL help='AntTweakBar.' "); // Message added to the help bar.	

		//glutFullScreen();
		glutSetCursor(GLUT_CURSOR_NONE);

		// REGISTER GLUT CALLBACKS
		glutKeyboardFunc(Input::keyPressed); // Tell GLUT to use the method "keyPressed" for key presses  
		glutKeyboardUpFunc(Input::keyUp); // Tell GLUT to use the method "keyUp" for key up events  
		glutSpecialFunc(Input::handleSpecialKeypress);
		glutSpecialUpFunc(Input::handleSpecialKeyReleased);
		glutMouseFunc (Input::mouseButton);
		glutMouseWheelFunc(Input::mouseWheel);
		glutPassiveMotionFunc(Input::passiveMouseMotion);
		//glutMotionFunc (MouseMotion);
	
		//glutDisplayFunc(draw);
		//glutReshapeFunc (reshape);

		TwGLUTModifiersFunc(glutGetModifiers);
		glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);

		// A call to glewInit() must be done after glut is initialized!
		glewExperimental = GL_TRUE;
		GLenum res = glewInit();

		glClearColor(155.0/255.0, 155.0/255.0, 155.0/255.0, 1);
		glEnable (GL_CULL_FACE); // cull face 
		glCullFace (GL_BACK); // cull back face 
		glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
		glEnable(GL_DEPTH_TEST);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable( GL_BLEND );

		projectionMatrix = glm::perspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f /*near plane*/, 100.f /*far plane*/); // Create our perspective projection matrix

		camera = new Camera(glm::vec3(0.0f, 0.0f, 0.0f), CameraMode::tp);

		shaderManager.Init(); //TODO - constructor for shader
	}

	void Run()
	{
		glutMainLoop();
	}

	// GLUT CALLBACK FUNCTIONS
	virtual void update()
	{
		//Calculate deltaTime
		int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
		deltaTime = timeSinceStart - oldTimeSinceStart;
		oldTimeSinceStart = timeSinceStart;

		//Calculate fps
		frames++;
		frameCounterTime += deltaTime;
		if(frameCounterTime > 1000)
		{
			fps = frames;
			frames = frameCounterTime = 0;
		}

		HandleInput();
		
		if(Input::wasKeyPressed)
			Input::wasKeyPressed = false;
	
		camera->Update(deltaTime);
	}

	virtual void HandleInput()
	{
		if(!freeMouse)
		{
			if(Input::mouseMoved)
			{
				camera->MouseRotate(Input::mouseX, Input::mouseY, WINDOW_WIDTH, WINDOW_HEIGHT, deltaTime); 
				Input::mouseMoved = false;
			}

			glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
		}

		if(Input::wasKeyPressed)
		{
			camera->ProcessKeyboardOnce(Input::keyPress); 

			if(Input::keyPress == KEY::KEY_h || Input::keyPress == KEY::KEY_H) 
				printText = !printText;

			if(Input::keyPress == KEY::KEY_SPACE || Input::keyPress == KEY::KEY_ESCAPE) 
			{
				if(!freeMouse)
				{
					freeMouse = true;
					glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
				}
				else
				{
					freeMouse = false;
					glutSetCursor(GLUT_CURSOR_NONE);
				}
			}

			
		}

		camera->ProcessKeyboardContinuous(Input::keyStates, deltaTime);
	}

	void DrawModels()
	{
		for (int i = 0; i < modelList.size(); i++)
		{
			if (modelList[i]->drawMe)
			{
				glm::mat4 MVP = projectionMatrix * viewMatrix * modelList.at(i)->GetModelMatrix(); //TODO - move these calculations to the graphics card?
			
				shaderManager.SetShaderProgram(modelList[i]->GetShaderProgramID());
				ShaderManager::SetUniform(modelList[i]->GetShaderProgramID(), "mvpMatrix", MVP);

				if(modelList[i]->renderColour)
					ShaderManager::SetUniform(modelList[i]->GetShaderProgramID(), "boundColour", modelList[i]->colour);

				modelList.at(i)->Render(shaderManager.GetCurrentShaderProgramID());
	
				shaderManager.SetShaderProgram(shaderManager.GetShaderProgramID("bounding"));
				ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "mvpMatrix", MVP);
				ShaderManager::SetUniform(shaderManager.GetCurrentShaderProgramID(), "boundColour", glm::vec4(0,0,0,1));
				modelList.at(i)->Render(shaderManager.GetShaderProgramID("bounding"), true);
			}
		}	
	}

	void printStream()
	{
		currentLine += 20;
		drawText(WINDOW_WIDTH-(strlen(ss.str().c_str())*LETTER_WIDTH),WINDOW_HEIGHT-currentLine, ss.str().c_str()); //Bottom left is 0,0
		ss.str(std::string()); //reset
	}

	virtual void printouts() = 0;

};

inline GLProgram::~GLProgram() { };