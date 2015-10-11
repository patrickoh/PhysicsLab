#pragma once

#include "Common.h"

struct BoundingSphere
{
	glm::vec3 centre; //in local space
	float radius;

	//TODO - visualisation
	//bool vis;
	
	glm::vec4 colour;

	//std::vector<glm::vec3> vertices;

	BoundingSphere(const std::vector<glm::vec3>& v)
	{
		calculate3(v);

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

	//Ritter's (Sarah)
	void calculate1(const std::vector<glm::vec3>& v)
	{
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
	}

	//Ritter's Unity Forums
	void calculate2(const std::vector<glm::vec3>& v)
	{
		glm::vec3 xmin, xmax, ymin, ymax, zmin, zmax;

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
		}
	}

	//Really naive approach that doesn't even work!
	void calculate3(const std::vector<glm::vec3>& v)
	{
		glm::vec3 p1, p2;
		float diameter = 0.0f;

		for(int i = 0; i < v.size(); i++)
		{
			for(int j = 0; j < v.size(); j++)
			{
				if(glm::distance(v[i], v[j]) > diameter)
				{
					diameter = glm::distance(v[i], v[j]);
					p1 = v[i];
					p2 = v[j];
				}
			}
		}

		centre = (p1 + p2) * 0.5f;
		radius = diameter * 0.5f;
	}

	//Bouncing Bubble Algorithm
	void calculate4(const std::vector<glm::vec3>& vertices)
	{
		centre = vertices[0];
		radius = 0.0001f;

		glm::vec3 pos, diff;
		float len, alpha, alphaSq;

		for (int i = 0; i < 2; i++)
		{
			for (int i = 0; i < vertices.size(); i++)
			{
				pos = vertices[i];
				diff = pos - centre;
				len = diff.length();
				
				if (len > radius)
				{
					alpha = len / radius;
					alphaSq = alpha * alpha;
					radius = 0.5f * (alpha + 1 / alpha) * radius;
					centre = 0.5f * ((1 + 1 / alphaSq) * centre + (1 - 1 / alphaSq) * pos);
				}
			}
		}

		for (int i = 0; i < vertices.size(); i++)
		{
			pos = vertices[i];
			diff = pos - centre;
			len = diff.length();
			
			if (len > radius)
			{
				radius = (radius + len) / 2.0f;
				centre = centre + ((len - radius) / len * diff);
			}
		}
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