#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <assert.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 convertAssimpMatrix (aiMatrix4x4 m);
glm::quat formQuaternion(double x, double y, double z, double angle);
glm::vec3 lerp(glm::vec3 v0, glm::vec3 v1, float t);
float lerp(float w0, float w1, float t);
glm::vec3 cubicLerp(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float t);
//betterLerp(v0,v1,t) { return (1-t)*v0 + t*v1; }
void drawText(int x, int y, const char *st);
void dialogue(int x, int y, const char *st);
void decomposeTRS(const glm::mat4& m, glm::vec3& scaling,
        glm::mat4& rotation, glm::vec3& translation);
glm::vec3 decomposeT(glm::mat4 m);
double round(double d);
double vectorSquaredDistance(glm::vec3 v1, glm::vec3 v2);
glm::mat3 setAsCrossProductMatrix( glm::vec3 v );

