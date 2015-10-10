#pragma once

#include "Common.h"

struct BoundingSphere
{
	glm::vec3 centre; //in local space
	float radius;

	//TODO - visualisation
	//bool vis;
	//glm::vec4 colour;

	//std::vector<glm::vec3> vertices;

	BoundingSphere(const std::vector<glm::vec3>& v)
	{
		//Ritter's algorithm 

		glm::vec3 vmin, vmax;

		vmin = vmax = v[0];
		int num = v.size();

		for(int i=1; i<num; i++)
		{
			if(v[i].x < vmin.x)
				vmin.x = v[i].x;
			if(v[i].y < vmin.y)
				vmin.y = v[i].y;
			if(v[i].z < vmin.z)
				vmin.z = v[i].z;

			if(v[i].x > vmax.x)
				vmax.x = v[i].x;
			if(v[i].y > vmax.y)
				vmax.y = v[i].y;
			if(v[i].z > vmax.z)
				vmax.z = v[i].z;
		}
		float xdiff = vmax.x - vmin.x;
		float ydiff = vmax.y - vmin.y;
		float zdiff = vmax.z - vmin.y;

		glm::vec3 mid = (vmax+vmin)*(0.5f);

		float max_dist = max(xdiff, max(ydiff, zdiff));


		glm::vec3 c, p;
		float r;

		c = mid;
		r = max_dist/2;
		float r2 = r*r;
		int n = 0;

		for(int i=0; i<num; i++)
		{
			p=v[i];

			glm::vec3 direction = p-c;
			float dist2 = glm::length2(direction);

			if(dist2 > r2)
			{
				float distance = sqrt(dist2);
			
				float diff = distance - r;

				float diameter = 2*r;
				diameter += diff;
				r = diameter/2;
				r2 = r*r;

				diff /= 2;

				c += diff*direction;
			}
		}
	
		centre = c;
		radius = r;

		/*glm::vec3 xmin, xmax, ymin, ymax, zmin, zmax;

        for (glm::vec3 p : v)
        {
            if(p.x < xmin.x) xmin = p;
            if(p.x > xmax.x) xmax = p;
            if(p.y < ymin.y) ymin = p;
            if(p.y > ymax.y) ymax = p;
            if(p.z < zmin.z) zmin = p;
            if(p.z > zmax.z) zmax = p;
        }

		float xSpan = glm::length2(xmax - xmin);
        float ySpan = glm::length2(ymax - ymin);
        float zSpan = glm::length2(zmax - zmin);

        glm::vec3 dia1 = xmin;
        glm::vec3 dia2 = xmax;
        float maxSpan = xSpan;
        if (ySpan > maxSpan)
        {
            maxSpan = ySpan;
            dia1 = ymin; dia2 = ymax;
        }
        if (zSpan > maxSpan)
        {
            dia1 = zmin; dia2 = zmax;
        }
        centre = (dia1 + dia2) * 0.5f;
        float sqRad = glm::length2(dia2 - centre);
		radius = glm::sqrt(sqRad);

        for (glm::vec3 p : v)
        {
            float d = glm::length2(p - centre);
            if(d > sqRad)
            {
				float r = glm::sqrt(d);
                radius = (radius + r) * 0.5f;
                sqRad = radius * radius;
                float offset = r - radius;
                centre = (radius * centre + offset * p) / r;
            }
		}*/

		//glm::vec3 min, max;

		//for (glm::vec3 point : v)
		//{
		//	if (point.x < min.x) min.x = point.x;
		//	if (point.x > max.x) max.x = point.x;
		//	if (point.y < min.y) min.y = point.y;
		//	if (point.y > max.y) max.y = point.y;
		//	if (point.z < min.z) min.z = point.z;
		//	if (point.z > max.z) max.z = point.z;
		//}

		//float xSpan = max.x - min.x;
		//float ySpan = max.y - min.y;
		//float zSpan = max.z - min.y;

		//float maxSpan = xSpan;

		//if (ySpan > maxSpan)
		//	maxSpan = ySpan;

		//if (zSpan > maxSpan)
		//	maxSpan = zSpan;

		//centre = (min+max) * 0.5f;
		//radius = maxSpan / 2;

		//for  (glm::vec3 point : v)
		//{
		//	//float distance = (point - centre).length;
		//	glm::vec3 direction = point - centre;

		//	if(direction.length() > radius)
		//	{
		//		float difference = direction.length() - radius;
		//		radius = (radius + direction.length()) / 2;
		//		centre += difference * glm::normalize(direction);
		//	}
		//}

		#pragma region DRAW OWN SPHERE
		//float X1,Y1,X2,Y2,Z1,Z2;
		//float inc1,inc2,inc3,inc4,inc5,Radius1,Radius2;
		//int i = 0;

		//int resolution = 50;
		//vertices.resize(resolution * resolution * 6);

		//for(int w = 0; w < resolution; w++) 
		//{
		//	for(int h = (-resolution/2); h < (resolution/2); h++)
		//	{


		//		inc1 = (w/(float)resolution)*2*PI;
		//		inc2 = ((w+1)/(float)resolution)*2*PI;

		//		inc3 = (h/(float)resolution)*PI;
		//		inc4 = ((h+1)/(float)resolution)*PI;


		//		X1 = sin(inc1);
		//		Y1 = cos(inc1);
		//		X2 = sin(inc2);
		//		Y2 = cos(inc2);

		//		// store the upper and lower radius, remember everything is going to be drawn as triangles
		//		Radius1 = radius*cos(inc3);
		//		Radius2 = radius*cos(inc4);

		//		Z1 = radius*sin(inc3); 
		//		Z2 = radius*sin(inc4);

		//		// insert the triangle coordinates
		//		vertices[i++] = glm::vec3(Radius1*X1,Z1,Radius1*Y1); //+ center;
		//		vertices[i++] = glm::vec3(Radius1*X2,Z1,Radius1*Y2); //+ center;
		//		vertices[i++] = glm::vec3(Radius2*X2,Z2,Radius2*Y2); //+ center;

		//		vertices[i++] = glm::vec3(Radius1*X1,Z1,Radius1*Y1);
		//		vertices[i++] = glm::vec3(Radius2*X2,Z2,Radius2*Y2);
		//		vertices[i++] = glm::vec3(Radius2*X1,Z2,Radius2*Y1);


		//		//indexVBO(v, t, n, indices, indexed_vertices, indexed_uvs, indexed_normals);	 
		//	}
		//}
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