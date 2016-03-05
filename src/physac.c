/**********************************************************************************************
*
*   [physac] raylib physics module - Basic functions to apply physics to 2D objects
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

#include <stdlib.h>         // Declares malloc() and free() for memory management
#include <math.h>           // abs() and fminf()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_PHYSIC_OBJECTS      256
#define PHYSICS_GRAVITY         -9.81f/2
#define PHYSICS_STEPS           450
#define PHYSICS_ACCURACY        0.0001f     // Velocity subtract operations round filter (friction)
#define PHYSICS_ERRORPERCENT    0.001f        // Collision resolve position fix

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for PHYSAC_STANDALONE usage
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static PhysicObject *physicObjects[MAX_PHYSIC_OBJECTS];             // Physic objects pool
static int physicObjectsCount;                                      // Counts current enabled physic objects

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static float Vector2DotProduct(Vector2 v1, Vector2 v2);             // Returns the dot product of two Vector2

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Initializes pointers array (just pointers, fixed size)
void InitPhysics()
{
    // Initialize physics variables
    physicObjectsCount = 0;
}

// Update physic objects, calculating physic behaviours and collisions detection
void UpdatePhysics()
{
    // Reset all physic objects is grounded state
    for(int i = 0; i < physicObjectsCount; i++)
    {
        if(physicObjects[i]->rigidbody.enabled) physicObjects[i]->rigidbody.isGrounded = false;
    }
    
    for(int steps = 0; steps < PHYSICS_STEPS; steps++)
    {
        for(int i = 0; i < physicObjectsCount; i++)
        {
            if(physicObjects[i]->enabled)
            {
                // Update physic behaviour
                if(physicObjects[i]->rigidbody.enabled)
                {
                    // Apply friction to acceleration in X axis
                    if (physicObjects[i]->rigidbody.acceleration.x > PHYSICS_ACCURACY) physicObjects[i]->rigidbody.acceleration.x -= physicObjects[i]->rigidbody.friction/PHYSICS_STEPS;
                    else if (physicObjects[i]->rigidbody.acceleration.x < PHYSICS_ACCURACY) physicObjects[i]->rigidbody.acceleration.x += physicObjects[i]->rigidbody.friction/PHYSICS_STEPS;
                    else physicObjects[i]->rigidbody.acceleration.x = 0.0f;
                    
                    // Apply friction to velocity in X axis
                    if (physicObjects[i]->rigidbody.velocity.x > PHYSICS_ACCURACY) physicObjects[i]->rigidbody.velocity.x -= physicObjects[i]->rigidbody.friction/PHYSICS_STEPS;
                    else if (physicObjects[i]->rigidbody.velocity.x < PHYSICS_ACCURACY) physicObjects[i]->rigidbody.velocity.x += physicObjects[i]->rigidbody.friction/PHYSICS_STEPS;
                    else physicObjects[i]->rigidbody.velocity.x = 0.0f;
                    
                    // Apply gravity to velocity
                    if (physicObjects[i]->rigidbody.applyGravity) physicObjects[i]->rigidbody.velocity.y += PHYSICS_GRAVITY/PHYSICS_STEPS;
                    
                    // Apply acceleration to velocity
                    physicObjects[i]->rigidbody.velocity.x += physicObjects[i]->rigidbody.acceleration.x/PHYSICS_STEPS;
                    physicObjects[i]->rigidbody.velocity.y += physicObjects[i]->rigidbody.acceleration.y/PHYSICS_STEPS;
                    
                    // Apply velocity to position
                    physicObjects[i]->transform.position.x += physicObjects[i]->rigidbody.velocity.x/PHYSICS_STEPS;
                    physicObjects[i]->transform.position.y -= physicObjects[i]->rigidbody.velocity.y/PHYSICS_STEPS;
                }
                
                // Update collision detection
                if (physicObjects[i]->collider.enabled)
                {
                    // Update collider bounds
                    physicObjects[i]->collider.bounds = TransformToRectangle(physicObjects[i]->transform);
                    
                    // Check collision with other colliders
                    for (int k = 0; k < physicObjectsCount; k++)
                    {
                        if (physicObjects[k]->collider.enabled && i != k)
                        {
                            // Check if colliders are overlapped
                            if (CheckCollisionRecs(physicObjects[i]->collider.bounds, physicObjects[k]->collider.bounds))
                            {
                                // Resolve physic collision
                                // NOTE: collision resolve is generic for all directions and conditions (no axis separated cases behaviours)
                                // and it is separated in rigidbody attributes resolve (velocity changes by impulse) and position correction (position overlap)
                                
                                // 1. Calculate collision normal
                                // -------------------------------------------------------------------------------------------------------------------------------------
                                
                                // Define collision ontact normal
                                Vector2 contactNormal = { 0.0f, 0.0f };
                                
                                // Calculate direction vector from i to k
                                Vector2 direction;
                                direction.x = (physicObjects[k]->transform.position.x + physicObjects[k]->transform.scale.x/2) - (physicObjects[i]->transform.position.x + physicObjects[i]->transform.scale.x/2);
                                direction.y = (physicObjects[k]->transform.position.y + physicObjects[k]->transform.scale.y/2) - (physicObjects[i]->transform.position.y + physicObjects[i]->transform.scale.y/2);
                                
                                // Define overlapping and penetration attributes
                                Vector2 overlap;
                                float penetrationDepth = 0.0f;
                                
                                // Calculate overlap on X axis
                                overlap.x = (physicObjects[i]->transform.scale.x + physicObjects[k]->transform.scale.x)/2 - abs(direction.x);
                                
                                // SAT test on X axis
                                if (overlap.x > 0.0f)
                                {
                                    // Calculate overlap on Y axis
                                    overlap.y = (physicObjects[i]->transform.scale.y + physicObjects[k]->transform.scale.y)/2 - abs(direction.y);
                                    
                                    // SAT test on Y axis
                                    if (overlap.y > 0.0f)
                                    {
                                        // Find out which axis is axis of least penetration
                                        if (overlap.y > overlap.x)
                                        {
                                            // Point towards k knowing that direction points from i to k
                                            if (direction.x < 0.0f) contactNormal = (Vector2){ -1.0f, 0.0f };
                                            else contactNormal = (Vector2){ 1.0f, 0.0f };
                                            
                                            // Update penetration depth for position correction
                                            penetrationDepth = overlap.x;
                                        }
                                        else
                                        {
                                            // Point towards k knowing that direction points from i to k
                                            if (direction.y < 0.0f) contactNormal = (Vector2){ 0.0f, 1.0f };
                                            else contactNormal = (Vector2){ 0.0f, -1.0f };
                                            
                                            // Update penetration depth for position correction
                                            penetrationDepth = overlap.y;
                                        }
                                    }
                                }
                                
                                // Update rigidbody grounded state
                                if (physicObjects[i]->rigidbody.enabled)
                                {
                                    if (contactNormal.y < 0.0f) physicObjects[i]->rigidbody.isGrounded = true;
                                }
                                
                                // 2. Calculate collision impulse
                                // -------------------------------------------------------------------------------------------------------------------------------------
                                
                                // Calculate relative velocity
                                Vector2 relVelocity = { physicObjects[k]->rigidbody.velocity.x - physicObjects[i]->rigidbody.velocity.x, physicObjects[k]->rigidbody.velocity.y - physicObjects[i]->rigidbody.velocity.y };

                                // Calculate relative velocity in terms of the normal direction
                                float velAlongNormal = Vector2DotProduct(relVelocity, contactNormal);
                            
                                // Dot not resolve if velocities are separating
                                if (velAlongNormal <= 0.0f)
                                {
                                    // Calculate minimum bounciness value from both objects
                                    float e = fminf(physicObjects[i]->rigidbody.bounciness, physicObjects[k]->rigidbody.bounciness);
                                    
                                    // Calculate impulse scalar value
                                    float j = -(1.0f + e) * velAlongNormal;
                                    j /= 1.0f/physicObjects[i]->rigidbody.mass + 1.0f/physicObjects[k]->rigidbody.mass;
                                    
                                    // Calculate final impulse vector
                                    Vector2 impulse = { j*contactNormal.x, j*contactNormal.y };
                                    
                                    // Calculate collision mass ration
                                    float massSum = physicObjects[i]->rigidbody.mass + physicObjects[k]->rigidbody.mass;
                                    float ratio = 0.0f;
                                    
                                    // Apply impulse to current rigidbodies velocities if they are enabled
                                    if (physicObjects[i]->rigidbody.enabled) 
                                    {
                                        // Calculate inverted mass ration
                                        ratio = physicObjects[i]->rigidbody.mass/massSum;
                                        
                                        // Apply impulse direction to velocity
                                        physicObjects[i]->rigidbody.velocity.x -= impulse.x*ratio;
                                        physicObjects[i]->rigidbody.velocity.y -= impulse.y*ratio;
                                    }
                                    
                                    if (physicObjects[k]->rigidbody.enabled) 
                                    {
                                        // Calculate inverted mass ration
                                        ratio = physicObjects[k]->rigidbody.mass/massSum;
                                        
                                        // Apply impulse direction to velocity
                                        physicObjects[k]->rigidbody.velocity.x += impulse.x*ratio;
                                        physicObjects[k]->rigidbody.velocity.y += impulse.y*ratio;
                                    }
                                    
                                    // 3. Correct colliders overlaping (transform position)
                                    // ---------------------------------------------------------------------------------------------------------------------------------
                                    
                                    // Calculate transform position penetration correction
                                    Vector2 posCorrection;
                                    posCorrection.x = penetrationDepth/((1.0f/physicObjects[i]->rigidbody.mass) + (1.0f/physicObjects[k]->rigidbody.mass))*PHYSICS_ERRORPERCENT*contactNormal.x;
                                    posCorrection.y = penetrationDepth/((1.0f/physicObjects[i]->rigidbody.mass) + (1.0f/physicObjects[k]->rigidbody.mass))*PHYSICS_ERRORPERCENT*contactNormal.y;
                                    
                                    // Fix transform positions
                                    if (physicObjects[i]->rigidbody.enabled)
                                    {                                        
                                        // Fix physic objects transform position
                                        physicObjects[i]->transform.position.x -= 1.0f/physicObjects[i]->rigidbody.mass*posCorrection.x;
                                        physicObjects[i]->transform.position.y += 1.0f/physicObjects[i]->rigidbody.mass*posCorrection.y;
                                        
                                        // Update collider bounds
                                        physicObjects[i]->collider.bounds = TransformToRectangle(physicObjects[i]->transform);
                                        
                                        if (physicObjects[k]->rigidbody.enabled)
                                        {
                                            // Fix physic objects transform position
                                            physicObjects[k]->transform.position.x += 1.0f/physicObjects[k]->rigidbody.mass*posCorrection.x;
                                            physicObjects[k]->transform.position.y -= 1.0f/physicObjects[k]->rigidbody.mass*posCorrection.y;
                                            
                                            // Update collider bounds
                                            physicObjects[k]->collider.bounds = TransformToRectangle(physicObjects[k]->transform);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Unitialize all physic objects and empty the objects pool
void ClosePhysics()
{
    // Free all dynamic memory allocations
    for (int i = 0; i < physicObjectsCount; i++) free(physicObjects[i]);
    
    // Reset enabled physic objects count
    physicObjectsCount = 0;
}

// Create a new physic object dinamically, initialize it and add to pool
PhysicObject *CreatePhysicObject(Vector2 position, float rotation, Vector2 scale)
{
    // Allocate dynamic memory
    PhysicObject *obj = (PhysicObject *)malloc(sizeof(PhysicObject));
    
    // Initialize physic object values with generic values
    obj->id = physicObjectsCount;
    obj->enabled = true;
    
    obj->transform = (Transform){ (Vector2){ position.x - scale.x/2, position.y - scale.y/2 }, rotation, scale };
    
    obj->rigidbody.enabled = false;
    obj->rigidbody.mass = 1.0f;
    obj->rigidbody.acceleration = (Vector2){ 0.0f, 0.0f };
    obj->rigidbody.velocity = (Vector2){ 0.0f, 0.0f };
    obj->rigidbody.applyGravity = false;
    obj->rigidbody.isGrounded = false;
    obj->rigidbody.friction = 0.0f;
    obj->rigidbody.bounciness = 0.0f;
    
    obj->collider.enabled = false;
    obj->collider.type = COLLIDER_RECTANGLE;
    obj->collider.bounds = TransformToRectangle(obj->transform);
    obj->collider.radius = 0.0f;
    
    // Add new physic object to the pointers array
    physicObjects[physicObjectsCount] = obj;
    
    // Increase enabled physic objects count
    physicObjectsCount++;
    
    return obj;
}

// Destroy a specific physic object and take it out of the list
void DestroyPhysicObject(PhysicObject *pObj)
{
    // Free dynamic memory allocation
    free(physicObjects[pObj->id]);
    
    // Remove *obj from the pointers array
    for (int i = pObj->id; i < physicObjectsCount; i++)
    {
        // Resort all the following pointers of the array
        if ((i + 1) < physicObjectsCount)
        {
            physicObjects[i] = physicObjects[i + 1];
            physicObjects[i]->id = physicObjects[i + 1]->id;
        }
        else free(physicObjects[i]);
    }
    
    // Decrease enabled physic objects count
    physicObjectsCount--;
}

// Convert Transform data type to Rectangle (position and scale)
Rectangle TransformToRectangle(Transform transform)
{
    return (Rectangle){transform.position.x, transform.position.y, transform.scale.x, transform.scale.y};
}

// Draw physic object information at screen position
void DrawPhysicObjectInfo(PhysicObject *pObj, Vector2 position, int fontSize)
{
    // Draw physic object ID
    DrawText(FormatText("PhysicObject ID: %i - Enabled: %i", pObj->id, pObj->enabled), position.x, position.y, fontSize, BLACK);
    
    // Draw physic object transform values
    DrawText(FormatText("\nTRANSFORM\nPosition: %f, %f\nRotation: %f\nScale: %f, %f", pObj->transform.position.x, pObj->transform.position.y, pObj->transform.rotation, pObj->transform.scale.x, pObj->transform.scale.y), position.x, position.y, fontSize, BLACK);
    
    // Draw physic object rigidbody values
    DrawText(FormatText("\n\n\n\n\n\nRIGIDBODY\nEnabled: %i\nMass: %f\nAcceleration: %f, %f\nVelocity: %f, %f\nApplyGravity: %i\nIsGrounded: %i\nFriction: %f\nBounciness: %f", pObj->rigidbody.enabled, pObj->rigidbody.mass, pObj->rigidbody.acceleration.x, pObj->rigidbody.acceleration.y, 
    pObj->rigidbody.velocity.x, pObj->rigidbody.velocity.y, pObj->rigidbody.applyGravity, pObj->rigidbody.isGrounded, pObj->rigidbody.friction, pObj->rigidbody.bounciness), position.x, position.y, fontSize, BLACK);
    
    DrawText(FormatText("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nCOLLIDER\nEnabled: %i\nBounds: %i, %i, %i, %i\nRadius: %i", pObj->collider.enabled, pObj->collider.bounds.x, pObj->collider.bounds.y, pObj->collider.bounds.width, pObj->collider.bounds.height, pObj->collider.radius), position.x, position.y, fontSize, BLACK);
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Returns the dot product of two Vector2
static float Vector2DotProduct(Vector2 v1, Vector2 v2)
{
    float result;

    result = v1.x*v2.x + v1.y*v2.y;

    return result;
}
