#ifndef _CAMERA_H                // Prevent multiple definitions if this 
#define _CAMERA_H                // file is included in more than one place

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>

#include "Keys.h"
#include "Common.h"

enum CameraMode { flycam, tp, NUM_CAM_MODES };

struct ViewProperties 
{
	glm::vec3 position;
	glm::vec3 forward; 
	glm::vec3 up;
	glm::quat XZrotation;
};

class Camera
{
	private:

	public:

		ViewProperties viewProperties;

		CameraMode mode;

		int winw, winh;
		int inputX, inputY;

		//Flycam
		float horizontalXZAngle;
		float verticalAngle;

		float turnSpeed;
		float moveSpeed;

		//Third-person cam
		glm::vec3 target;
		float distance;
		float minDistance, maxDistance;

		float scrollWheelSensivity;

		//float yMaxLimit; //= -40f
		//float yMinLimit; //=70f

		glm::vec3 targetYOffset;

		static Camera* Instance;

		glm::mat4 projectionMatrix;

		Camera(glm::vec3 position, CameraMode p_mode, 
			float p_turnSpeed = 0.0002f, float p_moveSpeed = 0.01f)
		{
			Instance = this;

			turnSpeed =  p_turnSpeed; 
			moveSpeed = p_moveSpeed; 
	
			minDistance = 2;
			maxDistance = 15;
			scrollWheelSensivity = 0.15f;

			SetUp(position, p_mode);
		}
		
		//0.005f, 0.01f
		void SetUp(glm::vec3 position, CameraMode p_mode) 
		{ 
			viewProperties.position = position;
			viewProperties.up = glm::vec3(0,1,0);

			mode = p_mode;
			distance = 3;

			target = glm::vec3(0,0,0);
			targetYOffset = glm::vec3(0,1,0);

			horizontalXZAngle = 0;
			verticalAngle = 0;
		}

		void MouseRotate(int x, int y, int p_winw, int p_winh, int deltaTime)
		{
			inputX = x;
			inputY = y;

			//TODO - if third person cam clamp angle

			winh = p_winh;
			winw = p_winw;

			horizontalXZAngle += turnSpeed * deltaTime * float(winw/2 - inputX);
			verticalAngle += turnSpeed * deltaTime * float(winh/2 - inputY);
		}

		void Zoom(GLfloat amount)
		{
			if(mode == CameraMode::tp)
			{
				distance += (amount * scrollWheelSensivity);
				distance = glm::clamp(distance, minDistance, maxDistance);
			}
		}

		void SetTarget(glm::vec3 p_target)
		{
			target = p_target + targetYOffset;
		}

		void Update(int deltaTime)
		{
			if(mode == CameraMode::flycam)
			{
				viewProperties.forward = glm::vec3(cos(verticalAngle) * sin(horizontalXZAngle),
											   sin(verticalAngle),
											   cos(verticalAngle) * cos(horizontalXZAngle));

				glm::vec3 right = glm::vec3(sin(horizontalXZAngle - 3.14f/2.0f), 0, cos(horizontalXZAngle - 3.14f/2.0f));
				viewProperties.up = glm::cross(right, viewProperties.forward);
			}
			else if(mode == CameraMode::tp)
			{
				glm::vec3 direction = glm::vec3(0, 0, -distance);

				glm::vec3 right = glm::vec3(sin(horizontalXZAngle - 3.14f/2.0f), 0, cos(horizontalXZAngle - 3.14f/2.0f));
				
				glm::quat rotation = glm::quat(glm::vec3(0, -horizontalXZAngle, 0)); //yaw
				this->viewProperties.XZrotation = rotation;
				rotation *= glm::angleAxis(-verticalAngle * 100, right); //pitch
				//could actually have done it with just quat it seems!

				//xz rotation is around y axis
				//y rotation is around arbitary axis

				viewProperties.position = direction * rotation + target; 	
				viewProperties.forward = target - viewProperties.position;	
				viewProperties.up = glm::cross(right, viewProperties.forward);
			}
			else
			{
				viewProperties.forward = target - viewProperties.position;	
				viewProperties.up = glm::vec3(0,1,0);
			}
		}

		glm::mat4 GetViewMatrix()
		{
			return glm::lookAt(viewProperties.position, viewProperties.position + viewProperties.forward, viewProperties.up); 
			//glm::LookAt(
				//cameraPosition, // the position of your camera, in world space
				//cameraTarget,   // where you want to look at, in world space
				//upVector        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
			//);
		}

		void ProcessKeyboardContinuous(bool* keyStates, double deltaTime)
		{
			if(mode == CameraMode::flycam)
			{
				if(keyStates[KEY::KEY_w] || keyStates[KEY::KEY_W])
					viewProperties.position += viewProperties.forward * float(deltaTime) * moveSpeed; 
				if(keyStates[KEY::KEY_s] || keyStates[KEY::KEY_S])
					viewProperties.position -= viewProperties.forward  * float(deltaTime) * moveSpeed; 
				if(keyStates[KEY::KEY_a] || keyStates[KEY::KEY_A])
					viewProperties.position -= glm::cross(viewProperties.forward, viewProperties.up) * float(deltaTime) * moveSpeed;
				if(keyStates[KEY::KEY_d] || keyStates[KEY::KEY_D])
					viewProperties.position += glm::cross(viewProperties.forward, viewProperties.up) * float(deltaTime) * moveSpeed;
				if(keyStates[KEY::KEY_q] || keyStates[KEY::KEY_Q])
					viewProperties.position -= viewProperties.up * float(deltaTime) * moveSpeed; 
				if(keyStates[KEY::KEY_e] || keyStates[KEY::KEY_E])
					viewProperties.position += viewProperties.up * float(deltaTime) * moveSpeed; 
			}
		}

		void ProcessKeyboardOnce(unsigned char key)
		{
			if (key == KEY::KEY_C || key == KEY::KEY_c)
			{
				mode = static_cast<CameraMode>((mode+1) % NUM_CAM_MODES);

				SetUp(glm::vec3(0), mode);
			}
		}

};

#endif