/**********************************************************************************************
*
*   [physac] raylib physics module - Basic functions to apply physics to 2D objects
*
*   Copyright (c) 2016 Victor Fisac and Ramon Santamaria
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

#ifndef PHYSAC_H
#define PHYSAC_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for PHYSAC_STANDALONE usage
//----------------------------------------------------------------------------------

// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef enum { COLLIDER_CIRCLE, COLLIDER_RECTANGLE } ColliderType;

typedef struct Transform {
    Vector2 position;
    float rotation;         // Radians (not used)
    Vector2 scale;          // Just for rectangle physic objects, for circle physic objects use collider radius and keep scale as { 0, 0 }
} Transform;

typedef struct Rigidbody {
    bool enabled;           // Acts as kinematic state (collisions are calculated anyway)
    float mass;
    Vector2 acceleration;
    Vector2 velocity;
    bool applyGravity;
    bool isGrounded;
    float friction;         // Normalized value
    float bounciness;
} Rigidbody;

typedef struct Collider {
    bool enabled;
    ColliderType type;
    Rectangle bounds;       // Used for COLLIDER_RECTANGLE
    int radius;             // Used for COLLIDER_CIRCLE
} Collider;

typedef struct PhysicObjectData {
    unsigned int id;
    Transform transform;
    Rigidbody rigidbody;
    Collider collider;
    bool enabled;
} PhysicObjectData, *PhysicObject;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void InitPhysics(Vector2 gravity);                                                      // Initializes pointers array (just pointers, fixed size)
void UpdatePhysics();                                                                   // Update physic objects, calculating physic behaviours and collisions detection
void ClosePhysics();                                                                    // Unitialize all physic objects and empty the objects pool

PhysicObject CreatePhysicObject(Vector2 position, float rotation, Vector2 scale);       // Create a new physic object dinamically, initialize it and add to pool
void DestroyPhysicObject(PhysicObject pObj);                                            // Destroy a specific physic object and take it out of the list

void ApplyForce(PhysicObject pObj, Vector2 force);                                      // Apply directional force to a physic object
void ApplyForceAtPosition(Vector2 position, float force, float radius);                 // Apply radial force to all physic objects in range

Rectangle TransformToRectangle(Transform transform);                                    // Convert Transform data type to Rectangle (position and scale)
void DrawPhysicObjectInfo(PhysicObject pObj, Vector2 position, int fontSize);           // Draw physic object information at screen position

#ifdef __cplusplus
}
#endif

#endif // PHYSAC_H
