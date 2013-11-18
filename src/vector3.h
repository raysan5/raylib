/*********************************************************************************************
* 
*	raylib.vector3
*	
*	Some useful functions to work with Vector3
* 
*	Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*	
*	This software is provided "as-is", without any express or implied warranty. In no event 
*	will the authors be held liable for any damages arising from the use of this software.
*
*	Permission is granted to anyone to use this software for any purpose, including commercial 
*	applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*	  1. The origin of this software must not be misrepresented; you must not claim that you 
*	  wrote the original software. If you use this software in a product, an acknowledgment 
*	  in the product documentation would be appreciated but is not required.
*
*	  2. Altered source versions must be plainly marked as such, and must not be misrepresented
*	  as being the original software.
*
*	  3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef VECTOR3_H
#define VECTOR3_H

#include "raylib.h"		// Defines Vector3 structure

#ifdef __cplusplus
extern "C" {			// Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Global Variables Definition
//------------------------------------------------------------------------------------
// It's lonely here...

//------------------------------------------------------------------------------------
// Functions Declaration to work with Vector3
//------------------------------------------------------------------------------------
Vector3 VectorAdd(Vector3 v1, Vector3 v2);				// Add two vectors
Vector3 VectorSubtract(Vector3 v1, Vector3 v2);			// Substract two vectors
Vector3 VectorCrossProduct(Vector3 v1, Vector3 v2);		// Calculate two vectors cross product
Vector3 VectorPerpendicular(Vector3 v);					// Calculate one vector perpendicular vector
float VectorDotProduct(Vector3 v1, Vector3 v2);			// Calculate two vectors dot product
float VectorLength(const Vector3 v);					// Calculate vector lenght
void VectorScale(Vector3 *v, float scale);				// Scale provided vector
void VectorInverse(Vector3 *v);							// Invert provided vector (direction)
void VectorNormalize(Vector3 *v);						// Normalize provided vector

#ifdef __cplusplus
}
#endif

#endif // VECTOR3_H