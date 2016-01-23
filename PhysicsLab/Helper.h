#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Magick++.h"
#include <iostream>

glm::mat4 convertAssimpMatrix (aiMatrix4x4 m);
glm::quat formQuaternion(double x, double y, double z, double angle);
glm::vec3 lerp(glm::vec3 v0, glm::vec3 v1, float t);
float lerp(float w0, float w1, float t);
glm::vec3 cubicLerp(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float t);
//betterLerp(v0,v1,t) { return (1-t)*v0 + t*v1; }
void drawText(int x, int y, const char *st);
void decomposeTRS(const glm::mat4& m, glm::vec3& scaling,
        glm::mat4& rotation, glm::vec3& translation);
glm::vec3 decomposeT(glm::mat4 m);
double round(double d);
double vectorSquaredDistance(glm::vec3 v1, glm::vec3 v2);
glm::mat4 setAsCrossProductMatrix( glm::vec3 v );
glm::quat quatFromAngularVelocityByTimestep(glm::vec3 w);
glm::vec3 GetOGLPos(int x, int y, int WINDOW_WIDTH, int WINDOW_HEIGHT, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
glm::vec3 doubleCross(const glm::vec3& v1, const glm::vec3& v2);
bool isCCW(glm::vec3 adjEdge1, glm::vec3 adjEdge2);
glm::vec3 closestPointOnTriangle( const std::vector<glm::vec3> triangle, const glm::vec3 sourcePosition);
float clamp(float x, float a, float b);
glm::vec3 barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);
std::vector<glm::vec3> getConvexHull(std::vector<glm::vec3> &_pts);
bool lexicalComparison(const glm::vec3 &_v1, const glm::vec3 &_v2);
bool isRightTurn(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
bool isSameDirection(glm::vec3 v1, glm::vec3 v2);
GLuint loadTexture(const char* fileName);
