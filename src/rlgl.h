/**********************************************************************************************
*
*   rlgl - raylib OpenGL abstraction layer
*
*   rlgl allows usage of OpenGL 1.1 style functions (rlVertex) that are internally mapped to 
*   selected OpenGL version (1.1, 2.1, 3.3 Core, ES 2.0). 
*
*   When chosing an OpenGL version greater than OpenGL 1.1, rlgl stores vertex data on internal 
*   VBO buffers (and VAOs if available). It requires calling 3 functions:
*       rlglInit()  - Initialize internal buffers and auxiliar resources
*       rlglDraw()  - Process internal buffers and send required draw calls
*       rlglClose() - De-initialize internal buffers data and other auxiliar resources
*
*   External libs:
*       raymath     - 3D math functionality (Vector3, Matrix, Quaternion)
*       GLAD        - OpenGL extensions loading (OpenGL 3.3 Core only)
*
*   Module Configuration Flags:
*       GRAPHICS_API_OPENGL_11  - Use OpenGL 1.1 backend
*       GRAPHICS_API_OPENGL_21  - Use OpenGL 2.1 backend
*       GRAPHICS_API_OPENGL_33  - Use OpenGL 3.3 Core profile backend
*       GRAPHICS_API_OPENGL_ES2 - Use OpenGL ES 2.0 backend
*
*       RLGL_STANDALONE             - Use rlgl as standalone library (no raylib dependency)
*
*
*   Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
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
    #include "raylib.h"         // Required for: Model, Shader, Texture2D
    #include "utils.h"          // Required for: TraceLog()
#endif

#ifdef RLGL_STANDALONE
    #define RAYMATH_STANDALONE
#endif

#include "raymath.h"            // Required for: Vector3, Matrix

// Select desired OpenGL version
// NOTE: Those preprocessor defines are only used on rlgl module,
// if OpenGL version is required by any other module, it uses rlGetVersion()

// Choose opengl version here or just define it at compile time: -DGRAPHICS_API_OPENGL_33
//#define GRAPHICS_API_OPENGL_11     // Only available on PLATFORM_DESKTOP
//#define GRAPHICS_API_OPENGL_33     // Only available on PLATFORM_DESKTOP and RLGL_OCULUS_SUPPORT
//#define GRAPHICS_API_OPENGL_ES2    // Only available on PLATFORM_ANDROID or PLATFORM_RPI or PLATFORM_WEB

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && !defined(GRAPHICS_API_OPENGL_21) && !defined(GRAPHICS_API_OPENGL_33) && !defined(GRAPHICS_API_OPENGL_ES2)
    #define GRAPHICS_API_OPENGL_11
#endif

// Security check in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(GRAPHICS_API_OPENGL_21)
        #undef GRAPHICS_API_OPENGL_21
    #endif
    #if defined(GRAPHICS_API_OPENGL_33)
        #undef GRAPHICS_API_OPENGL_33
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #undef GRAPHICS_API_OPENGL_ES2
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
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

// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S               0x2802      // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T               0x2803      // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER           0x2800      // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER           0x2801      // GL_TEXTURE_MIN_FILTER
#define RL_TEXTURE_ANISOTROPIC_FILTER   0x3000      // Anisotropic filter (custom identifier)

#define RL_FILTER_NEAREST               0x2600      // GL_NEAREST
#define RL_FILTER_LINEAR                0x2601      // GL_LINEAR
#define RL_FILTER_MIP_NEAREST           0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define RL_FILTER_NEAREST_MIP_LINEAR    0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define RL_FILTER_LINEAR_MIP_NEAREST    0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define RL_FILTER_MIP_LINEAR            0x2703      // GL_LINEAR_MIPMAP_LINEAR

#define RL_WRAP_REPEAT                  0x2901      // GL_REPEAT
#define RL_WRAP_CLAMP                   0x812F      // GL_CLAMP_TO_EDGE
#define RL_WRAP_CLAMP_MIRROR            0x8742      // GL_MIRROR_CLAMP_EXT

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { OPENGL_11 = 1, OPENGL_21, OPENGL_33, OPENGL_ES_20 } GlVersion;

typedef enum { RL_PROJECTION, RL_MODELVIEW, RL_TEXTURE } MatrixMode;

typedef enum { RL_LINES, RL_TRIANGLES, RL_QUADS } DrawMode;

typedef unsigned char byte;

#if defined(RLGL_STANDALONE)
    #ifndef __cplusplus
    // Boolean type
    typedef enum { false, true } bool;
    #endif

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

    // Vertex data definning a mesh
    typedef struct Mesh {
        int vertexCount;        // number of vertices stored in arrays
        int triangleCount;      // number of triangles stored (indexed or not)
        float *vertices;        // vertex position (XYZ - 3 components per vertex) (shader-location = 0)
        float *texcoords;       // vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
        float *texcoords2;      // vertex second texture coordinates (useful for lightmaps) (shader-location = 5)
        float *normals;         // vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
        float *tangents;        // vertex tangents (XYZ - 3 components per vertex) (shader-location = 4)
        unsigned char *colors;  // vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
        unsigned short *indices;// vertex indices (in case vertex data comes indexed)

        unsigned int vaoId;     // OpenGL Vertex Array Object id
        unsigned int vboId[7];  // OpenGL Vertex Buffer Objects id (7 types of vertex data)
    } Mesh;

    // Shader type (generic shader)
    typedef struct Shader {
        unsigned int id;        // Shader program id

        // Vertex attributes locations (default locations)
        int vertexLoc;          // Vertex attribute location point (default-location = 0)
        int texcoordLoc;        // Texcoord attribute location point (default-location = 1)
        int normalLoc;          // Normal attribute location point (default-location = 2)
        int colorLoc;           // Color attibute location point (default-location = 3)
        int tangentLoc;         // Tangent attribute location point (default-location = 4)
        int texcoord2Loc;       // Texcoord2 attribute location point (default-location = 5)

        // Uniform locations
        int mvpLoc;             // ModelView-Projection matrix uniform location point (vertex shader)
        int colDiffuseLoc;       // Color uniform location point (fragment shader)
        int colAmbientLoc;      // Ambient color uniform location point (fragment shader)
        int colSpecularLoc;     // Specular color uniform location point (fragment shader)

        // Texture map locations (generic for any kind of map)
        int mapTexture0Loc;     // Map texture uniform location point (default-texture-unit = 0)
        int mapTexture1Loc;     // Map texture uniform location point (default-texture-unit = 1)
        int mapTexture2Loc;     // Map texture uniform location point (default-texture-unit = 2)
    } Shader;

    // Texture2D type
    // NOTE: Data stored in GPU memory
    typedef struct Texture2D {
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (TextureFormat)
    } Texture2D;

    // RenderTexture2D type, for texture rendering
    typedef struct RenderTexture2D {
        unsigned int id;        // Render texture (fbo) id
        Texture2D texture;      // Color buffer attachment texture
        Texture2D depth;        // Depth buffer attachment texture
    } RenderTexture2D;

    // Material type
    typedef struct Material {
        Shader shader;          // Standard shader (supports 3 map types: diffuse, normal, specular)

        Texture2D texDiffuse;   // Diffuse texture
        Texture2D texNormal;    // Normal texture
        Texture2D texSpecular;  // Specular texture

        Color colDiffuse;       // Diffuse color
        Color colAmbient;       // Ambient color
        Color colSpecular;      // Specular color

        float glossiness;       // Glossiness level (Ranges from 0 to 1000)
    } Material;

    // Camera type, defines a camera position/orientation in 3d space
    typedef struct Camera {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view apperture in Y (degrees)
    } Camera;

    // Light type
    typedef struct LightData {
        unsigned int id;        // Light unique id
        bool enabled;           // Light enabled
        int type;               // Light type: LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT

        Vector3 position;       // Light position
        Vector3 target;         // Light target: LIGHT_DIRECTIONAL and LIGHT_SPOT (cone direction target)
        float radius;           // Light attenuation radius light intensity reduced with distance (world distance)

        Color diffuse;          // Light diffuse color
        float intensity;        // Light intensity level

        float coneAngle;        // Light cone max angle: LIGHT_SPOT
    } LightData, *Light;

    // Light types
    typedef enum { LIGHT_POINT, LIGHT_DIRECTIONAL, LIGHT_SPOT } LightType;
    
    // Texture parameters: filter mode
    // NOTE 1: Filtering considers mipmaps if available in the texture
    // NOTE 2: Filter is accordingly set for minification and magnification
    typedef enum { 
        FILTER_POINT = 0,               // No filter, just pixel aproximation
        FILTER_BILINEAR,                // Linear filtering
        FILTER_TRILINEAR,               // Trilinear filtering (linear with mipmaps)
        FILTER_ANISOTROPIC_4X,          // Anisotropic filtering 4x
        FILTER_ANISOTROPIC_8X,          // Anisotropic filtering 8x
        FILTER_ANISOTROPIC_16X,         // Anisotropic filtering 16x
    } TextureFilterMode;
    
    // Texture parameters: wrap mode
    typedef enum { WRAP_REPEAT = 0, WRAP_CLAMP, WRAP_MIRROR } TextureWrapMode;

    // Color blending modes (pre-defined)
    typedef enum { BLEND_ALPHA = 0, BLEND_ADDITIVE, BLEND_MULTIPLIED } BlendMode;

    // TraceLog message types
    typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;

    // VR Head Mounted Display devices
    typedef enum {
        HMD_DEFAULT_DEVICE = 0,
        HMD_OCULUS_RIFT_DK2,
        HMD_OCULUS_RIFT_CV1,
        HMD_VALVE_HTC_VIVE,
        HMD_SAMSUNG_GEAR_VR,
        HMD_GOOGLE_CARDBOARD,
        HMD_SONY_PLAYSTATION_VR,
        HMD_RAZER_OSVR,
        HMD_FOVE_VR,
    } VrDevice;
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
void rlViewport(int x, int y, int width, int height); // Set the viewport area

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
void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)
void rlEnableRenderTexture(unsigned int id);    // Enable render texture (fbo)
void rlDisableRenderTexture(void);              // Disable render texture (fbo), return to default framebuffer
void rlEnableDepthTest(void);                   // Enable depth test
void rlDisableDepthTest(void);                  // Disable depth test
void rlEnableWireMode(void);                    // Enable wire mode
void rlDisableWireMode(void);                   // Disable wire mode
void rlDeleteTextures(unsigned int id);         // Delete OpenGL texture from GPU
void rlDeleteRenderTextures(RenderTexture2D target);    // Delete render textures (fbo) from GPU
void rlDeleteShader(unsigned int id);           // Delete OpenGL shader program from GPU
void rlDeleteVertexArrays(unsigned int id);     // Unload vertex data (VAO) from GPU memory
void rlDeleteBuffers(unsigned int id);          // Unload vertex data (VBO) from GPU memory
void rlClearColor(byte r, byte g, byte b, byte a);  // Clear color buffer with color
void rlClearScreenBuffers(void);                // Clear used screen buffers (color and depth)
int rlGetVersion(void);                         // Returns current OpenGL version

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
void rlglInit(int width, int height);           // Initialize rlgl (buffers, shaders, textures, states)
void rlglClose(void);                           // De-init rlgl
void rlglDraw(void);                            // Draw VAO/VBO
void rlglLoadExtensions(void *loader);          // Load OpenGL extensions

unsigned int rlglLoadTexture(void *data, int width, int height, int textureFormat, int mipmapCount);    // Load texture in GPU
RenderTexture2D rlglLoadRenderTexture(int width, int height);   // Load a texture to be used for rendering (fbo with color and depth attachments)
void rlglUpdateTexture(unsigned int id, int width, int height, int format, void *data);         // Update GPU texture with new data
void rlglGenerateMipmaps(Texture2D *texture);                       // Generate mipmap data for selected texture

void rlglLoadMesh(Mesh *mesh, bool dynamic);                        // Upload vertex data into GPU and provided VAO/VBO ids
void rlglUpdateMesh(Mesh mesh, int buffer, int numVertex);          // Update vertex data on GPU (upload new data to one buffer)
void rlglDrawMesh(Mesh mesh, Material material, Matrix transform);  // Draw a 3d mesh with material and transform
void rlglUnloadMesh(Mesh *mesh);                                    // Unload mesh data from CPU and GPU

Vector3 rlglUnproject(Vector3 source, Matrix proj, Matrix view);    // Get world coordinates from screen coordinates

unsigned char *rlglReadScreenPixels(int width, int height);         // Read screen pixel data (color buffer)
void *rlglReadTexturePixels(Texture2D texture);                     // Read texture pixel data

// VR functions exposed to core module but not to raylib users
void BeginVrDrawing(void);                  // Begin VR drawing configuration
void EndVrDrawing(void);                    // End VR drawing process (and desktop mirror)

// NOTE: There is a set of shader related functions that are available to end user,
// to avoid creating function wrappers through core module, they have been directly declared in raylib.h

#if defined(RLGL_STANDALONE)
//------------------------------------------------------------------------------------
// Shaders System Functions (Module: rlgl)
// NOTE: This functions are useless when using OpenGL 1.1
//------------------------------------------------------------------------------------
Shader LoadShader(char *vsFileName, char *fsFileName);              // Load a custom shader and bind default locations
void UnloadShader(Shader shader);                                   // Unload a custom shader from memory

Shader GetDefaultShader(void);                                      // Get default shader
Shader GetStandardShader(void);                                     // Get default shader
Texture2D GetDefaultTexture(void);                                  // Get default texture

int GetShaderLocation(Shader shader, const char *uniformName);              // Get shader uniform location
void SetShaderValue(Shader shader, int uniformLoc, float *value, int size); // Set shader uniform value (float)
void SetShaderValuei(Shader shader, int uniformLoc, int *value, int size);  // Set shader uniform value (int)
void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat);       // Set shader uniform value (matrix 4x4)

void SetMatrixProjection(Matrix proj);                              // Set a custom projection matrix (replaces internal projection matrix)
void SetMatrixModelview(Matrix view);                               // Set a custom modelview matrix (replaces internal modelview matrix)

void BeginShaderMode(Shader shader);                                // Begin custom shader drawing
void EndShaderMode(void);                                           // End custom shader drawing (use default shader)
void BeginBlendMode(int mode);                                      // Begin blending mode (alpha, additive, multiplied)
void EndBlendMode(void);                                            // End blending mode (reset to default: alpha blending)

Light CreateLight(int type, Vector3 position, Color diffuse);       // Create a new light, initialize it and add to pool
void DestroyLight(Light light);                                     // Destroy a light and take it out of the list

void TraceLog(int msgType, const char *text, ...);
float *MatrixToFloat(Matrix mat);

void InitVrDevice(int vrDevice);            // Init VR device
void CloseVrDevice(void);                   // Close VR device
bool IsVrDeviceReady(void);                 // Detect if VR device is ready
bool IsVrSimulator(void);                   // Detect if VR simulator is running
void UpdateVrTracking(Camera *camera);      // Update VR tracking (position and orientation) and camera
void ToggleVrMode(void);                    // Enable/Disable VR experience (device or simulator)

// Oculus Rift API for direct access the device (no simulator)
bool InitOculusDevice(void);                // Initialize Oculus device (returns true if success)
void CloseOculusDevice(void);               // Close Oculus device
void UpdateOculusTracking(Camera *camera);  // Update Oculus head position-orientation tracking (and camera)
void BeginOculusDrawing(void);              // Setup Oculus buffers for drawing
void EndOculusDrawing(void);                // Finish Oculus drawing and blit framebuffer to mirror
#endif

#ifdef __cplusplus
}
#endif

#endif // RLGL_H