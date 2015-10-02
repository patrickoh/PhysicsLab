#pragma once

#include "GL\glew.h"
#include <vector>
#include <glm/glm.hpp>
#include "glm\gtc\random.hpp"

class Sphere
{
	private:

		GLuint vbo, vao;
		std::vector<glm::vec3> vertices;
		
		glm::vec3 translation;
		//glm::vec3 scale;
	
	public:

		Sphere(float radius, int resolution, glm::vec3 position)
		{
			//sphere procedural generation

			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);

			glBindVertexArray(vao);
			glBindBuffer(GL_VERTEX_ARRAY, vbo);
			glBufferData(GL_VERTEX_ARRAY, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);	
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
		}

		void Draw()
		{
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
			glBindVertexArray(0);
		}	
};