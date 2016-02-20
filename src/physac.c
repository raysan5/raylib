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

//#define PHYSAC_STANDALONE     // NOTE: To use the physics module as standalone lib, just uncomment this line

#if defined(PHYSAC_STANDALONE)
    #include "physac.h"
#else
    #include "raylib.h"
#endif

#include <math.h>
#include <stdlib.h>             // Required for: malloc(), free()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define DECIMAL_FIX     0.26f       // Decimal margin for collision checks (avoid rigidbodies shake)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Collider *colliders;         // Colliders array, dynamically allocated at runtime
static Rigidbody *rigidbodies;      // Rigitbody array, dynamically allocated at runtime
static bool collisionChecker;

static int maxElements;             // Max physic elements to compute
static bool enabled;                // Physics enabled? (true by default)
static Vector2 gravity;             // Gravity value used for physic calculations

//----------------------------------------------------------------------------------
// Module specific Functions Declarations
//----------------------------------------------------------------------------------
static float Vector2Length(Vector2 vector);
static float Vector2Distance(Vector2 a, Vector2 b);
static void Vector2Normalize(Vector2 *vector);

//----------------------------------------------------------------------------------
// Module Functions Definitions
//----------------------------------------------------------------------------------
void InitPhysics(int maxPhysicElements)
{
    maxElements = maxPhysicElements;
    
    colliders = (Collider *)malloc(maxElements*sizeof(Collider));
    rigidbodies = (Rigidbody *)malloc(maxElements*sizeof(Rigidbody));
    
    for (int i = 0; i < maxElements; i++)
    {
        colliders[i].enabled = false;
        colliders[i].bounds = (Rectangle){ 0, 0, 0, 0 };
        colliders[i].radius = 0;
        
        rigidbodies[i].enabled = false;
        rigidbodies[i].mass = 0.0f;
        rigidbodies[i].velocity = (Vector2){ 0.0f, 0.0f };
        rigidbodies[i].acceleration = (Vector2){ 0.0f, 0.0f };
        rigidbodies[i].isGrounded = false;
        rigidbodies[i].isContact = false;
        rigidbodies[i].friction = 0.0f;
    }
    
    collisionChecker = false;
    enabled = true;
    
    // NOTE: To get better results, gravity needs to be 1:10 from original parameter
    gravity = (Vector2){ 0.0f, -9.81f/10.0f };     // By default, standard gravity
}

void UnloadPhysics()
{
    free(colliders);
    free(rigidbodies);
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
        // Apply friction to acceleration
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
        
        if (rigidbodies[index].acceleration.y > DECIMAL_FIX / 2)
        {
            rigidbodies[index].acceleration.y -= rigidbodies[index].friction;
        }
        else if (rigidbodies[index].acceleration.y < -DECIMAL_FIX / 2)
        {
            rigidbodies[index].acceleration.y += rigidbodies[index].friction;
        }
        else
        {
            rigidbodies[index].acceleration.y = 0;
        }
        
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
        
        if (rigidbodies[index].velocity.y > DECIMAL_FIX / 2)
        {
            rigidbodies[index].velocity.y -= rigidbodies[index].friction;
        }
        else if (rigidbodies[index].velocity.y < -DECIMAL_FIX / 2)
        {
            rigidbodies[index].velocity.y += rigidbodies[index].friction;
        }
        else
        {
            rigidbodies[index].velocity.y = 0;
        }
        
        // Apply gravity
        rigidbodies[index].velocity.y += gravity.y;
        rigidbodies[index].velocity.x += gravity.x;
        
        // Apply acceleration
        rigidbodies[index].velocity.y += rigidbodies[index].acceleration.y;
        rigidbodies[index].velocity.x += rigidbodies[index].acceleration.x;
        
        // Update position vector
        position->x += rigidbodies[index].velocity.x;        
        position->y -= rigidbodies[index].velocity.y;
        
        // Update collider bounds
        colliders[index].bounds.x = position->x;
        colliders[index].bounds.y = position->y;
        
        // Check collision with other colliders
        collisionChecker = false;
        rigidbodies[index].isContact = false;
        for (int j = 0; j < maxElements; j++)
        {
            if (index != j)
            {
                if (colliders[index].enabled && colliders[j].enabled)
                {
                    if (colliders[index].type == COLLIDER_RECTANGLE)
                    {
                        if (colliders[j].type == COLLIDER_RECTANGLE)
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
                        if (colliders[j].type == COLLIDER_RECTANGLE)
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

void SetRigidbodyAcceleration(int index, Vector2 acceleration)
{
    rigidbodies[index].acceleration.x = acceleration.x;
    rigidbodies[index].acceleration.y = acceleration.y;
}

void AddRigidbodyForce(int index, Vector2 force)
{
    rigidbodies[index].acceleration.x = force.x / rigidbodies[index].mass;
    rigidbodies[index].acceleration.y = force.y / rigidbodies[index].mass;
}

void AddForceAtPosition(Vector2 position, float intensity, float radius)
{
    for(int i = 0; i < maxElements; i++)
    {
        if(rigidbodies[i].enabled)
        {
            // Get position from its collider
            Vector2 pos = {colliders[i].bounds.x, colliders[i].bounds.y};
            
            // Get distance between rigidbody position and target position
            float distance = Vector2Distance(position, pos);
            
            if(distance <= radius)
            {
                // Calculate force based on direction
                Vector2 force = {colliders[i].bounds.x - position.x, colliders[i].bounds.y - position.y};
                
                // Normalize the direction vector
                Vector2Normalize(&force);
                
                // Invert y value
                force.y *= -1;
                
                // Apply intensity and distance
                force = (Vector2){force.x * intensity / distance, force.y * intensity / distance};
                
                // Add calculated force to the rigidbodies
                AddRigidbodyForce(i, force);
            }
        }
    }
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

//----------------------------------------------------------------------------------
// Module specific Functions Definitions
//----------------------------------------------------------------------------------
static float Vector2Length(Vector2 vector)
{
    return sqrt((vector.x * vector.x) + (vector.y * vector.y));
}

static float Vector2Distance(Vector2 a, Vector2 b)
{
    Vector2 vector = {b.x - a.x, b.y - a.y};
    return sqrt((vector.x * vector.x) + (vector.y * vector.y));
}

static void Vector2Normalize(Vector2 *vector)
{
    float length = Vector2Length(*vector);
    
    if (length != 0.0f)
    {
        vector->x /= length;
        vector->y /= length;
    }
    else 
    {
        vector->x = 0.0f;
        vector->y = 0.0f;
    }
}
