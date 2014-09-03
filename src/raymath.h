/*********************************************************************************************
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

#ifndef RAYMATH_H
#define RAYMATH_H

//#define RAYMATH_STANDALONE    // NOTE: To use raymath as standalone lib, just uncomment this line

#ifndef RAYMATH_STANDALONE
    #include "raylib.h"         // Required for typedef: Vector3
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef PI
    #define PI 3.14159265358979323846
#endif

#define DEG2RAD (PI / 180.0)
#define RAD2DEG (180.0 / PI)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

#ifdef RAYMATH_STANDALONE
    // Vector3 type
    typedef struct Vector3 {
        float x;
        float y;
        float z;
    } Vector3;
#endif

// Matrix type (OpenGL style 4x4 - right handed)
typedef struct Matrix {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} Matrix;

// Quaternion type
typedef struct Quaternion {
    float x;
    float y;
    float z;
    float w;
} Quaternion;


#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration to work with Vector3
//------------------------------------------------------------------------------------
Vector3 VectorAdd(Vector3 v1, Vector3 v2);              // Add two vectors
Vector3 VectorSubtract(Vector3 v1, Vector3 v2);         // Substract two vectors
Vector3 VectorCrossProduct(Vector3 v1, Vector3 v2);     // Calculate two vectors cross product
Vector3 VectorPerpendicular(Vector3 v);                 // Calculate one vector perpendicular vector
float VectorDotProduct(Vector3 v1, Vector3 v2);         // Calculate two vectors dot product
float VectorLength(const Vector3 v);                    // Calculate vector lenght
void VectorScale(Vector3 *v, float scale);              // Scale provided vector
void VectorNegate(Vector3 *v);                          // Negate provided vector (invert direction)
void VectorNormalize(Vector3 *v);                       // Normalize provided vector
float VectorDistance(Vector3 v1, Vector3 v2);           // Calculate distance between two points
Vector3 VectorLerp(Vector3 v1, Vector3 v2, float amount); // Calculate linear interpolation between two vectors
Vector3 VectorReflect(Vector3 vector, Vector3 normal);  // Calculate reflected vector to normal
void VectorTransform(Vector3 *v, Matrix mat);           // Transforms a Vector3 with a given Matrix
Vector3 VectorZero(void);                               // Return a Vector3 init to zero

//------------------------------------------------------------------------------------
// Functions Declaration to work with Matrix
//------------------------------------------------------------------------------------
float *GetMatrixVector(Matrix mat);                     // Returns an OpenGL-ready vector (glMultMatrixf)
float MatrixDeterminant(Matrix mat);                    // Compute matrix determinant
float MatrixTrace(Matrix mat);                          // Returns the trace of the matrix (sum of the values along the diagonal)
void MatrixTranspose(Matrix *mat);                      // Transposes provided matrix
void MatrixInvert(Matrix *mat);                        // Invert provided matrix
void MatrixNormalize(Matrix *mat);                      // Normalize provided matrix
Matrix MatrixIdentity();                                // Returns identity matrix
Matrix MatrixAdd(Matrix left, Matrix right);            // Add two matrices
Matrix MatrixSubstract(Matrix left, Matrix right);      // Substract two matrices (left - right)
Matrix MatrixTranslate(float x, float y, float z);      // Returns translation matrix
Matrix MatrixRotate(float angleX, float angleY, float angleZ); // Returns rotation matrix
Matrix MatrixRotateAroundAxis(Vector3 axis, float angle);      // Returns rotation matrix for an angle around an specified axis
Matrix MatrixRotateAroundAxis2(Vector3 axis, float angle);     // Returns rotation matrix for an angle around an specified axis (test another implemntation)
Matrix MatrixFromQuaternion(Quaternion q);              // Returns rotation matrix for a given quaternion
Matrix MatrixRotateX(float angle);                      // Returns x-rotation matrix (angle in radians)
Matrix MatrixRotateY(float angle);                      // Returns y-rotation matrix (angle in radians)
Matrix MatrixRotateZ(float angle);                      // Returns z-rotation matrix (angle in radians)
Matrix MatrixScale(float x, float y, float z);          // Returns scaling matrix
Matrix MatrixTransform(Vector3 translation, Vector3 rotation, Vector3 scale);   // Returns transformation matrix for a given translation, rotation and scale
Matrix MatrixMultiply(Matrix left, Matrix right);       // Returns two matrix multiplication
Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far);  // Returns perspective projection matrix
Matrix MatrixPerspective(double fovy, double aspect, double near, double far);                        // Returns perspective projection matrix
Matrix MatrixOrtho(double left, double right, double bottom, double top, double near, double far);    // Returns orthographic projection matrix
Matrix MatrixLookAt(Vector3 position, Vector3 target, Vector3 up);  // Returns camera look-at matrix (view matrix)
void PrintMatrix(Matrix m);                             // Print matrix utility

//------------------------------------------------------------------------------------
// Functions Declaration to work with Quaternions
//------------------------------------------------------------------------------------
float QuaternionLength(Quaternion quat);                // Calculates the length of a quaternion
void QuaternionNormalize(Quaternion *q);                // Normalize provided quaternion
Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2);    // Calculate two quaternion multiplication
Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float slerp); // Calculates spherical linear interpolation between two quaternions
Quaternion QuaternionFromMatrix(Matrix matrix);                 // Returns a quaternion from a given rotation matrix
Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle);  // Returns rotation quaternion for an angle around an axis
Matrix QuaternionToMatrix(Quaternion q);                        // Calculates the matrix from the given quaternion
void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle); // Returns the axis and the angle for a given quaternion

#ifdef __cplusplus
}
#endif

#endif // RAYMATH_H