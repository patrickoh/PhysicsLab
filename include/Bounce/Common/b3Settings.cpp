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

#include "b3Settings.h"

void* b3Alloc(i32 size) {
	return ::malloc(size);
}

void b3Free(void* memory) {
	return ::free(memory);
}

bool b3ExtraSettings::bWarmStart = true;
r32 b3ExtraSettings::timeToSleep = 1.0f;
bool b3ExtraSettings::bApplyDamping = true;
bool b3ExtraSettings::bEnforceFrictionConstraints = true;
bool b3ExtraSettings::bEnforceNonPenetrationContraint = true;
float b3ExtraSettings::baumgarte = 0.2f;
float b3ExtraSettings::sleepAngularTolerance = B3_SLEEP_ANGULAR_TOL;
float b3ExtraSettings::sleepLinearTolerance = B3_SLEEP_LINEAR_TOL;