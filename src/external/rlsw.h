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

#ifndef SW_GL_BINDING_COPY_TEXTURE
    #define SW_GL_BINDING_COPY_TEXTURE      true
#endif

#ifndef SW_COLOR_BUFFER_BITS
    #define SW_COLOR_BUFFER_BITS            24
#endif

#ifndef SW_DEPTH_BUFFER_BITS
    #define SW_DEPTH_BUFFER_BITS            16
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

// Under normal circumstances, clipping a polygon can add at most one vertex per clipping plane.
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

#ifdef __cplusplus
    #define CURLY_INIT(name) name
#else
    #define CURLY_INIT(name) (name)
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
#define glGenTextures(c, v)                         swGenTextures((c), (v))
#define glDeleteTextures(c, v)                      swDeleteTextures((c), (v))
#define glTexImage2D(tr, l, if, w, h, b, f, t, p)   swTexImage2D((w), (h), (f), (t), SW_GL_BINDING_COPY_TEXTURE, (p))
#define glTexParameteri(tr, pname, param)           swTexParameteri((pname), (param))
#define glBindTexture(tr, id)                       swBindTexture((id))

// OpenGL functions NOT IMPLEMENTED by rlsw
#define glClearDepth(X)                         ((void)(X))
#define glDepthMask(X)                          ((void)(X))
#define glColorMask(X,Y,Z,W)                    ((void)(X),(void)(Y),(void)(Z),(void)(W))
#define glPixelStorei(X,Y)                      ((void)(X),(void)(Y))
#define glHint(X,Y)                             ((void)(X),(void)(Y))
#define glShadeModel(X)                         ((void)(X))
#define glFrontFace(X)                          ((void)(X))
#define glDepthFunc(X)                          ((void)(X))
#define glTexSubImage2D(X,Y,Z,W,A,B,C,D,E)      ((void)(X),(void)(Y),(void)(Z),(void)(W),(void)(A),(void)(B),(void)(C),(void)(D),(void)(E))
#define glGetTexImage(X,Y,Z,W,A)                ((void)(X),(void)(Y),(void)(Z),(void)(W),(void)(A))
#define glDrawElements(X,Y,Z,W)                 ((void)(X),(void)(Y),(void)(Z),(void)(W))
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
SWAPI void *swGetColorBuffer(int *w, int *h);

SWAPI void swEnable(SWstate state);
SWAPI void swDisable(SWstate state);

SWAPI void swGetFloatv(SWget name, float *v);
SWAPI const char *swGetString(SWget name);
SWAPI SWerrcode swGetError(void);

SWAPI void swViewport(int x, int y, int width, int height);
SWAPI void swScissor(int x, int y, int width, int height);

SWAPI void swClearColor(float r, float g, float b, float a);
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

SWAPI void swGenTextures(int count, uint32_t *textures);
SWAPI void swDeleteTextures(int count, uint32_t *textures);

SWAPI void swTexImage2D(int width, int height, SWformat format, SWtype type, bool copy, const void *data);
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

#include <stdlib.h>
#include <stddef.h>
#include <math.h>           // Required for: floorf(), fabsf()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define SW_PI       3.14159265358979323846f
#define SW_DEG2RAD  (SW_PI/180.0f)
#define SW_RAD2DEG  (180.0f/SW_PI)

#define SW_COLOR_PIXEL_SIZE     (SW_COLOR_BUFFER_BITS/8)
#define SW_DEPTH_PIXEL_SIZE     (SW_DEPTH_BUFFER_BITS/8)

#define SW_STATE_CHECK(flags)   ((RLSW.stateFlags & (flags)) == (flags))

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
    // Dirty hack for copied data  
    // TODO: Rework copied image handling
    union {
        const void *cptr;       // NOTE: Is used for all data reads
        void *ptr;              // WARNING: Should only be used to allocate and free data
    } pixels;

    int width, height;          // Dimensions of the texture
    int wMinus1, hMinus1;       // Dimensions minus one
    sw_pixelformat_t format;    // Pixel format (internal representation)

    SWfilter minFilter;         // Minification filter
    SWfilter magFilter;         // Magnification filter

    SWwrap sWrap;               // texcoord.x wrap mode
    SWwrap tWrap;               // texcoord.y wrap mode

    float tx;                   // Texel width
    float ty;                   // Texel height

    bool copy;                  // Flag indicating whether memory has been allocated

} sw_texture_t;

typedef struct {
    void *color;
    void *depth;
    int width;
    int height;
    int allocSz;
} sw_framebuffer_t;

typedef struct {
    sw_framebuffer_t framebuffer;   // Main framebuffer
    float clearColor[4];            // Color used to clear the screen
    float clearDepth;               // Depth value used to clear the screen

    int vpCenter[2];                // Viewport center
    int vpHalfSize[2];              // Viewport half dimensions
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

static inline void sw_lerp_vertex_PTCH(
    sw_vertex_t *SW_RESTRICT out,
    const sw_vertex_t *SW_RESTRICT a,
    const sw_vertex_t *SW_RESTRICT b,
    float t)
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

static inline void sw_get_vertex_grad_PTCH(
    sw_vertex_t *SW_RESTRICT out,
    const sw_vertex_t *SW_RESTRICT a,
    const sw_vertex_t *SW_RESTRICT b,
    float scale)
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

static inline void sw_add_vertex_grad_PTCH(
    sw_vertex_t *SW_RESTRICT out,
    const sw_vertex_t *SW_RESTRICT gradients)
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

// Half floating point management functions
static inline uint32_t sw_f16_to_f32_ui(uint16_t h)
{
    uint32_t s = (uint32_t)(h & 0x8000) << 16;
    int32_t em = h & 0x7fff;

    // bias exponent and pad mantissa with 0; 112 is relative exponent bias (127-15)
    int32_t r = (em + (112 << 10)) << 13;

    // denormal: flush to zero
    r = (em < (1 << 10))? 0 : r;

    // infinity/NaN; note that we preserve NaN payload as a byproduct of unifying inf/nan cases
    // 112 is an exponent bias fixup; since we already applied it once, applying it twice converts 31 to 255
    r += (em >= (31 << 10))? (112 << 23) : 0;

    return s | r;
}

static inline float sw_f16_to_f32(sw_half_t y)
{
    union { float f; uint32_t i; } v = { .i = sw_f16_to_f32_ui(y) };

    return v.f;
}

static inline uint16_t sw_f16_from_f32_ui(uint32_t ui)
{
    int32_t s = (ui >> 16) & 0x8000;
    int32_t em = ui & 0x7fffffff;

    // Bias exponent and round to nearest; 112 is relative exponent bias (127-15)
    int32_t h = (em - (112 << 23) + (1 << 12)) >> 13;

    // Underflow: flush to zero; 113 encodes exponent -14
    h = (em < (113 << 23))? 0 : h;

    // Overflow: infinity; 143 encodes exponent 16
    h = (em >= (143 << 23))? 0x7c00 : h;

    // NaN; note that we convert all types of NaN to qNaN
    h = (em > (255 << 23))? 0x7e00 : h;

    return (uint16_t)(s | h);
}

static inline sw_half_t sw_f16_from_f32(float i)
{
    union { float f; uint32_t i; } v;
    v.f = i;
    return sw_f16_from_f32_ui(v.i);
}

// Framebuffer management functions
static inline bool sw_framebuffer_load(int w, int h)
{
    int size = w*h;

    RLSW.framebuffer.color = SW_MALLOC(SW_COLOR_PIXEL_SIZE*size);
    if (RLSW.framebuffer.color == NULL) return false;

    RLSW.framebuffer.depth = SW_MALLOC(SW_DEPTH_PIXEL_SIZE*size);
    if (RLSW.framebuffer.depth == NULL) return false;

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

    void *newColor = SW_REALLOC(RLSW.framebuffer.color, newSize);
    if (newColor == NULL) return false;

    void *newDepth = SW_REALLOC(RLSW.framebuffer.depth, newSize);
    if (newDepth == NULL) return false;

    RLSW.framebuffer.color = newColor;
    RLSW.framebuffer.depth = newDepth;

    RLSW.framebuffer.width = w;
    RLSW.framebuffer.height = h;
    RLSW.framebuffer.allocSz = newSize;

    return true;
}

static inline void *sw_framebuffer_get_color_addr(const void *ptr, uint32_t offset)
{
    return (uint8_t *)ptr + offset*SW_COLOR_PIXEL_SIZE;
}

static inline void sw_framebuffer_inc_color_addr(void **ptr)
{
    *ptr = (void *)(((uint8_t *)*ptr) + SW_COLOR_PIXEL_SIZE);
}

static inline void sw_framebuffer_inc_const_color_addr(const void **ptr)
{
    *ptr = (const void *)(((const uint8_t *)*ptr) + SW_COLOR_PIXEL_SIZE);
}

static inline void *sw_framebuffer_get_depth_addr(const void *ptr, uint32_t offset)
{
    return (uint8_t *)ptr + offset*SW_DEPTH_PIXEL_SIZE;
}

static inline void sw_framebuffer_inc_depth_addr(void* *ptr)
{
    *ptr = (void*)(((uint8_t *)*ptr) + SW_DEPTH_PIXEL_SIZE);
}

#if (SW_COLOR_BUFFER_BITS == 8)     // RGB - (3:3:2)
static inline void sw_framebuffer_read_color(float dst[4], const void *src)
{
    uint8_t pixel = ((uint8_t *)src)[0];

    dst[0] = ((pixel >> 5) & 0x07)*(1.0f/7.0f);
    dst[1] = ((pixel >> 2) & 0x07)*(1.0f/7.0f);
    dst[2] = (pixel & 0x03)*(1.0f/3.0f);
    dst[3] = 1.0f;
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const void *src)
{
    uint8_t pixel = ((const uint8_t *)src)[0];

    uint8_t r = (pixel >> 5) & 0x07;
    uint8_t g = (pixel >> 2) & 0x07;
    uint8_t b = pixel & 0x03;

    dst[0] = (r*255 + 3)/7;
    dst[1] = (g*255 + 3)/7;
    dst[2] = (b*255 + 1)/3;
    dst[3] = 255;
}

static inline void sw_framebuffer_write_color(void *dst, const float color[3])
{
    uint8_t r = ((uint8_t)(color[0]*UINT8_MAX) >> 5) & 0x07;
    uint8_t g = ((uint8_t)(color[1]*UINT8_MAX) >> 5) & 0x07;
    uint8_t b = ((uint8_t)(color[2]*UINT8_MAX) >> 6) & 0x03;

    ((uint8_t *)dst)[0] = (r << 5) | (g << 2) | b;
}

static inline void sw_framebuffer_fill_color(void *ptr, int size, const float color[3])
{
    uint8_t r8 = (uint8_t)(color[0]*7.0f + 0.5f);
    uint8_t g8 = (uint8_t)(color[1]*7.0f + 0.5f);
    uint8_t b8 = (uint8_t)(color[2]*3.0f + 0.5f);

    uint8_t packedColor = ((r8 & 0x07) << 5) | ((g8 & 0x07) << 2) | (b8 & 0x03);

    uint8_t *p = (uint8_t *)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            uint8_t *curPtr = p + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) *curPtr++ = packedColor;
        }
    }
    else
    {
        for (int i = 0; i < size; i++) *p++ = packedColor;
    }
}

#elif (SW_COLOR_BUFFER_BITS == 16)      // RGB - (5:6:5)

static inline void sw_framebuffer_read_color(float dst[4], const void *src)
{
    uint16_t pixel = ((uint16_t *)src)[0];

    dst[0] = ((pixel >> 11) & 0x1F)*(1.0f/31.0f);
    dst[1] = ((pixel >> 5) & 0x3F)*(1.0f/63.0f);
    dst[2] = (pixel & 0x1F)*(1.0f/31.0f);
    dst[3] = 1.0f;
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const void *src)
{
    uint16_t pixel = ((const uint16_t *)src)[0];

    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5)  & 0x3F;
    uint8_t b = pixel & 0x1F;

    dst[0] = (r*255 + 15)/31;
    dst[1] = (g*255 + 31)/63;
    dst[2] = (b*255 + 15)/31;
    dst[3] = 255;
}

static inline void sw_framebuffer_write_color(void *dst, const float color[3])
{
    uint8_t r = (uint8_t)(color[0]*31.0f + 0.5f) & 0x1F;
    uint8_t g = (uint8_t)(color[1]*63.0f + 0.5f) & 0x3F;
    uint8_t b = (uint8_t)(color[2]*31.0f + 0.5f) & 0x1F;

    ((uint16_t *)dst)[0] = (r << 11) | (g << 5) | b;
}

static inline void sw_framebuffer_fill_color(void *ptr, int size, const float color[3])
{
    uint16_t r16 = (uint16_t)(color[0]*31.0f + 0.5f);
    uint16_t g_16 = (uint16_t)(color[1]*63.0f + 0.5f);
    uint16_t b_16 = (uint16_t)(color[2]*31.0f + 0.5f);

    uint16_t packedColor = ((r16 & 0x1F) << 11) | ((g_16 & 0x3F) << 5) | (b_16 & 0x1F);

    uint16_t *p = (uint16_t *)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            uint16_t *curPtr = p + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) *curPtr++ = packedColor;
        }
    }
    else
    {
        for (int i = 0; i < size; i++) *p++ = packedColor;
    }
}

#elif (SW_COLOR_BUFFER_BITS == 24)      // RGB - (8:8:8)

static inline void sw_framebuffer_read_color(float dst[4], const void *src)
{
    dst[0] = ((uint8_t *)src)[0]*(1.0f/255.0f);
    dst[1] = ((uint8_t *)src)[1]*(1.0f/255.0f);
    dst[2] = ((uint8_t *)src)[2]*(1.0f/255.0f);
    dst[3] = 1.0f;
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const void *src)
{
    dst[0] = ((uint8_t *)src)[0];
    dst[1] = ((uint8_t *)src)[1];
    dst[2] = ((uint8_t *)src)[2];
    dst[3] = 255;
}

static inline void sw_framebuffer_write_color(void *dst, const float color[3])
{
    ((uint8_t *)dst)[0] = (uint8_t)(color[0]*UINT8_MAX);
    ((uint8_t *)dst)[1] = (uint8_t)(color[1]*UINT8_MAX);
    ((uint8_t *)dst)[2] = (uint8_t)(color[2]*UINT8_MAX);
}

static inline void sw_framebuffer_fill_color(void *ptr, int size, const float color[3])
{
    uint8_t r = (uint8_t)(color[0]*255.0f);
    uint8_t g = (uint8_t)(color[1]*255.0f);
    uint8_t b = (uint8_t)(color[2]*255.0f);

    uint8_t *p = (uint8_t *)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            uint8_t *curPtr = p + 3*(y*RLSW.framebuffer.width + RLSW.scMin[0]);
            for (int xCount = 0; xCount < wScissor; xCount++)
            {
                *curPtr++ = r;
                *curPtr++ = g;
                *curPtr++ = b;
            }
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            *p++ = r;
            *p++ = g;
            *p++ = b;
        }
    }
}

#endif // SW_COLOR_BUFFER_BITS

#if (SW_DEPTH_BUFFER_BITS == 8)

static inline float sw_framebuffer_read_depth(const void *src)
{
    return (float)((uint8_t *)src)[0]*(1.0f/UINT8_MAX);
}

static inline void sw_framebuffer_write_depth(void *dst, float depth)
{
    ((uint8_t *)dst)[0] = (uint8_t)(depth*UINT8_MAX);
}

static inline void sw_framebuffer_fill_depth(void *ptr, int size, float value)
{
    uint8_t d8  = (uint8_t)(value*UINT8_MAX);
    uint8_t *p = (uint8_t *)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            uint8_t *curPtr = p + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) *curPtr++ = d8;
        }
    }
    else
    {
        for (int i = 0; i < size; i++) *p++ = d8;
    }
}

#elif (SW_DEPTH_BUFFER_BITS == 16)

static inline float sw_framebuffer_read_depth(const void *src)
{
    return (float)((uint16_t *)src)[0]*(1.0f/UINT16_MAX);
}

static inline void sw_framebuffer_write_depth(void *dst, float depth)
{
    ((uint16_t *)dst)[0] = (uint16_t)(depth*UINT16_MAX);
}

static inline void sw_framebuffer_fill_depth(void *ptr, int size, float value)
{
    uint16_t d16  = (uint16_t)(value*UINT16_MAX);
    uint16_t *p = (uint16_t *)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            uint16_t *curPtr = p + y*RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) *curPtr++ = d16;
        }
    }
    else
    {
        for (int i = 0; i < size; i++) *p++ = d16;
    }
}

#elif (SW_DEPTH_BUFFER_BITS == 24)

static inline float sw_framebuffer_read_depth(const void *src)
{
    uint32_t depth24 = (((uint8_t *)src)[0] << 16) |
                       (((uint8_t *)src)[1] << 8) |
                       ((uint8_t *)src)[2];

    return depth24/(float)0xFFFFFF;
}

static inline void sw_framebuffer_write_depth(void *dst, float depth)
{
    uint32_t depth24 = (uint32_t)(depth*0xFFFFFF);

    ((uint8_t *)dst)[0] = (depth24 >> 16) & 0xFF;
    ((uint8_t *)dst)[1] = (depth24 >> 8) & 0xFF;
    ((uint8_t *)dst)[2] = depth24 & 0xFF;
}

static inline void sw_framebuffer_fill_depth(void *ptr, int size, float value)
{
    uint32_t d32 = (uint32_t)(value*UINT32_MAX);
    uint8_t d_byte0 = (uint8_t)((d32 >> 16) & 0xFF);
    uint8_t d_byte1 = (uint8_t)((d32 >> 8) & 0xFF);
    uint8_t d_byte2 = (uint8_t)(d32 & 0xFF);

    uint8_t *p = (uint8_t *)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            uint8_t *curPtr = p + 3*(y*RLSW.framebuffer.width + RLSW.scMin[0]);
            for (int xCount = 0; xCount < wScissor; xCount++)
            {
                *curPtr++ = d_byte0;
                *curPtr++ = d_byte1;
                *curPtr++ = d_byte2;
            }
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            *p++ = d_byte0;
            *p++ = d_byte1;
            *p++ = d_byte2;
        }
    }
}

#endif // SW_DEPTH_BUFFER_BITS

static inline void sw_framebuffer_fill(void *colorPtr, void *depthPtr, int size, float color[4], float depth_value)
{
#if (SW_COLOR_BUFFER_BITS == 8)
    // Calculate and pack 3:3:2 color
    // Scale color components to the max value for each bit depth and round
    uint8_t r8 = (uint8_t)(color[0]*7.0f + 0.5f);
    uint8_t g8 = (uint8_t)(color[1]*7.0f + 0.5f);
    uint8_t b8 = (uint8_t)(color[2]*3.0f + 0.5f);
    // Pack the components into a single byte
    uint8_t packedColor = ((r8 & 0x07) << 5) | ((g8 & 0x07) << 2) | (b8 & 0x03);
    uint8_t *cptr = (uint8_t *)colorPtr;
#elif (SW_COLOR_BUFFER_BITS == 16)
    // Calculate and pack 5:6:5 color
    // Scale color components to the max value for each bit depth and round
    uint16_t r16 = (uint16_t)(color[0]*31.0f + 0.5f);
    uint16_t g16 = (uint16_t)(color[1]*63.0f + 0.5f);
    uint16_t b16 = (uint16_t)(color[2]*31.0f + 0.5f);
    // Pack the components into a 16-bit value
    uint16_t packedColor = ((r16 & 0x1F) << 11) | ((g16 & 0x3F) << 5) | (b16 & 0x1F);
    uint16_t *cptr = (uint16_t *)colorPtr;
#elif (SW_COLOR_BUFFER_BITS == 24)
    // Calculate 8:8:8 color components
    uint8_t r24 = (uint8_t)(color[0]*255.0f);
    uint8_t g24 = (uint8_t)(color[1]*255.0f);
    uint8_t b24 = (uint8_t)(color[2]*255.0f);
    uint8_t *cptr = (uint8_t *)colorPtr;
#endif

#if (SW_DEPTH_BUFFER_BITS == 8)
    // Calculate 8-bit depth
    uint8_t d8  = (uint8_t)(depth_value*UINT8_MAX);
    uint8_t *dptr = (uint8_t *)depthPtr;
#elif (SW_DEPTH_BUFFER_BITS == 16)
    // Calculate 16-bit depth
    uint16_t d16  = (uint16_t)(depth_value*UINT16_MAX);
    uint16_t *dptr = (uint16_t *)depthPtr;
#elif (SW_DEPTH_BUFFER_BITS == 24)
    // Calculate 24-bit depth and pre-calculate bytes
    uint32_t d32 = (uint32_t)(depth_value*UINT32_MAX);
    uint8_t dByte0 = (uint8_t)((d32 >> 16) & 0xFF);
    uint8_t dByte1 = (uint8_t)((d32 >> 8) & 0xFF);
    uint8_t dByte2 = (uint8_t)(d32 & 0xFF);
    uint8_t *dptr = (uint8_t *)depthPtr;
#endif

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST)
    {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++)
        {
            int rowStartIdx = y*RLSW.framebuffer.width + RLSW.scMin[0];

            // Calculate starting pointers for the current row within the scissor rectangle
        #if (SW_COLOR_BUFFER_BITS == 8)
            uint8_t *curCPtr = cptr + rowStartIdx;
        #elif (SW_COLOR_BUFFER_BITS == 16)
            uint16_t *curCPtr = cptr + rowStartIdx;
        #elif (SW_COLOR_BUFFER_BITS == 24)
            uint8_t *curCPtr = cptr + 3*rowStartIdx;
        #endif

        #if (SW_DEPTH_BUFFER_BITS == 8)
            uint8_t *curDPtr = dptr + rowStartIdx;
        #elif (SW_DEPTH_BUFFER_BITS == 16)
            uint16_t *curDPtr = dptr + rowStartIdx;
        #elif (SW_DEPTH_BUFFER_BITS == 24)
            uint8_t *curDPtr = dptr + 3*rowStartIdx;
        #endif

            // Fill the current row within the scissor rectangle
            for (int xCount = 0; xCount < wScissor; xCount++)
            {
                // Write color
            #if (SW_COLOR_BUFFER_BITS == 8)
                *curCPtr++ = packedColor;
            #elif (SW_COLOR_BUFFER_BITS == 16)
                *curCPtr++ = packedColor;
            #elif (SW_COLOR_BUFFER_BITS == 24)
                *curCPtr++ = r24;
                *curCPtr++ = g24;
                *curCPtr++ = b24;
            #endif

                // Write depth
            #if (SW_DEPTH_BUFFER_BITS == 8)
                *curDPtr++ = d8;
            #elif (SW_DEPTH_BUFFER_BITS == 16)
                *curDPtr++ = d16;
            #elif (SW_DEPTH_BUFFER_BITS == 24)
                *curDPtr++ = dByte0;
                *curDPtr++ = dByte1;
                *curDPtr++ = dByte2;
            #endif
            }
        }
        return;
    }

    for (int i = 0; i < size; i++)
    {
        // Write color
    #if (SW_COLOR_BUFFER_BITS == 8)
        *cptr++ = packedColor;
    #elif (SW_COLOR_BUFFER_BITS == 16)
        *cptr++ = packedColor;
    #elif (SW_COLOR_BUFFER_BITS == 24)
        *cptr++ = r24;
        *cptr++ = g24;
        *cptr++ = b24;
    #endif

        // Write depth
    #if (SW_DEPTH_BUFFER_BITS == 8)
        *dptr++ = d8;
    #elif (SW_DEPTH_BUFFER_BITS == 16)
        *dptr++ = d16;
    #elif (SW_DEPTH_BUFFER_BITS == 24)
        *dptr++ = dByte0;
        *dptr++ = dByte1;
        *dptr++ = dByte2;
    #endif
    }
}

#define DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_copy_to_##name(int x, int y, int w, int h, DST_PTR_T *dst)   \
{                                                                               \
    const void *src = RLSW.framebuffer.color;                                   \
                                                                                \
    for (int iy = y; iy < h; iy++) {                                            \
        for (int ix = x; ix < w; ix++) {                                        \
            uint8_t color[4];                                                   \
            sw_framebuffer_read_color8(color, src);                             \

#define DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_copy_to_##name(int x, int y, int w, int h, DST_PTR_T *dst)   \
{                                                                               \
    const void *src = RLSW.framebuffer.color;                                   \
                                                                                \
    for (int iy = y; iy < h; iy++) {                                            \
        for (int ix = x; ix < w; ix++) {                                        \
            float color[4];                                                     \
            sw_framebuffer_read_color(color, src);                              \

#define DEFINE_FRAMEBUFFER_COPY_END()                                           \
            sw_framebuffer_inc_const_color_addr(&src);                          \
        }                                                                       \
    }                                                                           \
}

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(GRAYSCALE, uint8_t)
{
    // NTSC grayscale conversion: Y = 0.299R + 0.587G + 0.114B
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);
    *dst++ = gray;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(GRAYALPHA, uint8_t)
{
    // Convert RGB to grayscale using NTSC formula
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);

    dst[0] = gray;
    dst[1] = color[3]; // alpha

    dst += 2;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R5G6B5, uint16_t)
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

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R8G8B8, uint8_t)
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

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R5G5B5A1, uint16_t)
{
    uint8_t r5 = (color[0]*31 + 127)/255;
    uint8_t g5 = (color[1]*31 + 127)/255;
    uint8_t b5 = (color[2]*31 + 127)/255;
    uint8_t a1 = color[3] >= 128 ? 1 : 0;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t pixel = (b5 << 11) | (g5 << 6) | (r5 << 1) | a1;
#else // RGBA
    uint16_t pixel = (r5 << 11) | (g5 << 6) | (b5 << 1) | a1;
#endif

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R4G4B4A4, uint16_t)
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

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R8G8B8A8, uint8_t)
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

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R32, float)
{
    dst[0] = color[0];
    dst++;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R32G32B32, float)
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

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R32G32B32A32, float)
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

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R16, sw_half_t)
{
    dst[0] = sw_f16_from_f32(color[0]);
    dst++;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R16G16B16, sw_half_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = sw_f16_from_f32(color[2]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[0]);
#else // RGBA
    dst[0] = sw_f16_from_f32(color[0]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[2]);
#endif

    dst += 3;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R16G16B16A16, sw_half_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = sw_f16_from_f32(color[2]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[0]);
#else // RGBA
    dst[0] = sw_f16_from_f32(color[0]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[2]);
#endif
    dst[3] = sw_f16_from_f32(color[3]);

    dst += 4;
}
DEFINE_FRAMEBUFFER_COPY_END()

#define DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_blit_to_##name(                               \
    int xDst, int yDst, int wDst, int hDst,                                     \
    int xSrc, int ySrc, int wSrc, int hSrc,                                     \
    DST_PTR_T *dst)                                                             \
{                                                                               \
    const uint8_t *srcBase = (uint8_t*)RLSW.framebuffer.color;                  \
    int fbWidth = RLSW.framebuffer.width;                                       \
                                                                                \
    uint32_t xScale = ((uint32_t)wSrc << 16)/(uint32_t)wDst;                  \
    uint32_t yScale = ((uint32_t)hSrc << 16)/(uint32_t)hDst;                  \
                                                                                \
    for (int dy = 0; dy < hDst; dy++) {                                         \
        uint32_t yFix = ((uint32_t)ySrc << 16) + dy*yScale;                   \
        int sy = yFix >> 16;                                                    \
                                                                                \
        for (int dx = 0; dx < wDst; dx++) {                                     \
            uint32_t xFix = dx*xScale;                                        \
            int sx = xFix >> 16;                                                \
            const void *srcPtr = sw_framebuffer_get_color_addr(srcBase, sy*fbWidth + sx); \
            uint8_t color[4];                                                   \
            sw_framebuffer_read_color8(color, srcPtr);                          \

#define DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_blit_to_##name(                               \
    int xDst, int yDst, int wDst, int hDst,                                     \
    int xSrc, int ySrc, int wSrc, int hSrc,                                     \
    DST_PTR_T *dst)                                                             \
{                                                                               \
    const uint8_t *srcBase = (uint8_t *)RLSW.framebuffer.color;                  \
    int fbWidth = RLSW.framebuffer.width;                                       \
                                                                                \
    uint32_t xScale = ((uint32_t)wSrc << 16)/(uint32_t)wDst;                  \
    uint32_t yScale = ((uint32_t)hSrc << 16)/(uint32_t)hDst;                  \
                                                                                \
    for (int dy = 0; dy < hDst; dy++) {                                         \
        uint32_t yFix = ((uint32_t)ySrc << 16) + dy*yScale;                   \
        int sy = yFix >> 16;                                                    \
                                                                                \
        for (int dx = 0; dx < wDst; dx++) {                                     \
            uint32_t xFix = dx*xScale;                                        \
            int sx = xFix >> 16;                                                \
            const void *srcPtr = sw_framebuffer_get_color_addr(srcBase, sy*fbWidth + sx); \
            float color[4];                                                     \
            sw_framebuffer_read_color(color, srcPtr);                           \

#define DEFINE_FRAMEBUFFER_BLIT_END()                                           \
        }                                                                       \
    }                                                                           \
}

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(GRAYSCALE, uint8_t)
{
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);
    *dst++ = gray;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(GRAYALPHA, uint8_t)
{
    uint8_t gray = (uint8_t)((color[0]*299 + color[1]*587 + color[2]*114 + 500)/1000);

    dst[0] = gray;
    dst[1] = color[3]; // alpha

    dst += 2;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R5G6B5, uint16_t)
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

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R8G8B8, uint8_t)
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

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R5G5B5A1, uint16_t)
{
    uint8_t r5 = (color[0]*31 + 127)/255;
    uint8_t g5 = (color[1]*31 + 127)/255;
    uint8_t b5 = (color[2]*31 + 127)/255;
    uint8_t a1 = color[3] >= 128 ? 1 : 0;

#if SW_GL_FRAMEBUFFER_COPY_BGRA
    uint16_t pixel = (b5 << 11) | (g5 << 6) | (r5 << 1) | a1;
#else // RGBA
    uint16_t pixel = (r5 << 11) | (g5 << 6) | (b5 << 1) | a1;
#endif

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R4G4B4A4, uint16_t)
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

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R8G8B8A8, uint8_t)
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

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R32, uint8_t)
{
    dst[0] = color[0];
    dst++;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R32G32B32, float)
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

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R32G32B32A32, float)
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

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R16, sw_half_t)
{
    dst[0] = sw_f16_from_f32(color[0]);
    dst++;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R16G16B16, sw_half_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = sw_f16_from_f32(color[2]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[0]);
#else // RGBA
    dst[0] = sw_f16_from_f32(color[0]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[2]);
#endif

    dst += 3;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R16G16B16A16, sw_half_t)
{
#if SW_GL_FRAMEBUFFER_COPY_BGRA
    dst[0] = sw_f16_from_f32(color[2]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[0]);
#else // RGBA
    dst[0] = sw_f16_from_f32(color[0]);
    dst[1] = sw_f16_from_f32(color[1]);
    dst[2] = sw_f16_from_f32(color[2]);
#endif
    dst[3] = sw_f16_from_f32(color[3]);

    dst += 4;
}
DEFINE_FRAMEBUFFER_BLIT_END()

// Pixel format management functions

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

int sw_get_pixel_bytes(sw_pixelformat_t format)
{
    int bpp = 0;

    switch (format)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 1; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 2; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 4; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 3; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32: bpp = 4; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 4*3; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 4*4; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16: bpp = 2; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16: bpp = 2*3; break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: bpp = 2*4; break;
        default: break;
    }

    return bpp;
}

static inline void sw_get_pixel_grayscale(float *color, const void *pixels, uint32_t offset)
{
    float gray = (float)((uint8_t *)pixels)[offset]*(1.0f/255);

    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_red_16(float *color, const void *pixels, uint32_t offset)
{
    float value = sw_f16_to_f32(((sw_half_t *)pixels)[offset]);

    color[0] = value;
    color[1] = value;
    color[2] = value;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_red_32(float *color, const void *pixels, uint32_t offset)
{
    float value = ((float *)pixels)[offset];

    color[0] = value;
    color[1] = value;
    color[2] = value;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_grayscale_alpha(float *color, const void *pixels, uint32_t offset)
{
    const uint8_t *pixelData = (const uint8_t *)pixels + 2*offset;

    color[0] = color[1] = color[2] = (float)pixelData[0]*(1.0f/255);
    color[3] = (float)pixelData[1]*(1.0f/255);
}

static inline void sw_get_pixel_rgb_565(float *color, const void *pixels, uint32_t offset)
{
    uint16_t pixel = ((uint16_t *)pixels)[offset];

    color[0] = (float)((pixel & 0xF800) >> 11)/31;
    color[1] = (float)((pixel & 0x7E0) >> 5)/63;
    color[2] = (float)(pixel & 0x1F)/31;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgb_888(float *color, const void *pixels, uint32_t offset)
{
    const uint8_t *pixel = (const uint8_t *)pixels + 3*offset;

    color[0] = (float)pixel[0]*(1.0f/255);
    color[1] = (float)pixel[1]*(1.0f/255);
    color[2] = (float)pixel[2]*(1.0f/255);
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgb_161616(float *color, const void *pixels, uint32_t offset)
{
    const sw_half_t *pixel = (sw_half_t *)pixels + 3*offset;

    color[0] = sw_f16_to_f32(pixel[0]);
    color[1] = sw_f16_to_f32(pixel[1]);
    color[2] = sw_f16_to_f32(pixel[2]);
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgb_323232(float *color, const void *pixels, uint32_t offset)
{
    const float *pixel = (float *)pixels + 3*offset;

    color[0] = pixel[0];
    color[1] = pixel[1];
    color[2] = pixel[2];
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgba_5551(float *color, const void *pixels, uint32_t offset)
{
    uint16_t pixel = ((uint16_t *)pixels)[offset];

    color[0] = (float)((pixel & 0xF800) >> 11)/31;
    color[1] = (float)((pixel & 0x7C0) >> 6)/31;
    color[2] = (float)((pixel & 0x3E) >> 1)/31;
    color[3] = (float)(pixel & 0x1);
}

static inline void sw_get_pixel_rgba_4444(float *color, const void *pixels, uint32_t offset)
{
    uint16_t pixel = ((uint16_t *)pixels)[offset];

    color[0] = (float)((pixel & 0xF000) >> 12)/15;
    color[1] = (float)((pixel & 0xF00) >> 8)/15;
    color[2] = (float)((pixel & 0xF0) >> 4)/15;
    color[3] = (float)(pixel & 0xF)/15;
}

static inline void sw_get_pixel_rgba_8888(float *color, const void *pixels, uint32_t offset)
{
    const uint8_t *pixel = (uint8_t *)pixels + 4*offset;

    color[0] = (float)pixel[0]*(1.0f/255);
    color[1] = (float)pixel[1]*(1.0f/255);
    color[2] = (float)pixel[2]*(1.0f/255);
    color[3] = (float)pixel[3]*(1.0f/255);
}

static inline void sw_get_pixel_rgba_16161616(float *color, const void *pixels, uint32_t offset)
{
    const sw_half_t *pixel = (sw_half_t *)pixels + 4*offset;

    color[0] = sw_f16_to_f32(pixel[0]);
    color[1] = sw_f16_to_f32(pixel[1]);
    color[2] = sw_f16_to_f32(pixel[2]);
    color[3] = sw_f16_to_f32(pixel[3]);
}

static inline void sw_get_pixel_rgba_32323232(float *color, const void *pixels, uint32_t offset)
{
    const float *pixel = (float *)pixels + 4*offset;

    color[0] = pixel[0];
    color[1] = pixel[1];
    color[2] = pixel[2];
    color[3] = pixel[3];
}

static inline void sw_get_pixel(float *color, const void *pixels, uint32_t offset, sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: sw_get_pixel_grayscale(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: sw_get_pixel_grayscale_alpha(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5: sw_get_pixel_rgb_565(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8: sw_get_pixel_rgb_888(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: sw_get_pixel_rgba_5551(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: sw_get_pixel_rgba_4444(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: sw_get_pixel_rgba_8888(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32: sw_get_pixel_red_32(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32: sw_get_pixel_rgb_323232(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: sw_get_pixel_rgba_32323232(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16: sw_get_pixel_red_16(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16: sw_get_pixel_rgb_161616(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: sw_get_pixel_rgba_16161616(color, pixels, offset); break;
        case SW_PIXELFORMAT_UNKNOWN: break;
        default: break;
    }
}

// Texture sampling functionality

static inline void sw_texture_sample_nearest(float *color, const sw_texture_t *tex, float u, float v)
{
    u = (tex->sWrap == SW_REPEAT)? sw_fract(u) : sw_saturate(u);
    v = (tex->tWrap == SW_REPEAT)? sw_fract(v) : sw_saturate(v);

    int x = u*tex->width, y = v*tex->height;

    sw_get_pixel(color, tex->pixels.cptr, y*tex->width + x, tex->format);
}

static inline void sw_texture_sample_linear(float *color, const sw_texture_t *tex, float u, float v)
{
    // TODO: REVIEW: With a bit more cleverness we could clearly reduce the
    // number of operations here, but for now it works fine.

    float xf = u*tex->width  - 0.5f;
    float yf = v*tex->height - 0.5f;

    int x0 = (int)floorf(xf);
    int y0 = (int)floorf(yf);

    float fx = xf - x0;
    float fy = yf - y0;

    int x1 = x0 + 1;
    int y1 = y0 + 1;

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
    sw_get_pixel(c00, tex->pixels.cptr, y0*tex->width + x0, tex->format);
    sw_get_pixel(c10, tex->pixels.cptr, y0*tex->width + x1, tex->format);
    sw_get_pixel(c01, tex->pixels.cptr, y1*tex->width + x0, tex->format);
    sw_get_pixel(c11, tex->pixels.cptr, y1*tex->width + x1, tex->format);

    for (int i = 0; i < 4; i++)
    {
        float t = c00[i] + fx*(c10[i] - c00[i]);
        float b = c01[i] + fx*(c11[i] - c01[i]);
        color[i] = t + fy*(b - t);
    }
}

static inline void sw_texture_sample(float *color, const sw_texture_t *tex, float u, float v, float duDx, float duDy, float dvDx, float dvDy)
{
    // Previous method: There is no need to compute the square root
    // because using the squared value, the comparison remains `L2 > 1.0f*1.0f`
    //float du = sqrtf(duDx*duDx + duDy*duDy);
    //float dv = sqrtf(dvDx*dvDx + dvDy*dvDy);
    //float L = (du > dv)? du : dv;

    // Calculate the derivatives for each axis
    float du2 = duDx*duDx + duDy*duDy;
    float dv2 = dvDx*dvDx + dvDy*dvDy;
    float L2 = (du2 > dv2)? du2 : dv2;

    SWfilter filter = (L2 > 1.0f)? tex->minFilter : tex->magFilter;

    switch (filter)
    {
        case SW_NEAREST: sw_texture_sample_nearest(color, tex, u, v); break;
        case SW_LINEAR: sw_texture_sample_linear(color, tex, u, v); break;
        default: break;
    }
}

// Color Blending functionality

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

// Projection helper functions

static inline void sw_project_ndc_to_screen(float screen[2], const float ndc[4])
{
    screen[0] = RLSW.vpCenter[0] + ndc[0]*RLSW.vpHalfSize[0];
    screen[1] = RLSW.vpCenter[1] - ndc[1]*RLSW.vpHalfSize[1];
}

// Polygon Clipping management

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

// Frustum cliping functions

#define IS_INSIDE_PLANE_W(h) ((h)[3] >= SW_CLIP_EPSILON)
#define IS_INSIDE_PLANE_X_POS(h) ((h)[0] <= (h)[3])
#define IS_INSIDE_PLANE_X_NEG(h) (-(h)[0] <= (h)[3])
#define IS_INSIDE_PLANE_Y_POS(h) ((h)[1] <= (h)[3])
#define IS_INSIDE_PLANE_Y_NEG(h) (-(h)[1] <= (h)[3])
#define IS_INSIDE_PLANE_Z_POS(h) ((h)[2] <= (h)[3])
#define IS_INSIDE_PLANE_Z_NEG(h) (-(h)[2] <= (h)[3])

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

// Scissor clip functions

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

// Main clip function

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

static inline bool sw_triangle_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations.
    //       To handle triangles crossing the w=0 plane correctly,
    //       we perform the winding order test in homogeneous coordinates directly,
    //       before the perspective division (division by w).
    //       This test determines the orientation of the triangle in the (x,y,w) plane,
    //       which corresponds to the projected 2D winding order sign,
    //       even with negative w values.

    // Preload homogeneous coordinates into local variables
    const float *h0 = RLSW.vertexBuffer[0].homogeneous;
    const float *h1 = RLSW.vertexBuffer[1].homogeneous;
    const float *h2 = RLSW.vertexBuffer[2].homogeneous;

    // Compute a value proportional to the signed area in the projected 2D plane,
    // calculated directly using homogeneous coordinates BEFORE division by w.
    // This is the determinant of the matrix formed by the (x, y, w) components
    // of the vertices, which correctly captures the winding order in homogeneous
    // space and its relationship to the projected 2D winding order, even with
    // negative w values.
    // The determinant formula used here is:
    // h0.x*(h1.y*h2.w - h2.y*h1.w) +
    // h1.x*(h2.y*h0.w - h0.y*h2.w) +
    // h2.x*(h0.y*h1.w - h1.y*h0.w)

    const float hSgnArea =
        h0[0]*(h1[1]*h2[3] - h2[1]*h1[3]) +
        h1[0]*(h2[1]*h0[3] - h0[1]*h2[3]) +
        h2[0]*(h0[1]*h1[3] - h1[1]*h0[3]);

    // Discard the triangle if its winding order (determined by the sign
    // of the homogeneous area/determinant) matches the culled direction.
    // A positive hSgnArea typically corresponds to a counter-clockwise
    // winding in the projected space when all w > 0.
    // This test is robust for points with w > 0 or w < 0, correctly
    // capturing the change in orientation when crossing the w=0 plane.

    // The culling logic remains the same based on the signed area/determinant.
    // A value of 0 for hSgnArea means the points are collinear in (x, y, w)
    // space, which corresponds to a degenerate triangle projection.
    // Such triangles are typically not culled by this test (0 < 0 is false, 0 > 0 is false)
    // and should be handled by the clipper if necessary.
    return (RLSW.cullFace == SW_FRONT)
        ? (hSgnArea < 0) // Cull if winding is "clockwise" in the projected sense
        : (hSgnArea > 0); // Cull if winding is "counter-clockwise" in the projected sense
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
            const float invW = 1.0f/v->homogeneous[3];
            v->homogeneous[3] = invW;

            // Division of XYZ coordinates by weight
            v->homogeneous[0] *= invW;
            v->homogeneous[1] *= invW;
            v->homogeneous[2] *= invW;

            // Division of texture coordinates (perspective-correct)
            v->texcoord[0] *= invW;
            v->texcoord[1] *= invW;

            // Division of colors (perspective-correct)
            v->color[0] *= invW;
            v->color[1] *= invW;
            v->color[2] *= invW;
            v->color[3] *= invW;

            // Transformation to screen space
            sw_project_ndc_to_screen(v->screen, v->homogeneous);
        }
    }
}

#define DEFINE_TRIANGLE_RASTER_SCANLINE(FUNC_NAME, ENABLE_TEXTURE, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(const sw_texture_t *tex, const sw_vertex_t *start,     \
                             const sw_vertex_t *end, float duDy, float dvDy)        \
{                                                                                   \
    /* Convert and center the screen coordinates */                                 \
    int xStart = (int)(start->screen[0] + 0.5f);                                    \
    int xEnd   = (int)(end->screen[0] + 0.5f);                                      \
    int y      = (int)start->screen[1];                                             \
                                                                                    \
    /* Compute the inverse horizontal distance along the X axis */                  \
    float dx = end->screen[0] - start->screen[0];                                   \
    if (fabsf(dx) < 1e-6f) return;                                                  \
    float dxRcp = 1.0f/dx;                                                          \
                                                                                    \
    /* Compute the interpolation steps along the X axis */                          \
    float dzDx = (end->homogeneous[2] - start->homogeneous[2])*dxRcp;               \
    float dwDx = (end->homogeneous[3] - start->homogeneous[3])*dxRcp;               \
                                                                                    \
    float dcDx[4] = { 0 };                                                          \
    dcDx[0] = (end->color[0] - start->color[0])*dxRcp;                              \
    dcDx[1] = (end->color[1] - start->color[1])*dxRcp;                              \
    dcDx[2] = (end->color[2] - start->color[2])*dxRcp;                              \
    dcDx[3] = (end->color[3] - start->color[3])*dxRcp;                              \
                                                                                    \
    float duDx = 0.0f, dvDx = 0.0f;                                                 \
    if (ENABLE_TEXTURE) {                                                           \
        duDx = (end->texcoord[0] - start->texcoord[0])*dxRcp;                       \
        dvDx = (end->texcoord[1] - start->texcoord[1])*dxRcp;                       \
    }                                                                               \
                                                                                    \
    /* Initializing the interpolation starting values  */                           \
    float z = start->homogeneous[2];                                                \
    float w = start->homogeneous[3];                                                \
                                                                                    \
    float color[4] = { 0 };                                                         \
    color[0] = start->color[0];                                                     \
    color[1] = start->color[1];                                                     \
    color[2] = start->color[2];                                                     \
    color[3] = start->color[3];                                                     \
                                                                                    \
    float u = 0.0f, v = 0.0f;                                                       \
    if (ENABLE_TEXTURE) {                                                           \
        u = start->texcoord[0];                                                     \
        v = start->texcoord[1];                                                     \
    }                                                                               \
                                                                                    \
    /* Pre-calculate the starting pointers for the framebuffer row */               \
    void *cptr = sw_framebuffer_get_color_addr(RLSW.framebuffer.color, y*RLSW.framebuffer.width + xStart);  \
    void *dptr = sw_framebuffer_get_depth_addr(RLSW.framebuffer.depth, y*RLSW.framebuffer.width + xStart);  \
                                                                                    \
    /* Scanline rasterization */                                                    \
    for (int x = xStart; x < xEnd; x++)                                             \
    {                                                                               \
        /* Pixel color computation */                                               \
        float wRcp = 1.0f/w;                                                        \
        float srcColor[4] = {                                                       \
        color[0]*wRcp,                                                              \
        color[1]*wRcp,                                                              \
        color[2]*wRcp,                                                              \
        color[3]*wRcp                                                               \
        };                                                                          \
                                                                                    \
        /* Test and write depth */                                                  \
        if (ENABLE_DEPTH_TEST)                                                      \
        {                                                                           \
            /* TODO: Implement different depth funcs? */                            \
            float depth =  sw_framebuffer_read_depth(dptr);                         \
            if (z > depth) goto discard;                                            \
        }                                                                           \
                                                                                    \
        sw_framebuffer_write_depth(dptr, z);                                        \
                                                                                    \
        if (ENABLE_TEXTURE)                                                         \
        {                                                                           \
            float texColor[4];                                                      \
            float s = u*wRcp;                                                       \
            float t = v*wRcp;                                                       \
            sw_texture_sample(texColor, tex, s, t, duDx, duDy, dvDx, dvDy);         \
            srcColor[0] *= texColor[0];                                             \
            srcColor[1] *= texColor[1];                                             \
            srcColor[2] *= texColor[2];                                             \
            srcColor[3] *= texColor[3];                                             \
        }                                                                           \
                                                                                    \
        if (ENABLE_COLOR_BLEND)                                                     \
        {                                                                           \
            float dstColor[4];                                                      \
            sw_framebuffer_read_color(dstColor, cptr);                              \
                                                                                    \
            sw_blend_colors(dstColor, srcColor);                                    \
            dstColor[0] = sw_saturate(dstColor[0]);                                 \
            dstColor[1] = sw_saturate(dstColor[1]);                                 \
            dstColor[2] = sw_saturate(dstColor[2]);                                 \
                                                                                    \
            sw_framebuffer_write_color(cptr, dstColor);                             \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            sw_framebuffer_write_color(cptr, srcColor);                             \
        }                                                                           \
                                                                                    \
        /* Increment the interpolation parameter, UVs, and pointers */              \
    discard:                                                                        \
        z += dzDx;                                                                  \
        w += dwDx;                                                                  \
        color[0] += dcDx[0];                                                        \
        color[1] += dcDx[1];                                                        \
        color[2] += dcDx[2];                                                        \
        color[3] += dcDx[3];                                                        \
        if (ENABLE_TEXTURE)                                                         \
        {                                                                           \
            u += duDx;                                                              \
            v += dvDx;                                                              \
        }                                                                           \
                                                                                    \
        sw_framebuffer_inc_color_addr(&cptr);                                       \
        sw_framebuffer_inc_depth_addr(&dptr);                                       \
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
    float invH02 = 1.0f/h02;                                                        \
    float invH01 = (h01 > 1e-6f)? 1.0f/h01 : 0.0f;                                  \
    float invH12 = (h12 > 1e-6f)? 1.0f/h12 : 0.0f;                                  \
                                                                                    \
    /* Pre-calculation of slopes */                                                 \
    float dx02 = (x2 - x0)*invH02;                                                  \
    float dx01 = (x1 - x0)*invH01;                                                  \
    float dx12 = (x2 - x1)*invH12;                                                  \
                                                                                    \
    /* Y bounds (vertical clipping) */                                              \
    int yTop = (int)(y0 + 0.5f);                                                    \
    int yMiddle = (int)(y1 + 0.5f);                                                 \
    int yBottom = (int)(y2 + 0.5f);                                                 \
                                                                                    \
    /* Compute gradients for each side of the triangle */                           \
    sw_vertex_t vDy02, vDy01, vDy12;                                                \
    sw_get_vertex_grad_PTCH(&vDy02, v0, v2, invH02);                                \
    sw_get_vertex_grad_PTCH(&vDy01, v0, v1, invH01);                                \
    sw_get_vertex_grad_PTCH(&vDy12, v1, v2, invH12);                                \
                                                                                    \
    /* Initializing scanline variables */                                           \
    sw_vertex_t vLeft = *v0;                                                        \
    vLeft.screen[0] = x0;                                                           \
    sw_vertex_t vRight = *v0;                                                       \
    vRight.screen[0] = x0;                                                          \
                                                                                    \
    /* Scanline for the upper part of the triangle */                               \
    for (int y = yTop; y < yMiddle; y++)                                            \
    {                                                                               \
        vLeft.screen[1] = vRight.screen[1] = y;                                     \
                                                                                    \
        if (vLeft.screen[0] < vRight.screen[0]) FUNC_SCANLINE(tex, &vLeft, &vRight, vDy02.texcoord[0], vDy02.texcoord[1]); \
        else FUNC_SCANLINE(tex, &vRight, &vLeft, vDy02.texcoord[0], vDy02.texcoord[1]); \
                                                                                    \
        sw_add_vertex_grad_PTCH(&vLeft, &vDy02);                                    \
        vLeft.screen[0]  += dx02;                                                   \
        sw_add_vertex_grad_PTCH(&vRight, &vDy01);                                   \
        vRight.screen[0] += dx01;                                                   \
    }                                                                               \
                                                                                    \
    /* Scanline for the lower part of the triangle */                               \
    vRight = *v1, vRight.screen[0] = x1;                                            \
                                                                                    \
    for (int y = yMiddle; y < yBottom; y++)                                         \
    {                                                                               \
        vLeft.screen[1] = vRight.screen[1] = y;                                     \
                                                                                    \
        if (vLeft.screen[0] < vRight.screen[0]) FUNC_SCANLINE(tex, &vLeft, &vRight, vDy02.texcoord[0], vDy02.texcoord[1]); \
        else FUNC_SCANLINE(tex, &vRight, &vLeft, vDy02.texcoord[0], vDy02.texcoord[1]); \
                                                                                    \
        sw_add_vertex_grad_PTCH(&vLeft, &vDy02);                                    \
        vLeft.screen[0]  += dx02;                                                   \
        sw_add_vertex_grad_PTCH(&vRight, &vDy12);                                   \
        vRight.screen[0] += dx12;                                                   \
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

    if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH)
    else if (SW_STATE_CHECK(SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH)
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D)) TRIANGLE_RASTER(sw_triangle_raster_TEX)
    else TRIANGLE_RASTER(sw_triangle_raster)

    #undef TRIANGLE_RASTER
}

// Quad rendering logic

static inline bool sw_quad_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations.
    //       To handle quads crossing the w=0 plane correctly,
    //       we perform the winding order test in homogeneous coordinates directly,
    //       before the perspective division (division by w).
    //       For a convex quad with vertices P0, P1, P2, P3 in sequential order,
    //       the winding order of the quad is the same as the winding order
    //       of the triangle P0 P1 P2. We use the homogeneous triangle
    //       winding test on this first triangle.

    // Preload homogeneous coordinates into local variables
    const float *h0 = RLSW.vertexBuffer[0].homogeneous;
    const float *h1 = RLSW.vertexBuffer[1].homogeneous;
    const float *h2 = RLSW.vertexBuffer[2].homogeneous;

    // NOTE: h3 is not needed for this test
    // const float *h3 = RLSW.vertexBuffer[3].homogeneous;

    // Compute a value proportional to the signed area of the triangle P0 P1 P2
    // in the projected 2D plane, calculated directly using homogeneous coordinates
    // BEFORE division by w.
    // This is the determinant of the matrix formed by the (x, y, w) components
    // of the vertices P0, P1, and P2. Its sign correctly indicates the winding order
    // in homogeneous space and its relationship to the projected 2D winding order,
    // even with negative w values.
    // The determinant formula used here is:
    // h0.x*(h1.y*h2.w - h2.y*h1.w) +
    // h1.x*(h2.y*h0.w - h0.y*h2.w) +
    // h2.x*(h0.y*h1.w - h1.y*h0.w)

    const float hSgnArea =
        h0[0]*(h1[1]*h2[3] - h2[1]*h1[3]) +
        h1[0]*(h2[1]*h0[3] - h0[1]*h2[3]) +
        h2[0]*(h0[1]*h1[3] - h1[1]*h0[3]);

    // Perform face culling based on the winding order determined by the sign
    // of the homogeneous area/determinant of triangle P0 P1 P2.
    // This test is robust for points with w > 0 or w < 0 within the triangle,
    // correctly capturing the change in orientation when crossing the w=0 plane.

    // A positive hSgnArea typically corresponds to a counter-clockwise
    // winding in the projected space when all w > 0.
    // A value of 0 for hSgnArea means P0, P1, P2 are collinear in (x, y, w)
    // space, which corresponds to a degenerate triangle projection.
    // Such quads might also be degenerate or non-planar. They are typically
    // not culled by this test (0 < 0 is false, 0 > 0 is false)
    // and should be handled by the clipper if necessary.

    return (RLSW.cullFace == SW_FRONT)
        ? (hSgnArea < 0.0f) // Cull if winding is "clockwise" in the projected sense
        : (hSgnArea > 0.0f); // Cull if winding is "counter-clockwise" in the projected sense
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
            const float invW = 1.0f/v->homogeneous[3];
            v->homogeneous[3] = invW;

            // Division of XYZ coordinates by weight
            v->homogeneous[0] *= invW;
            v->homogeneous[1] *= invW;
            v->homogeneous[2] *= invW;

            // Division of texture coordinates (perspective-correct)
            v->texcoord[0] *= invW;
            v->texcoord[1] *= invW;

            // Division of colors (perspective-correct)
            v->color[0] *= invW;
            v->color[1] *= invW;
            v->color[2] *= invW;
            v->color[3] *= invW;

            // Transformation to screen space
            sw_project_ndc_to_screen(v->screen, v->homogeneous);
        }
    }
}

static inline bool sw_quad_is_axis_aligned(void)
{
    int horizontal = 0;
    int vertical = 0;

    for (int i = 0; i < 4; i++)
    {
        if (RLSW.vertexBuffer[i].homogeneous[3] != 1.0f) return false;

        const float *v0 = RLSW.vertexBuffer[i].position;
        const float *v1 = RLSW.vertexBuffer[(i + 1)%4].position;

        float dx = v1[0] - v0[0];
        float dy = v1[1] - v0[1];

        if ((fabsf(dx) > 1e-6f) && (fabsf(dy) < 1e-6f)) horizontal++;
        else if ((fabsf(dy) > 1e-6f) && (fabsf(dx) < 1e-6f)) vertical++;
        else return false; // Diagonal edge -> not axis-aligned
    }

    return ((horizontal == 2) && (vertical == 2));
}

static inline void sw_quad_sort_cw(const sw_vertex_t* *output)
{
    // Sort 4 quad vertices into clockwise order with fixed layout:
    //
    // v0 -- v1
    // |      |
    // v3 -- v2
    //
    // The goal is:
    // - v0: top-left (minimum Y, then minimum X)
    // - v1: top-right (minimum Y row, maximum X)
    // - v2: bottom-right (maximum Y, maximum X)
    // - v3: bottom-left (maximum Y, minimum X)
    const sw_vertex_t *input = RLSW.vertexBuffer;

    // Separate vertices into top and bottom based on Y-coordinate
    const sw_vertex_t *top[2] = {NULL, NULL};
    const sw_vertex_t *bottom[2] = {NULL, NULL};
    int topCount = 0, bottomCount = 0;

    // Find minimum and maximum Y
    float minY = input[0].screen[1];
    float maxY = input[0].screen[1];

    for (int i = 1; i < 4; i++)
    {
        if (input[i].screen[1] < minY) minY = input[i].screen[1];
        if (input[i].screen[1] > maxY) maxY = input[i].screen[1];
    }

    // Separate vertices based on Y-coordinate
    for (int i = 0; i < 4; i++)
    {
        if ((input[i].screen[1] == minY) && (topCount < 2)) top[topCount++] = &input[i];
        else if ((input[i].screen[1] == maxY) && (bottomCount < 2)) bottom[bottomCount++] = &input[i];
    }

    // If we don't have enough top/bottom vertices (e.g., Y values are all different),
    // classify vertices as top or bottom based on whether they're closer to minY or maxY
    for (int i = 0; i < 4; i++)
    {
        if ((topCount < 2) && (&input[i] != top[0]) && (&input[i] != bottom[0]) && (&input[i] != bottom[1]))
        {
            if (fabsf(input[i].screen[1] - minY) <= fabsf(input[i].screen[1] - maxY)) top[topCount++] = &input[i];
        }
        if ((bottomCount < 2) && (&input[i] != top[0]) && (&input[i] != top[1]) && (&input[i] != bottom[0]))
        {
            if (fabsf(input[i].screen[1] - maxY) < fabsf(input[i].screen[1] - minY)) bottom[bottomCount++] = &input[i];
        }
    }

    // Sort top vertices by X (left to right)
    if ((topCount == 2) && (top[0]->screen[0] > top[1]->screen[0]))
    {
        const sw_vertex_t *temp = top[0];
        top[0] = top[1];
        top[1] = temp;
    }

    // Sort bottom vertices by X (left to right)
    if ((bottomCount == 2) && (bottom[0]->screen[0] > bottom[1]->screen[0]))
    {
        const sw_vertex_t *temp = bottom[0];
        bottom[0] = bottom[1];
        bottom[1] = temp;
    }

    // Assign vertices in clockwise order as per the required layout
    output[0] = top[0];                 // v0: top-left
    output[1] = top[topCount-1];        // v1: top-right
    output[2] = bottom[bottomCount-1];  // v2: bottom-right
    output[3] = bottom[0];              // v3: bottom-left
}

// TODO: REVIEW: Could a perfectly aligned quad, where one of the four points has a different depth,
// still appear perfectly aligned from a certain point of view?
// Because in that case, we would still need to perform perspective division for textures and colors...
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
    int xMin = (int)(v0->screen[0] + 0.5f);                                     \
    int yMin = (int)(v0->screen[1] + 0.5f);                                     \
    int xMax = (int)(v2->screen[0] + 0.5f);                                     \
    int yMax = (int)(v2->screen[1] + 0.5f);                                     \
                                                                                \
    int width = xMax - xMin;                                                    \
    int height = yMax - yMin;                                                   \
                                                                                \
    if ((width == 0) || (height == 0)) return;                                  \
                                                                                \
    float wRcp = (width > 0.0f)? 1.0f/width : 0.0f;                             \
    float hRcp = (height > 0.0f)? 1.0f/height : 0.0f;                           \
                                                                                \
    /* Calculation of vertex gradients in X and Y */                            \
    float tcDx[2], tcDy[2];                                                     \
    if (ENABLE_TEXTURE) {                                                       \
        tcDx[0] = (v1->texcoord[0] - v0->texcoord[0])*wRcp;                     \
        tcDx[1] = (v1->texcoord[1] - v0->texcoord[1])*wRcp;                     \
        tcDy[0] = (v3->texcoord[0] - v0->texcoord[0])*hRcp;                     \
        tcDy[1] = (v3->texcoord[1] - v0->texcoord[1])*hRcp;                     \
    }                                                                           \
                                                                                \
    float cDx[4], cDy[4];                                                       \
    cDx[0] = (v1->color[0] - v0->color[0])*wRcp;                                \
    cDx[1] = (v1->color[1] - v0->color[1])*wRcp;                                \
    cDx[2] = (v1->color[2] - v0->color[2])*wRcp;                                \
    cDx[3] = (v1->color[3] - v0->color[3])*wRcp;                                \
    cDy[0] = (v3->color[0] - v0->color[0])*hRcp;                                \
    cDy[1] = (v3->color[1] - v0->color[1])*hRcp;                                \
    cDy[2] = (v3->color[2] - v0->color[2])*hRcp;                                \
    cDy[3] = (v3->color[3] - v0->color[3])*hRcp;                                \
                                                                                \
    float zDx, zDy;                                                             \
    zDx = (v1->homogeneous[2] - v0->homogeneous[2])*wRcp;                       \
    zDy = (v3->homogeneous[2] - v0->homogeneous[2])*hRcp;                       \
                                                                                \
    /* Start of quad rasterization */                                           \
    const sw_texture_t *tex;                                                    \
    if (ENABLE_TEXTURE) tex = &RLSW.loadedTextures[RLSW.currentTexture];        \
                                                                                \
    void *cDstBase = RLSW.framebuffer.color;                                    \
    void *dDstBase = RLSW.framebuffer.depth;                                    \
    int wDst = RLSW.framebuffer.width;                                          \
                                                                                \
    float zScanline = v0->homogeneous[2];                                       \
    float uScanline = v0->texcoord[0];                                          \
    float vScanline = v0->texcoord[1];                                          \
                                                                                \
    float colorScanline[4] = {                                                  \
        v0->color[0],                                                           \
        v0->color[1],                                                           \
        v0->color[2],                                                           \
        v0->color[3]                                                            \
    };                                                                          \
                                                                                \
    for (int y = yMin; y < yMax; y++)                                           \
    {                                                                           \
        void *cptr = sw_framebuffer_get_color_addr(cDstBase, y*wDst + xMin);    \
        void *dptr = sw_framebuffer_get_depth_addr(dDstBase, y*wDst + xMin);    \
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
                float depth =  sw_framebuffer_read_depth(dptr);                 \
                if (z > depth) goto discard;                                    \
            }                                                                   \
                                                                                \
            sw_framebuffer_write_depth(dptr, z);                                \
                                                                                \
            if (ENABLE_TEXTURE)                                                 \
            {                                                                   \
                float texColor[4];                                              \
                sw_texture_sample(texColor, tex, u, v, tcDx[0], tcDy[0], tcDx[1], tcDy[1]); \
                srcColor[0] *= texColor[0];                                     \
                srcColor[1] *= texColor[1];                                     \
                srcColor[2] *= texColor[2];                                     \
                srcColor[3] *= texColor[3];                                     \
            }                                                                   \
                                                                                \
            if (ENABLE_COLOR_BLEND)                                             \
            {                                                                   \
                float dstColor[4];                                              \
                sw_framebuffer_read_color(dstColor, cptr);                      \
                                                                                \
                sw_blend_colors(dstColor, srcColor);                            \
                dstColor[0] = sw_saturate(dstColor[0]);                         \
                dstColor[1] = sw_saturate(dstColor[1]);                         \
                dstColor[2] = sw_saturate(dstColor[2]);                         \
                dstColor[3] = sw_saturate(dstColor[3]);                         \
                                                                                \
                sw_framebuffer_write_color(cptr, dstColor);                     \
            }                                                                   \
            else sw_framebuffer_write_color(cptr, srcColor);                    \
                                                                                \
        discard:                                                                \
            z += zDx;                                                           \
            color[0] += cDx[0];                                                 \
            color[1] += cDx[1];                                                 \
            color[2] += cDx[2];                                                 \
            color[3] += cDx[3];                                                 \
                                                                                \
            if (ENABLE_TEXTURE)                                                 \
            {                                                                   \
                u += tcDx[0];                                                   \
                v += tcDx[1];                                                   \
            }                                                                   \
                                                                                \
            sw_framebuffer_inc_color_addr(&cptr);                               \
            sw_framebuffer_inc_depth_addr(&dptr);                               \
        }                                                                       \
                                                                                \
        zScanline += zDy;                                                       \
        colorScanline[0] += cDy[0];                                             \
        colorScanline[1] += cDy[1];                                             \
        colorScanline[2] += cDy[2];                                             \
        colorScanline[3] += cDy[3];                                             \
                                                                                \
        if (ENABLE_TEXTURE)                                                     \
        {                                                                       \
            uScanline += tcDy[0];                                               \
            vScanline += tcDy[1];                                               \
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

    if ((RLSW.vertexCounter == 4) && sw_quad_is_axis_aligned())
    {
        if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_quad_raster_axis_aligned_TEX_DEPTH_BLEND();
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) sw_quad_raster_axis_aligned_DEPTH_BLEND();
        else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) sw_quad_raster_axis_aligned_TEX_BLEND();
        else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) sw_quad_raster_axis_aligned_TEX_DEPTH();
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) sw_quad_raster_axis_aligned_BLEND();
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) sw_quad_raster_axis_aligned_DEPTH();
        else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D)) sw_quad_raster_axis_aligned_TEX();
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

    if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH)
    else if (SW_STATE_CHECK(SW_STATE_BLEND)) TRIANGLE_RASTER(sw_triangle_raster_BLEND)
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) TRIANGLE_RASTER(sw_triangle_raster_DEPTH)
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D)) TRIANGLE_RASTER(sw_triangle_raster_TEX)
    else TRIANGLE_RASTER(sw_triangle_raster)

    #undef TRIANGLE_RASTER
}

// Line rendering logic

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
    int x1 = (int)(v0->screen[0] + 0.5f);                               \
    int y1 = (int)(v0->screen[1] + 0.5f);                               \
    int x2 = (int)(v1->screen[0] + 0.5f);                               \
    int y2 = (int)(v1->screen[1] + 0.5f);                               \
                                                                        \
    int dx = x2 - x1;                                                   \
    int dy = y2 - y1;                                                   \
                                                                        \
    /* Handling of lines that are more horizontal or vertical */        \
    if ((dx == 0) && (dy == 0))                                             \
    {                                                                   \
        /* TODO: A point should be rendered here */                     \
        return;                                                         \
    }                                                                   \
                                                                        \
    bool yLonger = (abs(dy) > abs(dx));                                 \
    int longLen, shortLen;                                              \
                                                                        \
    if (yLonger)                                                        \
    {                                                                   \
        longLen = dy;                                                   \
        shortLen = dx;                                                  \
    }                                                                   \
    else                                                                \
    {                                                                   \
        longLen = dx;                                                   \
        shortLen = dy;                                                  \
    }                                                                   \
                                                                        \
    /* Handling of traversal direction */                               \
    int sgnInc = (longLen < 0)? -1 : 1;                                 \
    int abslongLen = abs(longLen);                                      \
                                                                        \
    /* Calculation of the increment step for the shorter coordinate */  \
    int decInc = 0;                                                     \
    if (abslongLen != 0) decInc = (shortLen << 16)/abslongLen;          \
                                                                        \
    float longLenRcp = (abslongLen != 0)? (1.0f/abslongLen) : 0.0f;     \
                                                                        \
    /* Calculation of interpolation steps */                            \
    const float zStep = (v1->homogeneous[2] - v0->homogeneous[2])*longLenRcp; \
    const float rStep = (v1->color[0] - v0->color[0])*longLenRcp;       \
    const float gStep = (v1->color[1] - v0->color[1])*longLenRcp;       \
    const float bStep = (v1->color[2] - v0->color[2])*longLenRcp;       \
    const float aStep = (v1->color[3] - v0->color[3])*longLenRcp;       \
                                                                        \
    float z = v0->homogeneous[2];                                       \
                                                                        \
    float color[4] = {                                                  \
        v0->color[0],                                                   \
        v0->color[1],                                                   \
        v0->color[2],                                                   \
        v0->color[3]                                                    \
    };                                                                  \
                                                                        \
    const int fbWidth = RLSW.framebuffer.width;                         \
    void *cBuffer = RLSW.framebuffer.color;                             \
    void *dBuffer = RLSW.framebuffer.depth;                             \
                                                                        \
    int j = 0;                                                          \
    if (yLonger)                                                        \
    {                                                                   \
        for (int i = 0; i != longLen; i += sgnInc)                      \
        {                                                               \
            int offset = (y1 + i)*fbWidth + (x1 + (j >> 16));           \
            void *dptr = sw_framebuffer_get_depth_addr(dBuffer, offset); \
            void *cptr;                                                 \
                                                                        \
            if (ENABLE_DEPTH_TEST)                                      \
            {                                                           \
                float depth = sw_framebuffer_read_depth(dptr);          \
                if (z > depth) goto discardA;                           \
            }                                                           \
                                                                        \
            sw_framebuffer_write_depth(dptr, z);                        \
                                                                        \
            cptr = sw_framebuffer_get_color_addr(cBuffer, offset); \
                                                                        \
            if (ENABLE_COLOR_BLEND)                                     \
            {                                                           \
                float dstColor[4];                                      \
                sw_framebuffer_read_color(dstColor, cptr);              \
                                                                        \
                sw_blend_colors(dstColor, color);                       \
                sw_framebuffer_write_color(cptr, dstColor);             \
            }                                                           \
            else sw_framebuffer_write_color(cptr, color);               \
                                                                        \
        discardA:                                                       \
            j += decInc;                                                \
            z += zStep;                                                 \
            color[0] += rStep;                                          \
            color[1] += gStep;                                          \
            color[2] += bStep;                                          \
            color[3] += aStep;                                          \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        for (int i = 0; i != longLen; i += sgnInc)                      \
        {                                                               \
            int offset = (y1 + (j >> 16))*fbWidth + (x1 + i);           \
            void *dptr = sw_framebuffer_get_depth_addr(dBuffer, offset); \
            void *cptr;                                                 \
                                                                        \
            if (ENABLE_DEPTH_TEST)                                      \
            {                                                           \
                float depth = sw_framebuffer_read_depth(dptr);          \
                if (z > depth) goto discardB;                           \
            }                                                           \
                                                                        \
            sw_framebuffer_write_depth(dptr, z);                        \
                                                                        \
            cptr = sw_framebuffer_get_color_addr(cBuffer, offset);      \
                                                                        \
            if (ENABLE_COLOR_BLEND)                                     \
            {                                                           \
                float dstColor[4];                                      \
                sw_framebuffer_read_color(dstColor, cptr);              \
                                                                        \
                sw_blend_colors(dstColor, color);                       \
                sw_framebuffer_write_color(cptr, dstColor);             \
            }                                                           \
            else sw_framebuffer_write_color(cptr, color);                \
                                                                        \
        discardB:                                                       \
            j += decInc;                                                \
            z += zStep;                                                 \
            color[0] += rStep;                                          \
            color[1] += gStep;                                          \
            color[2] += bStep;                                          \
            color[3] += aStep;                                          \
        }                                                               \
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

// Point rendering logic

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
                                                                            \
    void *dptr = sw_framebuffer_get_depth_addr(RLSW.framebuffer.depth, offset); \
                                                                            \
    if (ENABLE_DEPTH_TEST)                                                  \
    {                                                                       \
        float depth = sw_framebuffer_read_depth(dptr);                      \
        if (z > depth) return;                                              \
    }                                                                       \
                                                                            \
    sw_framebuffer_write_depth(dptr, z);                                    \
                                                                            \
    void *cptr = sw_framebuffer_get_color_addr(RLSW.framebuffer.color, offset); \
                                                                            \
    if (ENABLE_COLOR_BLEND)                                                 \
    {                                                                       \
        float dstColor[4];                                                  \
        sw_framebuffer_read_color(dstColor, cptr);                          \
                                                                            \
        sw_blend_colors(dstColor, color);                                   \
        sw_framebuffer_write_color(cptr, dstColor);                         \
    }                                                                       \
    else sw_framebuffer_write_color(cptr, color);                           \
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

// Polygon modes mendering logic

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

// Validity check helper functions

static inline bool sw_is_texture_valid(uint32_t id)
{
    bool valid = true;

    if (id == 0) valid = false;
    else if (id >= SW_MAX_TEXTURES) valid = false;
    else if (RLSW.loadedTextures[id].pixels.cptr == 0) valid = false;

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

    RLSW.clearColor[0] = 0.0f;
    RLSW.clearColor[1] = 0.0f;
    RLSW.clearColor[2] = 0.0f;
    RLSW.clearColor[3] = 1.0f;
    RLSW.clearDepth = 1.0f;

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

    static const float defTex[3*2*2] = {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    RLSW.loadedTextures[0].pixels.cptr = defTex;
    RLSW.loadedTextures[0].width = 2;
    RLSW.loadedTextures[0].height = 2;
    RLSW.loadedTextures[0].wMinus1 = 1;
    RLSW.loadedTextures[0].hMinus1 = 1;
    RLSW.loadedTextures[0].format = SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32;
    RLSW.loadedTextures[0].minFilter = SW_NEAREST;
    RLSW.loadedTextures[0].magFilter = SW_NEAREST;
    RLSW.loadedTextures[0].sWrap = SW_REPEAT;
    RLSW.loadedTextures[0].tWrap = SW_REPEAT;
    RLSW.loadedTextures[0].tx = 0.5f;
    RLSW.loadedTextures[0].ty = 0.5f;
    RLSW.loadedTextures[0].copy = false;

    RLSW.loadedTextureCount = 1;

    return true;
}

void swClose(void)
{
    for (int i = 1; i < RLSW.loadedTextureCount; i++)
    {
        sw_texture_t *texture = &RLSW.loadedTextures[i];
        if (sw_is_texture_valid(i) && texture->copy) SW_FREE(texture->pixels.ptr);
    }

    SW_FREE(RLSW.framebuffer.color);
    SW_FREE(RLSW.framebuffer.depth);
    SW_FREE(RLSW.loadedTextures);
    SW_FREE(RLSW.freeTextureIds);

    RLSW = CURLY_INIT(sw_context_t) { 0 };
}

bool swResizeFramebuffer(int w, int h)
{
    return sw_framebuffer_resize(w, h);
}

void swCopyFramebuffer(int x, int y, int w, int h, SWformat format, SWtype type, void *pixels)
{
    sw_pixelformat_t pFormat = (sw_pixelformat_t)sw_get_pixel_format(format, type);

    if (w <= 0)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (h <= 0)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (w > RLSW.framebuffer.width) w = RLSW.framebuffer.width;
    if (h > RLSW.framebuffer.height) h = RLSW.framebuffer.height;

    x = sw_clampi(x, 0, w);
    y = sw_clampi(y, 0, h);

    switch (pFormat)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: sw_framebuffer_copy_to_GRAYALPHA(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: sw_framebuffer_copy_to_GRAYALPHA(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5: sw_framebuffer_copy_to_R5G6B5(x, y, w, h, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8: sw_framebuffer_copy_to_R8G8B8(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: sw_framebuffer_copy_to_R5G5B5A1(x, y, w, h, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: sw_framebuffer_copy_to_R4G4B4A4(x, y, w, h, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: sw_framebuffer_copy_to_R8G8B8A8(x, y, w, h, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32: sw_framebuffer_copy_to_R32(x, y, w, h, (float *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32: sw_framebuffer_copy_to_R32G32B32(x, y, w, h, (float *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: sw_framebuffer_copy_to_R32G32B32A32(x, y, w, h, (float *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16: sw_framebuffer_copy_to_R16(x, y, w, h, (sw_half_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16: sw_framebuffer_copy_to_R16G16B16(x, y, w, h, (sw_half_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: sw_framebuffer_copy_to_R16G16B16A16(x, y, w, h, (sw_half_t *)pixels); break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swBlitFramebuffer(int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, SWformat format, SWtype type, void *pixels)
{
    sw_pixelformat_t pFormat = (sw_pixelformat_t)sw_get_pixel_format(format, type);

    if (wSrc <= 0)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (hSrc <= 0)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (wSrc > RLSW.framebuffer.width) wSrc = RLSW.framebuffer.width;
    if (hSrc > RLSW.framebuffer.height) hSrc = RLSW.framebuffer.height;

    xSrc = sw_clampi(xSrc, 0, wSrc);
    ySrc = sw_clampi(ySrc, 0, hSrc);

    switch (pFormat)
    {
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: sw_framebuffer_blit_to_GRAYALPHA(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: sw_framebuffer_blit_to_GRAYALPHA(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5: sw_framebuffer_blit_to_R5G6B5(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8: sw_framebuffer_blit_to_R8G8B8(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: sw_framebuffer_blit_to_R5G5B5A1(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: sw_framebuffer_blit_to_R4G4B4A4(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint16_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: sw_framebuffer_blit_to_R8G8B8A8(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32: sw_framebuffer_blit_to_R32(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (uint8_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32: sw_framebuffer_blit_to_R32G32B32(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (float *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: sw_framebuffer_blit_to_R32G32B32A32(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (float*)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16: sw_framebuffer_blit_to_R16(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (sw_half_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16: sw_framebuffer_blit_to_R16G16B16(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (sw_half_t *)pixels); break;
        case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: sw_framebuffer_blit_to_R16G16B16A16(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, (sw_half_t *)pixels); break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void *swGetColorBuffer(int *w, int *h)
{
    if (w) *w = RLSW.framebuffer.width;
    if (h) *h = RLSW.framebuffer.height;

    return RLSW.framebuffer.color;
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
            v[0] = RLSW.clearColor[0];
            v[1] = RLSW.clearColor[1];
            v[2] = RLSW.clearColor[2];
            v[3] = RLSW.clearColor[3];
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

    RLSW.vpHalfSize[0] = (int)(width/2.0f + 0.5f);
    RLSW.vpHalfSize[1] = (int)(height/2.0f + 0.5f);

    RLSW.vpCenter[0] = x + RLSW.vpHalfSize[0];
    RLSW.vpCenter[1] = y + RLSW.vpHalfSize[1];

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
    RLSW.clearColor[0] = r;
    RLSW.clearColor[1] = g;
    RLSW.clearColor[2] = b;
    RLSW.clearColor[3] = a;
}

void swClear(uint32_t bitmask)
{
    int size = RLSW.framebuffer.width*RLSW.framebuffer.height;

    if ((bitmask & (SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT)) == (SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT))
    {
        sw_framebuffer_fill(RLSW.framebuffer.color, RLSW.framebuffer.depth,size, RLSW.clearColor, RLSW.clearDepth);
    }
    else if (bitmask & (SW_COLOR_BUFFER_BIT))
    {
        sw_framebuffer_fill_color(RLSW.framebuffer.color, size, RLSW.clearColor);
    }
    else if (bitmask & SW_DEPTH_BUFFER_BIT)
    {
        sw_framebuffer_fill_depth(RLSW.framebuffer.depth, size, RLSW.clearDepth);
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
    swVertex4fv(v);
}

void swVertex2f(float x, float y)
{
    const float v[4] = { x, y, 0.0f, 1.0f };
    swVertex4fv(v);
}

void swVertex2fv(const float *v)
{
    const float v4[4] = { v[0], v[1], 0.0f, 1.0f };
    swVertex4fv(v4);
}

void swVertex3i(int x, int y, int z)
{
    const float v[4] = { (float)x, (float)y, (float)z, 1.0f };
    swVertex4fv(v);
}

void swVertex3f(float x, float y, float z)
{
    const float v[4] = { x, y, z, 1.0f };
    swVertex4fv(v);
}

void swVertex3fv(const float *v)
{
    const float v4[4] = { v[0], v[1], v[2], 1.0f };
    swVertex4fv(v4);
}

void swVertex4i(int x, int y, int z, int w)
{
    const float v[4] = { (float)x, (float)y, (float)z, (float)w };
    swVertex4fv(v);
}

void swVertex4f(float x, float y, float z, float w)
{
    const float v[4] = { x, y, z, w };
    swVertex4fv(v);
}

void swVertex4fv(const float *v)
{
    // Copy the position in the current vertex
    sw_vertex_t *vertex = &RLSW.vertexBuffer[RLSW.vertexCounter++];
    for (int i = 0; i < 4; i++) vertex->position[i] = v[i];

    // Copy additonal vertex data
    for (int i = 0; i < 2; i++) vertex->texcoord[i] = RLSW.current.texcoord[i];
    for (int i = 0; i < 4; i++) vertex->color[i] = RLSW.current.color[i];

    // Calculation of homogeneous coordinates
    const float *m = RLSW.matMVP;
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

void swColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
    float cv[4];
    cv[0] = (float)r/255;
    cv[1] = (float)g/255;
    cv[2] = (float)b/255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3ubv(const uint8_t *v)
{
    float cv[4];
    cv[0] = (float)v[0]/255;
    cv[1] = (float)v[1]/255;
    cv[2] = (float)v[2]/255;
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
    cv[0] = (float)r/255;
    cv[1] = (float)g/255;
    cv[2] = (float)b/255;
    cv[3] = (float)a/255;

    swColor4fv(cv);
}

void swColor4ubv(const uint8_t *v)
{
    float cv[4];
    cv[0] = (float)v[0]/255;
    cv[1] = (float)v[1]/255;
    cv[2] = (float)v[2]/255;
    cv[3] = (float)v[3]/255;

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
        swTexCoord2f(0.0f, 0.0f);
        swColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        for (int i = offset; i < count; i++)
        {
            if (RLSW.array.texcoords) swTexCoord2fv(RLSW.array.texcoords + 2*i);
            if (RLSW.array.colors) swColor4ubv(RLSW.array.colors + 4*i);
            swVertex3fv(RLSW.array.positions + 3*i);
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

        if (RLSW.loadedTextures[textures[i]].copy)
        {
            SW_FREE(RLSW.loadedTextures[textures[i]].pixels.ptr);
        }

        RLSW.loadedTextures[textures[i]].pixels.cptr = NULL;
        RLSW.freeTextureIds[RLSW.freeTextureIdCount++] = textures[i];
    }
}

void swTexImage2D(int width, int height, SWformat format, SWtype type, bool copy, const void *data)
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

    if (copy)
    {
        int bytes = sw_get_pixel_bytes((sw_pixelformat_t)pixelFormat);
        int size = bytes*width*height;
        texture->pixels.ptr = SW_MALLOC(size);

        if (texture->pixels.ptr == NULL)
        {
            RLSW.errCode = SW_STACK_OVERFLOW; // WARING: Out of memory...
            return;
        }

        for (int i = 0; i < size; i++)
        {
            ((uint8_t *)texture->pixels.ptr)[i] = ((uint8_t *)data)[i];
        }
    }
    else texture->pixels.cptr = data;

    texture->width = width;
    texture->height = height;
    texture->wMinus1 = width - 1;
    texture->hMinus1 = height - 1;
    texture->format = (sw_pixelformat_t)pixelFormat;
    texture->tx = 1.0f/width;
    texture->ty = 1.0f/height;
    texture->copy = copy;
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

    if (RLSW.loadedTextures[id].pixels.cptr == NULL)
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    RLSW.currentTexture = id;
}

#endif // RLSW_IMPLEMENTATION
