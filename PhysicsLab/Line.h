//#pragma once
//
//#include "Model.h"
//
//class Line 
//{
//	private:
//
//	public:
//
//		Line(glm::vec3 p1, glm::vec3 p2) {
//
//
//
//		}
//
//		WorldProperties worldProperties;
//
//		glm::mat4 GetModelMatrix() 
//		{ 
//			return glm::translate(glm::mat4(1.0f), worldProperties.translation) 
//				* worldProperties.orientation
//				* glm::scale(glm::mat4(1.0f), worldProperties.scale);
//		}	
//
//};

	/*glGenVertexArrays(1, &line_vao);
	glBindVertexArray (line_vao);
	
	lines.push_back(glm::vec3(0,0,0));
	lines.push_back(glm::vec3(0,100,0));

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer (GL_ARRAY_BUFFER, buf);
	glBufferData (GL_ARRAY_BUFFER, 3 * lines.size() * sizeof (GLfloat), &lines[0], GL_STATIC_DRAW);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);*/