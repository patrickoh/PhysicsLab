#pragma once

#define MAX_BONES 32 
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define PRECISION 3

#define LETTER_WIDTH 10

#define BOX "Models/cubeTri.obj"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

enum DKEY
{
	Right = 0, Left, Up, Down,
};