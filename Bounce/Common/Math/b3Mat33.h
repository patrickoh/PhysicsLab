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

#ifndef __B3_MAT33_H__
#define __B3_MAT33_H__

#include "b3Vec3.h"

// A column-major 3x3 matrix.
struct b3Mat33 {
	b3Mat33() { }

	b3Mat33(const glm::vec3& _x, const glm::vec3& _y, const glm::vec3& _z) : 
		x(_x), 
		y(_y), 
		z(_z) {
	}

	// A = A + B.
	void operator+=(const b3Mat33& B) {
		x += B.x;
		y += B.y;
		z += B.z;
	}

	// Get a column vector of this matrix.
	const glm::vec3& operator[](u32 i) const {
		return (&x)[i];
	}
	
	// Get a column vector of this matrix.
	glm::vec3& operator[](u32 i) {
		return (&x)[i];
	}

	// Set this matrix to the zero matrix.
	void SetZero() {
		x = glm::vec3(0);//.SetZero();
		y = glm::vec3(0);
		z = glm::vec3(0);
	}

	// Set this matrix to the identity matrix.
	void SetIdentity() {
		x = glm::vec3(B3_ONE, B3_ZERO, B3_ZERO);
		y = glm::vec3(B3_ZERO, B3_ONE, B3_ZERO);
		z = glm::vec3(B3_ZERO, B3_ZERO, B3_ONE);
	}

	glm::vec3 x, y, z;
};

// Compute C = A + B.
inline b3Mat33 operator+(const b3Mat33& A, const b3Mat33& B) {
	return b3Mat33(A.x + B.x, A.y + B.y, A.z + B.z);
}

// Compute u = Av.
inline glm::vec3 operator*(const b3Mat33& A, const glm::vec3& v) {
	return v.x * A.x + v.y * A.y + v.z * A.z;
}

// Compute C = AB.
inline b3Mat33 operator*(const b3Mat33& A, const b3Mat33& B) {
	return b3Mat33(A * B.x, A * B.y, A * B.z);
}

// Compute the transpose of a 3x3 matrix (A^T).
inline b3Mat33 b3Transpose(const b3Mat33& A) {
	return b3Mat33(
		glm::vec3(A.x.x, A.y.x, A.z.x),
		glm::vec3(A.x.y, A.y.y, A.z.y),
		glm::vec3(A.x.z, A.y.z, A.z.z)
		);
}

// Compute the transpose of a 3x3 matrix (A^T).
inline b3Mat33 b3Diagonal(r32 s) {
	return b3Mat33(
		glm::vec3(s, B3_ZERO, B3_ZERO),
		glm::vec3(B3_ZERO, s, B3_ZERO),
		glm::vec3(B3_ZERO, B3_ZERO, s)
		);
}

// Compute the inverse of a 3x3 matrix (A^-1).
inline b3Mat33 b3Inverse(const b3Mat33& A) {
	// Return the zero matrix if isn't invertable.
	glm::vec3 c1 = b3Cross(A.x, A.y);
	glm::vec3 c2 = b3Cross(A.y, A.z);
	glm::vec3 c3 = b3Cross(A.z, A.x);

	r32 det = b3Dot(c1, A.z);
	if (det != B3_ZERO) {
		det = B3_ONE / det;
	}

	b3Mat33 B;

	B.x.x = det * c2.x;
	B.x.y = det * c3.x;
	B.x.z = det * c1.x;

	B.y.x = det * c2.y;
	B.y.y = det * c3.y;
	B.y.z = det * c1.y;

	B.z.x = det * c2.z;
	B.z.y = det * c3.z;
	B.z.z = det * c1.z;

	return B;
}

// Compute a skew-symmetric matrix given a vector.
inline b3Mat33 b3SkewSymmetric(const glm::vec3& v) {
	b3Mat33 A;

	A.x.x = B3_ZERO;
	A.x.y = v.z;
	A.x.z = -v.y;

	A.y.x = -v.z;
	A.y.y = B3_ZERO;
	A.y.z = v.x;

	A.z.x = v.y;
	A.z.y = -v.x;
	A.z.z = B3_ZERO;

	return A;
}

#endif
