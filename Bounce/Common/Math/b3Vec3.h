/*
* Copyright (c) 2015-2015 Irlan Robson http://www.irlans.wordpress.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __B3_VEC3_H__
#define __B3_VEC3_H__

#include "..\b3Settings.h"

// Compute the length of a vector.
inline r32 b3Len(const glm::vec3& v) {
	return b3Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Compute the squared length of a vector.
inline r32 b3LenSq(const glm::vec3& v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

// Compute the dot-product of two vectors.
inline r32 b3Dot(const glm::vec3& a, const glm::vec3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Compute the cross-product of two vectors.
inline glm::vec3 b3Cross(const glm::vec3& a, const glm::vec3& b) {
	return glm::vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// Computed the normalized vector of a (non-zero!) vector.
inline glm::vec3 b3Normalize(const glm::vec3& v) {
	//@warning the vector must be normalized.
	r32 invLen = B3_ONE / b3Len(v);
	return invLen * v;
}

// Create a basis matrix given a vector.
inline void b3ComputeBasis(const glm::vec3& a, glm::vec3& b, glm::vec3& c) {
	// From Box2D.
	// Suppose vector a has all equal components and is a unit vector: a = (s, s, s)
	// Then 3*s*s = 1, s = sqrt(1/3) = 0.57735. This means that at least one component of a
	// unit vector must be greater or equal to 0.57735.
	if ( b3Abs(a.x) >= r32(0.57735027) ) {
		b = glm::vec3(a.y, -a.x, B3_ZERO);
	}
	else {
		b = glm::vec3(B3_ZERO, a.z, -a.y);
	}

	b = b3Normalize(b);
	c = b3Cross(a, b);
}

#endif
