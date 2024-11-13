/**********************************************************************************************
*
*   raymath v2.0 - Math functions to work with Vector2, Vector3, Matrix and Quaternions
*
*   CONVENTIONS:
*     - Matrix structure is defined as row-major (memory layout) but parameters naming AND all
*       math operations performed by the library consider the structure as it was column-major
*       It is like transposed versions of the matrices are used for all the maths
*       It benefits some functions making them cache-friendly and also avoids matrix
*       transpositions sometimes required by OpenGL
*       Example: In memory order, row0 is [m0 m4 m8 m12] but in semantic math row0 is [m0 m1 m2 m3]
*     - Functions are always self-contained, no function use another raymath function inside,
*       required code is directly re-implemented inside
*     - Functions input parameters are always received by value (2 unavoidable exceptions)
*     - Functions use always a "result" variable for return (except C++ operators)
*     - Functions are always defined inline
*     - Angles are always in radians (DEG2RAD/RAD2DEG macros provided for convenience)
*     - No compound literals used to make sure libray is compatible with C++
*
*   CONFIGURATION:
*       #define RAYMATH_IMPLEMENTATION
*           Generates the implementation of the library into the included file.
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation.
*
*       #define RAYMATH_STATIC_INLINE
*           Define static inline functions code, so #include header suffices for use.
*           This may use up lots of memory.
*
*       #define RAYMATH_DISABLE_CPP_OPERATORS
*           Disables C++ operator overloads for raymath types.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2015-2024 Ramon Santamaria (@raysan5)
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

#ifndef RAYMATH_H
#define RAYMATH_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>       // Required for: sinf(), cosf(), tan(), atan2f(), sqrtf(), floor(), fminf(), fmaxf(), fabsf()

#if defined(RAYMATH_IMPLEMENTATION) && defined(RAYMATH_STATIC_INLINE)
    #error "Specifying both RAYMATH_IMPLEMENTATION and RAYMATH_STATIC_INLINE is contradictory"
#endif

// Function specifiers definition
#if defined(RAYMATH_IMPLEMENTATION)
    #if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
        #define RMAPI __declspec(dllexport) extern inline // We are building raylib as a Win32 shared library (.dll)
    #elif defined(BUILD_LIBTYPE_SHARED)
        #define RMAPI __attribute__((visibility("default"))) // We are building raylib as a Unix shared library (.so/.dylib)
    #elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
        #define RMAPI __declspec(dllimport)         // We are using raylib as a Win32 shared library (.dll)
    #else
        #define RMAPI extern inline // Provide external definition
    #endif
#elif defined(RAYMATH_STATIC_INLINE)
    #define RMAPI static inline // Functions may be inlined, no external out-of-line definition
#else
    #if defined(__TINYC__)
        #define RMAPI static inline // plain inline not supported by tinycc (See issue #435)
    #else
        #define RMAPI inline        // Functions may be inlined or external definition used
    #endif
#endif


//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef PI
    #ifndef __GNU__
    #define M_PIf 3.14159265358979323846f
    #endif
    #define PI M_PIf
#endif

#ifndef EPSILON
    #define EPSILON 0.000001f
#endif

#ifndef DEG2RAD
    #define DEG2RAD (PI/180.0f)
#endif

#ifndef RAD2DEG
    #define RAD2DEG (180.0f/PI)
#endif

// Get float vector for Matrix
#ifndef MatrixToFloat
    #define MatrixToFloat(mat) (MatrixToFloatV(mat).v)
#endif

// Get float vector for Vector3
#ifndef Vector3ToFloat
    #define Vector3ToFloat(vec) (Vector3ToFloatV(vec).v)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if !defined(RL_VECTOR2_TYPE)
// Vector2 type
#ifdef __clang__
typedef float Vector2 __attribute__((ext_vector_type(2)));
#else
typedef struct Vector2 {
    float x;
    float y;
} Vector2;
#endif
#define RL_VECTOR2_TYPE
#endif

#if !defined(RL_VECTOR3_TYPE)
// Vector3 type
#ifdef __clang__
typedef float Vector3 __attribute__((ext_vector_type(3)));
#else
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;
#endif
#define RL_VECTOR3_TYPE
#endif

#if !defined(RL_VECTOR4_TYPE)
// Vector4 type
#ifdef __clang__
typedef float Vector4 __attribute__((ext_vector_type(4)));
#else
typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;
#endif
#define RL_VECTOR4_TYPE
#endif

#if !defined(RL_QUATERNION_TYPE)
// Quaternion type
typedef Vector4 Quaternion;
#define RL_QUATERNION_TYPE
#endif

#if !defined(RL_MATRIX_TYPE)
// Matrix type (OpenGL style 4x4 - right handed, column major)
typedef struct Matrix {
union {
struct {
    float m0, m4, m8, m12;  // Matrix first row (4 components)
    float m1, m5, m9, m13;  // Matrix second row (4 components)
    float m2, m6, m10, m14; // Matrix third row (4 components)
    float m3, m7, m11, m15; // Matrix fourth row (4 components)
};
struct {
    Vector4 r0, r1, r2, r3; // Matrix rows (4 rows)
};
};
} Matrix;
#define RL_MATRIX_TYPE
#endif

// NOTE: Helper types to be used instead of array return types for *ToFloat functions
typedef struct float3 {
    float v[3];
} float3;

typedef struct float16 {
    float v[16];
} float16;


//----------------------------------------------------------------------------------
// Module Functions Definition - Scalar math utils
//----------------------------------------------------------------------------------

RMAPI float Wrap(float value, float min, float max)                                                  { return value - (max - min)*floorf((value - min)/(max - min)); }
RMAPI float Clamp(float value, float min, float max)                                                 { float result = (value < min)? min : value; return (result > max) ? max : result; }
RMAPI float Lerp(float start, float end, float amount)                                               { return start + amount*(end - start); }
RMAPI float Normalize(float value, float start, float end)                                           { return (value - start)/(end - start); }
RMAPI float Remap(float value, float inputStart, float inputEnd, float outputStart, float outputEnd) { return (value - inputStart)/(inputEnd - inputStart)*(outputEnd - outputStart) + outputStart; }
RMAPI int   FloatEquals(float x, float y)                                                            { return (fabsf(x - y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y)))); }

//----------------------------------------------------------------------------------
// Module Functions Definition - Vector(2,3,4)/Quaternion/Matrix math
//----------------------------------------------------------------------------------
RMAPI Vector2 Vector2Zero(void)                                              { return (Vector2){ 0,0 }; }
RMAPI Vector3 Vector3Zero(void)                                              { return (Vector3){ 0,0,0 }; }
RMAPI Vector4 Vector4Zero(void)                                              { return (Vector4){ 0,0,0,0 }; }
#define QuaternionZero(void) Vector4Zero(void)

RMAPI Vector2 Vector2One(void)                                               { return (Vector2){ 1,1 }; }
RMAPI Vector3 Vector3One(void)                                               { return (Vector3){ 1,1,1 }; }
RMAPI Vector4 Vector4One(void)                                               { return (Vector4){ 1,1,1,1 }; }
#define QuaternionOne(void) Vector4One(void)

RMAPI Vector2 Vector2Identity(size_t i)                                      { return (Vector2){ (i % 2) == 0 ? 1 : 0, (i % 2) == 1 ? 1 : 0 }; }
RMAPI Vector3 Vector3Identity(size_t i)                                      { return (Vector3){ (i % 3) == 0 ? 1 : 0, (i % 3) == 1 ? 1 : 0, (i % 3) == 2 ? 1 : 0 }; }
RMAPI Vector4 Vector4Identity(size_t i)                                      { return (Vector4){ (i % 4) == 0 ? 1 : 0, (i % 4) == 1 ? 1 : 0, (i % 4) == 2 ? 1 : 0, (i % 4) == 3 ? 1 : 0 }; }
#define QuaternionIdentity() Vector4Identity(3)

RMAPI Matrix MatrixNeutral(void)                                             { return (Matrix){ 0 }; }
RMAPI Matrix MatrixIdentity(void)                                            { return (Matrix){ .r0 = Vector4Identity(0), .r1 = Vector4Identity(1), .r2 = Vector4Identity(2), .r3 = Vector4Identity(3) }; }
RMAPI Matrix MatrixScale(float x, float y, float z)                          { Matrix result = MatrixIdentity(); result.r0.x = x; result.r1.y = y; result.r2.z = z; return result; }
RMAPI Matrix MatrixTranslate(float x, float y, float z)                      { Matrix result = MatrixIdentity(); result.r0.w = x; result.r1.w = y; result.r2.w = z; return result; }

RMAPI Vector2 Vector2Negate(Vector2 v)
{
#ifdef __clang__
    return -v;
#else
    return (Vector2){ -v.x, -v.y };
#endif
}

RMAPI Vector3 Vector3Negate(Vector3 v)
{
#ifdef __clang__
    return -v;
#else
    return (Vector3){ -v.x, -v.y, -v.z };
#endif
}

RMAPI Vector4 Vector4Negate(Vector4 v)
{
#ifdef __clang__
	return -v;
#else
	return (Vector4){ -v.x, -v.y, -v.z, -v.w };
#endif 
}
#define QuaternionNegate(q) Vector4Negate(q)

RMAPI Vector3 Vector4Vector3(Vector4 v, float scale)                         { scale *= v.w; return ((scale != 0 && scale != 1) ? (Vector3){v.x/scale, v.y/scale, v.z/scale} : (Vector3){v.x, v.y, v.z}); }
RMAPI Vector4 Vector3Vector4(Vector3 v, float w)                             { return (Vector4){v.x, v.y, v.z, w}; };
RMAPI Vector4 Vector3Direction(Vector3 v)                                    { return (Vector4){v.x, v.y, v.z, 0}; };
RMAPI Vector4 Vector3Position(Vector3 v)                                     { return (Vector4){v.x, v.y, v.z, 1}; };

RMAPI Vector2 Vector2Add(Vector2 v1, Vector2 v2)
{
#ifdef __clang__
	return v1 + v2;
#else
	return (Vector2){ v1.x + v2.x, v1.y + v2.y };
#endif
}
RMAPI Vector3 Vector3Add(Vector3 v1, Vector3 v2)
{
#ifdef __clang__
	return v1 + v2;
#else
	return (Vector3){ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
#endif
}
RMAPI Vector4 Vector4Add(Vector4 v1, Vector4 v2)
{
#ifdef __clang__
	return v1 + v2;
#else
	return (Vector4){ v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w };
#endif
}
#define QuaternionAdd(q1, q2) Vector4Add(q1, q2)

RMAPI Vector2 Vector2AddValue(Vector2 v, float add)
{
#ifdef __clang__
	return v + add;
#else
	return (Vector2){ v.x + add, v.y + add };
#endif
}
RMAPI Vector3 Vector3AddValue(Vector3 v, float add)
{
#ifdef __clang__
	return v + add;
#else 
	return (Vector3){ v.x + add, v.y + add, v.z + add  };
#endif
}
RMAPI Vector4 Vector4AddValue(Vector4 v, float add)
{
#ifdef __clang__
	return v + add;
#else
	return (Vector4){ v.x + add, v.y + add, v.z + add, v.w + add };
#endif
}
#define QuaternionAddValue(q, add) Vector4Add(q, add)

RMAPI Vector2 Vector2Subtract(Vector2 v1, Vector2 v2)
{
#ifdef __clang__
	return v1 - v2;
#else
	return (Vector2){ v1.x - v2.x, v1.y - v2.y };
#endif
}
RMAPI Vector3 Vector3Subtract(Vector3 v1, Vector3 v2)
{
#ifdef __clang__
	return v1 - v2;
#else
	return (Vector3){ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
#endif
}
RMAPI Vector4 Vector4Subtract(Vector4 v1, Vector4 v2)
{
#ifdef __clang__
	return v1 - v2;
#else
	return (Vector4){ v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w };
#endif
}
#define QuaternionSubtract(q1, q2) Vector4Subtract(q1, q2)

RMAPI Vector2 Vector2SubtractValue(Vector2 v, float sub)
{
#ifdef __clang__
	return v - sub;
#else
	return (Vector2){ v.x - sub, v.y - sub };
#endif
}
RMAPI Vector3 Vector3SubtractValue(Vector3 v, float sub)
{
#ifdef __clang__
	return v - sub;
#else
	return (Vector3){ v.x - sub, v.y - sub, v.z - sub };
#endif
}
RMAPI Vector4 Vector4SubtractValue(Vector4 v, float sub)
{
#ifdef __clang__
	return v - sub;
#else
	return (Vector4){ v.x - sub, v.y - sub, v.z - sub, v.w - sub };
#endif
}
#define QuaternionSubtractValue(q, sub) Vector4Subtract(q, sub)

RMAPI Vector2 Vector2Multiply(Vector2 v1, Vector2 v2)
{
#ifdef __clang__
	return v1 * v2;
#else
	return (Vector2){ v1.x*v2.x, v1.y*v2.y };
#endif
}
RMAPI Vector3 Vector3Multiply(Vector3 v1, Vector3 v2)
{
#ifdef __clang__
	return v1 * v2;
#else
	return (Vector3){ v1.x*v2.x, v1.y*v2.y, v1.z*v2.z };
#endif
}
RMAPI Vector4 Vector4Multiply(Vector4 v1, Vector4 v2)
{
#ifdef __clang__
	return v1 * v2;
#else
	return (Vector4){ v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w };
#endif
}

RMAPI Vector2 Vector2Scale(Vector2 v, float scale)
{
#ifdef __clang__
	return v * scale;
#else
	return (Vector2){ v.x*scale, v.y*scale };
#endif
}
RMAPI Vector3 Vector3Scale(Vector3 v, float scale)
{
#ifdef __clang__
	return v * scale;
#else
	return (Vector3){ v.x*scale, v.y*scale, v.z*scale };
#endif
}
RMAPI Vector4 Vector4Scale(Vector4 v, float scale)
{
#ifdef __clang__
	return v * scale;
#else
	return (Vector4){ v.x*scale, v.y*scale, v.z*scale, v.w*scale };
#endif
}
#define QuaternionScale(q, mul) Vector4Scale(q, mul)

RMAPI Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2)
{
	return Vector4Add(QuaternionScale(q1, q2.w), Vector4Subtract(Vector4Add(Vector3Vector4(Vector3Scale(Vector4Vector3(q2, 0), q1.w), -q1.x*q2.x),Vector3Vector4((Vector3){q1.y*q2.z, q1.z*q2.x,q1.x*q2.y}, -q1.y*q2.y)),(Vector4){q1.z*q2.y,q1.x*q2.z,q1.y*q2.x,q1.z*q2.z}));
}

RMAPI Vector2 Vector2Divide(Vector2 v1, Vector2 v2)
{
#ifdef __clang__
	return v1 / v2;
#else
	return (Vector2){ v1.x/v2.x, v1.y/v2.y };
#endif
}
RMAPI Vector3 Vector3Divide(Vector3 v1, Vector3 v2)
{
#ifdef __clang__
	return v1 / v2;
#else
	return (Vector3){ v1.x/v2.x, v1.y/v2.y, v1.z/v2.z };
#endif
}
RMAPI Vector4 Vector4Divide(Vector4 v1, Vector4 v2)
{
#ifdef __clang__
	return v1 / v2;
#else 
	return (Vector4){ v1.x/v2.x, v1.y/v2.y, v1.z/v2.z, v1.w/v2.w };
#endif
}
#define QuaternionDivide(q1, q2) Vector4Divide(q1, q2)

RMAPI float Vector2DotProduct(Vector2 v1, Vector2 v2)                        { Vector2 result = Vector2Multiply(v1,v2); return result.x + result.y; }
RMAPI float Vector3DotProduct(Vector3 v1, Vector3 v2)                        { Vector3 result = Vector3Multiply(v1,v2); return result.x + result.y + result.z; }
RMAPI float Vector4DotProduct(Vector4 v1, Vector4 v2)                        { Vector4 result = Vector4Multiply(v1,v2); return result.x + result.y + result.z + result.w; }
#define QuaternionDotProduct(v1, v2) Vector4DotProduct(v1, v2)

RMAPI float Vector2LengthSqr(Vector2 v)                                      { return Vector2DotProduct(v,v); }
RMAPI float Vector3LengthSqr(Vector3 v)                                      { return Vector3DotProduct(v,v); }
RMAPI float Vector4LengthSqr(Vector4 v)                                      { return Vector4DotProduct(v,v); }
#define QuaternionLengthSqr(v) Vector4LengthSqr(v)

RMAPI float Vector2Length(Vector2 v)                                         { return sqrtf(Vector2LengthSqr(v)); }
RMAPI float Vector3Length(Vector3 v)                                         { return sqrtf(Vector3LengthSqr(v)); }
RMAPI float Vector4Length(Vector4 v)                                         { return sqrtf(Vector4LengthSqr(v)); }
#define QuaternionLength(v) Vector4Length(v)

RMAPI Vector2 Vector2Normalize(Vector2 v)                                    { float length = Vector2Length(v); return (length > 0 ? Vector2Scale(v, 1/length) : v); }
RMAPI Vector3 Vector3Normalize(Vector3 v)                                    { float length = Vector3Length(v); return (length > 0 ? Vector3Scale(v, 1/length) : v); }
RMAPI Vector4 Vector4Normalize(Vector4 v)                                    { float length = Vector4Length(v); return (length > 0 ? Vector4Scale(v, 1/length) : v); }
#define QuaternionNormalize(v) Vector4Normalize(v)

RMAPI float Vector2Distance(Vector2 v1, Vector2 v2)                          { return Vector2Length(Vector2Subtract(v1,v2)); }
RMAPI float Vector3Distance(Vector3 v1, Vector3 v2)                          { return Vector3Length(Vector3Subtract(v1,v2)); }
RMAPI float Vector4Distance(Vector4 v1, Vector4 v2)                          { return Vector4Length(Vector4Subtract(v1,v2)); }

RMAPI float Vector2DistanceSqr(Vector2 v1, Vector2 v2)                       { return Vector2LengthSqr(Vector2Subtract(v1,v2)); }
RMAPI float Vector3DistanceSqr(Vector3 v1, Vector3 v2)                       { return Vector3LengthSqr(Vector3Subtract(v1,v2)); }
RMAPI float Vector4DistanceSqr(Vector4 v1, Vector4 v2)                       { return Vector4LengthSqr(Vector4Subtract(v1,v2)); }

RMAPI Vector2 Vector2Invert(Vector2 v)
{
#ifdef __clang__
	return 1/v;
#else
	return (Vector2){ 1/v.x, 1/v.y };
#endif
}
RMAPI Vector3 Vector3Invert(Vector3 v)
{
#ifdef __clang__
	return 1/v;
#else
	return (Vector3){ 1/v.x,1/v.y,1/v.z };
#endif
}
RMAPI Vector4 Vector4Invert(Vector4 v)
{
#ifdef __clang__
	return 1/v;
#else
	return (Vector4){ 1/v.x,1/v.y,1/v.z,1/v.w };
#endif
}
RMAPI Quaternion QuaternionInvert(Quaternion q)
{
    Quaternion result = q;

    float lengthSq = QuaternionLengthSqr(q);

    if (lengthSq != 0)
    {
        q.x = -1/lengthSq;
	q.y = -1/lengthSq;
	q.z = -1/lengthSq;
    	q.w = 1/lengthSq;
    }

    return result;
}

RMAPI Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2)
{
	return Vector3Subtract(Vector3Multiply((Vector3){v1.y, v1.z, v1.x},(Vector3){v2.z,v2.x,v2.y}), Vector3Multiply((Vector3){v1.z,v1.x,v1.y},(Vector3){v2.y,v2.z,v2.x}));
}

RMAPI Vector3 Vector3Perpendicular(Vector3 v)                                { return Vector3CrossProduct(v, ((fabsf(v.y) < fabsf(v.x)) ? (Vector3){0.0f, 1.0f, 0.0f} : 
                                                                                                              (fabsf(v.z) < fabsf(v.y)) ? (Vector3){0.0f, 0.0f, 1.0f} :
                                                                                                                                          (Vector3){1.0f, 0.0f, 0.0f})); }

RMAPI Vector2 Vector2Min(Vector2 v1, Vector2 v2)                             { return (Vector2){ fminf(v1.x, v2.x), fminf(v1.y, v2.y) }; }
RMAPI Vector3 Vector3Min(Vector3 v1, Vector3 v2)                             { return (Vector3){ fminf(v1.x, v2.x), fminf(v1.y, v2.y), fminf(v1.z, v2.z) }; }
RMAPI Vector4 Vector4Min(Vector4 v1, Vector4 v2)                             { return (Vector4){ fminf(v1.x, v2.x), fminf(v1.y, v2.y), fminf(v1.z, v2.z), fminf(v1.w, v2.w) }; }

RMAPI Vector2 Vector2Max(Vector2 v1, Vector2 v2)                             { return (Vector2){ fmaxf(v1.x, v2.x), fmaxf(v1.y, v2.y) }; }
RMAPI Vector3 Vector3Max(Vector3 v1, Vector3 v2)                             { return (Vector3){ fmaxf(v1.x, v2.x), fmaxf(v1.y, v2.y), fmaxf(v1.z, v2.z) }; }
RMAPI Vector4 Vector4Max(Vector4 v1, Vector4 v2)                             { return (Vector4){ fmaxf(v1.x, v2.x), fmaxf(v1.y, v2.y), fmaxf(v1.z, v2.z), fmaxf(v1.w, v2.w) }; }

RMAPI Vector2 Vector2Lerp(Vector2 v1, Vector2 v2, float amount)              { return Vector2Add(v1,Vector2Scale(Vector2Subtract(v2, v1), amount)); }
RMAPI Vector3 Vector3Lerp(Vector3 v1, Vector3 v2, float amount)              { return Vector3Add(v1,Vector3Scale(Vector3Subtract(v2, v1), amount)); }
RMAPI Vector4 Vector4Lerp(Vector4 v1, Vector4 v2, float amount)              { return Vector4Add(v1,Vector4Scale(Vector4Subtract(v2, v1), amount)); }
#define QuaternionLerp(q1, q2, amount) Vector4Lerp(q1, q2, amount)
// Calculate slerp-optimized interpolation between two quaternions
RMAPI Quaternion QuaternionNlerp(Quaternion q1, Quaternion q2, float amount) { return QuaternionNormalize(QuaternionLerp(q1, q2, amount)); }
// Calculates spherical linear interpolation between two quaternions
RMAPI Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float amount)
{
    float cosHalfTheta = QuaternionDotProduct(q1,q2);

    if (cosHalfTheta < 0)
    {
    	q2 = QuaternionNegate(q2);
        cosHalfTheta = -cosHalfTheta;
    }

    if (fabsf(cosHalfTheta) >= 1)
        return q1;
    else if (cosHalfTheta > 0.95f)
        return QuaternionNlerp(q1, q2, amount);
    else
    {
        float halfTheta = acosf(cosHalfTheta);
        float sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);

        if (fabsf(sinHalfTheta) < EPSILON)
	    return QuaternionAdd(QuaternionScale(q1,0.5f),QuaternionScale(q2,0.5));
        else
	    return QuaternionAdd(QuaternionScale(q1,sinf((1 - amount)*halfTheta)/sinHalfTheta),QuaternionScale(q2,sinf(amount*halfTheta)/sinHalfTheta));
    }
}


RMAPI Vector2 Vector2Clamp(Vector2 v, Vector2 min, Vector2 max)              { return (Vector2){ fminf(max.x, fmaxf(min.x, v.x)), fminf(max.y, fmaxf(min.y, v.y)) }; }
RMAPI Vector3 Vector3Clamp(Vector3 v, Vector3 min, Vector3 max)              { return (Vector3){ fminf(max.x, fmaxf(min.x, v.x)), fminf(max.y, fmaxf(min.y, v.y)), fminf(max.z, fmaxf(min.z, v.z)) }; }
RMAPI Vector4 Vector4Clamp(Vector4 v, Vector4 min, Vector4 max)              { return (Vector4){ fminf(max.x, fmaxf(min.x, v.x)), fminf(max.y, fmaxf(min.y, v.y)), fminf(max.z, fmaxf(min.z, v.z)), fminf(max.w, fmaxf(min.w, v.w)) }; }

RMAPI float Vector2Determinant(Vector2 v1, Vector2 v2)                       { Vector2 result = Vector2Multiply(v1,(Vector2){v2.y, v2.x}); return result.x - result.y; }
RMAPI float Vector2Angle(Vector2 v1, Vector2 v2)                             { return atan2f(Vector2Determinant(v1,v2), Vector2DotProduct(v1,v2)); } // NOTE: Angle is calculated from origin point (0, 0)
RMAPI float Vector3Angle(Vector3 v1, Vector3 v2)                             { return atan2f(Vector3Length(Vector3CrossProduct(v1,v2)), Vector3DotProduct(v1,v2)); }
RMAPI float Vector2LineAngle(Vector2 start, Vector2 end)                     { return -atan2f(end.y - start.y, end.x - start.x); } // TODO(10/9/2023): Currently angles move clockwise, determine if this is wanted behavior

RMAPI Vector3 Vector3Project(Vector3 v1, Vector3 v2)                         { return Vector3Scale(v2, Vector3DotProduct(v1,v2)/Vector3LengthSqr(v2)); }
RMAPI Vector3 Vector3Reject(Vector3 v1, Vector3 v2)                          { return Vector3Subtract(v1, Vector3Project(v1, v2)); }

RMAPI void Vector3OrthoNormalize(Vector3 *v1, Vector3 *v2)                   { *v1 = Vector3Normalize(*v1); *v2 = Vector3CrossProduct(Vector3Normalize(Vector3CrossProduct(*v1, *v2)), *v1); }

RMAPI Vector2 Vector2Reflect(Vector2 v, Vector2 normal)                      { return Vector2Subtract(v,Vector2Scale(normal,2.0f*Vector2DotProduct(v,normal))); }
RMAPI Vector3 Vector3Reflect(Vector3 v, Vector3 normal)                      { return Vector3Subtract(v,Vector3Scale(normal,2.0f*Vector3DotProduct(v,normal))); }

RMAPI Vector2 Vector2Transform(Vector2 v, Matrix mat)                        { Vector4 t = (Vector4){ v.x, v.y,   0, 1   }; return (Vector2){ Vector4DotProduct(mat.r0,t), Vector4DotProduct(mat.r1,t) }; }
RMAPI Vector3 Vector3Transform(Vector3 v, Matrix mat)                        { Vector4 t = (Vector4){ v.x, v.y, v.z, 1   }; return (Vector3){ Vector4DotProduct(mat.r0,t), Vector4DotProduct(mat.r1,t), Vector4DotProduct(mat.r2,t) }; }
RMAPI Vector4 Vector4Transform(Vector4 v, Matrix mat)                        { Vector4 t = (Vector4){ v.x, v.y, v.z, v.w }; return (Vector4){ Vector4DotProduct(mat.r0,t), Vector4DotProduct(mat.r1,t), Vector4DotProduct(mat.r2,t), Vector4DotProduct(mat.r3, t) }; }

RMAPI int Vector2Equals(Vector2 p, Vector2 q)
{

    int result = ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                 ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y)))));

    return result;
}
RMAPI int Vector3Equals(Vector3 p, Vector3 q)
{
    return ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
           ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
           ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z)))));
}
RMAPI int Vector4Equals(Vector4 p, Vector4 q)
{
    return ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
           ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
           ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) &&
           ((fabsf(p.w - q.w)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w)))));
}
RMAPI int QuaternionEquals(Quaternion p, Quaternion q)
{
    int result = (((fabsf(p.x - q.x))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y - q.y))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                  ((fabsf(p.z - q.z))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) &&
                  ((fabsf(p.w - q.w))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w)))))) ||
                  (((fabsf(p.x + q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y + q.y))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                  ((fabsf(p.z + q.z))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) &&
                  ((fabsf(p.w + q.w))  <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w))))));

    return result;
}

// Clamp the magnitude of the vector between two min and max values
RMAPI Vector2 Vector2ClampValue(Vector2 v, float min, float max)
{
    Vector2 result = v;

    float length = Vector2LengthSqr(v);
    if (length > 0)
    {
        length = sqrtf(length);

        float scale = 1;    // By default, 1 as the neutral element.
        if (length < min)
            scale = min/length;
        else if (length > max)
            scale = max/length;

        result = Vector2Scale(v, scale);
    }

    return result;
}

// Clamp the magnitude of the vector between two values
RMAPI Vector3 Vector3ClampValue(Vector3 v, float min, float max)
{
    Vector3 result = v;

    float length = Vector3LengthSqr(v);
    if (length > 0)
    {
        length = sqrtf(length);

        float scale = 1;    // By default, 1 as the neutral element.
        if (length < min)
            scale = min/length;
        else if (length > max)
            scale = max/length;

        Vector3Scale(v, scale);
    }

    return result;
}

// Clamp the magnitude of the vector between two values
RMAPI Vector4 Vector4ClampValue(Vector4 v, float min, float max)
{
    Vector4 result = v;

    float length = Vector4LengthSqr(v);
    if (length > 0)
    {
        length = sqrtf(length);

        float scale = 1;    // By default, 1 as the neutral element.
        if (length < min)
            scale = min/length;
        else if (length > max)
            scale = max/length;

	result = Vector4Scale(v, scale);
    }

    return result;
}

RMAPI Vector2 Vector2Rotate(Vector2 v, float angle)                          { float cosres = cosf(angle); float sinres = sinf(angle); return (Vector2){ v.x*cosres - v.y*sinres, v.x*sinres + v.y*cosres }; }
RMAPI Vector3 Vector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle) { angle /= 2.0f; Vector3 w = Vector3Scale(Vector3Normalize(axis), sinf(angle)); Vector3 wv = Vector3CrossProduct(w, v); return Vector3Add(v, Vector3Add(Vector3Scale(wv, 2*cosf(angle)), Vector3Scale(Vector3CrossProduct(w, wv), 2))); }
RMAPI Vector3 Vector3RotateByQuaternion(Vector3 v, Quaternion q)
{
    Vector3 result = { 0 };

    result.x = v.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + v.y*(2*q.x*q.y - 2*q.w*q.z) + v.z*(2*q.x*q.z + 2*q.w*q.y);
    result.y = v.x*(2*q.w*q.z + 2*q.x*q.y) + v.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + v.z*(-2*q.w*q.x + 2*q.y*q.z);
    result.z = v.x*(-2*q.w*q.y + 2*q.x*q.z) + v.y*(2*q.w*q.x + 2*q.y*q.z)+ v.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);

    return result;
}
// Calculate quaternion based on the rotation from one vector to another
RMAPI Quaternion QuaternionFromVector3ToVector3(Vector3 from, Vector3 to)    { return QuaternionNormalize((Quaternion)Vector3Vector4(Vector3CrossProduct(from, to), 1.0f + Vector3DotProduct(from, to))); }

// Move Vector towards target
RMAPI Vector2 Vector2MoveTowards(Vector2 v, Vector2 target, float maxDistance)
{
    Vector2 result = { 0 };

    float dx = target.x - v.x;
    float dy = target.y - v.y;
    float value = (dx*dx) + (dy*dy);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    float dist = sqrtf(value);

    result.x = v.x + dx/dist*maxDistance;
    result.y = v.y + dy/dist*maxDistance;

    return result;
}

RMAPI Vector3 Vector3MoveTowards(Vector3 v, Vector3 target, float maxDistance)
{
    Vector3 d = Vector3Subtract(target, v);

    float value = Vector3LengthSqr(d);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    float dist = sqrtf(value);

    return Vector3Add(v,Vector3Scale(d,1/dist * maxDistance));
}

RMAPI Vector4 Vector4MoveTowards(Vector4 v, Vector4 target, float maxDistance)
{
    Vector4 result = { 0 };

    float dx = target.x - v.x;
    float dy = target.y - v.y;
    float dz = target.z - v.z;
    float dw = target.w - v.w;
    float value = (dx*dx) + (dy*dy) + (dz*dz) + (dw*dw);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    float dist = sqrtf(value);

    result.x = v.x + dx/dist*maxDistance;
    result.y = v.y + dy/dist*maxDistance;
    result.z = v.z + dz/dist*maxDistance;
    result.w = v.w + dw/dist*maxDistance;

    return result;
}

// Calculate cubic hermite interpolation between two vectors and their tangents
// as described in the GLTF 2.0 specification: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#interpolation-cubic
RMAPI Vector3 Vector3CubicHermite(Vector3 v1, Vector3 tangent1, Vector3 v2, Vector3 tangent2, float amount)
{
    Vector3 result = { 0 };

    float amountPow2 = amount*amount;
    float amountPow3 = amount*amount*amount;

    result.x = (2*amountPow3 - 3*amountPow2 + 1)*v1.x + (amountPow3 - 2*amountPow2 + amount)*tangent1.x + (-2*amountPow3 + 3*amountPow2)*v2.x + (amountPow3 - amountPow2)*tangent2.x;
    result.y = (2*amountPow3 - 3*amountPow2 + 1)*v1.y + (amountPow3 - 2*amountPow2 + amount)*tangent1.y + (-2*amountPow3 + 3*amountPow2)*v2.y + (amountPow3 - amountPow2)*tangent2.y;
    result.z = (2*amountPow3 - 3*amountPow2 + 1)*v1.z + (amountPow3 - 2*amountPow2 + amount)*tangent1.z + (-2*amountPow3 + 3*amountPow2)*v2.z + (amountPow3 - amountPow2)*tangent2.z;

    return result;
}

// Calculate reflected vector to normal

// Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c)
// NOTE: Assumes P is on the plane of the triangle
RMAPI Vector3 Vector3Barycenter(Vector3 p, Vector3 a, Vector3 b, Vector3 c)
{
    Vector3 result = { 0 };

    Vector3 v0 = Vector3Subtract(b, a);
    Vector3 v1 = Vector3Subtract(c, a);
    Vector3 v2 = Vector3Subtract(p, a);
    float d00 = Vector3DotProduct(v0, v0);
    float d01 = Vector3DotProduct(v0, v1);
    float d11 = Vector3DotProduct(v1, v1);
    float d20 = Vector3DotProduct(v2, v0);
    float d21 = Vector3DotProduct(v2, v1);

    float denom = d00*d11 - d01*d01;

    result.y = (d11*d20 - d01*d21)/denom;
    result.z = (d00*d21 - d01*d20)/denom;
    result.x = 1.0f - (result.z + result.y);

    return result;
}

// Get Vector3 as float array
RMAPI float3 Vector3ToFloatV(Vector3 v)
{
    float3 buffer = { 0 };

    buffer.v[0] = v.x;
    buffer.v[1] = v.y;
    buffer.v[2] = v.z;

    return (float3){ .v[0] = v.x, .v[1] = v.y, .v[2] = v.z };
}

// Compute the direction of a refracted ray
// v: normalized direction of the incoming ray
// n: normalized normal vector of the interface of two optical media
// r: ratio of the refractive index of the medium from where the ray comes
//    to the refractive index of the medium on the other side of the surface
RMAPI Vector2 Vector2Refract(Vector2 v, Vector2 n, float r)
{
    Vector2 result = { 0 };

    float dot = Vector2DotProduct(v, n);
    float d = 1.0f - r*r*(1.0f - dot*dot);

    if (d >= 0)
    {
        d = sqrtf(d);
	result = Vector2Subtract(Vector2Scale(v,r),Vector2Scale(n, r * dot + d));
    }

    return result;
}
// Check whether two given vectors are almost equal
// Compute the direction of a refracted ray
// v: normalized direction of the incoming ray
// n: normalized normal vector of the interface of two optical media
// r: ratio of the refractive index of the medium from where the ray comes
//    to the refractive index of the medium on the other side of the surface
RMAPI Vector3 Vector3Refract(Vector3 v, Vector3 n, float r)
{
    Vector3 result = { 0 };

    float dot = Vector3DotProduct(v, n);
    float d = 1.0f - r*r*(1.0f - dot*dot);

    if (d >= 0)
    {
        d = sqrtf(d);
	result = Vector3Subtract(Vector3Scale(v, r),Vector3Scale(n, r * dot + d));
    }

    return result;
}



//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix math
//----------------------------------------------------------------------------------

RMAPI Vector3 Vector3LaPlace(Vector3 a, Vector3 b)
{
#ifdef __clang__
	return (Vector3){ Vector2Determinant(a.yz, b.yz),
	                  Vector2Determinant(a.xz, b.xz),
	                  Vector2Determinant(a.xy, b.xy) };
#else
	return (Vector3){ Vector2Determinant((Vector2){a.y,a.z}, (Vector2){b.y,b.z}),
	                  Vector2Determinant((Vector2){a.x,a.z}, (Vector2){b.x,b.z}),
	                  Vector2Determinant((Vector2){a.x,a.y}, (Vector2){b.x,b.y}) };
#endif
}

RMAPI float   Vector3Determinant(Vector3 a, Vector3 b, Vector3 c)
{
	Vector3 dst = Vector3Multiply(Vector3Multiply((Vector3){1,-1,1}, a),Vector3LaPlace(b, c));
	return dst.x + dst.y + dst.z;
}

RMAPI Vector4 Vector4LaPlace(Vector4 a, Vector4 b, Vector4 c)
{
	return (Vector4){ Vector3Determinant((Vector3){a.y,a.z,a.w}, (Vector3){b.y,b.z,b.w}, (Vector3){c.y,c.z,c.w}),
	                  Vector3Determinant((Vector3){a.x,a.z,a.w}, (Vector3){b.x,b.z,b.w}, (Vector3){c.x,c.z,c.w}),
	                  Vector3Determinant((Vector3){a.x,a.y,a.w}, (Vector3){b.x,b.y,b.w}, (Vector3){c.x,c.y,c.w}),
			  Vector3Determinant((Vector3){a.x,a.y,a.z}, (Vector3){b.x,b.y,b.z}, (Vector3){c.x,c.y,c.z}) };
}

RMAPI float Vector4Determinant(Vector4 a, Vector4 b, Vector4 c, Vector4 d)
{
    Vector4 dst = Vector4Multiply(Vector4Multiply((Vector4){1,-1,1,-1},a),Vector4LaPlace(b, c, d));
    return dst.x + dst.y + dst.z + dst.w;
}

// Compute matrix determinant
RMAPI float MatrixDeterminant(Matrix mat) { return Vector4Determinant(mat.r0, mat.r1, mat.r2, mat.r3); }

// Get the trace of the matrix (sum of the values along the diagonal)
RMAPI float MatrixTrace(Matrix mat) { return (mat.m0 + mat.m5 + mat.m10 + mat.m15); }

// Transposes provided matrix
RMAPI Matrix MatrixTranspose(Matrix mat)
{
    Matrix result = { 0 };

    result.m0 = mat.m0;
    result.m1 = mat.m4;
    result.m2 = mat.m8;
    result.m3 = mat.m12;
    result.m4 = mat.m1;
    result.m5 = mat.m5;
    result.m6 = mat.m9;
    result.m7 = mat.m13;
    result.m8 = mat.m2;
    result.m9 = mat.m6;
    result.m10 = mat.m10;
    result.m11 = mat.m14;
    result.m12 = mat.m3;
    result.m13 = mat.m7;
    result.m14 = mat.m11;
    result.m15 = mat.m15;

    return result;
}

// Invert provided matrix
RMAPI Matrix MatrixInvert(Matrix mat)
{
    Matrix result = { 0 };

    // Cache the matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
    float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

    float b00 = a00*a11 - a01*a10;
    float b01 = a00*a12 - a02*a10;
    float b02 = a00*a13 - a03*a10;
    float b03 = a01*a12 - a02*a11;
    float b04 = a01*a13 - a03*a11;
    float b05 = a02*a13 - a03*a12;
    float b06 = a20*a31 - a21*a30;
    float b07 = a20*a32 - a22*a30;
    float b08 = a20*a33 - a23*a30;
    float b09 = a21*a32 - a22*a31;
    float b10 = a21*a33 - a23*a31;
    float b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    result.m0 = (a11*b11 - a12*b10 + a13*b09)*invDet;
    result.m1 = (-a01*b11 + a02*b10 - a03*b09)*invDet;
    result.m2 = (a31*b05 - a32*b04 + a33*b03)*invDet;
    result.m3 = (-a21*b05 + a22*b04 - a23*b03)*invDet;
    result.m4 = (-a10*b11 + a12*b08 - a13*b07)*invDet;
    result.m5 = (a00*b11 - a02*b08 + a03*b07)*invDet;
    result.m6 = (-a30*b05 + a32*b02 - a33*b01)*invDet;
    result.m7 = (a20*b05 - a22*b02 + a23*b01)*invDet;
    result.m8 = (a10*b10 - a11*b08 + a13*b06)*invDet;
    result.m9 = (-a00*b10 + a01*b08 - a03*b06)*invDet;
    result.m10 = (a30*b04 - a31*b02 + a33*b00)*invDet;
    result.m11 = (-a20*b04 + a21*b02 - a23*b00)*invDet;
    result.m12 = (-a10*b09 + a11*b07 - a12*b06)*invDet;
    result.m13 = (a00*b09 - a01*b07 + a02*b06)*invDet;
    result.m14 = (-a30*b03 + a31*b01 - a32*b00)*invDet;
    result.m15 = (a20*b03 - a21*b01 + a22*b00)*invDet;

    return result;
}

// Add two matrices
RMAPI Matrix MatrixAdd(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0 + right.m0;
    result.m1 = left.m1 + right.m1;
    result.m2 = left.m2 + right.m2;
    result.m3 = left.m3 + right.m3;
    result.m4 = left.m4 + right.m4;
    result.m5 = left.m5 + right.m5;
    result.m6 = left.m6 + right.m6;
    result.m7 = left.m7 + right.m7;
    result.m8 = left.m8 + right.m8;
    result.m9 = left.m9 + right.m9;
    result.m10 = left.m10 + right.m10;
    result.m11 = left.m11 + right.m11;
    result.m12 = left.m12 + right.m12;
    result.m13 = left.m13 + right.m13;
    result.m14 = left.m14 + right.m14;
    result.m15 = left.m15 + right.m15;

    return result;
}

// Subtract two matrices (left - right)
RMAPI Matrix MatrixSubtract(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0 - right.m0;
    result.m1 = left.m1 - right.m1;
    result.m2 = left.m2 - right.m2;
    result.m3 = left.m3 - right.m3;
    result.m4 = left.m4 - right.m4;
    result.m5 = left.m5 - right.m5;
    result.m6 = left.m6 - right.m6;
    result.m7 = left.m7 - right.m7;
    result.m8 = left.m8 - right.m8;
    result.m9 = left.m9 - right.m9;
    result.m10 = left.m10 - right.m10;
    result.m11 = left.m11 - right.m11;
    result.m12 = left.m12 - right.m12;
    result.m13 = left.m13 - right.m13;
    result.m14 = left.m14 - right.m14;
    result.m15 = left.m15 - right.m15;

    return result;
}

// Get two matrix multiplication
// NOTE: When multiplying matrices... the order matters!
RMAPI Matrix MatrixMultiply(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0*right.m0 + left.m1*right.m4 + left.m2*right.m8 + left.m3*right.m12;
    result.m1 = left.m0*right.m1 + left.m1*right.m5 + left.m2*right.m9 + left.m3*right.m13;
    result.m2 = left.m0*right.m2 + left.m1*right.m6 + left.m2*right.m10 + left.m3*right.m14;
    result.m3 = left.m0*right.m3 + left.m1*right.m7 + left.m2*right.m11 + left.m3*right.m15;
    result.m4 = left.m4*right.m0 + left.m5*right.m4 + left.m6*right.m8 + left.m7*right.m12;
    result.m5 = left.m4*right.m1 + left.m5*right.m5 + left.m6*right.m9 + left.m7*right.m13;
    result.m6 = left.m4*right.m2 + left.m5*right.m6 + left.m6*right.m10 + left.m7*right.m14;
    result.m7 = left.m4*right.m3 + left.m5*right.m7 + left.m6*right.m11 + left.m7*right.m15;
    result.m8 = left.m8*right.m0 + left.m9*right.m4 + left.m10*right.m8 + left.m11*right.m12;
    result.m9 = left.m8*right.m1 + left.m9*right.m5 + left.m10*right.m9 + left.m11*right.m13;
    result.m10 = left.m8*right.m2 + left.m9*right.m6 + left.m10*right.m10 + left.m11*right.m14;
    result.m11 = left.m8*right.m3 + left.m9*right.m7 + left.m10*right.m11 + left.m11*right.m15;
    result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8 + left.m15*right.m12;
    result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9 + left.m15*right.m13;
    result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
    result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;

    return result;
}

// Create rotation matrix from axis and angle
// NOTE: Angle should be provided in radians
RMAPI Matrix MatrixRotate(Vector3 axis, float angle)
{
    Matrix result = { 0 };

    float x = axis.x, y = axis.y, z = axis.z;

    float lengthSquared = x*x + y*y + z*z;

    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
    {
        float ilength = 1.0f/sqrtf(lengthSquared);
        x *= ilength;
        y *= ilength;
        z *= ilength;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    result.m0 = x*x*t + cosres;
    result.m1 = y*x*t + z*sinres;
    result.m2 = z*x*t - y*sinres;
    result.m3 = 0.0f;

    result.m4 = x*y*t - z*sinres;
    result.m5 = y*y*t + cosres;
    result.m6 = z*y*t + x*sinres;
    result.m7 = 0.0f;

    result.m8 = x*z*t + y*sinres;
    result.m9 = y*z*t - x*sinres;
    result.m10 = z*z*t + cosres;
    result.m11 = 0.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

// Get x-rotation matrix
// NOTE: Angle must be provided in radians
RMAPI Matrix MatrixRotateX(float angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m5 = cosres;
    result.m6 = sinres;
    result.m9 = -sinres;
    result.m10 = cosres;

    return result;
}

// Get y-rotation matrix
// NOTE: Angle must be provided in radians
RMAPI Matrix MatrixRotateY(float angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m0 = cosres;
    result.m2 = -sinres;
    result.m8 = sinres;
    result.m10 = cosres;

    return result;
}

// Get z-rotation matrix
// NOTE: Angle must be provided in radians
RMAPI Matrix MatrixRotateZ(float angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m0 = cosres;
    result.m1 = sinres;
    result.m4 = -sinres;
    result.m5 = cosres;

    return result;
}


// Get xyz-rotation matrix
// NOTE: Angle must be provided in radians
RMAPI Matrix MatrixRotateXYZ(Vector3 angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    float cosz = cosf(-angle.z);
    float sinz = sinf(-angle.z);
    float cosy = cosf(-angle.y);
    float siny = sinf(-angle.y);
    float cosx = cosf(-angle.x);
    float sinx = sinf(-angle.x);

    result.m0 = cosz*cosy;
    result.m1 = (cosz*siny*sinx) - (sinz*cosx);
    result.m2 = (cosz*siny*cosx) + (sinz*sinx);

    result.m4 = sinz*cosy;
    result.m5 = (sinz*siny*sinx) + (cosz*cosx);
    result.m6 = (sinz*siny*cosx) - (cosz*sinx);

    result.m8 = -siny;
    result.m9 = cosy*sinx;
    result.m10= cosy*cosx;

    return result;
}

// Get zyx-rotation matrix
// NOTE: Angle must be provided in radians
RMAPI Matrix MatrixRotateZYX(Vector3 angle)
{
    Matrix result = { 0 };

    float cz = cosf(angle.z);
    float sz = sinf(angle.z);
    float cy = cosf(angle.y);
    float sy = sinf(angle.y);
    float cx = cosf(angle.x);
    float sx = sinf(angle.x);

    result.m0 = cz*cy;
    result.m4 = cz*sy*sx - cx*sz;
    result.m8 = sz*sx + cz*cx*sy;
    result.m12 = 0;

    result.m1 = cy*sz;
    result.m5 = cz*cx + sz*sy*sx;
    result.m9 = cx*sz*sy - cz*sx;
    result.m13 = 0;

    result.m2 = -sy;
    result.m6 = cy*sx;
    result.m10 = cy*cx;
    result.m14 = 0;

    result.m3 = 0;
    result.m7 = 0;
    result.m11 = 0;
    result.m15 = 1;

    return result;
}

// Get perspective projection matrix
RMAPI Matrix MatrixFrustum(double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    Matrix result = { 0 };

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(farPlane - nearPlane);

    result.m0 = ((float)nearPlane*2.0f)/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;

    result.m4 = 0.0f;
    result.m5 = ((float)nearPlane*2.0f)/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;

    result.m8 = ((float)right + (float)left)/rl;
    result.m9 = ((float)top + (float)bottom)/tb;
    result.m10 = -((float)farPlane + (float)nearPlane)/fn;
    result.m11 = -1.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = -((float)farPlane*(float)nearPlane*2.0f)/fn;
    result.m15 = 0.0f;

    return result;
}

// Get perspective projection matrix
// NOTE: Fovy angle must be provided in radians
RMAPI Matrix MatrixPerspective(double fovY, double aspect, double nearPlane, double farPlane)
{
    Matrix result = { 0 };

    double top = nearPlane*tan(fovY*0.5);
    double bottom = -top;
    double right = top*aspect;
    double left = -right;

    // MatrixFrustum(-right, right, -top, top, near, far);
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(farPlane - nearPlane);

    result.m0 = ((float)nearPlane*2.0f)/rl;
    result.m5 = ((float)nearPlane*2.0f)/tb;
    result.m8 = ((float)right + (float)left)/rl;
    result.m9 = ((float)top + (float)bottom)/tb;
    result.m10 = -((float)farPlane + (float)nearPlane)/fn;
    result.m11 = -1.0f;
    result.m14 = -((float)farPlane*(float)nearPlane*2.0f)/fn;

    return result;
}

// Get orthographic projection matrix
RMAPI Matrix MatrixOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    Matrix result = { 0 };

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(farPlane - nearPlane);

    result.m0 = 2.0f/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;
    result.m4 = 0.0f;
    result.m5 = 2.0f/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;
    result.m8 = 0.0f;
    result.m9 = 0.0f;
    result.m10 = -2.0f/fn;
    result.m11 = 0.0f;
    result.m12 = -((float)left + (float)right)/rl;
    result.m13 = -((float)top + (float)bottom)/tb;
    result.m14 = -((float)farPlane + (float)nearPlane)/fn;
    result.m15 = 1.0f;

    return result;
}

// Get camera look-at matrix (view matrix)
RMAPI Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up)
{
    Matrix result = { 0 };

    float length = 0.0f;
    float ilength = 0.0f;

    // Vector3Subtract(eye, target)
    Vector3 vz = { eye.x - target.x, eye.y - target.y, eye.z - target.z };

    // Vector3Normalize(vz)
    Vector3 v = vz;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vz.x *= ilength;
    vz.y *= ilength;
    vz.z *= ilength;

    // Vector3CrossProduct(up, vz)
    Vector3 vx = { up.y*vz.z - up.z*vz.y, up.z*vz.x - up.x*vz.z, up.x*vz.y - up.y*vz.x };

    // Vector3Normalize(x)
    v = vx;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vx.x *= ilength;
    vx.y *= ilength;
    vx.z *= ilength;

    // Vector3CrossProduct(vz, vx)
    Vector3 vy = { vz.y*vx.z - vz.z*vx.y, vz.z*vx.x - vz.x*vx.z, vz.x*vx.y - vz.y*vx.x };

    result.m0 = vx.x;
    result.m1 = vy.x;
    result.m2 = vz.x;
    result.m3 = 0.0f;
    result.m4 = vx.y;
    result.m5 = vy.y;
    result.m6 = vz.y;
    result.m7 = 0.0f;
    result.m8 = vx.z;
    result.m9 = vy.z;
    result.m10 = vz.z;
    result.m11 = 0.0f;
    result.m12 = -(vx.x*eye.x + vx.y*eye.y + vx.z*eye.z);   // Vector3DotProduct(vx, eye)
    result.m13 = -(vy.x*eye.x + vy.y*eye.y + vy.z*eye.z);   // Vector3DotProduct(vy, eye)
    result.m14 = -(vz.x*eye.x + vz.y*eye.y + vz.z*eye.z);   // Vector3DotProduct(vz, eye)
    result.m15 = 1.0f;

    return result;
}

// Get float array of matrix data
RMAPI float16 MatrixToFloatV(Matrix mat)
{
    float16 result = { 0 };

    result.v[0] = mat.m0;
    result.v[1] = mat.m1;
    result.v[2] = mat.m2;
    result.v[3] = mat.m3;
    result.v[4] = mat.m4;
    result.v[5] = mat.m5;
    result.v[6] = mat.m6;
    result.v[7] = mat.m7;
    result.v[8] = mat.m8;
    result.v[9] = mat.m9;
    result.v[10] = mat.m10;
    result.v[11] = mat.m11;
    result.v[12] = mat.m12;
    result.v[13] = mat.m13;
    result.v[14] = mat.m14;
    result.v[15] = mat.m15;

    return result;
}

RMAPI Vector4 MatrixColumn(Matrix mat, size_t i)
{
	float16 m = MatrixToFloatV(mat);
	return (Vector4){ m.v[((i % 4) * 4) + 0], m.v[((i % 4) * 4) + 1], m.v[((i % 4) * 4) + 2], m.v[((i % 4) * 4) + 3] };
}

// Transform a quaternion given a transformation matrix
RMAPI Quaternion QuaternionTransform(Quaternion q, Matrix mat) { return (Quaternion){ Vector4DotProduct(MatrixColumn(mat, 0), q), Vector4DotProduct(MatrixColumn(mat, 1), q), Vector4DotProduct(MatrixColumn(mat, 2), q), Vector4DotProduct(MatrixColumn(mat,3), q) }; }

// Projects a Vector3 from screen space into object space
// NOTE: We are avoiding calling other raymath functions despite available
RMAPI Vector3 Vector3Unproject(Vector3 source, Matrix projection, Matrix view)
{
    // Calculate unprojected matrix (multiply view matrix by projection matrix) and invert it
    Matrix matViewProj = MatrixMultiply(view, projection);

    // Calculate inverted matrix -> MatrixInvert(matViewProj);
    Matrix matViewProjInv = MatrixInvert(matViewProj);

    // Create quaternion from source point
    Quaternion quat = { source.x, source.y, source.z, 1.0f };

    // Multiply quat point by unprojecte matrix
    Quaternion qtransformed = QuaternionTransform(quat, matViewProjInv);

    // Normalized world points in vectors
    return Vector3Scale((Vector3){qtransformed.x, qtransformed.y, qtransformed.y}, 1/qtransformed.w);
}

// Check whether two given vectors are almost equal


// Calculate quaternion cubic spline interpolation using Cubic Hermite Spline algorithm
// as described in the GLTF 2.0 specification: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#interpolation-cubic
RMAPI Quaternion QuaternionCubicHermiteSpline(Quaternion q1, Quaternion outTangent1, Quaternion q2, Quaternion inTangent2, float t)
{
    float t2 = t*t;
    float t3 = t2*t;
    float h00 = 2*t3 - 3*t2 + 1;
    float h10 = t3 - 2*t2 + t;
    float h01 = -2*t3 + 3*t2;
    float h11 = t3 - t2;

    Quaternion p0 = QuaternionScale(q1, h00);
    Quaternion m0 = QuaternionScale(outTangent1, h10);
    Quaternion p1 = QuaternionScale(q2, h01);
    Quaternion m1 = QuaternionScale(inTangent2, h11);

    Quaternion result = { 0 };

    result = QuaternionAdd(p0, m0);
    result = QuaternionAdd(result, p1);
    result = QuaternionAdd(result, m1);
    result = QuaternionNormalize(result);

    return result;
}

// Get a quaternion for a given rotation matrix
RMAPI Quaternion QuaternionFromMatrix(Matrix mat)
{
    Quaternion result = { 0 };

    float fourWSquaredMinus1 = mat.m0  + mat.m5 + mat.m10;
    float fourXSquaredMinus1 = mat.m0  - mat.m5 - mat.m10;
    float fourYSquaredMinus1 = mat.m5  - mat.m0 - mat.m10;
    float fourZSquaredMinus1 = mat.m10 - mat.m0 - mat.m5;

    int biggestIndex = 0;
    float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourXSquaredMinus1;
        biggestIndex = 1;
    }

    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourYSquaredMinus1;
        biggestIndex = 2;
    }

    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourZSquaredMinus1;
        biggestIndex = 3;
    }

    float biggestVal = sqrtf(fourBiggestSquaredMinus1 + 1.0f)*0.5f;
    float mult = 0.25f/biggestVal;

    switch (biggestIndex)
    {
        case 0:
            result.w = biggestVal;
            result.x = (mat.m6 - mat.m9)*mult;
            result.y = (mat.m8 - mat.m2)*mult;
            result.z = (mat.m1 - mat.m4)*mult;
            break;
        case 1:
            result.x = biggestVal;
            result.w = (mat.m6 - mat.m9)*mult;
            result.y = (mat.m1 + mat.m4)*mult;
            result.z = (mat.m8 + mat.m2)*mult;
            break;
        case 2:
            result.y = biggestVal;
            result.w = (mat.m8 - mat.m2)*mult;
            result.x = (mat.m1 + mat.m4)*mult;
            result.z = (mat.m6 + mat.m9)*mult;
            break;
        case 3:
            result.z = biggestVal;
            result.w = (mat.m1 - mat.m4)*mult;
            result.x = (mat.m8 + mat.m2)*mult;
            result.y = (mat.m6 + mat.m9)*mult;
            break;
    }

    return result;
}

// Get a matrix for a given quaternion
RMAPI Matrix QuaternionToMatrix(Quaternion q)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    float a2 = q.x*q.x;
    float b2 = q.y*q.y;
    float c2 = q.z*q.z;
    float ac = q.x*q.z;
    float ab = q.x*q.y;
    float bc = q.y*q.z;
    float ad = q.w*q.x;
    float bd = q.w*q.y;
    float cd = q.w*q.z;

    result.m0 = 1 - 2*(b2 + c2);
    result.m1 = 2*(ab + cd);
    result.m2 = 2*(ac - bd);

    result.m4 = 2*(ab - cd);
    result.m5 = 1 - 2*(a2 + c2);
    result.m6 = 2*(bc + ad);

    result.m8 = 2*(ac + bd);
    result.m9 = 2*(bc - ad);
    result.m10 = 1 - 2*(a2 + b2);

    return result;
}

// Get rotation quaternion for an angle and axis
// NOTE: Angle must be provided in radians
RMAPI Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle)
{
    Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };

    if (Vector3Length(axis) != 0)
	QuaternionNormalize(Vector3Vector4(Vector3Scale(Vector3Normalize(axis),sinf(angle * 0.5f)), cosf(angle * 0.5f)));

    return result;
}

// Get the rotation angle and axis for a given quaternion
RMAPI void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle)
{
    if (fabsf(q.w) > 1.0f)
        q = QuaternionNormalize(q);

    Vector3 resAxis = { 0.0f, 0.0f, 0.0f };
    float resAngle = 2.0f*acosf(q.w);
    float den = sqrtf(1.0f - q.w*q.w);

    if (den > EPSILON)
    {
        resAxis.x = q.x/den;
        resAxis.y = q.y/den;
        resAxis.z = q.z/den;
    }
    else
    {
        // This occurs when the angle is zero.
        // Not a problem: just set an arbitrary normalized axis.
        resAxis.x = 1.0f;
    }

    *outAxis = resAxis;
    *outAngle = resAngle;
}

// Get the quaternion equivalent to Euler angles
// NOTE: Rotation order is ZYX
RMAPI Quaternion QuaternionFromEuler(float pitch, float yaw, float roll)
{
    Quaternion result = { 0 };

    float x0 = cosf(pitch*0.5f);
    float x1 = sinf(pitch*0.5f);
    float y0 = cosf(yaw*0.5f);
    float y1 = sinf(yaw*0.5f);
    float z0 = cosf(roll*0.5f);
    float z1 = sinf(roll*0.5f);

    result.x = x1*y0*z0 - x0*y1*z1;
    result.y = x0*y1*z0 + x1*y0*z1;
    result.z = x0*y0*z1 - x1*y1*z0;
    result.w = x0*y0*z0 + x1*y1*z1;

    return result;
}

// Get the Euler angles equivalent to quaternion (roll, pitch, yaw)
// NOTE: Angles are returned in a Vector3 struct in radians
RMAPI Vector3 QuaternionToEuler(Quaternion q)
{
    Vector3 result = { 0 };

    // Roll (x-axis rotation)
    float x0 = 2.0f*(q.w*q.x + q.y*q.z);
    float x1 = 1.0f - 2.0f*(q.x*q.x + q.y*q.y);
    result.x = atan2f(x0, x1);

    // Pitch (y-axis rotation)
    float y0 = 2.0f*(q.w*q.y - q.z*q.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    result.y = asinf(y0);

    // Yaw (z-axis rotation)
    float z0 = 2.0f*(q.w*q.z + q.x*q.y);
    float z1 = 1.0f - 2.0f*(q.y*q.y + q.z*q.z);
    result.z = atan2f(z0, z1);

    return result;
}


// Decompose a transformation matrix into its rotational, translational and scaling components
RMAPI void MatrixDecompose(Matrix mat, Vector3 *translation, Quaternion *rotation, Vector3 *scale)
{
    // Extract translation.
    translation->x = mat.m12;
    translation->y = mat.m13;
    translation->z = mat.m14;

    // Extract upper-left for determinant computation
    const float a = mat.m0;
    const float b = mat.m4;
    const float c = mat.m8;
    const float d = mat.m1;
    const float e = mat.m5;
    const float f = mat.m9;
    const float g = mat.m2;
    const float h = mat.m6;
    const float i = mat.m10;
    const float A = e*i - f*h;
    const float B = f*g - d*i;
    const float C = d*h - e*g;

    // Extract scale
    const float det = a*A + b*B + c*C;
    Vector3 abc = { a, b, c };
    Vector3 def = { d, e, f };
    Vector3 ghi = { g, h, i };

    float scalex = Vector3Length(abc);
    float scaley = Vector3Length(def);
    float scalez = Vector3Length(ghi);
    Vector3 s = { scalex, scaley, scalez };

    if (det < 0) s = Vector3Negate(s);

    *scale = s;

    // Remove scale from the matrix if it is not close to zero
    Matrix clone = mat;
    if (!FloatEquals(det, 0))
    {
        clone.m0 /= s.x;
        clone.m4 /= s.x;
        clone.m8 /= s.x;
        clone.m1 /= s.y;
        clone.m5 /= s.y;
        clone.m9 /= s.y;
        clone.m2 /= s.z;
        clone.m6 /= s.z;
        clone.m10 /= s.z;

        // Extract rotation
        *rotation = QuaternionFromMatrix(clone);
    }
    else
    {
        // Set to identity if close to zero
        *rotation = QuaternionIdentity();
    }
}

#if defined(__cplusplus) && !defined(RAYMATH_DISABLE_CPP_OPERATORS)
#include "raymath.hpp"
#endif  // C++ operators

#endif  // RAYMATH_H
