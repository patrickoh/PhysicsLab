#pragma once

#include "Common.h"
#include <vector>

class Triangle
{
	private:
		GLuint vao;
		GLuint *VBOs; 
		std::vector<glm::vec3> vertices;

	public:

		Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) 
		{
			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);

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

		~Triangle() 
		{ 
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, VBOs);
		}

		void Render(bool wireframe = false)
		{
			if(wireframe)
				glPolygonMode(GL_FRONT, GL_LINE); 

			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
			glBindVertexArray(0);

			if(wireframe)
				glPolygonMode(GL_FRONT, GL_FILL); 
		}
};

