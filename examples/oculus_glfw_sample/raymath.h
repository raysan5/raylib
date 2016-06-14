/**********************************************************************************************
*
*   raymath (header only file)
*
*   Some useful functions to work with Vector3, Matrix and Quaternions
*
*   You must:
*       #define RAYMATH_IMPLEMENTATION
*   before you include this file in *only one* C or C++ file to create the implementation.
*
*   Example:
*       #define RAYMATH_IMPLEMENTATION
*       #include "raymath.h"
*
*   You can also use:
*       #define RAYMATH_EXTERN_INLINE       // Inlines all functions code, so it runs faster.
*                                           // This requires lots of memory on system.
*       #define RAYMATH_STANDALONE          // Not dependent on raylib.h structs: Vector3, Matrix.
*
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
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

//#define RAYMATH_STANDALONE        // NOTE: To use raymath as standalone lib, just uncomment this line
//#define RAYMATH_EXTERN_INLINE     // NOTE: To compile functions as static inline, uncomment this line

#ifndef RAYMATH_STANDALONE
    #include "raylib.h"             // Required for structs: Vector3, Matrix
#endif

#ifdef __cplusplus
    #define RMEXTERN extern "C"     // Functions visible from other files (no name mangling of functions in C++)
#else
    #define RMEXTERN extern         // Functions visible from other files
#endif

#if defined(RAYMATH_EXTERN_INLINE)
    #define RMDEF RMEXTERN inline   // Functions are embeded inline (compiler generated code)
#else
    #define RMDEF RMEXTERN
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef PI
    #define PI 3.14159265358979323846
#endif

#ifndef DEG2RAD
    #define DEG2RAD (PI/180.0f)
#endif

#ifndef RAD2DEG
    #define RAD2DEG (180.0f/PI)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

#if defined(RAYMATH_STANDALONE)
    // Vector2 type
    typedef struct Vector2 {
        float x;
        float y;
    } Vector2;

    // Vector3 type
    typedef struct Vector3 {
        float x;
        float y;
        float z;
    } Vector3;

    // Matrix type (OpenGL style 4x4 - right handed, column major)
    typedef struct Matrix {
        float m0, m4, m8, m12;
        float m1, m5, m9, m13;
        float m2, m6, m10, m14;
        float m3, m7, m11, m15;
    } Matrix;
#endif

// Quaternion type
typedef struct Quaternion {
    float x;
    float y;
    float z;
    float w;
} Quaternion;

#ifndef RAYMATH_EXTERN_INLINE

//------------------------------------------------------------------------------------
// Functions Declaration to work with Vector3
//------------------------------------------------------------------------------------
RMDEF Vector3 VectorAdd(Vector3 v1, Vector3 v2);              // Add two vectors
RMDEF Vector3 VectorSubtract(Vector3 v1, Vector3 v2);         // Substract two vectors
RMDEF Vector3 VectorCrossProduct(Vector3 v1, Vector3 v2);     // Calculate two vectors cross product
RMDEF Vector3 VectorPerpendicular(Vector3 v);                 // Calculate one vector perpendicular vector
RMDEF float VectorDotProduct(Vector3 v1, Vector3 v2);         // Calculate two vectors dot product
RMDEF float VectorLength(const Vector3 v);                    // Calculate vector lenght
RMDEF void VectorScale(Vector3 *v, float scale);              // Scale provided vector
RMDEF void VectorNegate(Vector3 *v);                          // Negate provided vector (invert direction)
RMDEF void VectorNormalize(Vector3 *v);                       // Normalize provided vector
RMDEF float VectorDistance(Vector3 v1, Vector3 v2);           // Calculate distance between two points
RMDEF Vector3 VectorLerp(Vector3 v1, Vector3 v2, float amount); // Calculate linear interpolation between two vectors
RMDEF Vector3 VectorReflect(Vector3 vector, Vector3 normal);  // Calculate reflected vector to normal
RMDEF void VectorTransform(Vector3 *v, Matrix mat);           // Transforms a Vector3 by a given Matrix
RMDEF Vector3 VectorZero(void);                               // Return a Vector3 init to zero
RMDEF Vector3 VectorMin(Vector3 vec1, Vector3 vec2);          // Return min value for each pair of components
RMDEF Vector3 VectorMax(Vector3 vec1, Vector3 vec2);          // Return max value for each pair of components

//------------------------------------------------------------------------------------
// Functions Declaration to work with Matrix
//------------------------------------------------------------------------------------
RMDEF float MatrixDeterminant(Matrix mat);                    // Compute matrix determinant
RMDEF float MatrixTrace(Matrix mat);                          // Returns the trace of the matrix (sum of the values along the diagonal)
RMDEF void MatrixTranspose(Matrix *mat);                      // Transposes provided matrix
RMDEF void MatrixInvert(Matrix *mat);                         // Invert provided matrix
RMDEF void MatrixNormalize(Matrix *mat);                      // Normalize provided matrix
RMDEF Matrix MatrixIdentity(void);                            // Returns identity matrix
RMDEF Matrix MatrixAdd(Matrix left, Matrix right);            // Add two matrices
RMDEF Matrix MatrixSubstract(Matrix left, Matrix right);      // Substract two matrices (left - right)
RMDEF Matrix MatrixTranslate(float x, float y, float z);      // Returns translation matrix
RMDEF Matrix MatrixRotate(Vector3 axis, float angle);         // Returns rotation matrix for an angle around an specified axis (angle in radians)
RMDEF Matrix MatrixRotateX(float angle);                      // Returns x-rotation matrix (angle in radians)
RMDEF Matrix MatrixRotateY(float angle);                      // Returns y-rotation matrix (angle in radians)
RMDEF Matrix MatrixRotateZ(float angle);                      // Returns z-rotation matrix (angle in radians)
RMDEF Matrix MatrixScale(float x, float y, float z);          // Returns scaling matrix
RMDEF Matrix MatrixMultiply(Matrix left, Matrix right);       // Returns two matrix multiplication
RMDEF Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far);  // Returns perspective projection matrix
RMDEF Matrix MatrixPerspective(double fovy, double aspect, double near, double far);                        // Returns perspective projection matrix
RMDEF Matrix MatrixOrtho(double left, double right, double bottom, double top, double near, double far);    // Returns orthographic projection matrix
RMDEF Matrix MatrixLookAt(Vector3 position, Vector3 target, Vector3 up);  // Returns camera look-at matrix (view matrix)

//------------------------------------------------------------------------------------
// Functions Declaration to work with Quaternions
//------------------------------------------------------------------------------------
RMDEF float QuaternionLength(Quaternion quat);                // Compute the length of a quaternion
RMDEF void QuaternionNormalize(Quaternion *q);                // Normalize provided quaternion
RMDEF void QuaternionInvert(Quaternion *quat);                // Invert provided quaternion
RMDEF Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2);    // Calculate two quaternion multiplication
RMDEF Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float slerp); // Calculates spherical linear interpolation between two quaternions
RMDEF Quaternion QuaternionFromMatrix(Matrix matrix);                 // Returns a quaternion for a given rotation matrix
RMDEF Matrix QuaternionToMatrix(Quaternion q);                        // Returns a matrix for a given quaternion
RMDEF Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle);  // Returns rotation quaternion for an angle and axis
RMDEF void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle); // Returns the rotation angle and axis for a given quaternion
RMDEF void QuaternionTransform(Quaternion *q, Matrix mat);            // Transform a quaternion given a transformation matrix

#endif  // notdef RAYMATH_EXTERN_INLINE

#endif  // RAYMATH_H
//////////////////////////////////////////////////////////////////// end of header file

#if defined(RAYMATH_IMPLEMENTATION) || defined(RAYMATH_EXTERN_INLINE)

#include <math.h>       // Required for: sinf(), cosf(), tan(), fabs()

//----------------------------------------------------------------------------------
// Module Functions Definition - Vector3 math
//----------------------------------------------------------------------------------

// Add two vectors
RMDEF Vector3 VectorAdd(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
}

// Substract two vectors
RMDEF Vector3 VectorSubtract(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

// Calculate two vectors cross product
RMDEF Vector3 VectorCrossProduct(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.y*v2.z - v1.z*v2.y;
    result.y = v1.z*v2.x - v1.x*v2.z;
    result.z = v1.x*v2.y - v1.y*v2.x;

    return result;
}

// Calculate one vector perpendicular vector
RMDEF Vector3 VectorPerpendicular(Vector3 v)
{
    Vector3 result;

    float min = fabs(v.x);
    Vector3 cardinalAxis = {1.0f, 0.0f, 0.0f};

    if (fabs(v.y) < min)
    {
        min = fabs(v.y);
        cardinalAxis = (Vector3){0.0f, 1.0f, 0.0f};
    }

    if(fabs(v.z) < min)
    {
        cardinalAxis = (Vector3){0.0f, 0.0f, 1.0f};
    }

    result = VectorCrossProduct(v, cardinalAxis);

    return result;
}

// Calculate two vectors dot product
RMDEF float VectorDotProduct(Vector3 v1, Vector3 v2)
{
    float result;

    result = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;

    return result;
}

// Calculate vector lenght
RMDEF float VectorLength(const Vector3 v)
{
    float length;

    length = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);

    return length;
}

// Scale provided vector
RMDEF void VectorScale(Vector3 *v, float scale)
{
    v->x *= scale;
    v->y *= scale;
    v->z *= scale;
}

// Negate provided vector (invert direction)
RMDEF void VectorNegate(Vector3 *v)
{
    v->x = -v->x;
    v->y = -v->y;
    v->z = -v->z;
}

// Normalize provided vector
RMDEF void VectorNormalize(Vector3 *v)
{
    float length, ilength;

    length = VectorLength(*v);

    if (length == 0) length = 1.0f;

    ilength = 1.0f/length;

    v->x *= ilength;
    v->y *= ilength;
    v->z *= ilength;
}

// Calculate distance between two points
RMDEF float VectorDistance(Vector3 v1, Vector3 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    result = sqrt(dx*dx + dy*dy + dz*dz);

    return result;
}

// Calculate linear interpolation between two vectors
RMDEF Vector3 VectorLerp(Vector3 v1, Vector3 v2, float amount)
{
    Vector3 result;

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);
    result.z = v1.z + amount*(v2.z - v1.z);

    return result;
}

// Calculate reflected vector to normal
RMDEF Vector3 VectorReflect(Vector3 vector, Vector3 normal)
{
    // I is the original vector
    // N is the normal of the incident plane
    // R = I - (2*N*( DotProduct[ I,N] ))

    Vector3 result;

    float dotProduct = VectorDotProduct(vector, normal);

    result.x = vector.x - (2.0f*normal.x)*dotProduct;
    result.y = vector.y - (2.0f*normal.y)*dotProduct;
    result.z = vector.z - (2.0f*normal.z)*dotProduct;

    return result;
}

// Transforms a Vector3 by a given Matrix
// TODO: Review math (matrix transpose required?)
RMDEF void VectorTransform(Vector3 *v, Matrix mat)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;

    v->x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;
    v->y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;
    v->z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14;
};

// Return a Vector3 init to zero
RMDEF Vector3 VectorZero(void)
{
    Vector3 zero = { 0.0f, 0.0f, 0.0f };

    return zero;
}

// Return min value for each pair of components
RMDEF Vector3 VectorMin(Vector3 vec1, Vector3 vec2)
{
    Vector3 result;
    
    result.x = fminf(vec1.x, vec2.x);
    result.y = fminf(vec1.y, vec2.y);
    result.z = fminf(vec1.z, vec2.z);
    
    return result;
}

// Return max value for each pair of components
RMDEF Vector3 VectorMax(Vector3 vec1, Vector3 vec2)
{
    Vector3 result;
    
    result.x = fmaxf(vec1.x, vec2.x);
    result.y = fmaxf(vec1.y, vec2.y);
    result.z = fmaxf(vec1.z, vec2.z);
    
    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix math
//----------------------------------------------------------------------------------

// Compute matrix determinant
RMDEF float MatrixDeterminant(Matrix mat)
{
    float result;

    // Cache the matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
    float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

    result = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
             a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
             a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
             a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
             a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
             a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;

    return result;
}

// Returns the trace of the matrix (sum of the values along the diagonal)
RMDEF float MatrixTrace(Matrix mat)
{
    return (mat.m0 + mat.m5 + mat.m10 + mat.m15);
}

// Transposes provided matrix
RMDEF void MatrixTranspose(Matrix *mat)
{
    Matrix temp;

    temp.m0 = mat->m0;
    temp.m1 = mat->m4;
    temp.m2 = mat->m8;
    temp.m3 = mat->m12;
    temp.m4 = mat->m1;
    temp.m5 = mat->m5;
    temp.m6 = mat->m9;
    temp.m7 = mat->m13;
    temp.m8 = mat->m2;
    temp.m9 = mat->m6;
    temp.m10 = mat->m10;
    temp.m11 = mat->m14;
    temp.m12 = mat->m3;
    temp.m13 = mat->m7;
    temp.m14 = mat->m11;
    temp.m15 = mat->m15;

    *mat = temp;
}

// Invert provided matrix
RMDEF void MatrixInvert(Matrix *mat)
{
    Matrix temp;

    // Cache the matrix values (speed optimization)
    float a00 = mat->m0, a01 = mat->m1, a02 = mat->m2, a03 = mat->m3;
    float a10 = mat->m4, a11 = mat->m5, a12 = mat->m6, a13 = mat->m7;
    float a20 = mat->m8, a21 = mat->m9, a22 = mat->m10, a23 = mat->m11;
    float a30 = mat->m12, a31 = mat->m13, a32 = mat->m14, a33 = mat->m15;

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

    temp.m0 = (a11*b11 - a12*b10 + a13*b09)*invDet;
    temp.m1 = (-a01*b11 + a02*b10 - a03*b09)*invDet;
    temp.m2 = (a31*b05 - a32*b04 + a33*b03)*invDet;
    temp.m3 = (-a21*b05 + a22*b04 - a23*b03)*invDet;
    temp.m4 = (-a10*b11 + a12*b08 - a13*b07)*invDet;
    temp.m5 = (a00*b11 - a02*b08 + a03*b07)*invDet;
    temp.m6 = (-a30*b05 + a32*b02 - a33*b01)*invDet;
    temp.m7 = (a20*b05 - a22*b02 + a23*b01)*invDet;
    temp.m8 = (a10*b10 - a11*b08 + a13*b06)*invDet;
    temp.m9 = (-a00*b10 + a01*b08 - a03*b06)*invDet;
    temp.m10 = (a30*b04 - a31*b02 + a33*b00)*invDet;
    temp.m11 = (-a20*b04 + a21*b02 - a23*b00)*invDet;
    temp.m12 = (-a10*b09 + a11*b07 - a12*b06)*invDet;
    temp.m13 = (a00*b09 - a01*b07 + a02*b06)*invDet;
    temp.m14 = (-a30*b03 + a31*b01 - a32*b00)*invDet;
    temp.m15 = (a20*b03 - a21*b01 + a22*b00)*invDet;

    *mat = temp;
}

// Normalize provided matrix
RMDEF void MatrixNormalize(Matrix *mat)
{
    float det = MatrixDeterminant(*mat);

    mat->m0 /= det;
    mat->m1 /= det;
    mat->m2 /= det;
    mat->m3 /= det;
    mat->m4 /= det;
    mat->m5 /= det;
    mat->m6 /= det;
    mat->m7 /= det;
    mat->m8 /= det;
    mat->m9 /= det;
    mat->m10 /= det;
    mat->m11 /= det;
    mat->m12 /= det;
    mat->m13 /= det;
    mat->m14 /= det;
    mat->m15 /= det;
}

// Returns identity matrix
RMDEF Matrix MatrixIdentity(void)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Add two matrices
RMDEF Matrix MatrixAdd(Matrix left, Matrix right)
{
    Matrix result = MatrixIdentity();

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

// Substract two matrices (left - right)
RMDEF Matrix MatrixSubstract(Matrix left, Matrix right)
{
    Matrix result = MatrixIdentity();

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

// Returns translation matrix
RMDEF Matrix MatrixTranslate(float x, float y, float z)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 1.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 1.0f, 0.0f, 
                      x, y, z, 1.0f };

    return result;
}

// Create rotation matrix from axis and angle
// NOTE: Angle should be provided in radians
RMDEF Matrix MatrixRotate(Vector3 axis, float angle)
{
    Matrix result;

    Matrix mat = MatrixIdentity();

    float x = axis.x, y = axis.y, z = axis.z;

    float length = sqrt(x*x + y*y + z*z);

    if ((length != 1.0f) && (length != 0.0f))
    {
        length = 1.0f/length;
        x *= length;
        y *= length;
        z *= length;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    // Cache some matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;

    // Construct the elements of the rotation matrix
    float b00 = x*x*t + cosres, b01 = y*x*t + z*sinres, b02 = z*x*t - y*sinres;
    float b10 = x*y*t - z*sinres, b11 = y*y*t + cosres, b12 = z*y*t + x*sinres;
    float b20 = x*z*t + y*sinres, b21 = y*z*t - x*sinres, b22 = z*z*t + cosres;

    // Perform rotation-specific matrix multiplication
    result.m0 = a00*b00 + a10*b01 + a20*b02;
    result.m1 = a01*b00 + a11*b01 + a21*b02;
    result.m2 = a02*b00 + a12*b01 + a22*b02;
    result.m3 = a03*b00 + a13*b01 + a23*b02;
    result.m4 = a00*b10 + a10*b11 + a20*b12;
    result.m5 = a01*b10 + a11*b11 + a21*b12;
    result.m6 = a02*b10 + a12*b11 + a22*b12;
    result.m7 = a03*b10 + a13*b11 + a23*b12;
    result.m8 = a00*b20 + a10*b21 + a20*b22;
    result.m9 = a01*b20 + a11*b21 + a21*b22;
    result.m10 = a02*b20 + a12*b21 + a22*b22;
    result.m11 = a03*b20 + a13*b21 + a23*b22;
    result.m12 = mat.m12;
    result.m13 = mat.m13;
    result.m14 = mat.m14;
    result.m15 = mat.m15;

    return result;
}

/*
// Another implementation for MatrixRotate...
RMDEF Matrix MatrixRotate(float angle, float x, float y, float z)
{
    Matrix result = MatrixIdentity();

    float c = cosf(angle);      // cosine
    float s = sinf(angle);      // sine
    float c1 = 1.0f - c;        // 1 - c

    float m0 = result.m0, m4 = result.m4, m8 = result.m8, m12 = result.m12,
          m1 = result.m1, m5 = result.m5, m9 = result.m9,  m13 = result.m13,
          m2 = result.m2, m6 = result.m6, m10 = result.m10, m14 = result.m14;

    // build rotation matrix
    float r0 = x*x*c1 + c;
    float r1 = x*y*c1 + z*s;
    float r2 = x*z*c1 - y*s;
    float r4 = x*y*c1 - z*s;
    float r5 = y*y*c1 + c;
    float r6 = y*z*c1 + x*s;
    float r8 = x*z*c1 + y*s;
    float r9 = y*z*c1 - x*s;
    float r10= z*z*c1 + c;

    // multiply rotation matrix
    result.m0 = r0*m0 + r4*m1 + r8*m2;
    result.m1 = r1*m0 + r5*m1 + r9*m2;
    result.m2 = r2*m0 + r6*m1 + r10*m2;
    result.m4 = r0*m4 + r4*m5 + r8*m6;
    result.m5 = r1*m4 + r5*m5 + r9*m6;
    result.m6 = r2*m4 + r6*m5 + r10*m6;
    result.m8 = r0*m8 + r4*m9 + r8*m10;
    result.m9 = r1*m8 + r5*m9 + r9*m10;
    result.m10 = r2*m8 + r6*m9 + r10*m10;
    result.m12 = r0*m12+ r4*m13 + r8*m14;
    result.m13 = r1*m12+ r5*m13 + r9*m14;
    result.m14 = r2*m12+ r6*m13 + r10*m14;

    return result;
}
*/

// Returns x-rotation matrix (angle in radians)
RMDEF Matrix MatrixRotateX(float angle)
{
    Matrix result = MatrixIdentity();

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m5 = cosres;
    result.m6 = -sinres;
    result.m9 = sinres;
    result.m10 = cosres;

    return result;
}

// Returns y-rotation matrix (angle in radians)
RMDEF Matrix MatrixRotateY(float angle)
{
    Matrix result = MatrixIdentity();

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m0 = cosres;
    result.m2 = sinres;
    result.m8 = -sinres;
    result.m10 = cosres;

    return result;
}

// Returns z-rotation matrix (angle in radians)
RMDEF Matrix MatrixRotateZ(float angle)
{
    Matrix result = MatrixIdentity();

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.m0 = cosres;
    result.m1 = -sinres;
    result.m4 = sinres;
    result.m5 = cosres;

    return result;
}

// Returns scaling matrix
RMDEF Matrix MatrixScale(float x, float y, float z)
{
    Matrix result = { x, 0.0f, 0.0f, 0.0f, 
                      0.0f, y, 0.0f, 0.0f, 
                      0.0f, 0.0f, z, 0.0f, 
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Returns two matrix multiplication
// NOTE: When multiplying matrices... the order matters!
RMDEF Matrix MatrixMultiply(Matrix left, Matrix right)
{
    Matrix result;

    result.m0 = right.m0*left.m0 + right.m1*left.m4 + right.m2*left.m8 + right.m3*left.m12;
    result.m1 = right.m0*left.m1 + right.m1*left.m5 + right.m2*left.m9 + right.m3*left.m13;
    result.m2 = right.m0*left.m2 + right.m1*left.m6 + right.m2*left.m10 + right.m3*left.m14;
    result.m3 = right.m0*left.m3 + right.m1*left.m7 + right.m2*left.m11 + right.m3*left.m15;
    result.m4 = right.m4*left.m0 + right.m5*left.m4 + right.m6*left.m8 + right.m7*left.m12;
    result.m5 = right.m4*left.m1 + right.m5*left.m5 + right.m6*left.m9 + right.m7*left.m13;
    result.m6 = right.m4*left.m2 + right.m5*left.m6 + right.m6*left.m10 + right.m7*left.m14;
    result.m7 = right.m4*left.m3 + right.m5*left.m7 + right.m6*left.m11 + right.m7*left.m15;
    result.m8 = right.m8*left.m0 + right.m9*left.m4 + right.m10*left.m8 + right.m11*left.m12;
    result.m9 = right.m8*left.m1 + right.m9*left.m5 + right.m10*left.m9 + right.m11*left.m13;
    result.m10 = right.m8*left.m2 + right.m9*left.m6 + right.m10*left.m10 + right.m11*left.m14;
    result.m11 = right.m8*left.m3 + right.m9*left.m7 + right.m10*left.m11 + right.m11*left.m15;
    result.m12 = right.m12*left.m0 + right.m13*left.m4 + right.m14*left.m8 + right.m15*left.m12;
    result.m13 = right.m12*left.m1 + right.m13*left.m5 + right.m14*left.m9 + right.m15*left.m13;
    result.m14 = right.m12*left.m2 + right.m13*left.m6 + right.m14*left.m10 + right.m15*left.m14;
    result.m15 = right.m12*left.m3 + right.m13*left.m7 + right.m14*left.m11 + right.m15*left.m15;

    return result;
}

// Returns perspective projection matrix
RMDEF Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far)
{
    Matrix result;

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (far - near);

    result.m0 = (near*2.0f)/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;

    result.m4 = 0.0f;
    result.m5 = (near*2.0f)/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;

    result.m8 = (right + left)/rl;
    result.m9 = (top + bottom)/tb;
    result.m10 = -(far + near)/fn;
    result.m11 = -1.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = -(far*near*2.0f)/fn;
    result.m15 = 0.0f;

    return result;
}

// Returns perspective projection matrix
RMDEF Matrix MatrixPerspective(double fovy, double aspect, double near, double far)
{
    double top = near*tan(fovy*PI/360.0);
    double right = top*aspect;

    return MatrixFrustum(-right, right, -top, top, near, far);
}

// Returns orthographic projection matrix
RMDEF Matrix MatrixOrtho(double left, double right, double bottom, double top, double near, double far)
{
    Matrix result;

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (far - near);

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
    result.m12 = -(left + right)/rl;
    result.m13 = -(top + bottom)/tb;
    result.m14 = -(far + near)/fn;
    result.m15 = 1.0f;

    return result;
}

// Returns camera look-at matrix (view matrix)
RMDEF Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up)
{
    Matrix result;

    Vector3 z = VectorSubtract(eye, target);
    VectorNormalize(&z);
    Vector3 x = VectorCrossProduct(up, z);
    VectorNormalize(&x);
    Vector3 y = VectorCrossProduct(z, x);
    VectorNormalize(&y);

    result.m0 = x.x;
    result.m1 = x.y;
    result.m2 = x.z;
    result.m3 = -((x.x*eye.x) + (x.y*eye.y) + (x.z*eye.z));
    result.m4 = y.x;
    result.m5 = y.y;
    result.m6 = y.z;
    result.m7 = -((y.x*eye.x) + (y.y*eye.y) + (y.z*eye.z));
    result.m8 = z.x;
    result.m9 = z.y;
    result.m10 = z.z;
    result.m11 = -((z.x*eye.x) + (z.y*eye.y) + (z.z*eye.z));
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Quaternion math
//----------------------------------------------------------------------------------

// Computes the length of a quaternion
RMDEF float QuaternionLength(Quaternion quat)
{
    return sqrt(quat.x*quat.x + quat.y*quat.y + quat.z*quat.z + quat.w*quat.w);
}

// Normalize provided quaternion
RMDEF void QuaternionNormalize(Quaternion *q)
{
    float length, ilength;

    length = QuaternionLength(*q);

    if (length == 0.0f) length = 1.0f;

    ilength = 1.0f/length;

    q->x *= ilength;
    q->y *= ilength;
    q->z *= ilength;
    q->w *= ilength;
}

// Invert provided quaternion
RMDEF void QuaternionInvert(Quaternion *quat)
{
    float length = QuaternionLength(*quat);
    float lengthSq = length*length;
    
    if (lengthSq != 0.0)
    {
        float i = 1.0f/lengthSq;
        
        quat->x *= -i;
        quat->y *= -i;
        quat->z *= -i;
        quat->w *= i;
    }
}

// Calculate two quaternion multiplication
RMDEF Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2)
{
    Quaternion result;

    float qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
    float qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;

    result.x = qax*qbw + qaw*qbx + qay*qbz - qaz*qby;
    result.y = qay*qbw + qaw*qby + qaz*qbx - qax*qbz;
    result.z = qaz*qbw + qaw*qbz + qax*qby - qay*qbx;
    result.w = qaw*qbw - qax*qbx - qay*qby - qaz*qbz;

    return result;
}

// Calculates spherical linear interpolation between two quaternions
RMDEF Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float amount)
{
    Quaternion result;

    float cosHalfTheta =  q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (fabs(cosHalfTheta) >= 1.0f) result = q1;
    else
    {
        float halfTheta = acos(cosHalfTheta);
        float sinHalfTheta = sqrt(1.0f - cosHalfTheta*cosHalfTheta);

        if (fabs(sinHalfTheta) < 0.001f)
        {
            result.x = (q1.x*0.5f + q2.x*0.5f);
            result.y = (q1.y*0.5f + q2.y*0.5f);
            result.z = (q1.z*0.5f + q2.z*0.5f);
            result.w = (q1.w*0.5f + q2.w*0.5f);
        }
        else
        {
            float ratioA = sinf((1 - amount)*halfTheta)/sinHalfTheta;
            float ratioB = sinf(amount*halfTheta)/sinHalfTheta;

            result.x = (q1.x*ratioA + q2.x*ratioB);
            result.y = (q1.y*ratioA + q2.y*ratioB);
            result.z = (q1.z*ratioA + q2.z*ratioB);
            result.w = (q1.w*ratioA + q2.w*ratioB);
        }
    }

    return result;
}

// Returns a quaternion for a given rotation matrix
RMDEF Quaternion QuaternionFromMatrix(Matrix matrix)
{
    Quaternion result;

    float trace = MatrixTrace(matrix);

    if (trace > 0.0f)
    {
        float s = (float)sqrt(trace + 1)*2.0f;
        float invS = 1.0f/s;

        result.w = s*0.25f;
        result.x = (matrix.m6 - matrix.m9)*invS;
        result.y = (matrix.m8 - matrix.m2)*invS;
        result.z = (matrix.m1 - matrix.m4)*invS;
    }
    else
    {
        float m00 = matrix.m0, m11 = matrix.m5, m22 = matrix.m10;

        if (m00 > m11 && m00 > m22)
        {
            float s = (float)sqrt(1.0f + m00 - m11 - m22)*2.0f;
            float invS = 1.0f/s;

            result.w = (matrix.m6 - matrix.m9)*invS;
            result.x = s*0.25f;
            result.y = (matrix.m4 + matrix.m1)*invS;
            result.z = (matrix.m8 + matrix.m2)*invS;
        }
        else if (m11 > m22)
        {
            float s = (float)sqrt(1.0f + m11 - m00 - m22)*2.0f;
            float invS = 1.0f/s;

            result.w = (matrix.m8 - matrix.m2)*invS;
            result.x = (matrix.m4 + matrix.m1)*invS;
            result.y = s*0.25f;
            result.z = (matrix.m9 + matrix.m6)*invS;
        }
        else
        {
            float s = (float)sqrt(1.0f + m22 - m00 - m11)*2.0f;
            float invS = 1.0f/s;

            result.w = (matrix.m1 - matrix.m4)*invS;
            result.x = (matrix.m8 + matrix.m2)*invS;
            result.y = (matrix.m9 + matrix.m6)*invS;
            result.z = s*0.25f;
        }
    }

    return result;
}

// Returns a matrix for a given quaternion
RMDEF Matrix QuaternionToMatrix(Quaternion q)
{
    Matrix result;

    float x = q.x, y = q.y, z = q.z, w = q.w;

    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;

    float xx = x*x2;
    float xy = x*y2;
    float xz = x*z2;

    float yy = y*y2;
    float yz = y*z2;
    float zz = z*z2;

    float wx = w*x2;
    float wy = w*y2;
    float wz = w*z2;

    result.m0 = 1.0f - (yy + zz);
    result.m1 = xy - wz;
    result.m2 = xz + wy;
    result.m3 = 0.0f;
    result.m4 = xy + wz;
    result.m5 = 1.0f - (xx + zz);
    result.m6 = yz - wx;
    result.m7 = 0.0f;
    result.m8 = xz - wy;
    result.m9 = yz + wx;
    result.m10 = 1.0f - (xx + yy);
    result.m11 = 0.0f;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;
    
    return result;
}

// Returns rotation quaternion for an angle and axis
// NOTE: angle must be provided in radians
RMDEF Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle)
{
    Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };

    if (VectorLength(axis) != 0.0f)

    angle *= 0.5f;

    VectorNormalize(&axis);
    
    float sinres = sinf(angle);
    float cosres = cosf(angle);

    result.x = axis.x*sinres;
    result.y = axis.y*sinres;
    result.z = axis.z*sinres;
    result.w = cosres;

    QuaternionNormalize(&result);

    return result;
}

// Returns the rotation angle and axis for a given quaternion
RMDEF void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle)
{
    if (fabs(q.w) > 1.0f) QuaternionNormalize(&q);

    Vector3 resAxis = { 0.0f, 0.0f, 0.0f };
    float resAngle = 0.0f;

    resAngle = 2.0f*(float)acos(q.w);
    float den = (float)sqrt(1.0f - q.w*q.w);

    if (den > 0.0001f)
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

// Transform a quaternion given a transformation matrix
RMDEF void QuaternionTransform(Quaternion *q, Matrix mat)
{
    float x = q->x;
    float y = q->y;
    float z = q->z;
    float w = q->w;

    q->x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12*w;
    q->y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13*w;
    q->z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14*w;
    q->w = mat.m3*x + mat.m7*y + mat.m11*z + mat.m15*w;
}

#endif  // RAYMATH_IMPLEMENTATION
