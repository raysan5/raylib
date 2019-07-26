/**********************************************************************************************
*
*   Physac v1.0 - 2D Physics library for videogames
*
*   DESCRIPTION:
*
*   Physac is a small 2D physics engine written in pure C. The engine uses a fixed time-step thread loop
*   to simluate physics. A physics step contains the following phases: get collision information,
*   apply dynamics, collision solving and position correction. It uses a very simple struct for physic
*   bodies with a position vector to be used in any 3D rendering API.
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
*   #define PHYSAC_DEBUG
*       Traces log messages when creating and destroying physics bodies and detects errors in physics
*       calculations and reference exceptions; it is useful for debug purposes
*
*   #define PHYSAC_MALLOC()
*   #define PHYSAC_FREE()
*       You can define your own malloc/free implementation replacing stdlib.h malloc()/free() functions.
*       Otherwise it will include stdlib.h and use the C standard library malloc()/free() function.
*
*
*   NOTE 1: Physac requires multi-threading, when InitPhysics() a second thread is created to manage physics calculations.
*   NOTE 2: Physac requires static C library linkage to avoid dependency on MinGW DLL (-static -lpthread)
*
*   Use the following code to compile:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) -s -static -lraylib -lpthread -lopengl32 -lgdi32 -lwinmm -std=c99
*
*   VERY THANKS TO:
*       Ramon Santamaria (github: @raysan5)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016-2018 Victor Fisac (github: @victorfisac)
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

#if !defined(PHYSAC_H)
#define PHYSAC_H

// #define PHYSAC_STATIC
// #define  PHYSAC_NO_THREADS
// #define  PHYSAC_STANDALONE
// #define  PHYSAC_DEBUG

#if defined(PHYSAC_STATIC)
    #define PHYSACDEF static            // Functions just visible to module including this file
#else
    #if defined(__cplusplus)
        #define PHYSACDEF extern "C"    // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define PHYSACDEF extern        // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define     PHYSAC_MAX_BODIES               64
#define     PHYSAC_MAX_MANIFOLDS            4096
#define     PHYSAC_MAX_VERTICES             24
#define     PHYSAC_CIRCLE_VERTICES          24

#define     PHYSAC_COLLISION_ITERATIONS     100
#define     PHYSAC_PENETRATION_ALLOWANCE    0.05f
#define     PHYSAC_PENETRATION_CORRECTION   0.4f

#define     PHYSAC_PI                       3.14159265358979323846
#define     PHYSAC_DEG2RAD                  (PHYSAC_PI/180.0f)

#define     PHYSAC_MALLOC(size)             malloc(size)
#define     PHYSAC_FREE(ptr)                free(ptr)

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for PHYSAC_STANDALONE usage
//----------------------------------------------------------------------------------
#if defined(PHYSAC_STANDALONE)
    // Vector2 type
    typedef struct Vector2 {
        float x;
        float y;
    } Vector2;

    // Boolean type
    #if !defined(_STDBOOL_H)
        typedef enum { false, true } bool;
        #define _STDBOOL_H
    #endif
#endif

typedef enum PhysicsShapeType { PHYSICS_CIRCLE, PHYSICS_POLYGON } PhysicsShapeType;

// Previously defined to be used in PhysicsShape struct as circular dependencies
typedef struct PhysicsBodyData *PhysicsBody;

// Mat2 type (used for polygon shape rotation matrix)
typedef struct Mat2 {
    float m00;
    float m01;
    float m10;
    float m11;
} Mat2;

typedef struct PolygonData {
    unsigned int vertexCount;                   // Current used vertex and normals count
    Vector2 positions[PHYSAC_MAX_VERTICES];     // Polygon vertex positions vectors
    Vector2 normals[PHYSAC_MAX_VERTICES];       // Polygon vertex normals vectors
} PolygonData;

typedef struct PhysicsShape {
    PhysicsShapeType type;                      // Physics shape type (circle or polygon)
    PhysicsBody body;                           // Shape physics body reference
    float radius;                               // Circle shape radius (used for circle shapes)
    Mat2 transform;                             // Vertices transform matrix 2x2
    PolygonData vertexData;                     // Polygon shape vertices position and normals data (just used for polygon shapes)
} PhysicsShape;

typedef struct PhysicsBodyData {
    unsigned int id;                            // Reference unique identifier
    bool enabled;                               // Enabled dynamics state (collisions are calculated anyway)
    Vector2 position;                           // Physics body shape pivot
    Vector2 velocity;                           // Current linear velocity applied to position
    Vector2 force;                              // Current linear force (reset to 0 every step)
    float angularVelocity;                      // Current angular velocity applied to orient
    float torque;                               // Current angular force (reset to 0 every step)
    float orient;                               // Rotation in radians
    float inertia;                              // Moment of inertia
    float inverseInertia;                       // Inverse value of inertia
    float mass;                                 // Physics body mass
    float inverseMass;                          // Inverse value of mass
    float staticFriction;                       // Friction when the body has not movement (0 to 1)
    float dynamicFriction;                      // Friction when the body has movement (0 to 1)
    float restitution;                          // Restitution coefficient of the body (0 to 1)
    bool useGravity;                            // Apply gravity force to dynamics
    bool isGrounded;                            // Physics grounded on other body state
    bool freezeOrient;                          // Physics rotation constraint
    PhysicsShape shape;                         // Physics body shape information (type, radius, vertices, normals)
} PhysicsBodyData;

typedef struct PhysicsManifoldData {
    unsigned int id;                            // Reference unique identifier
    PhysicsBody bodyA;                          // Manifold first physics body reference
    PhysicsBody bodyB;                          // Manifold second physics body reference
    float penetration;                          // Depth of penetration from collision
    Vector2 normal;                             // Normal direction vector from 'a' to 'b'
    Vector2 contacts[2];                        // Points of contact during collision
    unsigned int contactsCount;                 // Current collision number of contacts
    float restitution;                          // Mixed restitution during collision
    float dynamicFriction;                      // Mixed dynamic friction during collision
    float staticFriction;                       // Mixed static friction during collision
} PhysicsManifoldData, *PhysicsManifold;

#if defined(__cplusplus)
extern "C" {                                    // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
PHYSACDEF void InitPhysics(void);                                                                           // Initializes physics values, pointers and creates physics loop thread
PHYSACDEF void RunPhysicsStep(void);                                                                        // Run physics step, to be used if PHYSICS_NO_THREADS is set in your main loop
PHYSACDEF void SetPhysicsTimeStep(double delta);                                                            // Sets physics fixed time step in milliseconds. 1.666666 by default
PHYSACDEF bool IsPhysicsEnabled(void);                                                                      // Returns true if physics thread is currently enabled
PHYSACDEF void SetPhysicsGravity(float x, float y);                                                         // Sets physics global gravity force
PHYSACDEF PhysicsBody CreatePhysicsBodyCircle(Vector2 pos, float radius, float density);                    // Creates a new circle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyRectangle(Vector2 pos, float width, float height, float density);    // Creates a new rectangle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyPolygon(Vector2 pos, float radius, int sides, float density);        // Creates a new polygon physics body with generic parameters
PHYSACDEF void PhysicsAddForce(PhysicsBody body, Vector2 force);                                            // Adds a force to a physics body
PHYSACDEF void PhysicsAddTorque(PhysicsBody body, float amount);                                            // Adds an angular force to a physics body
PHYSACDEF void PhysicsShatter(PhysicsBody body, Vector2 position, float force);                             // Shatters a polygon shape physics body to little physics bodies with explosion force
PHYSACDEF int GetPhysicsBodiesCount(void);                                                                  // Returns the current amount of created physics bodies
PHYSACDEF PhysicsBody GetPhysicsBody(int index);                                                            // Returns a physics body of the bodies pool at a specific index
PHYSACDEF int GetPhysicsShapeType(int index);                                                               // Returns the physics body shape type (PHYSICS_CIRCLE or PHYSICS_POLYGON)
PHYSACDEF int GetPhysicsShapeVerticesCount(int index);                                                      // Returns the amount of vertices of a physics body shape
PHYSACDEF Vector2 GetPhysicsShapeVertex(PhysicsBody body, int vertex);                                      // Returns transformed position of a body shape (body position + vertex transformed position)
PHYSACDEF void SetPhysicsBodyRotation(PhysicsBody body, float radians);                                     // Sets physics body shape transform based on radians parameter
PHYSACDEF void DestroyPhysicsBody(PhysicsBody body);                                                        // Unitializes and destroy a physics body
PHYSACDEF void ResetPhysics(void);                                                                          // Destroys created physics bodies and manifolds and resets global values
PHYSACDEF void ClosePhysics(void);                                                                          // Unitializes physics pointers and closes physics loop thread

#if defined(__cplusplus)
}
#endif

#endif // PHYSAC_H

/***********************************************************************************
*
*   PHYSAC IMPLEMENTATION
*
************************************************************************************/

#if defined(PHYSAC_IMPLEMENTATION)

#if !defined(PHYSAC_NO_THREADS)
    #include <pthread.h>            // Required for: pthread_t, pthread_create()
#endif

#if defined(PHYSAC_DEBUG)
    #include <stdio.h>              // Required for: printf()
#endif

#include <stdlib.h>                 // Required for: malloc(), free(), srand(), rand()
#include <math.h>                   // Required for: cosf(), sinf(), fabs(), sqrtf()
#include <stdint.h>                 // Required for: uint64_t

#if !defined(PHYSAC_STANDALONE)
    #include "raymath.h"            // Required for: Vector2Add(), Vector2Subtract()
#endif

// Time management functionality
#include <time.h>                   // Required for: time(), clock_gettime()
#if defined(_WIN32)
    // Functions required to query time on Windows
    int __stdcall QueryPerformanceCounter(unsigned long long int *lpPerformanceCount);
    int __stdcall QueryPerformanceFrequency(unsigned long long int *lpFrequency);
#elif defined(__linux__)
    #if _POSIX_C_SOURCE < 199309L
        #undef _POSIX_C_SOURCE
        #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
    #endif
    #include <sys/time.h>           // Required for: timespec
#elif defined(__APPLE__)            // macOS also defines __MACH__
    #include <mach/mach_time.h>     // Required for: mach_absolute_time()
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define     min(a,b)                    (((a)<(b))?(a):(b))
#define     max(a,b)                    (((a)>(b))?(a):(b))
#define     PHYSAC_FLT_MAX              3.402823466e+38f
#define     PHYSAC_EPSILON              0.000001f
#define     PHYSAC_K                    1.0f/3.0f
#define     PHYSAC_VECTOR_ZERO          (Vector2){ 0.0f, 0.0f }

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if !defined(PHYSAC_NO_THREADS)
static pthread_t physicsThreadId;                           // Physics thread id
#endif
static unsigned int usedMemory = 0;                         // Total allocated dynamic memory
static bool physicsThreadEnabled = false;                   // Physics thread enabled state
static double baseTime = 0.0;                               // Offset time for MONOTONIC clock
static double startTime = 0.0;                              // Start time in milliseconds
static double deltaTime = 1.0/60.0/10.0 * 1000;             // Delta time used for physics steps, in milliseconds
static double currentTime = 0.0;                            // Current time in milliseconds
static uint64_t frequency = 0;                              // Hi-res clock frequency

static double accumulator = 0.0;                            // Physics time step delta time accumulator
static unsigned int stepsCount = 0;                         // Total physics steps processed
static Vector2 gravityForce = { 0.0f, 9.81f };              // Physics world gravity force
static PhysicsBody bodies[PHYSAC_MAX_BODIES];               // Physics bodies pointers array
static unsigned int physicsBodiesCount = 0;                 // Physics world current bodies counter
static PhysicsManifold contacts[PHYSAC_MAX_MANIFOLDS];      // Physics bodies pointers array
static unsigned int physicsManifoldsCount = 0;              // Physics world current manifolds counter

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
static int FindAvailableBodyIndex();                                                                        // Finds a valid index for a new physics body initialization
static PolygonData CreateRandomPolygon(float radius, int sides);                                            // Creates a random polygon shape with max vertex distance from polygon pivot
static PolygonData CreateRectanglePolygon(Vector2 pos, Vector2 size);                                       // Creates a rectangle polygon shape based on a min and max positions
static void *PhysicsLoop(void *arg);                                                                        // Physics loop thread function
static void PhysicsStep(void);                                                                              // Physics steps calculations (dynamics, collisions and position corrections)
static int FindAvailableManifoldIndex();                                                                    // Finds a valid index for a new manifold initialization
static PhysicsManifold CreatePhysicsManifold(PhysicsBody a, PhysicsBody b);                                 // Creates a new physics manifold to solve collision
static void DestroyPhysicsManifold(PhysicsManifold manifold);                                               // Unitializes and destroys a physics manifold
static void SolvePhysicsManifold(PhysicsManifold manifold);                                                 // Solves a created physics manifold between two physics bodies
static void SolveCircleToCircle(PhysicsManifold manifold);                                                  // Solves collision between two circle shape physics bodies
static void SolveCircleToPolygon(PhysicsManifold manifold);                                                 // Solves collision between a circle to a polygon shape physics bodies
static void SolvePolygonToCircle(PhysicsManifold manifold);                                                 // Solves collision between a polygon to a circle shape physics bodies
static void SolvePolygonToPolygon(PhysicsManifold manifold);                                                // Solves collision between two polygons shape physics bodies
static void IntegratePhysicsForces(PhysicsBody body);                                                       // Integrates physics forces into velocity
static void InitializePhysicsManifolds(PhysicsManifold manifold);                                           // Initializes physics manifolds to solve collisions
static void IntegratePhysicsImpulses(PhysicsManifold manifold);                                             // Integrates physics collisions impulses to solve collisions
static void IntegratePhysicsVelocity(PhysicsBody body);                                                     // Integrates physics velocity into position and forces
static void CorrectPhysicsPositions(PhysicsManifold manifold);                                              // Corrects physics bodies positions based on manifolds collision information
static float FindAxisLeastPenetration(int *faceIndex, PhysicsShape shapeA, PhysicsShape shapeB);            // Finds polygon shapes axis least penetration
static void FindIncidentFace(Vector2 *v0, Vector2 *v1, PhysicsShape ref, PhysicsShape inc, int index);      // Finds two polygon shapes incident face
static int Clip(Vector2 normal, float clip, Vector2 *faceA, Vector2 *faceB);                                // Calculates clipping based on a normal and two faces
static bool BiasGreaterThan(float valueA, float valueB);                                                    // Check if values are between bias range
static Vector2 TriangleBarycenter(Vector2 v1, Vector2 v2, Vector2 v3);                                      // Returns the barycenter of a triangle given by 3 points

static void InitTimer(void);                                                                                // Initializes hi-resolution MONOTONIC timer
static uint64_t GetTimeCount(void);                                                                         // Get hi-res MONOTONIC time measure in mseconds
static double GetCurrentTime(void);                                                                         // Get current time measure in milliseconds

// Math functions
static Vector2 MathCross(float value, Vector2 vector);                                                      // Returns the cross product of a vector and a value
static float MathCrossVector2(Vector2 v1, Vector2 v2);                                                      // Returns the cross product of two vectors
static float MathLenSqr(Vector2 vector);                                                                    // Returns the len square root of a vector
static float MathDot(Vector2 v1, Vector2 v2);                                                               // Returns the dot product of two vectors
static inline float DistSqr(Vector2 v1, Vector2 v2);                                                        // Returns the square root of distance between two vectors
static void MathNormalize(Vector2 *vector);                                                                 // Returns the normalized values of a vector
#if defined(PHYSAC_STANDALONE)
static Vector2 Vector2Add(Vector2 v1, Vector2 v2);                                                          // Returns the sum of two given vectors
static Vector2 Vector2Subtract(Vector2 v1, Vector2 v2);                                                     // Returns the subtract of two given vectors
#endif

static Mat2 Mat2Radians(float radians);                                                                     // Creates a matrix 2x2 from a given radians value
static void Mat2Set(Mat2 *matrix, float radians);                                                           // Set values from radians to a created matrix 2x2
static inline Mat2 Mat2Transpose(Mat2 matrix);                                                              // Returns the transpose of a given matrix 2x2
static inline Vector2 Mat2MultiplyVector2(Mat2 matrix, Vector2 vector);                                     // Multiplies a vector by a matrix 2x2

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Initializes physics values, pointers and creates physics loop thread
PHYSACDEF void InitPhysics(void)
{
    #if !defined(PHYSAC_NO_THREADS)
        // NOTE: if defined, user will need to create a thread for PhysicsThread function manually
        // Create physics thread using POSIXS thread libraries
        pthread_create(&physicsThreadId, NULL, &PhysicsLoop, NULL);
    #endif

    // Initialize high resolution timer
    InitTimer();

    #if defined(PHYSAC_DEBUG)
        printf("[PHYSAC] physics module initialized successfully\n");
    #endif

    accumulator = 0.0;
}

// Returns true if physics thread is currently enabled
PHYSACDEF bool IsPhysicsEnabled(void)
{
    return physicsThreadEnabled;
}

// Sets physics global gravity force
PHYSACDEF void SetPhysicsGravity(float x, float y)
{
    gravityForce.x = x;
    gravityForce.y = y;
}

// Creates a new circle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyCircle(Vector2 pos, float radius, float density)
{
    PhysicsBody newBody = CreatePhysicsBodyPolygon(pos, radius, PHYSAC_CIRCLE_VERTICES, density);
    return newBody;
}

// Creates a new rectangle physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyRectangle(Vector2 pos, float width, float height, float density)
{
    PhysicsBody newBody = (PhysicsBody)PHYSAC_MALLOC(sizeof(PhysicsBodyData));
    usedMemory += sizeof(PhysicsBodyData);

    int newId = FindAvailableBodyIndex();
    if (newId != -1)
    {
        // Initialize new body with generic values
        newBody->id = newId;
        newBody->enabled = true;
        newBody->position = pos;
        newBody->velocity = (Vector2){ 0.0f };
        newBody->force = (Vector2){ 0.0f };
        newBody->angularVelocity = 0.0f;
        newBody->torque = 0.0f;
        newBody->orient = 0.0f;
        newBody->shape.type = PHYSICS_POLYGON;
        newBody->shape.body = newBody;
        newBody->shape.radius = 0.0f;
        newBody->shape.transform = Mat2Radians(0.0f);
        newBody->shape.vertexData = CreateRectanglePolygon(pos, (Vector2){ width, height });

        // Calculate centroid and moment of inertia
        Vector2 center = { 0.0f, 0.0f };
        float area = 0.0f;
        float inertia = 0.0f;

        for (int i = 0; i < newBody->shape.vertexData.vertexCount; i++)
        {
            // Triangle vertices, third vertex implied as (0, 0)
            Vector2 p1 = newBody->shape.vertexData.positions[i];
            int nextIndex = (((i + 1) < newBody->shape.vertexData.vertexCount) ? (i + 1) : 0);
            Vector2 p2 = newBody->shape.vertexData.positions[nextIndex];

            float D = MathCrossVector2(p1, p2);
            float triangleArea = D/2;

            area += triangleArea;

            // Use area to weight the centroid average, not just vertex position
            center.x += triangleArea*PHYSAC_K*(p1.x + p2.x);
            center.y += triangleArea*PHYSAC_K*(p1.y + p2.y);

            float intx2 = p1.x*p1.x + p2.x*p1.x + p2.x*p2.x;
            float inty2 = p1.y*p1.y + p2.y*p1.y + p2.y*p2.y;
            inertia += (0.25f*PHYSAC_K*D)*(intx2 + inty2);
        }

        center.x *= 1.0f/area;
        center.y *= 1.0f/area;

        // Translate vertices to centroid (make the centroid (0, 0) for the polygon in model space)
        // Note: this is not really necessary
        for (int i = 0; i < newBody->shape.vertexData.vertexCount; i++)
        {
            newBody->shape.vertexData.positions[i].x -= center.x;
            newBody->shape.vertexData.positions[i].y -= center.y;
        }

        newBody->mass = density*area;
        newBody->inverseMass = ((newBody->mass != 0.0f) ? 1.0f/newBody->mass : 0.0f);
        newBody->inertia = density*inertia;
        newBody->inverseInertia = ((newBody->inertia != 0.0f) ? 1.0f/newBody->inertia : 0.0f);
        newBody->staticFriction = 0.4f;
        newBody->dynamicFriction = 0.2f;
        newBody->restitution = 0.0f;
        newBody->useGravity = true;
        newBody->isGrounded = false;
        newBody->freezeOrient = false;

        // Add new body to bodies pointers array and update bodies count
        bodies[physicsBodiesCount] = newBody;
        physicsBodiesCount++;

        #if defined(PHYSAC_DEBUG)
            printf("[PHYSAC] created polygon physics body id %i\n", newBody->id);
        #endif
    }
    #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] new physics body creation failed because there is any available id to use\n");
    #endif

    return newBody;
}

// Creates a new polygon physics body with generic parameters
PHYSACDEF PhysicsBody CreatePhysicsBodyPolygon(Vector2 pos, float radius, int sides, float density)
{
    PhysicsBody newBody = (PhysicsBody)PHYSAC_MALLOC(sizeof(PhysicsBodyData));
    usedMemory += sizeof(PhysicsBodyData);

    int newId = FindAvailableBodyIndex();
    if (newId != -1)
    {
        // Initialize new body with generic values
        newBody->id = newId;
        newBody->enabled = true;
        newBody->position = pos;
        newBody->velocity = PHYSAC_VECTOR_ZERO;
        newBody->force = PHYSAC_VECTOR_ZERO;
        newBody->angularVelocity = 0.0f;
        newBody->torque = 0.0f;
        newBody->orient = 0.0f;
        newBody->shape.type = PHYSICS_POLYGON;
        newBody->shape.body = newBody;
        newBody->shape.transform = Mat2Radians(0.0f);
        newBody->shape.vertexData = CreateRandomPolygon(radius, sides);

        // Calculate centroid and moment of inertia
        Vector2 center = { 0.0f, 0.0f };
        float area = 0.0f;
        float inertia = 0.0f;

        for (int i = 0; i < newBody->shape.vertexData.vertexCount; i++)
        {
            // Triangle vertices, third vertex implied as (0, 0)
            Vector2 position1 = newBody->shape.vertexData.positions[i];
            int nextIndex = (((i + 1) < newBody->shape.vertexData.vertexCount) ? (i + 1) : 0);
            Vector2 position2 = newBody->shape.vertexData.positions[nextIndex];

            float cross = MathCrossVector2(position1, position2);
            float triangleArea = cross/2;

            area += triangleArea;

            // Use area to weight the centroid average, not just vertex position
            center.x += triangleArea*PHYSAC_K*(position1.x + position2.x);
            center.y += triangleArea*PHYSAC_K*(position1.y + position2.y);

            float intx2 = position1.x*position1.x + position2.x*position1.x + position2.x*position2.x;
            float inty2 = position1.y*position1.y + position2.y*position1.y + position2.y*position2.y;
            inertia += (0.25f*PHYSAC_K*cross)*(intx2 + inty2);
        }

        center.x *= 1.0f/area;
        center.y *= 1.0f/area;

        // Translate vertices to centroid (make the centroid (0, 0) for the polygon in model space)
        // Note: this is not really necessary
        for (int i = 0; i < newBody->shape.vertexData.vertexCount; i++)
        {
            newBody->shape.vertexData.positions[i].x -= center.x;
            newBody->shape.vertexData.positions[i].y -= center.y;
        }

        newBody->mass = density*area;
        newBody->inverseMass = ((newBody->mass != 0.0f) ? 1.0f/newBody->mass : 0.0f);
        newBody->inertia = density*inertia;
        newBody->inverseInertia = ((newBody->inertia != 0.0f) ? 1.0f/newBody->inertia : 0.0f);
        newBody->staticFriction = 0.4f;
        newBody->dynamicFriction = 0.2f;
        newBody->restitution = 0.0f;
        newBody->useGravity = true;
        newBody->isGrounded = false;
        newBody->freezeOrient = false;

        // Add new body to bodies pointers array and update bodies count
        bodies[physicsBodiesCount] = newBody;
        physicsBodiesCount++;

        #if defined(PHYSAC_DEBUG)
            printf("[PHYSAC] created polygon physics body id %i\n", newBody->id);
        #endif
    }
    #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] new physics body creation failed because there is any available id to use\n");
    #endif

    return newBody;
}

// Adds a force to a physics body
PHYSACDEF void PhysicsAddForce(PhysicsBody body, Vector2 force)
{
    if (body != NULL) body->force = Vector2Add(body->force, force);
}

// Adds an angular force to a physics body
PHYSACDEF void PhysicsAddTorque(PhysicsBody body, float amount)
{
    if (body != NULL) body->torque += amount;
}

// Shatters a polygon shape physics body to little physics bodies with explosion force
PHYSACDEF void PhysicsShatter(PhysicsBody body, Vector2 position, float force)
{
    if (body != NULL)
    {
        if (body->shape.type == PHYSICS_POLYGON)
        {
            PolygonData vertexData = body->shape.vertexData;
            bool collision = false;

            for (int i = 0; i < vertexData.vertexCount; i++)
            {
                Vector2 positionA = body->position;
                Vector2 positionB = Mat2MultiplyVector2(body->shape.transform, Vector2Add(body->position, vertexData.positions[i]));
                int nextIndex = (((i + 1) < vertexData.vertexCount) ? (i + 1) : 0);
                Vector2 positionC = Mat2MultiplyVector2(body->shape.transform, Vector2Add(body->position, vertexData.positions[nextIndex]));

                // Check collision between each triangle
                float alpha = ((positionB.y - positionC.y)*(position.x - positionC.x) + (positionC.x - positionB.x)*(position.y - positionC.y))/
                              ((positionB.y - positionC.y)*(positionA.x - positionC.x) + (positionC.x - positionB.x)*(positionA.y - positionC.y));

                float beta = ((positionC.y - positionA.y)*(position.x - positionC.x) + (positionA.x - positionC.x)*(position.y - positionC.y))/
                             ((positionB.y - positionC.y)*(positionA.x - positionC.x) + (positionC.x - positionB.x)*(positionA.y - positionC.y));

                float gamma = 1.0f - alpha - beta;

                if ((alpha > 0.0f) && (beta > 0.0f) & (gamma > 0.0f))
                {
                    collision = true;
                    break;
                }
            }

            if (collision)
            {
                int count = vertexData.vertexCount;
                Vector2 bodyPos = body->position;
                Vector2 *vertices = (Vector2*)malloc(sizeof(Vector2) * count);
                Mat2 trans = body->shape.transform;
                for (int i = 0; i < count; i++) vertices[i] = vertexData.positions[i];

                // Destroy shattered physics body
                DestroyPhysicsBody(body);

                for (int i = 0; i < count; i++)
                {
                    int nextIndex = (((i + 1) < count) ? (i + 1) : 0);
                    Vector2 center = TriangleBarycenter(vertices[i], vertices[nextIndex], PHYSAC_VECTOR_ZERO);
                    center = Vector2Add(bodyPos, center);
                    Vector2 offset = Vector2Subtract(center, bodyPos);

                    PhysicsBody newBody = CreatePhysicsBodyPolygon(center, 10, 3, 10);     // Create polygon physics body with relevant values

                    PolygonData newData = { 0 };
                    newData.vertexCount = 3;

                    newData.positions[0] = Vector2Subtract(vertices[i], offset);
                    newData.positions[1] = Vector2Subtract(vertices[nextIndex], offset);
                    newData.positions[2] = Vector2Subtract(position, center);

                    // Separate vertices to avoid unnecessary physics collisions
                    newData.positions[0].x *= 0.95f;
                    newData.positions[0].y *= 0.95f;
                    newData.positions[1].x *= 0.95f;
                    newData.positions[1].y *= 0.95f;
                    newData.positions[2].x *= 0.95f;
                    newData.positions[2].y *= 0.95f;

                    // Calculate polygon faces normals
                    for (int j = 0; j < newData.vertexCount; j++)
                    {
                        int nextVertex = (((j + 1) < newData.vertexCount) ? (j + 1) : 0);
                        Vector2 face = Vector2Subtract(newData.positions[nextVertex], newData.positions[j]);

                        newData.normals[j] = (Vector2){ face.y, -face.x };
                        MathNormalize(&newData.normals[j]);
                    }

                    // Apply computed vertex data to new physics body shape
                    newBody->shape.vertexData = newData;
                    newBody->shape.transform = trans;

                    // Calculate centroid and moment of inertia
                    center = PHYSAC_VECTOR_ZERO;
                    float area = 0.0f;
                    float inertia = 0.0f;

                    for (int j = 0; j < newBody->shape.vertexData.vertexCount; j++)
                    {
                        // Triangle vertices, third vertex implied as (0, 0)
                        Vector2 p1 = newBody->shape.vertexData.positions[j];
                        int nextVertex = (((j + 1) < newBody->shape.vertexData.vertexCount) ? (j + 1) : 0);
                        Vector2 p2 = newBody->shape.vertexData.positions[nextVertex];

                        float D = MathCrossVector2(p1, p2);
                        float triangleArea = D/2;

                        area += triangleArea;

                        // Use area to weight the centroid average, not just vertex position
                        center.x += triangleArea*PHYSAC_K*(p1.x + p2.x);
                        center.y += triangleArea*PHYSAC_K*(p1.y + p2.y);

                        float intx2 = p1.x*p1.x + p2.x*p1.x + p2.x*p2.x;
                        float inty2 = p1.y*p1.y + p2.y*p1.y + p2.y*p2.y;
                        inertia += (0.25f*PHYSAC_K*D)*(intx2 + inty2);
                    }

                    center.x *= 1.0f/area;
                    center.y *= 1.0f/area;

                    newBody->mass = area;
                    newBody->inverseMass = ((newBody->mass != 0.0f) ? 1.0f/newBody->mass : 0.0f);
                    newBody->inertia = inertia;
                    newBody->inverseInertia = ((newBody->inertia != 0.0f) ? 1.0f/newBody->inertia : 0.0f);

                    // Calculate explosion force direction
                    Vector2 pointA = newBody->position;
                    Vector2 pointB = Vector2Subtract(newData.positions[1], newData.positions[0]);
                    pointB.x /= 2.0f;
                    pointB.y /= 2.0f;
                    Vector2 forceDirection = Vector2Subtract(Vector2Add(pointA, Vector2Add(newData.positions[0], pointB)), newBody->position);
                    MathNormalize(&forceDirection);
                    forceDirection.x *= force;
                    forceDirection.y *= force;

                    // Apply force to new physics body
                    PhysicsAddForce(newBody, forceDirection);
                }

                free(vertices);
            }
        }
    }
    #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] error when trying to shatter a null reference physics body");
    #endif
}

// Returns the current amount of created physics bodies
PHYSACDEF int GetPhysicsBodiesCount(void)
{
    return physicsBodiesCount;
}

// Returns a physics body of the bodies pool at a specific index
PHYSACDEF PhysicsBody GetPhysicsBody(int index)
{
    PhysicsBody body = NULL;

    if (index < physicsBodiesCount)
    {
        body = bodies[index];

        if (body == NULL)
        {
            #if defined(PHYSAC_DEBUG)
                printf("[PHYSAC] error when trying to get a null reference physics body");
            #endif
        }
    }
    #if defined(PHYSAC_DEBUG)
    else printf("[PHYSAC] physics body index is out of bounds");
    #endif

    return body;
}

// Returns the physics body shape type (PHYSICS_CIRCLE or PHYSICS_POLYGON)
PHYSACDEF int GetPhysicsShapeType(int index)
{
    int result = -1;

    if (index < physicsBodiesCount)
    {
        PhysicsBody body = bodies[index];

        if (body != NULL) result = body->shape.type;
        #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] error when trying to get a null reference physics body");
        #endif
    }
    #if defined(PHYSAC_DEBUG)
    else printf("[PHYSAC] physics body index is out of bounds");
    #endif

    return result;
}

// Returns the amount of vertices of a physics body shape
PHYSACDEF int GetPhysicsShapeVerticesCount(int index)
{
    int result = 0;

    if (index < physicsBodiesCount)
    {
        PhysicsBody body = bodies[index];

        if (body != NULL)
        {
            switch (body->shape.type)
            {
                case PHYSICS_CIRCLE: result = PHYSAC_CIRCLE_VERTICES; break;
                case PHYSICS_POLYGON: result = body->shape.vertexData.vertexCount; break;
                default: break;
            }
        }
        #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] error when trying to get a null reference physics body");
        #endif
    }
    #if defined(PHYSAC_DEBUG)
    else printf("[PHYSAC] physics body index is out of bounds");
    #endif

    return result;
}

// Returns transformed position of a body shape (body position + vertex transformed position)
PHYSACDEF Vector2 GetPhysicsShapeVertex(PhysicsBody body, int vertex)
{
    Vector2 position = { 0.0f, 0.0f };

    if (body != NULL)
    {
        switch (body->shape.type)
        {
            case PHYSICS_CIRCLE:
            {
                position.x = body->position.x + cosf(360.0f/PHYSAC_CIRCLE_VERTICES*vertex*PHYSAC_DEG2RAD)*body->shape.radius;
                position.y = body->position.y + sinf(360.0f/PHYSAC_CIRCLE_VERTICES*vertex*PHYSAC_DEG2RAD)*body->shape.radius;
            } break;
            case PHYSICS_POLYGON:
            {
                PolygonData vertexData = body->shape.vertexData;
                position = Vector2Add(body->position, Mat2MultiplyVector2(body->shape.transform, vertexData.positions[vertex]));
            } break;
            default: break;
        }
    }
    #if defined(PHYSAC_DEBUG)
    else printf("[PHYSAC] error when trying to get a null reference physics body");
    #endif

    return position;
}

// Sets physics body shape transform based on radians parameter
PHYSACDEF void SetPhysicsBodyRotation(PhysicsBody body, float radians)
{
    if (body != NULL)
    {
        body->orient = radians;

        if (body->shape.type == PHYSICS_POLYGON) body->shape.transform = Mat2Radians(radians);
    }
}

// Unitializes and destroys a physics body
PHYSACDEF void DestroyPhysicsBody(PhysicsBody body)
{
    if (body != NULL)
    {
        int id = body->id;
        int index = -1;

        for (int i = 0; i < physicsBodiesCount; i++)
        {
            if (bodies[i]->id == id)
            {
                index = i;
                break;
            }
        }

        #if defined(PHYSAC_DEBUG)
        if (index == -1) printf("[PHYSAC] cannot find body id %i in pointers array\n", id);
        #endif

        // Free body allocated memory
        PHYSAC_FREE(body);
        usedMemory -= sizeof(PhysicsBodyData);
        bodies[index] = NULL;

        // Reorder physics bodies pointers array and its catched index
        for (int i = index; i < physicsBodiesCount; i++)
        {
            if ((i + 1) < physicsBodiesCount) bodies[i] = bodies[i + 1];
        }

        // Update physics bodies count
        physicsBodiesCount--;

        #if defined(PHYSAC_DEBUG)
            printf("[PHYSAC] destroyed physics body id %i\n", id);
        #endif
    }
    #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] error trying to destroy a null referenced body\n");
    #endif
}

// Destroys created physics bodies and manifolds and resets global values
PHYSACDEF void ResetPhysics(void)
{
    // Unitialize physics bodies dynamic memory allocations
    for (int i = physicsBodiesCount - 1; i >= 0; i--)
    {
        PhysicsBody body = bodies[i];

        if (body != NULL)
        {
            PHYSAC_FREE(body);
            bodies[i] = NULL;
            usedMemory -= sizeof(PhysicsBodyData);
        }
    }

    physicsBodiesCount = 0;

    // Unitialize physics manifolds dynamic memory allocations
    for (int i = physicsManifoldsCount - 1; i >= 0; i--)
    {
        PhysicsManifold manifold = contacts[i];

        if (manifold != NULL)
        {
            PHYSAC_FREE(manifold);
            contacts[i] = NULL;
            usedMemory -= sizeof(PhysicsManifoldData);
        }
    }

    physicsManifoldsCount = 0;

    #if defined(PHYSAC_DEBUG)
        printf("[PHYSAC] physics module reset successfully\n");
    #endif
}

// Unitializes physics pointers and exits physics loop thread
PHYSACDEF void ClosePhysics(void)
{
    // Exit physics loop thread
    physicsThreadEnabled = false;

    #if !defined(PHYSAC_NO_THREADS)
        pthread_join(physicsThreadId, NULL);
    #endif

    // Unitialize physics manifolds dynamic memory allocations
    for (int i = physicsManifoldsCount - 1; i >= 0; i--) DestroyPhysicsManifold(contacts[i]);

    // Unitialize physics bodies dynamic memory allocations
    for (int i = physicsBodiesCount - 1; i >= 0; i--) DestroyPhysicsBody(bodies[i]);

    #if defined(PHYSAC_DEBUG)
        if (physicsBodiesCount > 0 || usedMemory != 0) printf("[PHYSAC] physics module closed with %i still allocated bodies [MEMORY: %i bytes]\n", physicsBodiesCount, usedMemory);
        else if (physicsManifoldsCount > 0 || usedMemory != 0) printf("[PHYSAC] physics module closed with %i still allocated manifolds [MEMORY: %i bytes]\n", physicsManifoldsCount, usedMemory);
        else printf("[PHYSAC] physics module closed successfully\n");
    #endif
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------
// Finds a valid index for a new physics body initialization
static int FindAvailableBodyIndex()
{
    int index = -1;
    for (int i = 0; i < PHYSAC_MAX_BODIES; i++)
    {
        int currentId = i;

        // Check if current id already exist in other physics body
        for (int k = 0; k < physicsBodiesCount; k++)
        {
            if (bodies[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If it is not used, use it as new physics body id
        if (currentId == i)
        {
            index = i;
            break;
        }
    }

    return index;
}

// Creates a random polygon shape with max vertex distance from polygon pivot
static PolygonData CreateRandomPolygon(float radius, int sides)
{
    PolygonData data = { 0 };
    data.vertexCount = sides;

    // Calculate polygon vertices positions
    for (int i = 0; i < data.vertexCount; i++)
    {
        data.positions[i].x = cosf(360.0f/sides*i*PHYSAC_DEG2RAD)*radius;
        data.positions[i].y = sinf(360.0f/sides*i*PHYSAC_DEG2RAD)*radius;
    }

    // Calculate polygon faces normals
    for (int i = 0; i < data.vertexCount; i++)
    {
        int nextIndex = (((i + 1) < sides) ? (i + 1) : 0);
        Vector2 face = Vector2Subtract(data.positions[nextIndex], data.positions[i]);

        data.normals[i] = (Vector2){ face.y, -face.x };
        MathNormalize(&data.normals[i]);
    }

    return data;
}

// Creates a rectangle polygon shape based on a min and max positions
static PolygonData CreateRectanglePolygon(Vector2 pos, Vector2 size)
{
    PolygonData data = { 0 };
    data.vertexCount = 4;

    // Calculate polygon vertices positions
    data.positions[0] = (Vector2){ pos.x + size.x/2, pos.y - size.y/2 };
    data.positions[1] = (Vector2){ pos.x + size.x/2, pos.y + size.y/2 };
    data.positions[2] = (Vector2){ pos.x - size.x/2, pos.y + size.y/2 };
    data.positions[3] = (Vector2){ pos.x - size.x/2, pos.y - size.y/2 };

    // Calculate polygon faces normals
    for (int i = 0; i < data.vertexCount; i++)
    {
        int nextIndex = (((i + 1) < data.vertexCount) ? (i + 1) : 0);
        Vector2 face = Vector2Subtract(data.positions[nextIndex], data.positions[i]);

        data.normals[i] = (Vector2){ face.y, -face.x };
        MathNormalize(&data.normals[i]);
    }

    return data;
}

// Physics loop thread function
static void *PhysicsLoop(void *arg)
{
    #if defined(PHYSAC_DEBUG)
        printf("[PHYSAC] physics thread created successfully\n");
    #endif

    // Initialize physics loop thread values
    physicsThreadEnabled = true;

    // Physics update loop
    while (physicsThreadEnabled)
    {
        RunPhysicsStep();
    }

    return NULL;
}

// Physics steps calculations (dynamics, collisions and position corrections)
static void PhysicsStep(void)
{
    // Update current steps count
    stepsCount++;

    // Clear previous generated collisions information
    for (int i = physicsManifoldsCount - 1; i >= 0; i--)
    {
        PhysicsManifold manifold = contacts[i];
        if (manifold != NULL) DestroyPhysicsManifold(manifold);
    }

    // Reset physics bodies grounded state
    for (int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        body->isGrounded = false;
    }

    // Generate new collision information
    for (int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody bodyA = bodies[i];

        if (bodyA != NULL)
        {
            for (int j = i + 1; j < physicsBodiesCount; j++)
            {
                PhysicsBody bodyB = bodies[j];

                if (bodyB != NULL)
                {
                    if ((bodyA->inverseMass == 0) && (bodyB->inverseMass == 0)) continue;

                    PhysicsManifold manifold = CreatePhysicsManifold(bodyA, bodyB);
                    SolvePhysicsManifold(manifold);

                    if (manifold->contactsCount > 0)
                    {
                        // Create a new manifold with same information as previously solved manifold and add it to the manifolds pool last slot
                        PhysicsManifold newManifold = CreatePhysicsManifold(bodyA, bodyB);
                        newManifold->penetration = manifold->penetration;
                        newManifold->normal = manifold->normal;
                        newManifold->contacts[0] = manifold->contacts[0];
                        newManifold->contacts[1] = manifold->contacts[1];
                        newManifold->contactsCount = manifold->contactsCount;
                        newManifold->restitution = manifold->restitution;
                        newManifold->dynamicFriction = manifold->dynamicFriction;
                        newManifold->staticFriction = manifold->staticFriction;
                    }
                }
            }
        }
    }

    // Integrate forces to physics bodies
    for (int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        if (body != NULL) IntegratePhysicsForces(body);
    }

    // Initialize physics manifolds to solve collisions
    for (int i = 0; i < physicsManifoldsCount; i++)
    {
        PhysicsManifold manifold = contacts[i];
        if (manifold != NULL) InitializePhysicsManifolds(manifold);
    }

    // Integrate physics collisions impulses to solve collisions
    for (int i = 0; i < PHYSAC_COLLISION_ITERATIONS; i++)
    {
        for (int j = 0; j < physicsManifoldsCount; j++)
        {
            PhysicsManifold manifold = contacts[i];
            if (manifold != NULL) IntegratePhysicsImpulses(manifold);
        }
    }

    // Integrate velocity to physics bodies
    for (int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        if (body != NULL) IntegratePhysicsVelocity(body);
    }

    // Correct physics bodies positions based on manifolds collision information
    for (int i = 0; i < physicsManifoldsCount; i++)
    {
        PhysicsManifold manifold = contacts[i];
        if (manifold != NULL) CorrectPhysicsPositions(manifold);
    }

    // Clear physics bodies forces
    for (int i = 0; i < physicsBodiesCount; i++)
    {
        PhysicsBody body = bodies[i];
        if (body != NULL)
        {
            body->force = PHYSAC_VECTOR_ZERO;
            body->torque = 0.0f;
        }
    }
}

// Wrapper to ensure PhysicsStep is run with at a fixed time step
PHYSACDEF void RunPhysicsStep(void)
{
    // Calculate current time
    currentTime = GetCurrentTime();

    // Calculate current delta time
    const double delta = currentTime - startTime;

    // Store the time elapsed since the last frame began
    accumulator += delta;

    // Fixed time stepping loop
    while (accumulator >= deltaTime)
    {
#ifdef PHYSAC_DEBUG
        //printf("currentTime %f, startTime %f, accumulator-pre %f, accumulator-post %f, delta %f, deltaTime %f\n",
        //       currentTime, startTime, accumulator, accumulator-deltaTime, delta, deltaTime);
#endif
        PhysicsStep();
        accumulator -= deltaTime;
    }

    // Record the starting of this frame
    startTime = currentTime;
}

PHYSACDEF void SetPhysicsTimeStep(double delta)
{
    deltaTime = delta;
}

// Finds a valid index for a new manifold initialization
static int FindAvailableManifoldIndex()
{
    int index = -1;
    for (int i = 0; i < PHYSAC_MAX_MANIFOLDS; i++)
    {
        int currentId = i;

        // Check if current id already exist in other physics body
        for (int k = 0; k < physicsManifoldsCount; k++)
        {
            if (contacts[k]->id == currentId)
            {
                currentId++;
                break;
            }
        }

        // If it is not used, use it as new physics body id
        if (currentId == i)
        {
            index = i;
            break;
        }
    }

    return index;
}

// Creates a new physics manifold to solve collision
static PhysicsManifold CreatePhysicsManifold(PhysicsBody a, PhysicsBody b)
{
    PhysicsManifold newManifold = (PhysicsManifold)PHYSAC_MALLOC(sizeof(PhysicsManifoldData));
    usedMemory += sizeof(PhysicsManifoldData);

    int newId = FindAvailableManifoldIndex();
    if (newId != -1)
    {
        // Initialize new manifold with generic values
        newManifold->id = newId;
        newManifold->bodyA = a;
        newManifold->bodyB = b;
        newManifold->penetration = 0;
        newManifold->normal = PHYSAC_VECTOR_ZERO;
        newManifold->contacts[0] = PHYSAC_VECTOR_ZERO;
        newManifold->contacts[1] = PHYSAC_VECTOR_ZERO;
        newManifold->contactsCount = 0;
        newManifold->restitution = 0.0f;
        newManifold->dynamicFriction = 0.0f;
        newManifold->staticFriction = 0.0f;

        // Add new body to bodies pointers array and update bodies count
        contacts[physicsManifoldsCount] = newManifold;
        physicsManifoldsCount++;
    }
    #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] new physics manifold creation failed because there is any available id to use\n");
    #endif

    return newManifold;
}

// Unitializes and destroys a physics manifold
static void DestroyPhysicsManifold(PhysicsManifold manifold)
{
    if (manifold != NULL)
    {
        int id = manifold->id;
        int index = -1;

        for (int i = 0; i < physicsManifoldsCount; i++)
        {
            if (contacts[i]->id == id)
            {
                index = i;
                break;
            }
        }

        #if defined(PHYSAC_DEBUG)
            if (index == -1) printf("[PHYSAC] cannot find manifold id %i in pointers array\n", id);
        #endif

        // Free manifold allocated memory
        PHYSAC_FREE(manifold);
        usedMemory -= sizeof(PhysicsManifoldData);
        contacts[index] = NULL;

        // Reorder physics manifolds pointers array and its catched index
        for (int i = index; i < physicsManifoldsCount; i++)
        {
            if ((i + 1) < physicsManifoldsCount) contacts[i] = contacts[i + 1];
        }

        // Update physics manifolds count
        physicsManifoldsCount--;
    }
    #if defined(PHYSAC_DEBUG)
        else printf("[PHYSAC] error trying to destroy a null referenced manifold\n");
    #endif
}

// Solves a created physics manifold between two physics bodies
static void SolvePhysicsManifold(PhysicsManifold manifold)
{
    switch (manifold->bodyA->shape.type)
    {
        case PHYSICS_CIRCLE:
        {
            switch (manifold->bodyB->shape.type)
            {
                case PHYSICS_CIRCLE: SolveCircleToCircle(manifold); break;
                case PHYSICS_POLYGON: SolveCircleToPolygon(manifold); break;
                default: break;
            }
        } break;
        case PHYSICS_POLYGON:
        {
            switch (manifold->bodyB->shape.type)
            {
                case PHYSICS_CIRCLE: SolvePolygonToCircle(manifold); break;
                case PHYSICS_POLYGON: SolvePolygonToPolygon(manifold); break;
                default: break;
            }
        } break;
        default: break;
    }

    // Update physics body grounded state if normal direction is down and grounded state is not set yet in previous manifolds
    if (!manifold->bodyB->isGrounded) manifold->bodyB->isGrounded = (manifold->normal.y < 0);
}

// Solves collision between two circle shape physics bodies
static void SolveCircleToCircle(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    // Calculate translational vector, which is normal
    Vector2 normal = Vector2Subtract(bodyB->position, bodyA->position);

    float distSqr = MathLenSqr(normal);
    float radius = bodyA->shape.radius + bodyB->shape.radius;

    // Check if circles are not in contact
    if (distSqr >= radius*radius)
    {
        manifold->contactsCount = 0;
        return;
    }

    float distance = sqrtf(distSqr);
    manifold->contactsCount = 1;

    if (distance == 0.0f)
    {
        manifold->penetration = bodyA->shape.radius;
        manifold->normal = (Vector2){ 1.0f, 0.0f };
        manifold->contacts[0] = bodyA->position;
    }
    else
    {
        manifold->penetration = radius - distance;
        manifold->normal = (Vector2){ normal.x/distance, normal.y/distance }; // Faster than using MathNormalize() due to sqrt is already performed
        manifold->contacts[0] = (Vector2){ manifold->normal.x*bodyA->shape.radius + bodyA->position.x, manifold->normal.y*bodyA->shape.radius + bodyA->position.y };
    }

    // Update physics body grounded state if normal direction is down
    if (!bodyA->isGrounded) bodyA->isGrounded = (manifold->normal.y < 0);
}

// Solves collision between a circle to a polygon shape physics bodies
static void SolveCircleToPolygon(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    manifold->contactsCount = 0;

    // Transform circle center to polygon transform space
    Vector2 center = bodyA->position;
    center = Mat2MultiplyVector2(Mat2Transpose(bodyB->shape.transform), Vector2Subtract(center, bodyB->position));

    // Find edge with minimum penetration
    // It is the same concept as using support points in SolvePolygonToPolygon
    float separation = -PHYSAC_FLT_MAX;
    int faceNormal = 0;
    PolygonData vertexData = bodyB->shape.vertexData;

    for (int i = 0; i < vertexData.vertexCount; i++)
    {
        float currentSeparation = MathDot(vertexData.normals[i], Vector2Subtract(center, vertexData.positions[i]));

        if (currentSeparation > bodyA->shape.radius) return;

        if (currentSeparation > separation)
        {
            separation = currentSeparation;
            faceNormal = i;
        }
    }

    // Grab face's vertices
    Vector2 v1 = vertexData.positions[faceNormal];
    int nextIndex = (((faceNormal + 1) < vertexData.vertexCount) ? (faceNormal + 1) : 0);
    Vector2 v2 = vertexData.positions[nextIndex];

    // Check to see if center is within polygon
    if (separation < PHYSAC_EPSILON)
    {
        manifold->contactsCount = 1;
        Vector2 normal = Mat2MultiplyVector2(bodyB->shape.transform, vertexData.normals[faceNormal]);
        manifold->normal = (Vector2){ -normal.x, -normal.y };
        manifold->contacts[0] = (Vector2){ manifold->normal.x*bodyA->shape.radius + bodyA->position.x, manifold->normal.y*bodyA->shape.radius + bodyA->position.y };
        manifold->penetration = bodyA->shape.radius;
        return;
    }

    // Determine which voronoi region of the edge center of circle lies within
    float dot1 = MathDot(Vector2Subtract(center, v1), Vector2Subtract(v2, v1));
    float dot2 = MathDot(Vector2Subtract(center, v2), Vector2Subtract(v1, v2));
    manifold->penetration = bodyA->shape.radius - separation;

    if (dot1 <= 0.0f) // Closest to v1
    {
        if (DistSqr(center, v1) > bodyA->shape.radius*bodyA->shape.radius) return;

        manifold->contactsCount = 1;
        Vector2 normal = Vector2Subtract(v1, center);
        normal = Mat2MultiplyVector2(bodyB->shape.transform, normal);
        MathNormalize(&normal);
        manifold->normal = normal;
        v1 = Mat2MultiplyVector2(bodyB->shape.transform, v1);
        v1 = Vector2Add(v1, bodyB->position);
        manifold->contacts[0] = v1;
    }
    else if (dot2 <= 0.0f) // Closest to v2
    {
        if (DistSqr(center, v2) > bodyA->shape.radius*bodyA->shape.radius) return;

        manifold->contactsCount = 1;
        Vector2 normal = Vector2Subtract(v2, center);
        v2 = Mat2MultiplyVector2(bodyB->shape.transform, v2);
        v2 = Vector2Add(v2, bodyB->position);
        manifold->contacts[0] = v2;
        normal = Mat2MultiplyVector2(bodyB->shape.transform, normal);
        MathNormalize(&normal);
        manifold->normal = normal;
    }
    else // Closest to face
    {
        Vector2 normal = vertexData.normals[faceNormal];

        if (MathDot(Vector2Subtract(center, v1), normal) > bodyA->shape.radius) return;

        normal = Mat2MultiplyVector2(bodyB->shape.transform, normal);
        manifold->normal = (Vector2){ -normal.x, -normal.y };
        manifold->contacts[0] = (Vector2){ manifold->normal.x*bodyA->shape.radius + bodyA->position.x, manifold->normal.y*bodyA->shape.radius + bodyA->position.y };
        manifold->contactsCount = 1;
    }
}

// Solves collision between a polygon to a circle shape physics bodies
static void SolvePolygonToCircle(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    manifold->bodyA = bodyB;
    manifold->bodyB = bodyA;
    SolveCircleToPolygon(manifold);

    manifold->normal.x *= -1.0f;
    manifold->normal.y *= -1.0f;
}

// Solves collision between two polygons shape physics bodies
static void SolvePolygonToPolygon(PhysicsManifold manifold)
{
    if ((manifold->bodyA == NULL) || (manifold->bodyB == NULL)) return;

    PhysicsShape bodyA = manifold->bodyA->shape;
    PhysicsShape bodyB = manifold->bodyB->shape;
    manifold->contactsCount = 0;

    // Check for separating axis with A shape's face planes
    int faceA = 0;
    float penetrationA = FindAxisLeastPenetration(&faceA, bodyA, bodyB);
    if (penetrationA >= 0.0f) return;

    // Check for separating axis with B shape's face planes
    int faceB = 0;
    float penetrationB = FindAxisLeastPenetration(&faceB, bodyB, bodyA);
    if (penetrationB >= 0.0f) return;

    int referenceIndex = 0;
    bool flip = false;  // Always point from A shape to B shape

    PhysicsShape refPoly; // Reference
    PhysicsShape incPoly; // Incident

    // Determine which shape contains reference face
    if (BiasGreaterThan(penetrationA, penetrationB))
    {
        refPoly = bodyA;
        incPoly = bodyB;
        referenceIndex = faceA;
    }
    else
    {
        refPoly = bodyB;
        incPoly = bodyA;
        referenceIndex = faceB;
        flip = true;
    }

    // World space incident face
    Vector2 incidentFace[2];
    FindIncidentFace(&incidentFace[0], &incidentFace[1], refPoly, incPoly, referenceIndex);

    // Setup reference face vertices
    PolygonData refData = refPoly.vertexData;
    Vector2 v1 = refData.positions[referenceIndex];
    referenceIndex = (((referenceIndex + 1) < refData.vertexCount) ? (referenceIndex + 1) : 0);
    Vector2 v2 = refData.positions[referenceIndex];

    // Transform vertices to world space
    v1 = Mat2MultiplyVector2(refPoly.transform, v1);
    v1 = Vector2Add(v1, refPoly.body->position);
    v2 = Mat2MultiplyVector2(refPoly.transform, v2);
    v2 = Vector2Add(v2, refPoly.body->position);

    // Calculate reference face side normal in world space
    Vector2 sidePlaneNormal = Vector2Subtract(v2, v1);
    MathNormalize(&sidePlaneNormal);

    // Orthogonalize
    Vector2 refFaceNormal = { sidePlaneNormal.y, -sidePlaneNormal.x };
    float refC = MathDot(refFaceNormal, v1);
    float negSide = MathDot(sidePlaneNormal, v1)*-1;
    float posSide = MathDot(sidePlaneNormal, v2);

    // Clip incident face to reference face side planes (due to floating point error, possible to not have required points
    if (Clip((Vector2){ -sidePlaneNormal.x, -sidePlaneNormal.y }, negSide, &incidentFace[0], &incidentFace[1]) < 2) return;
    if (Clip(sidePlaneNormal, posSide, &incidentFace[0], &incidentFace[1]) < 2) return;

    // Flip normal if required
    manifold->normal = (flip ? (Vector2){ -refFaceNormal.x, -refFaceNormal.y } : refFaceNormal);

    // Keep points behind reference face
    int currentPoint = 0; // Clipped points behind reference face
    float separation = MathDot(refFaceNormal, incidentFace[0]) - refC;
    if (separation <= 0.0f)
    {
        manifold->contacts[currentPoint] = incidentFace[0];
        manifold->penetration = -separation;
        currentPoint++;
    }
    else manifold->penetration = 0.0f;

    separation = MathDot(refFaceNormal, incidentFace[1]) - refC;

    if (separation <= 0.0f)
    {
        manifold->contacts[currentPoint] = incidentFace[1];
        manifold->penetration += -separation;
        currentPoint++;

        // Calculate total penetration average
        manifold->penetration /= currentPoint;
    }

    manifold->contactsCount = currentPoint;
}

// Integrates physics forces into velocity
static void IntegratePhysicsForces(PhysicsBody body)
{
    if ((body == NULL) || (body->inverseMass == 0.0f) || !body->enabled) return;

    body->velocity.x += (body->force.x*body->inverseMass)*(deltaTime/2.0);
    body->velocity.y += (body->force.y*body->inverseMass)*(deltaTime/2.0);

    if (body->useGravity)
    {
        body->velocity.x += gravityForce.x*(deltaTime/1000/2.0);
        body->velocity.y += gravityForce.y*(deltaTime/1000/2.0);
    }

    if (!body->freezeOrient) body->angularVelocity += body->torque*body->inverseInertia*(deltaTime/2.0);
}

// Initializes physics manifolds to solve collisions
static void InitializePhysicsManifolds(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    // Calculate average restitution, static and dynamic friction
    manifold->restitution = sqrtf(bodyA->restitution*bodyB->restitution);
    manifold->staticFriction = sqrtf(bodyA->staticFriction*bodyB->staticFriction);
    manifold->dynamicFriction = sqrtf(bodyA->dynamicFriction*bodyB->dynamicFriction);

    for (int i = 0; i < manifold->contactsCount; i++)
    {
        // Caculate radius from center of mass to contact
        Vector2 radiusA = Vector2Subtract(manifold->contacts[i], bodyA->position);
        Vector2 radiusB = Vector2Subtract(manifold->contacts[i], bodyB->position);

        Vector2 crossA = MathCross(bodyA->angularVelocity, radiusA);
        Vector2 crossB = MathCross(bodyB->angularVelocity, radiusB);

        Vector2 radiusV = { 0.0f, 0.0f };
        radiusV.x = bodyB->velocity.x + crossB.x - bodyA->velocity.x - crossA.x;
        radiusV.y = bodyB->velocity.y + crossB.y - bodyA->velocity.y - crossA.y;

        // Determine if we should perform a resting collision or not;
        // The idea is if the only thing moving this object is gravity, then the collision should be performed without any restitution
        if (MathLenSqr(radiusV) < (MathLenSqr((Vector2){ gravityForce.x*deltaTime/1000, gravityForce.y*deltaTime/1000 }) + PHYSAC_EPSILON)) manifold->restitution = 0;
    }
}

// Integrates physics collisions impulses to solve collisions
static void IntegratePhysicsImpulses(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    // Early out and positional correct if both objects have infinite mass
    if (fabs(bodyA->inverseMass + bodyB->inverseMass) <= PHYSAC_EPSILON)
    {
        bodyA->velocity = PHYSAC_VECTOR_ZERO;
        bodyB->velocity = PHYSAC_VECTOR_ZERO;
        return;
    }

    for (int i = 0; i < manifold->contactsCount; i++)
    {
        // Calculate radius from center of mass to contact
        Vector2 radiusA = Vector2Subtract(manifold->contacts[i], bodyA->position);
        Vector2 radiusB = Vector2Subtract(manifold->contacts[i], bodyB->position);

        // Calculate relative velocity
        Vector2 radiusV = { 0.0f, 0.0f };
        radiusV.x = bodyB->velocity.x + MathCross(bodyB->angularVelocity, radiusB).x - bodyA->velocity.x - MathCross(bodyA->angularVelocity, radiusA).x;
        radiusV.y = bodyB->velocity.y + MathCross(bodyB->angularVelocity, radiusB).y - bodyA->velocity.y - MathCross(bodyA->angularVelocity, radiusA).y;

        // Relative velocity along the normal
        float contactVelocity = MathDot(radiusV, manifold->normal);

        // Do not resolve if velocities are separating
        if (contactVelocity > 0.0f) return;

        float raCrossN = MathCrossVector2(radiusA, manifold->normal);
        float rbCrossN = MathCrossVector2(radiusB, manifold->normal);

        float inverseMassSum = bodyA->inverseMass + bodyB->inverseMass + (raCrossN*raCrossN)*bodyA->inverseInertia + (rbCrossN*rbCrossN)*bodyB->inverseInertia;

        // Calculate impulse scalar value
        float impulse = -(1.0f + manifold->restitution)*contactVelocity;
        impulse /= inverseMassSum;
        impulse /= (float)manifold->contactsCount;

        // Apply impulse to each physics body
        Vector2 impulseV = { manifold->normal.x*impulse, manifold->normal.y*impulse };

        if (bodyA->enabled)
        {
            bodyA->velocity.x += bodyA->inverseMass*(-impulseV.x);
            bodyA->velocity.y += bodyA->inverseMass*(-impulseV.y);
            if (!bodyA->freezeOrient) bodyA->angularVelocity += bodyA->inverseInertia*MathCrossVector2(radiusA, (Vector2){ -impulseV.x, -impulseV.y });
        }

        if (bodyB->enabled)
        {
            bodyB->velocity.x += bodyB->inverseMass*(impulseV.x);
            bodyB->velocity.y += bodyB->inverseMass*(impulseV.y);
            if (!bodyB->freezeOrient) bodyB->angularVelocity += bodyB->inverseInertia*MathCrossVector2(radiusB, impulseV);
        }

        // Apply friction impulse to each physics body
        radiusV.x = bodyB->velocity.x + MathCross(bodyB->angularVelocity, radiusB).x - bodyA->velocity.x - MathCross(bodyA->angularVelocity, radiusA).x;
        radiusV.y = bodyB->velocity.y + MathCross(bodyB->angularVelocity, radiusB).y - bodyA->velocity.y - MathCross(bodyA->angularVelocity, radiusA).y;

        Vector2 tangent = { radiusV.x - (manifold->normal.x*MathDot(radiusV, manifold->normal)), radiusV.y - (manifold->normal.y*MathDot(radiusV, manifold->normal)) };
        MathNormalize(&tangent);

        // Calculate impulse tangent magnitude
        float impulseTangent = -MathDot(radiusV, tangent);
        impulseTangent /= inverseMassSum;
        impulseTangent /= (float)manifold->contactsCount;

        float absImpulseTangent = fabs(impulseTangent);

        // Don't apply tiny friction impulses
        if (absImpulseTangent <= PHYSAC_EPSILON) return;

        // Apply coulumb's law
        Vector2 tangentImpulse = { 0.0f, 0.0f };
        if (absImpulseTangent < impulse*manifold->staticFriction) tangentImpulse = (Vector2){ tangent.x*impulseTangent, tangent.y*impulseTangent };
        else tangentImpulse = (Vector2){ tangent.x*-impulse*manifold->dynamicFriction, tangent.y*-impulse*manifold->dynamicFriction };

        // Apply friction impulse
        if (bodyA->enabled)
        {
            bodyA->velocity.x += bodyA->inverseMass*(-tangentImpulse.x);
            bodyA->velocity.y += bodyA->inverseMass*(-tangentImpulse.y);

            if (!bodyA->freezeOrient) bodyA->angularVelocity += bodyA->inverseInertia*MathCrossVector2(radiusA, (Vector2){ -tangentImpulse.x, -tangentImpulse.y });
        }

        if (bodyB->enabled)
        {
            bodyB->velocity.x += bodyB->inverseMass*(tangentImpulse.x);
            bodyB->velocity.y += bodyB->inverseMass*(tangentImpulse.y);

            if (!bodyB->freezeOrient) bodyB->angularVelocity += bodyB->inverseInertia*MathCrossVector2(radiusB, tangentImpulse);
        }
    }
}

// Integrates physics velocity into position and forces
static void IntegratePhysicsVelocity(PhysicsBody body)
{
    if ((body == NULL) ||!body->enabled) return;

    body->position.x += body->velocity.x*deltaTime;
    body->position.y += body->velocity.y*deltaTime;

    if (!body->freezeOrient) body->orient += body->angularVelocity*deltaTime;
    Mat2Set(&body->shape.transform, body->orient);

    IntegratePhysicsForces(body);
}

// Corrects physics bodies positions based on manifolds collision information
static void CorrectPhysicsPositions(PhysicsManifold manifold)
{
    PhysicsBody bodyA = manifold->bodyA;
    PhysicsBody bodyB = manifold->bodyB;

    if ((bodyA == NULL) || (bodyB == NULL)) return;

    Vector2 correction = { 0.0f, 0.0f };
    correction.x = (max(manifold->penetration - PHYSAC_PENETRATION_ALLOWANCE, 0.0f)/(bodyA->inverseMass + bodyB->inverseMass))*manifold->normal.x*PHYSAC_PENETRATION_CORRECTION;
    correction.y = (max(manifold->penetration - PHYSAC_PENETRATION_ALLOWANCE, 0.0f)/(bodyA->inverseMass + bodyB->inverseMass))*manifold->normal.y*PHYSAC_PENETRATION_CORRECTION;

    if (bodyA->enabled)
    {
        bodyA->position.x -= correction.x*bodyA->inverseMass;
        bodyA->position.y -= correction.y*bodyA->inverseMass;
    }

    if (bodyB->enabled)
    {
        bodyB->position.x += correction.x*bodyB->inverseMass;
        bodyB->position.y += correction.y*bodyB->inverseMass;
    }
}

// Returns the extreme point along a direction within a polygon
static Vector2 GetSupport(PhysicsShape shape, Vector2 dir)
{
    float bestProjection = -PHYSAC_FLT_MAX;
    Vector2 bestVertex = { 0.0f, 0.0f };
    PolygonData data = shape.vertexData;

    for (int i = 0; i < data.vertexCount; i++)
    {
        Vector2 vertex = data.positions[i];
        float projection = MathDot(vertex, dir);

        if (projection > bestProjection)
        {
            bestVertex = vertex;
            bestProjection = projection;
        }
    }

    return bestVertex;
}

// Finds polygon shapes axis least penetration
static float FindAxisLeastPenetration(int *faceIndex, PhysicsShape shapeA, PhysicsShape shapeB)
{
    float bestDistance = -PHYSAC_FLT_MAX;
    int bestIndex = 0;

    PolygonData dataA = shapeA.vertexData;
    //PolygonData dataB = shapeB.vertexData;

    for (int i = 0; i < dataA.vertexCount; i++)
    {
        // Retrieve a face normal from A shape
        Vector2 normal = dataA.normals[i];
        Vector2 transNormal = Mat2MultiplyVector2(shapeA.transform, normal);

        // Transform face normal into B shape's model space
        Mat2 buT = Mat2Transpose(shapeB.transform);
        normal = Mat2MultiplyVector2(buT, transNormal);

        // Retrieve support point from B shape along -n
        Vector2 support = GetSupport(shapeB, (Vector2){ -normal.x, -normal.y });

        // Retrieve vertex on face from A shape, transform into B shape's model space
        Vector2 vertex = dataA.positions[i];
        vertex = Mat2MultiplyVector2(shapeA.transform, vertex);
        vertex = Vector2Add(vertex, shapeA.body->position);
        vertex = Vector2Subtract(vertex, shapeB.body->position);
        vertex = Mat2MultiplyVector2(buT, vertex);

        // Compute penetration distance in B shape's model space
        float distance = MathDot(normal, Vector2Subtract(support, vertex));

        // Store greatest distance
        if (distance > bestDistance)
        {
            bestDistance = distance;
            bestIndex = i;
        }
    }

    *faceIndex = bestIndex;
    return bestDistance;
}

// Finds two polygon shapes incident face
static void FindIncidentFace(Vector2 *v0, Vector2 *v1, PhysicsShape ref, PhysicsShape inc, int index)
{
    PolygonData refData = ref.vertexData;
    PolygonData incData = inc.vertexData;

    Vector2 referenceNormal = refData.normals[index];

    // Calculate normal in incident's frame of reference
    referenceNormal = Mat2MultiplyVector2(ref.transform, referenceNormal); // To world space
    referenceNormal = Mat2MultiplyVector2(Mat2Transpose(inc.transform), referenceNormal); // To incident's model space

    // Find most anti-normal face on polygon
    int incidentFace = 0;
    float minDot = PHYSAC_FLT_MAX;

    for (int i = 0; i < incData.vertexCount; i++)
    {
        float dot = MathDot(referenceNormal, incData.normals[i]);

        if (dot < minDot)
        {
            minDot = dot;
            incidentFace = i;
        }
    }

    // Assign face vertices for incident face
    *v0 = Mat2MultiplyVector2(inc.transform, incData.positions[incidentFace]);
    *v0 = Vector2Add(*v0, inc.body->position);
    incidentFace = (((incidentFace + 1) < incData.vertexCount) ? (incidentFace + 1) : 0);
    *v1 = Mat2MultiplyVector2(inc.transform, incData.positions[incidentFace]);
    *v1 = Vector2Add(*v1, inc.body->position);
}

// Calculates clipping based on a normal and two faces
static int Clip(Vector2 normal, float clip, Vector2 *faceA, Vector2 *faceB)
{
    int sp = 0;
    Vector2 out[2] = { *faceA, *faceB };

    // Retrieve distances from each endpoint to the line
    float distanceA = MathDot(normal, *faceA) - clip;
    float distanceB = MathDot(normal, *faceB) - clip;

    // If negative (behind plane)
    if (distanceA <= 0.0f) out[sp++] = *faceA;
    if (distanceB <= 0.0f) out[sp++] = *faceB;

    // If the points are on different sides of the plane
    if ((distanceA*distanceB) < 0.0f)
    {
        // Push intersection point
        float alpha = distanceA/(distanceA - distanceB);
        out[sp] = *faceA;
        Vector2 delta = Vector2Subtract(*faceB, *faceA);
        delta.x *= alpha;
        delta.y *= alpha;
        out[sp] = Vector2Add(out[sp], delta);
        sp++;
    }

    // Assign the new converted values
    *faceA = out[0];
    *faceB = out[1];

    return sp;
}

// Check if values are between bias range
static bool BiasGreaterThan(float valueA, float valueB)
{
    return (valueA >= (valueB*0.95f + valueA*0.01f));
}

// Returns the barycenter of a triangle given by 3 points
static Vector2 TriangleBarycenter(Vector2 v1, Vector2 v2, Vector2 v3)
{
    Vector2 result = { 0.0f, 0.0f };

    result.x = (v1.x + v2.x + v3.x)/3;
    result.y = (v1.y + v2.y + v3.y)/3;

    return result;
}

// Initializes hi-resolution MONOTONIC timer
static void InitTimer(void)
{
    srand(time(NULL));              // Initialize random seed
    
#if defined(_WIN32)
    QueryPerformanceFrequency((unsigned long long int *) &frequency);
#endif

#if defined(__linux__)
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0) frequency = 1000000000;
#endif

#if defined(__APPLE__)
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    frequency = (timebase.denom*1e9)/timebase.numer;
#endif
    
    baseTime = GetTimeCount();      // Get MONOTONIC clock time offset
    startTime = GetCurrentTime();   // Get current time
}

// Get hi-res MONOTONIC time measure in seconds
static uint64_t GetTimeCount(void)
{
    uint64_t value = 0;
    
#if defined(_WIN32)
    QueryPerformanceCounter((unsigned long long int *) &value);
#endif

#if defined(__linux__)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    value = (uint64_t)now.tv_sec*(uint64_t)1000000000 + (uint64_t)now.tv_nsec;
#endif

#if defined(__APPLE__)
    value = mach_absolute_time();
#endif

    return value;
}

// Get current time in milliseconds
static double GetCurrentTime(void)
{
    return (double)(GetTimeCount() - baseTime)/frequency*1000;
}

// Returns the cross product of a vector and a value
static inline Vector2 MathCross(float value, Vector2 vector)
{
    return (Vector2){ -value*vector.y, value*vector.x };
}

// Returns the cross product of two vectors
static inline float MathCrossVector2(Vector2 v1, Vector2 v2)
{
    return (v1.x*v2.y - v1.y*v2.x);
}

// Returns the len square root of a vector
static inline float MathLenSqr(Vector2 vector)
{
    return (vector.x*vector.x + vector.y*vector.y);
}

// Returns the dot product of two vectors
static inline float MathDot(Vector2 v1, Vector2 v2)
{
    return (v1.x*v2.x + v1.y*v2.y);
}

// Returns the square root of distance between two vectors
static inline float DistSqr(Vector2 v1, Vector2 v2)
{
    Vector2 dir = Vector2Subtract(v1, v2);
    return MathDot(dir, dir);
}

// Returns the normalized values of a vector
static void MathNormalize(Vector2 *vector)
{
    float length, ilength;

    Vector2 aux = *vector;
    length = sqrtf(aux.x*aux.x + aux.y*aux.y);

    if (length == 0) length = 1.0f;

    ilength = 1.0f/length;

    vector->x *= ilength;
    vector->y *= ilength;
}

#if defined(PHYSAC_STANDALONE)
// Returns the sum of two given vectors
static inline Vector2 Vector2Add(Vector2 v1, Vector2 v2)
{
    return (Vector2){ v1.x + v2.x, v1.y + v2.y };
}

// Returns the subtract of two given vectors
static inline Vector2 Vector2Subtract(Vector2 v1, Vector2 v2)
{
    return (Vector2){ v1.x - v2.x, v1.y - v2.y };
}
#endif

// Creates a matrix 2x2 from a given radians value
static Mat2 Mat2Radians(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    return (Mat2){ c, -s, s, c };
}

// Set values from radians to a created matrix 2x2
static void Mat2Set(Mat2 *matrix, float radians)
{
    float cos = cosf(radians);
    float sin = sinf(radians);

    matrix->m00 = cos;
    matrix->m01 = -sin;
    matrix->m10 = sin;
    matrix->m11 = cos;
}

// Returns the transpose of a given matrix 2x2
static inline Mat2 Mat2Transpose(Mat2 matrix)
{
    return (Mat2){ matrix.m00, matrix.m10, matrix.m01, matrix.m11 };
}

// Multiplies a vector by a matrix 2x2
static inline Vector2 Mat2MultiplyVector2(Mat2 matrix, Vector2 vector)
{
    return (Vector2){ matrix.m00*vector.x + matrix.m01*vector.y, matrix.m10*vector.x + matrix.m11*vector.y };
}

#endif  // PHYSAC_IMPLEMENTATION
