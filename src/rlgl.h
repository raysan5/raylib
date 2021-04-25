/**********************************************************************************************
*
*   rlgl v3.7 - raylib OpenGL abstraction layer
*
*   rlgl is a wrapper for multiple OpenGL versions (1.1, 2.1, 3.3 Core, ES 2.0) to
*   pseudo-OpenGL 1.1 style functions (rlVertex, rlTranslate, rlRotate...).
*
*   When chosing an OpenGL version greater than OpenGL 1.1, rlgl stores vertex data on internal
*   VBO buffers (and VAOs if available). It requires calling 3 functions:
*       rlglInit()  - Initialize internal buffers and auxiliary resources
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
*       required by any other module, use rlGetVersion() to check it
*
*   #define RLGL_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RLGL_STANDALONE
*       Use rlgl as standalone library (no raylib dependency)
*
*   #define SUPPORT_GL_DETAILS_INFO
*       Show OpenGL extensions and capabilities detailed logs on init
*
*   DEPENDENCIES:
*       raymath     - 3D math functionality (Vector3, Matrix, Quaternion)
*       GLAD        - OpenGL extensions loading (OpenGL 3.3 Core only)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2021 Ramon Santamaria (@raysan5)
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
    #define RAYMATH_HEADER_ONLY

    #define RLAPI   // We are building or using rlgl as a static library (or Linux shared library)

    #if defined(_WIN32)
        #if defined(BUILD_LIBTYPE_SHARED)
            #define RLAPI __declspec(dllexport)         // We are building raylib as a Win32 shared library (.dll)
        #elif defined(USE_LIBTYPE_SHARED)
            #define RLAPI __declspec(dllimport)         // We are using raylib as a Win32 shared library (.dll)
        #endif
    #endif

    // Support TRACELOG macros
    #if !defined(TRACELOG)
        #define TRACELOG(level, ...) (void)0
        #define TRACELOGD(...) (void)0
    #endif

    // Allow custom memory allocators
    #ifndef RL_MALLOC
        #define RL_MALLOC(sz)       malloc(sz)
    #endif
    #ifndef RL_CALLOC
        #define RL_CALLOC(n,sz)     calloc(n,sz)
    #endif
    #ifndef RL_REALLOC
        #define RL_REALLOC(n,sz)    realloc(n,sz)
    #endif
    #ifndef RL_FREE
        #define RL_FREE(p)          free(p)
    #endif
#else
    #include "raylib.h"         // Required for: Shader, Texture2D
#endif

#include "raymath.h"            // Required for: Vector3, Matrix

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_ES2)
        #define GRAPHICS_API_OPENGL_33
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

// OpenGL 2.1 uses most of OpenGL 3.3 Core functionality
// WARNING: Specific parts are checked with #if defines
#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
#endif

#define SUPPORT_RENDER_TEXTURES_HINT

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Default internal render batch limits
#ifndef DEFAULT_BATCH_BUFFER_ELEMENTS
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
        // This is the maximum amount of elements (quads) per batch
        // NOTE: Be careful with text, every letter maps to a quad
        #define DEFAULT_BATCH_BUFFER_ELEMENTS   8192
    #endif
    #if defined(GRAPHICS_API_OPENGL_ES2)
        // We reduce memory sizes for embedded systems (RPI and HTML5)
        // NOTE: On HTML5 (emscripten) this is allocated on heap,
        // by default it's only 16MB!...just take care...
        #define DEFAULT_BATCH_BUFFER_ELEMENTS   2048
    #endif
#endif
#ifndef DEFAULT_BATCH_BUFFERS
    #define DEFAULT_BATCH_BUFFERS            1      // Default number of batch buffers (multi-buffering)
#endif
#ifndef DEFAULT_BATCH_DRAWCALLS
    #define DEFAULT_BATCH_DRAWCALLS        256      // Default number of batch draw calls (by state changes: mode, texture)
#endif
#ifndef MAX_BATCH_ACTIVE_TEXTURES
    #define MAX_BATCH_ACTIVE_TEXTURES        4      // Maximum number of additional textures that can be activated on batch drawing (SetShaderValueTexture())
#endif

// Internal Matrix stack
#ifndef MAX_MATRIX_STACK_SIZE
    #define MAX_MATRIX_STACK_SIZE           32      // Maximum size of Matrix stack
#endif

// Vertex buffers id limit
#ifndef MAX_MESH_VERTEX_BUFFERS
    #define MAX_MESH_VERTEX_BUFFERS          7      // Maximum vertex buffers (VBO) per mesh
#endif

// Shader and material limits
#ifndef MAX_SHADER_LOCATIONS
    #define MAX_SHADER_LOCATIONS            32      // Maximum number of shader locations supported
#endif
#ifndef MAX_MATERIAL_MAPS
    #define MAX_MATERIAL_MAPS               12      // Maximum number of shader maps supported
#endif

// Projection matrix culling
#ifndef RL_CULL_DISTANCE_NEAR
    #define RL_CULL_DISTANCE_NEAR         0.01      // Default near cull distance
#endif
#ifndef RL_CULL_DISTANCE_FAR
    #define RL_CULL_DISTANCE_FAR        1000.0      // Default far cull distance
#endif

// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S               0x2802      // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T               0x2803      // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER           0x2800      // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER           0x2801      // GL_TEXTURE_MIN_FILTER

#define RL_TEXTURE_FILTER_NEAREST               0x2600      // GL_NEAREST
#define RL_TEXTURE_FILTER_LINEAR                0x2601      // GL_LINEAR
#define RL_TEXTURE_FILTER_MIP_NEAREST           0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR    0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST    0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_MIP_LINEAR            0x2703      // GL_LINEAR_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_ANISOTROPIC           0x3000      // Anisotropic filter (custom identifier)

#define RL_TEXTURE_WRAP_REPEAT                  0x2901      // GL_REPEAT
#define RL_TEXTURE_WRAP_CLAMP                   0x812F      // GL_CLAMP_TO_EDGE
#define RL_TEXTURE_WRAP_MIRROR_REPEAT           0x8370      // GL_MIRRORED_REPEAT
#define RL_TEXTURE_WRAP_MIRROR_CLAMP            0x8742      // GL_MIRROR_CLAMP_EXT

// Matrix modes (equivalent to OpenGL)
#define RL_MODELVIEW                    0x1700      // GL_MODELVIEW
#define RL_PROJECTION                   0x1701      // GL_PROJECTION
#define RL_TEXTURE                      0x1702      // GL_TEXTURE

// Primitive assembly draw modes
#define RL_LINES                        0x0001      // GL_LINES
#define RL_TRIANGLES                    0x0004      // GL_TRIANGLES
#define RL_QUADS                        0x0007      // GL_QUADS

// GL equivalent data types
#define RL_UNSIGNED_BYTE                0x1401      // GL_UNSIGNED_BYTE
#define RL_FLOAT                        0x1406      // GL_FLOAT

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { OPENGL_11 = 1, OPENGL_21, OPENGL_33, OPENGL_ES_20 } GlVersion;

typedef enum {
    RL_ATTACHMENT_COLOR_CHANNEL0 = 0,
    RL_ATTACHMENT_COLOR_CHANNEL1,
    RL_ATTACHMENT_COLOR_CHANNEL2,
    RL_ATTACHMENT_COLOR_CHANNEL3,
    RL_ATTACHMENT_COLOR_CHANNEL4,
    RL_ATTACHMENT_COLOR_CHANNEL5,
    RL_ATTACHMENT_COLOR_CHANNEL6,
    RL_ATTACHMENT_COLOR_CHANNEL7,
    RL_ATTACHMENT_DEPTH = 100,
    RL_ATTACHMENT_STENCIL = 200,
} FramebufferAttachType;

typedef enum {
    RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_X,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Y,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Z,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z,
    RL_ATTACHMENT_TEXTURE2D = 100,
    RL_ATTACHMENT_RENDERBUFFER = 200,
} FramebufferAttachTextureType;

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct VertexBuffer {
    int elementsCount;          // Number of elements in the buffer (QUADS)

    int vCounter;               // Vertex position counter to process (and draw) from full buffer
    int tcCounter;              // Vertex texcoord counter to process (and draw) from full buffer
    int cCounter;               // Vertex color counter to process (and draw) from full buffer

    float *vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
    unsigned int vaoId;         // OpenGL Vertex Array Object id
    unsigned int vboId[4];      // OpenGL Vertex Buffer Objects id (4 types of vertex data)
} VertexBuffer;

// Draw call type
// NOTE: Only texture changes register a new draw, other state-change-related elements are not
// used at this moment (vaoId, shaderId, matrices), raylib just forces a batch draw call if any
// of those state-change happens (this is done in core module)
typedef struct DrawCall {
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertexCount;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    //unsigned int vaoId;       // Vertex array id to be used on the draw -> Using RLGL.currentBatch->vertexBuffer.vaoId
    //unsigned int shaderId;    // Shader id to be used on the draw -> Using RLGL.currentShader.id
    unsigned int textureId;     // Texture id to be used on the draw -> Use to create new draw call if changes

    //Matrix projection;        // Projection matrix for this draw -> Using RLGL.projection by default
    //Matrix modelview;         // Modelview matrix for this draw -> Using RLGL.modelview by default
} DrawCall;

// RenderBatch type
typedef struct RenderBatch {
    int buffersCount;           // Number of vertex buffers (multi-buffering support)
    int currentBuffer;          // Current buffer tracking in case of multi-buffering
    VertexBuffer *vertexBuffer; // Dynamic buffer(s) for vertex data

    DrawCall *draws;            // Draw calls array, depends on textureId
    int drawsCounter;           // Draw calls counter
    float currentDepth;         // Current depth value for next draw
} RenderBatch;

// Shader attribute data types
typedef enum {
    SHADER_ATTRIB_FLOAT = 0,
    SHADER_ATTRIB_VEC2,
    SHADER_ATTRIB_VEC3,
    SHADER_ATTRIB_VEC4
} ShaderAttributeDataType;

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

    // Texture type
    // NOTE: Data stored in GPU memory
    typedef struct Texture2D {
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (PixelFormat)
    } Texture2D;

    // Shader type (generic)
    typedef struct Shader {
        unsigned int id;        // Shader program id
        int *locs;              // Shader locations array (MAX_SHADER_LOCATIONS)
    } Shader;

    // TraceLog message types
    typedef enum {
        LOG_ALL,
        LOG_TRACE,
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL,
        LOG_NONE
    } TraceLogLevel;

    // Texture formats (support depends on OpenGL version)
    typedef enum {
        PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
        PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
        PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
        PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
        PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
        PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
        PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
        PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
        PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
        PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
        PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
        PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
        PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
        PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
        PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
        PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
        PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
        PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
        PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
        PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
    } PixelFormat;

    // Texture parameters: filter mode
    // NOTE 1: Filtering considers mipmaps if available in the texture
    // NOTE 2: Filter is accordingly set for minification and magnification
    typedef enum {
        TEXTURE_FILTER_POINT = 0,               // No filter, just pixel aproximation
        TEXTURE_FILTER_BILINEAR,                // Linear filtering
        TEXTURE_FILTER_TRILINEAR,               // Trilinear filtering (linear with mipmaps)
        TEXTURE_FILTER_ANISOTROPIC_4X,          // Anisotropic filtering 4x
        TEXTURE_FILTER_ANISOTROPIC_8X,          // Anisotropic filtering 8x
        TEXTURE_FILTER_ANISOTROPIC_16X,         // Anisotropic filtering 16x
    } TextureFilter;

    // Texture parameters: wrap mode
    typedef enum {
        TEXTURE_WRAP_REPEAT = 0,        // Repeats texture in tiled mode
        TEXTURE_WRAP_CLAMP,             // Clamps texture to edge pixel in tiled mode
        TEXTURE_WRAP_MIRROR_REPEAT,     // Mirrors and repeats the texture in tiled mode
        TEXTURE_WRAP_MIRROR_CLAMP       // Mirrors and clamps to border the texture in tiled mode
    } TextureWrap;

    // Color blending modes (pre-defined)
    typedef enum {
        BLEND_ALPHA = 0,                // Blend textures considering alpha (default)
        BLEND_ADDITIVE,                 // Blend textures adding colors
        BLEND_MULTIPLIED,               // Blend textures multiplying colors
        BLEND_ADD_COLORS,               // Blend textures adding colors (alternative)
        BLEND_SUBTRACT_COLORS,          // Blend textures subtracting colors (alternative)
        BLEND_CUSTOM                    // Belnd textures using custom src/dst factors (use SetBlendModeCustom())
    } BlendMode;

    // Shader location point type
    typedef enum {
        SHADER_LOC_VERTEX_POSITION = 0,
        SHADER_LOC_VERTEX_TEXCOORD01,
        SHADER_LOC_VERTEX_TEXCOORD02,
        SHADER_LOC_VERTEX_NORMAL,
        SHADER_LOC_VERTEX_TANGENT,
        SHADER_LOC_VERTEX_COLOR,
        SHADER_LOC_MATRIX_MVP,
        SHADER_LOC_MATRIX_MODEL,
        SHADER_LOC_MATRIX_VIEW,
        SHADER_LOC_MATRIX_NORMAL,
        SHADER_LOC_MATRIX_PROJECTION,
        SHADER_LOC_VECTOR_VIEW,
        SHADER_LOC_COLOR_DIFFUSE,
        SHADER_LOC_COLOR_SPECULAR,
        SHADER_LOC_COLOR_AMBIENT,
        SHADER_LOC_MAP_ALBEDO,          // SHADER_LOC_MAP_DIFFUSE
        SHADER_LOC_MAP_METALNESS,       // SHADER_LOC_MAP_SPECULAR
        SHADER_LOC_MAP_NORMAL,
        SHADER_LOC_MAP_ROUGHNESS,
        SHADER_LOC_MAP_OCCLUSION,
        SHADER_LOC_MAP_EMISSION,
        SHADER_LOC_MAP_HEIGHT,
        SHADER_LOC_MAP_CUBEMAP,
        SHADER_LOC_MAP_IRRADIANCE,
        SHADER_LOC_MAP_PREFILTER,
        SHADER_LOC_MAP_BRDF
    } ShaderLocationIndex;

    #define SHADER_LOC_MAP_DIFFUSE      SHADER_LOC_MAP_ALBEDO
    #define SHADER_LOC_MAP_SPECULAR     SHADER_LOC_MAP_METALNESS

    // Shader uniform data types
    typedef enum {
        SHADER_UNIFORM_FLOAT = 0,
        SHADER_UNIFORM_VEC2,
        SHADER_UNIFORM_VEC3,
        SHADER_UNIFORM_VEC4,
        SHADER_UNIFORM_INT,
        SHADER_UNIFORM_IVEC2,
        SHADER_UNIFORM_IVEC3,
        SHADER_UNIFORM_IVEC4,
        SHADER_UNIFORM_SAMPLER2D
    } ShaderUniformDataType;
#endif

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------
RLAPI void rlMatrixMode(int mode);                    // Choose the current matrix to be transformed
RLAPI void rlPushMatrix(void);                        // Push the current matrix to stack
RLAPI void rlPopMatrix(void);                         // Pop lattest inserted matrix from stack
RLAPI void rlLoadIdentity(void);                      // Reset current matrix to identity matrix
RLAPI void rlTranslatef(float x, float y, float z);   // Multiply the current matrix by a translation matrix
RLAPI void rlRotatef(float angleDeg, float x, float y, float z);  // Multiply the current matrix by a rotation matrix
RLAPI void rlScalef(float x, float y, float z);       // Multiply the current matrix by a scaling matrix
RLAPI void rlMultMatrixf(float *matf);                // Multiply the current matrix by another matrix
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height); // Set the viewport area

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
RLAPI void rlBegin(int mode);                         // Initialize drawing mode (how to organize vertex)
RLAPI void rlEnd(void);                               // Finish vertex providing
RLAPI void rlVertex2i(int x, int y);                  // Define one vertex (position) - 2 int
RLAPI void rlVertex2f(float x, float y);              // Define one vertex (position) - 2 float
RLAPI void rlVertex3f(float x, float y, float z);     // Define one vertex (position) - 3 float
RLAPI void rlTexCoord2f(float x, float y);            // Define one vertex (texture coordinate) - 2 float
RLAPI void rlNormal3f(float x, float y, float z);     // Define one vertex (normal) - 3 float
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);  // Define one vertex (color) - 4 byte
RLAPI void rlColor3f(float x, float y, float z);          // Define one vertex (color) - 3 float
RLAPI void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL style functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer,
// some of them are direct wrappers over OpenGL calls, some others are custom
//------------------------------------------------------------------------------------

// Vertex buffers state
RLAPI bool rlEnableVertexArray(unsigned int vaoId);     // Enable vertex array (VAO, if supported)
RLAPI void rlDisableVertexArray(void);                  // Disable vertex array (VAO, if supported)
RLAPI void rlEnableVertexBuffer(unsigned int id);       // Enable vertex buffer (VBO)
RLAPI void rlDisableVertexBuffer(void);                 // Disable vertex buffer (VBO)
RLAPI void rlEnableVertexBufferElement(unsigned int id);// Enable vertex buffer element (VBO element)
RLAPI void rlDisableVertexBufferElement(void);          // Disable vertex buffer element (VBO element)
RLAPI void rlEnableVertexAttribute(unsigned int index); // Enable vertex attribute index
RLAPI void rlDisableVertexAttribute(unsigned int index);// Disable vertex attribute index
#if defined(GRAPHICS_API_OPENGL_11)
RLAPI void rlEnableStatePointer(int vertexAttribType, void *buffer);
RLAPI void rlDisableStatePointer(int vertexAttribType);
#endif

// Textures state
RLAPI void rlActiveTextureSlot(int slot);               // Select and active a texture slot
RLAPI void rlEnableTexture(unsigned int id);            // Enable texture
RLAPI void rlDisableTexture(void);                      // Disable texture
RLAPI void rlEnableTextureCubemap(unsigned int id);     // Enable texture cubemap
RLAPI void rlDisableTextureCubemap(void);               // Disable texture cubemap
RLAPI void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)

// Shader state
RLAPI void rlEnableShader(unsigned int id);             // Enable shader program
RLAPI void rlDisableShader(void);                       // Disable shader program

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id);        // Enable render texture (fbo)
RLAPI void rlDisableFramebuffer(void);                  // Disable render texture (fbo), return to default framebuffer

// General render state
RLAPI void rlEnableDepthTest(void);                     // Enable depth test
RLAPI void rlDisableDepthTest(void);                    // Disable depth test
RLAPI void rlEnableDepthMask(void);                     // Enable depth write
RLAPI void rlDisableDepthMask(void);                    // Disable depth write
RLAPI void rlEnableBackfaceCulling(void);               // Enable backface culling
RLAPI void rlDisableBackfaceCulling(void);              // Disable backface culling
RLAPI void rlEnableScissorTest(void);                   // Enable scissor test
RLAPI void rlDisableScissorTest(void);                  // Disable scissor test
RLAPI void rlScissor(int x, int y, int width, int height); // Scissor test
RLAPI void rlEnableWireMode(void);                      // Enable wire mode
RLAPI void rlDisableWireMode(void);                     // Disable wire mode
RLAPI void rlSetLineWidth(float width);                 // Set the line drawing width
RLAPI float rlGetLineWidth(void);                       // Get the line drawing width
RLAPI void rlEnableSmoothLines(void);                   // Enable line aliasing
RLAPI void rlDisableSmoothLines(void);                  // Disable line aliasing
RLAPI void rlEnableStereoRender(void);                  // Enable stereo rendering
RLAPI void rlDisableStereoRender(void);                 // Disable stereo rendering
RLAPI bool rlIsStereoRenderEnabled(void);               // Check if stereo render is enabled

RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Clear color buffer with color
RLAPI void rlClearScreenBuffers(void);                  // Clear used screen buffers (color and depth)
RLAPI void rlCheckErrors(void);                         // Check and log OpenGL error codes
RLAPI void rlSetBlendMode(int mode);                    // Set blending mode
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation); // Set blending mode factor and equation (using OpenGL factors)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
// rlgl initialization functions
RLAPI void rlglInit(int width, int height);           // Initialize rlgl (buffers, shaders, textures, states)
RLAPI void rlglClose(void);                           // De-inititialize rlgl (buffers, shaders, textures)
RLAPI void rlLoadExtensions(void* loader);            // Load OpenGL extensions (loader function pointer required)
RLAPI int rlGetVersion(void);                         // Returns current OpenGL version
RLAPI int rlGetFramebufferWidth(void);                // Get default framebuffer width
RLAPI int rlGetFramebufferHeight(void);               // Get default framebuffer height

RLAPI Shader rlGetShaderDefault(void);                // Get default shader
RLAPI Texture2D rlGetTextureDefault(void);            // Get default texture

// Render batch management
// NOTE: rlgl provides a default render batch to behave like OpenGL 1.1 immediate mode
// but this render batch API is exposed in case of custom batches are required
RLAPI RenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements);  // Load a render batch system
RLAPI void rlUnloadRenderBatch(RenderBatch batch);                        // Unload render batch system
RLAPI void rlDrawRenderBatch(RenderBatch *batch);                         // Draw render batch data (Update->Draw->Reset)
RLAPI void rlSetRenderBatchActive(RenderBatch *batch);                    // Set the active render batch for rlgl (NULL for default internal)
RLAPI void rlDrawRenderBatchActive(void);                                 // Update and draw internal render batch
RLAPI bool rlCheckRenderBatchLimit(int vCount);                           // Check internal buffer overflow for a given number of vertex
RLAPI void rlSetTexture(unsigned int id);           // Set current texture for render batch and check buffers limits

//------------------------------------------------------------------------------------------------------------------------

// Vertex buffers management
RLAPI unsigned int rlLoadVertexArray(void);                               // Load vertex array (vao) if supported
RLAPI unsigned int rlLoadVertexBuffer(void *buffer, int size, bool dynamic);            // Load a vertex buffer attribute
RLAPI unsigned int rlLoadVertexBufferElement(void *buffer, int size, bool dynamic);     // Load a new attributes element buffer
RLAPI void rlUpdateVertexBuffer(int bufferId, void *data, int dataSize, int offset);    // Update GPU buffer with new data
RLAPI void rlUnloadVertexArray(unsigned int vaoId);
RLAPI void rlUnloadVertexBuffer(unsigned int vboId);
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, void *pointer);
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor);
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count); // Set vertex attribute default value
RLAPI void rlDrawVertexArray(int offset, int count);
RLAPI void rlDrawVertexArrayElements(int offset, int count, void *buffer);
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances);
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, void *buffer, int instances);

// Textures management
RLAPI unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount); // Load texture in GPU
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer);               // Load depth texture/renderbuffer (to be attached to fbo)
RLAPI unsigned int rlLoadTextureCubemap(void *data, int size, int format);                        // Load texture cubemap
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data);  // Update GPU texture with new data
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType);  // Get OpenGL internal formats
RLAPI void rlUnloadTexture(unsigned int id);                              // Unload texture from GPU memory
RLAPI void rlGenerateMipmaps(Texture2D *texture);                         // Generate mipmap data for selected texture
RLAPI void *rlReadTexturePixels(Texture2D texture);                       // Read texture pixel data
RLAPI unsigned char *rlReadScreenPixels(int width, int height);           // Read screen pixel data (color buffer)

// Framebuffer management (fbo)
RLAPI unsigned int rlLoadFramebuffer(int width, int height);              // Load an empty framebuffer
RLAPI void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel);  // Attach texture/renderbuffer to a framebuffer
RLAPI bool rlFramebufferComplete(unsigned int id);                        // Verify framebuffer is complete
RLAPI void rlUnloadFramebuffer(unsigned int id);                          // Delete framebuffer from GPU

// Shaders management
RLAPI unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode);    // Load shader from code strings
RLAPI unsigned int rlCompileShader(const char *shaderCode, int type);           // Compile custom shader and return shader id (type: GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
RLAPI unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId); // Load custom shader program
RLAPI void rlUnloadShaderProgram(unsigned int id);                              // Unload shader program
RLAPI int rlGetLocationUniform(unsigned int shaderId, const char *uniformName); // Get shader location uniform
RLAPI int rlGetLocationAttrib(unsigned int shaderId, const char *attribName);   // Get shader location attribute
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count); // Set shader value uniform
RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat);                        // Set shader value matrix
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId);           // Set shader value sampler
RLAPI void rlSetShader(Shader shader);                                    // Set shader currently active

// Matrix state management
RLAPI Matrix rlGetMatrixModelview(void);                                  // Get internal modelview matrix
RLAPI Matrix rlGetMatrixProjection(void);                                 // Get internal projection matrix
RLAPI Matrix rlGetMatrixTransform(void);                                  // Get internal accumulated transform matrix
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);                        // Get internal projection matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);                        // Get internal view offset matrix for stereo render (selected eye)
RLAPI void rlSetMatrixProjection(Matrix proj);                            // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void rlSetMatrixModelview(Matrix view);                             // Set a custom modelview matrix (replaces internal modelview matrix)
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left);        // Set eyes projection matrices for stereo rendering
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left);        // Set eyes view offsets matrices for stereo rendering

// Quick and dirty cube/quad buffers load->draw->unload
RLAPI void rlLoadDrawCube(void);     // Load and draw a cube
RLAPI void rlLoadDrawQuad(void);     // Load and draw a quad
#if defined(__cplusplus)
}
#endif

#endif // RLGL_H

/***********************************************************************************
*
*   RLGL IMPLEMENTATION
*
************************************************************************************/

#if defined(RLGL_IMPLEMENTATION)

#if !defined(RLGL_STANDALONE)
    // Check if config flags have been externally provided on compilation line
    #if !defined(EXTERNAL_CONFIG_FLAGS)
        #include "config.h"             // Defines module configuration flags
    #endif
    #include "raymath.h"                // Required for: Vector3 and Matrix functions
#endif

#include <stdlib.h>                     // Required for: malloc(), free()
#include <string.h>                     // Required for: strcmp(), strlen() [Used in rlglInit(), on extensions loading]

#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(__APPLE__)
        #include <OpenGL/gl.h>          // OpenGL 1.1 library for OSX
        #include <OpenGL/glext.h>
    #else
        // APIENTRY for OpenGL function pointer declarations is required
        #ifndef APIENTRY
            #if defined(_WIN32)
                #define APIENTRY __stdcall
            #else
                #define APIENTRY
            #endif
        #endif
        // WINGDIAPI definition. Some Windows OpenGL headers need it
        #if !defined(WINGDIAPI) && defined(_WIN32)
            #define WINGDIAPI __declspec(dllimport)
        #endif

        #include <GL/gl.h>              // OpenGL 1.1 library
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_33)
    #if defined(__APPLE__)
        #include <OpenGL/gl3.h>         // OpenGL 3 library for OSX
        #include <OpenGL/gl3ext.h>      // OpenGL 3 extensions library for OSX
    #else
        #define GLAD_REALLOC RL_REALLOC
        #define GLAD_FREE RL_FREE

        #define GLAD_IMPLEMENTATION
        #if defined(RLGL_STANDALONE)
            #include "glad.h"           // GLAD extensions loading library, includes OpenGL headers
        #else
            #include "external/glad.h"  // GLAD extensions loading library, includes OpenGL headers
        #endif
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #define GL_GLEXT_PROTOTYPES
    #include <EGL/egl.h>                // EGL library
    #include <GLES2/gl2.h>              // OpenGL ES 2.0 library
    #include <GLES2/gl2ext.h>           // OpenGL ES 2.0 extensions library

    // It seems OpenGL ES 2.0 instancing entry points are not defined on Raspberry Pi
    // provided headers (despite being defined in official Khronos GLES2 headers)
    #if defined(PLATFORM_RPI) || defined(PLATFORM_DRM)
    typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef GL_SHADING_LANGUAGE_VERSION
    #define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#endif

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#endif
#ifndef GL_ETC1_RGB8_OES
    #define GL_ETC1_RGB8_OES                    0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2             0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    #define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    #define GL_UNSIGNED_SHORT_5_6_5             0x8363
    #define GL_UNSIGNED_SHORT_5_5_5_1           0x8034
    #define GL_UNSIGNED_SHORT_4_4_4_4           0x8033
#endif

#if defined(GRAPHICS_API_OPENGL_21)
    #define GL_LUMINANCE                        0x1909
    #define GL_LUMINANCE_ALPHA                  0x190A
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #define glClearDepth                 glClearDepthf
    #define GL_READ_FRAMEBUFFER         GL_FRAMEBUFFER
    #define GL_DRAW_FRAMEBUFFER         GL_FRAMEBUFFER
#endif

// Default shader vertex attribute names to set location points
#ifndef DEFAULT_SHADER_ATTRIB_NAME_POSITION
    #define DEFAULT_SHADER_ATTRIB_NAME_POSITION    "vertexPosition"    // Binded by default to shader location: 0
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
    #define DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD    "vertexTexCoord"    // Binded by default to shader location: 1
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_NORMAL
    #define DEFAULT_SHADER_ATTRIB_NAME_NORMAL      "vertexNormal"      // Binded by default to shader location: 2
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_COLOR
    #define DEFAULT_SHADER_ATTRIB_NAME_COLOR       "vertexColor"       // Binded by default to shader location: 3
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_TANGENT
    #define DEFAULT_SHADER_ATTRIB_NAME_TANGENT     "vertexTangent"     // Binded by default to shader location: 4
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
    #define DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2   "vertexTexCoord2"   // Binded by default to shader location: 5
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
typedef struct rlglData {
    RenderBatch *currentBatch;              // Current render batch
    RenderBatch defaultBatch;               // Default internal render batch

    struct {
        int currentMatrixMode;              // Current matrix mode
        Matrix *currentMatrix;              // Current matrix pointer
        Matrix modelview;                   // Default modelview matrix
        Matrix projection;                  // Default projection matrix
        Matrix transform;                   // Transform matrix to be used with rlTranslate, rlRotate, rlScale
        bool transformRequired;             // Require transform matrix application to current draw-call vertex (if required)
        Matrix stack[MAX_MATRIX_STACK_SIZE];// Matrix stack for push/pop
        int stackCounter;                   // Matrix stack counter

        unsigned int defaultTextureId;      // Default texture used on shapes/poly drawing (required by shader)
        unsigned int activeTextureId[MAX_BATCH_ACTIVE_TEXTURES];    // Active texture ids to be enabled on batch drawing (0 active by default)
        unsigned int defaultVShaderId;      // Default vertex shader id (used by default shader program)
        unsigned int defaultFShaderId;      // Default fragment shader Id (used by default shader program)
        Shader defaultShader;               // Basic shader, support vertex color and diffuse texture
        Shader currentShader;               // Shader to be used on rendering (by default, defaultShader)

        bool stereoRender;                  // Stereo rendering flag
        Matrix projectionStereo[2];         // VR stereo rendering eyes projection matrices
        Matrix viewOffsetStereo[2];         // VR stereo rendering eyes view offset matrices

        int currentBlendMode;               // Blending mode active
        int glBlendSrcFactor;               // Blending source factor
        int glBlendDstFactor;               // Blending destination factor
        int glBlendEquation;                // Blending equation

        int framebufferWidth;               // Default framebuffer width
        int framebufferHeight;              // Default framebuffer height

    } State;            // Renderer state
    struct {
        bool vao;                           // VAO support (OpenGL ES2 could not support VAO extension) (GL_ARB_vertex_array_object)
        bool instancing;                    // Instancing supported (GL_ANGLE_instanced_arrays, GL_EXT_draw_instanced + GL_EXT_instanced_arrays)
        bool texNPOT;                       // NPOT textures full support (GL_ARB_texture_non_power_of_two, GL_OES_texture_npot)
        bool texDepth;                      // Depth textures supported (GL_ARB_depth_texture, GL_WEBGL_depth_texture, GL_OES_depth_texture)
        bool texFloat32;                    // float textures support (32 bit per channel) (GL_OES_texture_float)
        bool texCompDXT;                    // DDS texture compression support (GL_EXT_texture_compression_s3tc, GL_WEBGL_compressed_texture_s3tc, GL_WEBKIT_WEBGL_compressed_texture_s3tc)
        bool texCompETC1;                   // ETC1 texture compression support (GL_OES_compressed_ETC1_RGB8_texture, GL_WEBGL_compressed_texture_etc1)
        bool texCompETC2;                   // ETC2/EAC texture compression support (GL_ARB_ES3_compatibility)
        bool texCompPVRT;                   // PVR texture compression support (GL_IMG_texture_compression_pvrtc)
        bool texCompASTC;                   // ASTC texture compression support (GL_KHR_texture_compression_astc_hdr, GL_KHR_texture_compression_astc_ldr)
        bool texMirrorClamp;                // Clamp mirror wrap mode supported (GL_EXT_texture_mirror_clamp)
        bool texAnisoFilter;                // Anisotropic texture filtering support (GL_EXT_texture_filter_anisotropic)

        float maxAnisotropyLevel;          // Maximum anisotropy level supported (minimum is 2.0f)
        int maxDepthBits;                   // Maximum bits for depth component

    } ExtSupported;     // Extensions supported flags
} rlglData;
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static rlglData RLGL = { 0 };
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

#if defined(GRAPHICS_API_OPENGL_ES2)
// NOTE: VAO functionality is exposed through extensions (OES)
static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray = NULL;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays = NULL;

// NOTE: Instancing functionality could also be available through extension
static PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstanced = NULL;
static PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstanced = NULL;
static PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisor = NULL;
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static void rlLoadShaderDefault(void);      // Load default shader (RLGL.State.defaultShader)
static void rlUnloadShaderDefault(void);    // Unload default shader (RLGL.State.defaultShader)
#if defined(SUPPORT_GL_DETAILS_INFO)
static char *rlGetCompressedFormatName(int format); // Get compressed format official GL identifier name
#endif  // SUPPORT_GL_DETAILS_INFO
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2
#if defined(GRAPHICS_API_OPENGL_11)
static int rlGenerateMipmapsData(unsigned char *data, int baseWidth, int baseHeight);   // Generate mipmaps data on CPU side
static Color *rlGenNextMipmapData(Color *srcData, int srcWidth, int srcHeight);         // Generate next mipmap level on CPU side
#endif
static int rlGetPixelDataSize(int width, int height, int format);   // Get pixel data size in bytes (image or texture)

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix operations
//----------------------------------------------------------------------------------

#if defined(GRAPHICS_API_OPENGL_11)
// Fallback to OpenGL 1.1 function calls
//---------------------------------------
void rlMatrixMode(int mode)
{
    switch (mode)
    {
        case RL_PROJECTION: glMatrixMode(GL_PROJECTION); break;
        case RL_MODELVIEW: glMatrixMode(GL_MODELVIEW); break;
        case RL_TEXTURE: glMatrixMode(GL_TEXTURE); break;
        default: break;
    }
}

void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    glFrustum(left, right, bottom, top, znear, zfar);
}

void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    glOrtho(left, right, bottom, top, znear, zfar);
}

void rlPushMatrix(void) { glPushMatrix(); }
void rlPopMatrix(void) { glPopMatrix(); }
void rlLoadIdentity(void) { glLoadIdentity(); }
void rlTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
void rlRotatef(float angleDeg, float x, float y, float z) { glRotatef(angleDeg, x, y, z); }
void rlScalef(float x, float y, float z) { glScalef(x, y, z); }
void rlMultMatrixf(float *matf) { glMultMatrixf(matf); }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) RLGL.State.currentMatrix = &RLGL.State.projection;
    else if (mode == RL_MODELVIEW) RLGL.State.currentMatrix = &RLGL.State.modelview;
    //else if (mode == RL_TEXTURE) // Not supported

    RLGL.State.currentMatrixMode = mode;
}

// Push the current matrix into RLGL.State.stack
void rlPushMatrix(void)
{
    if (RLGL.State.stackCounter >= MAX_MATRIX_STACK_SIZE) TRACELOG(LOG_ERROR, "RLGL: Matrix stack overflow (MAX_MATRIX_STACK_SIZE)");

    if (RLGL.State.currentMatrixMode == RL_MODELVIEW)
    {
        RLGL.State.transformRequired = true;
        RLGL.State.currentMatrix = &RLGL.State.transform;
    }

    RLGL.State.stack[RLGL.State.stackCounter] = *RLGL.State.currentMatrix;
    RLGL.State.stackCounter++;
}

// Pop lattest inserted matrix from RLGL.State.stack
void rlPopMatrix(void)
{
    if (RLGL.State.stackCounter > 0)
    {
        Matrix mat = RLGL.State.stack[RLGL.State.stackCounter - 1];
        *RLGL.State.currentMatrix = mat;
        RLGL.State.stackCounter--;
    }

    if ((RLGL.State.stackCounter == 0) && (RLGL.State.currentMatrixMode == RL_MODELVIEW))
    {
        RLGL.State.currentMatrix = &RLGL.State.modelview;
        RLGL.State.transformRequired = false;
    }
}

// Reset current matrix to identity matrix
void rlLoadIdentity(void)
{
    *RLGL.State.currentMatrix = MatrixIdentity();
}

// Multiply the current matrix by a translation matrix
void rlTranslatef(float x, float y, float z)
{
    Matrix matTranslation = MatrixTranslate(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matTranslation, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by a rotation matrix
void rlRotatef(float angleDeg, float x, float y, float z)
{
    Matrix matRotation = MatrixIdentity();

    Vector3 axis = (Vector3){ x, y, z };
    matRotation = MatrixRotate(Vector3Normalize(axis), angleDeg*DEG2RAD);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matRotation, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by a scaling matrix
void rlScalef(float x, float y, float z)
{
    Matrix matScale = MatrixScale(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matScale, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by another matrix
void rlMultMatrixf(float *matf)
{
    // Matrix creation from array
    Matrix mat = { matf[0], matf[4], matf[8], matf[12],
                   matf[1], matf[5], matf[9], matf[13],
                   matf[2], matf[6], matf[10], matf[14],
                   matf[3], matf[7], matf[11], matf[15] };

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, mat);
}

// Multiply the current matrix by a perspective matrix generated by parameters
void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    Matrix matPerps = MatrixFrustum(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matPerps);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    // NOTE: If left-right and top-botton values are equal it could create
    // a division by zero on MatrixOrtho(), response to it is platform/compiler dependant
    Matrix matOrtho = MatrixOrtho(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matOrtho);
}
#endif

// Set the viewport area (transformation from normalized device coordinates to window coordinates)
void rlViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Vertex level operations
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_11)
// Fallback to OpenGL 1.1 function calls
//---------------------------------------
void rlBegin(int mode)
{
    switch (mode)
    {
        case RL_LINES: glBegin(GL_LINES); break;
        case RL_TRIANGLES: glBegin(GL_TRIANGLES); break;
        case RL_QUADS: glBegin(GL_QUADS); break;
        default: break;
    }
}

void rlEnd() { glEnd(); }
void rlVertex2i(int x, int y) { glVertex2i(x, y); }
void rlVertex2f(float x, float y) { glVertex2f(x, y); }
void rlVertex3f(float x, float y, float z) { glVertex3f(x, y, z); }
void rlTexCoord2f(float x, float y) { glTexCoord2f(x, y); }
void rlNormal3f(float x, float y, float z) { glNormal3f(x, y, z); }
void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { glColor4ub(r, g, b, a); }
void rlColor3f(float x, float y, float z) { glColor3f(x, y, z); }
void rlColor4f(float x, float y, float z, float w) { glColor4f(x, y, z, w); }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Initialize drawing mode (how to organize vertex)
void rlBegin(int mode)
{
    // Draw mode can be RL_LINES, RL_TRIANGLES and RL_QUADS
    // NOTE: In all three cases, vertex are accumulated over default internal vertex buffer
    if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode != mode)
    {
        if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount > 0)
        {
            // Make sure current RLGL.currentBatch->draws[i].vertexCount is aligned a multiple of 4,
            // that way, following QUADS drawing will keep aligned with index processing
            // It implies adding some extra alignment vertex at the end of the draw,
            // those vertex are not processed but they are considered as an additional offset
            // for the next set of vertex to be drawn
            if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_LINES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount : RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4);
            else if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_TRIANGLES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4)));
            else RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = 0;

            if (!rlCheckRenderBatchLimit(RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment))
            {
                RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;

                RLGL.currentBatch->drawsCounter++;
            }
        }

        if (RLGL.currentBatch->drawsCounter >= DEFAULT_BATCH_DRAWCALLS) rlDrawRenderBatch(RLGL.currentBatch);

        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode = mode;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount = 0;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].textureId = RLGL.State.defaultTextureId;
    }
}

// Finish vertex providing
void rlEnd(void)
{
    // Make sure vertexCount is the same for vertices, texcoords, colors and normals
    // NOTE: In OpenGL 1.1, one glColor call can be made for all the subsequent glVertex calls

    // Make sure colors count match vertex count
    if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter != RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter)
    {
        int addColors = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter - RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter;

        for (int i = 0; i < addColors; i++)
        {
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 4];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 1] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 3];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 2] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 2];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 3] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 1];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter++;
        }
    }

    // Make sure texcoords count match vertex count
    if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter != RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter)
    {
        int addTexCoords = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter - RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter;

        for (int i = 0; i < addTexCoords; i++)
        {
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter] = 0.0f;
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter + 1] = 0.0f;
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter++;
        }
    }

    // TODO: Make sure normals count match vertex count... if normals support is added in a future... :P

    // NOTE: Depth increment is dependant on rlOrtho(): z-near and z-far values,
    // as well as depth buffer bit-depth (16bit or 24bit or 32bit)
    // Correct increment formula would be: depthInc = (zfar - znear)/pow(2, bits)
    RLGL.currentBatch->currentDepth += (1.0f/20000.0f);

    // Verify internal buffers limits
    // NOTE: This check is combined with usage of rlCheckRenderBatchLimit()
    if ((RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter) >=
        (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4 - 4))
    {
        // WARNING: If we are between rlPushMatrix() and rlPopMatrix() and we need to force a rlDrawRenderBatch(),
        // we need to call rlPopMatrix() before to recover *RLGL.State.currentMatrix (RLGL.State.modelview) for the next forced draw call!
        // If we have multiple matrix pushed, it will require "RLGL.State.stackCounter" pops before launching the draw
        for (int i = RLGL.State.stackCounter; i >= 0; i--) rlPopMatrix();
        rlDrawRenderBatch(RLGL.currentBatch);
    }
}

// Define one vertex (position)
// NOTE: Vertex position data is the basic information required for drawing
void rlVertex3f(float x, float y, float z)
{
    Vector3 vec = { x, y, z };

    // Transform provided vector if required
    if (RLGL.State.transformRequired) vec = Vector3Transform(vec, RLGL.State.transform);

    // Verify that current vertex buffer elements limit has not been reached
    if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter < (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4))
    {
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter] = vec.x;
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + 1] = vec.y;
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + 2] = vec.z;
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter++;

        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount++;
    }
    else TRACELOG(LOG_ERROR, "RLGL: Batch elements overflow");
}

// Define one vertex (position)
void rlVertex2f(float x, float y)
{
    rlVertex3f(x, y, RLGL.currentBatch->currentDepth);
}

// Define one vertex (position)
void rlVertex2i(int x, int y)
{
    rlVertex3f((float)x, (float)y, RLGL.currentBatch->currentDepth);
}

// Define one vertex (texture coordinate)
// NOTE: Texture coordinates are limited to QUADS only
void rlTexCoord2f(float x, float y)
{
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter] = x;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter + 1] = y;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter++;
}

// Define one vertex (normal)
// NOTE: Normals limited to TRIANGLES only?
void rlNormal3f(float x, float y, float z)
{
    // TODO: Normals usage...
}

// Define one vertex (color)
void rlColor4ub(unsigned char x, unsigned char y, unsigned char z, unsigned char w)
{
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter] = x;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 1] = y;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 2] = z;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 3] = w;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter++;
}

// Define one vertex (color)
void rlColor4f(float r, float g, float b, float a)
{
    rlColor4ub((unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255), (unsigned char)(a*255));
}

// Define one vertex (color)
void rlColor3f(float x, float y, float z)
{
    rlColor4ub((unsigned char)(x*255), (unsigned char)(y*255), (unsigned char)(z*255), 255);
}

#endif

//--------------------------------------------------------------------------------------
// Module Functions Definition - OpenGL style functions (common to 1.1, 3.3+, ES2)
//--------------------------------------------------------------------------------------

// Set current texture to use
void rlSetTexture(unsigned int id)
{
    if (id == 0)
    {
#if defined(GRAPHICS_API_OPENGL_11)
        rlDisableTexture();
#else
        // NOTE: If quads batch limit is reached, we force a draw call and next batch starts
        if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter >=
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4)
        {
            rlDrawRenderBatch(RLGL.currentBatch);
        }
#endif
    }
    else
    {
#if defined(GRAPHICS_API_OPENGL_11)
        rlEnableTexture(id);
#else
        if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].textureId != id)
        {
            if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount > 0)
            {
                // Make sure current RLGL.currentBatch->draws[i].vertexCount is aligned a multiple of 4,
                // that way, following QUADS drawing will keep aligned with index processing
                // It implies adding some extra alignment vertex at the end of the draw,
                // those vertex are not processed but they are considered as an additional offset
                // for the next set of vertex to be drawn
                if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_LINES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount : RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4);
                else if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_TRIANGLES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4)));
                else RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = 0;

                if (!rlCheckRenderBatchLimit(RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment))
                {
                    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;

                    RLGL.currentBatch->drawsCounter++;
                }
            }

            if (RLGL.currentBatch->drawsCounter >= DEFAULT_BATCH_DRAWCALLS) rlDrawRenderBatch(RLGL.currentBatch);

            RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].textureId = id;
            RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount = 0;
        }
#endif
    }
}

// Select and active a texture slot
void rlActiveTextureSlot(int slot)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glActiveTexture(GL_TEXTURE0 + slot);
#endif
}

// Enable texture
void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, id);
}

// Disable texture
void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Enable texture cubemap
void rlEnableTextureCubemap(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glEnable(GL_TEXTURE_CUBE_MAP);   // Core in OpenGL 1.4
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
#endif
}

// Disable texture cubemap
void rlDisableTextureCubemap(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDisable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}

// Set texture parameters (wrap mode/filter mode)
void rlTextureParameters(unsigned int id, int param, int value)
{
    glBindTexture(GL_TEXTURE_2D, id);

    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
#if !defined(GRAPHICS_API_OPENGL_11)
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_2D, param, value);
                else TRACELOG(LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
#endif
            }
            else glTexParameteri(GL_TEXTURE_2D, param, value);

        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_2D, param, value); break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
#if !defined(GRAPHICS_API_OPENGL_11)
            if (value <= RLGL.ExtSupported.maxAnisotropyLevel) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (RLGL.ExtSupported.maxAnisotropyLevel > 0.0f)
            {
                TRACELOG(LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, RLGL.ExtSupported.maxAnisotropyLevel);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else TRACELOG(LOG_WARNING, "GL: Anisotropic filtering not supported");
#endif
        } break;
        default: break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Enable shader program
void rlEnableShader(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(id);
#endif
}

// Disable shader program
void rlDisableShader(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(0);
#endif
}

// Enable rendering to texture (fbo)
void rlEnableFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);
#endif
}

// Disable rendering to texture
void rlDisableFramebuffer(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Enable depth test
void rlEnableDepthTest(void) { glEnable(GL_DEPTH_TEST); }

// Disable depth test
void rlDisableDepthTest(void) { glDisable(GL_DEPTH_TEST); }

// Enable depth write
void rlEnableDepthMask(void) { glDepthMask(GL_TRUE); }

// Disable depth write
void rlDisableDepthMask(void) { glDepthMask(GL_FALSE); }

// Enable backface culling
void rlEnableBackfaceCulling(void) { glEnable(GL_CULL_FACE); }

// Disable backface culling
void rlDisableBackfaceCulling(void) { glDisable(GL_CULL_FACE); }

// Enable scissor test
void rlEnableScissorTest(void) { glEnable(GL_SCISSOR_TEST); }

// Disable scissor test
void rlDisableScissorTest(void) { glDisable(GL_SCISSOR_TEST); }

// Scissor test
void rlScissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }

// Enable wire mode
void rlEnableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}

// Disable wire mode
void rlDisableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}
// Set the line drawing width
void rlSetLineWidth(float width)
{
    glLineWidth(width);
}

// Get the line drawing width
float rlGetLineWidth(void)
{
    float width = 0;
    glGetFloatv(GL_LINE_WIDTH, &width);
    return width;
}

// Enable line aliasing
void rlEnableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_LINE_SMOOTH);
#endif
}

// Disable line aliasing
void rlDisableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_LINE_SMOOTH);
#endif
}

// Enable stereo rendering
void rlEnableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = true;
#endif
}

// Disable stereo rendering
void rlDisableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = false;
#endif
}

// Check if stereo render is enabled
bool rlIsStereoRenderEnabled(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    return RLGL.State.stereoRender;
#else
    return false;
#endif
}

// Clear color buffer with color
void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    // Color values clamp to 0.0f(0) and 1.0f(255)
    float cr = (float)r/255;
    float cg = (float)g/255;
    float cb = (float)b/255;
    float ca = (float)a/255;

    glClearColor(cr, cg, cb, ca);
}

// Clear used screen buffers (color and depth)
void rlClearScreenBuffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers: Color and Depth (Depth is used for 3D)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     // Stencil buffer not used...
}

// Check and log OpenGL error codes
void rlCheckErrors()
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    int check = 1;
    while (check)
    {
        const GLenum err = glGetError();
        switch (err)
        {
            case GL_NO_ERROR: check = 0; break;
            case 0x0500: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_ENUM"); break;
            case 0x0501: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_VALUE"); break;
            case 0x0502: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_OPERATION"); break;
            case 0x0503: TRACELOG(LOG_WARNING, "GL: Error detected: GL_STACK_OVERFLOW"); break;
            case 0x0504: TRACELOG(LOG_WARNING, "GL: Error detected: GL_STACK_UNDERFLOW"); break;
            case 0x0505: TRACELOG(LOG_WARNING, "GL: Error detected: GL_OUT_OF_MEMORY"); break;
            case 0x0506: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_FRAMEBUFFER_OPERATION"); break;
            default: TRACELOG(LOG_WARNING, "GL: Error detected: Unknown error code: %x", err); break;
        }
    }
#endif
}

// Set blend mode
void rlSetBlendMode(int mode)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.State.currentBlendMode != mode)
    {
        rlDrawRenderBatch(RLGL.currentBatch);

        switch (mode)
        {
            case BLEND_ALPHA: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_ADDITIVE: glBlendFunc(GL_SRC_ALPHA, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_MULTIPLIED: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_ADD_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_SUBTRACT_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_SUBTRACT); break;
            case BLEND_CUSTOM: glBlendFunc(RLGL.State.glBlendSrcFactor, RLGL.State.glBlendDstFactor); glBlendEquation(RLGL.State.glBlendEquation); break;
            default: break;
        }

        RLGL.State.currentBlendMode = mode;
    }
#endif
}

// Set blending mode factor and equation
void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.glBlendSrcFactor = glSrcFactor;
    RLGL.State.glBlendDstFactor = glDstFactor;
    RLGL.State.glBlendEquation = glEquation;
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - rlgl functionality
//----------------------------------------------------------------------------------

// Initialize rlgl: OpenGL extensions, default buffers/shaders/textures, OpenGL states
void rlglInit(int width, int height)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Init default white texture
    unsigned char pixels[4] = { 255, 255, 255, 255 };   // 1 pixel RGBA (4 bytes)
    RLGL.State.defaultTextureId = rlLoadTexture(pixels, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    if (RLGL.State.defaultTextureId != 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Default texture loaded successfully", RLGL.State.defaultTextureId);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load default texture");

    // Init default Shader (customized for GL 3.3 and ES2)
    rlLoadShaderDefault(); // RLGL.State.defaultShader
    RLGL.State.currentShader = RLGL.State.defaultShader;

    // Init default vertex arrays buffers
    RLGL.defaultBatch = rlLoadRenderBatch(DEFAULT_BATCH_BUFFERS, DEFAULT_BATCH_BUFFER_ELEMENTS);
    RLGL.currentBatch = &RLGL.defaultBatch;

    // Init stack matrices (emulating OpenGL 1.1)
    for (int i = 0; i < MAX_MATRIX_STACK_SIZE; i++) RLGL.State.stack[i] = MatrixIdentity();

    // Init internal matrices
    RLGL.State.transform = MatrixIdentity();
    RLGL.State.projection = MatrixIdentity();
    RLGL.State.modelview = MatrixIdentity();
    RLGL.State.currentMatrix = &RLGL.State.modelview;
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

    // Initialize OpenGL default states
    //----------------------------------------------------------
    // Init state: Depth test
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    glDisable(GL_DEPTH_TEST);                               // Disable depth testing for 2D (only used for 3D)

    // Init state: Blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)

    // Init state: Culling
    // NOTE: All shapes/models triangles are drawn CCW
    glCullFace(GL_BACK);                                    // Cull the back face (default)
    glFrontFace(GL_CCW);                                    // Front face are defined counter clockwise (default)
    glEnable(GL_CULL_FACE);                                 // Enable backface culling

    // Init state: Cubemap seamless
#if defined(GRAPHICS_API_OPENGL_33)
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);                 // Seamless cubemaps (not supported on OpenGL ES 2.0)
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    // Init state: Color hints (deprecated in OpenGL 3.0+)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation
    glShadeModel(GL_SMOOTH);                                // Smooth shading between vertex (vertex colors interpolation)
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Store screen size into global variables
    RLGL.State.framebufferWidth = width;
    RLGL.State.framebufferHeight = height;

    TRACELOG(LOG_INFO, "RLGL: Default OpenGL state initialized successfully");
    //----------------------------------------------------------
#endif

    // Init state: Color/Depth buffers clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   // Set clear color (black)
    glClearDepth(1.0f);                                     // Set clear depth value (default)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear color and depth buffers (depth buffer required for 3D)
}

// Vertex Buffer Object deinitialization (memory free)
void rlglClose(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlUnloadRenderBatch(RLGL.defaultBatch);

    rlUnloadShaderDefault();          // Unload default shader

    glDeleteTextures(1, &RLGL.State.defaultTextureId); // Unload default texture
    TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Default texture unloaded successfully", RLGL.State.defaultTextureId);
#endif
}

// Load OpenGL extensions
// NOTE: External loader function could be passed as a pointer
void rlLoadExtensions(void *loader)
{
#if defined(GRAPHICS_API_OPENGL_33)     // Also defined for GRAPHICS_API_OPENGL_21
    // NOTE: glad is generated and contains only required OpenGL 3.3 Core extensions (and lower versions)
    #if !defined(__APPLE__)
        if (!gladLoadGLLoader((GLADloadproc)loader)) TRACELOG(LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
        else TRACELOG(LOG_INFO, "GLAD: OpenGL extensions loaded successfully");
    #endif

    // Get number of supported extensions
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    TRACELOG(LOG_INFO, "GL: Supported extensions count: %i", numExt);

#if defined(SUPPORT_GL_DETAILS_INFO)
    // Get supported extensions list
    // WARNING: glGetStringi() not available on OpenGL 2.1
    char **extList = RL_MALLOC(sizeof(char *)*numExt);
    TRACELOG(LOG_INFO, "GL: OpenGL extensions:");
    for (int i = 0; i < numExt; i++)
    {
        extList[i] = (char *)glGetStringi(GL_EXTENSIONS, i);
        TRACELOG(LOG_INFO, "    %s", extList[i]);
    }
    RL_FREE(extList);       // Free extensions pointers
#endif

    // Register supported extensions flags
    // OpenGL 3.3 extensions supported by default (core)
    RLGL.ExtSupported.vao = true;
    RLGL.ExtSupported.instancing = true;
    RLGL.ExtSupported.texNPOT = true;
    RLGL.ExtSupported.texFloat32 = true;
    RLGL.ExtSupported.texDepth = true;
    RLGL.ExtSupported.maxDepthBits = 32;
    RLGL.ExtSupported.texAnisoFilter = true;
    RLGL.ExtSupported.texMirrorClamp = true;
    #if !defined(__APPLE__)
    // NOTE: With GLAD, we can check if an extension is supported using the GLAD_GL_xxx booleans
    if (GLAD_GL_EXT_texture_compression_s3tc) RLGL.ExtSupported.texCompDXT = true;  // Texture compression: DXT
    if (GLAD_GL_ARB_ES3_compatibility) RLGL.ExtSupported.texCompETC2 = true;        // Texture compression: ETC2/EAC
    #endif
#endif  // GRAPHICS_API_OPENGL_33

#if defined(GRAPHICS_API_OPENGL_ES2)
    // Get supported extensions list
    GLint numExt = 0;
    const char **extList = RL_MALLOC(512*sizeof(const char *)); // Allocate 512 strings pointers (2 KB)
    const char *extensions = (const char *)glGetString(GL_EXTENSIONS);  // One big const string

    // NOTE: We have to duplicate string because glGetString() returns a const string
    int len = strlen(extensions) + 1;
    char *extensionsDup = (char *)RL_CALLOC(len, sizeof(char));
    strcpy(extensionsDup, extensions);
    extList[numExt] = extensionsDup;

    for (int i = 0; i < len; i++)
    {
        if (extensionsDup[i] == ' ')
        {
            extensionsDup[i] = '\0';
            numExt++;
            extList[numExt] = &extensionsDup[i + 1];
        }
    }

    TRACELOG(LOG_INFO, "GL: Supported extensions count: %i", numExt);

#if defined(SUPPORT_GL_DETAILS_INFO)
    TRACELOG(LOG_INFO, "GL: OpenGL extensions:");
    for (int i = 0; i < numExt; i++) TRACELOG(LOG_INFO, "    %s", extList[i]);
#endif

    // Check required extensions
    for (int i = 0; i < numExt; i++)
    {
        // Check VAO support
        // NOTE: Only check on OpenGL ES, OpenGL 3.3 has VAO support as core feature
        if (strcmp(extList[i], (const char *)"GL_OES_vertex_array_object") == 0)
        {
            // The extension is supported by our hardware and driver, try to get related functions pointers
            // NOTE: emscripten does not support VAOs natively, it uses emulation and it reduces overall performance...
            glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
            glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
            glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
            //glIsVertexArray = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");     // NOTE: Fails in WebGL, omitted

            if ((glGenVertexArrays != NULL) && (glBindVertexArray != NULL) && (glDeleteVertexArrays != NULL)) RLGL.ExtSupported.vao = true;
        }

        // Check instanced rendering support
        if (strcmp(extList[i], (const char *)"GL_ANGLE_instanced_arrays") == 0)         // Web ANGLE
        {
            glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)eglGetProcAddress("glDrawArraysInstancedANGLE");
            glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)eglGetProcAddress("glDrawElementsInstancedANGLE");
            glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOREXTPROC)eglGetProcAddress("glVertexAttribDivisorANGLE");

            if ((glDrawArraysInstanced != NULL) && (glDrawElementsInstanced != NULL) && (glVertexAttribDivisor != NULL)) RLGL.ExtSupported.instancing = true;
        }
        else
        {
            if ((strcmp(extList[i], (const char *)"GL_EXT_draw_instanced") == 0) &&     // Standard EXT
                (strcmp(extList[i], (const char *)"GL_EXT_instanced_arrays") == 0))
            {
                glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)eglGetProcAddress("glDrawArraysInstancedEXT");
                glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)eglGetProcAddress("glDrawElementsInstancedEXT");
                glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOREXTPROC)eglGetProcAddress("glVertexAttribDivisorEXT");

                if ((glDrawArraysInstanced != NULL) && (glDrawElementsInstanced != NULL) && (glVertexAttribDivisor != NULL)) RLGL.ExtSupported.instancing = true;
            }
        }

        // Check NPOT textures support
        // NOTE: Only check on OpenGL ES, OpenGL 3.3 has NPOT textures full support as core feature
        if (strcmp(extList[i], (const char *)"GL_OES_texture_npot") == 0) RLGL.ExtSupported.texNPOT = true;

        // Check texture float support
        if (strcmp(extList[i], (const char *)"GL_OES_texture_float") == 0) RLGL.ExtSupported.texFloat32 = true;

        // Check depth texture support
        if ((strcmp(extList[i], (const char *)"GL_OES_depth_texture") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_depth_texture") == 0)) RLGL.ExtSupported.texDepth = true;

        if (strcmp(extList[i], (const char *)"GL_OES_depth24") == 0) RLGL.ExtSupported.maxDepthBits = 24;
        if (strcmp(extList[i], (const char *)"GL_OES_depth32") == 0) RLGL.ExtSupported.maxDepthBits = 32;

        // Check texture compression support: DXT
        if ((strcmp(extList[i], (const char *)"GL_EXT_texture_compression_s3tc") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_s3tc") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBKIT_WEBGL_compressed_texture_s3tc") == 0)) RLGL.ExtSupported.texCompDXT = true;

        // Check texture compression support: ETC1
        if ((strcmp(extList[i], (const char *)"GL_OES_compressed_ETC1_RGB8_texture") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_etc1") == 0)) RLGL.ExtSupported.texCompETC1 = true;

        // Check texture compression support: ETC2/EAC
        if (strcmp(extList[i], (const char *)"GL_ARB_ES3_compatibility") == 0) RLGL.ExtSupported.texCompETC2 = true;

        // Check texture compression support: PVR
        if (strcmp(extList[i], (const char *)"GL_IMG_texture_compression_pvrtc") == 0) RLGL.ExtSupported.texCompPVRT = true;

        // Check texture compression support: ASTC
        if (strcmp(extList[i], (const char *)"GL_KHR_texture_compression_astc_hdr") == 0) RLGL.ExtSupported.texCompASTC = true;

        // Check anisotropic texture filter support
        if (strcmp(extList[i], (const char *)"GL_EXT_texture_filter_anisotropic") == 0) RLGL.ExtSupported.texAnisoFilter = true;

        // Check clamp mirror wrap mode support
        if (strcmp(extList[i], (const char *)"GL_EXT_texture_mirror_clamp") == 0) RLGL.ExtSupported.texMirrorClamp = true;
    }

    // Free extensions pointers
    RL_FREE(extList);
    RL_FREE(extensionsDup);    // Duplicated string must be deallocated
#endif  // GRAPHICS_API_OPENGL_ES2

    // Check OpenGL information and capabilities
    //------------------------------------------------------------------------------
    // Show current OpenGL and GLSL version
    TRACELOG(LOG_INFO, "GL: OpenGL device information:");
    TRACELOG(LOG_INFO, "    > Vendor:   %s", glGetString(GL_VENDOR));
    TRACELOG(LOG_INFO, "    > Renderer: %s", glGetString(GL_RENDERER));
    TRACELOG(LOG_INFO, "    > Version:  %s", glGetString(GL_VERSION));
    TRACELOG(LOG_INFO, "    > GLSL:     %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: Anisotropy levels capability is an extension
    #ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
    #endif
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &RLGL.ExtSupported.maxAnisotropyLevel);

#if defined(SUPPORT_GL_DETAILS_INFO)
    // Show some OpenGL GPU capabilities
    TRACELOG(LOG_INFO, "GL: OpenGL capabilities:");
    GLint capability = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_TEXTURE_SIZE: %i", capability);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_CUBE_MAP_TEXTURE_SIZE: %i", capability);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_TEXTURE_IMAGE_UNITS: %i", capability);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_VERTEX_ATTRIBS: %i", capability);
    #if !defined(GRAPHICS_API_OPENGL_ES2)
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_UNIFORM_BLOCK_SIZE: %i", capability);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_DRAW_BUFFERS: %i", capability);
    if (RLGL.ExtSupported.texAnisoFilter) TRACELOG(LOG_INFO, "    GL_MAX_TEXTURE_MAX_ANISOTROPY: %.0f", RLGL.ExtSupported.maxAnisotropyLevel);
    #endif
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &capability);
    TRACELOG(LOG_INFO, "    GL_NUM_COMPRESSED_TEXTURE_FORMATS: %i", capability);
    GLint format[32] = { 0 };
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, format);
    for (int i = 0; i < capability; i++) TRACELOG(LOG_INFO, "        %s", rlGetCompressedFormatName(format[i]));

    /*
    // Following capabilities are only supported by OpenGL 4.3 or greater
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_VERTEX_ATTRIB_BINDINGS: %i", capability);
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &capability);
    TRACELOG(LOG_INFO, "    GL_MAX_UNIFORM_LOCATIONS: %i", capability);
    */
#else   // SUPPORT_GL_DETAILS_INFO

    // Show some basic info about GL supported features
    #if defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) TRACELOG(LOG_INFO, "GL: VAO extension detected, VAO functions loaded successfully");
    else TRACELOG(LOG_WARNING, "GL: VAO extension not found, VAO not supported");
    if (RLGL.ExtSupported.texNPOT) TRACELOG(LOG_INFO, "GL: NPOT textures extension detected, full NPOT textures supported");
    else TRACELOG(LOG_WARNING, "GL: NPOT textures extension not found, limited NPOT support (no-mipmaps, no-repeat)");
    #endif
    if (RLGL.ExtSupported.texCompDXT) TRACELOG(LOG_INFO, "GL: DXT compressed textures supported");
    if (RLGL.ExtSupported.texCompETC1) TRACELOG(LOG_INFO, "GL: ETC1 compressed textures supported");
    if (RLGL.ExtSupported.texCompETC2) TRACELOG(LOG_INFO, "GL: ETC2/EAC compressed textures supported");
    if (RLGL.ExtSupported.texCompPVRT) TRACELOG(LOG_INFO, "GL: PVRT compressed textures supported");
    if (RLGL.ExtSupported.texCompASTC) TRACELOG(LOG_INFO, "GL: ASTC compressed textures supported");
#endif  // SUPPORT_GL_DETAILS_INFO

#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2
}

// Returns current OpenGL version
int rlGetVersion(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    return OPENGL_11;
#endif
#if defined(GRAPHICS_API_OPENGL_21)
    #if defined(__APPLE__)
        return OPENGL_33;           // NOTE: Force OpenGL 3.3 on OSX
    #else
        return OPENGL_21;
    #endif
#elif defined(GRAPHICS_API_OPENGL_33)
    return OPENGL_33;
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    return OPENGL_ES_20;
#endif
}

// Get default framebuffer width
int rlGetFramebufferWidth(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    return RLGL.State.framebufferWidth;
#else
    return 0;
#endif
}

// Get default framebuffer height
int rlGetFramebufferHeight(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    return RLGL.State.framebufferHeight;
#else
    return 0;
#endif
}

// Get default internal shader (simple texture + tint color)
Shader rlGetShaderDefault(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    return RLGL.State.defaultShader;
#else
    Shader shader = { 0 };
    return shader;
#endif
}

// Get default internal texture (white texture)
Texture2D rlGetTextureDefault(void)
{
    Texture2D texture = { 0 };
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    texture.id = RLGL.State.defaultTextureId;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
#endif
    return texture;
}

// Render batch management
//------------------------------------------------------------------------------------------------
// Load render batch
RenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements)
{
    RenderBatch batch = { 0 };

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Initialize CPU (RAM) vertex buffers (position, texcoord, color data and indexes)
    //--------------------------------------------------------------------------------------------
    batch.vertexBuffer = (VertexBuffer *)RL_MALLOC(sizeof(VertexBuffer)*numBuffers);

    for (int i = 0; i < numBuffers; i++)
    {
        batch.vertexBuffer[i].elementsCount = bufferElements;

        batch.vertexBuffer[i].vertices = (float *)RL_MALLOC(bufferElements*3*4*sizeof(float));        // 3 float by vertex, 4 vertex by quad
        batch.vertexBuffer[i].texcoords = (float *)RL_MALLOC(bufferElements*2*4*sizeof(float));       // 2 float by texcoord, 4 texcoord by quad
        batch.vertexBuffer[i].colors = (unsigned char *)RL_MALLOC(bufferElements*4*4*sizeof(unsigned char));   // 4 float by color, 4 colors by quad
#if defined(GRAPHICS_API_OPENGL_33)
        batch.vertexBuffer[i].indices = (unsigned int *)RL_MALLOC(bufferElements*6*sizeof(unsigned int));      // 6 int by quad (indices)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
        batch.vertexBuffer[i].indices = (unsigned short *)RL_MALLOC(bufferElements*6*sizeof(unsigned short));  // 6 int by quad (indices)
#endif

        for (int j = 0; j < (3*4*bufferElements); j++) batch.vertexBuffer[i].vertices[j] = 0.0f;
        for (int j = 0; j < (2*4*bufferElements); j++) batch.vertexBuffer[i].texcoords[j] = 0.0f;
        for (int j = 0; j < (4*4*bufferElements); j++) batch.vertexBuffer[i].colors[j] = 0;

        int k = 0;

        // Indices can be initialized right now
        for (int j = 0; j < (6*bufferElements); j += 6)
        {
            batch.vertexBuffer[i].indices[j] = 4*k;
            batch.vertexBuffer[i].indices[j + 1] = 4*k + 1;
            batch.vertexBuffer[i].indices[j + 2] = 4*k + 2;
            batch.vertexBuffer[i].indices[j + 3] = 4*k;
            batch.vertexBuffer[i].indices[j + 4] = 4*k + 2;
            batch.vertexBuffer[i].indices[j + 5] = 4*k + 3;

            k++;
        }

        batch.vertexBuffer[i].vCounter = 0;
        batch.vertexBuffer[i].tcCounter = 0;
        batch.vertexBuffer[i].cCounter = 0;
    }

    TRACELOG(LOG_INFO, "RLGL: Render batch vertex buffers loaded successfully in RAM (CPU)");
    //--------------------------------------------------------------------------------------------

    // Upload to GPU (VRAM) vertex data and initialize VAOs/VBOs
    //--------------------------------------------------------------------------------------------
    for (int i = 0; i < numBuffers; i++)
    {
        if (RLGL.ExtSupported.vao)
        {
            // Initialize Quads VAO
            glGenVertexArrays(1, &batch.vertexBuffer[i].vaoId);
            glBindVertexArray(batch.vertexBuffer[i].vaoId);
        }

        // Quads - Vertex buffers binding and attributes enable
        // Vertex position buffer (shader-location = 0)
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[0]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*3*4*sizeof(float), batch.vertexBuffer[i].vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION]);
        glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);

        // Vertex texcoord buffer (shader-location = 1)
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[1]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[1]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*2*4*sizeof(float), batch.vertexBuffer[i].texcoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);
        glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);

        // Vertex color buffer (shader-location = 3)
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[2]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[2]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*4*4*sizeof(unsigned char), batch.vertexBuffer[i].colors, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR]);
        glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        // Fill index buffer
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[3]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[3]);
#if defined(GRAPHICS_API_OPENGL_33)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElements*6*sizeof(int), batch.vertexBuffer[i].indices, GL_STATIC_DRAW);
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElements*6*sizeof(short), batch.vertexBuffer[i].indices, GL_STATIC_DRAW);
#endif
    }

    TRACELOG(LOG_INFO, "RLGL: Render batch vertex buffers loaded successfully in VRAM (GPU)");

    // Unbind the current VAO
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    //--------------------------------------------------------------------------------------------

    // Init draw calls tracking system
    //--------------------------------------------------------------------------------------------
    batch.draws = (DrawCall *)RL_MALLOC(DEFAULT_BATCH_DRAWCALLS*sizeof(DrawCall));

    for (int i = 0; i < DEFAULT_BATCH_DRAWCALLS; i++)
    {
        batch.draws[i].mode = RL_QUADS;
        batch.draws[i].vertexCount = 0;
        batch.draws[i].vertexAlignment = 0;
        //batch.draws[i].vaoId = 0;
        //batch.draws[i].shaderId = 0;
        batch.draws[i].textureId = RLGL.State.defaultTextureId;
        //batch.draws[i].RLGL.State.projection = MatrixIdentity();
        //batch.draws[i].RLGL.State.modelview = MatrixIdentity();
    }

    batch.buffersCount = numBuffers;    // Record buffer count
    batch.drawsCounter = 1;             // Reset draws counter
    batch.currentDepth = -1.0f;         // Reset depth value
    //--------------------------------------------------------------------------------------------
#endif

    return batch;
}

// Unload default internal buffers vertex data from CPU and GPU
void rlUnloadRenderBatch(RenderBatch batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Unbind everything
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Unload all vertex buffers data
    for (int i = 0; i < batch.buffersCount; i++)
    {
        // Delete VBOs from GPU (VRAM)
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[0]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[1]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[2]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[3]);

        // Delete VAOs from GPU (VRAM)
        if (RLGL.ExtSupported.vao) glDeleteVertexArrays(1, &batch.vertexBuffer[i].vaoId);

        // Free vertex arrays memory from CPU (RAM)
        RL_FREE(batch.vertexBuffer[i].vertices);
        RL_FREE(batch.vertexBuffer[i].texcoords);
        RL_FREE(batch.vertexBuffer[i].colors);
        RL_FREE(batch.vertexBuffer[i].indices);
    }

    // Unload arrays
    RL_FREE(batch.vertexBuffer);
    RL_FREE(batch.draws);
#endif
}

// Draw render batch
// NOTE: We require a pointer to reset batch and increase current buffer (multi-buffer)
void rlDrawRenderBatch(RenderBatch *batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Update batch vertex buffers
    //------------------------------------------------------------------------------------------------------------
    // NOTE: If there is not vertex data, buffers doesn't need to be updated (vertexCount > 0)
    // TODO: If no data changed on the CPU arrays --> No need to re-update GPU arrays (change flag required)
    if (batch->vertexBuffer[batch->currentBuffer].vCounter > 0)
    {
        // Activate elements VAO
        if (RLGL.ExtSupported.vao) glBindVertexArray(batch->vertexBuffer[batch->currentBuffer].vaoId);

        // Vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexBuffer[batch->currentBuffer].vCounter*3*sizeof(float), batch->vertexBuffer[batch->currentBuffer].vertices);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*batch->vertexBuffer[batch->currentBuffer].elementsCount, batch->vertexBuffer[batch->currentBuffer].vertices, GL_DYNAMIC_DRAW);  // Update all buffer

        // Texture coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexBuffer[batch->currentBuffer].vCounter*2*sizeof(float), batch->vertexBuffer[batch->currentBuffer].texcoords);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*batch->vertexBuffer[batch->currentBuffer].elementsCount, batch->vertexBuffer[batch->currentBuffer].texcoords, GL_DYNAMIC_DRAW); // Update all buffer

        // Colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexBuffer[batch->currentBuffer].vCounter*4*sizeof(unsigned char), batch->vertexBuffer[batch->currentBuffer].colors);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*batch->vertexBuffer[batch->currentBuffer].elementsCount, batch->vertexBuffer[batch->currentBuffer].colors, GL_DYNAMIC_DRAW);    // Update all buffer

        // NOTE: glMapBuffer() causes sync issue.
        // If GPU is working with this buffer, glMapBuffer() will wait(stall) until GPU to finish its job.
        // To avoid waiting (idle), you can call first glBufferData() with NULL pointer before glMapBuffer().
        // If you do that, the previous data in PBO will be discarded and glMapBuffer() returns a new
        // allocated pointer immediately even if GPU is still working with the previous data.

        // Another option: map the buffer object into client's memory
        // Probably this code could be moved somewhere else...
        // batch->vertexBuffer[batch->currentBuffer].vertices = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
        // if (batch->vertexBuffer[batch->currentBuffer].vertices)
        // {
            // Update vertex data
        // }
        // glUnmapBuffer(GL_ARRAY_BUFFER);

        // Unbind the current VAO
        if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    }
    //------------------------------------------------------------------------------------------------------------

    // Draw batch vertex buffers (considering VR stereo if required)
    //------------------------------------------------------------------------------------------------------------
    Matrix matProjection = RLGL.State.projection;
    Matrix matModelView = RLGL.State.modelview;

    int eyesCount = 1;
    if (RLGL.State.stereoRender) eyesCount = 2;

    for (int eye = 0; eye < eyesCount; eye++)
    {
        if (eyesCount == 2)
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*RLGL.State.framebufferWidth/2, 0, RLGL.State.framebufferWidth/2, RLGL.State.framebufferHeight);

            // Set current eye view offset to modelview matrix
            rlSetMatrixModelview(MatrixMultiply(matModelView, RLGL.State.viewOffsetStereo[eye]));
            // Set current eye projection matrix
            rlSetMatrixProjection(RLGL.State.projectionStereo[eye]);
        }

        // Draw buffers
        if (batch->vertexBuffer[batch->currentBuffer].vCounter > 0)
        {
            // Set current shader and upload current MVP matrix
            glUseProgram(RLGL.State.currentShader.id);

            // Create modelview-projection matrix and upload to shader
            Matrix matMVP = MatrixMultiply(RLGL.State.modelview, RLGL.State.projection);
            glUniformMatrix4fv(RLGL.State.currentShader.locs[SHADER_LOC_MATRIX_MVP], 1, false, MatrixToFloat(matMVP));

            if (RLGL.ExtSupported.vao) glBindVertexArray(batch->vertexBuffer[batch->currentBuffer].vaoId);
            else
            {
                // Bind vertex attrib: position (shader-location = 0)
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[0]);
                glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION]);

                // Bind vertex attrib: texcoord (shader-location = 1)
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[1]);
                glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

                // Bind vertex attrib: color (shader-location = 3)
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[2]);
                glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR]);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[3]);
            }

            // Setup some default shader values
            glUniform4f(RLGL.State.currentShader.locs[SHADER_LOC_COLOR_DIFFUSE], 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform1i(RLGL.State.currentShader.locs[SHADER_LOC_MAP_DIFFUSE], 0);  // Active default sampler2D: texture0

            // Activate additional sampler textures
            // Those additional textures will be common for all draw calls of the batch
            for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++)
            {
                if (RLGL.State.activeTextureId[i] > 0)
                {
                    glActiveTexture(GL_TEXTURE0 + 1 + i);
                    glBindTexture(GL_TEXTURE_2D, RLGL.State.activeTextureId[i]);
                }
            }

            // Activate default sampler2D texture0 (one texture is always active for default batch shader)
            // NOTE: Batch system accumulates calls by texture0 changes, additional textures are enabled for all the draw calls
            glActiveTexture(GL_TEXTURE0);

            for (int i = 0, vertexOffset = 0; i < batch->drawsCounter; i++)
            {
                // Bind current draw call texture, activated as GL_TEXTURE0 and binded to sampler2D texture0 by default
                glBindTexture(GL_TEXTURE_2D, batch->draws[i].textureId);

                if ((batch->draws[i].mode == RL_LINES) || (batch->draws[i].mode == RL_TRIANGLES)) glDrawArrays(batch->draws[i].mode, vertexOffset, batch->draws[i].vertexCount);
                else
                {
#if defined(GRAPHICS_API_OPENGL_33)
                    // We need to define the number of indices to be processed: quadsCount*6
                    // NOTE: The final parameter tells the GPU the offset in bytes from the
                    // start of the index buffer to the location of the first index to process
                    glDrawElements(GL_TRIANGLES, batch->draws[i].vertexCount/4*6, GL_UNSIGNED_INT, (GLvoid *)(vertexOffset/4*6*sizeof(GLuint)));
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
                    glDrawElements(GL_TRIANGLES, batch->draws[i].vertexCount/4*6, GL_UNSIGNED_SHORT, (GLvoid *)(vertexOffset/4*6*sizeof(GLushort)));
#endif
                }

                vertexOffset += (batch->draws[i].vertexCount + batch->draws[i].vertexAlignment);
            }

            if (!RLGL.ExtSupported.vao)
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

            glBindTexture(GL_TEXTURE_2D, 0);    // Unbind textures
        }

        if (RLGL.ExtSupported.vao) glBindVertexArray(0); // Unbind VAO

        glUseProgram(0);    // Unbind shader program
    }
    //------------------------------------------------------------------------------------------------------------

    // Reset batch buffers
    //------------------------------------------------------------------------------------------------------------
    // Reset vertex counters for next frame
    batch->vertexBuffer[batch->currentBuffer].vCounter = 0;
    batch->vertexBuffer[batch->currentBuffer].tcCounter = 0;
    batch->vertexBuffer[batch->currentBuffer].cCounter = 0;

    // Reset depth for next draw
    batch->currentDepth = -1.0f;

    // Restore projection/modelview matrices
    RLGL.State.projection = matProjection;
    RLGL.State.modelview = matModelView;

    // Reset RLGL.currentBatch->draws array
    for (int i = 0; i < DEFAULT_BATCH_DRAWCALLS; i++)
    {
        batch->draws[i].mode = RL_QUADS;
        batch->draws[i].vertexCount = 0;
        batch->draws[i].textureId = RLGL.State.defaultTextureId;
    }

    // Reset active texture units for next batch
    for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++) RLGL.State.activeTextureId[i] = 0;

    // Reset draws counter to one draw for the batch
    batch->drawsCounter = 1;
    //------------------------------------------------------------------------------------------------------------

    // Change to next buffer in the list (in case of multi-buffering)
    batch->currentBuffer++;
    if (batch->currentBuffer >= batch->buffersCount) batch->currentBuffer = 0;
#endif
}

// Set the active render batch for rlgl
void rlSetRenderBatchActive(RenderBatch *batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlDrawRenderBatch(RLGL.currentBatch);

    if (batch != NULL) RLGL.currentBatch = batch;
    else RLGL.currentBatch = &RLGL.defaultBatch;
#endif
}

// Update and draw internal render batch
void rlDrawRenderBatchActive(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlDrawRenderBatch(RLGL.currentBatch);    // NOTE: Stereo rendering is checked inside
#endif
}

// Check internal buffer overflow for a given number of vertex
// and force a RenderBatch draw call if required
bool rlCheckRenderBatchLimit(int vCount)
{
    bool overflow = false;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + vCount) >=
        (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4))
    {
        overflow = true;
        rlDrawRenderBatch(RLGL.currentBatch);    // NOTE: Stereo rendering is checked inside
    }
#endif

    return overflow;
}

// Textures data management
//-----------------------------------------------------------------------------------------
// Convert image data to OpenGL texture (returns OpenGL valid Id)
unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount)
{
    glBindTexture(GL_TEXTURE_2D, 0);    // Free any old binding

    unsigned int id = 0;

    // Check texture format support by OpenGL 1.1 (compressed textures not supported)
#if defined(GRAPHICS_API_OPENGL_11)
    if (format >= PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        TRACELOG(LOG_WARNING, "GL: OpenGL 1.1 does not support GPU compressed texture formats");
        return id;
    }
#else
    if ((!RLGL.ExtSupported.texCompDXT) && ((format == PIXELFORMAT_COMPRESSED_DXT1_RGB) || (format == PIXELFORMAT_COMPRESSED_DXT1_RGBA) ||
        (format == PIXELFORMAT_COMPRESSED_DXT3_RGBA) || (format == PIXELFORMAT_COMPRESSED_DXT5_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: DXT compressed texture format not supported");
        return id;
    }
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((!RLGL.ExtSupported.texCompETC1) && (format == PIXELFORMAT_COMPRESSED_ETC1_RGB))
    {
        TRACELOG(LOG_WARNING, "GL: ETC1 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompETC2) && ((format == PIXELFORMAT_COMPRESSED_ETC2_RGB) || (format == PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: ETC2 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompPVRT) && ((format == PIXELFORMAT_COMPRESSED_PVRT_RGB) || (format == PIXELFORMAT_COMPRESSED_PVRT_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: PVRT compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompASTC) && ((format == PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA) || (format == PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: ASTC compressed texture format not supported");
        return id;
    }
#endif
#endif  // GRAPHICS_API_OPENGL_11

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &id);              // Generate texture id

    glBindTexture(GL_TEXTURE_2D, id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;          // Mipmap data offset

    // Load the different mipmap levels
    for (int i = 0; i < mipmapCount; i++)
    {
        unsigned int mipSize = rlGetPixelDataSize(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

        TRACELOGD("TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != -1)
        {
            if (format < PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, (unsigned char *)data + mipOffset);
#if !defined(GRAPHICS_API_OPENGL_11)
            else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, (unsigned char *)data + mipOffset);
#endif

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize;

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    // Texture parameters configuration
    // NOTE: glTexParameteri does NOT affect texture uploading, just the way it's used
#if defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so CLAMP_TO_EDGE must be used
    if (RLGL.ExtSupported.texNPOT)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
    }
    else
    {
        // NOTE: If using negative texture coordinates (LoadOBJ()), it does not work!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       // Set texture to clamp on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       // Set texture to clamp on y-axis
    }
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
#endif

    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

#if defined(GRAPHICS_API_OPENGL_33)
    if (mipmapCount > 1)
    {
        // Activate Trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
#endif

    // At this point we have the texture loaded in GPU and texture parameters configured

    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Texture loaded successfully (%ix%i - %i mipmaps)", id, width, height, mipmapCount);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load texture");

    return id;
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture/WEBGL_depth_texture extensions
unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // In case depth textures not supported, we force renderbuffer usage
    if (!RLGL.ExtSupported.texDepth) useRenderBuffer = true;

    // NOTE: We let the implementation to choose the best bit-depth
    // Possible formats: GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32 and GL_DEPTH_COMPONENT32F
    unsigned int glInternalFormat = GL_DEPTH_COMPONENT;

#if defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.maxDepthBits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
    else if (RLGL.ExtSupported.maxDepthBits == 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
    else glInternalFormat = GL_DEPTH_COMPONENT16;
#endif

    if (!useRenderBuffer && RLGL.ExtSupported.texDepth)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        TRACELOG(LOG_INFO, "TEXTURE: Depth texture loaded successfully");
    }
    else
    {
        // Create the renderbuffer that will serve as the depth attachment for the framebuffer
        // NOTE: A renderbuffer is simpler than a texture and could offer better performance on embedded devices
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Depth renderbuffer loaded successfully (%i bits)", id, (RLGL.ExtSupported.maxDepthBits >= 24)? RLGL.ExtSupported.maxDepthBits : 16);
    }
#endif

    return id;
}

// Load texture cubemap
// NOTE: Cubemap data is expected to be 6 images in a single data array (one after the other),
// expected the following convention: +X, -X, +Y, -Y, +Z, -Z
unsigned int rlLoadTextureCubemap(void *data, int size, int format)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int dataSize = rlGetPixelDataSize(size, size, format);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if (glInternalFormat != -1)
    {
        // Load cubemap faces
        for (unsigned int i = 0; i < 6; i++)
        {
            if (data == NULL)
            {
                if (format < PIXELFORMAT_COMPRESSED_DXT1_RGB)
                {
                    if (format == PIXELFORMAT_UNCOMPRESSED_R32G32B32)
                    {
                        // Instead of using a sized internal texture format (GL_RGB16F, GL_RGB32F), we let the driver to choose the better format for us (GL_RGB)
                        if (RLGL.ExtSupported.texFloat32) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, NULL);
                        else TRACELOG(LOG_WARNING, "TEXTURES: Cubemap requested format not supported");
                    }
                    else if ((format == PIXELFORMAT_UNCOMPRESSED_R32) || (format == PIXELFORMAT_UNCOMPRESSED_R32G32B32A32)) TRACELOG(LOG_WARNING, "TEXTURES: Cubemap requested format not supported");
                    else glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, NULL);
                }
                else TRACELOG(LOG_WARNING, "TEXTURES: Empty cubemap creation does not support compressed format");
            }
            else
            {
                if (format < PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, (unsigned char *)data + i*dataSize);
                else glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, dataSize, (unsigned char *)data + i*dataSize);
            }

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }
    }

    // Set cubemap texture sampling parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GRAPHICS_API_OPENGL_33)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  // Flag not supported on OpenGL ES 2.0
#endif

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif

    if (id > 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Cubemap texture loaded successfully (%ix%i)", id, size, size);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load cubemap texture");

    return id;
}

// Update already loaded texture in GPU with new data
// NOTE: We don't know safely if internal texture format is the expected one...
void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data)
{
    glBindTexture(GL_TEXTURE_2D, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if ((glInternalFormat != -1) && (format < PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, glFormat, glType, (unsigned char *)data);
    }
    else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to update for current texture format (%i)", id, format);
}

// Get OpenGL internal formats and data type from raylib PixelFormat
void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType)
{
    *glInternalFormat = -1;
    *glFormat = -1;
    *glType = -1;

    switch (format)
    {
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_21) || defined(GRAPHICS_API_OPENGL_ES2)
        // NOTE: on OpenGL ES 2.0 (WebGL), internalFormat must match format and options allowed are: GL_LUMINANCE, GL_RGB, GL_RGBA
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_LUMINANCE_ALPHA; *glFormat = GL_LUMINANCE_ALPHA; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        #if !defined(GRAPHICS_API_OPENGL_11)
        case PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_FLOAT; break;   // NOTE: Requires extension OES_texture_float
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_FLOAT; break;         // NOTE: Requires extension OES_texture_float
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;    // NOTE: Requires extension OES_texture_float
        #endif
    #elif defined(GRAPHICS_API_OPENGL_33)
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
    #endif
    #if !defined(GRAPHICS_API_OPENGL_11)
        case PIXELFORMAT_COMPRESSED_DXT1_RGB: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        case PIXELFORMAT_COMPRESSED_ETC1_RGB: if (RLGL.ExtSupported.texCompETC1) *glInternalFormat = GL_ETC1_RGB8_OES; break;                      // NOTE: Requires OpenGL ES 2.0 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_ETC2_RGB: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break;               // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break;     // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_PVRT_RGB: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;    // NOTE: Requires PowerVR GPU
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;  // NOTE: Requires PowerVR GPU
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
    #endif
        default: TRACELOG(LOG_WARNING, "TEXTURE: Current format not supported (%i)", format); break;
    }
}

// Unload texture from GPU memory
void rlUnloadTexture(unsigned int id)
{
    glDeleteTextures(1, &id);
}

// Generate mipmap data for selected texture
void rlGenerateMipmaps(Texture2D *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Check if texture is power-of-two (POT)
    bool texIsPOT = false;

    if (((texture->width > 0) && ((texture->width & (texture->width - 1)) == 0)) &&
        ((texture->height > 0) && ((texture->height & (texture->height - 1)) == 0))) texIsPOT = true;

#if defined(GRAPHICS_API_OPENGL_11)
    if (texIsPOT)
    {
        // WARNING: Manual mipmap generation only works for RGBA 32bit textures!
        if (texture->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        {
            // Retrieve texture data from VRAM
            void *texData = rlReadTexturePixels(*texture);

            // NOTE: Texture data size is reallocated to fit mipmaps data
            // NOTE: CPU mipmap generation only supports RGBA 32bit data
            int mipmapCount = rlGenerateMipmapsData(texData, texture->width, texture->height);

            int size = texture->width*texture->height*4;
            int offset = size;

            int mipWidth = texture->width/2;
            int mipHeight = texture->height/2;

            // Load the mipmaps
            for (int level = 1; level < mipmapCount; level++)
            {
                glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, mipWidth, mipHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)texData + offset);

                size = mipWidth*mipHeight*4;
                offset += size;

                mipWidth /= 2;
                mipHeight /= 2;
            }

            texture->mipmaps = mipmapCount + 1;
            RL_FREE(texData); // Once mipmaps have been generated and data has been uploaded to GPU VRAM, we can discard RAM data

            TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Mipmaps generated manually on CPU side, total: %i", texture->id, texture->mipmaps);
        }
        else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps for provided texture format", texture->id);
    }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((texIsPOT) || (RLGL.ExtSupported.texNPOT))
    {
        //glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);   // Hint for mipmaps generation algorythm: GL_FASTEST, GL_NICEST, GL_DONT_CARE
        glGenerateMipmap(GL_TEXTURE_2D);    // Generate mipmaps automatically

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);   // Activate Trilinear filtering for mipmaps

        #define MIN(a,b) (((a)<(b))?(a):(b))
        #define MAX(a,b) (((a)>(b))?(a):(b))

        texture->mipmaps =  1 + (int)floor(log(MAX(texture->width, texture->height))/log(2));
        TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Mipmaps generated automatically, total: %i", texture->id, texture->mipmaps);
    }
#endif
    else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps", texture->id);

    glBindTexture(GL_TEXTURE_2D, 0);
}


// Read texture pixel data
void *rlReadTexturePixels(Texture2D texture)
{
    void *pixels = NULL;

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // NOTE: Using texture.id, we can retrieve some texture info (but not on OpenGL ES 2.0)
    // Possible texture info: GL_TEXTURE_RED_SIZE, GL_TEXTURE_GREEN_SIZE, GL_TEXTURE_BLUE_SIZE, GL_TEXTURE_ALPHA_SIZE
    //int width, height, format;
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

    // NOTE: Each row written to or read from by OpenGL pixel operations like glGetTexImage are aligned to a 4 byte boundary by default, which may add some padding.
    // Use glPixelStorei to modify padding with the GL_[UN]PACK_ALIGNMENT setting.
    // GL_PACK_ALIGNMENT affects operations that read from OpenGL memory (glReadPixels, glGetTexImage, etc.)
    // GL_UNPACK_ALIGNMENT affects operations that write to OpenGL memory (glTexImage, etc.)
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(texture.format, &glInternalFormat, &glFormat, &glType);
    unsigned int size = rlGetPixelDataSize(texture.width, texture.height, texture.format);

    if ((glInternalFormat != -1) && (texture.format < PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        pixels = RL_MALLOC(size);
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, glType, pixels);
    }
    else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Data retrieval not suported for pixel format (%i)", texture.id, texture.format);

    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    // glGetTexImage() is not available on OpenGL ES 2.0
    // Texture width and height are required on OpenGL ES 2.0. There is no way to get it from texture id.
    // Two possible Options:
    // 1 - Bind texture to color fbo attachment and glReadPixels()
    // 2 - Create an fbo, activate it, render quad with texture, glReadPixels()
    // We are using Option 1, just need to care for texture format on retrieval
    // NOTE: This behaviour could be conditioned by graphic driver...
    unsigned int fboId = rlLoadFramebuffer(texture.width, texture.height);

    // TODO: Create depth texture/renderbuffer for fbo?

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach our texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

    // We read data as RGBA because FBO texture is configured as RGBA, despite binding another texture format
    pixels = (unsigned char *)RL_MALLOC(rlGetPixelDataSize(texture.width, texture.height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));
    glReadPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clean up temporal fbo
    rlUnloadFramebuffer(fboId);
#endif

    return pixels;
}


// Read screen pixel data (color buffer)
unsigned char *rlReadScreenPixels(int width, int height)
{
    unsigned char *screenData = (unsigned char *)RL_CALLOC(width*height*4, sizeof(unsigned char));

    // NOTE 1: glReadPixels returns image flipped vertically -> (0,0) is the bottom left corner of the framebuffer
    // NOTE 2: We are getting alpha channel! Be careful, it can be transparent if not cleared properly!
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);

    // Flip image vertically!
    unsigned char *imgData = (unsigned char *)RL_MALLOC(width*height*4*sizeof(unsigned char));

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < (width*4); x++)
        {
            imgData[((height - 1) - y)*width*4 + x] = screenData[(y*width*4) + x];  // Flip line

            // Set alpha component value to 255 (no trasparent image retrieval)
            // NOTE: Alpha value has already been applied to RGB in framebuffer, we don't need it!
            if (((x + 1)%4) == 0) imgData[((height - 1) - y)*width*4 + x] = 255;
        }
    }

    RL_FREE(screenData);

    return imgData;     // NOTE: image data should be freed
}

// Framebuffer management (fbo)
//-----------------------------------------------------------------------------------------
// Load a framebuffer to be used for rendering
// NOTE: No textures attached
unsigned int rlLoadFramebuffer(int width, int height)
{
    unsigned int fboId = 0;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glGenFramebuffers(1, &fboId);       // Create the framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // Unbind any framebuffer
#endif

    return fboId;
}

// Attach color buffer texture to an fbo (unloads previous attachment)
// NOTE: Attach type: 0-Color, 1-Depth renderbuffer, 2-Depth texture
void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    switch (attachType)
    {
        case RL_ATTACHMENT_COLOR_CHANNEL0:
        case RL_ATTACHMENT_COLOR_CHANNEL1:
        case RL_ATTACHMENT_COLOR_CHANNEL2:
        case RL_ATTACHMENT_COLOR_CHANNEL3:
        case RL_ATTACHMENT_COLOR_CHANNEL4:
        case RL_ATTACHMENT_COLOR_CHANNEL5:
        case RL_ATTACHMENT_COLOR_CHANNEL6:
        case RL_ATTACHMENT_COLOR_CHANNEL7:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_RENDERBUFFER, texId);
            else if (texType >= RL_ATTACHMENT_CUBEMAP_POSITIVE_X) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + texType, texId, mipLevel);

        } break;
        case RL_ATTACHMENT_DEPTH:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texId);

        } break;
        case RL_ATTACHMENT_STENCIL:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, texId);

        } break;
        default: break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Verify render texture is complete
bool rlFramebufferComplete(unsigned int id)
{
    bool result = false;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer is unsupported", id); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete attachment", id); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete dimensions", id); break;
#endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has a missing attachment", id); break;
            default: break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

    return result;
}

// Unload framebuffer from GPU memory
// NOTE: All attached textures/cubemaps/renderbuffers are also deleted
void rlUnloadFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)

    // Query depth attachment to automatically delete texture/renderbuffer
    int depthType = 0, depthId = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, id);   // Bind framebuffer to query depth texture type
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthId);

    unsigned int depthIdU = (unsigned int)depthId;
    if (depthType == GL_RENDERBUFFER) glDeleteRenderbuffers(1, &depthIdU);
    else if (depthType == GL_RENDERBUFFER) glDeleteTextures(1, &depthIdU);

    // NOTE: If a texture object is deleted while its image is attached to the *currently bound* framebuffer,
    // the texture image is automatically detached from the currently bound framebuffer.

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &id);

    TRACELOG(LOG_INFO, "FBO: [ID %i] Unloaded framebuffer from VRAM (GPU)", id);
#endif
}

// Vertex data management
//-----------------------------------------------------------------------------------------
// Load a new attributes buffer
unsigned int rlLoadVertexBuffer(void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}

// Load a new attributes element buffer
unsigned int rlLoadVertexBufferElement(void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}

void rlEnableVertexBuffer(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, id);
#endif
}

void rlDisableVertexBuffer(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

void rlEnableVertexBufferElement(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
#endif
}

void rlDisableVertexBufferElement(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

// Update GPU buffer with new data
// NOTE: dataSize and offset must be provided in bytes
void rlUpdateVertexBuffer(int bufferId, void *data, int dataSize, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferSubData(GL_ARRAY_BUFFER, offset, dataSize, data);
#endif
}

bool rlEnableVertexArray(unsigned int vaoId)
{
    bool result = false;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(vaoId);
        result = true;
    }
#endif
    return result;
}

void rlDisableVertexArray(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
#endif
}

void rlEnableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glEnableVertexAttribArray(index);
#endif
}

void rlDisableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDisableVertexAttribArray(index);
#endif
}

void rlDrawVertexArray(int offset, int count)
{
    glDrawArrays(GL_TRIANGLES, offset, count);
}

void rlDrawVertexArrayElements(int offset, int count, void *buffer)
{
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (unsigned short*)buffer + offset);
}

void rlDrawVertexArrayInstanced(int offset, int count, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDrawArraysInstanced(GL_TRIANGLES, 0, count, instances);
#endif
}

void rlDrawVertexArrayElementsInstanced(int offset, int count, void *buffer, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (unsigned short*)buffer + offset, instances);
#endif
}

#if defined(GRAPHICS_API_OPENGL_11)
void rlEnableStatePointer(int vertexAttribType, void *buffer)
{
    if (buffer != NULL) glEnableClientState(vertexAttribType);
    switch (vertexAttribType)
    {
        case GL_VERTEX_ARRAY: glVertexPointer(3, GL_FLOAT, 0, buffer); break;
        case GL_TEXTURE_COORD_ARRAY: glTexCoordPointer(2, GL_FLOAT, 0, buffer); break;
        case GL_NORMAL_ARRAY: if (buffer != NULL) glNormalPointer(GL_FLOAT, 0, buffer); break;
        case GL_COLOR_ARRAY: if (buffer != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, buffer); break;
        //case GL_INDEX_ARRAY: if (buffer != NULL) glIndexPointer(GL_SHORT, 0, buffer); break; // Indexed colors
        default: break;
    }
}

void rlDisableStatePointer(int vertexAttribType)
{
    glDisableClientState(vertexAttribType);
}
#endif

unsigned int rlLoadVertexArray(void)
{
    unsigned int vaoId = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenVertexArrays(1, &vaoId);
#endif
    return vaoId;
}

void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, void *pointer)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribPointer(index, compSize, type, normalized, stride, pointer);
#endif
}

void rlSetVertexAttributeDivisor(unsigned int index, int divisor)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribDivisor(index, divisor);
#endif
}

void rlUnloadVertexArray(unsigned int vaoId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vaoId);
        TRACELOG(LOG_INFO, "VAO: [ID %i] Unloaded vertex array data from VRAM (GPU)", vaoId);
    }
#endif
}

void rlUnloadVertexBuffer(unsigned int vboId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteBuffers(1, &vboId);
    //TRACELOG(LOG_INFO, "VBO: Unloaded vertex data from VRAM (GPU)");
#endif
}

// Shaders management
//-----------------------------------------------------------------------------------------------
// Load shader from code strings
// NOTE: If shader string is NULL, using default vertex/fragment shaders
unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = RLGL.State.defaultVShaderId;
    unsigned int fragmentShaderId = RLGL.State.defaultFShaderId;

    if (vsCode != NULL) vertexShaderId = rlCompileShader(vsCode, GL_VERTEX_SHADER);
    if (fsCode != NULL) fragmentShaderId = rlCompileShader(fsCode, GL_FRAGMENT_SHADER);

    if ((vertexShaderId == RLGL.State.defaultVShaderId) && (fragmentShaderId == RLGL.State.defaultFShaderId)) id = RLGL.State.defaultShader.id;
    else
    {
        id = rlLoadShaderProgram(vertexShaderId, fragmentShaderId);

        if (vertexShaderId != RLGL.State.defaultVShaderId)
        {
            // Detach shader before deletion to make sure memory is freed
            glDetachShader(id, vertexShaderId);
            glDeleteShader(vertexShaderId);
        }
        if (fragmentShaderId != RLGL.State.defaultFShaderId)
        {
            // Detach shader before deletion to make sure memory is freed
            glDetachShader(id, fragmentShaderId);
            glDeleteShader(fragmentShaderId);
        }

        if (id == 0)
        {
            TRACELOG(LOG_WARNING, "SHADER: Failed to load custom shader code");
            id = RLGL.State.defaultShader.id;
        }
    }

    // Get available shader uniforms
    // NOTE: This information is useful for debug...
    int uniformCount = -1;

    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount);

    for (int i = 0; i < uniformCount; i++)
    {
        int namelen = -1;
        int num = -1;
        char name[256]; // Assume no variable names longer than 256
        GLenum type = GL_ZERO;

        // Get the name of the uniforms
        glGetActiveUniform(id, i, sizeof(name) - 1, &namelen, &num, &type, name);

        name[namelen] = 0;

        TRACELOGD("SHADER: [ID %i] Active uniform (%s) set at location: %i", id, name, glGetUniformLocation(id, name));
    }
#endif

    return id;
}

// Compile custom shader and return shader id
unsigned int rlCompileShader(const char *shaderCode, int type)
{
    unsigned int shader = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to compile vertex shader code", shader); break;
            case GL_FRAGMENT_SHADER: TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to compile fragment shader code", shader); break;
            //case GL_GEOMETRY_SHADER:
            //case GL_COMPUTE_SHADER:
            default: break;
        }

        int maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = RL_CALLOC(maxLength, sizeof(char));
            glGetShaderInfoLog(shader, maxLength, &length, log);
            TRACELOG(LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);
            RL_FREE(log);
        }
    }
    else
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: TRACELOG(LOG_INFO, "SHADER: [ID %i] Vertex shader compiled successfully", shader); break;
            case GL_FRAGMENT_SHADER: TRACELOG(LOG_INFO, "SHADER: [ID %i] Fragment shader compiled successfully", shader); break;
            //case GL_GEOMETRY_SHADER:
            //case GL_COMPUTE_SHADER:
            default: break;
        }
    }
#endif

    return shader;
}

// Load custom shader strings and return program id
unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId)
{
    unsigned int program = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    GLint success = 0;
    program = glCreateProgram();

    glAttachShader(program, vShaderId);
    glAttachShader(program, fShaderId);

    // NOTE: Default attribute shader locations must be binded before linking
    glBindAttribLocation(program, 0, DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, 1, DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, 2, DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, 3, DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, 4, DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, 5, DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);

    // NOTE: If some attrib name is no found on the shader, it locations becomes -1

    glLinkProgram(program);

    // NOTE: All uniform variables are intitialised to 0 when a program links

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to link shader program", program);

        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(program, maxLength, &length, log);
            TRACELOG(LOG_WARNING, "SHADER: [ID %i] Link error: %s", program, log);
            RL_FREE(log);
        }

        glDeleteProgram(program);

        program = 0;
    }
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Program shader loaded successfully", program);
#endif
    return program;
}

// Unload shader program
void rlUnloadShaderProgram(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteProgram(id);

    TRACELOG(LOG_INFO, "SHADER: [ID %i] Unloaded shader program data from VRAM (GPU)", id);
#endif
}

// Get shader location uniform
int rlGetLocationUniform(unsigned int shaderId, const char *uniformName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetUniformLocation(shaderId, uniformName);

    if (location == -1) TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to find shader uniform: %s", shaderId, uniformName);
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Shader uniform (%s) set at location: %i", shaderId, uniformName, location);
#endif
    return location;
}

// Get shader location attribute
int rlGetLocationAttrib(unsigned int shaderId, const char *attribName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetAttribLocation(shaderId, attribName);

    if (location == -1) TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to find shader attribute: %s", shaderId, attribName);
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Shader attribute (%s) set at location: %i", shaderId, attribName, location);
#endif
    return location;
}

// Set shader value uniform
void rlSetUniform(int locIndex, const void *value, int uniformType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (uniformType)
    {
        case SHADER_UNIFORM_FLOAT: glUniform1fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_VEC2: glUniform2fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_VEC3: glUniform3fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_VEC4: glUniform4fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_INT: glUniform1iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_IVEC2: glUniform2iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_IVEC3: glUniform3iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_IVEC4: glUniform4iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_SAMPLER2D: glUniform1iv(locIndex, count, (int *)value); break;
        default: TRACELOG(LOG_WARNING, "SHADER: Failed to set uniform value, data type not recognized");
    }
#endif
}

// Set shader value attribute
void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (attribType)
    {
        case SHADER_ATTRIB_FLOAT: if (count == 1) glVertexAttrib1fv(locIndex, (float *)value); break;
        case SHADER_ATTRIB_VEC2: if (count == 2) glVertexAttrib2fv(locIndex, (float *)value); break;
        case SHADER_ATTRIB_VEC3: if (count == 3) glVertexAttrib3fv(locIndex, (float *)value); break;
        case SHADER_ATTRIB_VEC4: if (count == 4) glVertexAttrib4fv(locIndex, (float *)value); break;
        default: TRACELOG(LOG_WARNING, "SHADER: Failed to set attrib default value, data type not recognized");
    }
#endif
}

// Set shader value uniform matrix
void rlSetUniformMatrix(int locIndex, Matrix mat)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glUniformMatrix4fv(locIndex, 1, false, MatrixToFloat(mat));
#endif
}

// Set shader value uniform sampler
void rlSetUniformSampler(int locIndex, unsigned int textureId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Check if texture is already active
    for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++) if (RLGL.State.activeTextureId[i] == textureId) return;

    // Register a new active texture for the internal batch system
    // NOTE: Default texture is always activated as GL_TEXTURE0
    for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++)
    {
        if (RLGL.State.activeTextureId[i] == 0)
        {
            glUniform1i(locIndex, 1 + i);              // Activate new texture unit
            RLGL.State.activeTextureId[i] = textureId; // Save texture id for binding on drawing
            break;
        }
    }
#endif
}

// Set shader currently active
void rlSetShader(Shader shader)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.State.currentShader.id != shader.id)
    {
        rlDrawRenderBatch(RLGL.currentBatch);
        RLGL.State.currentShader = shader;
    }
#endif
}

// Matrix state management
//-----------------------------------------------------------------------------------------
// Return internal modelview matrix
Matrix rlGetMatrixModelview(void)
{
    Matrix matrix = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    matrix.m0  = mat[0];     matrix.m1  = mat[1];     matrix.m2  = mat[2];     matrix.m3  = mat[3];
    matrix.m4  = mat[4];     matrix.m5  = mat[5];     matrix.m6  = mat[6];     matrix.m7  = mat[7];
    matrix.m8  = mat[8];     matrix.m9  = mat[9];     matrix.m10 = mat[10];    matrix.m11 = mat[11];
    matrix.m12 = mat[12];    matrix.m13 = mat[13];    matrix.m14 = mat[14];    matrix.m15 = mat[15];
#else
    matrix = RLGL.State.modelview;
#endif
    return matrix;
}

// Return internal projection matrix
Matrix rlGetMatrixProjection(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_PROJECTION_MATRIX,mat);
    Matrix m;
    m.m0  = mat[0];     m.m1  = mat[1];     m.m2  = mat[2];     m.m3  = mat[3];
    m.m4  = mat[4];     m.m5  = mat[5];     m.m6  = mat[6];     m.m7  = mat[7];
    m.m8  = mat[8];     m.m9  = mat[9];     m.m10 = mat[10];    m.m11 = mat[11];
    m.m12 = mat[12];    m.m13 = mat[13];    m.m14 = mat[14];    m.m15 = mat[15];
    return m;
#else
    return RLGL.State.projection;
#endif
}

// Get internal accumulated transform matrix
Matrix rlGetMatrixTransform(void)
{
    Matrix mat = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // TODO: Consider possible transform matrices in the RLGL.State.stack
    // Is this the right order? or should we start with the first stored matrix instead of the last one?
    //Matrix matStackTransform = MatrixIdentity();
    //for (int i = RLGL.State.stackCounter; i > 0; i--) matStackTransform = MatrixMultiply(RLGL.State.stack[i], matStackTransform);
    mat = RLGL.State.transform;
#endif
    return mat;
}

// Get internal projection matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixProjectionStereo(int eye)
{
    Matrix mat = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mat = RLGL.State.projectionStereo[eye];
#endif
    return mat;
}

// Get internal view offset matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye)
{
    Matrix mat = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mat = RLGL.State.viewOffsetStereo[eye];
#endif
    return mat;
}

// Set a custom modelview matrix (replaces internal modelview matrix)
void rlSetMatrixModelview(Matrix view)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.modelview = view;
#endif
}

// Set a custom projection matrix (replaces internal projection matrix)
void rlSetMatrixProjection(Matrix projection)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.projection = projection;
#endif
}

// Set eyes projection matrices for stereo rendering
void rlSetMatrixProjectionStereo(Matrix right, Matrix left)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.projectionStereo[0] = right;
    RLGL.State.projectionStereo[1] = left;
#endif
}

// Set eyes view offsets matrices for stereo rendering
void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.viewOffsetStereo[0] = right;
    RLGL.State.viewOffsetStereo[1] = left;
#endif
}

// Load and draw a 1x1 XY quad in NDC
void rlLoadDrawQuad(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    float vertices[] = {
         // Positions         Texcoords
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    };

    // Gen VAO to contain VBO
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    // Gen and fill vertex buffer (VBO)
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    // Bind vertex attributes (position, texcoords)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)0); // Positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float))); // Texcoords

    // Draw quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    // Delete buffers (VBO and VAO)
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
#endif
}

// Load and draw a 1x1 3D cube in NDC
void rlLoadDrawCube(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

    float vertices[] = {
         // Positions          Normals               Texcoords
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f
    };

    // Gen VAO to contain VBO
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    // Gen and fill vertex buffer (VBO)
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind vertex attributes (position, normals, texcoords)
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)0); // Positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(3*sizeof(float))); // Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(6*sizeof(float))); // Texcoords
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Delete VBO and VAO
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
#endif
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Load default shader (just vertex positioning and texture coloring)
// NOTE: This shader program is used for internal buffers
// NOTE: It uses global variable: RLGL.State.defaultShader
static void rlLoadShaderDefault(void)
{
    RLGL.State.defaultShader.locs = (int *)RL_CALLOC(MAX_SHADER_LOCATIONS, sizeof(int));

    // NOTE: All locations must be reseted to -1 (no location)
    for (int i = 0; i < MAX_SHADER_LOCATIONS; i++) RLGL.State.defaultShader.locs[i] = -1;

    // Vertex shader directly defined, no external file required
    const char *vShaderDefault =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330                       \n"
    "in vec3 vertexPosition;            \n"
    "in vec2 vertexTexCoord;            \n"
    "in vec4 vertexColor;               \n"
    "out vec2 fragTexCoord;             \n"
    "out vec4 fragColor;                \n"
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#endif
    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vertexTexCoord; \n"
    "    fragColor = vertexColor;       \n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
    "}                                  \n";

    // Fragment shader directly defined, no external file required
    const char *fShaderDefault =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330       \n"
    "in vec2 fragTexCoord;              \n"
    "in vec4 fragColor;                 \n"
    "out vec4 finalColor;               \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
    "    finalColor = texelColor*colDiffuse*fragColor;        \n"
    "}                                  \n";
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "precision mediump float;           \n"     // Precision required for OpenGL ES2 (WebGL)
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#endif

    // NOTE: Compiled vertex/fragment shaders are kept for re-use
    RLGL.State.defaultVShaderId = rlCompileShader(vShaderDefault, GL_VERTEX_SHADER);     // Compile default vertex shader
    RLGL.State.defaultFShaderId = rlCompileShader(fShaderDefault, GL_FRAGMENT_SHADER);   // Compile default fragment shader

    RLGL.State.defaultShader.id = rlLoadShaderProgram(RLGL.State.defaultVShaderId, RLGL.State.defaultFShaderId);

    if (RLGL.State.defaultShader.id > 0)
    {
        TRACELOG(LOG_INFO, "SHADER: [ID %i] Default shader loaded successfully", RLGL.State.defaultShader.id);

        // Set default shader locations: attributes locations
        RLGL.State.defaultShader.locs[SHADER_LOC_VERTEX_POSITION] = glGetAttribLocation(RLGL.State.defaultShader.id, "vertexPosition");
        RLGL.State.defaultShader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = glGetAttribLocation(RLGL.State.defaultShader.id, "vertexTexCoord");
        RLGL.State.defaultShader.locs[SHADER_LOC_VERTEX_COLOR] = glGetAttribLocation(RLGL.State.defaultShader.id, "vertexColor");

        // Set default shader locations: uniform locations
        RLGL.State.defaultShader.locs[SHADER_LOC_MATRIX_MVP]  = glGetUniformLocation(RLGL.State.defaultShader.id, "mvp");
        RLGL.State.defaultShader.locs[SHADER_LOC_COLOR_DIFFUSE] = glGetUniformLocation(RLGL.State.defaultShader.id, "colDiffuse");
        RLGL.State.defaultShader.locs[SHADER_LOC_MAP_DIFFUSE] = glGetUniformLocation(RLGL.State.defaultShader.id, "texture0");
    }
    else TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to load default shader", RLGL.State.defaultShader.id);
}

// Unload default shader
// NOTE: It uses global variable: RLGL.State.defaultShader
static void rlUnloadShaderDefault(void)
{
    glUseProgram(0);

    glDetachShader(RLGL.State.defaultShader.id, RLGL.State.defaultVShaderId);
    glDetachShader(RLGL.State.defaultShader.id, RLGL.State.defaultFShaderId);
    glDeleteShader(RLGL.State.defaultVShaderId);
    glDeleteShader(RLGL.State.defaultFShaderId);

    glDeleteProgram(RLGL.State.defaultShader.id);

    RL_FREE(RLGL.State.defaultShader.locs);

    TRACELOG(LOG_INFO, "SHADER: [ID %i] Default shader unloaded successfully", RLGL.State.defaultShader.id);
}

#if defined(SUPPORT_GL_DETAILS_INFO)
// Get compressed format official GL identifier name
static char *rlGetCompressedFormatName(int format)
{
    static char compName[64] = { 0 };
    memset(compName, 0, 64);

    switch (format)
    {
        // GL_EXT_texture_compression_s3tc
        case 0x83F0: strcpy(compName, "GL_COMPRESSED_RGB_S3TC_DXT1_EXT"); break;
        case 0x83F1: strcpy(compName, "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT"); break;
        case 0x83F2: strcpy(compName, "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT"); break;
        case 0x83F3: strcpy(compName, "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT"); break;
        // GL_3DFX_texture_compression_FXT1
        case 0x86B0: strcpy(compName, "GL_COMPRESSED_RGB_FXT1_3DFX"); break;
        case 0x86B1: strcpy(compName, "GL_COMPRESSED_RGBA_FXT1_3DFX"); break;
        // GL_IMG_texture_compression_pvrtc
        case 0x8C00: strcpy(compName, "GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG"); break;
        case 0x8C01: strcpy(compName, "GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG"); break;
        case 0x8C02: strcpy(compName, "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG"); break;
        case 0x8C03: strcpy(compName, "GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG"); break;
        // GL_OES_compressed_ETC1_RGB8_texture
        case 0x8D64: strcpy(compName, "GL_ETC1_RGB8_OES"); break;
        // GL_ARB_texture_compression_rgtc
        case 0x8DBB: strcpy(compName, "GL_COMPRESSED_RED_RGTC1"); break;
        case 0x8DBC: strcpy(compName, "GL_COMPRESSED_SIGNED_RED_RGTC1"); break;
        case 0x8DBD: strcpy(compName, "GL_COMPRESSED_RG_RGTC2"); break;
        case 0x8DBE: strcpy(compName, "GL_COMPRESSED_SIGNED_RG_RGTC2"); break;
        // GL_ARB_texture_compression_bptc
        case 0x8E8C: strcpy(compName, "GL_COMPRESSED_RGBA_BPTC_UNORM_ARB"); break;
        case 0x8E8D: strcpy(compName, "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB"); break;
        case 0x8E8E: strcpy(compName, "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB"); break;
        case 0x8E8F: strcpy(compName, "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB"); break;
        // GL_ARB_ES3_compatibility
        case 0x9274: strcpy(compName, "GL_COMPRESSED_RGB8_ETC2"); break;
        case 0x9275: strcpy(compName, "GL_COMPRESSED_SRGB8_ETC2"); break;
        case 0x9276: strcpy(compName, "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2"); break;
        case 0x9277: strcpy(compName, "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2"); break;
        case 0x9278: strcpy(compName, "GL_COMPRESSED_RGBA8_ETC2_EAC"); break;
        case 0x9279: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC"); break;
        case 0x9270: strcpy(compName, "GL_COMPRESSED_R11_EAC"); break;
        case 0x9271: strcpy(compName, "GL_COMPRESSED_SIGNED_R11_EAC"); break;
        case 0x9272: strcpy(compName, "GL_COMPRESSED_RG11_EAC"); break;
        case 0x9273: strcpy(compName, "GL_COMPRESSED_SIGNED_RG11_EAC"); break;
        // GL_KHR_texture_compression_astc_hdr
        case 0x93B0: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_4x4_KHR"); break;
        case 0x93B1: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_5x4_KHR"); break;
        case 0x93B2: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_5x5_KHR"); break;
        case 0x93B3: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_6x5_KHR"); break;
        case 0x93B4: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_6x6_KHR"); break;
        case 0x93B5: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_8x5_KHR"); break;
        case 0x93B6: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_8x6_KHR"); break;
        case 0x93B7: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_8x8_KHR"); break;
        case 0x93B8: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_10x5_KHR"); break;
        case 0x93B9: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_10x6_KHR"); break;
        case 0x93BA: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_10x8_KHR"); break;
        case 0x93BB: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_10x10_KHR"); break;
        case 0x93BC: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_12x10_KHR"); break;
        case 0x93BD: strcpy(compName, "GL_COMPRESSED_RGBA_ASTC_12x12_KHR"); break;
        case 0x93D0: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR"); break;
        case 0x93D1: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR"); break;
        case 0x93D2: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR"); break;
        case 0x93D3: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR"); break;
        case 0x93D4: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR"); break;
        case 0x93D5: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR"); break;
        case 0x93D6: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR"); break;
        case 0x93D7: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR"); break;
        case 0x93D8: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR"); break;
        case 0x93D9: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR"); break;
        case 0x93DA: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR"); break;
        case 0x93DB: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR"); break;
        case 0x93DC: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR"); break;
        case 0x93DD: strcpy(compName, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR"); break;
        default: strcpy(compName, "GL_COMPRESSED_UNKNOWN"); break;
    }

    return compName;
}
#endif  // SUPPORT_GL_DETAILS_INFO

#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

#if defined(GRAPHICS_API_OPENGL_11)
// Mipmaps data is generated after image data
// NOTE: Only works with RGBA (4 bytes) data!
static int rlGenerateMipmapsData(unsigned char *data, int baseWidth, int baseHeight)
{
    int mipmapCount = 1;                // Required mipmap levels count (including base level)
    int width = baseWidth;
    int height = baseHeight;
    int size = baseWidth*baseHeight*4;  // Size in bytes (will include mipmaps...), RGBA only

    // Count mipmap levels required
    while ((width != 1) && (height != 1))
    {
        width /= 2;
        height /= 2;

        TRACELOGD("TEXTURE: Next mipmap size: %i x %i", width, height);

        mipmapCount++;

        size += (width*height*4);       // Add mipmap size (in bytes)
    }

    TRACELOGD("TEXTURE: Total mipmaps required: %i", mipmapCount);
    TRACELOGD("TEXTURE: Total size of data required: %i", size);

    unsigned char *temp = RL_REALLOC(data, size);

    if (temp != NULL) data = temp;
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to re-allocate required mipmaps memory");

    width = baseWidth;
    height = baseHeight;
    size = (width*height*4);

    // Generate mipmaps
    // NOTE: Every mipmap data is stored after data
    Color *image = (Color *)RL_MALLOC(width*height*sizeof(Color));
    Color *mipmap = NULL;
    int offset = 0;
    int j = 0;

    for (int i = 0; i < size; i += 4)
    {
        image[j].r = data[i];
        image[j].g = data[i + 1];
        image[j].b = data[i + 2];
        image[j].a = data[i + 3];
        j++;
    }

    TRACELOGD("TEXTURE: Mipmap base size (%ix%i)", width, height);

    for (int mip = 1; mip < mipmapCount; mip++)
    {
        mipmap = rlGenNextMipmapData(image, width, height);

        offset += (width*height*4); // Size of last mipmap
        j = 0;

        width /= 2;
        height /= 2;
        size = (width*height*4);    // Mipmap size to store after offset

        // Add mipmap to data
        for (int i = 0; i < size; i += 4)
        {
            data[offset + i] = mipmap[j].r;
            data[offset + i + 1] = mipmap[j].g;
            data[offset + i + 2] = mipmap[j].b;
            data[offset + i + 3] = mipmap[j].a;
            j++;
        }

        RL_FREE(image);

        image = mipmap;
        mipmap = NULL;
    }

    RL_FREE(mipmap);       // free mipmap data

    return mipmapCount;
}

// Manual mipmap generation (basic scaling algorithm)
static Color *rlGenNextMipmapData(Color *srcData, int srcWidth, int srcHeight)
{
    int x2, y2;
    Color prow, pcol;

    int width = srcWidth/2;
    int height = srcHeight/2;

    Color *mipmap = (Color *)RL_MALLOC(width*height*sizeof(Color));

    // Scaling algorithm works perfectly (box-filter)
    for (int y = 0; y < height; y++)
    {
        y2 = 2*y;

        for (int x = 0; x < width; x++)
        {
            x2 = 2*x;

            prow.r = (srcData[y2*srcWidth + x2].r + srcData[y2*srcWidth + x2 + 1].r)/2;
            prow.g = (srcData[y2*srcWidth + x2].g + srcData[y2*srcWidth + x2 + 1].g)/2;
            prow.b = (srcData[y2*srcWidth + x2].b + srcData[y2*srcWidth + x2 + 1].b)/2;
            prow.a = (srcData[y2*srcWidth + x2].a + srcData[y2*srcWidth + x2 + 1].a)/2;

            pcol.r = (srcData[(y2+1)*srcWidth + x2].r + srcData[(y2+1)*srcWidth + x2 + 1].r)/2;
            pcol.g = (srcData[(y2+1)*srcWidth + x2].g + srcData[(y2+1)*srcWidth + x2 + 1].g)/2;
            pcol.b = (srcData[(y2+1)*srcWidth + x2].b + srcData[(y2+1)*srcWidth + x2 + 1].b)/2;
            pcol.a = (srcData[(y2+1)*srcWidth + x2].a + srcData[(y2+1)*srcWidth + x2 + 1].a)/2;

            mipmap[y*width + x].r = (prow.r + pcol.r)/2;
            mipmap[y*width + x].g = (prow.g + pcol.g)/2;
            mipmap[y*width + x].b = (prow.b + pcol.b)/2;
            mipmap[y*width + x].a = (prow.a + pcol.a)/2;
        }
    }

    TRACELOGD("TEXTURE: Mipmap generated successfully (%ix%i)", width, height);

    return mipmap;
}
#endif  // GRAPHICS_API_OPENGL_11

// Get pixel data size in bytes (image or texture)
// NOTE: Size depends on pixel format
static int rlGetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    dataSize = width*height*bpp/8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }

    return dataSize;
}
#endif  // RLGL_IMPLEMENTATION
