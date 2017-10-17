/**********************************************************************************************
*
*   rlgl - raylib OpenGL abstraction layer
*
*   rlgl is a wrapper for multiple OpenGL versions (1.1, 2.1, 3.3 Core, ES 2.0) to 
*   pseudo-OpenGL 1.1 style functions (rlVertex, rlTranslate, rlRotate...). 
*
*   When chosing an OpenGL version greater than OpenGL 1.1, rlgl stores vertex data on internal 
*   VBO buffers (and VAOs if available). It requires calling 3 functions:
*       rlglInit()  - Initialize internal buffers and auxiliar resources
*       rlglDraw()  - Process internal buffers and send required draw calls
*       rlglClose() - De-initialize internal buffers data and other auxiliar resources
*
*   CONFIGURATION:
*
*   #define GRAPHICS_API_OPENGL_11
*   #define GRAPHICS_API_OPENGL_21
*   #define GRAPHICS_API_OPENGL_33
*   #define GRAPHICS_API_OPENGL_ES2
*       Use selected OpenGL graphics backend, should be supported by platform
*       Those preprocessor defines are only used on rlgl module, if OpenGL version is 
*       required by any other module, use rlGetVersion() tocheck it
*
*   #define RLGL_STANDALONE
*       Use rlgl as standalone library (no raylib dependency)
*
*   #define SUPPORT_VR_SIMULATION / SUPPORT_STEREO_RENDERING
*       Support VR simulation functionality (stereo rendering)
*
*   #define SUPPORT_DISTORTION_SHADER
*       Include stereo rendering distortion shader (shader_distortion.h)
*
*   DEPENDENCIES:
*       raymath     - 3D math functionality (Vector3, Matrix, Quaternion)
*       GLAD        - OpenGL extensions loading (OpenGL 3.3 Core only)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2017 Ramon Santamaria (@raysan5)
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

#if defined(RLGL_STANDALONE)
    #define RAYMATH_STANDALONE
#else
    #include "raylib.h"         // Required for: Model, Shader, Texture2D, TraceLog()
#endif

#include "raymath.h"            // Required for: Vector3, Matrix

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
    #define MAX_QUADS_BATCH         8192
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

// Matrix modes (equivalent to OpenGL)
#define RL_MODELVIEW                    0x1700      // GL_MODELVIEW 
#define RL_PROJECTION                   0x1701      // GL_PROJECTION
#define RL_TEXTURE                      0x1702      // GL_TEXTURE

// Primitive assembly draw modes
#define RL_LINES                        0x0001      // GL_LINES
#define RL_TRIANGLES                    0x0004      // GL_TRIANGLES
#define RL_QUADS                        0x0007      // GL_QUADS

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { OPENGL_11 = 1, OPENGL_21, OPENGL_33, OPENGL_ES_20 } GlVersion;

typedef unsigned char byte;

#if defined(RLGL_STANDALONE)
    #ifndef __cplusplus
    // Boolean type
    typedef enum { false, true } bool;
    #endif
    
    // Shader location point type
    typedef enum {
        LOC_VERTEX_POSITION = 0,
        LOC_VERTEX_TEXCOORD01,
        LOC_VERTEX_TEXCOORD02,
        LOC_VERTEX_NORMAL,
        LOC_VERTEX_TANGENT,
        LOC_VERTEX_COLOR,
        LOC_MATRIX_MVP,
        LOC_MATRIX_MODEL,
        LOC_MATRIX_VIEW,
        LOC_MATRIX_PROJECTION,
        LOC_VECTOR_VIEW,
        LOC_COLOR_DIFFUSE,
        LOC_COLOR_SPECULAR,
        LOC_COLOR_AMBIENT,
        LOC_MAP_ALBEDO,          // LOC_MAP_DIFFUSE
        LOC_MAP_METALNESS,       // LOC_MAP_SPECULAR
        LOC_MAP_NORMAL,
        LOC_MAP_ROUGHNESS,
        LOC_MAP_OCCUSION,
        LOC_MAP_EMISSION,
        LOC_MAP_HEIGHT,
        LOC_MAP_CUBEMAP,
        LOC_MAP_IRRADIANCE,
        LOC_MAP_PREFILTER,
        LOC_MAP_BRDF
    } ShaderLocationIndex;

    #define LOC_MAP_DIFFUSE      LOC_MAP_ALBEDO
    #define LOC_MAP_SPECULAR     LOC_MAP_METALNESS

    // Material map type
    typedef enum {
        MAP_ALBEDO    = 0,       // MAP_DIFFUSE
        MAP_METALNESS = 1,       // MAP_SPECULAR
        MAP_NORMAL    = 2,
        MAP_ROUGHNESS = 3,
        MAP_OCCLUSION,
        MAP_EMISSION,
        MAP_HEIGHT,
        MAP_CUBEMAP,             // NOTE: Uses GL_TEXTURE_CUBE_MAP
        MAP_IRRADIANCE,          // NOTE: Uses GL_TEXTURE_CUBE_MAP
        MAP_PREFILTER,           // NOTE: Uses GL_TEXTURE_CUBE_MAP
        MAP_BRDF
    } TexmapIndex;

    #define MAP_DIFFUSE      MAP_ALBEDO
    #define MAP_SPECULAR     MAP_METALNESS

    // Color type, RGBA (32bit)
    typedef struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } Color;
    
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
    
    // Shader and material limits
    #define MAX_SHADER_LOCATIONS    32
    #define MAX_MATERIAL_MAPS       12
    
    // Shader type (generic)
    typedef struct Shader {
        unsigned int id;                // Shader program id
        int locs[MAX_SHADER_LOCATIONS]; // Shader locations array
    } Shader;

    // Material texture map
    typedef struct MaterialMap {
        Texture2D texture;      // Material map texture
        Color color;            // Material map color
        float value;            // Material map value
    } MaterialMap;

    // Material type (generic)
    typedef struct Material {
        Shader shader;          // Material shader
        MaterialMap maps[MAX_MATERIAL_MAPS]; // Material maps
        float *params;          // Material generic parameters (if required)
    } Material;

    // Camera type, defines a camera position/orientation in 3d space
    typedef struct Camera {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view apperture in Y (degrees)
    } Camera;
    
    // Head-Mounted-Display device parameters
    typedef struct VrDeviceInfo {
        int hResolution;                // HMD horizontal resolution in pixels
        int vResolution;                // HMD vertical resolution in pixels
        float hScreenSize;              // HMD horizontal size in meters
        float vScreenSize;              // HMD vertical size in meters
        float vScreenCenter;            // HMD screen center in meters
        float eyeToScreenDistance;      // HMD distance between eye and display in meters
        float lensSeparationDistance;   // HMD lens separation distance in meters
        float interpupillaryDistance;   // HMD IPD (distance between pupils) in meters
        float lensDistortionValues[4];  // HMD lens distortion constant parameters
        float chromaAbCorrection[4];    // HMD chromatic aberration correction parameters
    } VrDeviceInfo;
    
    // TraceLog message types
    typedef enum { 
        LOG_INFO = 0, 
        LOG_ERROR, 
        LOG_WARNING, 
        LOG_DEBUG, 
        LOG_OTHER 
    } TraceLogType;
    
    // Texture formats (support depends on OpenGL version)
    typedef enum {
        UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
        UNCOMPRESSED_GRAY_ALPHA,
        UNCOMPRESSED_R5G6B5,            // 16 bpp
        UNCOMPRESSED_R8G8B8,            // 24 bpp
        UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
        UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
        UNCOMPRESSED_R8G8B8A8,          // 32 bpp
        UNCOMPRESSED_R32G32B32,         // 32 bit per channel (float) - HDR
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
    typedef enum { 
        WRAP_REPEAT = 0, 
        WRAP_CLAMP, 
        WRAP_MIRROR 
    } TextureWrapMode;

    // Color blending modes (pre-defined)
    typedef enum { 
        BLEND_ALPHA = 0, 
        BLEND_ADDITIVE, 
        BLEND_MULTIPLIED 
    } BlendMode;

    // VR Head Mounted Display devices
    typedef enum {
        HMD_DEFAULT_DEVICE = 0,
        HMD_OCULUS_RIFT_DK2,
        HMD_OCULUS_RIFT_CV1,
        HMD_OCULUS_GO,
        HMD_VALVE_HTC_VIVE,
        HMD_SONY_PSVR
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
void rlMultMatrixf(float *matf);                // Multiply the current matrix by another matrix
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
void rlEnableTexture(unsigned int id);                  // Enable texture usage
void rlDisableTexture(void);                            // Disable texture usage
void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)
void rlEnableRenderTexture(unsigned int id);            // Enable render texture (fbo)
void rlDisableRenderTexture(void);                      // Disable render texture (fbo), return to default framebuffer
void rlEnableDepthTest(void);                           // Enable depth test
void rlDisableDepthTest(void);                          // Disable depth test
void rlEnableWireMode(void);                            // Enable wire mode
void rlDisableWireMode(void);                           // Disable wire mode
void rlDeleteTextures(unsigned int id);                 // Delete OpenGL texture from GPU
void rlDeleteRenderTextures(RenderTexture2D target);    // Delete render textures (fbo) from GPU
void rlDeleteShader(unsigned int id);                   // Delete OpenGL shader program from GPU
void rlDeleteVertexArrays(unsigned int id);             // Unload vertex data (VAO) from GPU memory
void rlDeleteBuffers(unsigned int id);                  // Unload vertex data (VBO) from GPU memory
void rlClearColor(byte r, byte g, byte b, byte a);      // Clear color buffer with color
void rlClearScreenBuffers(void);                        // Clear used screen buffers (color and depth)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
void rlglInit(int width, int height);           // Initialize rlgl (buffers, shaders, textures, states)
void rlglClose(void);                           // De-inititialize rlgl (buffers, shaders, textures)
void rlglDraw(void);                            // Update and Draw default buffers (lines, triangles, quads)

int rlGetVersion(void);                         // Returns current OpenGL version
void rlLoadExtensions(void *loader);            // Load OpenGL extensions
Vector3 rlUnproject(Vector3 source, Matrix proj, Matrix view);  // Get world coordinates from screen coordinates

// Textures data management
unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount);    // Load texture in GPU
void rlUpdateTexture(unsigned int id, int width, int height, int format, const void *data);    // Update GPU texture with new data
void rlUnloadTexture(unsigned int id);
void rlGenerateMipmaps(Texture2D *texture);                         // Generate mipmap data for selected texture
void *rlReadTexturePixels(Texture2D texture);                       // Read texture pixel data
unsigned char *rlReadScreenPixels(int width, int height);           // Read screen pixel data (color buffer)
RenderTexture2D rlLoadRenderTexture(int width, int height);         // Load a texture to be used for rendering (fbo with color and depth attachments)

// Vertex data management
void rlLoadMesh(Mesh *mesh, bool dynamic);                          // Upload vertex data into GPU and provided VAO/VBO ids
void rlUpdateMesh(Mesh mesh, int buffer, int numVertex);            // Update vertex data on GPU (upload new data to one buffer)
void rlDrawMesh(Mesh mesh, Material material, Matrix transform);    // Draw a 3d mesh with material and transform
void rlUnloadMesh(Mesh *mesh);                                      // Unload mesh data from CPU and GPU

// NOTE: There is a set of shader related functions that are available to end user,
// to avoid creating function wrappers through core module, they have been directly declared in raylib.h

#if defined(RLGL_STANDALONE)
//------------------------------------------------------------------------------------
// Shaders System Functions (Module: rlgl)
// NOTE: This functions are useless when using OpenGL 1.1
//------------------------------------------------------------------------------------
Shader LoadShader(char *vsFileName, char *fsFileName);  // Load a custom shader and bind default locations
void UnloadShader(Shader shader);                       // Unload a custom shader from memory

Shader GetShaderDefault(void);                          // Get default shader
Texture2D GetTextureDefault(void);                      // Get default texture

// Shader configuration functions
int GetShaderLocation(Shader shader, const char *uniformName);              // Get shader uniform location
void SetShaderValue(Shader shader, int uniformLoc, float *value, int size); // Set shader uniform value (float)
void SetShaderValuei(Shader shader, int uniformLoc, int *value, int size);  // Set shader uniform value (int)
void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat);       // Set shader uniform value (matrix 4x4)
void SetMatrixProjection(Matrix proj);                              // Set a custom projection matrix (replaces internal projection matrix)
void SetMatrixModelview(Matrix view);                               // Set a custom modelview matrix (replaces internal modelview matrix)

// Texture maps generation (PBR)
// NOTE: Required shaders should be provided
Texture2D GenTextureCubemap(Shader shader, Texture2D skyHDR, int size);     // Generate cubemap texture from HDR texture
Texture2D GenTextureIrradiance(Shader shader, Texture2D cubemap, int size); // Generate irradiance texture using cubemap data
Texture2D GenTexturePrefilter(Shader shader, Texture2D cubemap, int size);  // Generate prefilter texture using cubemap data
Texture2D GenTextureBRDF(Shader shader, Texture2D cubemap, int size);       // Generate BRDF texture using cubemap data

// Shading and blending
void BeginShaderMode(Shader shader);                    // Begin custom shader drawing
void EndShaderMode(void);                               // End custom shader drawing (use default shader)
void BeginBlendMode(int mode);                          // Begin blending mode (alpha, additive, multiplied)
void EndBlendMode(void);                                // End blending mode (reset to default: alpha blending)

// VR simulator functionality
VrDeviceInfo GetVrDeviceInfo(int vrDeviceType);         // Get VR device information for some standard devices
void InitVrSimulator(VrDeviceInfo info);                // Init VR simulator for selected device parameters
void CloseVrSimulator(void);                            // Close VR simulator for current device
void UpdateVrTracking(Camera *camera);                  // Update VR tracking (position and orientation) and camera
void ToggleVrMode(void);                                // Enable/Disable VR experience (device or simulator)
void BeginVrDrawing(void);                              // Begin VR stereo rendering
void EndVrDrawing(void);                                // End VR stereo rendering

void TraceLog(int msgType, const char *text, ...);      // Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
#endif

#ifdef __cplusplus
}
#endif

#endif // RLGL_H