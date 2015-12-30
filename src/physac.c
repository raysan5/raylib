/**********************************************************************************************
*
*   raylib physics engine module - Basic functions to apply physics to 2D objects
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

//#define PHYSAC_STANDALONE     // NOTE: To use the physics module as standalone lib, just uncomment this line

#if defined(PHYSAC_STANDALONE)
    #include "physac.h"
#else
    #include "raylib.h"
#endif

#include <math.h>
#include <stdio.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_ELEMENTS 1024       // Stored rigidbodies and colliders array length
#define DECIMAL_FIX 0.01f       // Decimal margin for collision checks (avoid rigidbodies shake)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Physics physics;
static Collider colliders[MAX_ELEMENTS];
static Rigidbody rigidbodies[MAX_ELEMENTS];
static bool collisionChecker = false;

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void InitPhysics()
{    
    for (int i = 0; i < MAX_ELEMENTS; i++)
    {
        rigidbodies[i].enabled = false;
        rigidbodies[i].mass = 0.0f;
        rigidbodies[i].velocity = (Vector2){0, 0};
        rigidbodies[i].acceleration = (Vector2){0, 0};
        rigidbodies[i].isGrounded = false;
        rigidbodies[i].isContact = false;
        rigidbodies[i].friction = 0.0f;
        
        colliders[i].enabled = false;
        colliders[i].bounds = (Rectangle){0, 0, 0, 0};
        colliders[i].radius = 0;
    }
}

void SetPhysics(Physics settings)
{
    physics = settings;
    
    // To get good results, gravity needs to be 1:10 from original parameter
    physics.gravity = (Vector2){physics.gravity.x / 10, physics.gravity.y / 10};
}

void AddCollider(int index, Collider collider)
{
    colliders[index] = collider;
}

void AddRigidbody(int index, Rigidbody rigidbody)
{
    rigidbodies[index] = rigidbody;
}

void ApplyPhysics(int index, Vector2 *position)
{
    if (rigidbodies[index].enabled)
    {
        // Apply gravity
        rigidbodies[index].velocity.y += rigidbodies[index].acceleration.y;
        rigidbodies[index].velocity.x += rigidbodies[index].acceleration.x;
        
        rigidbodies[index].velocity.y += physics.gravity.y;
        rigidbodies[index].velocity.x += physics.gravity.x;
        
        // Apply friction to velocity
        if (rigidbodies[index].isGrounded)
        {
            if (rigidbodies[index].velocity.x > DECIMAL_FIX)
            {
                rigidbodies[index].velocity.x -= rigidbodies[index].friction;
            }
            else if (rigidbodies[index].velocity.x < -DECIMAL_FIX)
            {
                rigidbodies[index].velocity.x += rigidbodies[index].friction;
            }
            else
            {
                rigidbodies[index].velocity.x = 0;
            }
        }
        
        if (rigidbodies[index].velocity.y > DECIMAL_FIX)
        {
            rigidbodies[index].velocity.y -= rigidbodies[index].friction;
        }
        else if (rigidbodies[index].velocity.y < -DECIMAL_FIX)
        {
            rigidbodies[index].velocity.y += rigidbodies[index].friction;
        }
        else
        {
            rigidbodies[index].velocity.y = 0;
        }
        
        // Apply friction to acceleration
        if (rigidbodies[index].isGrounded)
        {
            if (rigidbodies[index].acceleration.x > DECIMAL_FIX)
            {
                rigidbodies[index].acceleration.x -= rigidbodies[index].friction;
            }
            else if (rigidbodies[index].acceleration.x < -DECIMAL_FIX)
            {
                rigidbodies[index].acceleration.x += rigidbodies[index].friction;
            }
            else
            {
                rigidbodies[index].acceleration.x = 0;
            }
        }
        
        if (rigidbodies[index].acceleration.y > DECIMAL_FIX)
        {
            rigidbodies[index].acceleration.y -= rigidbodies[index].friction;
        }
        else if (rigidbodies[index].acceleration.y < -DECIMAL_FIX)
        {
            rigidbodies[index].acceleration.y += rigidbodies[index].friction;
        }
        else
        {
            rigidbodies[index].acceleration.y = 0;
        }
        
        // Update position vector
        position->x += rigidbodies[index].velocity.x;        
        position->y -= rigidbodies[index].velocity.y;
        
        // Update collider bounds
        colliders[index].bounds.x = position->x;
        colliders[index].bounds.y = position->y;
        
        // Check collision with other colliders
        collisionChecker = false;
        rigidbodies[index].isContact = false;
        for (int j = 0; j < MAX_ELEMENTS; j++)
        {
            if (index != j)
            {
                if (colliders[index].enabled && colliders[j].enabled)
                {
                    if (colliders[index].type == RectangleCollider)
                    {
                        if (colliders[j].type == RectangleCollider)
                        {
                            if (CheckCollisionRecs(colliders[index].bounds, colliders[j].bounds))
                            {
                                collisionChecker = true;
                                
                                if ((colliders[index].bounds.y + colliders[index].bounds.height <= colliders[j].bounds.y) == false)
                                {
                                    rigidbodies[index].isContact = true;
                                }
                            }
                        }
                        else
                        {
                            if (CheckCollisionCircleRec((Vector2){colliders[j].bounds.x, colliders[j].bounds.y}, colliders[j].radius, colliders[index].bounds))
                            {
                                collisionChecker = true;
                            }
                        }
                    }
                    else
                    {
                        if (colliders[j].type == RectangleCollider)
                        {
                            if (CheckCollisionCircleRec((Vector2){colliders[index].bounds.x, colliders[index].bounds.y}, colliders[index].radius, colliders[j].bounds))
                            {
                                collisionChecker = true;
                            }
                        }
                        else
                        {
                            if (CheckCollisionCircles((Vector2){colliders[j].bounds.x, colliders[j].bounds.y}, colliders[j].radius, (Vector2){colliders[index].bounds.x, colliders[index].bounds.y}, colliders[index].radius))
                            {
                                collisionChecker = true;
                            }
                        }
                    }
                }
            }
        }
        
        // Update grounded rigidbody state
        rigidbodies[index].isGrounded = collisionChecker;
        
        // Set grounded state if needed (fix overlap and set y velocity)
        if (collisionChecker && rigidbodies[index].velocity.y != 0)
        {
            position->y += rigidbodies[index].velocity.y;
            rigidbodies[index].velocity.y = -rigidbodies[index].velocity.y * rigidbodies[index].bounciness;
        }
        
        if (rigidbodies[index].isContact)
        {
            position->x -= rigidbodies[index].velocity.x;
            rigidbodies[index].velocity.x = rigidbodies[index].velocity.x;
        }
    }
}

void SetRigidbodyEnabled(int index, bool state)
{
    rigidbodies[index].enabled = state;
}

void SetRigidbodyVelocity(int index, Vector2 velocity)
{
    rigidbodies[index].velocity.x = velocity.x;
    rigidbodies[index].velocity.y = velocity.y;
}

void AddRigidbodyForce(int index, Vector2 force)
{
    rigidbodies[index].acceleration.x = force.x * rigidbodies[index].mass;
    rigidbodies[index].acceleration.y = force.y * rigidbodies[index].mass;
}

void SetColliderEnabled(int index, bool state)
{
    colliders[index].enabled = state;
}

Collider GetCollider(int index)
{
    return colliders[index];
}

Rigidbody GetRigidbody(int index)
{
    return rigidbodies[index];
}
