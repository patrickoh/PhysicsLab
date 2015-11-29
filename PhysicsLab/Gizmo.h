#pragma once

#include "Common.h"
#include <vector>

//Can be used to draw Line, Triangle, Simplex
class Gizmo 
{
	private:
		GLuint vao;
		GLuint *VBOs; 
		std::vector<glm::vec3> vertices;

	public:

		Gizmo(std::vector<glm::vec3> p_vertices) 
		{
			vertices = p_vertices;

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

		~Gizmo() 
		{ 
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, VBOs);
		}

		void Render(GLuint shader)
		{
			glBindVertexArray(vao);

			glPolygonMode(GL_FRONT, GL_LINE); 

			if(vertices.size() < 3)
				glDrawArrays (GL_LINE_LOOP, 0, vertices.size());
			else
				glDrawArrays(GL_TRIANGLES, 0, vertices.size());	
			
			glPolygonMode(GL_FRONT, GL_FILL); 
		}

};

