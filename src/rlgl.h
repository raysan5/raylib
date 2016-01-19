/**********************************************************************************************
*
*   rlgl - raylib OpenGL abstraction layer
*
*   raylib now uses OpenGL 1.1 style functions (rlVertex) that are mapped to selected OpenGL version:
*       OpenGL 1.1  - Direct map rl* -> gl*
*       OpenGL 3.3  - Vertex data is stored in VAOs, call rlglDraw() to render
*       OpenGL ES 2 - Vertex data is stored in VBOs or VAOs (when available), call rlglDraw() to render
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
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
    #include "raylib.h"         // Required for typedef(s): Model, Shader, Texture2D
    #include "utils.h"          // Required for function TraceLog()
#endif

#if defined(RLGL_STANDALONE)
    #define RAYMATH_STANDALONE
#endif

#include "raymath.h"            // Required for data type Matrix and Matrix functions

// Select desired OpenGL version
// NOTE: Those preprocessor defines are only used on rlgl module,
// if OpenGL version is required by any other module, it uses rlGetVersion()

// Choose opengl version here or just define it at compile time: -DGRAPHICS_API_OPENGL_33
//#define GRAPHICS_API_OPENGL_11     // Only available on PLATFORM_DESKTOP
//#define GRAPHICS_API_OPENGL_33     // Only available on PLATFORM_DESKTOP
//#define GRAPHICS_API_OPENGL_ES2    // Only available on PLATFORM_ANDROID or PLATFORM_RPI or PLATFORM_WEB

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && !defined(GRAPHICS_API_OPENGL_33) && !defined(GRAPHICS_API_OPENGL_ES2)
    #define GRAPHICS_API_OPENGL_11
#endif

// Security check in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(GRAPHICS_API_OPENGL_33)
        #undef GRAPHICS_API_OPENGL_33
    #endif

    #if defined(GRAPHICS_API_OPENGL_ES2)
        #undef GRAPHICS_API_OPENGL_ES2
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: This is the maximum amount of lines, triangles and quads per frame, be careful!
    #define MAX_LINES_BATCH         8192
    #define MAX_TRIANGLES_BATCH     4096
    #define MAX_QUADS_BATCH         4096
#elif defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: Reduce memory sizes for embedded systems (RPI and HTML5)
    // NOTE: On HTML5 (emscripten) this is allocated on heap, by default it's only 16MB!...just take care...
    #define MAX_LINES_BATCH         1024    // Critical for wire shapes (sphere)
    #define MAX_TRIANGLES_BATCH     2048    // Critical for some shapes (sphere)
    #define MAX_QUADS_BATCH         1024    // Be careful with text, every letter maps a quad
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { RL_PROJECTION, RL_MODELVIEW, RL_TEXTURE } MatrixMode;

typedef enum { RL_LINES, RL_TRIANGLES, RL_QUADS } DrawMode;

typedef enum { OPENGL_11 = 1, OPENGL_33, OPENGL_ES_20 } GlVersion;

#ifdef RLGL_STANDALONE
    #ifndef __cplusplus
    // Boolean type
    typedef enum { false, true } bool;
    #endif

    // byte type
    typedef unsigned char byte;
    
    // Color type, RGBA (32bit)
    typedef struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } Color;

    // Texture formats (support depends on OpenGL version)
    typedef enum { 
        UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
        UNCOMPRESSED_GRAY_ALPHA,
        UNCOMPRESSED_R5G6B5,            // 16 bpp
        UNCOMPRESSED_R8G8B8,            // 24 bpp
        UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
        UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
        UNCOMPRESSED_R8G8B8A8,          // 32 bpp
        COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
        COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
        COMPRESSED_DXT3_RGBA,           // 8 bpp
        COMPRESSED_DXT5_RGBA,           // 8 bpp
        COMPRESSED_ETC1_RGB,            // 4 bpp
        COMPRESSED_ETC2_RGB,            // 4 bpp
        COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
        COMPRESSED_PVRT_RGB,            // 4 bpp
        COMPRESSED_PVRT_RGBA,           // 4 bpp
        COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
        COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
    } TextureFormat;

    // Mesh with vertex data type
    // NOTE: If using OpenGL 1.1, data loaded in CPU; if OpenGL 3.3+ data loaded in GPU (vaoId)
    typedef struct Mesh {
        int vertexCount;            // num vertices
        float *vertices;            // vertex position (XYZ - 3 components per vertex)
        float *texcoords;           // vertex texture coordinates (UV - 2 components per vertex)
        float *texcoords2;          // vertex second texture coordinates (useful for lightmaps)
        float *normals;             // vertex normals (XYZ - 3 components per vertex)
        float *tangents;            // vertex tangents (XYZ - 3 components per vertex)
        unsigned char *colors;      // vertex colors (RGBA - 4 components per vertex)
        
        BoundingBox bounds;         // mesh limits defined by min and max points
        
        unsigned int vaoId;         // OpenGL Vertex Array Object id
        unsigned int vboId[6];      // OpenGL Vertex Buffer Objects id (6 types of vertex data)
    } Mesh;

    // Shader type
    typedef struct Shader {
        unsigned int id;                // Shader program id

        // TODO: This should be Texture2D objects
        unsigned int texDiffuseId;      // Diffuse texture id
        unsigned int texNormalId;       // Normal texture id
        unsigned int texSpecularId;     // Specular texture id
        
        // Variable attributes
        int vertexLoc;        // Vertex attribute location point (vertex shader)
        int texcoordLoc;      // Texcoord attribute location point (vertex shader)
        int normalLoc;        // Normal attribute location point (vertex shader)
        int colorLoc;         // Color attibute location point (vertex shader)

        // Uniforms
        int mvpLoc;           // ModelView-Projection matrix uniform location point (vertex shader)
    
        int modelLoc;         // Model transformation matrix uniform location point (vertex shader)
        int viewLoc;          // View transformation matrix uniform location point (vertex shader)
        int tintColorLoc;     // Color uniform location point (fragment shader)
        
        int mapDiffuseLoc;    // Diffuse map texture uniform location point (fragment shader)
        int mapNormalLoc;     // Normal map texture uniform location point (fragment shader)
        int mapSpecularLoc;   // Specular map texture uniform location point (fragment shader)
    } Shader;

    // Texture2D type
    typedef struct Texture2D {
        unsigned int id;            // Texture id
        int width;
        int height;
    } Texture2D;
    
    // 3d Model type
    typedef struct Model {
        Mesh mesh;
        Matrix transform;
        Texture2D texture;
        Shader shader;
    } Model;
	
    // Color blending modes (pre-defined)
    typedef enum { BLEND_ALPHA = 0, BLEND_ADDITIVE, BLEND_MULTIPLIED } BlendMode;
#endif

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------
void rlMatrixMode(int mode);                    // Choose the current matrix to be transformed
void rlPushMatrix(void);                        // Push the current matrix to stack
void rlPopMatrix(void);                         // Pop lattest inserted matrix from stack
void rlLoadIdentity(void);                      // Reset current matrix to identity matrix
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
void rlEnd(void);                               // Finish vertex providing
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
void rlEnableTexture(unsigned int id);          // Enable texture usage
void rlDisableTexture(void);                    // Disable texture usage
void rlDeleteTextures(unsigned int id);         // Delete OpenGL texture from GPU
void rlDeleteShader(unsigned int id);           // Delete OpenGL shader program from GPU
void rlDeleteVertexArrays(unsigned int id);     // Unload vertex data (VAO) from GPU memory
void rlDeleteBuffers(unsigned int id);          // Unload vertex data (VBO) from GPU memory
void rlClearColor(byte r, byte g, byte b, byte a);  // Clear color buffer with color
void rlClearScreenBuffers(void);                // Clear used screen buffers (color and depth)
int rlGetVersion(void);                         // Returns current OpenGL version
void rlEnablePostproFBO(void);                  // Enable rendering to postprocessing FBO

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
void rlglInit(void);                            // Initialize rlgl (shaders, VAO, VBO...)
void rlglClose(void);                           // De-init rlgl
void rlglDraw(void);                            // Draw VAO/VBO
void rlglInitGraphics(int offsetX, int offsetY, int width, int height);  // Initialize Graphics (OpenGL stuff)

unsigned int rlglLoadTexture(void *data, int width, int height, int textureFormat, int mipmapCount);    // Load texture in GPU
void rlglUpdateTexture(unsigned int id, int width, int height, int format, void *data);         // Update GPU texture with new data
void rlglGenerateMipmaps(Texture2D texture);                             // Generate mipmap data for selected texture

// NOTE: There is a set of shader related functions that are available to end user,
// to avoid creating function wrappers through core module, they have been directly declared in raylib.h

void rlglInitPostpro(void);                     // Initialize postprocessing system
void rlglDrawPostpro(void);                     // Draw with postprocessing shader

Model rlglLoadModel(Mesh mesh);           // Upload vertex data into GPU and provided VAO/VBO ids
void rlglDrawModel(Model model, Vector3 position, float rotationAngle, Vector3 rotationAxis, Vector3 scale, Color color, bool wires);

Vector3 rlglUnproject(Vector3 source, Matrix proj, Matrix view);    // Get world coordinates from screen coordinates

unsigned char *rlglReadScreenPixels(int width, int height);         // Read screen pixel data (color buffer)
void *rlglReadTexturePixels(Texture2D texture);                     // Read texture pixel data

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
void PrintProjectionMatrix(void);       // DEBUG: Print projection matrix
void PrintModelviewMatrix(void);        // DEBUG: Print modelview matrix
#endif

#if defined(RLGL_STANDALONE)
//------------------------------------------------------------------------------------
// Shaders System Functions (Module: rlgl)
// NOTE: This functions are useless when using OpenGL 1.1
//------------------------------------------------------------------------------------
Shader LoadShader(char *vsFileName, char *fsFileName);              // Load a custom shader and bind default locations
unsigned int LoadShaderProgram(char *vShaderStr, char *fShaderStr); // Load custom shader strings and return program id
void UnloadShader(Shader shader);                                   // Unload a custom shader from memory
void SetPostproShader(Shader shader);                               // Set fullscreen postproduction shader
void SetCustomShader(Shader shader);                                // Set custom shader to be used in batch draw
void SetDefaultShader(void);                                        // Set default shader to be used in batch draw
void SetModelShader(Model *model, Shader shader);                   // Link a shader to a model
bool IsPosproShaderEnabled(void);                                   // Check if postprocessing shader is enabled

int GetShaderLocation(Shader shader, const char *uniformName);                          // Get shader uniform location
void SetShaderValue(Shader shader, int uniformLoc, float *value, int size);             // Set shader uniform value (float)
void SetShaderValuei(Shader shader, int uniformLoc, int *value, int size);              // Set shader uniform value (int)
void SetShaderMapDiffuse(Shader *shader, Texture2D texture);                            // Default diffuse shader map texture assignment
void SetShaderMapNormal(Shader *shader, const char *uniformName, Texture2D texture);    // Normal map texture shader assignment
void SetShaderMapSpecular(Shader *shader, const char *uniformName, Texture2D texture);  // Specular map texture shader assignment
void SetShaderMap(Shader *shader, int mapLocation, Texture2D texture, int textureUnit); // TODO: Generic shader map assignment

void SetBlendMode(int mode);                                        // Set blending mode (alpha, additive, multiplied)
#endif

#ifdef __cplusplus
}
#endif

#endif // RLGL_H