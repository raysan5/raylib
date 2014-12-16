/**********************************************************************************************
*
*   raymath
*
*   Some useful functions to work with Vector3, Matrix and Quaternions
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

#include "raymath.h"

#include <stdio.h>      // Used only on PrintMatrix()
#include <math.h>       // Standard math libary: sin(), cos(), tan()...
#include <stdlib.h>     // Used for abs()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Definition - Vector3 math
//----------------------------------------------------------------------------------

// Add two vectors
Vector3 VectorAdd(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
}

// Substract two vectors
Vector3 VectorSubtract(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

// Calculate two vectors cross product
Vector3 VectorCrossProduct(Vector3 v1, Vector3 v2)
{
    Vector3 result;

    result.x = v1.y*v2.z - v1.z*v2.y;
    result.y = v1.z*v2.x - v1.x*v2.z;
    result.z = v1.x*v2.y - v1.y*v2.x;

    return result;
}

// Calculate one vector perpendicular vector
Vector3 VectorPerpendicular(Vector3 v)
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
float VectorDotProduct(Vector3 v1, Vector3 v2)
{
    float result;

    result = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;

    return result;
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

// Negate provided vector (invert direction)
void VectorNegate(Vector3 *v)
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

// Calculate distance between two points
float VectorDistance(Vector3 v1, Vector3 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    result = sqrt(dx*dx + dy*dy + dz*dz);

    return result;
}

// Calculate linear interpolation between two vectors
Vector3 VectorLerp(Vector3 v1, Vector3 v2, float amount)
{
    Vector3 result;

    result.x = v1.x + amount * (v2.x - v1.x);
    result.y = v1.y + amount * (v2.y - v1.y);
    result.z = v1.z + amount * (v2.z - v1.z);

    return result;
}

// Calculate reflected vector to normal
Vector3 VectorReflect(Vector3 vector, Vector3 normal)
{
    // I is the original vector
    // N is the normal of the incident plane
    // R = I - (2 * N * ( DotProduct[ I,N] ))

    Vector3 result;

    float dotProduct = VectorDotProduct(vector, normal);

    result.x = vector.x - (2.0 * normal.x) * dotProduct;
    result.y = vector.y - (2.0 * normal.y) * dotProduct;
    result.z = vector.z - (2.0 * normal.z) * dotProduct;

    return result;
}

// Transforms a Vector3 with a given Matrix
void VectorTransform(Vector3 *v, Matrix mat)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;

    //MatrixTranspose(&mat);

    v->x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;
    v->y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;
    v->z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14;
};

// Return a Vector3 init to zero
Vector3 VectorZero(void)
{
    Vector3 zero = { 0.0f, 0.0f, 0.0f };

    return zero;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix math
//----------------------------------------------------------------------------------

// Returns an OpenGL-ready vector (glMultMatrixf)
float *GetMatrixVector(Matrix mat)
{
    static float vector[16];

    vector[0] = mat.m0;
    vector[1] = mat.m4;
    vector[2] = mat.m8;
    vector[3] = mat.m12;
    vector[4] = mat.m1;
    vector[5] = mat.m5;
    vector[6] = mat.m9;
    vector[7] = mat.m13;
    vector[8] = mat.m2;
    vector[9] = mat.m6;
    vector[10] = mat.m10;
    vector[11] = mat.m14;
    vector[12] = mat.m3;
    vector[13] = mat.m7;
    vector[14] = mat.m11;
    vector[15] = mat.m15;

    return vector;
}

// Compute matrix determinant
float MatrixDeterminant(Matrix mat)
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
float MatrixTrace(Matrix mat)
{
    return (mat.m0 + mat.m5 + mat.m10 + mat.m15);
}

// Transposes provided matrix
void MatrixTranspose(Matrix *mat)
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
void MatrixInvert(Matrix *mat)
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
    float invDet = 1/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    printf("%f\n", invDet);

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

    PrintMatrix(temp);

    *mat = temp;
}

// Normalize provided matrix
void MatrixNormalize(Matrix *mat)
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
Matrix MatrixIdentity(void)
{
    Matrix result = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

    return result;
}

// Add two matrices
Matrix MatrixAdd(Matrix left, Matrix right)
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
Matrix MatrixSubstract(Matrix left, Matrix right)
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
// TODO: Review this function
Matrix MatrixTranslate(float x, float y, float z)
{
/*
    For OpenGL
        1, 0, 0, 0
        0, 1, 0, 0
        0, 0, 1, 0
        x, y, z, 1
    Is the correct Translation Matrix. Why? Opengl Uses column-major matrix ordering.
    Which is the Transpose of the Matrix you initially presented, which is in row-major ordering.
    Row major is used in most math text-books and also DirectX, so it is a common
    point of confusion for those new to OpenGL.

    * matrix notation used in opengl documentation does not describe in-memory layout for OpenGL matrices

    Translation matrix should be laid out in memory like this:
    { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, trabsX, transY, transZ, 1 }


    9.005 Are OpenGL matrices column-major or row-major?

    For programming purposes, OpenGL matrices are 16-value arrays with base vectors laid out
    contiguously in memory. The translation components occupy the 13th, 14th, and 15th elements
    of the 16-element matrix, where indices are numbered from 1 to 16 as described in section
    2.11.2 of the OpenGL 2.1 Specification.

    Column-major versus row-major is purely a notational convention. Note that post-multiplying
    with column-major matrices produces the same result as pre-multiplying with row-major matrices.
    The OpenGL Specification and the OpenGL Reference Manual both use column-major notation.
    You can use any notation, as long as it's clearly stated.

    Sadly, the use of column-major format in the spec and blue book has resulted in endless confusion
    in the OpenGL programming community. Column-major notation suggests that matrices
    are not laid out in memory as a programmer would expect.
*/

    Matrix result = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1 };

    return result;
}

// Returns rotation matrix
// TODO: Review this function
Matrix MatrixRotate(float angleX, float angleY, float angleZ)
{
    Matrix result;

    Matrix rotX = MatrixRotateX(angleX);
    Matrix rotY = MatrixRotateY(angleY);
    Matrix rotZ = MatrixRotateZ(angleZ);

    result = MatrixMultiply(MatrixMultiply(rotX, rotY), rotZ);

    return result;
}

// Create rotation matrix from axis and angle
// TODO: Test this function
// NOTE: NO prototype defined!
Matrix MatrixFromAxisAngle(Vector3 axis, float angle)
{
    Matrix result;

    Matrix mat = MatrixIdentity();

    float x = axis.x, y = axis.y, z = axis.z;

    float length = sqrt(x*x + y*y + z*z);

    if ((length != 1) && (length != 0))
    {
        length = 1 / length;
        x *= length;
        y *= length;
        z *= length;
    }

    float s = sin(angle);
    float c = cos(angle);
    float t = 1-c;

    // Cache some matrix values (speed optimization)
    float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;

    // Construct the elements of the rotation matrix
    float b00 = x*x*t + c, b01 = y*x*t + z*s, b02 = z*x*t - y*s;
    float b10 = x*y*t - z*s, b11 = y*y*t + c, b12 = z*y*t + x*s;
    float b20 = x*z*t + y*s, b21 = y*z*t - x*s, b22 = z*z*t + c;

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
};

// Create rotation matrix from axis and angle (version 2)
// TODO: Test this function
// NOTE: NO prototype defined!
Matrix MatrixFromAxisAngle2(Vector3 axis, float angle)
{
    Matrix result;

    VectorNormalize(&axis);
    float axisX = axis.x, axisY = axis.y, axisZ = axis.y;

    // Calculate angles
    float cosres = (float)cos(angle);
    float sinres = (float)sin(angle);
    float t = 1.0f - cosres;

    // Do the conversion math once
    float tXX = t * axisX * axisX;
    float tXY = t * axisX * axisY;
    float tXZ = t * axisX * axisZ;
    float tYY = t * axisY * axisY;
    float tYZ = t * axisY * axisZ;
    float tZZ = t * axisZ * axisZ;

    float sinX = sinres * axisX;
    float sinY = sinres * axisY;
    float sinZ = sinres * axisZ;

    result.m0 = tXX + cosres;
    result.m1 = tXY + sinZ;
    result.m2 = tXZ - sinY;
    result.m3 = 0;
    result.m4 = tXY - sinZ;
    result.m5 = tYY + cosres;
    result.m6 = tYZ + sinX;
    result.m7 = 0;
    result.m8 = tXZ + sinY;
    result.m9 = tYZ - sinX;
    result.m10 = tZZ + cosres;
    result.m11 = 0;
    result.m12 = 0;
    result.m13 = 0;
    result.m14 = 0;
    result.m15 = 1;

    return result;
}

// Returns rotation matrix for a given quaternion
Matrix MatrixFromQuaternion(Quaternion q)
{
    Matrix result = MatrixIdentity();

    Vector3 axis;
    float angle;

    QuaternionToAxisAngle(q, &axis, &angle);

    result = MatrixFromAxisAngle2(axis, angle);

    return result;
}

// Returns x-rotation matrix (angle in radians)
Matrix MatrixRotateX(float angle)
{
    Matrix result = MatrixIdentity();

    float cosres = (float)cos(angle);
    float sinres = (float)sin(angle);

    result.m5 = cosres;
    result.m6 = -sinres;
    result.m9 = sinres;
    result.m10 = cosres;

    return result;
}

// Returns y-rotation matrix (angle in radians)
Matrix MatrixRotateY(float angle)
{
    Matrix result = MatrixIdentity();

    float cosres = (float)cos(angle);
    float sinres = (float)sin(angle);

    result.m0 = cosres;
    result.m2 = sinres;
    result.m8 = -sinres;
    result.m10 = cosres;

    return result;
}

// Returns z-rotation matrix (angle in radians)
Matrix MatrixRotateZ(float angle)
{
    Matrix result = MatrixIdentity();

    float cosres = (float)cos(angle);
    float sinres = (float)sin(angle);

    result.m0 = cosres;
    result.m1 = -sinres;
    result.m4 = sinres;
    result.m5 = cosres;

    return result;
}

// Returns scaling matrix
Matrix MatrixScale(float x, float y, float z)
{
    Matrix result = { x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1 };

    return result;
}

// Returns transformation matrix for a given translation, rotation and scale
// NOTE: Transformation order is rotation -> scale -> translation
Matrix MatrixTransform(Vector3 translation, Vector3 rotation, Vector3 scale)
{
    Matrix result = MatrixIdentity();

	// TODO: Review, use DEG2RAD here?
	//Matrix mRotation = MatrixRotate(rotation.x*DEG2RAD, rotation.y*DEG2RAD, rotation.z*DEG2RAD);
    Matrix mRotation = MatrixRotate(rotation.x, rotation.y, rotation.z);
    Matrix mScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix mTranslate = MatrixTranslate(translation.x, translation.y, translation.z);

    result = MatrixMultiply(MatrixMultiply(mRotation, mScale), mTranslate);

    return result;
}

// Returns two matrix multiplication
// NOTE: When multiplying matrices... the order matters!
Matrix MatrixMultiply(Matrix left, Matrix right)
{
    Matrix result;

    // Cache the matrix values (speed optimization)
    float a00 = left.m0, a01 = left.m1, a02 = left.m2, a03 = left.m3;
    float a10 = left.m4, a11 = left.m5, a12 = left.m6, a13 = left.m7;
    float a20 = left.m8, a21 = left.m9, a22 = left.m10, a23 = left.m11;
    float a30 = left.m12, a31 = left.m13, a32 = left.m14, a33 = left.m15;

    float b00 = right.m0, b01 = right.m1, b02 = right.m2, b03 = right.m3;
    float b10 = right.m4, b11 = right.m5, b12 = right.m6, b13 = right.m7;
    float b20 = right.m8, b21 = right.m9, b22 = right.m10, b23 = right.m11;
    float b30 = right.m12, b31 = right.m13, b32 = right.m14, b33 = right.m15;

    result.m0 = b00*a00 + b01*a10 + b02*a20 + b03*a30;
    result.m1 = b00*a01 + b01*a11 + b02*a21 + b03*a31;
    result.m2 = b00*a02 + b01*a12 + b02*a22 + b03*a32;
    result.m3 = b00*a03 + b01*a13 + b02*a23 + b03*a33;
    result.m4 = b10*a00 + b11*a10 + b12*a20 + b13*a30;
    result.m5 = b10*a01 + b11*a11 + b12*a21 + b13*a31;
    result.m6 = b10*a02 + b11*a12 + b12*a22 + b13*a32;
    result.m7 = b10*a03 + b11*a13 + b12*a23 + b13*a33;
    result.m8 = b20*a00 + b21*a10 + b22*a20 + b23*a30;
    result.m9 = b20*a01 + b21*a11 + b22*a21 + b23*a31;
    result.m10 = b20*a02 + b21*a12 + b22*a22 + b23*a32;
    result.m11 = b20*a03 + b21*a13 + b22*a23 + b23*a33;
    result.m12 = b30*a00 + b31*a10 + b32*a20 + b33*a30;
    result.m13 = b30*a01 + b31*a11 + b32*a21 + b33*a31;
    result.m14 = b30*a02 + b31*a12 + b32*a22 + b33*a32;
    result.m15 = b30*a03 + b31*a13 + b32*a23 + b33*a33;

    return result;
}

// Returns perspective projection matrix
Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far)
{
    Matrix result;

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (far - near);

    result.m0 = (near*2.0f) / rl;
    result.m1 = 0;
    result.m2 = 0;
    result.m3 = 0;

    result.m4 = 0;
    result.m5 = (near*2.0f) / tb;
    result.m6 = 0;
    result.m7 = 0;

    result.m8 = (right + left) / rl;
    result.m9 = (top + bottom) / tb;
    result.m10 = -(far + near) / fn;
    result.m11 = -1.0f;

    result.m12 = 0;
    result.m13 = 0;
    result.m14 = -(far*near*2.0f) / fn;
    result.m15 = 0;

    return result;
}

// Returns perspective projection matrix
Matrix MatrixPerspective(double fovy, double aspect, double near, double far)
{
    double top = near*tanf(fovy*PI / 360.0f);
    double right = top*aspect;

    return MatrixFrustum(-right, right, -top, top, near, far);
}

// Returns orthographic projection matrix
Matrix MatrixOrtho(double left, double right, double bottom, double top, double near, double far)
{
    Matrix result;

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (far - near);

    result.m0 = 2 / rl;
    result.m1 = 0;
    result.m2 = 0;
    result.m3 = 0;
    result.m4 = 0;
    result.m5 = 2 / tb;
    result.m6 = 0;
    result.m7 = 0;
    result.m8 = 0;
    result.m9 = 0;
    result.m10 = -2 / fn;
    result.m11 = 0;
    result.m12 = -(left + right) / rl;
    result.m13 = -(top + bottom) / tb;
    result.m14 = -(far + near) / fn;
    result.m15 = 1;

    return result;
}

// Returns camera look-at matrix (view matrix)
Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up)
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
    result.m3 = -((x.x * eye.x) + (x.y * eye.y) + (x.z * eye.z));
    result.m4 = y.x;
    result.m5 = y.y;
    result.m6 = y.z;
    result.m7 = -((y.x * eye.x) + (y.y * eye.y) + (y.z * eye.z));
    result.m8 = z.x;
    result.m9 = z.y;
    result.m10 = z.z;
    result.m11 = -((z.x * eye.x) + (z.y * eye.y) + (z.z * eye.z));
    result.m12 = 0;
    result.m13 = 0;
    result.m14 = 0;
    result.m15 = 1;

    return result;
}

// Print matrix utility (for debug)
void PrintMatrix(Matrix m)
{
    printf("----------------------\n");
    printf("%2.2f %2.2f %2.2f %2.2f\n", m.m0, m.m4, m.m8, m.m12);
    printf("%2.2f %2.2f %2.2f %2.2f\n", m.m1, m.m5, m.m9, m.m13);
    printf("%2.2f %2.2f %2.2f %2.2f\n", m.m2, m.m6, m.m10, m.m14);
    printf("%2.2f %2.2f %2.2f %2.2f\n", m.m3, m.m7, m.m11, m.m15);
    printf("----------------------\n");
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Quaternion math
//----------------------------------------------------------------------------------

// Calculates the length of a quaternion
float QuaternionLength(Quaternion quat)
{
    return sqrt(quat.x*quat.x + quat.y*quat.y + quat.z*quat.z + quat.w*quat.w);
}

// Normalize provided quaternion
void QuaternionNormalize(Quaternion *q)
{
    float length, ilength;

    length = QuaternionLength(*q);

    if (length == 0) length = 1;

    ilength = 1.0/length;

    q->x *= ilength;
    q->y *= ilength;
    q->z *= ilength;
    q->w *= ilength;
}

// Calculate two quaternion multiplication
Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2)
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
Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float amount)
{
    Quaternion result;

    float cosHalfTheta =  q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (abs(cosHalfTheta) >= 1.0f) result = q1;
    else
    {
        float halfTheta = acos(cosHalfTheta);
        float sinHalfTheta = sqrt(1.0f - cosHalfTheta*cosHalfTheta);

        if (abs(sinHalfTheta) < 0.001f)
        {
            result.x = (q1.x*0.5f + q2.x*0.5f);
            result.y = (q1.y*0.5f + q2.y*0.5f);
            result.z = (q1.z*0.5f + q2.z*0.5f);
            result.w = (q1.w*0.5f + q2.w*0.5f);
        }
        else
        {
            float ratioA = sin((1 - amount)*halfTheta) / sinHalfTheta;
            float ratioB = sin(amount*halfTheta) / sinHalfTheta;

            result.x = (q1.x*ratioA + q2.x*ratioB);
            result.y = (q1.y*ratioA + q2.y*ratioB);
            result.z = (q1.z*ratioA + q2.z*ratioB);
            result.w = (q1.w*ratioA + q2.w*ratioB);
        }
    }

    return result;
}

// Returns a quaternion from a given rotation matrix
Quaternion QuaternionFromMatrix(Matrix matrix)
{
    Quaternion result;

    float trace = MatrixTrace(matrix);

    if (trace > 0)
    {
        float s = (float)sqrt(trace + 1) * 2;
        float invS = 1 / s;

        result.w = s * 0.25;
        result.x = (matrix.m6 - matrix.m9) * invS;
        result.y = (matrix.m8 - matrix.m2) * invS;
        result.z = (matrix.m1 - matrix.m4) * invS;
    }
    else
    {
        float m00 = matrix.m0, m11 = matrix.m5, m22 = matrix.m10;

        if (m00 > m11 && m00 > m22)
        {
            float s = (float)sqrt(1 + m00 - m11 - m22) * 2;
            float invS = 1 / s;

            result.w = (matrix.m6 - matrix.m9) * invS;
            result.x = s * 0.25;
            result.y = (matrix.m4 + matrix.m1) * invS;
            result.z = (matrix.m8 + matrix.m2) * invS;
        }
        else if (m11 > m22)
        {
            float s = (float)sqrt(1 + m11 - m00 - m22) * 2;
            float invS = 1 / s;

            result.w = (matrix.m8 - matrix.m2) * invS;
            result.x = (matrix.m4 + matrix.m1) * invS;
            result.y = s * 0.25;
            result.z = (matrix.m9 + matrix.m6) * invS;
        }
        else
        {
            float s = (float)sqrt(1 + m22 - m00 - m11) * 2;
            float invS = 1 / s;

            result.w = (matrix.m1 - matrix.m4) * invS;
            result.x = (matrix.m8 + matrix.m2) * invS;
            result.y = (matrix.m9 + matrix.m6) * invS;
            result.z = s * 0.25;
        }
    }

    return result;
}

// Returns rotation quaternion for an angle around an axis
// NOTE: angle must be provided in radians
Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle)
{
    Quaternion result = { 0, 0, 0, 1 };

    if (VectorLength(axis) != 0.0)

    angle *= 0.5;

    VectorNormalize(&axis);

    result.x = axis.x * (float)sin(angle);
    result.y = axis.y * (float)sin(angle);
    result.z = axis.z * (float)sin(angle);
    result.w = (float)cos(angle);

    QuaternionNormalize(&result);

    return result;
}

// Calculates the matrix from the given quaternion
Matrix QuaternionToMatrix(Quaternion q)
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

    result.m0 = 1 - (yy + zz);
    result.m1 = xy - wz;
    result.m2 = xz + wy;
    result.m3 = 0;
    result.m4 = xy + wz;
    result.m5 = 1 - (xx + zz);
    result.m6 = yz - wx;
    result.m7 = 0;
    result.m8 = xz - wy;
    result.m9 = yz + wx;
    result.m10 = 1 - (xx + yy);
    result.m11 = 0;
    result.m12 = 0;
    result.m13 = 0;
    result.m14 = 0;
    result.m15 = 1;

    return result;
}

// Returns the axis and the angle for a given quaternion
void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle)
{
    if (abs(q.w) > 1.0f) QuaternionNormalize(&q);

    Vector3 resAxis = { 0, 0, 0 };
    float resAngle = 0;

    resAngle = 2.0f * (float)acos(q.w);
    float den = (float)sqrt(1.0 - q.w * q.w);

    if (den > 0.0001f)
    {
        resAxis.x = q.x / den;
        resAxis.y = q.y / den;
        resAxis.z = q.z / den;
    }
    else
    {
        // This occurs when the angle is zero.
        // Not a problem: just set an arbitrary normalized axis.
        resAxis.x = 1.0;
    }

    *outAxis = resAxis;
    *outAngle = resAngle;
}