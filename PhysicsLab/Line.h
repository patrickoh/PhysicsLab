#pragma once

#include "Common.h"
#include <vector>

//Can be used to draw Line, Triangle, Simplex
class Line
{
	private:
		GLuint vao;
		GLuint *VBOs; 
		std::vector<glm::vec3> vertices;

	public:

		Line(glm::vec3 from, glm::vec3 to) 
		{
			vertices.push_back(from);
			vertices.push_back(to);

			glGenVertexArrays (1, &vao);
			VBOs = new GLuint [1];
			glGenBuffers(1, VBOs);

			glBindVertexArray( vao );
			
			glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
			glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		~Line() 
		{ 
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, VBOs);
		}

		void Render()
		{
			glBindVertexArray(vao);
			glDrawArrays (GL_LINE_LOOP, 0, vertices.size());
		}
};

