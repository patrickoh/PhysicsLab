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

//http://www.geometrictools.com/Documentation/DistancePoint3Triangle3.pdf
glm::vec3 closestPointOnTriangle( const std::vector<glm::vec3> triangle, const glm::vec3 sourcePosition )
{
	glm::vec3 edge0 = triangle[1] - triangle[0];
	glm::vec3 edge1 = triangle[2] - triangle[0];
	glm::vec3 v0 = triangle[0] - sourcePosition;

	float a = glm::dot(edge0, edge0);
	float b = glm::dot(edge0, edge1);
	float c = glm::dot(edge1, edge1);
	float d = glm::dot(edge0, v0);
	float e = glm::dot(edge1, v0);

	float det = a*c - b*b;
	float s = b*e - c*d;
	float t = b*d - a*e;

	if ( s + t < det )
	{
		if ( s < 0.f )
		{
			if ( t < 0.f )
			{
				if ( d < 0.f )
				{
					s = clamp( -d/a, 0.f, 1.f );
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = clamp( -e/c, 0.f, 1.f );
				}
			}
			else
			{
				s = 0.f;
				t = clamp( -e/c, 0.f, 1.f );
			}
		}
		else if ( t < 0.f )
		{
			s = clamp( -d/a, 0.f, 1.f );
			t = 0.f;
		}
		else
		{
			float invDet = 1.f / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if ( s < 0.f )
		{
			float tmp0 = b+d;
			float tmp1 = c+e;
			if ( tmp1 > tmp0 )
			{
				float numer = tmp1 - tmp0;
				float denom = a-2*b+c;
				s = clamp( numer/denom, 0.f, 1.f );
				t = 1-s;
			}
			else
			{
				t = clamp( -e/c, 0.f, 1.f );
				s = 0.f;
			}
		}
		else if ( t < 0.f )
		{
			if ( a+d > b+e )
			{
				float numer = c+e-b-d;
				float denom = a-2*b+c;
				s = clamp( numer/denom, 0.f, 1.f );
				t = 1-s;
			}
			else
			{
				s = clamp( -e/c, 0.f, 1.f );
				t = 0.f;
			}
		}
		else
		{
			float numer = c+e-b-d;
			float denom = a-2*b+c;
			s = clamp( numer/denom, 0.f, 1.f );
			t = 1.f - s;
		}
	}

	return triangle[0] + s * edge0 + t * edge1;
}

//http://www.gamedev.net/topic/473207-clamping-a-value-to-a-range-in-c-quickly/#entry4105200
float clamp(float x, float a, float b)
{
	return x < a ? a : (x > b ? b : x);
}

// Compute barycentric coordinates/weights for
// point p with respect to triangle (a, b, c)
// (From Christer Ericson's Real-Time Collision Detection)
glm::vec3 barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) 
{
    glm::vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);
	float denom = (d00 * d11 - d01 * d01);
	glm::vec3 bary;
	bary.y = (d11 * d20 - d01 * d21) / denom; //v
    bary.z = (d00 * d21 - d01 * d20) / denom; //w
    bary.x = 1.f - bary.y - bary.z; //u
   
	return bary;
}

//https://github.com/tangrams/glmTools/
std::vector<glm::vec3> getConvexHull(std::vector<glm::vec3> &pts){
    std::vector<glm::vec3> hull;
    glm::vec3 h1,h2,h3;
    
    if (pts.size() < 3) {
        //std::cout << "Error: you need at least three points to calculate the convex hull" << std::endl;
        return hull;
    }
    
    std::sort(pts.begin(), pts.end(), &lexicalComparison);
    
    hull.push_back(pts.at(0));
    hull.push_back(pts.at(1));
    
    int currentPoint = 2;
    int direction = 1;
    
    for (int i=0; i<3000; i++) { //max 1000 tries
        
        hull.push_back(pts.at(currentPoint));
        
        // look at the turn direction in the last three points
        h1 = hull.at(hull.size()-3);
        h2 = hull.at(hull.size()-2);
        h3 = hull.at(hull.size()-1);
        
        // while there are more than two points in the hull
        // and the last three points do not make a right turn
        while (!isRightTurn(h1, h2, h3) && hull.size() > 2) {
            
            // remove the middle of the last three points
            hull.erase(hull.end() - 2);
            
            if (hull.size() >= 3) {
                h1 = hull.at(hull.size()-3);
            }
            h2 = hull.at(hull.size()-2);
            h3 = hull.at(hull.size()-1);
        }
        
        // going through left-to-right calculates the top hull
        // when we get to the end, we reverse direction
        // and go back again right-to-left to calculate the bottom hull
        if (currentPoint == pts.size() -1 || currentPoint == 0) {
            direction = direction * -1;
        }
        
        currentPoint+=direction;
        
        if (hull.front()==hull.back()) {
            if(currentPoint == 3 && direction == 1){
                currentPoint = 4;
            } else {
                break;
            }
        }
    }
    
    return hull;
}

bool lexicalComparison(const glm::vec3 &_v1, const glm::vec3 &_v2) {
    if (_v1.x > _v2.x) return true;
    else if (_v1.x < _v2.x) return false;
    else if (_v1.y > _v2.y) return true;
    else return false;
}

bool isRightTurn(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c) {
    // use the cross product to determin if we have a right turn
    return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

bool isSameDirection(glm::vec3 v1, glm::vec3 v2)
{
	return glm::dot(v1, v2) > 0.0f;
}

GLuint loadTexture(const char* fileName) 
{		
	Magick::Blob blob;
	Magick::Image* image; 

	std::string stringFileName(fileName);
	std::string fullPath = "Textures/" + stringFileName;

	try {
		image = new Magick::Image(fullPath.c_str());
		image->write(&blob, "RGBA");
	}
	catch (Magick::Error& Error) {
		std::cout << "Error loading texture '" << fullPath << "': " << Error.what() << std::endl;

		delete image;
		return false;
	}

	GLuint textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
			
	//Load the image data in to the texture
	glTexImage2D(GL_TEXTURE_2D, 0/*LOD*/, GL_RGBA, image->columns(), image->rows(), 0/*BORDER*/, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());

	//Parameter stuff, for magnifying texture etc.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
			
	glBindTexture(GL_TEXTURE_2D, 0); 

	delete image;  
	return textureID;
}

glm::vec2 randomPointOnCircumference(float radius)
{
	//float angle = glm::linearRand(0.0f, 1.0f) * 3.14f * 2;
	//return glm::vec2(glm::cos(angle)*radius, glm::sin(angle)*radius);

	return glm::circularRand(radius);
}

void toStringStream(glm::vec3 v, std::stringstream& ss)
{
	ss << "(" << std::fixed << std::setprecision(PRECISION) << v.x << ", " << v.y << ", " << v.z << ")";
}