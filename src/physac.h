/**********************************************************************************************
*
*   physac 1.0 - 2D Physics library for raylib (https://github.com/raysan5/raylib)
*
*   // TODO: Description...
* 
*   CONFIGURATION:
*   
*   #define PHYSAC_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers 
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define PHYSAC_STATIC (defined by default)
*       The generated implementation will stay private inside implementation file and all 
*       internal symbols and functions will only be visible inside that file.
*
*   #define PHYSAC_NO_THREADS
*       The generated implementation won't include pthread library and user must create a secondary thread to call PhysicsThread().
*       It is so important that the thread where PhysicsThread() is called must not have v-sync or any other CPU limitation.
*
*   #define PHYSAC_STANDALONE
*       Avoid raylib.h header inclusion in this file. Data types defined on raylib are defined
*       internally in the library and input management and drawing functions must be provided by
*       the user (check library implementation for further details).
*
*   #define PHYSAC_MALLOC()
*   #define PHYSAC_FREE()
*       You can define your own malloc/free implementation replacing stdlib.h malloc()/free() functions.
*       Otherwise it will include stdlib.h and use the C standard library malloc()/free() function.
*       
*   LIMITATIONS:
*
*       - There is a limit of 256 physic objects.
*       - Physics behaviour can be unexpected using bounciness or friction values out of 0.0f - 1.0f range.
*       - The module is limited to 2D axis oriented physics.
*       - Physics colliders must be rectangle or circle shapes (there is not a custom polygon collider type).
*
*   VERSIONS:
*
*   1.0 (14-Jun-2016) New module defines and fixed some delta time calculation bugs.
*   0.9 (09-Jun-2016) Module names review and converted to header-only.
*   0.8 (23-Mar-2016) Complete module redesign, steps-based for better physics resolution.
*   0.3 (13-Feb-2016) Reviewed to add PhysicObjects pool.
*   0.2 (03-Jan-2016) Improved physics calculations.
*   0.1 (30-Dec-2015) Initial release.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016 Victor Fisac (main developer) and Ramon Santamaria
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

#if !defined(RAYGUI_STANDALONE)
    #include "raylib.h"
#endif

#define PHYSAC_STATIC
#ifdef PHYSAC_STATIC
    #define PHYSACDEF static            // Functions just visible to module including this file
#else
    #ifdef __cplusplus
        #define PHYSACDEF extern "C"    // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define PHYSACDEF extern        // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for PHYSAC_STANDALONE usage
//----------------------------------------------------------------------------------
#if defined(PHYSAC_STANDALONE)
    #ifndef __cplusplus
    // Boolean type
        #ifndef true
            typedef enum { false, true } bool;
        #endif
    #endif

    // Vector2 type
    typedef struct Vector2 {
        float x;
        float y;
    } Vector2;

    // Rectangle type
    typedef struct Rectangle {
        int x;
        int y;
        int width;
        int height;
    } Rectangle;
#endif

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

typedef struct PhysicBodyData {
    unsigned int id;
    Transform transform;
    Rigidbody rigidbody;
    Collider collider;
    bool enabled;
} PhysicBodyData, *PhysicBody;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
PHYSACDEF void InitPhysics(Vector2 gravity);                                            // Initializes pointers array (just pointers, fixed size)
PHYSACDEF void* PhysicsThread(void *arg);                                               // Physics calculations thread function
PHYSACDEF void ClosePhysics();                                                          // Unitialize all physic objects and empty the objects pool

PHYSACDEF PhysicBody CreatePhysicBody(Vector2 position, float rotation, Vector2 scale); // Create a new physic body dinamically, initialize it and add to pool
PHYSACDEF void DestroyPhysicBody(PhysicBody pbody);                                     // Destroy a specific physic body and take it out of the list

PHYSACDEF void ApplyForce(PhysicBody pbody, Vector2 force);                             // Apply directional force to a physic body
PHYSACDEF void ApplyForceAtPosition(Vector2 position, float force, float radius);       // Apply radial force to all physic objects in range

PHYSACDEF Rectangle TransformToRectangle(Transform transform);                          // Convert Transform data type to Rectangle (position and scale)

#endif // PHYSAC_H


/***********************************************************************************
*
*   PHYSAC IMPLEMENTATION
*
************************************************************************************/

#if defined(PHYSAC_IMPLEMENTATION)

// Check if custom malloc/free functions defined, if not, using standard ones
#if !defined(PHYSAC_MALLOC)
    #include <stdlib.h>     // Required for: malloc(), free()
    
    #define PHYSAC_MALLOC(size)  malloc(size)
    #define PHYSAC_FREE(ptr)     free(ptr)
#endif

#include <math.h>           // Required for: cos(), sin(), abs(), fminf()
#include <stdint.h>         // Required for typedef unsigned long long int uint64_t, used by hi-res timer

#ifndef PHYSAC_NO_THREADS
    #include <pthread.h>        // Required for: pthread_create()
#endif

#if defined(PLATFORM_DESKTOP)
    // Functions required to query time on Windows
    int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
    int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    #include <sys/time.h>       // Required for: timespec
    #include <time.h>           // Required for: clock_gettime()
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_PHYSIC_BODIES       256             // Maximum available physic bodies slots in bodies pool
#define PHYSICS_TIMESTEP        0.016666        // Physics fixed time step (1/fps)
#define PHYSICS_ACCURACY        0.0001f         // Velocity subtract operations round filter (friction)
#define PHYSICS_ERRORPERCENT    0.001f          // Collision resolve position fix

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for PHYSAC_STANDALONE usage
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static bool physicsThreadEnabled = false;                           // Physics calculations thread exit control
static uint64_t baseTime;                                           // Base time measure for hi-res timer
static double currentTime, previousTime;                            // Used to track timmings
static PhysicBody physicBodies[MAX_PHYSIC_BODIES];                  // Physic bodies pool
static int physicBodiesCount;                                       // Counts current enabled physic bodies
static Vector2 gravityForce;                                        // Gravity force

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void UpdatePhysics(double deltaTime);                        // Update physic objects, calculating physic behaviours and collisions detection
static void InitTimer(void);                                        // Initialize hi-resolution timer
static double GetCurrentTime(void);                                 // Time measure returned are microseconds
static float Vector2DotProduct(Vector2 v1, Vector2 v2);             // Returns the dot product of two Vector2
static float Vector2Length(Vector2 v);                              // Returns the length of a Vector2

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Initializes pointers array (just pointers, fixed size)
PHYSACDEF void InitPhysics(Vector2 gravity)
{
    // Initialize physics variables
    physicBodiesCount = 0;
    gravityForce = gravity;
    
    #ifndef PHYSAC_NO_THREADS       // NOTE: if defined, user will need to create a thread for PhysicsThread function manually
        // Create physics thread
        pthread_t tid;
        pthread_create(&tid, NULL, &PhysicsThread, NULL);
    #endif
}

// Unitialize all physic objects and empty the objects pool
PHYSACDEF void ClosePhysics()
{
    // Exit physics thread loop
    physicsThreadEnabled = false;
    
    // Free all dynamic memory allocations
    for (int i = 0; i < physicBodiesCount; i++) PHYSAC_FREE(physicBodies[i]);
    
    // Reset enabled physic objects count
    physicBodiesCount = 0;
}

// Create a new physic body dinamically, initialize it and add to pool
PHYSACDEF PhysicBody CreatePhysicBody(Vector2 position, float rotation, Vector2 scale)
{
    // Allocate dynamic memory
    PhysicBody obj = (PhysicBody)PHYSAC_MALLOC(sizeof(PhysicBodyData));
    
    // Initialize physic body values with generic values
    obj->id = physicBodiesCount;
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
    
    obj->collider.enabled = true;
    obj->collider.type = COLLIDER_RECTANGLE;
    obj->collider.bounds = TransformToRectangle(obj->transform);
    obj->collider.radius = 0.0f;
    
    // Add new physic body to the pointers array
    physicBodies[physicBodiesCount] = obj;
    
    // Increase enabled physic bodies count
    physicBodiesCount++;
    
    return obj;
}

// Destroy a specific physic body and take it out of the list
PHYSACDEF void DestroyPhysicBody(PhysicBody pbody)
{
    // Free dynamic memory allocation
    PHYSAC_FREE(physicBodies[pbody->id]);
    
    // Remove *obj from the pointers array
    for (int i = pbody->id; i < physicBodiesCount; i++)
    {
        // Resort all the following pointers of the array
        if ((i + 1) < physicBodiesCount)
        {
            physicBodies[i] = physicBodies[i + 1];
            physicBodies[i]->id = physicBodies[i + 1]->id;
        }
        else PHYSAC_FREE(physicBodies[i]);
    }
    
    // Decrease enabled physic bodies count
    physicBodiesCount--;
}

// Apply directional force to a physic body
PHYSACDEF void ApplyForce(PhysicBody pbody, Vector2 force)
{
    if (pbody->rigidbody.enabled)
    {
        pbody->rigidbody.velocity.x += force.x/pbody->rigidbody.mass;
        pbody->rigidbody.velocity.y += force.y/pbody->rigidbody.mass;
    }
}

// Apply radial force to all physic objects in range
PHYSACDEF void ApplyForceAtPosition(Vector2 position, float force, float radius)
{
    for (int i = 0; i < physicBodiesCount; i++)
    {
        if (physicBodies[i]->rigidbody.enabled)
        {
            // Calculate direction and distance between force and physic body position
            Vector2 distance = (Vector2){ physicBodies[i]->transform.position.x - position.x, physicBodies[i]->transform.position.y - position.y };

            if (physicBodies[i]->collider.type == COLLIDER_RECTANGLE)
            {
                distance.x += physicBodies[i]->transform.scale.x/2;
                distance.y += physicBodies[i]->transform.scale.y/2;
            }
            
            float distanceLength = Vector2Length(distance);
            
            // Check if physic body is in force range
            if (distanceLength <= radius)
            {
                // Normalize force direction
                distance.x /= distanceLength;
                distance.y /= -distanceLength;
                
                // Calculate final force
                Vector2 finalForce = { distance.x*force, distance.y*force };
                
                // Apply force to the physic body
                ApplyForce(physicBodies[i], finalForce);
            }
        }
    }
}

// Convert Transform data type to Rectangle (position and scale)
PHYSACDEF Rectangle TransformToRectangle(Transform transform)
{
    return (Rectangle){transform.position.x, transform.position.y, transform.scale.x, transform.scale.y};
}

// Physics calculations thread function
PHYSACDEF void* PhysicsThread(void *arg)
{
    // Initialize thread loop state
    physicsThreadEnabled = true;
    
    // Initialize hi-resolution timer
    InitTimer();
    
    // Physics update loop
    while (physicsThreadEnabled) 
    {
        currentTime = GetCurrentTime();
        double deltaTime = (double)(currentTime - previousTime);
        previousTime = currentTime;

        // Delta time value needs to be inverse multiplied by physics time step value (1/target fps)
        UpdatePhysics(deltaTime/PHYSICS_TIMESTEP);
    }
    
    return NULL;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Initialize hi-resolution timer
static void InitTimer(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)  // Success
    {
        baseTime = (uint64_t)now.tv_sec*1000000000LLU + (uint64_t)now.tv_nsec;
    }
#endif

    previousTime = GetCurrentTime();       // Get time as double
}

// Time measure returned are microseconds
static double GetCurrentTime(void)
{
    double time;
    
#if defined(PLATFORM_DESKTOP)
    unsigned long long int clockFrequency, currentTime;
    
    QueryPerformanceFrequency(&clockFrequency);
    QueryPerformanceCounter(&currentTime);
    
    time = (double)((double)currentTime/(double)clockFrequency);
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_RPI)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t temp = (uint64_t)ts.tv_sec*1000000000LLU + (uint64_t)ts.tv_nsec;

    time = (double)(temp - baseTime)*1e-9;
#endif

    return time;
}

// Returns the dot product of two Vector2
static float Vector2DotProduct(Vector2 v1, Vector2 v2)
{
    float result;

    result = v1.x*v2.x + v1.y*v2.y;

    return result;
}

static float Vector2Length(Vector2 v)
{
    float result;
    
    result = sqrt(v.x*v.x + v.y*v.y);
    
    return result;
}

// Update physic objects, calculating physic behaviours and collisions detection
static void UpdatePhysics(double deltaTime)
{
    for (int i = 0; i < physicBodiesCount; i++)
    {
        if (physicBodies[i]->enabled)
        {
            // Update physic behaviour
            if (physicBodies[i]->rigidbody.enabled)
            {
                // Apply friction to acceleration in X axis
                if (physicBodies[i]->rigidbody.acceleration.x > PHYSICS_ACCURACY) physicBodies[i]->rigidbody.acceleration.x -= physicBodies[i]->rigidbody.friction*deltaTime;
                else if (physicBodies[i]->rigidbody.acceleration.x < PHYSICS_ACCURACY) physicBodies[i]->rigidbody.acceleration.x += physicBodies[i]->rigidbody.friction*deltaTime;
                else physicBodies[i]->rigidbody.acceleration.x = 0.0f;
                
                // Apply friction to acceleration in Y axis
                if (physicBodies[i]->rigidbody.acceleration.y > PHYSICS_ACCURACY) physicBodies[i]->rigidbody.acceleration.y -= physicBodies[i]->rigidbody.friction*deltaTime;
                else if (physicBodies[i]->rigidbody.acceleration.y < PHYSICS_ACCURACY) physicBodies[i]->rigidbody.acceleration.y += physicBodies[i]->rigidbody.friction*deltaTime;
                else physicBodies[i]->rigidbody.acceleration.y = 0.0f;
                
                // Apply friction to velocity in X axis
                if (physicBodies[i]->rigidbody.velocity.x > PHYSICS_ACCURACY) physicBodies[i]->rigidbody.velocity.x -= physicBodies[i]->rigidbody.friction*deltaTime;
                else if (physicBodies[i]->rigidbody.velocity.x < PHYSICS_ACCURACY) physicBodies[i]->rigidbody.velocity.x += physicBodies[i]->rigidbody.friction*deltaTime;
                else physicBodies[i]->rigidbody.velocity.x = 0.0f;
                
                // Apply friction to velocity in Y axis
                if (physicBodies[i]->rigidbody.velocity.y > PHYSICS_ACCURACY) physicBodies[i]->rigidbody.velocity.y -= physicBodies[i]->rigidbody.friction*deltaTime;
                else if (physicBodies[i]->rigidbody.velocity.y < PHYSICS_ACCURACY) physicBodies[i]->rigidbody.velocity.y += physicBodies[i]->rigidbody.friction*deltaTime;
                else physicBodies[i]->rigidbody.velocity.y = 0.0f;
                
                // Apply gravity to velocity
                if (physicBodies[i]->rigidbody.applyGravity)
                {
                    physicBodies[i]->rigidbody.velocity.x += gravityForce.x*deltaTime;
                    physicBodies[i]->rigidbody.velocity.y += gravityForce.y*deltaTime;
                }
                
                // Apply acceleration to velocity
                physicBodies[i]->rigidbody.velocity.x += physicBodies[i]->rigidbody.acceleration.x*deltaTime;
                physicBodies[i]->rigidbody.velocity.y += physicBodies[i]->rigidbody.acceleration.y*deltaTime;
                
                // Apply velocity to position
                physicBodies[i]->transform.position.x += physicBodies[i]->rigidbody.velocity.x*deltaTime;
                physicBodies[i]->transform.position.y -= physicBodies[i]->rigidbody.velocity.y*deltaTime;
            }
            
            // Update collision detection
            if (physicBodies[i]->collider.enabled)
            {
                // Update collider bounds
                physicBodies[i]->collider.bounds = TransformToRectangle(physicBodies[i]->transform);
                
                // Check collision with other colliders
                for (int k = 0; k < physicBodiesCount; k++)
                {
                    if (physicBodies[k]->collider.enabled && i != k)
                    {
                        // Resolve physic collision
                        // NOTE: collision resolve is generic for all directions and conditions (no axis separated cases behaviours)
                        // and it is separated in rigidbody attributes resolve (velocity changes by impulse) and position correction (position overlap)
                        
                        // 1. Calculate collision normal
                        // -------------------------------------------------------------------------------------------------------------------------------------
                        
                        // Define collision contact normal, direction and penetration depth
                        Vector2 contactNormal = { 0.0f, 0.0f };
                        Vector2 direction = { 0.0f, 0.0f };
                        float penetrationDepth = 0.0f;
                        
                        switch (physicBodies[i]->collider.type)
                        {
                            case COLLIDER_RECTANGLE:
                            {
                                switch (physicBodies[k]->collider.type)
                                {
                                    case COLLIDER_RECTANGLE:
                                    {
                                        // Check if colliders are overlapped
                                        if (CheckCollisionRecs(physicBodies[i]->collider.bounds, physicBodies[k]->collider.bounds))
                                        {
                                            // Calculate direction vector from i to k
                                            direction.x = (physicBodies[k]->transform.position.x + physicBodies[k]->transform.scale.x/2) - (physicBodies[i]->transform.position.x + physicBodies[i]->transform.scale.x/2);
                                            direction.y = (physicBodies[k]->transform.position.y + physicBodies[k]->transform.scale.y/2) - (physicBodies[i]->transform.position.y + physicBodies[i]->transform.scale.y/2);
                                            
                                            // Define overlapping and penetration attributes
                                            Vector2 overlap;

                                            // Calculate overlap on X axis
                                            overlap.x = (physicBodies[i]->transform.scale.x + physicBodies[k]->transform.scale.x)/2 - abs(direction.x);
                                            
                                            // SAT test on X axis
                                            if (overlap.x > 0.0f)
                                            {
                                                // Calculate overlap on Y axis
                                                overlap.y = (physicBodies[i]->transform.scale.y + physicBodies[k]->transform.scale.y)/2 - abs(direction.y);
                                                
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
                                        }
                                    } break;
                                    case COLLIDER_CIRCLE:
                                    {
                                        if (CheckCollisionCircleRec(physicBodies[k]->transform.position, physicBodies[k]->collider.radius, physicBodies[i]->collider.bounds))
                                        {
                                            // Calculate direction vector between circles
                                            direction.x = physicBodies[k]->transform.position.x - physicBodies[i]->transform.position.x + physicBodies[i]->transform.scale.x/2;
                                            direction.y = physicBodies[k]->transform.position.y - physicBodies[i]->transform.position.y + physicBodies[i]->transform.scale.y/2;
                                            
                                            // Calculate closest point on rectangle to circle
                                            Vector2 closestPoint = { 0.0f, 0.0f };
                                            if (direction.x > 0.0f) closestPoint.x = physicBodies[i]->collider.bounds.x + physicBodies[i]->collider.bounds.width;
                                            else closestPoint.x = physicBodies[i]->collider.bounds.x;
                                            
                                            if (direction.y > 0.0f) closestPoint.y = physicBodies[i]->collider.bounds.y + physicBodies[i]->collider.bounds.height;
                                            else closestPoint.y = physicBodies[i]->collider.bounds.y;
                                            
                                            // Check if the closest point is inside the circle
                                            if (CheckCollisionPointCircle(closestPoint, physicBodies[k]->transform.position, physicBodies[k]->collider.radius))
                                            {
                                                // Recalculate direction based on closest point position
                                                direction.x = physicBodies[k]->transform.position.x - closestPoint.x;
                                                direction.y = physicBodies[k]->transform.position.y - closestPoint.y;
                                                float distance = Vector2Length(direction);
                                                
                                                // Calculate final contact normal
                                                contactNormal.x = direction.x/distance;
                                                contactNormal.y = -direction.y/distance;
                                                
                                                // Calculate penetration depth
                                                penetrationDepth = physicBodies[k]->collider.radius - distance;
                                            }
                                            else
                                            {
                                                if (abs(direction.y) < abs(direction.x))
                                                {
                                                    // Calculate final contact normal
                                                    if (direction.y > 0.0f)
                                                    {
                                                        contactNormal = (Vector2){ 0.0f, -1.0f };
                                                        penetrationDepth = fabs(physicBodies[i]->collider.bounds.y - physicBodies[k]->transform.position.y - physicBodies[k]->collider.radius);
                                                    }
                                                    else 
                                                    {
                                                        contactNormal = (Vector2){ 0.0f, 1.0f };
                                                        penetrationDepth = fabs(physicBodies[i]->collider.bounds.y - physicBodies[k]->transform.position.y + physicBodies[k]->collider.radius);
                                                    }
                                                }
                                                else
                                                {
                                                    // Calculate final contact normal
                                                    if (direction.x > 0.0f)
                                                    {
                                                        contactNormal = (Vector2){ 1.0f, 0.0f };
                                                        penetrationDepth = fabs(physicBodies[k]->transform.position.x + physicBodies[k]->collider.radius - physicBodies[i]->collider.bounds.x);
                                                    }
                                                    else 
                                                    {
                                                        contactNormal = (Vector2){ -1.0f, 0.0f };
                                                        penetrationDepth = fabs(physicBodies[i]->collider.bounds.x + physicBodies[i]->collider.bounds.width - physicBodies[k]->transform.position.x - physicBodies[k]->collider.radius);
                                                    }
                                                }
                                            }
                                        }
                                    } break;
                                }
                            } break;
                            case COLLIDER_CIRCLE:
                            {
                                switch (physicBodies[k]->collider.type)
                                {
                                    case COLLIDER_RECTANGLE:
                                    {
                                        if (CheckCollisionCircleRec(physicBodies[i]->transform.position, physicBodies[i]->collider.radius, physicBodies[k]->collider.bounds))
                                        {
                                            // Calculate direction vector between circles
                                            direction.x = physicBodies[k]->transform.position.x + physicBodies[i]->transform.scale.x/2 - physicBodies[i]->transform.position.x;
                                            direction.y = physicBodies[k]->transform.position.y + physicBodies[i]->transform.scale.y/2 - physicBodies[i]->transform.position.y;
                                            
                                            // Calculate closest point on rectangle to circle
                                            Vector2 closestPoint = { 0.0f, 0.0f };
                                            if (direction.x > 0.0f) closestPoint.x = physicBodies[k]->collider.bounds.x + physicBodies[k]->collider.bounds.width;
                                            else closestPoint.x = physicBodies[k]->collider.bounds.x;
                                            
                                            if (direction.y > 0.0f) closestPoint.y = physicBodies[k]->collider.bounds.y + physicBodies[k]->collider.bounds.height;
                                            else closestPoint.y = physicBodies[k]->collider.bounds.y;
                                            
                                            // Check if the closest point is inside the circle
                                            if (CheckCollisionPointCircle(closestPoint, physicBodies[i]->transform.position, physicBodies[i]->collider.radius))
                                            {
                                                // Recalculate direction based on closest point position
                                                direction.x = physicBodies[i]->transform.position.x - closestPoint.x;
                                                direction.y = physicBodies[i]->transform.position.y - closestPoint.y;
                                                float distance = Vector2Length(direction);
                                                
                                                // Calculate final contact normal
                                                contactNormal.x = direction.x/distance;
                                                contactNormal.y = -direction.y/distance;
                                                
                                                // Calculate penetration depth
                                                penetrationDepth = physicBodies[k]->collider.radius - distance;
                                            }
                                            else
                                            {
                                                if (abs(direction.y) < abs(direction.x))
                                                {
                                                    // Calculate final contact normal
                                                    if (direction.y > 0.0f)
                                                    {
                                                        contactNormal = (Vector2){ 0.0f, -1.0f };
                                                        penetrationDepth = fabs(physicBodies[k]->collider.bounds.y - physicBodies[i]->transform.position.y - physicBodies[i]->collider.radius);
                                                    }
                                                    else 
                                                    {
                                                        contactNormal = (Vector2){ 0.0f, 1.0f };
                                                        penetrationDepth = fabs(physicBodies[k]->collider.bounds.y - physicBodies[i]->transform.position.y + physicBodies[i]->collider.radius);
                                                    }
                                                }
                                                else
                                                {
                                                    // Calculate final contact normal and penetration depth
                                                    if (direction.x > 0.0f)
                                                    {
                                                        contactNormal = (Vector2){ 1.0f, 0.0f };
                                                        penetrationDepth = fabs(physicBodies[i]->transform.position.x + physicBodies[i]->collider.radius - physicBodies[k]->collider.bounds.x);
                                                    }
                                                    else 
                                                    {
                                                        contactNormal = (Vector2){ -1.0f, 0.0f };
                                                        penetrationDepth = fabs(physicBodies[k]->collider.bounds.x + physicBodies[k]->collider.bounds.width - physicBodies[i]->transform.position.x - physicBodies[i]->collider.radius);
                                                    }
                                                }
                                            }
                                        }
                                    } break;
                                    case COLLIDER_CIRCLE:
                                    {
                                        // Check if colliders are overlapped
                                        if (CheckCollisionCircles(physicBodies[i]->transform.position, physicBodies[i]->collider.radius, physicBodies[k]->transform.position, physicBodies[k]->collider.radius))
                                        {
                                            // Calculate direction vector between circles
                                            direction.x = physicBodies[k]->transform.position.x - physicBodies[i]->transform.position.x;
                                            direction.y = physicBodies[k]->transform.position.y - physicBodies[i]->transform.position.y;
                                            
                                            // Calculate distance between circles
                                            float distance = Vector2Length(direction);
                                            
                                            // Check if circles are not completely overlapped
                                            if (distance != 0.0f)
                                            {                                                    
                                                // Calculate contact normal direction (Y axis needs to be flipped)
                                                contactNormal.x = direction.x/distance;
                                                contactNormal.y = -direction.y/distance;
                                            }
                                            else contactNormal = (Vector2){ 1.0f, 0.0f };   // Choose random (but consistent) values
                                        }
                                    } break;
                                    default: break;
                                }
                            } break;
                            default: break;
                        }
                        
                        // Update rigidbody grounded state
                        if (physicBodies[i]->rigidbody.enabled) physicBodies[i]->rigidbody.isGrounded = (contactNormal.y < 0.0f);
                        
                        // 2. Calculate collision impulse
                        // -------------------------------------------------------------------------------------------------------------------------------------
                        
                        // Calculate relative velocity
                        Vector2 relVelocity = { 0.0f, 0.0f };
                        relVelocity.x = physicBodies[k]->rigidbody.velocity.x - physicBodies[i]->rigidbody.velocity.x;
                        relVelocity.y = physicBodies[k]->rigidbody.velocity.y - physicBodies[i]->rigidbody.velocity.y;

                        // Calculate relative velocity in terms of the normal direction
                        float velAlongNormal = Vector2DotProduct(relVelocity, contactNormal);
                    
                        // Dot not resolve if velocities are separating
                        if (velAlongNormal <= 0.0f)
                        {
                            // Calculate minimum bounciness value from both objects
                            float e = fminf(physicBodies[i]->rigidbody.bounciness, physicBodies[k]->rigidbody.bounciness);
                            
                            // Calculate impulse scalar value
                            float j = -(1.0f + e)*velAlongNormal;
                            j /= 1.0f/physicBodies[i]->rigidbody.mass + 1.0f/physicBodies[k]->rigidbody.mass;
                            
                            // Calculate final impulse vector
                            Vector2 impulse = { j*contactNormal.x, j*contactNormal.y };
                            
                            // Calculate collision mass ration
                            float massSum = physicBodies[i]->rigidbody.mass + physicBodies[k]->rigidbody.mass;
                            float ratio = 0.0f;
                            
                            // Apply impulse to current rigidbodies velocities if they are enabled
                            if (physicBodies[i]->rigidbody.enabled) 
                            {
                                // Calculate inverted mass ration
                                ratio = physicBodies[i]->rigidbody.mass/massSum;
                                
                                // Apply impulse direction to velocity
                                physicBodies[i]->rigidbody.velocity.x -= impulse.x*ratio*(1.0f+physicBodies[i]->rigidbody.bounciness);
                                physicBodies[i]->rigidbody.velocity.y -= impulse.y*ratio*(1.0f+physicBodies[i]->rigidbody.bounciness);
                            }
                            
                            if (physicBodies[k]->rigidbody.enabled) 
                            {
                                // Calculate inverted mass ration
                                ratio = physicBodies[k]->rigidbody.mass/massSum;
                                
                                // Apply impulse direction to velocity
                                physicBodies[k]->rigidbody.velocity.x += impulse.x*ratio*(1.0f+physicBodies[i]->rigidbody.bounciness);
                                physicBodies[k]->rigidbody.velocity.y += impulse.y*ratio*(1.0f+physicBodies[i]->rigidbody.bounciness);
                            }
                            
                            // 3. Correct colliders overlaping (transform position)
                            // ---------------------------------------------------------------------------------------------------------------------------------
                            
                            // Calculate transform position penetration correction
                            Vector2 posCorrection;
                            posCorrection.x = penetrationDepth/((1.0f/physicBodies[i]->rigidbody.mass) + (1.0f/physicBodies[k]->rigidbody.mass))*PHYSICS_ERRORPERCENT*contactNormal.x;
                            posCorrection.y = penetrationDepth/((1.0f/physicBodies[i]->rigidbody.mass) + (1.0f/physicBodies[k]->rigidbody.mass))*PHYSICS_ERRORPERCENT*contactNormal.y;
                            
                            // Fix transform positions
                            if (physicBodies[i]->rigidbody.enabled)
                            {                                        
                                // Fix physic objects transform position
                                physicBodies[i]->transform.position.x -= 1.0f/physicBodies[i]->rigidbody.mass*posCorrection.x;
                                physicBodies[i]->transform.position.y += 1.0f/physicBodies[i]->rigidbody.mass*posCorrection.y;
                                
                                // Update collider bounds
                                physicBodies[i]->collider.bounds = TransformToRectangle(physicBodies[i]->transform);
                                
                                if (physicBodies[k]->rigidbody.enabled)
                                {
                                    // Fix physic objects transform position
                                    physicBodies[k]->transform.position.x += 1.0f/physicBodies[k]->rigidbody.mass*posCorrection.x;
                                    physicBodies[k]->transform.position.y -= 1.0f/physicBodies[k]->rigidbody.mass*posCorrection.y;
                                    
                                    // Update collider bounds
                                    physicBodies[k]->collider.bounds = TransformToRectangle(physicBodies[k]->transform);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#endif  // PHYSAC_IMPLEMENTATION