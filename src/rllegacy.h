/*********************************************************************************************
* 
*   rllegacy - raylib OpenGL 1.1 functions replacement
*    
*   OpenGL 1.1 functions mapping to OpenGL 3.2+ (and OpenGL ES 2.0)
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

#ifndef RLLEGACY_H
#define RLLEGACY_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_LINES_BATCH         1024
#define MAX_TRIANGLES_BATCH     2048
#define MAX_QUADS_BATCH         2048

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef unsigned char byte;

typedef enum { RL_PROJECTION, RL_MODELVIEW, RL_TEXTURE } MatrixMode;

typedef enum { RL_POINTS, RL_LINES, RL_LINE_STRIP, RL_LINE_LOOP, RL_TRIANGLES, 
               RL_TRIANGLE_STRIP, RL_TRIANGLE_FAN, RL_QUADS, RL_QUAD_STRIP, RL_POLYGON } PrimitiveType;


#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------
void rlMatrixMode(int mode);                    // Choose the current matrix to be transformed
void rlPushMatrix();                            // Push the current matrix to stack
void rlPopMatrix();                             // Pop lattest inserted matrix from stack
void rlLoadIdentity();                          // Reset current matrix to identity matrix
void rlTranslatef(float x, float y, float z);   // Multiply the current matrix by a translation matrix
void rlRotatef(float angleDeg, float x, float y, float z);  // Multiply the current matrix by a rotation matrix
void rlScalef(float x, float y, float z);       // Multiply the current matrix by a scaling matrix
void rlMultMatrixf(float *mat);                 // Multiply the current matrix by another matrix
void rlFrustum(double left, double right, double bottom, double top, double near, double far);
void rlOrtho(double left, double right, double bottom, double top, double near, double far);

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
void rlBegin(int mode);                         // Initialize drawing mode (how to organize vertex)
void rlEnd();                                   // Finish vertex providing
void rlVertex3f(float x, float y, float z);     // Define one vertex (position) - 3 float
void rlVertex2f(float x, float y);              // Define one vertex (position) - 2 float
void rlVertex2i(int x, int y);                  // Define one vertex (position) - 2 int
void rlTexCoord2f(float x, float y);            // Define one vertex (texture coordinate) - 2 float
void rlNormal3f(float x, float y, float z);     // Define one vertex (normal) - 3 float
void rlColor4ub(byte r, byte g, byte b, byte a);    // Define one vertex (color) - 4 byte
void rlColor3f(float x, float y, float z);          // Define one vertex (color) - 3 float
void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

void InitRlLegacy();
void CloseRlLegacy();
void DrawRlLegacy();

#ifdef __cplusplus
}
#endif

#endif // RLLEGACY_H