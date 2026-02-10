/**********************************************************************************************
*
*   rlsw v1.0 - An OpenGL 1.1-style software renderer implementation
*
*   DESCRIPTION:
*       rlsw is a custom OpenGL 1.1-style implementation on software, intended to provide all
*       functionality available on rlgl.h library used by raylib, becoming a direct software
*       rendering replacement for OpenGL 1.1 backend and allowing to run raylib on GPU-less
*       devices when required
*
*   FEATURES:
*       - Rendering to custom internal framebuffer with multiple color modes supported:
*           - Color buffer: RGB - 8-bit (3:3:2) | RGB - 16-bit (5:6:5) | RGB - 24-bit (8:8:8)
*           - Depth buffer: D - 8-bit (unorm) | D - 16-bit (unorm) | D - 24-bit (unorm)
*       - Rendering modes supported: POINT, LINES, TRIANGLE, QUADS
*           - Additional features: Polygon modes, Point width, Line width
*       - Clipping support for all rendering modes
*       - Texture features supported:
*           - All uncompressed texture formats supported by raylib
*           - Texture Minification/Magnification checks
*           - Point and Bilinear filtering
*           - Texture Wrap Modes with separate checks for S/T coordinates
*       - Vertex Arrays support with direct primitive drawing mode
*       - Matrix Stack support (Matrix Push/Pop)
*       - Other GL misc features:
*           - GL-style getter functions
*           - Framebuffer resizing
*           - Perspective correction
*           - Scissor clipping
*           - Depth testing
*           - Blend modes
*           - Face culling
*
*   ADDITIONAL NOTES:
*       Check PR for more info: https://github.com/raysan5/raylib/pull/4832
*
*   CONFIGURATION:
*       #define RLSW_IMPLEMENTATION
*           Generates the implementation of the library into the included file
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation
*
*       #define RLSW_USE_SIMD_INTRINSICS
*           Detect and use SIMD intrinsics on the host compilation platform
*           SIMD could improve rendering considerable vectorizing some raster operations
*           but the target platforms running the compiled program with SIMD enabled
*           must support the SIMD the program has been built for, making them only
*           recommended under specific situations and only if the developers know
*           what are they doing; this flag is not defined by default
*
*       rlsw capabilities could be customized just defining some internal
*       values before library inclusion (default values listed):
*
*           #define SW_GL_FRAMEBUFFER_COPY_BGRA     true
*           #define SW_GL_BINDING_COPY_TEXTURE      true
*           #define SW_COLOR_BUFFER_BITS            24
*           #define SW_DEPTH_BUFFER_BITS            16
*           #define SW_MAX_PROJECTION_STACK_SIZE    2
*           #define SW_MAX_MODELVIEW_STACK_SIZE     8
*           #define SW_MAX_TEXTURE_STACK_SIZE       2
*           #define SW_MAX_TEXTURES                 128
*
*
*   LICENSE: MIT
*
*   Copyright (c) 2025-2026 Le Juez Victor (@Bigfoot71), reviewed by Ramon Santamaria (@raysan5)
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#ifndef RLSW_H
#define RLSW_H

#include <stdbool.h>
#include <stdint.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Function specifiers definition
#ifndef SWAPI
    #define SWAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

#ifndef SW_MALLOC
    #define SW_MALLOC(sz) malloc(sz)
#endif
#ifndef SW_REALLOC
    #define SW_REALLOC(ptr, newSz) realloc(ptr, newSz)
#endif
#ifndef SW_FREE
    #define SW_FREE(ptr) free(ptr)
#endif

#ifndef SW_RESTRICT
    #ifdef _MSC_VER
        #define SW_RESTRICT __restrict
    #else
        #define SW_RESTRICT restrict
    #endif
#endif

#ifndef SW_GL_FRAMEBUFFER_COPY_BGRA
    #define SW_GL_FRAMEBUFFER_COPY_BGRA     true
#endif

#ifndef SW_COLOR_BUFFER_BITS
    #define SW_COLOR_BUFFER_BITS            32  //< 32 (rgba), 16 (rgb packed) or 8 (rgb packed)
#endif

#ifndef SW_DEPTH_BUFFER_BITS
    #define SW_DEPTH_BUFFER_BITS            16  //< 32, 24 or 16
#endif

#ifndef SW_MAX_PROJECTION_STACK_SIZE
    #define SW_MAX_PROJECTION_STACK_SIZE    2
#endif

#ifndef SW_MAX_MODELVIEW_STACK_SIZE
    #define SW_MAX_MODELVIEW_STACK_SIZE     8
#endif

#ifndef SW_MAX_TEXTURE_STACK_SIZE
    #define SW_MAX_TEXTURE_STACK_SIZE       2
#endif

#ifndef SW_MAX_TEXTURES
    #define SW_MAX_TEXTURES                 128
#endif

// Under normal circumstances, clipping a polygon can add at most one vertex per clipping plane
// Considering the largest polygon involved is a quadrilateral (4 vertices),
// and that clipping occurs against both the frustum (6 planes) and the scissors (4 planes),
// the maximum number of vertices after clipping is:
// 4 (original vertices) + 6 (frustum planes) + 4 (scissors planes) = 14
#ifndef SW_MAX_CLIPPED_POLYGON_VERTICES
    #define SW_MAX_CLIPPED_POLYGON_VERTICES 14
#endif

#ifndef SW_CLIP_EPSILON
    #define SW_CLIP_EPSILON                 1e-4f
#endif

//----------------------------------------------------------------------------------
// OpenGL Compatibility Types
//----------------------------------------------------------------------------------
typedef unsigned int        GLenum;
typedef unsigned char       GLboolean;
typedef unsigned int        GLbitfield;
typedef void                GLvoid;
typedef signed char         GLbyte;
typedef short               GLshort;
typedef int                 GLint;
typedef unsigned char       GLubyte;
typedef unsigned short      GLushort;
typedef unsigned int        GLuint;
typedef int                 GLsizei;
typedef float               GLfloat;
typedef float               GLclampf;
typedef double              GLdouble;
typedef double              GLclampd;

//----------------------------------------------------------------------------------
// OpenGL Definitions
// NOTE: Not used/supported definitions are commented
//----------------------------------------------------------------------------------
#define GL_FALSE                            0
#define GL_TRUE                             1

#define GL_SCISSOR_TEST                     0x0C11
#define GL_TEXTURE_2D                       0x0DE1
#define GL_DEPTH_TEST                       0x0B71
#define GL_CULL_FACE                        0x0B44
#define GL_BLEND                            0x0BE2

#define GL_VENDOR                           0x1F00
#define GL_RENDERER                         0x1F01
#define GL_VERSION                          0x1F02
#define GL_EXTENSIONS                       0x1F03

//#define GL_ATTRIB_STACK_DEPTH             0x0BB0
//#define GL_CLIENT_ATTRIB_STACK_DEPTH      0x0BB1
#define GL_COLOR_CLEAR_VALUE                0x0C22
#define GL_DEPTH_CLEAR_VALUE                0x0B73
//#define GL_COLOR_WRITEMASK                0x0C23
//#define GL_CURRENT_INDEX                  0x0B01
#define GL_CURRENT_COLOR                    0x0B00
//#define GL_CURRENT_NORMAL                 0x0B02
//#define GL_CURRENT_RASTER_COLOR           0x0B04
//#define GL_CURRENT_RASTER_DISTANCE        0x0B09
//#define GL_CURRENT_RASTER_INDEX           0x0B05
//#define GL_CURRENT_RASTER_POSITION        0x0B07
//#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
//#define GL_CURRENT_RASTER_POSITION_VALID  0x0B08
#define GL_CURRENT_TEXTURE_COORDS           0x0B03
#define GL_POINT_SIZE                       0x0B11
#define GL_LINE_WIDTH                       0x0B21
//#define GL_INDEX_CLEAR_VALUE              0x0C20
//#define GL_INDEX_MODE                     0x0C30
//#define GL_INDEX_WRITEMASK                0x0C21
#define GL_MODELVIEW_MATRIX                 0x0BA6
#define GL_MODELVIEW_STACK_DEPTH            0x0BA3
//#define GL_NAME_STACK_DEPTH               0x0D70
#define GL_PROJECTION_MATRIX                0x0BA7
#define GL_PROJECTION_STACK_DEPTH           0x0BA4
//#define GL_RENDER_MODE                    0x0C40
//#define GL_RGBA_MODE                      0x0C31
#define GL_TEXTURE_MATRIX                   0x0BA8
#define GL_TEXTURE_STACK_DEPTH              0x0BA5
#define GL_VIEWPORT                         0x0BA2

#define GL_COLOR_BUFFER_BIT                 0x00004000
#define GL_DEPTH_BUFFER_BIT                 0x00000100

#define GL_MODELVIEW                        0x1700
#define GL_PROJECTION                       0x1701
#define GL_TEXTURE                          0x1702

#define GL_VERTEX_ARRAY                     0x8074
#define GL_NORMAL_ARRAY                     0x8075      // WARNING: Not implemented (defined for RLGL)
#define GL_COLOR_ARRAY                      0x8076
//#define GL_INDEX_ARRAY                    0x8077
#define GL_TEXTURE_COORD_ARRAY              0x8078

#define GL_POINTS                           0x0000
#define GL_LINES                            0x0001
//#define GL_LINE_LOOP                      0x0002
//#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                        0x0004
//#define GL_TRIANGLE_STRIP                 0x0005
//#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                            0x0007
//#define GL_QUAD_STRIP                     0x0008
//#define GL_POLYGON                        0x0009

#define GL_POINT                            0x1B00
#define GL_LINE                             0x1B01
#define GL_FILL                             0x1B02

#define GL_FRONT                            0x0404
#define GL_BACK                             0x0405

#define GL_ZERO                             0
#define GL_ONE                              1
#define GL_SRC_COLOR                        0x0300
#define GL_ONE_MINUS_SRC_COLOR              0x0301
#define GL_SRC_ALPHA                        0x0302
#define GL_ONE_MINUS_SRC_ALPHA              0x0303
#define GL_DST_ALPHA                        0x0304
#define GL_ONE_MINUS_DST_ALPHA              0x0305
#define GL_DST_COLOR                        0x0306
#define GL_ONE_MINUS_DST_COLOR              0x0307
#define GL_SRC_ALPHA_SATURATE               0x0308

#define GL_NEAREST                          0x2600
#define GL_LINEAR                           0x2601

#define GL_REPEAT                           0x2901
#define GL_CLAMP                            0x2900

#define GL_TEXTURE_MAG_FILTER               0x2800
#define GL_TEXTURE_MIN_FILTER               0x2801

#define GL_TEXTURE_WRAP_S                   0x2802
#define GL_TEXTURE_WRAP_T                   0x2803

#define GL_NO_ERROR                         0
#define GL_INVALID_ENUM                     0x0500
#define GL_INVALID_VALUE                    0x0501
#define GL_INVALID_OPERATION                0x0502
#define GL_STACK_OVERFLOW                   0x0503
#define GL_STACK_UNDERFLOW                  0x0504
#define GL_OUT_OF_MEMORY                    0x0505

#define GL_ALPHA                            0x1906
#define GL_LUMINANCE                        0x1909
#define GL_LUMINANCE_ALPHA                  0x190A
#define GL_RGB                              0x1907
#define GL_RGBA                             0x1908

#define GL_BYTE                             0x1400
#define GL_UNSIGNED_BYTE                    0x1401
#define GL_SHORT                            0x1402
#define GL_UNSIGNED_SHORT                   0x1403
#define GL_INT                              0x1404
#define GL_UNSIGNED_INT                     0x1405
#define GL_FLOAT                            0x1406

// OpenGL Definitions NOT USED
#define GL_PERSPECTIVE_CORRECTION_HINT      0x0C50
#define GL_PACK_ALIGNMENT                   0x0D05
#define GL_UNPACK_ALIGNMENT                 0x0CF5
#define GL_LINE_SMOOTH                      0x0B20
#define GL_SMOOTH                           0x1D01
#define GL_NICEST                           0x1102
#define GL_CCW                              0x0901
#define GL_CW                               0x0900
#define GL_NEVER                            0x0200
#define GL_LESS                             0x0201
#define GL_EQUAL                            0x0202
#define GL_LEQUAL                           0x0203
#define GL_GREATER                          0x0204
#define GL_NOTEQUAL                         0x0205
#define GL_GEQUAL                           0x0206
#define GL_ALWAYS                           0x0207

//----------------------------------------------------------------------------------
// OpenGL Bindings to rlsw
//----------------------------------------------------------------------------------
#define glReadPixels(x, y, w, h, f, t, p)           swCopyFramebuffer((x), (y), (w), (h), (f), (t), (p))
#define glEnable(state)                             swEnable((state))
#define glDisable(state)                            swDisable((state))
#define glGetFloatv(pname, params)                  swGetFloatv((pname), (params))
#define glGetString(pname)                          swGetString((pname))
#define glGetError()                                swGetError()
#define glViewport(x, y, w, h)                      swViewport((x), (y), (w), (h))
#define glScissor(x, y, w, h)                       swScissor((x), (y), (w), (h))
#define glClearColor(r, g, b, a)                    swClearColor((r), (g), (b), (a))
#define glClearDepth(d)                             swClearDepth((d))
#define glClear(bitmask)                            swClear((bitmask))
#define glBlendFunc(sfactor, dfactor)               swBlendFunc((sfactor), (dfactor))
#define glPolygonMode(face, mode)                   swPolygonMode((mode))
#define glCullFace(face)                            swCullFace((face))
#define glPointSize(size)                           swPointSize((size))
#define glLineWidth(width)                          swLineWidth((width))
#define glMatrixMode(mode)                          swMatrixMode((mode))
#define glPushMatrix()                              swPushMatrix()
#define glPopMatrix()                               swPopMatrix()
#define glLoadIdentity()                            swLoadIdentity()
#define glTranslatef(x, y, z)                       swTranslatef((x), (y), (z))
#define glRotatef(a, x, y, z)                       swRotatef((a), (x), (y), (z))
#define glScalef(x, y, z)                           swScalef((x), (y), (z))
#define glMultMatrixf(v)                            swMultMatrixf((v))
#define glFrustum(l, r, b, t, n, f)                 swFrustum((l), (r), (b), (t), (n), (f))
#define glOrtho(l, r, b, t, n, f)                   swOrtho((l), (r), (b), (t), (n), (f))
#define glBegin(mode)                               swBegin((mode))
#define glEnd()                                     swEnd()
#define glVertex2i(x, y)                            swVertex2i((x), (y))
#define glVertex2f(x, y)                            swVertex2f((x), (y))
#define glVertex2fv(v)                              swVertex2fv((v))
#define glVertex3i(x, y, z)                         swVertex3i((x), (y), (z))
#define glVertex3f(x, y, z)                         swVertex3f((x), (y), (z))
#define glvertex3fv(v)                              swVertex3fv((v))
#define glVertex4i(x, y, z, w)                      swVertex4i((x), (y), (z), (w))
#define glVertex4f(x, y, z, w)                      swVertex4f((x), (y), (z), (w))
#define glVertex4fv(v)                              swVertex4fv((v))
#define glColor3ub(r, g, b)                         swColor3ub((r), (g), (b))
#define glColor3ubv(v)                              swColor3ubv((v))
#define glColor3f(r, g, b)                          swColor3f((r), (g), (b))
#define glColor3fv(v)                               swColor3fv((v))
#define glColor4ub(r, g, b, a)                      swColor4ub((r), (g), (b), (a))
#define glColor4ubv(v)                              swColor4ubv((v))
#define glColor4f(r, g, b, a)                       swColor4f((r), (g), (b), (a))
#define glColor4fv(v)                               swColor4fv((v))
#define glTexCoord2f(u, v)                          swTexCoord2f((u), (v))
#define glTexCoord2fv(v)                            swTexCoord2fv((v))

#define glEnableClientState(t)                      ((void)(t))
#define glDisableClientState(t)                     swBindArray((t), 0)
#define glVertexPointer(sz, t, s, p)                swBindArray(SW_VERTEX_ARRAY, (p))
#define glTexCoordPointer(sz, t, s, p)              swBindArray(SW_TEXTURE_COORD_ARRAY, (p))
#define glColorPointer(sz, t, s, p)                 swBindArray(SW_COLOR_ARRAY, (p))
#define glDrawArrays(m, o, c)                       swDrawArrays((m), (o), (c))
#define glDrawElements(m,c,t,i)                     swDrawElements((m),(c),(t),(i))
#define glGenTextures(c, v)                         swGenTextures((c), (v))
#define glDeleteTextures(c, v)                      swDeleteTextures((c), (v))
#define glTexImage2D(tr, l, if, w, h, b, f, t, p)   swTexImage2D((w), (h), (f), (t), (p))
#define glTexParameteri(tr, pname, param)           swTexParameteri((pname), (param))
#define glBindTexture(tr, id)                       swBindTexture((id))

// OpenGL functions NOT IMPLEMENTED by rlsw
#define glDepthMask(X)                          ((void)(X))
#define glColorMask(X,Y,Z,W)                    ((void)(X),(void)(Y),(void)(Z),(void)(W))
#define glPixelStorei(X,Y)                      ((void)(X),(void)(Y))
#define glHint(X,Y)                             ((void)(X),(void)(Y))
#define glShadeModel(X)                         ((void)(X))
#define glFrontFace(X)                          ((void)(X))
#define glDepthFunc(X)                          ((void)(X))
#define glTexSubImage2D(X,Y,Z,W,A,B,C,D,E)      ((void)(X),(void)(Y),(void)(Z),(void)(W),(void)(A),(void)(B),(void)(C),(void)(D),(void)(E))
#define glGetTexImage(X,Y,Z,W,A)                ((void)(X),(void)(Y),(void)(Z),(void)(W),(void)(A))
#define glNormal3f(X,Y,Z)                       ((void)(X),(void)(Y),(void)(Z))
#define glNormal3fv(X)                          ((void)(X))
#define glNormalPointer(X,Y,Z)                  ((void)(X),(void)(Y),(void)(Z))

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    SW_SCISSOR_TEST = GL_SCISSOR_TEST,
    SW_TEXTURE_2D = GL_TEXTURE_2D,
    SW_DEPTH_TEST = GL_DEPTH_TEST,
    SW_CULL_FACE = GL_CULL_FACE,
    SW_BLEND = GL_BLEND
} SWstate;

typedef enum {
    SW_VENDOR = GL_VENDOR,
    SW_RENDERER = GL_RENDERER,
    SW_VERSION = GL_VERSION,
    SW_EXTENSIONS = GL_EXTENSIONS,
    SW_COLOR_CLEAR_VALUE = GL_COLOR_CLEAR_VALUE,
    SW_DEPTH_CLEAR_VALUE = GL_DEPTH_CLEAR_VALUE,
    SW_CURRENT_COLOR = GL_CURRENT_COLOR,
    SW_CURRENT_TEXTURE_COORDS = GL_CURRENT_TEXTURE_COORDS,
    SW_POINT_SIZE = GL_POINT_SIZE,
    SW_LINE_WIDTH = GL_LINE_WIDTH,
    SW_MODELVIEW_MATRIX = GL_MODELVIEW_MATRIX,
    SW_MODELVIEW_STACK_DEPTH = GL_MODELVIEW_STACK_DEPTH,
    SW_PROJECTION_MATRIX = GL_PROJECTION_MATRIX,
    SW_PROJECTION_STACK_DEPTH = GL_PROJECTION_STACK_DEPTH,
    SW_TEXTURE_MATRIX = GL_TEXTURE_MATRIX,
    SW_TEXTURE_STACK_DEPTH = GL_TEXTURE_STACK_DEPTH,
    SW_VIEWPORT = GL_VIEWPORT
} SWget;

typedef enum {
    SW_COLOR_BUFFER_BIT = GL_COLOR_BUFFER_BIT,
    SW_DEPTH_BUFFER_BIT = GL_DEPTH_BUFFER_BIT
} SWbuffer;

typedef enum {
    SW_PROJECTION = GL_PROJECTION,
    SW_MODELVIEW = GL_MODELVIEW,
    SW_TEXTURE = GL_TEXTURE
} SWmatrix;

typedef enum {
    SW_VERTEX_ARRAY = GL_VERTEX_ARRAY,
    SW_TEXTURE_COORD_ARRAY = GL_TEXTURE_COORD_ARRAY,
    SW_COLOR_ARRAY = GL_COLOR_ARRAY
} SWarray;

typedef enum {
    SW_POINTS = GL_POINTS,
    SW_LINES = GL_LINES,
    SW_TRIANGLES = GL_TRIANGLES,
    SW_QUADS = GL_QUADS
} SWdraw;

typedef enum {
    SW_POINT = GL_POINT,
    SW_LINE = GL_LINE,
    SW_FILL = GL_FILL
} SWpoly;

typedef enum {
    SW_FRONT = GL_FRONT,
    SW_BACK = GL_BACK,
} SWface;

typedef enum {
    SW_ZERO = GL_ZERO,
    SW_ONE = GL_ONE,
    SW_SRC_COLOR = GL_SRC_COLOR,
    SW_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
    SW_SRC_ALPHA = GL_SRC_ALPHA,
    SW_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
    SW_DST_ALPHA = GL_DST_ALPHA,
    SW_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
    SW_DST_COLOR = GL_DST_COLOR,
    SW_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
    SW_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE
} SWfactor;

typedef enum {
    SW_LUMINANCE = GL_LUMINANCE,
    SW_LUMINANCE_ALPHA = GL_LUMINANCE_ALPHA,
    SW_RGB = GL_RGB,
    SW_RGBA = GL_RGBA,
} SWformat;

typedef enum {
    SW_UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    SW_BYTE = GL_BYTE,
    SW_UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
    SW_SHORT = GL_SHORT,
    SW_UNSIGNED_INT = GL_UNSIGNED_INT,
    SW_INT = GL_INT,
    SW_FLOAT = GL_FLOAT
} SWtype;

typedef enum {
    SW_NEAREST = GL_NEAREST,
    SW_LINEAR = GL_LINEAR
} SWfilter;

typedef enum {
    SW_REPEAT = GL_REPEAT,
    SW_CLAMP = GL_CLAMP,
} SWwrap;

typedef enum {
    SW_TEXTURE_MIN_FILTER = GL_TEXTURE_MIN_FILTER,
    SW_TEXTURE_MAG_FILTER = GL_TEXTURE_MAG_FILTER,
    SW_TEXTURE_WRAP_S = GL_TEXTURE_WRAP_S,
    SW_TEXTURE_WRAP_T = GL_TEXTURE_WRAP_T
} SWtexparam;

typedef enum {
    SW_NO_ERROR = GL_NO_ERROR,
    SW_INVALID_ENUM = GL_INVALID_ENUM,
    SW_INVALID_VALUE = GL_INVALID_VALUE,
    SW_STACK_OVERFLOW = GL_STACK_OVERFLOW,
    SW_STACK_UNDERFLOW = GL_STACK_UNDERFLOW,
    SW_INVALID_OPERATION = GL_INVALID_OPERATION,
} SWerrcode;

//------------------------------------------------------------------------------------
// Functions Declaration - Public API
//------------------------------------------------------------------------------------
SWAPI bool swInit(int w, int h);
SWAPI void swClose(void);

SWAPI bool swResizeFramebuffer(int w, int h);
SWAPI void swCopyFramebuffer(int x, int y, int w, int h, SWformat format, SWtype type, void *pixels);
SWAPI void swBlitFramebuffer(int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, SWformat format, SWtype type, void *pixels);

SWAPI void swEnable(SWstate state);
SWAPI void swDisable(SWstate state);

SWAPI void swGetFloatv(SWget name, float *v);
SWAPI const char *swGetString(SWget name);
SWAPI SWerrcode swGetError(void);

SWAPI void swViewport(int x, int y, int width, int height);
SWAPI void swScissor(int x, int y, int width, int height);

SWAPI void swClearColor(float r, float g, float b, float a);
SWAPI void swClearDepth(float depth);
SWAPI void swClear(uint32_t bitmask);

SWAPI void swBlendFunc(SWfactor sfactor, SWfactor dfactor);
SWAPI void swPolygonMode(SWpoly mode);
SWAPI void swCullFace(SWface face);

SWAPI void swPointSize(float size);
SWAPI void swLineWidth(float width);

SWAPI void swMatrixMode(SWmatrix mode);
SWAPI void swPushMatrix(void);
SWAPI void swPopMatrix(void);
SWAPI void swLoadIdentity(void);
SWAPI void swTranslatef(float x, float y, float z);
SWAPI void swRotatef(float angle, float x, float y, float z);
SWAPI void swScalef(float x, float y, float z);
SWAPI void swMultMatrixf(const float *mat);
SWAPI void swFrustum(double left, double right, double bottom, double top, double znear, double zfar);
SWAPI void swOrtho(double left, double right, double bottom, double top, double znear, double zfar);

SWAPI void swBegin(SWdraw mode);
SWAPI void swEnd(void);

SWAPI void swVertex2i(int x, int y);
SWAPI void swVertex2f(float x, float y);
SWAPI void swVertex2fv(const float *v);
SWAPI void swVertex3i(int x, int y, int z);
SWAPI void swVertex3f(float x, float y, float z);
SWAPI void swVertex3fv(const float *v);
SWAPI void swVertex4i(int x, int y, int z, int w);
SWAPI void swVertex4f(float x, float y, float z, float w);
SWAPI void swVertex4fv(const float *v);

SWAPI void swColor3ub(uint8_t r, uint8_t g, uint8_t b);
SWAPI void swColor3ubv(const uint8_t *v);
SWAPI void swColor3f(float r, float g, float b);
SWAPI void swColor3fv(const float *v);
SWAPI void swColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
SWAPI void swColor4ubv(const uint8_t *v);
SWAPI void swColor4f(float r, float g, float b, float a);
SWAPI void swColor4fv(const float *v);

SWAPI void swTexCoord2f(float u, float v);
SWAPI void swTexCoord2fv(const float *v);

SWAPI void swBindArray(SWarray type, void *buffer);
SWAPI void swDrawArrays(SWdraw mode, int offset, int count);
SWAPI void swDrawElements(SWdraw mode, int count, int type, const void *indices);

SWAPI void swGenTextures(int count, uint32_t *textures);
SWAPI void swDeleteTextures(int count, uint32_t *textures);

SWAPI void swTexImage2D(int width, int height, SWformat format, SWtype type, const void *data);
SWAPI void swTexParameteri(int param, int value);
SWAPI void swBindTexture(uint32_t id);

#endif // RLSW_H

/***********************************************************************************
*
*   RLSW IMPLEMENTATION
*
************************************************************************************/
#define RLSW_IMPLEMENTATION
#if defined(RLSW_IMPLEMENTATION)

#include <stdlib.h>         // Required for: malloc(), free()
#include <stddef.h>         // Required for: NULL, size_t, uint8_t, uint16_t, uint32_t...
#include <math.h>           // Required for: sinf(), cosf(), floorf(), fabsf(), sqrtf(), roundf()

// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SW_SUPPORT_LOG_INFO
#if defined(SW_SUPPORT_LOG_INFO) //&& defined(_DEBUG)      // WARNING: LOG() output required for this tool
    #include <stdio.h>
    #define SW_LOG(...) printf(__VA_ARGS__)
#else
    #define SW_LOG(...)
#endif

#if defined(_MSC_VER)
    #define SW_ALIGN(x) __declspec(align(x))
#elif defined(__GNUC__) || defined(__clang__)
    #define SW_ALIGN(x) __attribute__((aligned(x)))
#else
    #define SW_ALIGN(x) // Do nothing if not available
#endif

#if defined(_M_X64) || defined(__x86_64__)
    #define SW_ARCH_X86_64
#elif defined(_M_IX86) || defined(__i386__)
    #define SW_ARCH_X86
#elif defined(_M_ARM) || defined(__arm__)
    #define SW_ARCH_ARM32
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define SW_ARCH_ARM64
#elif defined(__riscv)
    #define SW_ARCH_RISCV
#endif

#if defined(RLSW_USE_SIMD_INTRINSICS)
    // Check for SIMD vector instructions
    // NOTE: Compiler is responsible to enable required flags for host device,
    // supported features are detected at compiler init but varies depending on compiler
    // TODO: This logic must be reviewed to avoid the inclusion of multiple headers
    // and enable the higher level of SIMD available
    #if defined(__FMA__) && defined(__AVX2__)
        #define SW_HAS_FMA_AVX2
        #include <immintrin.h>
    #elif defined(__FMA__) && defined(__AVX__)
        #define SW_HAS_FMA_AVX
        #include <immintrin.h>
    #elif defined(__AVX2__)
        #define SW_HAS_AVX2
        #include <immintrin.h>
    #elif defined(__AVX__)
        #define SW_HAS_AVX
        #include <immintrin.h>
    #endif
    #if defined(__SSE4_2__)
        #define SW_HAS_SSE42
        #include <nmmintrin.h>
    #elif defined(__SSE4_1__)
        #define SW_HAS_SSE41
        #include <smmintrin.h>
    #elif defined(__SSSE3__)
        #define SW_HAS_SSSE3
        #include <tmmintrin.h>
    #elif defined(__SSE3__)
        #define SW_HAS_SSE3
        #include <pmmintrin.h>
    #elif defined(__SSE2__) || (defined(_M_AMD64) || defined(_M_X64)) // SSE2 x64
        #define SW_HAS_SSE2
        #include <emmintrin.h>
    #elif defined(__SSE__)
        #define SW_HAS_SSE
        #include <xmmintrin.h>
    #endif
    #if defined(__ARM_NEON) || defined(__aarch64__)
        #if defined(__ARM_FEATURE_FMA)
            #define SW_HAS_NEON_FMA
        #else
            #define SW_HAS_NEON
        #endif
        #include <arm_neon.h>
    #endif
    #if defined(__riscv_vector)
        // NOTE: Requires compilation flags: -march=rv64gcv -mabi=lp64d
        #define SW_HAS_RVV
        #include <riscv_vector.h>
    #endif
#endif  // RLSW_USE_SIMD_INTRINSICS

#ifdef __cplusplus
    #define SW_CURLY_INIT(name) name
#else
    #define SW_CURLY_INIT(name) (name)
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define SW_PI       3.14159265358979323846f
#define SW_INV_255  0.00392156862745098f            // 1.0f/255.0f
#define SW_DEG2RAD  (SW_PI/180.0f)
#define SW_RAD2DEG  (180.0f/SW_PI)

#define SW_COLOR_PIXEL_SIZE     (SW_COLOR_BUFFER_BITS >> 3)
#define SW_DEPTH_PIXEL_SIZE     (SW_DEPTH_BUFFER_BITS >> 3)
#define SW_PIXEL_SIZE           (SW_COLOR_PIXEL_SIZE + SW_DEPTH_PIXEL_SIZE)

#if (SW_PIXEL_SIZE <= 4)
    #define SW_PIXEL_ALIGNMENT 4
#else // if (SW_PIXEL_SIZE <= 8)
    #define SW_PIXEL_ALIGNMENT 8
#endif

#if (SW_COLOR_BUFFER_BITS == 8)
    #define SW_COLOR_TYPE       uint8_t
    #define SW_COLOR_IS_PACKED  1
    #define SW_COLOR_PACK_COMP  1
    #define SW_PACK_COLOR(r,g,b) ((((uint8_t)((r)*7+0.5f))&0x07)<<5 | (((uint8_t)((g)*7+0.5f))&0x07)<<2 | ((uint8_t)((b)*3+0.5f))&0x03)
    #define SW_UNPACK_R(p)      (((p)>>5)&0x07)
    #define SW_UNPACK_G(p)      (((p)>>2)&0x07)
    #define SW_UNPACK_B(p)      ((p)&0x03)
    #define SW_SCALE_R(v)       ((v)*255+3)/7
    #define SW_SCALE_G(v)       ((v)*255+3)/7
    #define SW_SCALE_B(v)       ((v)*255+1)/3
    #define SW_TO_FLOAT_R(v)    ((v)*(1.0f/7.0f))
    #define SW_TO_FLOAT_G(v)    ((v)*(1.0f/7.0f))
    #define SW_TO_FLOAT_B(v)    ((v)*(1.0f/3.0f))
#elif (SW_COLOR_BUFFER_BITS == 16)
    #define SW_COLOR_TYPE       uint16_t
    #define SW_COLOR_IS_PACKED  1
    #define SW_COLOR_PACK_COMP  1
    #define SW_PACK_COLOR(r,g,b) ((((uint16_t)((r)*31+0.5f))&0x1F)<<11 | (((uint16_t)((g)*63+0.5f))&0x3F)<<5 | ((uint16_t)((b)*31+0.5f))&0x1F)
    #define SW_UNPACK_R(p)      (((p)>>11)&0x1F)
    #define SW_UNPACK_G(p)      (((p)>>5)&0x3F)
    #define SW_UNPACK_B(p)      ((p)&0x1F)
    #define SW_SCALE_R(v)       ((v)*255+15)/31
    #define SW_SCALE_G(v)       ((v)*255+31)/63
    #define SW_SCALE_B(v)       ((v)*255+15)/31
    #define SW_TO_FLOAT_R(v)    ((v)*(1.0f/31.0f))
    #define SW_TO_FLOAT_G(v)    ((v)*(1.0f/63.0f))
    #define SW_TO_FLOAT_B(v)    ((v)*(1.0f/31.0f))
#else // 32 bits
    #define SW_COLOR_TYPE       uint8_t
    #define SW_COLOR_IS_PACKED  0
    #define SW_COLOR_PACK_COMP  4
#endif

#if (SW_DEPTH_BUFFER_BITS == 16)
    #define SW_DEPTH_TYPE       uint16_t
    #define SW_DEPTH_IS_PACKED  1
    #define SW_DEPTH_PACK_COMP  1
    #define SW_DEPTH_MAX        UINT16_MAX
    #define SW_DEPTH_SCALE      (1.0f/UINT16_MAX)
    #define SW_PACK_DEPTH(d)    ((SW_DEPTH_TYPE)((d)*SW_DEPTH_MAX))
    #define SW_UNPACK_DEPTH(p)  (p)
#elif (SW_DEPTH_BUFFER_BITS == 24)
    #define SW_DEPTH_TYPE       uint8_t
    #define SW_DEPTH_IS_PACKED  0
    #define SW_DEPTH_PACK_COMP  3
    #define SW_DEPTH_MAX        0xFFFFFFU
    #define SW_DEPTH_SCALE      (1.0f/0xFFFFFFU)
    #define SW_PACK_DEPTH_0(d)  ((uint8_t)(((uint32_t)((d)*SW_DEPTH_MAX)>>16)&0xFFU))
    #define SW_PACK_DEPTH_1(d)  ((uint8_t)(((uint32_t)((d)*SW_DEPTH_MAX)>>8)&0xFFU))
    #define SW_PACK_DEPTH_2(d)  ((uint8_t)((uint32_t)((d)*SW_DEPTH_MAX)&0xFFU))
    #define SW_UNPACK_DEPTH(p)  ((((uint32_t)(p)[0]<<16)|((uint32_t)(p)[1]<<8)|(uint32_t)(p)[2]))
#else // 32 bits
    #define SW_DEPTH_TYPE       float
    #define SW_DEPTH_IS_PACKED  1
    #define SW_DEPTH_PACK_COMP  1
    #define SW_DEPTH_MAX        1.0f
    #define SW_DEPTH_SCALE      1.0f
    #define SW_PACK_DEPTH(d)    ((SW_DEPTH_TYPE)(d))
    #define SW_UNPACK_DEPTH(p)  (p)
#endif

#define SW_STATE_CHECK(flags)   (SW_STATE_CHECK_EX(RLSW.stateFlags, (flags)))
#define SW_STATE_CHECK_EX(state, flags) (((state) & (flags)) == (flags))

#define SW_STATE_SCISSOR_TEST   (1 << 0)
#define SW_STATE_TEXTURE_2D     (1 << 1)
#define SW_STATE_DEPTH_TEST     (1 << 2)
#define SW_STATE_CULL_FACE      (1 << 3)
#define SW_STATE_BLEND          (1 << 4)

//----------------------------------------------------------------------------------
// Module Types and Structures Definition
//----------------------------------------------------------------------------------
// Pixel data format type
// NOTE: Enum aligned with raylib PixelFormat
typedef enum {
    SW_PIXELFORMAT_UNKNOWN = 0,
    SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,         // 8 bit per pixel (no alpha)
    SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,        // 8*2 bpp (2 channels)
    SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
    SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
    SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
    SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
    SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
    SW_PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
    SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
    SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
    SW_PIXELFORMAT_UNCOMPRESSED_R16,               // 16 bpp (1 channel - half float)
    SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16,         // 16*3 bpp (3 channels - half float)
    SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,      // 16*4 bpp (4 channels - half float)
} sw_pixelformat_t;

typedef void (*sw_factor_f)(
    float *SW_RESTRICT factor,
    const float *SW_RESTRICT src,
    const float *SW_RESTRICT dst
);

typedef float sw_matrix_t[4*4];
typedef uint16_t sw_half_t;

typedef struct {
    float position[4];          // Position coordinates
    float texcoord[2];          // Texture coordinates
    float color[4];             // Color value (RGBA)

    float homogeneous[4];       // Homogeneous coordinates
    float screen[2];            // Screen coordinates
} sw_vertex_t;

typedef struct {
    uint8_t *pixels;            // Texture pixels (RGBA32)

    int width, height;          // Dimensions of the texture
    int wMinus1, hMinus1;       // Dimensions minus one

    SWfilter minFilter;         // Minification filter
    SWfilter magFilter;         // Magnification filter

    SWwrap sWrap;               // texcoord.x wrap mode
    SWwrap tWrap;               // texcoord.y wrap mode

    float tx;                   // Texel width
    float ty;                   // Texel height
} sw_texture_t;

// Pixel data type
typedef SW_ALIGN(SW_PIXEL_ALIGNMENT) struct {
    SW_COLOR_TYPE color[SW_COLOR_PACK_COMP];
    SW_DEPTH_TYPE depth[SW_DEPTH_PACK_COMP];
#if (SW_PIXEL_SIZE % SW_PIXEL_ALIGNMENT != 0)
    uint8_t padding[SW_PIXEL_ALIGNMENT - SW_PIXEL_SIZE % SW_PIXEL_ALIGNMENT];
#endif
} sw_pixel_t;

typedef struct {
    sw_pixel_t *pixels;
    int width;
    int height;
    int allocSz;
} sw_framebuffer_t;

typedef struct {
    sw_framebuffer_t framebuffer;   // Main framebuffer
    sw_pixel_t clearValue;          // Clear value of the framebuffer

    float vpCenter[2];              // Viewport center
    float vpHalf[2];                // Viewport half dimensions
    int vpSize[2];                  // Viewport dimensions (minus one)
    int vpMin[2];                   // Viewport minimum renderable point (top-left)
    int vpMax[2];                   // Viewport maximum renderable point (bottom-right)

    int scMin[2];                   // Scissor rectangle minimum renderable point (top-left)
    int scMax[2];                   // Scissor rectangle maximum renderable point (bottom-right)
    float scClipMin[2];             // Scissor rectangle minimum renderable point in clip space
    float scClipMax[2];             // Scissor rectangle maximum renderable point in clip space

    uint32_t currentTexture;        // Current active texture id

    struct {
        float *positions;
        float *texcoords;
        uint8_t *colors;
    } array;

    struct {
        float texcoord[2];
        float color[4];
    } current;

    sw_vertex_t vertexBuffer[SW_MAX_CLIPPED_POLYGON_VERTICES];  // Buffer used for storing primitive vertices, used for processing and rendering
    int vertexCounter;                                          // Number of vertices in 'ctx.vertexBuffer'

    SWdraw drawMode;                                            // Current primitive mode (e.g., lines, triangles)
    SWpoly polyMode;                                            // Current polygon filling mode (e.g., lines, triangles)
    int reqVertices;                                            // Number of vertices required for the primitive being drawn
    float pointRadius;                                          // Rasterized point radius
    float lineWidth;                                            // Rasterized line width

    sw_matrix_t stackProjection[SW_MAX_PROJECTION_STACK_SIZE];  // Projection matrix stack for push/pop operations
    sw_matrix_t stackModelview[SW_MAX_MODELVIEW_STACK_SIZE];    // Modelview matrix stack for push/pop operations
    sw_matrix_t stackTexture[SW_MAX_TEXTURE_STACK_SIZE];        // Texture matrix stack for push/pop operations
    uint32_t stackProjectionCounter;                            // Counter for matrix stack operations
    uint32_t stackModelviewCounter;                             // Counter for matrix stack operations
    uint32_t stackTextureCounter;                               // Counter for matrix stack operations
    SWmatrix currentMatrixMode;                                 // Current matrix mode (e.g., sw_MODELVIEW, sw_PROJECTION)
    sw_matrix_t *currentMatrix;                                 // Pointer to the currently used matrix according to the mode
    sw_matrix_t matMVP;                                         // Model view projection matrix, calculated and used internally
    bool isDirtyMVP;                                            // Indicates if the MVP matrix should be rebuilt

    SWfactor srcFactor;
    SWfactor dstFactor;

    sw_factor_f srcFactorFunc;
    sw_factor_f dstFactorFunc;

    SWface cullFace;                                            // Faces to cull
    SWerrcode errCode;                                          // Last error code

    sw_texture_t *loadedTextures;
    int loadedTextureCount;

    uint32_t *freeTextureIds;
    int freeTextureIdCount;

    uint32_t stateFlags;
} sw_context_t;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static sw_context_t RLSW = { 0 };

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static inline void sw_matrix_id(sw_matrix_t dst)
{
    dst[0]  = 1, dst[1]  = 0, dst[2]  = 0, dst[3]  = 0;
    dst[4]  = 0, dst[5]  = 1, dst[6]  = 0, dst[7]  = 0;
    dst[8]  = 0, dst[9]  = 0, dst[10] = 1, dst[11] = 0;
    dst[12] = 0, dst[13] = 0, dst[14] = 0, dst[15] = 1;
}

static inline void sw_matrix_mul_rst(float *SW_RESTRICT dst, const float *SW_RESTRICT left, const float *SW_RESTRICT right)
{
    float l00 = left[0],  l01 = left[1],  l02 = left[2],  l03 = left[3];
    float l10 = left[4],  l11 = left[5],  l12 = left[6],  l13 = left[7];
    float l20 = left[8],  l21 = left[9],  l22 = left[10], l23 = left[11];
    float l30 = left[12], l31 = left[13], l32 = left[14], l33 = left[15];

    dst[0]  = l00*right[0] + l01*right[4] + l02*right[8]  + l03*right[12];
    dst[4]  = l10*right[0] + l11*right[4] + l12*right[8]  + l13*right[12];
    dst[8]  = l20*right[0] + l21*right[4] + l22*right[8]  + l23*right[12];
    dst[12] = l30*right[0] + l31*right[4] + l32*right[8]  + l33*right[12];

    dst[1]  = l00*right[1] + l01*right[5] + l02*right[9]  + l03*right[13];
    dst[5]  = l10*right[1] + l11*right[5] + l12*right[9]  + l13*right[13];
    dst[9]  = l20*right[1] + l21*right[5] + l22*right[9]  + l23*right[13];
    dst[13] = l30*right[1] + l31*right[5] + l32*right[9]  + l33*right[13];

    dst[2]  = l00*right[2] + l01*right[6] + l02*right[10] + l03*right[14];
    dst[6]  = l10*right[2] + l11*right[6] + l12*right[10] + l13*right[14];
    dst[10] = l20*right[2] + l21*right[6] + l22*right[10] + l23*right[14];
    dst[14] = l30*right[2] + l31*right[6] + l32*right[10] + l33*right[14];

    dst[3]  = l00*right[3] + l01*right[7] + l02*right[11] + l03*right[15];
    dst[7]  = l10*right[3] + l11*right[7] + l12*right[11] + l13*right[15];
    dst[11] = l20*right[3] + l21*right[7] + l22*right[11] + l23*right[15];
    dst[15] = l30*right[3] + l31*right[7] + l32*right[11] + l33*right[15];
}

static inline void sw_matrix_mul(sw_matrix_t dst, const sw_matrix_t left, const sw_matrix_t right)
{
    float result[16];

    sw_matrix_mul_rst(result, left, right);

    for (int i = 0; i < 16; i++) dst[i] = result[i];
}

static inline float sw_saturate(float x)
{
    union { float f; uint32_t u; } fb;
    fb.f = x;

    // Check if x < 0.0f
    // If sign bit is set (MSB), x is negative
    if ((fb.u & 0x80000000) != 0) return 0.0f;

    // Check if x > 1.0f
    // Works for positive floats: IEEE 754 ordering matches integer ordering
    if (fb.u > 0x3F800000) return 1.0f;

    // x is in [0.0f, 1.0f]
    return x;
}

static inline float sw_fract(float x)
{
    return (x - floorf(x));
}

static inline int sw_clampi(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static inline void sw_lerp_vertex_PTCH(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT a, const sw_vertex_t *SW_RESTRICT b, float t)
{
    const float tInv = 1.0f - t;

    // Position interpolation (4 components)
    out->position[0] = a->position[0]*tInv + b->position[0]*t;
    out->position[1] = a->position[1]*tInv + b->position[1]*t;
    out->position[2] = a->position[2]*tInv + b->position[2]*t;
    out->position[3] = a->position[3]*tInv + b->position[3]*t;

    // Texture coordinate interpolation (2 components)
    out->texcoord[0] = a->texcoord[0]*tInv + b->texcoord[0]*t;
    out->texcoord[1] = a->texcoord[1]*tInv + b->texcoord[1]*t;

    // Color interpolation (4 components)
    out->color[0] = a->color[0]*tInv + b->color[0]*t;
    out->color[1] = a->color[1]*tInv + b->color[1]*t;
    out->color[2] = a->color[2]*tInv + b->color[2]*t;
    out->color[3] = a->color[3]*tInv + b->color[3]*t;

    // Homogeneous coordinate interpolation (4 components)
    out->homogeneous[0] = a->homogeneous[0]*tInv + b->homogeneous[0]*t;
    out->homogeneous[1] = a->homogeneous[1]*tInv + b->homogeneous[1]*t;
    out->homogeneous[2] = a->homogeneous[2]*tInv + b->homogeneous[2]*t;
    out->homogeneous[3] = a->homogeneous[3]*tInv + b->homogeneous[3]*t;
}

static inline void sw_get_vertex_grad_PTCH(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT a, const sw_vertex_t *SW_RESTRICT b, float scale)
{
    // Calculate gradients for Position
    out->position[0] = (b->position[0] - a->position[0])*scale;
    out->position[1] = (b->position[1] - a->position[1])*scale;
    out->position[2] = (b->position[2] - a->position[2])*scale;
    out->position[3] = (b->position[3] - a->position[3])*scale;

    // Calculate gradients for Texture coordinates
    out->texcoord[0] = (b->texcoord[0] - a->texcoord[0])*scale;
    out->texcoord[1] = (b->texcoord[1] - a->texcoord[1])*scale;

    // Calculate gradients for Color
    out->color[0] = (b->color[0] - a->color[0])*scale;
    out->color[1] = (b->color[1] - a->color[1])*scale;
    out->color[2] = (b->color[2] - a->color[2])*scale;
    out->color[3] = (b->color[3] - a->color[3])*scale;

    // Calculate gradients for Homogeneous coordinates
    out->homogeneous[0] = (b->homogeneous[0] - a->homogeneous[0])*scale;
    out->homogeneous[1] = (b->homogeneous[1] - a->homogeneous[1])*scale;
    out->homogeneous[2] = (b->homogeneous[2] - a->homogeneous[2])*scale;
    out->homogeneous[3] = (b->homogeneous[3] - a->homogeneous[3])*scale;
}

static inline void sw_add_vertex_grad_PTCH(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT gradients)
{
    // Add gradients to Position
    out->position[0] += gradients->position[0];
    out->position[1] += gradients->position[1];
    out->position[2] += gradients->position[2];
    out->position[3] += gradients->position[3];

    // Add gradients to Texture coordinates
    out->texcoord[0] += gradients->texcoord[0];
    out->texcoord[1] += gradients->texcoord[1];

    // Add gradients to Color
    out->color[0] += gradients->color[0];
    out->color[1] += gradients->color[1];
    out->color[2] += gradients->color[2];
    out->color[3] += gradients->color[3];

    // Add gradients to Homogeneous coordinates
    out->homogeneous[0] += gradients->homogeneous[0];
    out->homogeneous[1] += gradients->homogeneous[1];
    out->homogeneous[2] += gradients->homogeneous[2];
    out->homogeneous[3] += gradients->homogeneous[3];
}

static inline void sw_add_vertex_grad_scaled_PTCH(
    sw_vertex_t *SW_RESTRICT out,
    const sw_vertex_t *SW_RESTRICT gradients,
    float scale)
{
    // Add gradients to Position
    out->position[0] += gradients->position[0]*scale;
    out->position[1] += gradients->position[1]*scale;
    out->position[2] += gradients->position[2]*scale;
    out->position[3] += gradients->position[3]*scale;

    // Add gradients to Texture coordinates
    out->texcoord[0] += gradients->texcoord[0]*scale;
    out->texcoord[1] += gradients->texcoord[1]*scale;

    // Add gradients to Color
    out->color[0] += gradients->color[0]*scale;
    out->color[1] += gradients->color[1]*scale;
    out->color[2] += gradients->color[2]*scale;
    out->color[3] += gradients->color[3]*scale;

    // Add gradients to Homogeneous coordinates
    out->homogeneous[0] += gradients->homogeneous[0]*scale;
    out->homogeneous[1] += gradients->homogeneous[1]*scale;
    out->homogeneous[2] += gradients->homogeneous[2]*scale;
    out->homogeneous[3] += gradients->homogeneous[3]*scale;
}

static inline void sw_float_to_unorm8_simd(uint8_t dst[4], const float src[4])
{
#if defined(SW_HAS_NEON)
    float32x4_t values = vld1q_f32(src);
    float32x4_t scaled = vmulq_n_f32(values, 255.0f);
    int32x4_t clamped_s32 = vcvtq_s32_f32(scaled);  // f32 -> s32 (truncated)
    int16x4_t narrow16_s = vqmovn_s32(clamped_s32);
    int16x8_t combined16_s = vcombine_s16(narrow16_s, narrow16_s);
    uint8x8_t narrow8_u = vqmovun_s16(combined16_s);
    vst1_lane_u32((uint32_t*)dst, vreinterpret_u32_u8(narrow8_u), 0);
#elif defined(SW_HAS_SSE41)
    __m128 values = _mm_loadu_ps(src);
    __m128 scaled = _mm_mul_ps(values, _mm_set1_ps(255.0f));
    __m128i clamped = _mm_cvtps_epi32(scaled);      // f32 -> s32 (truncated)
    clamped = _mm_packus_epi32(clamped, clamped);   // s32 -> u16 (saturated < 0 to 0)
    clamped = _mm_packus_epi16(clamped, clamped);   // u16 -> u8 (saturated > 255 to 255)
    *(uint32_t*)dst = _mm_cvtsi128_si32(clamped);
#elif defined(SW_HAS_SSE2)
    __m128 values = _mm_loadu_ps(src);
    __m128 scaled = _mm_mul_ps(values, _mm_set1_ps(255.0f));
    __m128i clamped = _mm_cvtps_epi32(scaled);      // f32 -> s32 (truncated)
    clamped = _mm_packs_epi32(clamped, clamped);    // s32 -> s16 (saturated)
    clamped = _mm_packus_epi16(clamped, clamped);   // s16 -> u8 (saturated < 0 to 0)
    *(uint32_t*)dst = _mm_cvtsi128_si32(clamped);
#elif defined(SW_HAS_RVV)
    // TODO: Sample code generated by AI, needs testing and review
    // NOTE: RVV 1.0 specs define the use of __riscv_ prefix for instrinsic functions
    size_t vl = __riscv_vsetvl_e32m1(4); // Load up to 4 floats into a vector register
    vfloat32m1_t vsrc = __riscv_vle32_v_f32m1(src, vl); // Load float32 values

    // Clamp to [0.0f, 1.0f]
    vfloat32m1_t vzero = __riscv_vfmv_v_f_f32m1(0.0f, vl);
    vfloat32m1_t vone  = __riscv_vfmv_v_f_f32m1(1.0f, vl);
    vsrc = __riscv_vfmin_vv_f32m1(vsrc, vone, vl);
    vsrc = __riscv_vfmax_vv_f32m1(vsrc, vzero, vl);

    // Multiply by 255.0f and add 0.5f for rounding
    vfloat32m1_t vscaled = __riscv_vfmul_vf_f32m1(vsrc, 255.0f, vl);
    vscaled = __riscv_vfadd_vf_f32m1(vscaled, 0.5f, vl);

    // Convert to unsigned integer (truncate toward zero)
    vuint32m1_t vu32 = __riscv_vfcvt_xu_f_v_u32m1(vscaled, vl);

    // Narrow from u32 -> u8
    vuint8m1_t vu8 = __riscv_vnclipu_wx_u8m1(vu32, 0, vl); // Round toward zero
    __riscv_vse8_v_u8m1(dst, vu8, vl); // Store result
#else
    for (int i = 0; i < 4; i++)
    {
        float val = src[i]*255.0f;
        val = (val > 255.0f)? 255.0f : val;
        val = (val < 0.0f)? 0.0f : val;
        dst[i] = (uint8_t)val;
    }
#endif
}

static inline void sw_float_from_unorm8_simd(float dst[4], const uint8_t src[4])
{
#if defined(SW_HAS_NEON)
    uint8x8_t bytes8 = vld1_u8(src); // Reading 8 bytes, faster, but let's hope not hitting the end of the page (unlikely)...
    uint16x8_t bytes16 = vmovl_u8(bytes8);
    uint32x4_t ints = vmovl_u16(vget_low_u16(bytes16));
    float32x4_t floats = vcvtq_f32_u32(ints);
    floats = vmulq_n_f32(floats, SW_INV_255);
    vst1q_f32(dst, floats);
#elif defined(SW_HAS_SSE41)
    __m128i bytes = _mm_cvtsi32_si128(*(const uint32_t *)src);
    __m128i ints = _mm_cvtepu8_epi32(bytes);
    __m128 floats = _mm_cvtepi32_ps(ints);
    floats = _mm_mul_ps(floats, _mm_set1_ps(SW_INV_255));
    _mm_storeu_ps(dst, floats);
#elif defined(SW_HAS_SSE2)
    __m128i bytes = _mm_cvtsi32_si128(*(const uint32_t *)src);
    bytes = _mm_unpacklo_epi8(bytes, _mm_setzero_si128());
    __m128i ints = _mm_unpacklo_epi16(bytes, _mm_setzero_si128());
    __m128 floats = _mm_cvtepi32_ps(ints);
    floats = _mm_mul_ps(floats, _mm_set1_ps(SW_INV_255));
    _mm_storeu_ps(dst, floats);
#elif defined(SW_HAS_RVV)
    // TODO: Sample code generated by AI, needs testing and review
    size_t vl = __riscv_vsetvl_e8m1(4); // Set vector length for 8-bit input elements
    vuint8m1_t vsrc_u8 = __riscv_vle8_v_u8m1(src, vl); // Load 4 unsigned 8-bit integers
    vuint32m1_t vsrc_u32 = __riscv_vwcvt_xu_u_v_u32m1(vsrc_u8, vl); // Widen to 32-bit unsigned integers
    vfloat32m1_t vsrc_f32 = __riscv_vfcvt_f_xu_v_f32m1(vsrc_u32, vl); // Convert to float32
    vfloat32m1_t vnorm = __riscv_vfmul_vf_f32m1(vsrc_f32, SW_INV_255, vl); // Multiply by 1/255.0 to normalize
    __riscv_vse32_v_f32m1(dst, vnorm, vl); // Store result
#else
    dst[0] = (float)src[0]*SW_INV_255;
    dst[1] = (float)src[1]*SW_INV_255;
    dst[2] = (float)src[2]*SW_INV_255;
    dst[3] = (float)src[3]*SW_INV_255;
#endif
}

// Half conversion functions
static inline uint32_t sw_half_to_float_ui(uint16_t h)
{
    uint32_t s = (uint32_t)(h & 0x8000) << 16;
    int32_t em = h & 0x7fff;

    // bias exponent and pad mantissa with 0; 112 is relative exponent bias (127-15)
    int32_t r = (em + (112 << 10)) << 13;

    // denormal: flush to zero
    r = (em < (1 << 10))? 0 : r;

    // NOTE: infinity/NaN; NaN payload is preserved as a byproduct of unifying inf/nan cases
    // 112 is an exponent bias fixup; since it is already applied once, applying it twice converts 31 to 255
    r += (em >= (31 << 10))? (112 << 23) : 0;

    return s | r;
}

static inline float sw_half_to_float(sw_half_t y)
{
    union { float f; uint32_t i; } v = { .i = sw_half_to_float_ui(y) };

    return v.f;
}

static inline uint16_t sw_half_from_float_ui(uint32_t ui)
{
    int32_t s = (ui >> 16) & 0x8000;
    int32_t em = ui & 0x7fffffff;

    // Bias exponent and round to nearest; 112 is relative exponent bias (127-15)
    int32_t h = (em - (112 << 23) + (1 << 12)) >> 13;

    // Underflow: flush to zero; 113 encodes exponent -14
    h = (em < (113 << 23))? 0 : h;

    // Overflow: infinity; 143 encodes exponent 16
    h = (em >= (143 << 23))? 0x7c00 : h;

    // NOTE: NaN; all types of NaN aree converted to qNaN
    h = (em > (255 << 23))? 0x7e00 : h;

    return (uint16_t)(s | h);
}

static inline sw_half_t sw_half_from_float(float i)
{
    union { float f; uint32_t i; } v;
    v.f = i;
    return sw_half_from_float_ui(v.i);
}

// Framebuffer management functions
//-------------------------------------------------------------------------------------------
static inline bool sw_framebuffer_load(int w, int h)
{
    int size = w*h;

    RLSW.framebuffer.pixels = SW_MALLOC(sizeof(sw_pixel_t)*size);
    if (RLSW.framebuffer.pixels == NULL) return false;

    RLSW.framebuffer.width = w;
    RLSW.framebuffer.height = h;
    RLSW.framebuffer.allocSz = size;

    return true;
}

static inline bool sw_framebuffer_resize(int w, int h)
{
    int newSize = w*h;

    if (newSize <= RLSW.framebuffer.allocSz)
    {
        RLSW.framebuffer.width = w;
        RLSW.framebuffer.height = h;
        return true;
    }

    void *newPixels = SW_REALLOC(RLSW.framebuffer.pixels, sizeof(sw_pixel_t)*newSize);
    if (newPixels == NULL) return false;

    RLSW.framebuffer.pixels = newPixels;

    RLSW.framebuffer.width = w;
    RLSW.framebuffer.height = h;
    RLSW.framebuffer.allocSz = newSize;

    return true;
}

static inline void sw_framebuffer_read_color(float dst[4], const sw_pixel_t *src)
{
#if SW_COLOR_IS_PACKED
    SW_COLOR_TYPE pixel = src->color[0];
    dst[0] = SW_TO_FLOAT_R(SW_UNPACK_R(pixel));
    dst[1] = SW_TO_FLOAT_G(SW_UNPACK_G(pixel));
    dst[2] = SW_TO_FLOAT_B(SW_UNPACK_B(pixel));
    dst[3] = 1.0f;
#else
    sw_float_from_unorm8_simd(dst, src->color);
#endif
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const sw_pixel_t *src)
{
#if SW_COLOR_IS_PACKED
    SW_COLOR_TYPE pixel = src->color[0];
    dst[0] = SW_SCALE_R(SW_UNPACK_R(pixel));
    dst[1] = SW_SCALE_G(SW_UNPACK_G(pixel));
    dst[2] = SW_SCALE_B(SW_UNPACK_B(pixel));
    dst[3] = 255;
#else
    const SW_COLOR_TYPE *p = src->color;
    dst[0] = p[0];
    dst[1] = p[1];
    dst[2] = p[2];
    dst[3] = p[3];
#endif
}

static inline float sw_framebuffer_read_depth(const sw_pixel_t *src)
{
#if SW_DEPTH_IS_PACKED
    return src->depth[0]*SW_DEPTH_SCALE;
#else
    return SW_UNPACK_DEPTH(src->depth)*SW_DEPTH_SCALE;
#endif
}

static inline void sw_framebuffer_write_color(sw_pixel_t *dst, const float src[4])
{
#if SW_COLOR_IS_PACKED
    dst->color[0] = SW_PACK_COLOR(src[0], src[1], src[2]);
#else
    sw_float_to_unorm8_simd(dst->color, src);
#endif
}

static inline void sw_framebuffer_write_depth(sw_pixel_t *dst, float depth)
{
    depth = sw_saturate(depth); // REVIEW: An overflow can occur in certain circumstances with clipping, and needs to be reviewed...

#if SW_DEPTH_IS_PACKED
    dst->depth[0] = SW_PACK_DEPTH(depth);
#else
    dst->depth[0] = SW_PACK_DEPTH_0(depth);
    dst->depth[1] = SW_PACK_DEPTH_1(depth);
    dst->depth[2] = SW_PACK_DEPTH_2(depth);
#endif
}

static inline void sw_framebuffer_fill_color(sw_pixel_t *ptr, int size, const SW_COLOR_TYPE color[SW_COLOR_PACK_COMP])
{
    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int w = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            sw_pixel_t *row = ptr + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int x = 0; x < w; x++, row++)
            {
                for (int i = 0; i < SW_COLOR_PACK_COMP; i++) row->color[i] = color[i];
            }
        }
    }
    else
    {
        for (int i = 0; i < size; i++, ptr++)
        {
            for (int j = 0; j < SW_COLOR_PACK_COMP; j++) ptr->color[j] = color[j];
        }
    }
}

static inline void sw_framebuffer_fill_depth(sw_pixel_t *ptr, int size, const SW_DEPTH_TYPE depth[SW_DEPTH_PACK_COMP])
{
    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int w = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            sw_pixel_t *row = ptr + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int x = 0; x < w; x++, row++)
            {
                for (int i = 0; i < SW_DEPTH_PACK_COMP; i++) row->depth[i] = depth[i];
            }
        }
    }
    else
    {
        for (int i = 0; i < size; i++, ptr++)
        {
            for (int j = 0; j < SW_DEPTH_PACK_COMP; j++) ptr->depth[j] = depth[j];
        }
    }
}

static inline void sw_framebuffer_fill(sw_pixel_t *ptr, int size, sw_pixel_t value)
{
    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int w = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            sw_pixel_t *row = ptr + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int x = 0; x < w; x++, row++) *row = value;
        }
    }
    else
    {
        for (int i = 0; i < size; i++, ptr++) *ptr = value;
    }
}

static inline void sw_framebuffer_copy_fast(void* dst)
{
    int size = RLSW.framebuffer.width*RLSW.framebuffer.height;
    const sw_pixel_t *pixels = RLSW.framebuffer.pixels;

#if SW_COLOR_BUFFER_BITS == 8
    uint8_t *dst8 = (uint8_t*)dst;
    for (int i = 0; i < size; i++) dst8[i] = pixels[i].color[0];
#elif SW_COLOR_BUFFER_BITS == 16
    uint16_t *dst16 = (uint16_t*)dst;
    for (int i = 0; i < size; i++) dst16[i] = *(uint16_t*)pixels[i].color;
#else // 32 bits
    uint32_t *dst32 = (uint32_t*)dst;
    #if SW_GL_FRAMEBUFFER_COPY_BGRA
        for (int i = 0; i < size; i++)
        {
            const uint8_t *c = pixels[i].color;
            dst32[i] = (uint32_t)c[2] | ((uint32_t)c[1] << 8) | ((uint32_t)c[0] << 16) | ((uint32_t)c[3] << 24);
        }
    #else // RGBA
        for (int i = 0; i < size; i++) dst32[i] = *(uint32_t*)pixels[i].color;
    #endif
#endif
}

#define DEFINE_FRAMEBUFFER_COPY_BEGIN(name, DST_PTR_T)                          \
static inline void sw_framebuffer_copy_to_##name(int x, int y, int w, int h, DST_PTR_T *dst) \
{                                                                               \
    const int stride = RLSW.framebuffer.width;                                  \
    const sw_pixel_t *src = RLSW.framebuffer.pixels + (y*stride + x);           \
                                                                                \
    for (int iy = 0; iy < h; iy++) {                                            \
        const sw_pixel_t *line = src;                                           \
        for (int ix = 0; ix < w; ix++) {                                        \
            uint8_t color[4];                                                   \
            sw_framebuffer_read_color8(color, line);                            \

#define DEFINE_FRAMEBUFFER_COPY_END()                                           \
            ++line;                                                             \
        }                                                                       \
        src += stride;                                                          \
    }                                                                           \
}

DEFINE_FRAMEBUFFER_COPY_BEGIN(GRAYSCALE, uint8_t)
{
    // NTSC grayscale conversion: Y = 0.299R + 0.587G + 0.114B
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);
    *dst++ = gray;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_BEGIN(GRAYALPHA, uint8_t)
{
    // Convert RGB to grayscale using NTSC formula
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);

    dst[0] = gray;
    dst[1] = color[3]; // alpha

    dst += 2;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_BEGIN(R5G6B5, uint16_t)
{
    // Convert 8-bit RGB to 5:6:5 format
    uint8_t r5 = (color[0]*31 + 127)/255;
    uint8_t g6 = (color[1]*63 + 127)/255;
    uint8_t b5 = (color[2]*31 + 127)/255;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t rgb565 = (b5 << 11) | (g6 << 5) | r5;
#else // RGBA
    uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
#endif

    *dst++ = rgb565;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_BEGIN(R8G8B8, uint8_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = color[2];
    dst[1] = color[1];
    dst[2] = color[0];
#else // RGBA
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
#endif

    dst += 3;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_BEGIN(R5G5B5A1, uint16_t)
{
    uint8_t r5 = (color[0]*31 + 127)/255;
    uint8_t g5 = (color[1]*31 + 127)/255;
    uint8_t b5 = (color[2]*31 + 127)/255;
    uint8_t a1 = (color[3] >= 128)? 1 : 0;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t pixel = (b5 << 11) | (g5 << 6) | (r5 << 1) | a1;
#else // RGBA
    uint16_t pixel = (r5 << 11) | (g5 << 6) | (b5 << 1) | a1;
#endif

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_BEGIN(R4G4B4A4, uint16_t)
{
    uint8_t r4 = (color[0]*15 + 127)/255;
    uint8_t g4 = (color[1]*15 + 127)/255;
    uint8_t b4 = (color[2]*15 + 127)/255;
    uint8_t a4 = (color[3]*15 + 127)/255;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t pixel = (b4 << 12) | (g4 << 8) | (r4 << 4) | a4;
#else // RGBA
    uint16_t pixel = (r4 << 12) | (g4 << 8) | (b4 << 4) | a4;
#endif

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_BEGIN(R8G8B8A8, uint8_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = color[2];
    dst[1] = color[1];
    dst[2] = color[0];
#else // RGBA
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
#endif
    dst[3] = color[3];

    dst += 4;
}
DEFINE_FRAMEBUFFER_COPY_END()

#define DEFINE_FRAMEBUFFER_BLIT_BEGIN(name, DST_PTR_T)                          \
static inline void sw_framebuffer_blit_to_##name(                               \
    int xDst, int yDst, int wDst, int hDst,                                     \
    int xSrc, int ySrc, int wSrc, int hSrc,                                     \
    DST_PTR_T *dst)                                                             \
{                                                                               \
    const sw_pixel_t *srcBase = RLSW.framebuffer.pixels;                        \
    const int fbWidth = RLSW.framebuffer.width;                                 \
                                                                                \
    const uint32_t xScale = ((uint32_t)wSrc << 16)/(uint32_t)wDst;              \
    const uint32_t yScale = ((uint32_t)hSrc << 16)/(uint32_t)hDst;              \
                                                                                \
    for (int dy = 0; dy < hDst; dy++) {                                         \
        uint32_t yFix = ((uint32_t)ySrc << 16) + dy*yScale;                     \
        int sy = yFix >> 16;                                                    \
        const sw_pixel_t *srcLine = srcBase + sy*fbWidth + xSrc;                \
                                                                                \
        const sw_pixel_t *srcPtr = srcLine;                                     \
        for (int dx = 0; dx < wDst; dx++) {                                     \
            uint32_t xFix = dx*xScale;                                          \
            int sx = xFix >> 16;                                                \
            const sw_pixel_t *pixel = srcPtr + sx;                              \
            uint8_t color[4];                                                   \
            sw_framebuffer_read_color8(color, pixel);

#define DEFINE_FRAMEBUFFER_BLIT_END()                                           \
        }                                                                       \
    }                                                                           \
}

DEFINE_FRAMEBUFFER_BLIT_BEGIN(GRAYSCALE, uint8_t)
{
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);
    *dst++ = gray;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_BEGIN(GRAYALPHA, uint8_t)
{
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);

    dst[0] = gray;
    dst[1] = color[3]; // alpha

    dst += 2;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_BEGIN(R5G6B5, uint16_t)
{
    uint8_t r5 = (color[0]*31 + 127)/255;
    uint8_t g6 = (color[1]*63 + 127)/255;
    uint8_t b5 = (color[2]*31 + 127)/255;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t rgb565 = (b5 << 11) | (g6 << 5) | r5;
#else // RGBA
    uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
#endif

    *dst++ = rgb565;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_BEGIN(R8G8B8, uint8_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = color[2];
    dst[1] = color[1];
    dst[2] = color[0];
#else // RGBA
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
#endif

    dst += 3;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_BEGIN(R5G5B5A1, uint16_t)
{
    uint8_t r5 = (color[0]*31 + 127)/255;
    uint8_t g5 = (color[1]*31 + 127)/255;
    uint8_t b5 = (color[2]*31 + 127)/255;
    uint8_t a1 = (color[3] >= 128)? 1 : 0;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t pixel = (b5 << 11) | (g5 << 6) | (r5 << 1) | a1;
#else // RGBA
    uint16_t pixel = (r5 << 11) | (g5 << 6) | (b5 << 1) | a1;
#endif

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_BEGIN(R4G4B4A4, uint16_t)
{
    uint8_t r4 = (color[0]*15 + 127)/255;
    uint8_t g4 = (color[1]*15 + 127)/255;
    uint8_t b4 = (color[2]*15 + 127)/255;
    uint8_t a4 = (color[3]*15 + 127)/255;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t pixel = (b4 << 12) | (g4 << 8) | (r4 << 4) | a4;
#else // RGBA
    uint16_t pixel = (r4 << 12) | (g4 << 8) | (b4 << 4) | a4;
#endif

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_BEGIN(R8G8B8A8, uint8_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = color[2];
    dst[1] = color[1];
    dst[2] = color[0];
#else // RGBA
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
#endif
    dst[3] = color[3];

    dst += 4;
}
DEFINE_FRAMEBUFFER_BLIT_END()
//-------------------------------------------------------------------------------------------

// Pixel format management functions
//-------------------------------------------------------------------------------------------
static inline int sw_get_pixel_format(SWformat format, SWtype type)
{
    int channels = 0;
    int bitsPerChannel = 8; // Default: 8 bits per channel

    // Determine the number of channels (format)
    switch (format)
    {
        case SW_LUMINANCE:        channels = 1; break;
        case SW_LUMINANCE_ALPHA:  channels = 2; break;
        case SW_RGB:              channels = 3; break;
        case SW_RGBA:             channels = 4; break;
        default: return SW_PIXELFORMAT_UNKNOWN;
    }

    // Determine the depth of each channel (type)
    switch (type)
    {
        case SW_UNSIGNED_BYTE:    bitsPerChannel = 8;  break;
        case SW_BYTE:             bitsPerChannel = 8;  break;
        case SW_UNSIGNED_SHORT:   bitsPerChannel = 16; break;
        case SW_SHORT:            bitsPerChannel = 16; break;
        case SW_UNSIGNED_INT:     bitsPerChannel = 32; break;
        case SW_INT:              bitsPerChannel = 32; break;
        case SW_FLOAT:            bitsPerChannel = 32; break;
        default: return SW_PIXELFORMAT_UNKNOWN;
    }

    // Map the format and type to the correct internal format
    if (bitsPerChannel == 8)
    {
        if (channels == 1) return SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
        if (channels == 2) return SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
        if (channels == 3) return SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8;
        if (channels == 4) return SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    }
    else if (bitsPerChannel == 16)
    {
        if (channels == 1) return SW_PIXELFORMAT_UNCOMPRESSED_R16;
        if (channels == 3) return SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16;
        if (channels == 4) return SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;
    }
    else if (bitsPerChannel == 32)
    {
        if (channels == 1) return SW_PIXELFORMAT_UNCOMPRESSED_R32;
        if (channels == 3) return SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32;
        if (channels == 4) return SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
    }

    return SW_PIXELFORMAT_UNKNOWN;
}

static inline void sw_get_pixel(uint8_t *color, const void *pixels, uint32_t offset, sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        {
            uint8_t gray = ((const uint8_t*)pixels)[offset];
            color[0] = gray;
            color[1] = gray;
            color[2] = gray;
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        {
            const uint8_t *src = &((const uint8_t*)pixels)[offset*2];
            color[0] = src[0];
            color[1] = src[0];
            color[2] = src[0];
            color[3] = src[1];
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        {
            uint16_t pixel = ((const uint16_t*)pixels)[offset];
            color[0] = ((pixel >> 11) & 0x1F)*255/31;   // R (5 bits)
            color[1] = ((pixel >> 5) & 0x3F)*255/63;    // G (6 bits)
            color[2] = (pixel & 0x1F)*255/31;           // B (5 bits)
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        {
            const uint8_t *src = &((const uint8_t*)pixels)[offset*3];
            color[0] = src[0];
            color[1] = src[1];
            color[2] = src[2];
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        {
            uint16_t pixel = ((const uint16_t*)pixels)[offset];
            color[0] = ((pixel >> 11) & 0x1F)*255/31;   // R (5 bits)
            color[1] = ((pixel >> 6) & 0x1F)*255/31;    // G (5 bits)
            color[2] = ((pixel >> 1) & 0x1F)*255/31;    // B (5 bits)
            color[3] = (pixel & 0x01)*255;              // A (1 bit)
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        {
            uint16_t pixel = ((const uint16_t*)pixels)[offset];
            color[0] = ((pixel >> 12) & 0x0F)*255/15;   // R (4 bits)
            color[1] = ((pixel >> 8) & 0x0F)*255/15;    // G (4 bits)
            color[2] = ((pixel >> 4) & 0x0F)*255/15;    // B (4 bits)
            color[3] = (pixel & 0x0F)*255/15;           // A (4 bits)
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        {
            const uint8_t *src = &((const uint8_t*)pixels)[offset*4];
            color[0] = src[0];
            color[1] = src[1];
            color[2] = src[2];
            color[3] = src[3];
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R32:
        {
            float val = ((const float*)pixels)[offset];
            uint8_t gray = (uint8_t)(val*255.0f);
            color[0] = gray;
            color[1] = gray;
            color[2] = gray;
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        {
            const float *src = &((const float*)pixels)[offset*3];
            color[0] = (uint8_t)(src[0]*255.0f);
            color[1] = (uint8_t)(src[1]*255.0f);
            color[2] = (uint8_t)(src[2]*255.0f);
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        {
            const float *src = &((const float*)pixels)[offset*4];
            color[0] = (uint8_t)(src[0]*255.0f);
            color[1] = (uint8_t)(src[1]*255.0f);
            color[2] = (uint8_t)(src[2]*255.0f);
            color[3] = (uint8_t)(src[3]*255.0f);
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R16:
        {
            uint16_t val = ((const uint16_t*)pixels)[offset];
            uint8_t gray = sw_half_to_float(val)*SW_INV_255;
            color[0] = gray;
            color[1] = gray;
            color[2] = gray;
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        {
            const uint16_t *src = &((const uint16_t*)pixels)[offset*3];
            color[0] = sw_half_to_float(src[0])*SW_INV_255;
            color[1] = sw_half_to_float(src[1])*SW_INV_255;
            color[2] = sw_half_to_float(src[2])*SW_INV_255;
            color[3] = 255;
            break;
        }
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        {
            const uint16_t *src = &((const uint16_t*)pixels)[offset*4];
            color[0] = sw_half_to_float(src[0])*SW_INV_255;
            color[1] = sw_half_to_float(src[1])*SW_INV_255;
            color[2] = sw_half_to_float(src[2])*SW_INV_255;
            color[3] = sw_half_to_float(src[3])*SW_INV_255;
            break;
        }
        case SW_PIXELFORMAT_UNKNOWN:
        default:
        {
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            color[3] = 0;
            break;
        }
    }
}
//-------------------------------------------------------------------------------------------

// Texture sampling functionality
//-------------------------------------------------------------------------------------------
static inline void sw_texture_fetch(float* color, const sw_texture_t* tex, int x, int y)
{
    sw_float_from_unorm8_simd(color, &tex->pixels[4*(y*tex->width + x)]);
}

static inline void sw_texture_sample_nearest(float *color, const sw_texture_t *tex, float u, float v)
{
    u = (tex->sWrap == SW_REPEAT)? sw_fract(u) : sw_saturate(u);
    v = (tex->tWrap == SW_REPEAT)? sw_fract(v) : sw_saturate(v);

    int x = u*tex->width;
    int y = v*tex->height;

    sw_texture_fetch(color, tex, x, y);
}

static inline void sw_texture_sample_linear(float *color, const sw_texture_t *tex, float u, float v)
{
    // TODO: With a bit more cleverness thee number of operations can
    // be clearly reduced, but for now it works fine

    float xf = (u*tex->width) - 0.5f;
    float yf = (v*tex->height) - 0.5f;

    float fx = sw_fract(xf);
    float fy = sw_fract(yf);

    int x0 = (int)xf;
    int y0 = (int)yf;

    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // NOTE: If the textures are POT, avoid the division for SW_REPEAT

    if (tex->sWrap == SW_CLAMP)
    {
        x0 = (x0 > tex->wMinus1)? tex->wMinus1 : x0;
        x1 = (x1 > tex->wMinus1)? tex->wMinus1 : x1;
    }
    else
    {
        x0 = (x0%tex->width + tex->width)%tex->width;
        x1 = (x1%tex->width + tex->width)%tex->width;
    }

    if (tex->tWrap == SW_CLAMP)
    {
        y0 = (y0 > tex->hMinus1)? tex->hMinus1 : y0;
        y1 = (y1 > tex->hMinus1)? tex->hMinus1 : y1;
    }
    else
    {
        y0 = (y0%tex->height + tex->height)%tex->height;
        y1 = (y1%tex->height + tex->height)%tex->height;
    }

    float c00[4], c10[4], c01[4], c11[4];
    sw_texture_fetch(c00, tex, x0, y0);
    sw_texture_fetch(c10, tex, x1, y0);
    sw_texture_fetch(c01, tex, x0, y1);
    sw_texture_fetch(c11, tex, x1, y1);

    for (int i = 0; i < 4; i++)
    {
        float t = c00[i] + fx*(c10[i] - c00[i]);
        float b = c01[i] + fx*(c11[i] - c01[i]);
        color[i] = t + fy*(b - t);
    }
}

static inline void sw_texture_sample(float *color, const sw_texture_t *tex, float u, float v, float dUdx, float dUdy, float dVdx, float dVdy)
{
    // Previous method: There is no need to compute the square root
    // because using the squared value, the comparison remains (L2 > 1.0f*1.0f)
    //float du = sqrtf(dUdx*dUdx + dUdy*dUdy);
    //float dv = sqrtf(dVdx*dVdx + dVdy*dVdy);
    //float L = (du > dv)? du : dv;

    // Calculate the derivatives for each axis
    float dU2 = dUdx*dUdx + dUdy*dUdy;
    float dV2 = dVdx*dVdx + dVdy*dVdy;
    float L2 = (dU2 > dV2)? dU2 : dV2;

    SWfilter filter = (L2 > 1.0f)? tex->minFilter : tex->magFilter;

    switch (filter)
    {
        case SW_NEAREST: sw_texture_sample_nearest(color, tex, u, v); break;
        case SW_LINEAR: sw_texture_sample_linear(color, tex, u, v); break;
        default: break;
    }
}
//-------------------------------------------------------------------------------------------

// Color blending functionality
//-------------------------------------------------------------------------------------------
static inline void sw_factor_zero(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = 0.0f;
}

static inline void sw_factor_one(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = 1.0f;
}

static inline void sw_factor_src_color(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = src[0]; factor[1] = src[1]; factor[2] = src[2]; factor[3] = src[3];
}

static inline void sw_factor_one_minus_src_color(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = 1.0f - src[0]; factor[1] = 1.0f - src[1];
    factor[2] = 1.0f - src[2]; factor[3] = 1.0f - src[3];
}

static inline void sw_factor_src_alpha(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = src[3];
}

static inline void sw_factor_one_minus_src_alpha(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    float invAlpha = 1.0f - src[3];
    factor[0] = factor[1] = factor[2] = factor[3] = invAlpha;
}

static inline void sw_factor_dst_alpha(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = dst[3];
}

static inline void sw_factor_one_minus_dst_alpha(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    float invAlpha = 1.0f - dst[3];
    factor[0] = factor[1] = factor[2] = factor[3] = invAlpha;
}

static inline void sw_factor_dst_color(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = dst[0]; factor[1] = dst[1]; factor[2] = dst[2]; factor[3] = dst[3];
}

static inline void sw_factor_one_minus_dst_color(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = 1.0f - dst[0]; factor[1] = 1.0f - dst[1];
    factor[2] = 1.0f - dst[2]; factor[3] = 1.0f - dst[3];
}

static inline void sw_factor_src_alpha_saturate(float *SW_RESTRICT factor, const float *SW_RESTRICT src, const float *SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = 1.0f;
    factor[3] = (src[3] < 1.0f)? src[3] : 1.0f;
}

static inline void sw_blend_colors(float *SW_RESTRICT dst/*[4]*/, const float *SW_RESTRICT src/*[4]*/)
{
    float srcFactor[4], dstFactor[4];

    RLSW.srcFactorFunc(srcFactor, src, dst);
    RLSW.dstFactorFunc(dstFactor, src, dst);

    dst[0] = srcFactor[0]*src[0] + dstFactor[0]*dst[0];
    dst[1] = srcFactor[1]*src[1] + dstFactor[1]*dst[1];
    dst[2] = srcFactor[2]*src[2] + dstFactor[2]*dst[2];
    dst[3] = srcFactor[3]*src[3] + dstFactor[3]*dst[3];
}
//-------------------------------------------------------------------------------------------

// Projection helper functions
//-------------------------------------------------------------------------------------------
static inline void sw_project_ndc_to_screen(float screen[2], const float ndc[4])
{
    screen[0] = RLSW.vpCenter[0] + ndc[0]*RLSW.vpHalf[0] + 0.5f;
    screen[1] = RLSW.vpCenter[1] - ndc[1]*RLSW.vpHalf[1] + 0.5f;
}
//-------------------------------------------------------------------------------------------

// Polygon clipping management
//-------------------------------------------------------------------------------------------
#define DEFINE_CLIP_FUNC(name, FUNC_IS_INSIDE, FUNC_COMPUTE_T)                          \
static inline int sw_clip_##name(                                                       \
    sw_vertex_t output[SW_MAX_CLIPPED_POLYGON_VERTICES],                                \
    const sw_vertex_t input[SW_MAX_CLIPPED_POLYGON_VERTICES],                           \
    int n)                                                                              \
{                                                                                       \
    const sw_vertex_t *prev = &input[n - 1];                                            \
    int prevInside = FUNC_IS_INSIDE(prev->homogeneous);                                 \
    int outputCount = 0;                                                                \
                                                                                        \
    for (int i = 0; i < n; i++) {                                                       \
        const sw_vertex_t *curr = &input[i];                                            \
        int currInside = FUNC_IS_INSIDE(curr->homogeneous);                             \
                                                                                        \
        /* If transition between interior/exterior, calculate intersection point */     \
        if (prevInside != currInside) {                                                 \
            float t = FUNC_COMPUTE_T(prev->homogeneous, curr->homogeneous);             \
            sw_lerp_vertex_PTCH(&output[outputCount++], prev, curr, t);                 \
        }                                                                               \
                                                                                        \
        /* If current vertex inside, add it */                                          \
        if (currInside) {                                                               \
            output[outputCount++] = *curr;                                              \
        }                                                                               \
                                                                                        \
        prev = curr;                                                                    \
        prevInside = currInside;                                                        \
    }                                                                                   \
                                                                                        \
    return outputCount;                                                                 \
}
//-------------------------------------------------------------------------------------------

// Frustum cliping functions
//-------------------------------------------------------------------------------------------
#define IS_INSIDE_PLANE_W(h) ((h)[3] >= SW_CLIP_EPSILON)
#define IS_INSIDE_PLANE_X_POS(h) ( (h)[0] <  (h)[3])        // Exclusive for +X
#define IS_INSIDE_PLANE_X_NEG(h) (-(h)[0] <  (h)[3])        // Exclusive for -X
#define IS_INSIDE_PLANE_Y_POS(h) ( (h)[1] <  (h)[3])        // Exclusive for +Y
#define IS_INSIDE_PLANE_Y_NEG(h) (-(h)[1] <  (h)[3])        // Exclusive for -Y
#define IS_INSIDE_PLANE_Z_POS(h) ( (h)[2] <= (h)[3])        // Inclusive for +Z
#define IS_INSIDE_PLANE_Z_NEG(h) (-(h)[2] <= (h)[3])        // Inclusive for -Z

#define COMPUTE_T_PLANE_W(hPrev, hCurr) ((SW_CLIP_EPSILON - (hPrev)[3])/((hCurr)[3] - (hPrev)[3]))
#define COMPUTE_T_PLANE_X_POS(hPrev, hCurr) (((hPrev)[3] - (hPrev)[0])/(((hPrev)[3] - (hPrev)[0]) - ((hCurr)[3] - (hCurr)[0])))
#define COMPUTE_T_PLANE_X_NEG(hPrev, hCurr) (((hPrev)[3] + (hPrev)[0])/(((hPrev)[3] + (hPrev)[0]) - ((hCurr)[3] + (hCurr)[0])))
#define COMPUTE_T_PLANE_Y_POS(hPrev, hCurr) (((hPrev)[3] - (hPrev)[1])/(((hPrev)[3] - (hPrev)[1]) - ((hCurr)[3] - (hCurr)[1])))
#define COMPUTE_T_PLANE_Y_NEG(hPrev, hCurr) (((hPrev)[3] + (hPrev)[1])/(((hPrev)[3] + (hPrev)[1]) - ((hCurr)[3] + (hCurr)[1])))
#define COMPUTE_T_PLANE_Z_POS(hPrev, hCurr) (((hPrev)[3] - (hPrev)[2])/(((hPrev)[3] - (hPrev)[2]) - ((hCurr)[3] - (hCurr)[2])))
#define COMPUTE_T_PLANE_Z_NEG(hPrev, hCurr) (((hPrev)[3] + (hPrev)[2])/(((hPrev)[3] + (hPrev)[2]) - ((hCurr)[3] + (hCurr)[2])))

DEFINE_CLIP_FUNC(w, IS_INSIDE_PLANE_W, COMPUTE_T_PLANE_W)
DEFINE_CLIP_FUNC(x_pos, IS_INSIDE_PLANE_X_POS, COMPUTE_T_PLANE_X_POS)
DEFINE_CLIP_FUNC(x_neg, IS_INSIDE_PLANE_X_NEG, COMPUTE_T_PLANE_X_NEG)
DEFINE_CLIP_FUNC(y_pos, IS_INSIDE_PLANE_Y_POS, COMPUTE_T_PLANE_Y_POS)
DEFINE_CLIP_FUNC(y_neg, IS_INSIDE_PLANE_Y_NEG, COMPUTE_T_PLANE_Y_NEG)
DEFINE_CLIP_FUNC(z_pos, IS_INSIDE_PLANE_Z_POS, COMPUTE_T_PLANE_Z_POS)
DEFINE_CLIP_FUNC(z_neg, IS_INSIDE_PLANE_Z_NEG, COMPUTE_T_PLANE_Z_NEG)
//-------------------------------------------------------------------------------------------

// Scissor clip functions
//-------------------------------------------------------------------------------------------
#define COMPUTE_T_SCISSOR_X_MIN(hPrev, hCurr) (((RLSW.scClipMin[0])*(hPrev)[3] - (hPrev)[0])/(((hCurr)[0] - (RLSW.scClipMin[0])*(hCurr)[3]) - ((hPrev)[0] - (RLSW.scClipMin[0])*(hPrev)[3])))
#define COMPUTE_T_SCISSOR_X_MAX(hPrev, hCurr) (((RLSW.scClipMax[0])*(hPrev)[3] - (hPrev)[0])/(((hCurr)[0] - (RLSW.scClipMax[0])*(hCurr)[3]) - ((hPrev)[0] - (RLSW.scClipMax[0])*(hPrev)[3])))
#define COMPUTE_T_SCISSOR_Y_MIN(hPrev, hCurr) (((RLSW.scClipMin[1])*(hPrev)[3] - (hPrev)[1])/(((hCurr)[1] - (RLSW.scClipMin[1])*(hCurr)[3]) - ((hPrev)[1] - (RLSW.scClipMin[1])*(hPrev)[3])))
#define COMPUTE_T_SCISSOR_Y_MAX(hPrev, hCurr) (((RLSW.scClipMax[1])*(hPrev)[3] - (hPrev)[1])/(((hCurr)[1] - (RLSW.scClipMax[1])*(hCurr)[3]) - ((hPrev)[1] - (RLSW.scClipMax[1])*(hPrev)[3])))

#define IS_INSIDE_SCISSOR_X_MIN(h) ((h)[0] >= (RLSW.scClipMin[0])*(h)[3])
#define IS_INSIDE_SCISSOR_X_MAX(h) ((h)[0] <= (RLSW.scClipMax[0])*(h)[3])
#define IS_INSIDE_SCISSOR_Y_MIN(h) ((h)[1] >= (RLSW.scClipMin[1])*(h)[3])
#define IS_INSIDE_SCISSOR_Y_MAX(h) ((h)[1] <= (RLSW.scClipMax[1])*(h)[3])

DEFINE_CLIP_FUNC(scissor_x_min, IS_INSIDE_SCISSOR_X_MIN, COMPUTE_T_SCISSOR_X_MIN)
DEFINE_CLIP_FUNC(scissor_x_max, IS_INSIDE_SCISSOR_X_MAX, COMPUTE_T_SCISSOR_X_MAX)
DEFINE_CLIP_FUNC(scissor_y_min, IS_INSIDE_SCISSOR_Y_MIN, COMPUTE_T_SCISSOR_Y_MIN)
DEFINE_CLIP_FUNC(scissor_y_max, IS_INSIDE_SCISSOR_Y_MAX, COMPUTE_T_SCISSOR_Y_MAX)
//-------------------------------------------------------------------------------------------

// Main polygon clip function
static inline bool sw_polygon_clip(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int *vertexCounter)
{
    static sw_vertex_t tmp[SW_MAX_CLIPPED_POLYGON_VERTICES];

    int n = *vertexCounter;

    #define CLIP_AGAINST_PLANE(FUNC_CLIP)                       \
    {                                                           \
        n = FUNC_CLIP(tmp, polygon, n);                         \
        if (n < 3)                                              \
        {                                                       \
            *vertexCounter = 0;                                 \
            return false;                                       \
        }                                                       \
        for (int i = 0; i < n; i++) polygon[i] = tmp[i];        \
    }

    CLIP_AGAINST_PLANE(sw_clip_w);
    CLIP_AGAINST_PLANE(sw_clip_x_pos);
    CLIP_AGAINST_PLANE(sw_clip_x_neg);
    CLIP_AGAINST_PLANE(sw_clip_y_pos);
    CLIP_AGAINST_PLANE(sw_clip_y_neg);
    CLIP_AGAINST_PLANE(sw_clip_z_pos);
    CLIP_AGAINST_PLANE(sw_clip_z_neg);

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        CLIP_AGAINST_PLANE(sw_clip_scissor_x_min);
        CLIP_AGAINST_PLANE(sw_clip_scissor_x_max);
        CLIP_AGAINST_PLANE(sw_clip_scissor_y_min);
        CLIP_AGAINST_PLANE(sw_clip_scissor_y_max);
    }

    *vertexCounter = n;

    return (n >= 3);
}

// Triangle rendering logic
//-------------------------------------------------------------------------------------------
static inline bool sw_triangle_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations
    // To handle triangles crossing the w=0 plane correctly,
    // the winding order test is performeed in homogeneous coordinates directly,
    // before the perspective division (division by w)
    // This test determines the orientation of the triangle in the (x,y,w) plane,
    // which corresponds to the projected 2D winding order sign,
    // even with negative w values

    // Preload homogeneous coordinates into local variables
    const float *h0 = RLSW.vertexBuffer[0].homogeneous;
    const float *h1 = RLSW.vertexBuffer[1].homogeneous;
    const float *h2 = RLSW.vertexBuffer[2].homogeneous;

    // Compute a value proportional to the signed area in the projected 2D plane,
    // calculated directly using homogeneous coordinates BEFORE division by w
    // This is the determinant of the matrix formed by the (x, y, w) components
    // of the vertices, which correctly captures the winding order in homogeneous
    // space and its relationship to the projected 2D winding order, even with
    // negative w values
    // The determinant formula used here is:
    // h0.x*(h1.y*h2.w - h2.y*h1.w) +
    // h1.x*(h2.y*h0.w - h0.y*h2.w) +
    // h2.x*(h0.y*h1.w - h1.y*h0.w)

    const float hSgnArea =
        h0[0]*(h1[1]*h2[3] - h2[1]*h1[3]) +
        h1[0]*(h2[1]*h0[3] - h0[1]*h2[3]) +
        h2[0]*(h0[1]*h1[3] - h1[1]*h0[3]);

    // Discard the triangle if its winding order (determined by the sign
    // of the homogeneous area/determinant) matches the culled direction
    // A positive hSgnArea typically corresponds to a counter-clockwise
    // winding in the projected space when all w > 0
    // This test is robust for points with w > 0 or w < 0, correctly
    // capturing the change in orientation when crossing the w=0 plane

    // The culling logic remains the same based on the signed area/determinant
    // A value of 0 for hSgnArea means the points are collinear in (x, y, w)
    // space, which corresponds to a degenerate triangle projection
    // Such triangles are typically not culled by this test (0 < 0 is false, 0 > 0 is false)
    // and should be handled by the clipper if necessary
    return (RLSW.cullFace == SW_FRONT)? (hSgnArea < 0) : (hSgnArea > 0); // Cull if winding is "clockwise" : "counter-clockwise"
}

static inline void sw_triangle_clip_and_project(void)
{
    sw_vertex_t *polygon = RLSW.vertexBuffer;
    int *vertexCounter = &RLSW.vertexCounter;

    if (sw_polygon_clip(polygon, vertexCounter))
    {
        // Transformation to screen space and normalization
        for (int i = 0; i < *vertexCounter; i++)
        {
            sw_vertex_t *v = &polygon[i];

            // Calculation of the reciprocal of W for normalization
            // as well as perspective-correct attributes
            const float wRcp = 1.0f/v->homogeneous[3];
            v->homogeneous[3] = wRcp;

            // Division of XYZ coordinates by weight
            v->homogeneous[0] *= wRcp;
            v->homogeneous[1] *= wRcp;
            v->homogeneous[2] *= wRcp;

            // Division of texture coordinates (perspective-correct)
            v->texcoord[0] *= wRcp;
            v->texcoord[1] *= wRcp;

            // Division of colors (perspective-correct)
            v->color[0] *= wRcp;
            v->color[1] *= wRcp;
            v->color[2] *= wRcp;
            v->color[3] *= wRcp;

            // Transformation to screen space
            sw_project_ndc_to_screen(v->screen, v->homogeneous);
        }
    }
}

#define DEFINE_TRIANGLE_RASTER_SCANLINE(FUNC_NAME, ENABLE_TEXTURE, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(const sw_texture_t *tex, const sw_vertex_t *start,     \
                             const sw_vertex_t *end, float dUdy, float dVdy)        \
{                                                                                   \
    /* Gets the start and end coordinates */                                        \
    int xStart = (int)start->screen[0];                                             \
    int xEnd   = (int)end->screen[0];                                               \
                                                                                    \
    /* Avoid empty lines */                                                         \
    if (xStart == xEnd) return;                                                     \
                                                                                    \
    /* Compute the subpixel distance to traverse before the first pixel */          \
    float xSubstep = 1.0f - sw_fract(start->screen[0]);                             \
                                                                                    \
    /* Compute the inverse horizontal distance along the X axis */                  \
    float dxRcp = 1.0f/(end->screen[0] - start->screen[0]);                         \
                                                                                    \
    /* Compute the interpolation steps along the X axis */                          \
    float dZdx = (end->homogeneous[2] - start->homogeneous[2])*dxRcp;               \
    float dWdx = (end->homogeneous[3] - start->homogeneous[3])*dxRcp;               \
                                                                                    \
    float dCdx[4] = {                                                               \
        (end->color[0] - start->color[0])*dxRcp,                                    \
        (end->color[1] - start->color[1])*dxRcp,                                    \
        (end->color[2] - start->color[2])*dxRcp,                                    \
        (end->color[3] - start->color[3])*dxRcp                                     \
    };                                                                              \
                                                                                    \
    float dUdx = 0.0f;                                                              \
    float dVdx = 0.0f;                                                              \
    if (ENABLE_TEXTURE) {                                                           \
        dUdx = (end->texcoord[0] - start->texcoord[0])*dxRcp;                       \
        dVdx = (end->texcoord[1] - start->texcoord[1])*dxRcp;                       \
    }                                                                               \
                                                                                    \
    /* Initializing the interpolation starting values  */                           \
    float z = start->homogeneous[2] + dZdx*xSubstep;                                \
    float w = start->homogeneous[3] + dWdx*xSubstep;                                \
                                                                                    \
    float color[4] = {                                                              \
        start->color[0] + dCdx[0]*xSubstep,                                         \
        start->color[1] + dCdx[1]*xSubstep,                                         \
        start->color[2] + dCdx[2]*xSubstep,                                         \
        start->color[3] + dCdx[3]*xSubstep                                          \
    };                                                                              \
                                                                                    \
    float u = 0.0f;                                                                 \
    float v = 0.0f;                                                                 \
    if (ENABLE_TEXTURE) {                                                           \
        u = start->texcoord[0] + dUdx*xSubstep;                                     \
        v = start->texcoord[1] + dVdx*xSubstep;                                     \
    }                                                                               \
                                                                                    \
    /* Pre-calculate the starting pointers for the framebuffer row */               \
    int y = (int)start->screen[1];                                                  \
    sw_pixel_t *ptr = RLSW.framebuffer.pixels + y*RLSW.framebuffer.width + xStart;  \
                                                                                    \
    /* Scanline rasterization */                                                    \
    for (int x = xStart; x < xEnd; x++)                                             \
    {                                                                               \
        float wRcp = 1.0f/w;                                                        \
        float srcColor[4] = {                                                       \
            color[0]*wRcp,                                                          \
            color[1]*wRcp,                                                          \
            color[2]*wRcp,                                                          \
            color[3]*wRcp                                                           \
        };                                                                          \
                                                                                    \
        if (ENABLE_DEPTH_TEST)                                                      \
        {                                                                           \
            /* TODO: Implement different depth funcs? */                            \
            float depth =  sw_framebuffer_read_depth(ptr);                          \
            if (z > depth) goto discard;                                            \
        }                                                                           \
                                                                                    \
        /* TODO: Implement depth mask */                                            \
        sw_framebuffer_write_depth(ptr, z);                                         \
                                                                                    \
        if (ENABLE_TEXTURE)                                                         \
        {                                                                           \
            float texColor[4];                                                      \
            float s = u*wRcp;                                                       \
            float t = v*wRcp;                                                       \
            sw_texture_sample(texColor, tex, s, t, dUdx, dUdy, dVdx, dVdy);         \
            srcColor[0] *= texColor[0];                                             \
            srcColor[1] *= texColor[1];                                             \
            srcColor[2] *= texColor[2];                                             \
            srcColor[3] *= texColor[3];                                             \
        }                                                                           \
                                                                                    \
        if (ENABLE_COLOR_BLEND)                                                     \
        {                                                                           \
            float dstColor[4];                                                      \
            sw_framebuffer_read_color(dstColor, ptr);                               \
            sw_blend_colors(dstColor, srcColor);                                    \
            sw_framebuffer_write_color(ptr, dstColor);                              \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            sw_framebuffer_write_color(ptr, srcColor);                              \
        }                                                                           \
                                                                                    \
        /* Increment the interpolation parameter, UVs, and pointers */              \
    discard:                                                                        \
        z += dZdx;                                                                  \
        w += dWdx;                                                                  \
        color[0] += dCdx[0];                                                        \
        color[1] += dCdx[1];                                                        \
        color[2] += dCdx[2];                                                        \
        color[3] += dCdx[3];                                                        \
        if (ENABLE_TEXTURE)                                                         \
        {                                                                           \
            u += dUdx;                                                              \
            v += dVdx;                                                              \
        }                                                                           \
        ++ptr;                                                                      \
    }                                                                               \
}

#define DEFINE_TRIANGLE_RASTER(FUNC_NAME, FUNC_SCANLINE, ENABLE_TEXTURE)            \
static inline void FUNC_NAME(const sw_vertex_t *v0, const sw_vertex_t *v1,          \
                             const sw_vertex_t *v2, const sw_texture_t *tex)        \
{                                                                                   \
    /* Swap vertices by increasing y */                                             \
    if (v0->screen[1] > v1->screen[1]) { const sw_vertex_t *tmp = v0; v0 = v1; v1 = tmp; } \
    if (v1->screen[1] > v2->screen[1]) { const sw_vertex_t *tmp = v1; v1 = v2; v2 = tmp; } \
    if (v0->screen[1] > v1->screen[1]) { const sw_vertex_t *tmp = v0; v0 = v1; v1 = tmp; } \
                                                                                    \
    /* Extracting coordinates from the sorted vertices */                           \
    float x0 = v0->screen[0], y0 = v0->screen[1];                                   \
    float x1 = v1->screen[0], y1 = v1->screen[1];                                   \
    float x2 = v2->screen[0], y2 = v2->screen[1];                                   \
                                                                                    \
    /* Compute height differences */                                                \
    float h02 = y2 - y0;                                                            \
    float h01 = y1 - y0;                                                            \
    float h12 = y2 - y1;                                                            \
                                                                                    \
    if (h02 < 1e-6f) return;                                                        \
                                                                                    \
    /* Precompute the inverse values without additional checks */                   \
    float h02Rcp = 1.0f/h02;                                                        \
    float h01Rcp = (h01 > 1e-6f)? 1.0f/h01 : 0.0f;                                  \
    float h12Rcp = (h12 > 1e-6f)? 1.0f/h12 : 0.0f;                                  \
                                                                                    \
    /* Pre-calculation of slopes */                                                 \
    float dXdy02 = (x2 - x0)*h02Rcp;                                                \
    float dXdy01 = (x1 - x0)*h01Rcp;                                                \
    float dXdy12 = (x2 - x1)*h12Rcp;                                                \
                                                                                    \
    /* Y subpixel correction */                                                     \
    float y0Substep = 1.0f - sw_fract(y0);                                          \
    float y1Substep = 1.0f - sw_fract(y1);                                          \
                                                                                    \
    /* Y bounds (vertical clipping) */                                              \
    int yTop = (int)y0;                                                             \
    int yMid = (int)y1;                                                             \
    int yBot = (int)y2;                                                             \
                                                                                    \
    /* Compute gradients for each side of the triangle */                           \
    sw_vertex_t dVXdy02, dVXdy01, dVXdy12;                                          \
    sw_get_vertex_grad_PTCH(&dVXdy02, v0, v2, h02Rcp);                              \
    sw_get_vertex_grad_PTCH(&dVXdy01, v0, v1, h01Rcp);                              \
    sw_get_vertex_grad_PTCH(&dVXdy12, v1, v2, h12Rcp);                              \
                                                                                    \
    /* Get a copy of vertices for interpolation and apply substep correction */     \
    sw_vertex_t vLeft = *v0, vRight = *v0;                                          \
    sw_add_vertex_grad_scaled_PTCH(&vLeft, &dVXdy02, y0Substep);                    \
    sw_add_vertex_grad_scaled_PTCH(&vRight, &dVXdy01, y0Substep);                   \
                                                                                    \
    vLeft.screen[0] += dXdy02*y0Substep;                                            \
    vRight.screen[0] += dXdy01*y0Substep;                                           \
                                                                                    \
    /* Scanline for the upper part of the triangle */                               \
    for (int y = yTop; y < yMid; y++)                                               \
    {                                                                               \
        vLeft.screen[1] = vRight.screen[1] = y;                                     \
                                                                                    \
        if (vLeft.screen[0] < vRight.screen[0]) FUNC_SCANLINE(tex, &vLeft, &vRight, dVXdy02.texcoord[0], dVXdy02.texcoord[1]); \
        else FUNC_SCANLINE(tex, &vRight, &vLeft, dVXdy02.texcoord[0], dVXdy02.texcoord[1]); \
                                                                                    \
        sw_add_vertex_grad_PTCH(&vLeft, &dVXdy02);                                  \
        vLeft.screen[0] += dXdy02;                                                  \
                                                                                    \
        sw_add_vertex_grad_PTCH(&vRight, &dVXdy01);                                 \
        vRight.screen[0] += dXdy01;                                                 \
    }                                                                               \
                                                                                    \
    /* Get a copy of next right for interpolation and apply substep correction */   \
    vRight = *v1;                                                                   \
    sw_add_vertex_grad_scaled_PTCH(&vRight, &dVXdy12, y1Substep);                   \
    vRight.screen[0] += dXdy12*y1Substep;                                           \
                                                                                    \
    /* Scanline for the lower part of the triangle */                               \
    for (int y = yMid; y < yBot; y++)                                               \
    {                                                                               \
        vLeft.screen[1] = vRight.screen[1] = y;                                     \
                                                                                    \
        if (vLeft.screen[0] < vRight.screen[0]) FUNC_SCANLINE(tex, &vLeft, &vRight, dVXdy02.texcoord[0], dVXdy02.texcoord[1]); \
        else FUNC_SCANLINE(tex, &vRight, &vLeft, dVXdy02.texcoord[0], dVXdy02.texcoord[1]); \
                                                                                    \
        sw_add_vertex_grad_PTCH(&vLeft, &dVXdy02);                                  \
        vLeft.screen[0] += dXdy02;                                                  \
                                                                                    \
        sw_add_vertex_grad_PTCH(&vRight, &dVXdy12);                                 \
        vRight.screen[0] += dXdy12;                                                 \
    }                                                                               \
}

DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline, 0, 0, 0)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_TEX, 1, 0, 0)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_DEPTH, 0, 1, 0)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_BLEND, 0, 0, 1)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_TEX_DEPTH, 1, 1, 0)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_TEX_BLEND, 1, 0, 1)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_DEPTH_BLEND, 0, 1, 1)
DEFINE_TRIANGLE_RASTER_SCANLINE(sw_triangle_raster_scanline_TEX_DEPTH_BLEND, 1, 1, 1)

DEFINE_TRIANGLE_RASTER(sw_triangle_raster, sw_triangle_raster_scanline, false)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_TEX, sw_triangle_raster_scanline_TEX, true)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_DEPTH, sw_triangle_raster_scanline_DEPTH, false)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_BLEND, sw_triangle_raster_scanline_BLEND, false)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH, sw_triangle_raster_scanline_TEX_DEPTH, true)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND, sw_triangle_raster_scanline_TEX_BLEND, true)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND, sw_triangle_raster_scanline_DEPTH_BLEND, false)
DEFINE_TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND, sw_triangle_raster_scanline_TEX_DEPTH_BLEND, true)

static inline void sw_triangle_render(void)
{
    if (RLSW.stateFlags & SW_STATE_CULL_FACE)
    {
        if (!sw_triangle_face_culling()) return;
    }

    sw_triangle_clip_and_project();

    if (RLSW.vertexCounter < 3) return;

    #define TRIANGLE_RASTER(RASTER_FUNC)                        \
    {                                                           \
        for (int i = 0; i < RLSW.vertexCounter - 2; i++)        \
        {                                                       \
            RASTER_FUNC(                                        \
                &RLSW.vertexBuffer[0],                          \
                &RLSW.vertexBuffer[i + 1],                      \
                &RLSW.vertexBuffer[i + 2],                      \
                &RLSW.loadedTextures[RLSW.currentTexture]       \
            );                                                  \
        }                                                       \
    }

    uint32_t state = RLSW.stateFlags;
    if (RLSW.currentTexture == 0) state &= ~SW_STATE_TEXTURE_2D;
    if ((RLSW.srcFactor == SW_ONE) && (RLSW.dstFactor == SW_ZERO)) state &= ~SW_STATE_BLEND;

    if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D)) TRIANGLE_RASTER(sw_triangle_raster_TEX)
    else TRIANGLE_RASTER(sw_triangle_raster)

    #undef TRIANGLE_RASTER
}
//-------------------------------------------------------------------------------------------

// Quad rendering logic
//-------------------------------------------------------------------------------------------
static inline bool sw_quad_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations
    // To handle quads crossing the w=0 plane correctly,
    // the winding order test is performed in homogeneous coordinates directly,
    // before the perspective division (division by w)
    // For a convex quad with vertices P0, P1, P2, P3 in sequential order,
    // the winding order of the quad is the same as the winding order
    // of the triangle P0 P1 P2. The homogeneous triangle is used on
    // winding test on this first triangle

    // Preload homogeneous coordinates into local variables
    const float *h0 = RLSW.vertexBuffer[0].homogeneous;
    const float *h1 = RLSW.vertexBuffer[1].homogeneous;
    const float *h2 = RLSW.vertexBuffer[2].homogeneous;

    // NOTE: h3 is not needed for this test
    // const float *h3 = RLSW.vertexBuffer[3].homogeneous;

    // Compute a value proportional to the signed area of the triangle P0 P1 P2
    // in the projected 2D plane, calculated directly using homogeneous coordinates
    // BEFORE division by w
    // This is the determinant of the matrix formed by the (x, y, w) components
    // of the vertices P0, P1, and P2. Its sign correctly indicates the winding order
    // in homogeneous space and its relationship to the projected 2D winding order,
    // even with negative w values
    // The determinant formula used here is:
    // h0.x*(h1.y*h2.w - h2.y*h1.w) +
    // h1.x*(h2.y*h0.w - h0.y*h2.w) +
    // h2.x*(h0.y*h1.w - h1.y*h0.w)

    const float hSgnArea =
        h0[0]*(h1[1]*h2[3] - h2[1]*h1[3]) +
        h1[0]*(h2[1]*h0[3] - h0[1]*h2[3]) +
        h2[0]*(h0[1]*h1[3] - h1[1]*h0[3]);

    // Perform face culling based on the winding order determined by the sign
    // of the homogeneous area/determinant of triangle P0 P1 P2
    // This test is robust for points with w > 0 or w < 0 within the triangle,
    // correctly capturing the change in orientation when crossing the w=0 plane

    // A positive hSgnArea typically corresponds to a counter-clockwise
    // winding in the projected space when all w > 0
    // A value of 0 for hSgnArea means P0, P1, P2 are collinear in (x, y, w)
    // space, which corresponds to a degenerate triangle projection
    // Such quads might also be degenerate or non-planar. They are typically
    // not culled by this test (0 < 0 is false, 0 > 0 is false)
    // and should be handled by the clipper if necessary

    return (RLSW.cullFace == SW_FRONT)? (hSgnArea < 0.0f) : (hSgnArea > 0.0f); // Cull if winding is "clockwise" : "counter-clockwise"
}

static inline void sw_quad_clip_and_project(void)
{
    sw_vertex_t *polygon = RLSW.vertexBuffer;
    int *vertexCounter = &RLSW.vertexCounter;

    if (sw_polygon_clip(polygon, vertexCounter))
    {
        // Transformation to screen space and normalization
        for (int i = 0; i < *vertexCounter; i++)
        {
            sw_vertex_t *v = &polygon[i];

            // Calculation of the reciprocal of W for normalization
            // as well as perspective-correct attributes
            const float wRcp = 1.0f/v->homogeneous[3];
            v->homogeneous[3] = wRcp;

            // Division of XYZ coordinates by weight
            v->homogeneous[0] *= wRcp;
            v->homogeneous[1] *= wRcp;
            v->homogeneous[2] *= wRcp;

            // Division of texture coordinates (perspective-correct)
            v->texcoord[0] *= wRcp;
            v->texcoord[1] *= wRcp;

            // Division of colors (perspective-correct)
            v->color[0] *= wRcp;
            v->color[1] *= wRcp;
            v->color[2] *= wRcp;
            v->color[3] *= wRcp;

            // Transformation to screen space
            sw_project_ndc_to_screen(v->screen, v->homogeneous);
        }
    }
}

static inline bool sw_quad_is_axis_aligned(void)
{
    // Reject quads with perspective projection
    // The fast path assumes affine (non-perspective) quads,
    // so it's required for all vertices to have homogeneous w = 1.0
    for (int i = 0; i < 4; i++)
    {
        if (RLSW.vertexBuffer[i].homogeneous[3] != 1.0f) return false;
    }

    // Epsilon tolerance in screen space (pixels)
    const float epsilon = 0.5f;

    // Fetch screen-space positions for the four quad vertices
    const float *p0 = RLSW.vertexBuffer[0].screen;
    const float *p1 = RLSW.vertexBuffer[1].screen;
    const float *p2 = RLSW.vertexBuffer[2].screen;
    const float *p3 = RLSW.vertexBuffer[3].screen;

    // Compute edge vectors between consecutive vertices
    // These define the four sides of the quad in screen space
    float dx01 = p1[0] - p0[0], dy01 = p1[1] - p0[1];
    float dx12 = p2[0] - p1[0], dy12 = p2[1] - p1[1];
    float dx23 = p3[0] - p2[0], dy23 = p3[1] - p2[1];
    float dx30 = p0[0] - p3[0], dy30 = p0[1] - p3[1];

    // Each edge must be either horizontal or vertical within epsilon tolerance
    // If any edge deviates significantly from either axis, the quad is not axis-aligned
    if (!((fabsf(dy01) < epsilon) || (fabsf(dx01) < epsilon))) return false;
    if (!((fabsf(dy12) < epsilon) || (fabsf(dx12) < epsilon))) return false;
    if (!((fabsf(dy23) < epsilon) || (fabsf(dx23) < epsilon))) return false;
    if (!((fabsf(dy30) < epsilon) || (fabsf(dx30) < epsilon))) return false;

    return true;
}

static inline void sw_quad_sort_cw(const sw_vertex_t* *output)
{
    const sw_vertex_t *input = RLSW.vertexBuffer;

    // Calculate the centroid of the quad
    float cx = (input[0].screen[0] + input[1].screen[0] +
                input[2].screen[0] + input[3].screen[0])*0.25f;
    float cy = (input[0].screen[1] + input[1].screen[1] +
                input[2].screen[1] + input[3].screen[1])*0.25f;

    // Calculate the angle of each vertex relative to the center
    // and assign them directly to their correct position
    const sw_vertex_t *corners[4] = { 0 };

    for (int i = 0; i < 4; i++)
    {
        float dx = input[i].screen[0] - cx;
        float dy = input[i].screen[1] - cy;

        // Determine the quadrant (clockwise from top-left)
        // top-left: dx < 0, dy < 0
        // top-right: dx >= 0, dy < 0
        // bottom-right: dx >= 0, dy >= 0
        // bottom-left: dx < 0, dy >= 0

        int idx;
        if (dy < 0) idx = (dx < 0)? 0 : 1; // Top row
        else idx = (dx < 0)? 3 : 2; // Bottom row

        corners[idx] = &input[i];
    }

    output[0] = corners[0];  // top-left
    output[1] = corners[1];  // top-right
    output[2] = corners[2];  // bottom-right
    output[3] = corners[3];  // bottom-left
}

// TODO: REVIEW: Could a perfectly aligned quad, where one of the four points has a different depth,
// still appear perfectly aligned from a certain point of view?
// Because in that case, it's still needed to perform perspective division for textures and colors...
#define DEFINE_QUAD_RASTER_AXIS_ALIGNED(FUNC_NAME, ENABLE_TEXTURE, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(void)                                              \
{                                                                               \
    const sw_vertex_t *sortedVerts[4];                                          \
    sw_quad_sort_cw(sortedVerts);                                               \
                                                                                \
    const sw_vertex_t *v0 = sortedVerts[0];                                     \
    const sw_vertex_t *v1 = sortedVerts[1];                                     \
    const sw_vertex_t *v2 = sortedVerts[2];                                     \
    const sw_vertex_t *v3 = sortedVerts[3];                                     \
                                                                                \
    /* Screen bounds (axis-aligned) */                                          \
    int xMin = (int)v0->screen[0];                                              \
    int yMin = (int)v0->screen[1];                                              \
    int xMax = (int)v2->screen[0];                                              \
    int yMax = (int)v2->screen[1];                                              \
                                                                                \
    float w = v2->screen[0] - v0->screen[0];                                    \
    float h = v2->screen[1] - v0->screen[1];                                    \
                                                                                \
    if ((w == 0) || (h == 0)) return;                                           \
                                                                                \
    float wRcp = (w > 0.0f)? 1.0f/w : 0.0f;                                     \
    float hRcp = (h > 0.0f)? 1.0f/h : 0.0f;                                     \
                                                                                \
    /* Subpixel corrections */                                                  \
    float xSubstep = 1.0f - sw_fract(v0->screen[0]);                            \
    float ySubstep = 1.0f - sw_fract(v0->screen[1]);                            \
                                                                                \
    /* Calculation of vertex gradients in X and Y */                            \
    float dUdx = 0.0f, dVdx = 0.0f;                                             \
    float dUdy = 0.0f, dVdy = 0.0f;                                             \
    if (ENABLE_TEXTURE) {                                                       \
        dUdx = (v1->texcoord[0] - v0->texcoord[0])*wRcp;                        \
        dVdx = (v1->texcoord[1] - v0->texcoord[1])*wRcp;                        \
        dUdy = (v3->texcoord[0] - v0->texcoord[0])*hRcp;                        \
        dVdy = (v3->texcoord[1] - v0->texcoord[1])*hRcp;                        \
    }                                                                           \
                                                                                \
    float dCdx[4], dCdy[4];                                                     \
    dCdx[0] = (v1->color[0] - v0->color[0])*wRcp;                               \
    dCdx[1] = (v1->color[1] - v0->color[1])*wRcp;                               \
    dCdx[2] = (v1->color[2] - v0->color[2])*wRcp;                               \
    dCdx[3] = (v1->color[3] - v0->color[3])*wRcp;                               \
    dCdy[0] = (v3->color[0] - v0->color[0])*hRcp;                               \
    dCdy[1] = (v3->color[1] - v0->color[1])*hRcp;                               \
    dCdy[2] = (v3->color[2] - v0->color[2])*hRcp;                               \
    dCdy[3] = (v3->color[3] - v0->color[3])*hRcp;                               \
                                                                                \
    float dZdx, dZdy;                                                           \
    dZdx = (v1->homogeneous[2] - v0->homogeneous[2])*wRcp;                      \
    dZdy = (v3->homogeneous[2] - v0->homogeneous[2])*hRcp;                      \
                                                                                \
    /* Start of quad rasterization */                                           \
    const sw_texture_t *tex;                                                    \
    if (ENABLE_TEXTURE) tex = &RLSW.loadedTextures[RLSW.currentTexture];        \
                                                                                \
    sw_pixel_t *pixels = RLSW.framebuffer.pixels;                               \
    int wDst = RLSW.framebuffer.width;                                          \
                                                                                \
    float zScanline = v0->homogeneous[2] + dZdx*xSubstep + dZdy*ySubstep;       \
    float uScanline = v0->texcoord[0] + dUdx*xSubstep + dUdy*ySubstep;          \
    float vScanline = v0->texcoord[1] + dVdx*xSubstep + dVdy*ySubstep;          \
                                                                                \
    float colorScanline[4] = {                                                  \
        v0->color[0] + dCdx[0]*xSubstep + dCdy[0]*ySubstep,                     \
        v0->color[1] + dCdx[1]*xSubstep + dCdy[1]*ySubstep,                     \
        v0->color[2] + dCdx[2]*xSubstep + dCdy[2]*ySubstep,                     \
        v0->color[3] + dCdx[3]*xSubstep + dCdy[3]*ySubstep                      \
    };                                                                          \
                                                                                \
    for (int y = yMin; y < yMax; y++)                                           \
    {                                                                           \
        sw_pixel_t *ptr = pixels + y*wDst + xMin;                               \
                                                                                \
        float z = zScanline;                                                    \
        float u = uScanline;                                                    \
        float v = vScanline;                                                    \
                                                                                \
        float color[4] = {                                                      \
            colorScanline[0],                                                   \
            colorScanline[1],                                                   \
            colorScanline[2],                                                   \
            colorScanline[3]                                                    \
        };                                                                      \
                                                                                \
        /* Scanline rasterization */                                            \
        for (int x = xMin; x < xMax; x++)                                       \
        {                                                                       \
            /* Pixel color computation */                                       \
            float srcColor[4] = {                                               \
                color[0],                                                       \
                color[1],                                                       \
                color[2],                                                       \
                color[3]                                                        \
            };                                                                  \
                                                                                \
            /* Test and write depth */                                          \
            if (ENABLE_DEPTH_TEST)                                              \
            {                                                                   \
                /* TODO: Implement different depth funcs? */                    \
                float depth =  sw_framebuffer_read_depth(ptr);                  \
                if (z > depth) goto discard;                                    \
            }                                                                   \
                                                                                \
            /* TODO: Implement depth mask */                                    \
            sw_framebuffer_write_depth(ptr, z);                                 \
                                                                                \
            if (ENABLE_TEXTURE)                                                 \
            {                                                                   \
                float texColor[4];                                              \
                sw_texture_sample(texColor, tex, u, v, dUdx, dUdy, dVdx, dVdy); \
                srcColor[0] *= texColor[0];                                     \
                srcColor[1] *= texColor[1];                                     \
                srcColor[2] *= texColor[2];                                     \
                srcColor[3] *= texColor[3];                                     \
            }                                                                   \
                                                                                \
            if (ENABLE_COLOR_BLEND)                                             \
            {                                                                   \
                float dstColor[4];                                              \
                sw_framebuffer_read_color(dstColor, ptr);                       \
                sw_blend_colors(dstColor, srcColor);                            \
                sw_framebuffer_write_color(ptr, dstColor);                      \
            }                                                                   \
            else sw_framebuffer_write_color(ptr, srcColor);                     \
                                                                                \
        discard:                                                                \
            z += dZdx;                                                          \
            color[0] += dCdx[0];                                                \
            color[1] += dCdx[1];                                                \
            color[2] += dCdx[2];                                                \
            color[3] += dCdx[3];                                                \
            if (ENABLE_TEXTURE)                                                 \
            {                                                                   \
                u += dUdx;                                                      \
                v += dVdx;                                                      \
            }                                                                   \
            ++ptr;                                                              \
        }                                                                       \
                                                                                \
        zScanline += dZdy;                                                      \
        colorScanline[0] += dCdy[0];                                            \
        colorScanline[1] += dCdy[1];                                            \
        colorScanline[2] += dCdy[2];                                            \
        colorScanline[3] += dCdy[3];                                            \
                                                                                \
        if (ENABLE_TEXTURE)                                                     \
        {                                                                       \
            uScanline += dUdy;                                                  \
            vScanline += dVdy;                                                  \
        }                                                                       \
    }                                                                           \
}

DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned, 0, 0, 0)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_TEX, 1, 0, 0)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_DEPTH, 0, 1, 0)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_BLEND, 0, 0, 1)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_TEX_DEPTH, 1, 1, 0)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_TEX_BLEND, 1, 0, 1)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_DEPTH_BLEND, 0, 1, 1)
DEFINE_QUAD_RASTER_AXIS_ALIGNED(sw_quad_raster_axis_aligned_TEX_DEPTH_BLEND, 1, 1, 1)

static inline void sw_quad_render(void)
{
    if (RLSW.stateFlags & SW_STATE_CULL_FACE)
    {
        if (!sw_quad_face_culling()) return;
    }

    sw_quad_clip_and_project();

    if (RLSW.vertexCounter < 3) return;

    uint32_t state = RLSW.stateFlags;
    if (RLSW.currentTexture == 0) state &= ~SW_STATE_TEXTURE_2D;
    if ((RLSW.srcFactor == SW_ONE) && (RLSW.dstFactor == SW_ZERO)) state &= ~SW_STATE_BLEND;

    if ((RLSW.vertexCounter == 4) && sw_quad_is_axis_aligned())
    {
        if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_quad_raster_axis_aligned_TEX_DEPTH_BLEND();
        else if (SW_STATE_CHECK_EX(state, SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_quad_raster_axis_aligned_DEPTH_BLEND();
        else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) sw_quad_raster_axis_aligned_TEX_BLEND();
        else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) sw_quad_raster_axis_aligned_TEX_DEPTH();
        else if (SW_STATE_CHECK_EX(state, SW_STATE_BLEND)) sw_quad_raster_axis_aligned_BLEND();
        else if (SW_STATE_CHECK_EX(state, SW_STATE_DEPTH_TEST)) sw_quad_raster_axis_aligned_DEPTH();
        else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D)) sw_quad_raster_axis_aligned_TEX();
        else sw_quad_raster_axis_aligned();
        return;
    }

    #define TRIANGLE_RASTER(RASTER_FUNC)                        \
    {                                                           \
        for (int i = 0; i < RLSW.vertexCounter - 2; i++)        \
        {                                                       \
            RASTER_FUNC(                                        \
                &RLSW.vertexBuffer[0],                          \
                &RLSW.vertexBuffer[i + 1],                      \
                &RLSW.vertexBuffer[i + 2],                      \
                &RLSW.loadedTextures[RLSW.currentTexture]       \
            );                                                  \
        }                                                       \
    }

    if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_BLEND)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH)
    else if (SW_STATE_CHECK_EX(state, SW_STATE_TEXTURE_2D)) TRIANGLE_RASTER(sw_triangle_raster_TEX)
    else TRIANGLE_RASTER(sw_triangle_raster)

    #undef TRIANGLE_RASTER
}
//-------------------------------------------------------------------------------------------

// Line rendering logic
//-------------------------------------------------------------------------------------------
static inline bool sw_line_clip_coord(float q, float p, float *t0, float *t1)
{
    if (fabsf(p) < SW_CLIP_EPSILON)
    {
        // Check if the line is entirely outside the window
        if (q < -SW_CLIP_EPSILON) return 0; // Completely outside
        return 1; // Completely inside or on the edges
    }

    const float r = q/p;

    if (p < 0)
    {
        if (r > *t1) return 0;
        if (r > *t0) *t0 = r;
    }
    else
    {
        if (r < *t0) return 0;
        if (r < *t1) *t1 = r;
    }

    return 1;
}

static inline bool sw_line_clip(sw_vertex_t *v0, sw_vertex_t *v1)
{
    float t0 = 0.0f, t1 = 1.0f;
    float dH[4], dC[4];

    for (int i = 0; i < 4; i++)
    {
        dH[i] = v1->homogeneous[i] - v0->homogeneous[i];
        dC[i] = v1->color[i] - v0->color[i];
    }

    // Clipping Liang-Barsky
    if (!sw_line_clip_coord(v0->homogeneous[3] - v0->homogeneous[0], -dH[3] + dH[0], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->homogeneous[3] + v0->homogeneous[0], -dH[3] - dH[0], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->homogeneous[3] - v0->homogeneous[1], -dH[3] + dH[1], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->homogeneous[3] + v0->homogeneous[1], -dH[3] - dH[1], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->homogeneous[3] - v0->homogeneous[2], -dH[3] + dH[2], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->homogeneous[3] + v0->homogeneous[2], -dH[3] - dH[2], &t0, &t1)) return false;

    // Clipping Scissor
    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        if (!sw_line_clip_coord(v0->homogeneous[0] - RLSW.scClipMin[0]*v0->homogeneous[3], RLSW.scClipMin[0]*dH[3] - dH[0], &t0, &t1)) return false;
        if (!sw_line_clip_coord(RLSW.scClipMax[0]*v0->homogeneous[3] - v0->homogeneous[0], dH[0] - RLSW.scClipMax[0]*dH[3], &t0, &t1)) return false;
        if (!sw_line_clip_coord(v0->homogeneous[1] - RLSW.scClipMin[1]*v0->homogeneous[3], RLSW.scClipMin[1]*dH[3] - dH[1], &t0, &t1)) return false;
        if (!sw_line_clip_coord(RLSW.scClipMax[1]*v0->homogeneous[3] - v0->homogeneous[1], dH[1] - RLSW.scClipMax[1]*dH[3], &t0, &t1)) return false;
    }

    // Interpolation of new coordinates
    if (t1 < 1.0f)
    {
        for (int i = 0; i < 4; i++)
        {
            v1->homogeneous[i] = v0->homogeneous[i] + t1*dH[i];
            v1->color[i] = v0->color[i] + t1*dC[i];
        }
    }

    if (t0 > 0.0f)
    {
        for (int i = 0; i < 4; i++)
        {
            v0->homogeneous[i] += t0*dH[i];
            v0->color[i] += t0*dC[i];
        }
    }

    return true;
}

static inline bool sw_line_clip_and_project(sw_vertex_t *v0, sw_vertex_t *v1)
{
    if (!sw_line_clip(v0, v1)) return false;

    // Convert homogeneous coordinates to NDC
    v0->homogeneous[3] = 1.0f/v0->homogeneous[3];
    v1->homogeneous[3] = 1.0f/v1->homogeneous[3];
    for (int i = 0; i < 3; i++)
    {
        v0->homogeneous[i] *= v0->homogeneous[3];
        v1->homogeneous[i] *= v1->homogeneous[3];
    }

    // Convert NDC coordinates to screen space
    sw_project_ndc_to_screen(v0->screen, v0->homogeneous);
    sw_project_ndc_to_screen(v1->screen, v1->homogeneous);

    return true;
}

#define DEFINE_LINE_RASTER(FUNC_NAME, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(const sw_vertex_t *v0, const sw_vertex_t *v1) \
{                                                                       \
    float x0 = v0->screen[0];                                           \
    float y0 = v0->screen[1];                                           \
    float x1 = v1->screen[0];                                           \
    float y1 = v1->screen[1];                                           \
                                                                        \
    float dx = x1 - x0;                                                 \
    float dy = y1 - y0;                                                 \
                                                                        \
    /* Compute dominant axis and subpixel offset */                     \
    float steps, substep;                                               \
    if (fabsf(dx) > fabsf(dy))                                          \
    {                                                                   \
        steps = fabsf(dx);                                              \
        if (steps < 1.0f) return;                                       \
        substep = (dx >= 0.0f)? (1.0f - sw_fract(x0)) : sw_fract(x0);   \
    }                                                                   \
    else                                                                \
    {                                                                   \
        steps = fabsf(dy);                                              \
        if (steps < 1.0f) return;                                       \
        substep = (dy >= 0.0f)? (1.0f - sw_fract(y0)) : sw_fract(y0);   \
    }                                                                   \
                                                                        \
    /* Compute per pixel increments */                                  \
    float xInc = dx/steps;                                              \
    float yInc = dy/steps;                                              \
    float stepRcp = 1.0f/steps;                                         \
                                                                        \
    float zInc = (v1->homogeneous[2] - v0->homogeneous[2])*stepRcp;     \
    float rInc = (v1->color[0] - v0->color[0])*stepRcp;                 \
    float gInc = (v1->color[1] - v0->color[1])*stepRcp;                 \
    float bInc = (v1->color[2] - v0->color[2])*stepRcp;                 \
    float aInc = (v1->color[3] - v0->color[3])*stepRcp;                 \
                                                                        \
    /* Initializing the interpolation starting values  */               \
    float x = x0 + xInc*substep;                                        \
    float y = y0 + yInc*substep;                                        \
    float z = v0->homogeneous[2] + zInc*substep;                        \
    float r = v0->color[0] + rInc*substep;                              \
    float g = v0->color[1] + gInc*substep;                              \
    float b = v0->color[2] + bInc*substep;                              \
    float a = v0->color[3] + aInc*substep;                              \
                                                                        \
    const int fbWidth = RLSW.framebuffer.width;                         \
    sw_pixel_t *pixels = RLSW.framebuffer.pixels;                       \
                                                                        \
    int numPixels = (int)(steps - substep) + 1;                         \
                                                                        \
    for (int i = 0; i < numPixels; i++)                                 \
    {                                                                   \
        /* TODO: REVIEW: May require reviewing projection details */    \
        int px = (int)(x - 0.5f);                                       \
        int py = (int)(y - 0.5f);                                       \
                                                                        \
        sw_pixel_t *ptr = pixels + py*fbWidth + px;                     \
                                                                        \
        if (ENABLE_DEPTH_TEST)                                          \
        {                                                               \
            float depth = sw_framebuffer_read_depth(ptr);               \
            if (z > depth) goto discard;                                \
        }                                                               \
                                                                        \
        sw_framebuffer_write_depth(ptr, z);                             \
                                                                        \
        float color[4] = {r, g, b, a};                                  \
                                                                        \
        if (ENABLE_COLOR_BLEND)                                         \
        {                                                               \
            float dstColor[4];                                          \
            sw_framebuffer_read_color(dstColor, ptr);                   \
            sw_blend_colors(dstColor, color);                           \
            sw_framebuffer_write_color(ptr, dstColor);                  \
        }                                                               \
        else sw_framebuffer_write_color(ptr, color);                    \
                                                                        \
    discard:                                                            \
        x += xInc; y += yInc; z += zInc;                                \
        r += rInc; g += gInc; b += bInc; a += aInc;                     \
    }                                                                   \
}

#define DEFINE_LINE_THICK_RASTER(FUNC_NAME, RASTER_FUNC)                \
void FUNC_NAME(const sw_vertex_t *v1, const sw_vertex_t *v2)            \
{                                                                       \
    sw_vertex_t tv1, tv2;                                               \
                                                                        \
    int x1 = (int)v1->screen[0];                                        \
    int y1 = (int)v1->screen[1];                                        \
    int x2 = (int)v2->screen[0];                                        \
    int y2 = (int)v2->screen[1];                                        \
                                                                        \
    int dx = x2 - x1;                                                   \
    int dy = y2 - y1;                                                   \
                                                                        \
    RASTER_FUNC(v1, v2);                                                \
                                                                        \
    if ((dx != 0) && (abs(dy/dx) < 1))                                  \
    {                                                                   \
        int wy = (int)((RLSW.lineWidth - 1.0f)*abs(dx)/sqrtf(dx*dx + dy*dy)); \
        wy >>= 1;                                                       \
        for (int i = 1; i <= wy; i++)                                   \
        {                                                               \
            tv1 = *v1, tv2 = *v2;                                       \
            tv1.screen[1] -= i;                                         \
            tv2.screen[1] -= i;                                         \
            RASTER_FUNC(&tv1, &tv2);                                    \
            tv1 = *v1, tv2 = *v2;                                       \
            tv1.screen[1] += i;                                         \
            tv2.screen[1] += i;                                         \
            RASTER_FUNC(&tv1, &tv2);                                    \
        }                                                               \
    }                                                                   \
    else if (dy != 0)                                                   \
    {                                                                   \
        int wx = (int)((RLSW.lineWidth - 1.0f)*abs(dy)/sqrtf(dx*dx + dy*dy)); \
        wx >>= 1;                                                       \
        for (int i = 1; i <= wx; i++)                                   \
        {                                                               \
            tv1 = *v1, tv2 = *v2;                                       \
            tv1.screen[0] -= i;                                         \
            tv2.screen[0] -= i;                                         \
            RASTER_FUNC(&tv1, &tv2);                                    \
            tv1 = *v1, tv2 = *v2;                                       \
            tv1.screen[0] += i;                                         \
            tv2.screen[0] += i;                                         \
            RASTER_FUNC(&tv1, &tv2);                                    \
        }                                                               \
    }                                                                   \
}

DEFINE_LINE_RASTER(sw_line_raster, 0, 0)
DEFINE_LINE_RASTER(sw_line_raster_DEPTH, 1, 0)
DEFINE_LINE_RASTER(sw_line_raster_BLEND, 0, 1)
DEFINE_LINE_RASTER(sw_line_raster_DEPTH_BLEND, 1, 1)

DEFINE_LINE_THICK_RASTER(sw_line_thick_raster, sw_line_raster)
DEFINE_LINE_THICK_RASTER(sw_line_thick_raster_DEPTH, sw_line_raster_DEPTH)
DEFINE_LINE_THICK_RASTER(sw_line_thick_raster_BLEND, sw_line_raster_BLEND)
DEFINE_LINE_THICK_RASTER(sw_line_thick_raster_DEPTH_BLEND, sw_line_raster_DEPTH_BLEND)

static inline void sw_line_render(sw_vertex_t *vertices)
{
    if (!sw_line_clip_and_project(&vertices[0], &vertices[1])) return;

    if (RLSW.lineWidth >= 2.0f)
    {
        if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_line_thick_raster_DEPTH_BLEND(&vertices[0], &vertices[1]);
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) sw_line_thick_raster_BLEND(&vertices[0], &vertices[1]);
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) sw_line_thick_raster_DEPTH(&vertices[0], &vertices[1]);
        else sw_line_thick_raster(&vertices[0], &vertices[1]);
    }
    else
    {
        if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_line_raster_DEPTH_BLEND(&vertices[0], &vertices[1]);
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) sw_line_raster_BLEND(&vertices[0], &vertices[1]);
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) sw_line_raster_DEPTH(&vertices[0], &vertices[1]);
        else sw_line_raster(&vertices[0], &vertices[1]);
    }
}
//-------------------------------------------------------------------------------------------

// Point rendering logic
//-------------------------------------------------------------------------------------------
static inline bool sw_point_clip_and_project(sw_vertex_t *v)
{
    if (v->homogeneous[3] != 1.0f)
    {
        for (int_fast8_t i = 0; i < 3; i++)
        {
            if ((v->homogeneous[i] < -v->homogeneous[3]) || (v->homogeneous[i] > v->homogeneous[3])) return false;
        }

        v->homogeneous[3] = 1.0f/v->homogeneous[3];
        v->homogeneous[0] *= v->homogeneous[3];
        v->homogeneous[1] *= v->homogeneous[3];
        v->homogeneous[2] *= v->homogeneous[3];
    }

    sw_project_ndc_to_screen(v->screen, v->homogeneous);

    const int *min = NULL, *max = NULL;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        min = RLSW.scMin;
        max = RLSW.scMax;
    }
    else
    {
        min = RLSW.vpMin;
        max = RLSW.vpMax;
    }

    bool insideX = (v->screen[0] - RLSW.pointRadius < max[0]) && (v->screen[0] + RLSW.pointRadius > min[0]);
    bool insideY = (v->screen[1] - RLSW.pointRadius < max[1]) && (v->screen[1] + RLSW.pointRadius > min[1]);

    return (insideX && insideY);
}

#define DEFINE_POINT_RASTER(FUNC_NAME, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND, CHECK_BOUNDS) \
static inline void FUNC_NAME(int x, int y, float z, const float color[4])   \
{                                                                           \
    if (CHECK_BOUNDS == 1)                                                  \
    {                                                                       \
        if ((x < RLSW.vpMin[0]) || (x >= RLSW.vpMax[0])) return;            \
        if ((y < RLSW.vpMin[1]) || (y >= RLSW.vpMax[1])) return;            \
    }                                                                       \
    else if (CHECK_BOUNDS == SW_SCISSOR_TEST)                               \
    {                                                                       \
        if ((x < RLSW.scMin[0]) || (x >= RLSW.scMax[0])) return;            \
        if ((y < RLSW.scMin[1]) || (y >= RLSW.scMax[1])) return;            \
    }                                                                       \
                                                                            \
    int offset = y*RLSW.framebuffer.width + x;                              \
    sw_pixel_t *ptr = RLSW.framebuffer.pixels + offset;                     \
                                                                            \
    if (ENABLE_DEPTH_TEST)                                                  \
    {                                                                       \
        float depth = sw_framebuffer_read_depth(ptr);                       \
        if (z > depth) return;                                              \
    }                                                                       \
                                                                            \
    sw_framebuffer_write_depth(ptr, z);                                     \
                                                                            \
    if (ENABLE_COLOR_BLEND)                                                 \
    {                                                                       \
        float dstColor[4];                                                  \
        sw_framebuffer_read_color(dstColor, ptr);                           \
        sw_blend_colors(dstColor, color);                                   \
        sw_framebuffer_write_color(ptr, dstColor);                          \
    }                                                                       \
    else sw_framebuffer_write_color(ptr, color);                            \
}

#define DEFINE_POINT_THICK_RASTER(FUNC_NAME, RASTER_FUNC)                   \
static inline void FUNC_NAME(sw_vertex_t *v)                                \
{                                                                           \
    int cx = v->screen[0];                                                  \
    int cy = v->screen[1];                                                  \
    float cz = v->homogeneous[2];                                           \
    int radius = RLSW.pointRadius;                                          \
    const float *color = v->color;                                          \
                                                                            \
    int x = 0;                                                              \
    int y = radius;                                                         \
    int d = 3 - 2*radius;                                                   \
                                                                            \
    while (x <= y)                                                          \
    {                                                                       \
        for (int i = -x; i <= x; i++)                                       \
        {                                                                   \
            RASTER_FUNC(cx + i, cy + y, cz, color);                         \
            RASTER_FUNC(cx + i, cy - y, cz, color);                         \
        }                                                                   \
        for (int i = -y; i <= y; i++)                                       \
        {                                                                   \
            RASTER_FUNC(cx + i, cy + x, cz, color);                         \
            RASTER_FUNC(cx + i, cy - x, cz, color);                         \
        }                                                                   \
        if (d > 0)                                                          \
        {                                                                   \
            y--;                                                            \
            d = d + 4*(x - y) + 10;                                         \
        }                                                                   \
        else d = d + 4*x + 6;                                               \
        x++;                                                                \
    }                                                                       \
}

DEFINE_POINT_RASTER(sw_point_raster, 0, 0, 0)
DEFINE_POINT_RASTER(sw_point_raster_DEPTH, 1, 0, 0)
DEFINE_POINT_RASTER(sw_point_raster_BLEND, 0, 1, 0)
DEFINE_POINT_RASTER(sw_point_raster_DEPTH_BLEND, 1, 1, 0)

DEFINE_POINT_RASTER(sw_point_raster_CHECK, 0, 0, 1)
DEFINE_POINT_RASTER(sw_point_raster_DEPTH_CHECK, 1, 0, 1)
DEFINE_POINT_RASTER(sw_point_raster_BLEND_CHECK, 0, 1, 1)
DEFINE_POINT_RASTER(sw_point_raster_DEPTH_BLEND_CHECK, 1, 1, 1)

DEFINE_POINT_RASTER(sw_point_raster_CHECK_SCISSOR, 0, 0, SW_SCISSOR_TEST)
DEFINE_POINT_RASTER(sw_point_raster_DEPTH_CHECK_SCISSOR, 1, 0, SW_SCISSOR_TEST)
DEFINE_POINT_RASTER(sw_point_raster_BLEND_CHECK_SCISSOR, 0, 1, SW_SCISSOR_TEST)
DEFINE_POINT_RASTER(sw_point_raster_DEPTH_BLEND_CHECK_SCISSOR, 1, 1, SW_SCISSOR_TEST)

DEFINE_POINT_THICK_RASTER(sw_point_thick_raster, sw_point_raster_CHECK)
DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_DEPTH, sw_point_raster_DEPTH_CHECK)
DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_BLEND, sw_point_raster_BLEND_CHECK)
DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_DEPTH_BLEND, sw_point_raster_DEPTH_BLEND_CHECK)

DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_SCISSOR, sw_point_raster_CHECK_SCISSOR)
DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_DEPTH_SCISSOR, sw_point_raster_DEPTH_CHECK_SCISSOR)
DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_BLEND_SCISSOR, sw_point_raster_BLEND_CHECK_SCISSOR)
DEFINE_POINT_THICK_RASTER(sw_point_thick_raster_DEPTH_BLEND_SCISSOR, sw_point_raster_DEPTH_BLEND_CHECK_SCISSOR)

static inline void sw_point_render(sw_vertex_t *v)
{
    if (!sw_point_clip_and_project(v)) return;

    if (RLSW.pointRadius >= 1.0f)
    {
        if (SW_STATE_CHECK(SW_STATE_SCISSOR_TEST))
        {
            if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_point_thick_raster_DEPTH_BLEND_SCISSOR(v);
            else if (SW_STATE_CHECK(SW_STATE_BLEND)) sw_point_thick_raster_BLEND_SCISSOR(v);
            else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) sw_point_thick_raster_DEPTH_SCISSOR(v);
            else sw_point_thick_raster_SCISSOR(v);
        }
        else
        {
            if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_point_thick_raster_DEPTH_BLEND(v);
            else if (SW_STATE_CHECK(SW_STATE_BLEND)) sw_point_thick_raster_BLEND(v);
            else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) sw_point_thick_raster_DEPTH(v);
            else sw_point_thick_raster(v);
        }
    }
    else
    {
        if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_point_raster_DEPTH_BLEND(v->screen[0], v->screen[1], v->homogeneous[2], v->color);
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) sw_point_raster_BLEND(v->screen[0], v->screen[1], v->homogeneous[2], v->color);
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) sw_point_raster_DEPTH(v->screen[0], v->screen[1], v->homogeneous[2], v->color);
        else sw_point_raster(v->screen[0], v->screen[1], v->homogeneous[2], v->color);
    }
}
//-------------------------------------------------------------------------------------------

// Polygon modes rendering logic
//-------------------------------------------------------------------------------------------
static inline void sw_poly_point_render(void)
{
    for (int i = 0; i < RLSW.vertexCounter; i++) sw_point_render(&RLSW.vertexBuffer[i]);
}

static inline void sw_poly_line_render(void)
{
    const sw_vertex_t *vertices = RLSW.vertexBuffer;
    int cm1 = RLSW.vertexCounter - 1;

    for (int i = 0; i < cm1; i++)
    {
        sw_vertex_t verts[2] = { vertices[i], vertices[i + 1] };
        sw_line_render(verts);
    }

    sw_vertex_t verts[2] = { vertices[cm1], vertices[0] };
    sw_line_render(verts);
}

static inline void sw_poly_fill_render(void)
{
    switch (RLSW.drawMode)
    {
        case SW_POINTS: sw_point_render(&RLSW.vertexBuffer[0]); break;
        case SW_LINES: sw_line_render(RLSW.vertexBuffer); break;
        case SW_TRIANGLES: sw_triangle_render(); break;
        case SW_QUADS: sw_quad_render(); break;
    }
}
//-------------------------------------------------------------------------------------------

// Immediate rendering logic
//-------------------------------------------------------------------------------------------
void sw_immediate_push_vertex(const float position[4], const float color[4], const float texcoord[2])
{
    // Copy the attributes in the current vertex
    sw_vertex_t *vertex = &RLSW.vertexBuffer[RLSW.vertexCounter++];
    for (int i = 0; i < 4; i++)
    {
        vertex->position[i] = position[i];
        if (i < 2) vertex->texcoord[i] = texcoord[i];
        vertex->color[i] = color[i];
    }

    // Calculate homogeneous coordinates
    const float *m = RLSW.matMVP, *v = vertex->position;
    vertex->homogeneous[0] = m[0]*v[0] + m[4]*v[1] + m[8]*v[2] + m[12]*v[3];
    vertex->homogeneous[1] = m[1]*v[0] + m[5]*v[1] + m[9]*v[2] + m[13]*v[3];
    vertex->homogeneous[2] = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3];
    vertex->homogeneous[3] = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3];

    // Immediate rendering of the primitive if the required number is reached
    if (RLSW.vertexCounter == RLSW.reqVertices)
    {
        switch (RLSW.polyMode)
        {
            case SW_FILL: sw_poly_fill_render(); break;
            case SW_LINE: sw_poly_line_render(); break;
            case SW_POINT: sw_poly_point_render(); break;
            default: break;
        }

        RLSW.vertexCounter = 0;
    }
}

//-------------------------------------------------------------------------------------------

// Validity check helper functions
//-------------------------------------------------------------------------------------------
static inline bool sw_is_texture_valid(uint32_t id)
{
    bool valid = true;

    if (id == 0) valid = false;
    else if (id >= SW_MAX_TEXTURES) valid = false;
    else if (RLSW.loadedTextures[id].pixels == NULL) valid = false;

    return true;
}

static inline bool sw_is_texture_filter_valid(int filter)
{
    return ((filter == SW_NEAREST) || (filter == SW_LINEAR));
}

static inline bool sw_is_texture_wrap_valid(int wrap)
{
    return ((wrap == SW_REPEAT) || (wrap == SW_CLAMP));
}

static inline bool sw_is_draw_mode_valid(int mode)
{
    bool result = false;

    switch (mode)
    {
        case SW_POINTS:
        case SW_LINES:
        case SW_TRIANGLES:
        case SW_QUADS: result = true; break;
        default: break;
    }

    return result;
}

static inline bool sw_is_poly_mode_valid(int mode)
{
    bool result = false;

    switch (mode)
    {
        case SW_POINT:
        case SW_LINE:
        case SW_FILL: result = true; break;
        default: break;
    }

    return result;
}

static inline bool sw_is_face_valid(int face)
{
    return (face == SW_FRONT || face == SW_BACK);
}

static inline bool sw_is_blend_src_factor_valid(int blend)
{
    bool result = false;

    switch (blend)
    {
        case SW_ZERO:
        case SW_ONE:
        case SW_SRC_COLOR:
        case SW_ONE_MINUS_SRC_COLOR:
        case SW_SRC_ALPHA:
        case SW_ONE_MINUS_SRC_ALPHA:
        case SW_DST_ALPHA:
        case SW_ONE_MINUS_DST_ALPHA:
        case SW_DST_COLOR:
        case SW_ONE_MINUS_DST_COLOR:
        case SW_SRC_ALPHA_SATURATE: result = true; break;
        default: break;
    }

    return result;
}

static inline bool sw_is_blend_dst_factor_valid(int blend)
{
    bool result = false;

    switch (blend)
    {
        case SW_ZERO:
        case SW_ONE:
        case SW_SRC_COLOR:
        case SW_ONE_MINUS_SRC_COLOR:
        case SW_SRC_ALPHA:
        case SW_ONE_MINUS_SRC_ALPHA:
        case SW_DST_ALPHA:
        case SW_ONE_MINUS_DST_ALPHA:
        case SW_DST_COLOR:
        case SW_ONE_MINUS_DST_COLOR: result = true; break;
        default: break;
    }

    return result;
}
//-------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
bool swInit(int w, int h)
{
    if (!sw_framebuffer_load(w, h)) { swClose(); return false; }

    swViewport(0, 0, w, h);
    swScissor(0, 0, w, h);

    RLSW.loadedTextures = (sw_texture_t *)SW_MALLOC(SW_MAX_TEXTURES*sizeof(sw_texture_t));
    if (RLSW.loadedTextures == NULL) { swClose(); return false; }

    RLSW.freeTextureIds = (uint32_t *)SW_MALLOC(SW_MAX_TEXTURES*sizeof(uint32_t));
    if (RLSW.loadedTextures == NULL) { swClose(); return false; }

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    sw_framebuffer_write_color(&RLSW.clearValue, clearColor);
    sw_framebuffer_write_depth(&RLSW.clearValue, 1.0f);

    RLSW.currentMatrixMode = SW_MODELVIEW;
    RLSW.currentMatrix = &RLSW.stackModelview[0];

    sw_matrix_id(RLSW.stackProjection[0]);
    sw_matrix_id(RLSW.stackModelview[0]);
    sw_matrix_id(RLSW.stackTexture[0]);
    sw_matrix_id(RLSW.matMVP);

    RLSW.stackProjectionCounter = 1;
    RLSW.stackModelviewCounter = 1;
    RLSW.stackTextureCounter = 1;
    RLSW.isDirtyMVP = false;

    RLSW.current.texcoord[0] = 0.0f;
    RLSW.current.texcoord[1] = 0.0f;

    RLSW.current.color[0] = 1.0f;
    RLSW.current.color[1] = 1.0f;
    RLSW.current.color[2] = 1.0f;
    RLSW.current.color[3] = 1.0f;

    RLSW.srcFactor = SW_SRC_ALPHA;
    RLSW.dstFactor = SW_ONE_MINUS_SRC_ALPHA;

    RLSW.srcFactorFunc = sw_factor_src_alpha;
    RLSW.dstFactorFunc = sw_factor_one_minus_src_alpha;

    RLSW.polyMode = SW_FILL;
    RLSW.cullFace = SW_BACK;

    static uint32_t defaultTex[3*2*2] = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
    };

    RLSW.loadedTextures[0].pixels = (uint8_t*)defaultTex;
    RLSW.loadedTextures[0].width = 2;
    RLSW.loadedTextures[0].height = 2;
    RLSW.loadedTextures[0].wMinus1 = 1;
    RLSW.loadedTextures[0].hMinus1 = 1;
    RLSW.loadedTextures[0].minFilter = SW_NEAREST;
    RLSW.loadedTextures[0].magFilter = SW_NEAREST;
    RLSW.loadedTextures[0].sWrap = SW_REPEAT;
    RLSW.loadedTextures[0].tWrap = SW_REPEAT;
    RLSW.loadedTextures[0].tx = 0.5f;
    RLSW.loadedTextures[0].ty = 0.5f;

    RLSW.loadedTextureCount = 1;

    SW_LOG("INFO: RLSW: Software renderer initialized successfully\n");
#if defined(SW_HAS_FMA_AVX) && defined(SW_HAS_FMA_AVX2)
    SW_LOG("INFO: RLSW: Using SIMD instructions: FMA AVX\n");
#endif
#if defined(SW_HAS_AVX) || defined(SW_HAS_AVX2)
    SW_LOG("INFO: RLSW: Using SIMD instructions: AVX\n");
#endif
#if defined(SW_HAS_SSE) || defined(SW_HAS_SSE2) || defined(SW_HAS_SSE3) || defined(SW_HAS_SSSE3) || defined(SW_HAS_SSE41) || defined(SW_HAS_SSE42)
    SW_LOG("INFO: RLSW: Using SIMD instructions: SSE\n");
#endif
#if defined(SW_HAS_NEON_FMA) || defined(SW_HAS_NEON)
    SW_LOG("INFO: RLSW: Using SIMD instructions: NEON\n");
#endif
#if defined(SW_HAS_RVV)
    SW_LOG("INFO: RLSW: Using SIMD instructions: RVV\n");
#endif

    return true;
}

void swClose(void)
{
    // NOTE: Starts at texture 1, texture 0 does not have to be freed
    for (int i = 1; i < RLSW.loadedTextureCount; i++)
    {
        if (sw_is_texture_valid(i))
        {
            SW_FREE(RLSW.loadedTextures[i].pixels);
        }
    }

    SW_FREE(RLSW.framebuffer.pixels);
    SW_FREE(RLSW.loadedTextures);
    SW_FREE(RLSW.freeTextureIds);

    RLSW = SW_CURLY_INIT(sw_context_t) { 0 };
}

bool swResizeFramebuffer(int w, int h)
{
    return sw_framebuffer_resize(w, h);
}

void swCopyFramebuffer(int x, int y, int w, int h, SWformat format, SWtype type, void *pixels)
{
    sw_pixelformat_t pFormat = (sw_pixelformat_t)sw_get_pixel_format(format, type);

    if (w <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }
    if (h <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }

    if (w > RLSW.framebuffer.width) w = RLSW.framebuffer.width;
    if (h > RLSW.framebuffer.height) h = RLSW.framebuffer.height;

    x = sw_clampi(x, 0, w);
    y = sw_clampi(y, 0, h);

    if ((x >= w) || (y >= h)) return;

    if ((x == 0) && (y == 0) && (w == RLSW.framebuffer.width) && (h == RLSW.framebuffer.height))
    {
        #if SW_COLOR_BUFFER_BITS == 32
            if (pFormat == SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            {
                sw_framebuffer_copy_fast(pixels);
                return;
            }
        #elif SW_COLOR_BUFFER_BITS == 16
            if (pFormat == SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5)
            {
                sw_framebuffer_copy_fast(pixels);
                return;
            }
        #endif
    }

    switch (pFormat)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: sw_framebuffer_copy_to_GRAYALPHA(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: sw_framebuffer_copy_to_GRAYALPHA(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5: sw_framebuffer_copy_to_R5G6B5(x, y, w, h, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8: sw_framebuffer_copy_to_R8G8B8(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: sw_framebuffer_copy_to_R5G5B5A1(x, y, w, h, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: sw_framebuffer_copy_to_R4G4B4A4(x, y, w, h, (uint16_t *)pixels); break;
        //case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: sw_framebuffer_copy_to_R8G8B8A8(x, y, w, h, (uint8_t *)pixels); break;
        // Below: not implemented
        case SW_PIXELFORMAT_UNCOMPRESSED_R32:
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        case SW_PIXELFORMAT_UNCOMPRESSED_R16:
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swBlitFramebuffer(int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, SWformat format, SWtype type, void *pixels)
{
    sw_pixelformat_t pFormat = (sw_pixelformat_t)sw_get_pixel_format(format, type);

    if (wSrc <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }
    if (hSrc <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }

    if (wSrc > RLSW.framebuffer.width) wSrc = RLSW.framebuffer.width;
    if (hSrc > RLSW.framebuffer.height) hSrc = RLSW.framebuffer.height;

    xSrc = sw_clampi(xSrc, 0, wSrc);
    ySrc = sw_clampi(ySrc, 0, hSrc);

    // Check if the sizes are identical after clamping the source to avoid unexpected issues
    // TODO: REVIEW: This repeats the operations if true, so a copy function can be made without these checks
    if (xDst == xSrc && yDst == ySrc && wDst == wSrc && hDst == hSrc)
    {
        swCopyFramebuffer(xSrc, ySrc, wSrc, hSrc, format, type, pixels);
    }

    switch (pFormat)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: sw_framebuffer_blit_to_GRAYALPHA(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: sw_framebuffer_blit_to_GRAYALPHA(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5: sw_framebuffer_blit_to_R5G6B5(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8: sw_framebuffer_blit_to_R8G8B8(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: sw_framebuffer_blit_to_R5G5B5A1(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: sw_framebuffer_blit_to_R4G4B4A4(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: sw_framebuffer_blit_to_R8G8B8A8(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        // Below: not implemented
        case SW_PIXELFORMAT_UNCOMPRESSED_R32:
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        case SW_PIXELFORMAT_UNCOMPRESSED_R16:
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        default:
            RLSW.errCode = SW_INVALID_ENUM;
            break;
    }
}

void swEnable(SWstate state)
{
    switch (state)
    {
        case SW_SCISSOR_TEST: RLSW.stateFlags |= SW_STATE_SCISSOR_TEST; break;
        case SW_TEXTURE_2D: RLSW.stateFlags |= SW_STATE_TEXTURE_2D; break;
        case SW_DEPTH_TEST: RLSW.stateFlags |= SW_STATE_DEPTH_TEST; break;
        case SW_CULL_FACE: RLSW.stateFlags |= SW_STATE_CULL_FACE; break;
        case SW_BLEND: RLSW.stateFlags |= SW_STATE_BLEND; break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swDisable(SWstate state)
{
    switch (state)
    {
        case SW_SCISSOR_TEST: RLSW.stateFlags &= ~SW_STATE_SCISSOR_TEST; break;
        case SW_TEXTURE_2D: RLSW.stateFlags &= ~SW_STATE_TEXTURE_2D; break;
        case SW_DEPTH_TEST: RLSW.stateFlags &= ~SW_STATE_DEPTH_TEST; break;
        case SW_CULL_FACE: RLSW.stateFlags &= ~SW_STATE_CULL_FACE; break;
        case SW_BLEND: RLSW.stateFlags &= ~SW_STATE_BLEND; break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swGetIntegerv(SWget name, int *v)
{
    switch (name)
    {
        case SW_MODELVIEW_STACK_DEPTH: *v = SW_MODELVIEW_STACK_DEPTH; break;
        case SW_PROJECTION_STACK_DEPTH: *v = SW_PROJECTION_STACK_DEPTH; break;
        case SW_TEXTURE_STACK_DEPTH: *v = SW_TEXTURE_STACK_DEPTH; break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swGetFloatv(SWget name, float *v)
{
    switch (name)
    {
        case SW_COLOR_CLEAR_VALUE:
        {
            sw_framebuffer_read_color(v, &RLSW.clearValue);
        } break;
        case SW_DEPTH_CLEAR_VALUE:
        {
            v[0] = sw_framebuffer_read_depth(&RLSW.clearValue);
        } break;
        case SW_CURRENT_COLOR:
        {
            v[0] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[0];
            v[1] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[1];
            v[2] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[2];
            v[3] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[3];
        } break;
        case SW_CURRENT_TEXTURE_COORDS:
        {
            v[0] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].texcoord[0];
            v[1] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].texcoord[1];
        } break;
        case SW_POINT_SIZE:
        {
            v[0] = 2.0f*RLSW.pointRadius;
        } break;
        case SW_LINE_WIDTH:
        {
            v[0] = RLSW.lineWidth;
        } break;
        case SW_MODELVIEW_MATRIX:
        {
            for (int i = 0; i < 16; i++) v[i] = RLSW.stackModelview[RLSW.stackModelviewCounter - 1][i];

        } break;
        case SW_PROJECTION_MATRIX:
        {
            for (int i = 0; i < 16; i++) v[i] = RLSW.stackProjection[RLSW.stackProjectionCounter - 1][i];

        } break;
        case SW_TEXTURE_MATRIX:
        {
            for (int i = 0; i < 16; i++) v[i] = RLSW.stackTexture[RLSW.stackTextureCounter - 1][i];

        } break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

const char *swGetString(SWget name)
{
    const char *result = NULL;

    switch (name)
    {
        case SW_VENDOR: result = "RLSW Header"; break;
        case SW_RENDERER: result = "RLSW Software Renderer"; break;
        case SW_VERSION: result = "RLSW 1.0"; break;
        case SW_EXTENSIONS: result = "None"; break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }

    return result;
}

SWerrcode swGetError(void)
{
    SWerrcode ret = RLSW.errCode;
    RLSW.errCode = SW_NO_ERROR;
    return ret;
}

void swViewport(int x, int y, int width, int height)
{
    if ((width < 0) || (height < 0))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.vpSize[0] = width;
    RLSW.vpSize[1] = height;

    RLSW.vpHalf[0] = width/2.0f;
    RLSW.vpHalf[1] = height/2.0f;

    RLSW.vpCenter[0] = (float)x + RLSW.vpHalf[0];
    RLSW.vpCenter[1] = (float)y + RLSW.vpHalf[1];

    RLSW.vpMin[0] = sw_clampi(x, 0, RLSW.framebuffer.width - 1);
    RLSW.vpMin[1] = sw_clampi(y, 0, RLSW.framebuffer.height - 1);
    RLSW.vpMax[0] = sw_clampi(x + width, 0, RLSW.framebuffer.width - 1);
    RLSW.vpMax[1] = sw_clampi(y + height, 0, RLSW.framebuffer.height - 1);
}

void swScissor(int x, int y, int width, int height)
{
    if ((width < 0) || (height < 0))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.scMin[0] = sw_clampi(x, 0, RLSW.framebuffer.width - 1);
    RLSW.scMin[1] = sw_clampi(y, 0, RLSW.framebuffer.height - 1);
    RLSW.scMax[0] = sw_clampi(x + width, 0, RLSW.framebuffer.width - 1);
    RLSW.scMax[1] = sw_clampi(y + height, 0, RLSW.framebuffer.height - 1);

    RLSW.scClipMin[0] = (2.0f*(float)RLSW.scMin[0]/(float)RLSW.vpSize[0]) - 1.0f;
    RLSW.scClipMax[0] = (2.0f*(float)RLSW.scMax[0]/(float)RLSW.vpSize[0]) - 1.0f;
    RLSW.scClipMax[1] = 1.0f - (2.0f*(float)RLSW.scMin[1]/(float)RLSW.vpSize[1]);
    RLSW.scClipMin[1] = 1.0f - (2.0f*(float)RLSW.scMax[1]/(float)RLSW.vpSize[1]);
}

void swClearColor(float r, float g, float b, float a)
{
    float v[4] = { r, g, b, a };
    sw_framebuffer_write_color(&RLSW.clearValue, v);
}

void swClearDepth(float depth)
{
    sw_framebuffer_write_depth(&RLSW.clearValue, depth);
}

void swClear(uint32_t bitmask)
{
    int size = RLSW.framebuffer.width*RLSW.framebuffer.height;

    if ((bitmask & (SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT)) == (SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT))
    {
        sw_framebuffer_fill(RLSW.framebuffer.pixels, size, RLSW.clearValue);
    }
    else if (bitmask & (SW_COLOR_BUFFER_BIT))
    {
        sw_framebuffer_fill_color(RLSW.framebuffer.pixels, size, RLSW.clearValue.color);
    }
    else if (bitmask & SW_DEPTH_BUFFER_BIT)
    {
        sw_framebuffer_fill_depth(RLSW.framebuffer.pixels, size, RLSW.clearValue.depth);
    }
}

void swBlendFunc(SWfactor sfactor, SWfactor dfactor)
{
    if (!sw_is_blend_src_factor_valid(sfactor) ||
        !sw_is_blend_dst_factor_valid(dfactor))
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    RLSW.srcFactor = sfactor;
    RLSW.dstFactor = dfactor;

    switch (sfactor)
    {
        case SW_ZERO: RLSW.srcFactorFunc = sw_factor_zero; break;
        case SW_ONE: RLSW.srcFactorFunc = sw_factor_one; break;
        case SW_SRC_COLOR: RLSW.srcFactorFunc = sw_factor_src_color; break;
        case SW_ONE_MINUS_SRC_COLOR: RLSW.srcFactorFunc = sw_factor_one_minus_src_color; break;
        case SW_SRC_ALPHA: RLSW.srcFactorFunc = sw_factor_src_alpha; break;
        case SW_ONE_MINUS_SRC_ALPHA: RLSW.srcFactorFunc = sw_factor_one_minus_src_alpha; break;
        case SW_DST_ALPHA: RLSW.srcFactorFunc = sw_factor_dst_alpha; break;
        case SW_ONE_MINUS_DST_ALPHA: RLSW.srcFactorFunc = sw_factor_one_minus_dst_alpha; break;
        case SW_DST_COLOR: RLSW.srcFactorFunc = sw_factor_dst_color; break;
        case SW_ONE_MINUS_DST_COLOR: RLSW.srcFactorFunc = sw_factor_one_minus_dst_color; break;
        case SW_SRC_ALPHA_SATURATE: RLSW.srcFactorFunc = sw_factor_src_alpha_saturate; break;
        default: break;
    }

    switch (dfactor)
    {
        case SW_ZERO: RLSW.dstFactorFunc = sw_factor_zero; break;
        case SW_ONE: RLSW.dstFactorFunc = sw_factor_one; break;
        case SW_SRC_COLOR: RLSW.dstFactorFunc = sw_factor_src_color; break;
        case SW_ONE_MINUS_SRC_COLOR: RLSW.dstFactorFunc = sw_factor_one_minus_src_color; break;
        case SW_SRC_ALPHA: RLSW.dstFactorFunc = sw_factor_src_alpha; break;
        case SW_ONE_MINUS_SRC_ALPHA: RLSW.dstFactorFunc = sw_factor_one_minus_src_alpha; break;
        case SW_DST_ALPHA: RLSW.dstFactorFunc = sw_factor_dst_alpha; break;
        case SW_ONE_MINUS_DST_ALPHA: RLSW.dstFactorFunc = sw_factor_one_minus_dst_alpha; break;
        case SW_DST_COLOR: RLSW.dstFactorFunc = sw_factor_dst_color; break;
        case SW_ONE_MINUS_DST_COLOR: RLSW.dstFactorFunc = sw_factor_one_minus_dst_color; break;
        case SW_SRC_ALPHA_SATURATE: break;
        default: break;
    }
}

void swPolygonMode(SWpoly mode)
{
    if (!sw_is_poly_mode_valid(mode))
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    RLSW.polyMode = mode;
}

void swCullFace(SWface face)
{
    if (!sw_is_face_valid(face))
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    RLSW.cullFace = face;
}

void swPointSize(float size)
{
    RLSW.pointRadius = floorf(size*0.5f);
}

void swLineWidth(float width)
{
    RLSW.lineWidth = roundf(width);
}

void swMatrixMode(SWmatrix mode)
{
    switch (mode)
    {
        case SW_PROJECTION: RLSW.currentMatrix = &RLSW.stackProjection[RLSW.stackProjectionCounter - 1]; break;
        case SW_MODELVIEW: RLSW.currentMatrix = &RLSW.stackModelview[RLSW.stackModelviewCounter - 1]; break;
        case SW_TEXTURE: RLSW.currentMatrix = &RLSW.stackTexture[RLSW.stackTextureCounter - 1]; break;
        default: RLSW.errCode = SW_INVALID_ENUM; return;
    }

    RLSW.currentMatrixMode = mode;
}

void swPushMatrix(void)
{
    switch (RLSW.currentMatrixMode)
    {
        case SW_PROJECTION:
        {
            if (RLSW.stackProjectionCounter >= SW_MAX_PROJECTION_STACK_SIZE)
            {
                RLSW.errCode = SW_STACK_OVERFLOW;
                return;
            }

            int iOld = RLSW.stackProjectionCounter - 1;
            int iNew = RLSW.stackProjectionCounter++;

            for (int i = 0; i < 16; i++)
            {
                RLSW.stackProjection[iNew][i] = RLSW.stackProjection[iOld][i];
            }

            RLSW.currentMatrix = &RLSW.stackProjection[iNew];
        } break;
        case SW_MODELVIEW:
        {
            if (RLSW.stackModelviewCounter >= SW_MAX_MODELVIEW_STACK_SIZE)
            {
                RLSW.errCode = SW_STACK_OVERFLOW;
                return;
            }

            int iOld = RLSW.stackModelviewCounter - 1;
            int iNew = RLSW.stackModelviewCounter++;

            for (int i = 0; i < 16; i++)
            {
                RLSW.stackModelview[iNew][i] = RLSW.stackModelview[iOld][i];
            }

            RLSW.currentMatrix = &RLSW.stackModelview[iNew];
        } break;
        case SW_TEXTURE:
        {
            if (RLSW.stackTextureCounter >= SW_MAX_TEXTURE_STACK_SIZE)
            {
                RLSW.errCode = SW_STACK_OVERFLOW;
                return;
            }

            int iOld = RLSW.stackTextureCounter - 1;
            int iNew = RLSW.stackTextureCounter++;

            for (int i = 0; i < 16; i++)
            {
                RLSW.stackTexture[iNew][i] = RLSW.stackTexture[iOld][i];
            }

            RLSW.currentMatrix = &RLSW.stackTexture[iNew];
        } break;
        default: break;
    }
}

void swPopMatrix(void)
{
    switch (RLSW.currentMatrixMode)
    {
        case SW_PROJECTION:
        {
            if (RLSW.stackProjectionCounter <= 0)
            {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackProjection[--RLSW.stackProjectionCounter];
            RLSW.isDirtyMVP = true; //< The MVP is considered to have been changed
        } break;
        case SW_MODELVIEW:
        {
            if (RLSW.stackModelviewCounter <= 0)
            {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackModelview[--RLSW.stackModelviewCounter];
            RLSW.isDirtyMVP = true; //< The MVP is considered to have been changed
        } break;
        case SW_TEXTURE:
        {
            if (RLSW.stackTextureCounter <= 0)
            {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackTexture[--RLSW.stackTextureCounter];
        } break;
        default: break;
    }
}

void swLoadIdentity(void)
{
    sw_matrix_id(*RLSW.currentMatrix);
    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swTranslatef(float x, float y, float z)
{
    sw_matrix_t mat;
    sw_matrix_id(mat);

    mat[12] = x;
    mat[13] = y;
    mat[14] = z;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swRotatef(float angle, float x, float y, float z)
{
    angle *= SW_DEG2RAD;

    float lengthSq = x*x + y*y + z*z;

    if ((lengthSq != 1.0f) && (lengthSq != 0.0f))
    {
        float invLength = 1.0f/sqrtf(lengthSq);
        x *= invLength;
        y *= invLength;
        z *= invLength;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    sw_matrix_t mat;

    mat[0] = x*x*t + cosres;
    mat[1] = y*x*t + z*sinres;
    mat[2] = z*x*t - y*sinres;
    mat[3] = 0.0f;

    mat[4] = x*y*t - z*sinres;
    mat[5] = y*y*t + cosres;
    mat[6] = z*y*t + x*sinres;
    mat[7] = 0.0f;

    mat[8] = x*z*t + y*sinres;
    mat[9] = y*z*t - x*sinres;
    mat[10] = z*z*t + cosres;
    mat[11] = 0.0f;

    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = 0.0f;
    mat[15] = 1.0f;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swScalef(float x, float y, float z)
{
    sw_matrix_t mat;

    mat[0]  = x, mat[1]  = 0, mat[2]  = 0, mat[3]  = 0;
    mat[4]  = 0, mat[5]  = y, mat[6]  = 0, mat[7]  = 0;
    mat[8]  = 0, mat[9]  = 0, mat[10] = z, mat[11] = 0;
    mat[12] = 0, mat[13] = 0, mat[14] = 0, mat[15] = 1;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swMultMatrixf(const float *mat)
{
    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    sw_matrix_t mat;

    double rl = right - left;
    double tb = top - bottom;
    double fn = zfar - znear;

    mat[0] = (float)(znear*2.0)/rl;
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    mat[3] = 0.0f;

    mat[4] = 0.0f;
    mat[5] = (float)(znear*2.0)/tb;
    mat[6] = 0.0f;
    mat[7] = 0.0f;

    mat[8] = (float)(right + left)/rl;
    mat[9] = (float)(top + bottom)/tb;
    mat[10] = -(float)(zfar + znear)/fn;
    mat[11] = -1.0f;

    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = -(zfar*znear*2.0)/fn;
    mat[15] = 0.0f;

    sw_matrix_mul(*RLSW.currentMatrix, *RLSW.currentMatrix, mat);

    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    sw_matrix_t mat;

    double rl = right - left;
    double tb = top - bottom;
    double fn = zfar - znear;

    mat[0] = 2.0f/(float)rl;
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    mat[3] = 0.0f;

    mat[4] = 0.0f;
    mat[5] = 2.0f/(float)tb;
    mat[6] = 0.0f;
    mat[7] = 0.0f;

    mat[8] = 0.0f;
    mat[9] = 0.0f;
    mat[10] = -2.0f/(float)fn;
    mat[11] = 0.0f;

    mat[12] = -(float)(left + right)/rl;
    mat[13] = -(float)(top + bottom)/tb;
    mat[14] = -(float)(zfar + znear)/fn;
    mat[15] = 1.0f;

    sw_matrix_mul(*RLSW.currentMatrix, *RLSW.currentMatrix, mat);

    if (RLSW.currentMatrixMode != SW_TEXTURE) RLSW.isDirtyMVP = true;
}

void swBegin(SWdraw mode)
{
    // Check if the draw mode is valid
    if (!sw_is_draw_mode_valid(mode))
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    // Recalculate the MVP if this is needed
    if (RLSW.isDirtyMVP)
    {
        sw_matrix_mul_rst(RLSW.matMVP,
            RLSW.stackModelview[RLSW.stackModelviewCounter - 1],
            RLSW.stackProjection[RLSW.stackProjectionCounter - 1]);

        RLSW.isDirtyMVP = false;
    }

    // Obtain the number of vertices needed for this primitive
    switch (mode)
    {
        case SW_POINTS: RLSW.reqVertices = 1; break;
        case SW_LINES: RLSW.reqVertices = 2; break;
        case SW_TRIANGLES: RLSW.reqVertices = 3; break;
        case SW_QUADS: RLSW.reqVertices = 4; break;
    }

    // Initialize required values
    RLSW.vertexCounter = 0;
    RLSW.drawMode = mode;
}

void swEnd(void)
{
    RLSW.drawMode = (SWdraw)0;
}

void swVertex2i(int x, int y)
{
    const float v[4] = { (float)x, (float)y, 0.0f, 1.0f };
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex2f(float x, float y)
{
    const float v[4] = { x, y, 0.0f, 1.0f };
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex2fv(const float *v)
{
    const float v4[4] = { v[0], v[1], 0.0f, 1.0f };
    sw_immediate_push_vertex(v4, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex3i(int x, int y, int z)
{
    const float v[4] = { (float)x, (float)y, (float)z, 1.0f };
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex3f(float x, float y, float z)
{
    const float v[4] = { x, y, z, 1.0f };
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex3fv(const float *v)
{
    const float v4[4] = { v[0], v[1], v[2], 1.0f };
    sw_immediate_push_vertex(v4, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex4i(int x, int y, int z, int w)
{
    const float v[4] = { (float)x, (float)y, (float)z, (float)w };
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex4f(float x, float y, float z, float w)
{
    const float v[4] = { x, y, z, w };
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swVertex4fv(const float *v)
{
    sw_immediate_push_vertex(v, RLSW.current.color, RLSW.current.texcoord);
}

void swColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
    float cv[4];
    cv[0] = (float)r*SW_INV_255;
    cv[1] = (float)g*SW_INV_255;
    cv[2] = (float)b*SW_INV_255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3ubv(const uint8_t *v)
{
    float cv[4];
    cv[0] = (float)v[0]*SW_INV_255;
    cv[1] = (float)v[1]*SW_INV_255;
    cv[2] = (float)v[2]*SW_INV_255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3f(float r, float g, float b)
{
    float cv[4];
    cv[0] = r;
    cv[1] = g;
    cv[2] = b;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3fv(const float *v)
{
    float cv[4];
    cv[0] = v[0];
    cv[1] = v[1];
    cv[2] = v[2];
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    float cv[4];
    cv[0] = (float)r*SW_INV_255;
    cv[1] = (float)g*SW_INV_255;
    cv[2] = (float)b*SW_INV_255;
    cv[3] = (float)a*SW_INV_255;

    swColor4fv(cv);
}

void swColor4ubv(const uint8_t *v)
{
    float cv[4];
    cv[0] = (float)v[0]*SW_INV_255;
    cv[1] = (float)v[1]*SW_INV_255;
    cv[2] = (float)v[2]*SW_INV_255;
    cv[3] = (float)v[3]*SW_INV_255;

    swColor4fv(cv);
}

void swColor4f(float r, float g, float b, float a)
{
    float cv[4];
    cv[0] = r;
    cv[1] = g;
    cv[2] = b;
    cv[3] = a;

    swColor4fv(cv);
}

void swColor4fv(const float *v)
{
    for (int i = 0; i < 4; i++) RLSW.current.color[i] = v[i];
}

void swTexCoord2f(float u, float v)
{
    const float *m = RLSW.stackTexture[RLSW.stackTextureCounter - 1];

    RLSW.current.texcoord[0] = m[0]*u + m[4]*v + m[12];
    RLSW.current.texcoord[1] = m[1]*u + m[5]*v + m[13];
}

void swTexCoord2fv(const float *v)
{
    const float *m = RLSW.stackTexture[RLSW.stackTextureCounter - 1];

    RLSW.current.texcoord[0] = m[0]*v[0] + m[4]*v[1] + m[12];
    RLSW.current.texcoord[1] = m[1]*v[0] + m[5]*v[1] + m[13];
}

void swBindArray(SWarray type, void *buffer)
{
    switch (type)
    {
        case SW_VERTEX_ARRAY: RLSW.array.positions = (float *)buffer; break;
        case SW_TEXTURE_COORD_ARRAY: RLSW.array.texcoords = (float *)buffer; break;
        case SW_COLOR_ARRAY: RLSW.array.colors = (uint8_t *)buffer; break;
        default: break;
    }
}

void swDrawArrays(SWdraw mode, int offset, int count)
{
    if (RLSW.array.positions == 0)
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    swBegin(mode);
    {
        const float *texMatrix = RLSW.stackTexture[RLSW.stackTextureCounter - 1];
        const float *defaultTexcoord = RLSW.current.texcoord;
        const float *defaultColor = RLSW.current.color;

        const float *positions = RLSW.array.positions;
        const float *texcoords = RLSW.array.texcoords;
        const uint8_t *colors = RLSW.array.colors;

        int end = offset + count;

        for (int i = offset; i < end; i++)
        {
            float u, v;
            if (texcoords)
            {
                int idx = 2*i;
                u = texcoords[idx];
                v = texcoords[idx + 1];
            }
            else
            {
                u = defaultTexcoord[0];
                v = defaultTexcoord[1];
            }

            float texcoord[2];
            texcoord[0] = texMatrix[0]*u + texMatrix[4]*v + texMatrix[12];
            texcoord[1] = texMatrix[1]*u + texMatrix[5]*v + texMatrix[13];

            float color[4] = {
                defaultColor[0],
                defaultColor[1],
                defaultColor[2],
                defaultColor[3]
            };

            if (colors)
            {
                int idx = 4*i;
                color[0] *= (float)colors[idx]*SW_INV_255;
                color[1] *= (float)colors[idx + 1]*SW_INV_255;
                color[2] *= (float)colors[idx + 2]*SW_INV_255;
                color[3] *= (float)colors[idx + 3]*SW_INV_255;
            }

            int idx = 3*i;
            float position[4] = {
                positions[idx],
                positions[idx + 1],
                positions[idx + 2],
                1.0f
            };

            sw_immediate_push_vertex(position, color, texcoord);
        }
    }
    swEnd();
}

void swDrawElements(SWdraw mode, int count, int type, const void *indices)
{
    if (RLSW.array.positions == 0)
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (count < 0)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    const uint8_t *indicesUb = NULL;
    const uint16_t *indicesUs = NULL;
    const uint32_t *indicesUi = NULL;

    switch (type)
    {
        case SW_UNSIGNED_BYTE:
            indicesUb = (const uint8_t *)indices;
            break;
        case SW_UNSIGNED_SHORT:
            indicesUs = (const uint16_t *)indices;
            break;
        case SW_UNSIGNED_INT:
            indicesUi = (const uint32_t *)indices;
            break;
        default:
            RLSW.errCode = SW_INVALID_ENUM;
            return;
    }

    swBegin(mode);
    {
        const float *texMatrix = RLSW.stackTexture[RLSW.stackTextureCounter - 1];
        const float *defaultTexcoord = RLSW.current.texcoord;
        const float *defaultColor = RLSW.current.color;

        const float *positions = RLSW.array.positions;
        const float *texcoords = RLSW.array.texcoords;
        const uint8_t *colors = RLSW.array.colors;

        for (int i = 0; i < count; i++)
        {
            int index = indicesUb? indicesUb[i] :
                       (indicesUs? indicesUs[i] : indicesUi[i]);

            float u, v;
            if (texcoords)
            {
                int idx = 2*index;
                u = texcoords[idx];
                v = texcoords[idx + 1];
            }
            else
            {
                u = defaultTexcoord[0];
                v = defaultTexcoord[1];
            }

            float texcoord[2];
            texcoord[0] = texMatrix[0]*u + texMatrix[4]*v + texMatrix[12];
            texcoord[1] = texMatrix[1]*u + texMatrix[5]*v + texMatrix[13];

            float color[4] = {
                defaultColor[0],
                defaultColor[1],
                defaultColor[2],
                defaultColor[3]
            };

            if (colors)
            {
                int idx = 4*index;
                color[0] *= (float)colors[idx]*SW_INV_255;
                color[1] *= (float)colors[idx + 1]*SW_INV_255;
                color[2] *= (float)colors[idx + 2]*SW_INV_255;
                color[3] *= (float)colors[idx + 3]*SW_INV_255;
            }

            int idx = 3*index;
            float position[4] = {
                positions[idx],
                positions[idx + 1],
                positions[idx + 2],
                1.0f
            };

            sw_immediate_push_vertex(position, color, texcoord);
        }
    }
    swEnd();
}

void swGenTextures(int count, uint32_t *textures)
{
    if ((count == 0) || (textures == NULL)) return;

    for (int i = 0; i < count; i++)
    {
        if (RLSW.loadedTextureCount >= SW_MAX_TEXTURES)
        {
            RLSW.errCode = SW_STACK_OVERFLOW; // WARNING: Out of memory, not really stack overflow
            return;
        }

        uint32_t id = 0;
        if (RLSW.freeTextureIdCount > 0) id = RLSW.freeTextureIds[--RLSW.freeTextureIdCount];
        else id = RLSW.loadedTextureCount++;

        RLSW.loadedTextures[id] = RLSW.loadedTextures[0];
        textures[i] = id;
    }
}

void swDeleteTextures(int count, uint32_t *textures)
{
    if ((count == 0) || (textures == NULL)) return;

    for (int i = 0; i < count; i++)
    {
        if (!sw_is_texture_valid(textures[i]))
        {
            RLSW.errCode = SW_INVALID_VALUE;
            continue;
        }

        SW_FREE(RLSW.loadedTextures[textures[i]].pixels);

        RLSW.loadedTextures[textures[i]].pixels = NULL;
        RLSW.freeTextureIds[RLSW.freeTextureIdCount++] = textures[i];
    }
}

void swTexImage2D(int width, int height, SWformat format, SWtype type, const void *data)
{
    uint32_t id = RLSW.currentTexture;

    if (!sw_is_texture_valid(id))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    int pixelFormat = sw_get_pixel_format(format, type);

    if (pixelFormat <= SW_PIXELFORMAT_UNKNOWN)
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    sw_texture_t *texture = &RLSW.loadedTextures[id];

    int size = width*height;
    texture->pixels = SW_MALLOC(4*size);

    if (texture->pixels == NULL)
    {
        RLSW.errCode = SW_STACK_OVERFLOW; // WARNING: Out of memory...
        return;
    }

    for (int i = 0; i < size; i++)
    {
        uint32_t *dst = &((uint32_t*)texture->pixels)[i];
        sw_get_pixel((uint8_t*)dst, data, i, pixelFormat);
    }

    texture->width = width;
    texture->height = height;
    texture->wMinus1 = width - 1;
    texture->hMinus1 = height - 1;
    texture->tx = 1.0f/width;
    texture->ty = 1.0f/height;
}

void swTexParameteri(int param, int value)
{
    uint32_t id = RLSW.currentTexture;

    if (!sw_is_texture_valid(id))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    sw_texture_t *texture = &RLSW.loadedTextures[id];

    switch (param)
    {
        case SW_TEXTURE_MIN_FILTER:
        {
            if (!sw_is_texture_filter_valid(value))
            {
                RLSW.errCode = SW_INVALID_ENUM;
                return;
            }

            texture->minFilter = (SWfilter)value;
        } break;
        case SW_TEXTURE_MAG_FILTER:
        {
            if (!sw_is_texture_filter_valid(value))
            {
                RLSW.errCode = SW_INVALID_ENUM;
                return;
            }

            texture->magFilter = (SWfilter)value;
        } break;
        case SW_TEXTURE_WRAP_S:
        {
            if (!sw_is_texture_wrap_valid(value))
            {
                RLSW.errCode = SW_INVALID_ENUM;
                return;
            }

            texture->sWrap = (SWwrap)value;
        } break;
        case SW_TEXTURE_WRAP_T:
        {
            if (!sw_is_texture_wrap_valid(value))
            {
                RLSW.errCode = SW_INVALID_ENUM;
                return;
            }

            texture->tWrap = (SWwrap)value;
        } break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swBindTexture(uint32_t id)
{
    if (id >= SW_MAX_TEXTURES)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (RLSW.loadedTextures[id].pixels == NULL)
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    RLSW.currentTexture = id;
}

#endif // RLSW_IMPLEMENTATION
