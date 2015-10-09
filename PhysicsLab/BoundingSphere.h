#pragma once

#include "Common.h"

struct BoundingSphere
{
	glm::vec3 centre; //in local space
	float radius;

	//TODO - visualisation
	//bool vis;
	//glm::vec4 colour;

	std::vector<glm::vec3> vertices;

	BoundingSphere(const std::vector<glm::vec3>& v)
	{
		//Ritter's algorithm 
		glm::vec3 min, max;

		for (glm::vec3 point : v)
		{
			if (point.x < min.x) min.x = point.x;
			if (point.x > max.x) max.x = point.x;
			if (point.y < min.y) min.y = point.y;
			if (point.y > max.y) max.y = point.y;
			if (point.z < min.z) min.z = point.z;
			if (point.z > max.z) max.z = point.z;
		}

		float xSpan = max.x - min.x;
		float ySpan = max.y - min.y;
		float zSpan = max.z - min.y;

		float maxSpan = xSpan;

		if (ySpan > maxSpan)
			maxSpan = ySpan;

		if (zSpan > maxSpan)
			maxSpan = zSpan;

		centre = (min+max) * 0.5f;
		radius = maxSpan / 2;

		for  (glm::vec3 point : v)
		{
			//float distance = (point - centre).length;
			glm::vec3 direction = point - centre;

			if(direction.length() > radius)
			{
				float difference = direction.length() - radius;
				radius = (radius + direction.length()) / 2;
				centre += difference * glm::normalize(direction);
			}
		}

		#pragma region DRAW OWN SPHERE
		float X1,Y1,X2,Y2,Z1,Z2;
		float inc1,inc2,inc3,inc4,inc5,Radius1,Radius2;
		int i = 0;

		int resolution = 50;
		vertices.resize(resolution * resolution * 6);

		for(int w = 0; w < resolution; w++) 
		{
			for(int h = (-resolution/2); h < (resolution/2); h++)
			{


				inc1 = (w/(float)resolution)*2*PI;
				inc2 = ((w+1)/(float)resolution)*2*PI;

				inc3 = (h/(float)resolution)*PI;
				inc4 = ((h+1)/(float)resolution)*PI;


				X1 = sin(inc1);
				Y1 = cos(inc1);
				X2 = sin(inc2);
				Y2 = cos(inc2);

				// store the upper and lower radius, remember everything is going to be drawn as triangles
				Radius1 = radius*cos(inc3);
				Radius2 = radius*cos(inc4);

				Z1 = radius*sin(inc3); 
				Z2 = radius*sin(inc4);

				// insert the triangle coordinates
				vertices[i++] = glm::vec3(Radius1*X1,Z1,Radius1*Y1); //+ center;
				vertices[i++] = glm::vec3(Radius1*X2,Z1,Radius1*Y2); //+ center;
				vertices[i++] = glm::vec3(Radius2*X2,Z2,Radius2*Y2); //+ center;

				vertices[i++] = glm::vec3(Radius1*X1,Z1,Radius1*Y1);
				vertices[i++] = glm::vec3(Radius2*X2,Z2,Radius2*Y2);
				vertices[i++] = glm::vec3(Radius2*X1,Z2,Radius2*Y1);


				//indexVBO(v, t, n, indices, indexed_vertices, indexed_uvs, indexed_normals);	 
			}
		}
		#pragma endregion

	}

	bool collides(BoundingSphere* other)
	{
		return (glm::distance(centre, other->centre) < (radius + other->radius));
	}

	void draw()
	{
		glutWireSphere(radius, 25, 25);
	}
};