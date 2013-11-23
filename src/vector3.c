/*********************************************************************************************
*
*   raylib.vector3
*
*   Vector3 Functions Definition
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*    
*   This software is provided "as-is", without any express or implied warranty. In no event 
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial 
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you 
*     wrote the original software. If you use this software in a product, an acknowledgment 
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "vector3.h"

#include <math.h>

// Add two vectors
Vector3 VectorAdd(Vector3 v1, Vector3 v2)
{
    Vector3 out;

    out.x = v1.x + v2.x;
    out.y = v1.y + v2.y;
    out.z = v1.z + v2.z;
    
    return out;
}

// Substract two vectors
Vector3 VectorSubtract(Vector3 v1, Vector3 v2)
{
    Vector3 out;

    out.x = v1.x - v2.x;
    out.y = v1.y - v2.y;
    out.z = v1.z - v2.z;
    
    return out;
}

// Calculate two vectors cross product
Vector3 VectorCrossProduct(Vector3 v1, Vector3 v2)
{
    Vector3 cross;

    cross.x = v1.y*v2.z - v1.z*v2.y;
    cross.y = v1.z*v2.x - v1.x*v2.z;
    cross.z = v1.x*v2.y - v1.y*v2.x;
    
    return cross;
}

// Calculate one vector perpendicular vector
Vector3 VectorPerpendicular(Vector3 v) 
{
    Vector3 out;
    
    float min = fabs(v.x);
    Vector3 cardinalAxis = {1.0, 0.0, 0.0};
 
    if (fabs(v.y) < min) 
    {
        min = fabs(v.y);
        cardinalAxis = (Vector3){0.0, 1.0, 0.0};
    }
 
    if(fabs(v.z) < min) 
    {
        cardinalAxis = (Vector3){0.0, 0.0, 1.0};
    }
    
    out = VectorCrossProduct(v, cardinalAxis);
 
    return out;
}

// Calculate two vectors dot product
float VectorDotProduct(Vector3 v1, Vector3 v2)
{
    float dot;
    
    dot = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    
    return dot;
}

// Calculate vector lenght
float VectorLength(const Vector3 v)
{
    float length;
    
    length = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    
    return length;
}

// Scale provided vector
void VectorScale(Vector3 *v, float scale)
{
    v->x *= scale;
    v->y *= scale;
    v->z *= scale;
}

// Invert provided vector (direction)
void VectorInverse(Vector3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
}

// Normalize provided vector
void VectorNormalize(Vector3 *v)
{
    float length, ilength;

    length = VectorLength(*v);
    
    if (length == 0) length = 1;

    ilength = 1.0/length;
    
    v->x *= ilength;
    v->y *= ilength;
    v->z *= ilength;
}
