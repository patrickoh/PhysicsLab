#pragma once

#include "Common.h"
#include <vector>
#include "Point.h"

class Tetrahedron
{
	private:
		GLuint vao;
		GLuint *VBOs; 

		std::vector<glm::vec3> vertices;
		GLubyte indices[6]; 

	public:

		Tetrahedron(std::vector<glm::vec3> p_vertices) 
		{
			indices[0] =  0;
			indices[1] =  1;
			indices[2] =  2;
			indices[3] =  3;
			indices[4] =  0;
			indices[5] =  1;

			for(glm::vec3 p : p_vertices)
				vertices.push_back(p);

			glGenVertexArrays (1, &vao);
			VBOs = new GLuint [2];
			glGenBuffers(2, VBOs);

			glBindVertexArray( vao );
			
			glBindBuffer( GL_ARRAY_BUFFER, VBOs[0] );
			glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STREAM_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); 

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VBOs[1] );
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, &indices[0], GL_STREAM_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		~Tetrahedron() 
		{ 
			//glDeleteVertexArrays(1, &vao);
			//glDeleteBuffers(2, VBOs);
		}

		void Update(std::vector<glm::vec3> p_vertices)
		{
			vertices.clear();

			for(glm::vec3 p : p_vertices)
				vertices.push_back(p);

			glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
			glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STREAM_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void Update(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4)
		{
			std::vector<glm::vec3> vec;

			vec.push_back(v1);
			vec.push_back(v2);
			vec.push_back(v3);
			vec.push_back(v4);

			Update(vec);
		}

		void Render(GLuint shader, bool wireframe)
		{
			if(wireframe)
				glPolygonMode(GL_FRONT, GL_LINE);
			
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_BYTE, 0);
	
			if(wireframe)
				glPolygonMode(GL_FRONT, GL_FILL);
		}
};