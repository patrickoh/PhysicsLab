#pragma once

#include "Common.h"
#include <vector>

//Can be used to draw Line, Triangle, Simplex
class Point
{
	private:
		GLuint vao;
		GLuint *VBOs; 

	public:

		glm::vec3 point;

		Point(glm::vec3 p) 
		{
			point = p;

			glGenVertexArrays (1, &vao);
			VBOs = new GLuint [1];
			glGenBuffers(1, VBOs);

			glBindVertexArray( vao );
			
			glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
			glBufferData( GL_ARRAY_BUFFER, sizeof(glm::vec3), &point, GL_STREAM_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		~Point() 
		{ 
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, VBOs);

			vao = 0;
			VBOs[0] = 0;
		}

		void Update(glm::vec3 p_p)
		{
			point = p_p;

			glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), &point);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void Render(float size)
		{
			glBindVertexArray(vao);

			glPointSize(size);              //specify size of points in pixels
			glDrawArrays(GL_POINTS, 0, 1);
 
			glBindVertexArray(0);
		}
};
