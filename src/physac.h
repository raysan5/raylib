/**********************************************************************************************
*
*   [physac] raylib physics engine module - Basic functions to apply physics to 2D objects
*
*   Copyright (c) 2015 Victor Fisac and Ramon Santamaria
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
//----------------------------------------------------------------------------------
typedef enum { RectangleCollider, CircleCollider } ColliderType;

// Physics struct
typedef struct Physics {
    bool enabled;
    bool debug;     // Should be used by programmer for testing purposes
    Vector2 gravity;
} Physics;

// Transform struct
typedef struct Transform {
    Vector2 position;
    float rotation;
    Vector2 scale;
} Transform;

// Rigidbody struct
typedef struct Rigidbody {
    bool enabled;
    float mass;
    Vector2 acceleration;
    Vector2 velocity;
    bool isGrounded;
    bool isContact;     // Avoid freeze player when touching floor
    bool applyGravity;
    float friction;     // 0.0f to 1.0f
    float bounciness;   // 0.0f to 1.0f
} Rigidbody;

// Collider struct
typedef struct Collider {
    bool enabled;
    ColliderType type;
    Rectangle bounds;   // Just used for RectangleCollider type
    int radius;     // Just used for CircleCollider type
} Collider;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declarations
//----------------------------------------------------------------------------------
void InitPhysics();                                                         // Initialize all internal physics values
void SetPhysics(Physics settings);                                          // Set physics settings values using Physics data type to overwrite internal physics settings

void AddRigidbody(int index, Rigidbody rigidbody);                          // Initialize a new rigidbody with parameters to internal index slot
void AddCollider(int index, Collider collider);                             // Initialize a new Collider with parameters to internal index slot

void ApplyPhysics(int index, Vector2 *position);                            // Apply physics to internal rigidbody, physics calculations are applied to position pointer parameter
void SetRigidbodyEnabled(int index, bool state);                            // Set enabled state to a defined rigidbody
void SetRigidbodyVelocity(int index, Vector2 velocity);                     // Set velocity of rigidbody (without considering of mass value)
void SetRigidbodyAcceleration(int index, Vector2 acceleration);             // Set acceleration of rigidbody (without considering of mass value)
void AddRigidbodyForce(int index, Vector2 force);                           // Set rigidbody force (considering mass value)
void AddForceAtPosition(Vector2 position, float intensity, float radius);   // Add a force to all enabled rigidbodies at a position

void SetColliderEnabled(int index, bool state);                             // Set enabled state to a defined collider

Rigidbody GetRigidbody(int index);                                          // Returns the internal rigidbody data defined by index parameter
Collider GetCollider(int index);                                            // Returns the internal collider data defined by index parameter

#ifdef __cplusplus
}
#endif

#endif // PHYSAC_H
