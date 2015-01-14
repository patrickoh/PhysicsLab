#pragma once

#define MAX_BONES 32 
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define PRECISION 3

#define LETTER_WIDTH 10

#define MAX_ANIMATIONS 5

#define HAND "Models/hand_with_animation.dae"
#define CUBE "Models/cubeTri.obj"
#define BOB "Models/boblampclean.md5mesh"
#define CONES "Models/Cones3.dae"
#define IZANUGI "Models/izanugi.DAE"
#define SQUALL "Models/Squall.DAE"
#define RIKKU "Models/Rikku.DAE"

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