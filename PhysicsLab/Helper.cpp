#include "Helper.h"

bool disableText = false;

glm::quat formQuaternion(double x, double y, double z, double angle)
{
	glm::quat out;

	//x, y, and z form a normalized vector which is now the axis of rotation.
	out.w  = cosf( angle/2);
	out.x = x * sinf( angle/2 );
	out.y = y * sinf( angle/2 );
	out.z = z * sinf( angle/2 );
	return out;
}

glm::vec3 lerp(glm::vec3 v0, glm::vec3 v1, float t) 
{
	return v0 + t*(v1-v0);
}

float lerp(float w0, float w1, float t)
{
	return w0 + t*(w1-w0);
}

glm::vec3 cubicLerp(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float t)
{
	glm::vec3 a0, a1, a2, a3;
	
	float t2 = t*t;
	a0 = v3 - v2 - v0 + v1;
	a1 = v0 - v1 - a0;
	a2 = v2 - v0;
	a3 = v1;

	return (a0*t*t2 + a1*t2 + a2*t + a3);
}

//draw text in screenspace
void drawText(int x, int y, const char *st)
{
	int l,i;

	l=strlen(st); // see how many characters are in text string.

	glWindowPos2i(x, y); // location to start printing text
	
	for(i=0; i < l; i++) // loop until i is greater then l
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, st[i]);
	}
}

glm::mat4 convertAssimpMatrix(aiMatrix4x4 from)
{
	 glm::mat4 to;

	 to[0][0] = (GLfloat)from.a1; to[1][0] = (GLfloat)from.a2;
	 to[2][0] = (GLfloat)from.a3; to[3][0] = (GLfloat)from.a4;
	 to[0][1] = (GLfloat)from.b1; to[1][1] = (GLfloat)from.b2;
	 to[2][1] = (GLfloat)from.b3; to[3][1] = (GLfloat)from.b4;
	 to[0][2] = (GLfloat)from.c1; to[1][2] = (GLfloat)from.c2;
	 to[2][2] = (GLfloat)from.c3; to[3][2] = (GLfloat)from.c4;
	 to[0][3] = (GLfloat)from.d1; to[1][3] = (GLfloat)from.d2;
	 to[2][3] = (GLfloat)from.d3; to[3][3] = (GLfloat)from.d4;

	 return to;
}

/**
 * Decomposes matrix M such that T * R * S = M, where T is translation matrix,
 * R is rotation matrix and S is scaling matrix.
 * http://code.google.com/p/assimp-net/source/browse/trunk/AssimpNet/Matrix4x4.cs
 * (this method is exact to at least 0.0001f)
 *
 * | 1  0  0  T1 | | R11 R12 R13 0 | | a 0 0 0 |   | aR11 bR12 cR13 T1 |
 * | 0  1  0  T2 |.| R21 R22 R23 0 |.| 0 b 0 0 | = | aR21 bR22 cR23 T2 |
 * | 0  0  0  T3 | | R31 R32 R33 0 | | 0 0 c 0 |   | aR31 bR32 cR33 T3 |
 * | 0  0  0   1 | |  0   0   0  1 | | 0 0 0 1 |   |  0    0    0    1 |
 *
 * @param m (in) matrix to decompose
 * @param scaling (out) scaling vector
 * @param rotation (out) rotation matrix
 * @param translation (out) translation vector
 */
void decomposeTRS(const glm::mat4& m, glm::vec3& translation, glm::mat4& rotation, glm::vec3& scaling)
{
    // Extract the translation
    translation.x = m[3][0];
    translation.y = m[3][1];
    translation.z = m[3][2];

    // Extract col vectors of the matrix
    glm::vec3 col1(m[0][0], m[0][1], m[0][2]);
    glm::vec3 col2(m[1][0], m[1][1], m[1][2]);
    glm::vec3 col3(m[2][0], m[2][1], m[2][2]);

    //Extract the scaling factors
    scaling.x = glm::length(col1);
    scaling.y = glm::length(col2);
    scaling.z = glm::length(col3);

    // Handle negative scaling
    if (glm::determinant(m) < 0) {
        scaling.x = -scaling.x;
        scaling.y = -scaling.y;
        scaling.z = -scaling.z;
    }

    // Remove scaling from the matrix
    if (scaling.x != 0) {
        col1 /= scaling.x;
    }

    if (scaling.y != 0) {
        col2 /= scaling.y;
    }

    if (scaling.z != 0) {
        col3 /= scaling.z;
    }

    rotation[0][0] = col1.x;
    rotation[0][1] = col1.y;
    rotation[0][2] = col1.z;
    rotation[0][3] = 0.0;

    rotation[1][0] = col2.x;
    rotation[1][1] = col2.y;
    rotation[1][2] = col2.z;
    rotation[1][3] = 0.0;

    rotation[2][0] = col3.x;
    rotation[2][1] = col3.y;
    rotation[2][2] = col3.z;
    rotation[2][3] = 0.0;

    rotation[3][0] = 0.0;
    rotation[3][1] = 0.0;
    rotation[3][2] = 0.0;
    rotation[3][3] = 1.0;
}

double round(double d)
{
	return floor(d + 0.5);
}

glm::vec3 decomposeT(glm::mat4 m)
{
	glm::vec3 translation;

	translation.x = m[3][0];
    translation.y = m[3][1];
    translation.z = m[3][2];

	return translation;
}

glm::mat4 setAsCrossProductMatrix( glm::vec3 w )
{
	return glm::transpose(glm::mat4(0, -w.z,  w.y,   0,
					w.z,    0,  -w.x, 0,
					-w.y,  w.x,    0, 0,
					0 ,0 , 0,         1));
}

// w is equal to angular_velocity*time_between_frames
glm::quat quatFromAngularVelocityByTimestep(glm::vec3 w)
{
    const float x = w[0];
    const float y = w[1];
    const float z = w[2];

    const float angle = sqrt(x*x + y*y + z*z);  //module of angular velocity

    if (angle > 0.0) //the formulas from the link
    {
		return glm::quat(x*sin(angle/2.0f)/angle,
			y*sin(angle/2.0f)/angle,
			z*sin(angle/2.0f)/angle,
			cos(angle/2.0f));
    }else    //to avoid illegal expressions
    {
        return glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

glm::vec3 GetOGLPos(int x, int y, int WINDOW_WIDTH, int WINDOW_HEIGHT, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	glm::vec4 viewport = glm::vec4(0,0,WINDOW_WIDTH, WINDOW_HEIGHT);
	float fixY = WINDOW_HEIGHT - y; //0,0 in OpenGL is bottom left

	GLfloat z; 
	glReadPixels( x, fixY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z ); //get z position in depth buffer
	
	return glm::unProject(glm::vec3(x, fixY, z), viewMatrix, projectionMatrix, viewport);
}

glm::vec3 doubleCross(const glm::vec3& v1, const glm::vec3& v2)
{
	return glm::cross(glm::cross(v1, v2), v1);
}

//Assuming convex poly
bool isCCW(glm::vec3 adjEdge1, glm::vec3 adjEdge2)
{
	return glm::dot(glm::cross(adjEdge1, adjEdge2), glm::vec3(0,0,1)) > 0;
}