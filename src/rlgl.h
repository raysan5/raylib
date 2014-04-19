/*********************************************************************************************
* 
*   rlgl - raylib OpenGL abstraction layer
*    
*   raylib now uses OpenGL 1.1 style functions (rlVertex) that are mapped to selected OpenGL version:
*       OpenGL 1.1  - Direct map rl* -> gl*
*       OpenGL 3.3+ - Vertex data is stored in VAOs, call rlglDraw() to render
*       OpenGL ES 2 - Same behaviour as OpenGL 3.3+ (NOT TESTED)
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

#ifndef RLGL_H
#define RLGL_H

//#define RLGL_STANDALONE       // NOTE: To use rlgl as standalone lib, just uncomment this line

#ifndef RLGL_STANDALONE
    #include "raylib.h"         // Required for typedef: Model
    #include "utils.h"          // Required for function TraceLog()
#endif

#include "raymath.h"            // Required for data type Matrix and Matrix functions

// Select desired OpenGL version
#define USE_OPENGL_11
//#define USE_OPENGL_33
//#define USE_OPENGL_ES2

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_LINES_BATCH         8192    // NOTE: Be careful with limits!
#define MAX_TRIANGLES_BATCH     4096    // NOTE: Be careful with limits!
#define MAX_QUADS_BATCH         8192    // NOTE: Be careful with limits!

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef unsigned char byte;

typedef enum { RL_PROJECTION, RL_MODELVIEW, RL_TEXTURE } MatrixMode;

typedef enum { RL_LINES, RL_TRIANGLES, RL_QUADS } DrawMode;

#ifdef RLGL_STANDALONE
    typedef struct {
        int vertexCount;
        float *vertices;            // 3 components per vertex
        float *texcoords;           // 2 components per vertex
        float *normals;             // 3 components per vertex
        float *colors;
    } VertexData;

    typedef struct Model {
        VertexData mesh;
        unsigned int vaoId;
        unsigned int textureId;
        //Matrix transform;
    } Model;
#endif

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
void rlVertex2i(int x, int y);                  // Define one vertex (position) - 2 int
void rlVertex2f(float x, float y);              // Define one vertex (position) - 2 float
void rlVertex3f(float x, float y, float z);     // Define one vertex (position) - 3 float
void rlTexCoord2f(float x, float y);            // Define one vertex (texture coordinate) - 2 float
void rlNormal3f(float x, float y, float z);     // Define one vertex (normal) - 3 float
void rlColor4ub(byte r, byte g, byte b, byte a);    // Define one vertex (color) - 4 byte
void rlColor3f(float x, float y, float z);          // Define one vertex (color) - 3 float
void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL equivalent functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer
//------------------------------------------------------------------------------------
void rlEnableTexture(unsigned int id);      // Enable texture usage
void rlDisableTexture();                    // Disable texture usage
void rlDeleteTextures(unsigned int id);     // Delete OpenGL texture from GPU
void rlDeleteVertexArrays(unsigned int id); // Unload vertex data from GPU memory
void rlClearColor(byte r, byte g, byte b, byte a);  // Clear color buffer with color
void rlClearScreenBuffers();                // Clear used screen buffers (color and depth)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
void rlglInit();                                // Initialize rlgl (shaders, VAO, VBO...)
void rlglClose();                               // De-init rlgl
void rlglDraw();                                // Draw VAOs
unsigned int rlglLoadModel(VertexData mesh);
unsigned int rlglLoadCompressedTexture(unsigned char *data, int width, int height, int mipmapCount, int format);
#endif

void rlglDrawModel(Model model, Vector3 position, Vector3 rotation, Vector3 scale, Color color, bool wires);

void rlglInitGraphicsDevice(int fbWidth, int fbHeight);  // Initialize Graphics Device (OpenGL stuff)
unsigned int rlglLoadTexture(unsigned char *data, int width, int height, bool genMipmaps); // Load in GPU OpenGL texture
byte *rlglReadScreenPixels(int width, int height);    // Read screen pixel data (color buffer)

#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
void PrintProjectionMatrix();       // DEBUG: Print projection matrix
void PrintModelviewMatrix();        // DEBUG: Print modelview matrix
#endif

#ifdef __cplusplus
}
#endif

#endif // RLGL_H