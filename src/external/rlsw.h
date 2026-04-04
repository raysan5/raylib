/**********************************************************************************************
*
*   rlsw v1.5 - An OpenGL 1.1-style software renderer implementation
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
*       rlsw capabilities could be customized defining some internal
*       values before library inclusion (default values listed):
*
*           #define SW_FRAMEBUFFER_OUTPUT_BGRA      true
*           #define SW_FRAMEBUFFER_COLOR_TYPE       R8G8B8A8
*           #define SW_FRAMEBUFFER_DEPTH_TYPE       D32
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

#define RLSW_VERSION  "1.5"

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
#ifndef SW_CALLOC
    #define SW_CALLOC(n,sz) calloc(n,sz)
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

#ifndef SW_FRAMEBUFFER_OUTPUT_BGRA
    #define SW_FRAMEBUFFER_OUTPUT_BGRA      true
#endif

#ifndef SW_FRAMEBUFFER_COLOR_TYPE
    #define SW_FRAMEBUFFER_COLOR_TYPE       R8G8B8A8    // Or R5G6B5, R3G3B2, etc; see `sw_pixelformat_t`
#endif

#ifndef SW_FRAMEBUFFER_DEPTH_TYPE
    #define SW_FRAMEBUFFER_DEPTH_TYPE       D32         // Or D16, D8
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

#ifndef SW_MAX_FRAMEBUFFERS
    #define SW_MAX_FRAMEBUFFERS             8
#endif

#ifndef SW_MAX_TEXTURES
    #define SW_MAX_TEXTURES                 128
#endif

// Enables the use of a lookup table for uint8_t to float conversion
// Requires an additional 1KB of global memory
// Disabled when SIMD intrinsics are enabled
#ifndef SW_USE_COLOR_LUT
    #if RLSW_USE_SIMD_INTRINSICS
        #define SW_USE_COLOR_LUT            false
    #else
        #define SW_USE_COLOR_LUT            true
    #endif
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
#define GL_DEPTH_COMPONENT			        0x1902

#define GL_BYTE                             0x1400
#define GL_UNSIGNED_BYTE                    0x1401
#define GL_SHORT                            0x1402
#define GL_UNSIGNED_SHORT                   0x1403
#define GL_INT                              0x1404
#define GL_UNSIGNED_INT                     0x1405
#define GL_FLOAT                            0x1406
#define GL_UNSIGNED_BYTE_3_3_2              0x8032
#define GL_UNSIGNED_SHORT_5_6_5             0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4           0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1           0x8034

// OpenGL internal formats (not all are supported; see SWinternalformat)
#define GL_ALPHA4                           0x803B
#define GL_ALPHA8                           0x803C
#define GL_ALPHA12                          0x803D
#define GL_ALPHA16                          0x803E
#define GL_LUMINANCE4                       0x803F
#define GL_LUMINANCE8                       0x8040
#define GL_LUMINANCE12                      0x8041
#define GL_LUMINANCE16                      0x8042
#define GL_LUMINANCE4_ALPHA4                0x8043
#define GL_LUMINANCE6_ALPHA2                0x8044
#define GL_LUMINANCE8_ALPHA8                0x8045
#define GL_LUMINANCE12_ALPHA4               0x8046
#define GL_LUMINANCE12_ALPHA12              0x8047
#define GL_LUMINANCE16_ALPHA16              0x8048
#define GL_INTENSITY                        0x8049
#define GL_INTENSITY4                       0x804A
#define GL_INTENSITY8                       0x804B
#define GL_INTENSITY12                      0x804C
#define GL_INTENSITY16                      0x804D
#define GL_R3_G3_B2                         0x2A10
#define GL_RGB4                             0x804F
#define GL_RGB5                             0x8050
#define GL_RGB8                             0x8051
#define GL_RGB10                            0x8052
#define GL_RGB12                            0x8053
#define GL_RGB16                            0x8054
#define GL_RGBA2                            0x8055
#define GL_RGBA4                            0x8056
#define GL_RGB5_A1                          0x8057
#define GL_RGBA8                            0x8058
#define GL_RGB10_A2                         0x8059
#define GL_RGBA12                           0x805A
#define GL_RGBA16                           0x805B

// OpenGL internal formats extension
#define GL_DEPTH_COMPONENT16                0x81A5
#define GL_DEPTH_COMPONENT24                0x81A6
#define GL_DEPTH_COMPONENT32                0x81A7
#define GL_DEPTH_COMPONENT32F               0x8CAC
#define GL_R16F                             0x822D
#define GL_RGB16F                           0x881B
#define GL_RGBA16F                          0x881A
#define GL_R32F                             0x822E
#define GL_RGB32F                           0x8815
#define GL_RGBA32F                          0x8814

// OpenGL GL_EXT_framebuffer_object
#define GL_DRAW_FRAMEBUFFER_BINDING                     0x8CA6
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GL_FRAMEBUFFER_COMPLETE                         0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT            0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT    0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS            0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED                      0x8CDD
#define GL_COLOR_ATTACHMENT0                            0x8CE0
#define GL_DEPTH_ATTACHMENT                             0x8D00
#define GL_STENCIL_ATTACHMENT                           0x8D20  // WARNING: Not implemented (defined for RLGL)

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
#define GL_RENDERBUFFER                     0x8D41
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X      0x8516
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y      0x8518
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z      0x851A
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X      0x8515
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y      0x8517
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z      0x8519

//----------------------------------------------------------------------------------
// OpenGL Bindings to rlsw
//----------------------------------------------------------------------------------
#define glReadPixels(x, y, w, h, f, t, p)           swReadPixels((x), (y), (w), (h), (f), (t), (p))
#define glEnable(state)                             swEnable((state))
#define glDisable(state)                            swDisable((state))
#define glGetIntegerv(pname, params)                swGetIntegerv((pname), (params))
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
#define glBindTexture(tr, id)                       swBindTexture((id))
#define glTexImage2D(tr, l, if, w, h, b, f, t, p)   swTexImage2D((w), (h), (f), (t), (p))
#define glTexSubImage2D(tr, l, x, y, w, h, f, t, p) swTexSubImage2D((x), (y), (w), (h), (f), (t), (p));
#define glTexParameteri(tr, pname, param)           swTexParameteri((pname), (param))

// OpenGL GL_EXT_framebuffer_object
#define glGenFramebuffers(c, v)                             swGenFramebuffers((c), (v))
#define glDeleteFramebuffers(c, v)                          swDeleteFramebuffers((c), (v))
#define glBindFramebuffer(tr, id)                           swBindFramebuffer((id))
#define glFramebufferTexture2D(tr, a, t, tex, ml)           swFramebufferTexture2D((a), (tex))
#define glFramebufferRenderbuffer(tr, a, rbtr, rb)          swFramebufferTexture2D((a), (rb))
#define glCheckFramebufferStatus(tr)                        swCheckFramebufferStatus()
#define glGetFramebufferAttachmentParameteriv(tr, a, p, v)  swGetFramebufferAttachmentParameteriv((a), (p), (v))
#define glGenRenderbuffers(c, v)                            swGenTextures((c), (v))
#define glDeleteRenderbuffers(c, v)                         swDeleteTextures((c), (v))
#define glBindRenderbuffer(tr, rb)                          swBindTexture((rb))
#define glRenderbufferStorage(tr, f, w, h)                  swTexStorage2D((w), (h), (f))

// OpenGL functions NOT IMPLEMENTED by rlsw
#define glDepthMask(X)                              ((void)(X))
#define glColorMask(X,Y,Z,W)                        ((void)(X),(void)(Y),(void)(Z),(void)(W))
#define glPixelStorei(X,Y)                          ((void)(X),(void)(Y))
#define glHint(X,Y)                                 ((void)(X),(void)(Y))
#define glShadeModel(X)                             ((void)(X))
#define glFrontFace(X)                              ((void)(X))
#define glDepthFunc(X)                              ((void)(X))
#define glGetTexImage(X,Y,Z,W,A)                    ((void)(X),(void)(Y),(void)(Z),(void)(W),(void)(A))
#define glNormal3f(X,Y,Z)                           ((void)(X),(void)(Y),(void)(Z))
#define glNormal3fv(X)                              ((void)(X))
#define glNormalPointer(X,Y,Z)                      ((void)(X),(void)(Y),(void)(Z))

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
    SW_VIEWPORT = GL_VIEWPORT,
    SW_DRAW_FRAMEBUFFER_BINDING = GL_DRAW_FRAMEBUFFER_BINDING,
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
    SW_DRAW_INVALID = -1,
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
    SW_DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
} SWformat;

typedef enum {
    SW_UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    SW_UNSIGNED_BYTE_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
    SW_BYTE = GL_BYTE,
    SW_UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
    SW_UNSIGNED_SHORT_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
    SW_UNSIGNED_SHORT_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
    SW_UNSIGNED_SHORT_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
    SW_SHORT = GL_SHORT,
    SW_UNSIGNED_INT = GL_UNSIGNED_INT,
    SW_INT = GL_INT,
    SW_FLOAT = GL_FLOAT
} SWtype;

typedef enum {
    SW_LUMINANCE8 = GL_LUMINANCE8,
    SW_LUMINANCE8_ALPHA8 = GL_LUMINANCE8_ALPHA8,
    SW_R3_G3_B2 = GL_R3_G3_B2,
    SW_RGB8 = GL_RGB8,
    SW_RGBA4 = GL_RGBA4,
    SW_RGB5_A1 = GL_RGB5_A1,
    SW_RGBA8 = GL_RGBA8,
    SW_R16F = GL_R16F,
    SW_RGB16F = GL_RGB16F,
    SW_RGBA16F = GL_RGBA16F,
    SW_R32F = GL_R32F,
    SW_RGB32F = GL_RGB32F,
    SW_RGBA32F = GL_RGBA32F,
    SW_DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
    SW_DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
    SW_DEPTH_COMPONENT32 = GL_DEPTH_COMPONENT32,
    SW_DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
    //SW_R5_G6_B5, // Not defined by OpenGL
} SWinternalformat;

typedef enum {
    SW_NEAREST = GL_NEAREST,
    SW_LINEAR  = GL_LINEAR
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
    SW_COLOR_ATTACHMENT = GL_COLOR_ATTACHMENT0,
    SW_DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT,
} SWattachment;

typedef enum {
    SW_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
    SW_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
} SWattachget;

typedef enum {
    SW_FRAMEBUFFER_COMPLETE = GL_FRAMEBUFFER_COMPLETE,
    SW_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
    SW_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
    SW_FRAMEBUFFER_INCOMPLETE_DIMENSIONS = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
} SWfbstatus;

typedef enum {
    SW_NO_ERROR = GL_NO_ERROR,
    SW_INVALID_ENUM = GL_INVALID_ENUM,
    SW_INVALID_VALUE = GL_INVALID_VALUE,
    SW_STACK_OVERFLOW = GL_STACK_OVERFLOW,
    SW_STACK_UNDERFLOW = GL_STACK_UNDERFLOW,
    SW_INVALID_OPERATION = GL_INVALID_OPERATION,
    SW_OUT_OF_MEMORY = GL_OUT_OF_MEMORY,
} SWerrcode;

//------------------------------------------------------------------------------------
// Functions Declaration - Public API
//------------------------------------------------------------------------------------
SWAPI bool swInit(int w, int h);
SWAPI void swClose(void);

SWAPI bool swResize(int w, int h);
SWAPI void swReadPixels(int x, int y, int w, int h, SWformat format, SWtype type, void *pixels);
SWAPI void swBlitPixels(int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, SWformat format, SWtype type, void *pixels);
SWAPI void *swGetColorBuffer(int *width, int *height); // Restored for ESP-IDF compatibility

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
SWAPI void swBindTexture(uint32_t id);
SWAPI void swTexImage2D(int width, int height, SWformat format, SWtype type, const void *data);
SWAPI void swTexSubImage2D(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
SWAPI void swTexParameteri(int param, int value);

SWAPI void swGenFramebuffers(int count, uint32_t *framebuffers);
SWAPI void swDeleteFramebuffers(int count, uint32_t *framebuffers);
SWAPI void swBindFramebuffer(uint32_t id);
SWAPI void swFramebufferTexture2D(SWattachment attach, uint32_t texture);
SWAPI SWfbstatus swCheckFramebufferStatus(void);
SWAPI void swGetFramebufferAttachmentParameteriv(SWattachment attachment, SWattachget property, int *v);

#endif // RLSW_H

/***********************************************************************************
*
*   RLSW IMPLEMENTATION
*
************************************************************************************/
#if defined(RLSW_IMPLEMENTATION)

#undef RLSW_IMPLEMENTATION  // Undef to allow template expanding without implementation redefinition

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

#if RLSW_USE_SIMD_INTRINSICS
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
#endif

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

// When clipping a convex polygon against a plane, at most one vertex is added.
// Starting from a quadrilateral (4 vertices), clipped sequentially against
// the frustum (6 planes) then the scissor rectangle (4 planes):
// 4 + 6 + 4 = 14 vertices maximum.
#define SW_MAX_CLIPPED_POLYGON_VERTICES 14
#define SW_CLIP_EPSILON                 1e-4f

#define SW_HANDLE_NULL          0u
#define SW_POOL_SLOT_LIVE       0x80u   // bit7 of the generation byte
#define SW_POOL_SLOT_VER_MASK   0x7Fu   // bits6:0 = anti-ABA counter

#define SW_CONCAT(a, b) a##b
#define SW_CONCATX(a, b) SW_CONCAT(a, b)

#define SW_FRAMEBUFFER_COLOR8_GET(c,p,o) SW_CONCATX(sw_pixel_read_color8_, SW_FRAMEBUFFER_COLOR_TYPE)((c),(p),(o))
#define SW_FRAMEBUFFER_COLOR_GET(c,p,o) SW_CONCATX(sw_pixel_read_color_, SW_FRAMEBUFFER_COLOR_TYPE)((c),(p),(o))
#define SW_FRAMEBUFFER_COLOR_SET(p,c,o) SW_CONCATX(sw_pixel_write_color_, SW_FRAMEBUFFER_COLOR_TYPE)((p),(c),(o))

#define SW_FRAMEBUFFER_DEPTH_GET(p,o) SW_CONCATX(sw_pixel_read_depth_, SW_FRAMEBUFFER_DEPTH_TYPE)((p),(o))
#define SW_FRAMEBUFFER_DEPTH_SET(p,d,o) SW_CONCATX(sw_pixel_write_depth_, SW_FRAMEBUFFER_DEPTH_TYPE)((p),(d),(o))

#define SW_FRAMEBUFFER_COLOR_FORMAT SW_CONCATX(SW_PIXELFORMAT_COLOR_, SW_FRAMEBUFFER_COLOR_TYPE)
#define SW_FRAMEBUFFER_DEPTH_FORMAT SW_CONCATX(SW_PIXELFORMAT_DEPTH_, SW_FRAMEBUFFER_DEPTH_TYPE)

#define SW_FRAMEBUFFER_COLOR_SIZE SW_PIXELFORMAT_SIZE[SW_FRAMEBUFFER_COLOR_FORMAT]
#define SW_FRAMEBUFFER_DEPTH_SIZE SW_PIXELFORMAT_SIZE[SW_FRAMEBUFFER_DEPTH_FORMAT]

#define SW_STATE_SCISSOR_TEST   (1 << 0)
#define SW_STATE_TEXTURE_2D     (1 << 1)
#define SW_STATE_DEPTH_TEST     (1 << 2)
#define SW_STATE_CULL_FACE      (1 << 3)
#define SW_STATE_BLEND          (1 << 4)

#define SW_BLEND_FLAG_NOOP          (1 << 0)
#define SW_BLEND_FLAG_NEEDS_ALPHA   (1 << 1)

//----------------------------------------------------------------------------------
// Module Types and Structures Definition
//----------------------------------------------------------------------------------
// Aliases types
typedef uint32_t sw_handle_t;
typedef uint16_t sw_half_t;

// Pixel data format type
typedef enum {
    SW_PIXELFORMAT_UNKNOWN = 0,
    SW_PIXELFORMAT_COLOR_GRAYSCALE,         // 8 bit per pixel (no alpha)
    SW_PIXELFORMAT_COLOR_GRAYALPHA,         // 8*2 bpp (2 channels)
    SW_PIXELFORMAT_COLOR_R3G3B2,            // 8 bpp
    SW_PIXELFORMAT_COLOR_R5G6B5,            // 16 bpp
    SW_PIXELFORMAT_COLOR_R8G8B8,            // 24 bpp
    SW_PIXELFORMAT_COLOR_R5G5B5A1,          // 16 bpp (1 bit alpha)
    SW_PIXELFORMAT_COLOR_R4G4B4A4,          // 16 bpp (4 bit alpha)
    SW_PIXELFORMAT_COLOR_R8G8B8A8,          // 32 bpp
    SW_PIXELFORMAT_COLOR_R32,               // 32 bpp (1 channel - float)
    SW_PIXELFORMAT_COLOR_R32G32B32,         // 32*3 bpp (3 channels - float)
    SW_PIXELFORMAT_COLOR_R32G32B32A32,      // 32*4 bpp (4 channels - float)
    SW_PIXELFORMAT_COLOR_R16,               // 16 bpp (1 channel - half float)
    SW_PIXELFORMAT_COLOR_R16G16B16,         // 16*3 bpp (3 channels - half float)
    SW_PIXELFORMAT_COLOR_R16G16B16A16,      // 16*4 bpp (4 channels - half float)
    SW_PIXELFORMAT_DEPTH_D8,                // 1 bpp
    SW_PIXELFORMAT_DEPTH_D16,               // 2 bpp
    SW_PIXELFORMAT_DEPTH_D32,               // 4 bpp
    SW_PIXELFORMAT_COUNT
} sw_pixelformat_t;

typedef enum {
    SW_PIXEL_ALPHA_NONE = 0,    // No transparency
    SW_PIXEL_ALPHA_BIN,         // Binary transparency
    SW_PIXEL_ALPHA_YES,         // Contains transparency
} sw_pixel_alpha_t;

// Forward declarations
typedef struct sw_vertex sw_vertex_t;

// Pixel getter functions
typedef void (*sw_pixel_read_color8_f)(uint8_t *SW_RESTRICT, const void *SW_RESTRICT, uint32_t);
typedef void (*sw_pixel_read_color_f)(float *SW_RESTRICT, const void *SW_RESTRICT, uint32_t);

// Pixel setter functions
typedef void (*sw_pixel_write_color8_f)(void *SW_RESTRICT, const uint8_t *SW_RESTRICT, uint32_t);
typedef void (*sw_pixel_write_color_f)(void *SW_RESTRICT, const float *SW_RESTRICT, uint32_t);

// Color blending function
typedef void (*sw_blend_f)(float *SW_RESTRICT, const float *SW_RESTRICT);

// Rasterizer functions
typedef void (*sw_raster_triangle_f)(const sw_vertex_t*, const sw_vertex_t*, const sw_vertex_t*);
typedef void (*sw_raster_quad_f)(const sw_vertex_t*, const sw_vertex_t*, const sw_vertex_t*, const sw_vertex_t*);
typedef void (*sw_raster_line_f)(const sw_vertex_t*, const sw_vertex_t*);
typedef void (*sw_raster_point_f)(const sw_vertex_t*);

typedef float sw_matrix_t[4*4];

typedef struct sw_vertex {
    float position[4];          // Clip space (x,y,z,w) -> NDC (after /w) -> screen space (x,y,z,1/w)
    float color[4];             // Color value (RGBA)
    float texcoord[2];          // Texture coordinates
} sw_vertex_t;

typedef struct {
    void *pixels;                       // Texture pixels
    sw_pixel_read_color8_f readColor8;  // Texel read RGBA8
    sw_pixel_read_color_f readColor;    // Texel read RGBA32F
    sw_pixelformat_t format;            // Texture format
    sw_pixel_alpha_t alpha;             // Texture alpha mode
    int width, height;                  // Dimensions of the texture
    int wMinus1, hMinus1;               // Dimensions minus one
    int allocSz;                        // Allocated size
    SWfilter minFilter;                 // Minification filter
    SWfilter magFilter;                 // Magnification filter
    SWwrap sWrap;                       // Wrap mode for texcoord.x
    SWwrap tWrap;                       // Wrap mode for texcoord.y
    float tx;                           // Texel width
    float ty;                           // Texel height
} sw_texture_t;

typedef struct {
    sw_texture_t color;         // Default framebuffer color texture
    sw_texture_t depth;         // Default framebuffer depth texture
} sw_default_framebuffer_t;

typedef struct {
    sw_handle_t colorAttachment; // Framebuffer color attachment id
    sw_handle_t depthAttachment; // Framebuffer depth attachment id
} sw_framebuffer_t;

typedef struct {
    void *data;             // Flat storage [capacity*stride] bytes
    uint8_t *gen;           // Generation per slot [capacity]
    uint32_t *freeList;     // Free indices stack [capacity]
    int freeCount;
    int watermark;          // Next blank index (starts at 1, skips 0)
    int capacity;
    size_t stride;
} sw_pool_t;

// Graphic context data structure
typedef struct {
    sw_default_framebuffer_t framebuffer;   // Default framebuffer
    float clearColor[4];                    // Clear color of the framebuffer
    float clearDepth;                       // Clear depth of the framebuffer

    float vpCenter[2];                      // Viewport center
    float vpHalf[2];                        // Viewport half dimensions
    int vpSize[2];                          // Viewport dimensions

    int scMin[2];                           // Scissor rectangle minimum renderable point (top-left)
    int scMax[2];                           // Scissor rectangle maximum renderable point (bottom-right)
    float scClipMin[2];                     // Scissor rectangle minimum renderable point in clip space
    float scClipMax[2];                     // Scissor rectangle maximum renderable point in clip space

    struct {
        sw_vertex_t buffer[SW_MAX_CLIPPED_POLYGON_VERTICES];    // Buffer used for storing primitive vertices, used for processing and rendering
        int vertexCount;                                        // Number of vertices in the primtive buffer
        float color[4];                                         // Current color for the next pushed vertex
        float texcoord[2];                                      // Current texture coordinates for the next push vertex
        bool hasColorAlpha;                                     // Flag indicating whether the current primitive contains transparency
    } primitive;

    struct {
        float *positions;
        float *texcoords;
        uint8_t *colors;
    } array;

    SWdraw drawMode;                                            // Current primitive mode (e.g., lines, triangles)
    SWpoly polyMode;                                            // Current polygon filling mode (e.g., lines, triangles)
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

    sw_handle_t boundFramebufferId;                             // Framebuffer currently bound
    sw_texture_t *colorBuffer;                                  // Color buffer currently bound
    sw_texture_t *depthBuffer;                                  // Depth buffer currently bound
    sw_pool_t framebufferPool;                                  // Framebuffer object pool

    sw_texture_t *boundTexture;                                 // Texture currently bound
    sw_pool_t texturePool;                                      // Texture object pool

    SWfactor srcFactor;                                         // Source blending factor
    SWfactor dstFactor;                                         // Destination bleending factor
    uint32_t blendFlags;                                        // Flags about the current blend mode
    sw_blend_f blendFunc;                                       // Source blend function

    SWface cullFace;                                            // Faces to cull
    SWerrcode errCode;                                          // Last error code

    uint32_t userState;                                         // User-defined pipeline state
    uint32_t rasterState;                                       // Cleaned pipeline state for the rasterizer
} sw_context_t;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static sw_context_t RLSW = { 0 };

#if SW_USE_COLOR_LUT
static float SW_LUT_UINT8_TO_FLOAT[256] = { 0 };
#endif

//----------------------------------------------------------------------------------
// Internal Constants Definition
//----------------------------------------------------------------------------------
// Pixel formats that has an alpha channel
static const sw_pixel_alpha_t SW_PIXELFORMAT_ALPHA[SW_PIXELFORMAT_COUNT] =
{
    [SW_PIXELFORMAT_COLOR_GRAYSCALE]    = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_GRAYALPHA]    = SW_PIXEL_ALPHA_YES,
    [SW_PIXELFORMAT_COLOR_R3G3B2]       = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R5G6B5]       = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R8G8B8]       = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R5G5B5A1]     = SW_PIXEL_ALPHA_BIN,
    [SW_PIXELFORMAT_COLOR_R4G4B4A4]     = SW_PIXEL_ALPHA_YES,
    [SW_PIXELFORMAT_COLOR_R8G8B8A8]     = SW_PIXEL_ALPHA_YES,
    [SW_PIXELFORMAT_COLOR_R32]          = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R32G32B32]    = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R32G32B32A32] = SW_PIXEL_ALPHA_YES,
    [SW_PIXELFORMAT_COLOR_R16]          = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R16G16B16]    = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_COLOR_R16G16B16A16] = SW_PIXEL_ALPHA_YES,
    [SW_PIXELFORMAT_DEPTH_D8]           = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_DEPTH_D16]          = SW_PIXEL_ALPHA_NONE,
    [SW_PIXELFORMAT_DEPTH_D32]          = SW_PIXEL_ALPHA_NONE,
};

// Pixel formats sizes in bytes
static const int SW_PIXELFORMAT_SIZE[SW_PIXELFORMAT_COUNT] =
{
    [SW_PIXELFORMAT_COLOR_GRAYSCALE]    = 1,
    [SW_PIXELFORMAT_COLOR_GRAYALPHA]    = 2,
    [SW_PIXELFORMAT_COLOR_R3G3B2]       = 1,
    [SW_PIXELFORMAT_COLOR_R5G6B5]       = 2,
    [SW_PIXELFORMAT_COLOR_R8G8B8]       = 3,
    [SW_PIXELFORMAT_COLOR_R5G5B5A1]     = 2,
    [SW_PIXELFORMAT_COLOR_R4G4B4A4]     = 2,
    [SW_PIXELFORMAT_COLOR_R8G8B8A8]     = 4,
    [SW_PIXELFORMAT_COLOR_R32]          = 4,
    [SW_PIXELFORMAT_COLOR_R32G32B32]    = 12,
    [SW_PIXELFORMAT_COLOR_R32G32B32A32] = 16,
    [SW_PIXELFORMAT_COLOR_R16]          = 2,
    [SW_PIXELFORMAT_COLOR_R16G16B16]    = 6,
    [SW_PIXELFORMAT_COLOR_R16G16B16A16] = 8,
    [SW_PIXELFORMAT_DEPTH_D8]           = 1,
    [SW_PIXELFORMAT_DEPTH_D16]          = 2,
    [SW_PIXELFORMAT_DEPTH_D32]          = 4,
};

static const int SW_PRIMITIVE_VERTEX_COUNT[] =
{
    // Remember that this is acceptable; these are small indices
    [SW_POINTS]     = 1,
    [SW_LINES]      = 2,
    [SW_TRIANGLES]  = 3,
    [SW_QUADS]      = 4,
};

//----------------------------------------------------------------------------------
// Internal Functions Definitions
//----------------------------------------------------------------------------------

// Math helper functions
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

static inline int sw_clamp_int(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static inline int sw_clamp(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
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

static inline uint8_t sw_luminance8(const uint8_t *color)
{
    return (uint8_t)((color[0]*77 + color[1]*150 + color[2]*29) >> 8);
}

static inline float sw_luminance(const float *color)
{
    return color[0]*0.299f + color[1]*0.587f + color[2]*0.114f;
}

static inline void sw_lerp_vertex_PCT(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT a, const sw_vertex_t *SW_RESTRICT b, float t)
{
    const float tInv = 1.0f - t;

    // Position interpolation (4 components)
    out->position[0] = a->position[0]*tInv + b->position[0]*t;
    out->position[1] = a->position[1]*tInv + b->position[1]*t;
    out->position[2] = a->position[2]*tInv + b->position[2]*t;
    out->position[3] = a->position[3]*tInv + b->position[3]*t;

    // Color interpolation (4 components)
    out->color[0] = a->color[0]*tInv + b->color[0]*t;
    out->color[1] = a->color[1]*tInv + b->color[1]*t;
    out->color[2] = a->color[2]*tInv + b->color[2]*t;
    out->color[3] = a->color[3]*tInv + b->color[3]*t;

    // Texture coordinate interpolation (2 components)
    out->texcoord[0] = a->texcoord[0]*tInv + b->texcoord[0]*t;
    out->texcoord[1] = a->texcoord[1]*tInv + b->texcoord[1]*t;
}

static inline void sw_get_vertex_grad_PCT(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT a, const sw_vertex_t *SW_RESTRICT b, float scale)
{
    // Calculate gradients for Position
    out->position[0] = (b->position[0] - a->position[0])*scale;
    out->position[1] = (b->position[1] - a->position[1])*scale;
    out->position[2] = (b->position[2] - a->position[2])*scale;
    out->position[3] = (b->position[3] - a->position[3])*scale;

    // Calculate gradients for Color
    out->color[0] = (b->color[0] - a->color[0])*scale;
    out->color[1] = (b->color[1] - a->color[1])*scale;
    out->color[2] = (b->color[2] - a->color[2])*scale;
    out->color[3] = (b->color[3] - a->color[3])*scale;

    // Calculate gradients for Texture coordinates
    out->texcoord[0] = (b->texcoord[0] - a->texcoord[0])*scale;
    out->texcoord[1] = (b->texcoord[1] - a->texcoord[1])*scale;
}

static inline void sw_add_vertex_grad_PCT(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT gradients)
{
    // Add gradients to Position
    out->position[0] += gradients->position[0];
    out->position[1] += gradients->position[1];
    out->position[2] += gradients->position[2];
    out->position[3] += gradients->position[3];

    // Add gradients to Color
    out->color[0] += gradients->color[0];
    out->color[1] += gradients->color[1];
    out->color[2] += gradients->color[2];
    out->color[3] += gradients->color[3];

    // Add gradients to Texture coordinates
    out->texcoord[0] += gradients->texcoord[0];
    out->texcoord[1] += gradients->texcoord[1];
}

static inline void sw_add_vertex_grad_scaled_PCT(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT gradients, float scale)
{
    // Add gradients to Position
    out->position[0] += gradients->position[0]*scale;
    out->position[1] += gradients->position[1]*scale;
    out->position[2] += gradients->position[2]*scale;
    out->position[3] += gradients->position[3]*scale;

    // Add gradients to Color
    out->color[0] += gradients->color[0]*scale;
    out->color[1] += gradients->color[1]*scale;
    out->color[2] += gradients->color[2]*scale;
    out->color[3] += gradients->color[3]*scale;

    // Add gradients to Texture coordinates
    out->texcoord[0] += gradients->texcoord[0]*scale;
    out->texcoord[1] += gradients->texcoord[1]*scale;
}

static inline void sw_get_vertex_grad_PC(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT a, const sw_vertex_t *SW_RESTRICT b, float scale)
{
    // Calculate gradients for Position
    out->position[0] = (b->position[0] - a->position[0])*scale;
    out->position[1] = (b->position[1] - a->position[1])*scale;
    out->position[2] = (b->position[2] - a->position[2])*scale;
    out->position[3] = (b->position[3] - a->position[3])*scale;

    // Calculate gradients for Color
    out->color[0] = (b->color[0] - a->color[0])*scale;
    out->color[1] = (b->color[1] - a->color[1])*scale;
    out->color[2] = (b->color[2] - a->color[2])*scale;
    out->color[3] = (b->color[3] - a->color[3])*scale;
}

static inline void sw_add_vertex_grad_PC(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT gradients)
{
    // Add gradients to Position
    out->position[0] += gradients->position[0];
    out->position[1] += gradients->position[1];
    out->position[2] += gradients->position[2];
    out->position[3] += gradients->position[3];

    // Add gradients to Color
    out->color[0] += gradients->color[0];
    out->color[1] += gradients->color[1];
    out->color[2] += gradients->color[2];
    out->color[3] += gradients->color[3];
}

static inline void sw_add_vertex_grad_scaled_PC(sw_vertex_t *SW_RESTRICT out, const sw_vertex_t *SW_RESTRICT gradients, float scale)
{
    // Add gradients to Position
    out->position[0] += gradients->position[0]*scale;
    out->position[1] += gradients->position[1]*scale;
    out->position[2] += gradients->position[2]*scale;
    out->position[3] += gradients->position[3]*scale;

    // Add gradients to Color
    out->color[0] += gradients->color[0]*scale;
    out->color[1] += gradients->color[1]*scale;
    out->color[2] += gradients->color[2]*scale;
    out->color[3] += gradients->color[3]*scale;
}

// Half conversion functions
static inline uint16_t sw_float_to_half_ui(uint32_t ui)
{
    int32_t s = (ui >> 16) & 0x8000;
    int32_t em = ui & 0x7fffffff;

    // Bias exponent and round to nearest; 112 is relative exponent bias (127-15)
    int32_t h = (em - (112 << 23) + (1 << 12)) >> 13;

    // Underflow: flush to zero; 113 encodes exponent -14
    h = (em < (113 << 23))? 0 : h;

    // Overflow: infinity; 143 encodes exponent 16
    h = (em >= (143 << 23))? 0x7c00 : h;

    // NaN; note that all types of NaN are converted to qNaN
    h = (em > (255 << 23))? 0x7e00 : h;

    return (uint16_t)(s | h);
}

static inline uint32_t sw_half_to_float_ui(uint16_t h)
{
    uint32_t s = (unsigned)(h & 0x8000) << 16;
    int32_t em = h & 0x7fff;

    // Bias exponent and pad mantissa with 0; 112 is relative exponent bias (127-15)
    int32_t r = (em + (112 << 10)) << 13;

    // Denormal: flush to zero
    r = (em < (1 << 10))? 0 : r;

    // Infinity/NaN; note that NaN payload is preeserved as a byproduct of unifying inf/nan cases
    // 112 is an exponent bias fixup; since it was already applied once, applying it twice converts 31 to 255
    r += (em >= (31 << 10))? (112 << 23) : 0;

    return s | r;
}

static inline sw_half_t sw_float_to_half(float i)
{
    union { float f; uint32_t i; } v;
    v.f = i;
    return sw_float_to_half_ui(v.i);
}

static inline float sw_half_to_float(sw_half_t y)
{
    union { float f; uint32_t i; } v;
    v.i = sw_half_to_float_ui(y);
    return v.f;
}

static inline uint8_t sw_expand_1to8(uint32_t v) { return v? 255 : 0; }
static inline uint8_t sw_expand_2to8(uint32_t v) { return (uint8_t)(v*85); }
static inline uint8_t sw_expand_3to8(uint32_t v) { return (uint8_t)((v << 5) | (v << 2) | (v >> 1)); }
static inline uint8_t sw_expand_4to8(uint32_t v) { return (uint8_t)((v << 4) | v); }
static inline uint8_t sw_expand_5to8(uint32_t v) { return (uint8_t)((v << 3) | (v >> 2)); }
static inline uint8_t sw_expand_6to8(uint32_t v) { return (uint8_t)((v << 2) | (v >> 4)); }

static inline uint32_t sw_compress_8to1(uint8_t v) { return v >> 7; }
static inline uint32_t sw_compress_8to2(uint8_t v) { return v >> 6; }
static inline uint32_t sw_compress_8to3(uint8_t v) { return v >> 5; }
static inline uint32_t sw_compress_8to4(uint8_t v) { return v >> 4; }
static inline uint32_t sw_compress_8to5(uint8_t v) { return v >> 3; }
static inline uint32_t sw_compress_8to6(uint8_t v) { return v >> 2; }

static inline void sw_color8_to_color(float *SW_RESTRICT dst, const uint8_t *SW_RESTRICT src)
{
#if defined(SW_HAS_NEON)
    uint8x8_t bytes = vreinterpret_u8_u32(vld1_dup_u32((const uint32_t *)src));
    uint16x8_t words = vmovl_u8(bytes);
    uint32x4_t dwords = vmovl_u16(vget_low_u16(words));
    float32x4_t fvals = vmulq_f32(vcvtq_f32_u32(dwords), vdupq_n_f32(SW_INV_255));
    vst1q_f32(dst, fvals);

#elif defined(SW_HAS_SSE41)
    __m128i bytes = _mm_loadu_si32(src);
    __m128 fvals = _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvtepu8_epi32(bytes)), _mm_set1_ps(SW_INV_255));
    _mm_storeu_ps(dst, fvals);

#elif defined(SW_HAS_SSE2)
    __m128i zero = _mm_setzero_si128();
    __m128i bytes = _mm_loadu_si32(src);
    __m128i words = _mm_unpacklo_epi8(bytes, zero);
    __m128i dwords = _mm_unpacklo_epi16(words, zero);
    __m128 fvals = _mm_mul_ps(_mm_cvtepi32_ps(dwords), _mm_set1_ps(SW_INV_255));
    _mm_storeu_ps(dst, fvals);

#elif defined(SW_HAS_RVV)
    // TODO: WARNING: Sample code generated by AI, needs testing and review
    size_t vl = __riscv_vsetvl_e8m1(4); // Set vector length for 8-bit input elements
    vuint8m1_t vsrc_u8 = __riscv_vle8_v_u8m1(src, vl); // Load 4 unsigned 8-bit integers
    vuint32m1_t vsrc_u32 = __riscv_vwcvt_xu_u_v_u32m1(vsrc_u8, vl); // Widen to 32-bit unsigned integers
    vfloat32m1_t vsrc_f32 = __riscv_vfcvt_f_xu_v_f32m1(vsrc_u32, vl); // Convert to float32
    vfloat32m1_t vnorm = __riscv_vfmul_vf_f32m1(vsrc_f32, SW_INV_255, vl); // Multiply by 1/255.0 to normalize
    __riscv_vse32_v_f32m1(dst, vnorm, vl); // Store result

#elif SW_USE_COLOR_LUT
    dst[0] = SW_LUT_UINT8_TO_FLOAT[src[0]];
    dst[1] = SW_LUT_UINT8_TO_FLOAT[src[1]];
    dst[2] = SW_LUT_UINT8_TO_FLOAT[src[2]];
    dst[3] = SW_LUT_UINT8_TO_FLOAT[src[3]];

#else
    dst[0] = (float)src[0]*SW_INV_255;
    dst[1] = (float)src[1]*SW_INV_255;
    dst[2] = (float)src[2]*SW_INV_255;
    dst[3] = (float)src[3]*SW_INV_255;
#endif
}

static inline void sw_color_to_color8(uint8_t *SW_RESTRICT dst, const float *SW_RESTRICT src)
{
#if defined(SW_HAS_NEON)
    float32x4_t fvals = vmulq_f32(vld1q_f32(src), vdupq_n_f32(255.0f));
    uint32x4_t i32 = vcvtq_u32_f32(fvals);
    uint16x4_t i16 = vmovn_u32(i32);
    uint8x8_t i8 = vmovn_u16(vcombine_u16(i16, i16));
    vst1_lane_u32((uint32_t *)dst, vreinterpret_u32_u8(i8), 0);

#elif defined(SW_HAS_SSE41)
    __m128 fvals = _mm_mul_ps(_mm_loadu_ps(src), _mm_set1_ps(255.0f));
    __m128i i32 = _mm_cvttps_epi32(fvals);
    __m128i i16 = _mm_packus_epi32(i32, i32);
    __m128i i8 = _mm_packus_epi16(i16, i16);
    _mm_storeu_si32(dst, i8);

#elif defined(SW_HAS_SSE2)
    __m128 fvals = _mm_mul_ps(_mm_loadu_ps(src), _mm_set1_ps(255.0f));
    __m128i i32 = _mm_cvttps_epi32(fvals);
    __m128i i16 = _mm_packs_epi32(i32, i32);
    __m128i i8 = _mm_packus_epi16(i16, i16);
    _mm_storeu_si32(dst, i8);

#elif defined(SW_HAS_RVV)
    // TODO: WARNING: Sample code generated by AI, needs testing and review
    // REVIEW: It shouldn't perform so many operations; take inspiration from other versions
    // NOTE: RVV 1.0 specs define the use of __riscv_ prefix for instrinsic functions
    size_t vl = __riscv_vsetvl_e32m1(4); // Load up to 4 floats into a vector register
    vfloat32m1_t vsrc = __riscv_vle32_v_f32m1(src, vl); // Load float32 values

    // Multiply by 255.0f and add 0.5f for rounding
    vfloat32m1_t vscaled = __riscv_vfmul_vf_f32m1(vsrc, 255.0f, vl);
    vscaled = __riscv_vfadd_vf_f32m1(vscaled, 0.5f, vl);

    // Convert to unsigned integer (truncate toward zero)
    vuint32m1_t vu32 = __riscv_vfcvt_xu_f_v_u32m1(vscaled, vl);

    // Narrow from u32 -> u8
    vuint8m1_t vu8 = __riscv_vnclipu_wx_u8m1(vu32, 0, vl); // Round toward zero
    __riscv_vse8_v_u8m1(dst, vu8, vl); // Store result

#else
    dst[0] = (uint8_t)(src[0]*255.0f);
    dst[1] = (uint8_t)(src[1]*255.0f);
    dst[2] = (uint8_t)(src[2]*255.0f);
    dst[3] = (uint8_t)(src[3]*255.0f);
#endif
}
//-------------------------------------------------------------------------------------------

// Object pool functions
//-------------------------------------------------------------------------------------------
static bool sw_pool_init(sw_pool_t *pool, int capacity, size_t stride)
{
    *pool = (sw_pool_t) { 0 };

    pool->data = SW_CALLOC(capacity, stride);
    pool->gen = SW_CALLOC(capacity, sizeof(uint8_t));
    pool->freeList = SW_MALLOC(capacity*sizeof(uint32_t));

    if (!pool->data || !pool->gen || !pool->freeList)
    {
        SW_FREE(pool->data);
        SW_FREE(pool->gen);
        SW_FREE(pool->freeList);
        return false;
    }

    pool->watermark = 1;
    pool->capacity = capacity;
    pool->stride = stride;

    return true;
}

static void sw_pool_destroy(sw_pool_t *pool)
{
    SW_FREE(pool->data);
    SW_FREE(pool->gen);
    SW_FREE(pool->freeList);
    *pool = (sw_pool_t) { 0 };
}

static sw_handle_t sw_pool_alloc(sw_pool_t *pool)
{
    uint32_t index;

    if (pool->freeCount > 0)
    {
        index = pool->freeList[--pool->freeCount];
    }
    else
    {
        if (pool->watermark >= pool->capacity) return SW_HANDLE_NULL;
        index = (uint32_t)pool->watermark++;
    }

    uint8_t ver = ((pool->gen[index] & SW_POOL_SLOT_VER_MASK) + 1) & SW_POOL_SLOT_VER_MASK;
    if (ver == 0) ver = 1;
    pool->gen[index] = SW_POOL_SLOT_LIVE | ver;

    uint8_t *slot = (uint8_t *)pool->data + index*pool->stride;
    for (size_t i = 0; i < pool->stride; i++) slot[i] = 0;

    return (sw_handle_t)index;
}

static void *sw_pool_get(const sw_pool_t *pool, sw_handle_t handle)
{
    if (handle == SW_HANDLE_NULL) return NULL;
    if ((int)handle >= pool->capacity) return NULL;
    if (!(pool->gen[handle] & SW_POOL_SLOT_LIVE)) return NULL;

    return (char *)pool->data + handle*pool->stride;
}

static bool sw_pool_valid(const sw_pool_t *pool, sw_handle_t handle)
{
    return sw_pool_get(pool, handle) != NULL;
}

static bool sw_pool_free(sw_pool_t *pool, sw_handle_t handle)
{
    if (!sw_pool_valid(pool, handle)) return false;

    pool->gen[handle] &= SW_POOL_SLOT_VER_MASK;   // delete live flag
    pool->freeList[pool->freeCount++] = handle;
    return true;
}
//-------------------------------------------------------------------------------------------

// Validity check helper functions
//-------------------------------------------------------------------------------------------
static inline bool sw_is_texture_valid(sw_handle_t id)
{
    return sw_pool_valid(&RLSW.texturePool, id);
}

static inline bool sw_is_texture_complete(sw_texture_t *tex)
{
    return (tex != NULL) && (tex->pixels != NULL);
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

static inline bool sw_is_ready_to_render(void)
{
    return (swCheckFramebufferStatus() == SW_FRAMEBUFFER_COMPLETE);
}
//-------------------------------------------------------------------------------------------

// Pixel management functions
//-------------------------------------------------------------------------------------------
static inline int sw_pixel_get_format(SWformat format, SWtype type)
{
    int channels = 0;
    int bitsPerChannel = 8; // Default: 8 bits per channel

    // Handle case where format is a depth component
    if (format == SW_DEPTH_COMPONENT)
    {
        switch (type)
        {
            case SW_UNSIGNED_BYTE:  return SW_PIXELFORMAT_DEPTH_D8;
            case SW_BYTE:           return SW_PIXELFORMAT_DEPTH_D8;
            case SW_UNSIGNED_SHORT: return SW_PIXELFORMAT_DEPTH_D16;
            case SW_SHORT:          return SW_PIXELFORMAT_DEPTH_D16;
            case SW_UNSIGNED_INT:   return SW_PIXELFORMAT_DEPTH_D32;
            case SW_INT:            return SW_PIXELFORMAT_DEPTH_D32;
            case SW_FLOAT:          return SW_PIXELFORMAT_DEPTH_D32;
            default: return SW_PIXELFORMAT_UNKNOWN;
        }
    }

    // Handle case where the type is a packed color
    switch (type)
    {
        case SW_UNSIGNED_BYTE_3_3_2:    return SW_PIXELFORMAT_COLOR_R3G3B2;
        case SW_UNSIGNED_SHORT_5_6_5:   return SW_PIXELFORMAT_COLOR_R5G6B5;
        case SW_UNSIGNED_SHORT_4_4_4_4: return SW_PIXELFORMAT_COLOR_R4G4B4A4;
        case SW_UNSIGNED_SHORT_5_5_5_1: return SW_PIXELFORMAT_COLOR_R5G5B5A1;
        default: break;
    }

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
        if (channels == 1) return SW_PIXELFORMAT_COLOR_GRAYSCALE;
        if (channels == 2) return SW_PIXELFORMAT_COLOR_GRAYALPHA;
        if (channels == 3) return SW_PIXELFORMAT_COLOR_R8G8B8;
        if (channels == 4) return SW_PIXELFORMAT_COLOR_R8G8B8A8;
    }
    else if (bitsPerChannel == 16)
    {
        if (channels == 1) return SW_PIXELFORMAT_COLOR_R16;
        if (channels == 3) return SW_PIXELFORMAT_COLOR_R16G16B16;
        if (channels == 4) return SW_PIXELFORMAT_COLOR_R16G16B16A16;
    }
    else if (bitsPerChannel == 32)
    {
        if (channels == 1) return SW_PIXELFORMAT_COLOR_R32;
        if (channels == 3) return SW_PIXELFORMAT_COLOR_R32G32B32;
        if (channels == 4) return SW_PIXELFORMAT_COLOR_R32G32B32A32;
    }

    return SW_PIXELFORMAT_UNKNOWN;
}

static inline bool sw_pixel_is_depth_format(sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_DEPTH_D8:
        case SW_PIXELFORMAT_DEPTH_D16:
        case SW_PIXELFORMAT_DEPTH_D32: return true;
        default: break;
    }

    return false;
}

static inline void sw_pixel_read_color8_GRAYSCALE(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t gray = ((const uint8_t *)pixels)[offset];
    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = 255;
}

static inline void sw_pixel_read_color8_GRAYALPHA(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint8_t *src = &((const uint8_t *)pixels)[offset*2];
    color[0] = src[0];
    color[1] = src[0];
    color[2] = src[0];
    color[3] = src[1];
}

static inline void sw_pixel_read_color8_R3G3B2(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t pixel = ((const uint8_t *)pixels)[offset];
    color[0] = sw_expand_3to8((pixel >> 5) & 0x07);
    color[1] = sw_expand_3to8((pixel >> 2) & 0x07);
    color[2] = sw_expand_2to8( pixel       & 0x03);
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R5G6B5(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint16_t pixel = ((const uint16_t *)pixels)[offset];
    color[0] = sw_expand_5to8((pixel >> 11) & 0x1F);
    color[1] = sw_expand_6to8((pixel >>  5) & 0x3F);
    color[2] = sw_expand_5to8( pixel        & 0x1F);
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R8G8B8(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint8_t *src = &((const uint8_t *)pixels)[offset*3];
    color[0] = src[0];
    color[1] = src[1];
    color[2] = src[2];
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R5G5B5A1(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint16_t pixel = ((const uint16_t *)pixels)[offset];
    color[0] = sw_expand_5to8((pixel >> 11) & 0x1F);
    color[1] = sw_expand_5to8((pixel >>  6) & 0x1F);
    color[2] = sw_expand_5to8((pixel >>  1) & 0x1F);
    color[3] = sw_expand_1to8( pixel        & 0x01);
}

static inline void sw_pixel_read_color8_R4G4B4A4(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint16_t pixel = ((const uint16_t *)pixels)[offset];
    color[0] = sw_expand_4to8((pixel >> 12) & 0x0F);
    color[1] = sw_expand_4to8((pixel >>  8) & 0x0F);
    color[2] = sw_expand_4to8((pixel >>  4) & 0x0F);
    color[3] = sw_expand_4to8( pixel        & 0x0F);
}

static inline void sw_pixel_read_color8_R8G8B8A8(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint8_t *src = &((const uint8_t *)pixels)[offset*4];
    color[0] = src[0];
    color[1] = src[1];
    color[2] = src[2];
    color[3] = src[3];
}

static inline void sw_pixel_read_color8_R32(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t gray = (uint8_t)(((const float *)pixels)[offset]*255.0f);
    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R32G32B32(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const float *src = &((const float *)pixels)[offset*3];
    color[0] = (uint8_t)(src[0]*255.0f);
    color[1] = (uint8_t)(src[1]*255.0f);
    color[2] = (uint8_t)(src[2]*255.0f);
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R32G32B32A32(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const float *src = &((const float *)pixels)[offset*4];
    color[0] = (uint8_t)(src[0]*255.0f);
    color[1] = (uint8_t)(src[1]*255.0f);
    color[2] = (uint8_t)(src[2]*255.0f);
    color[3] = (uint8_t)(src[3]*255.0f);
}

static inline void sw_pixel_read_color8_R16(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t gray = (uint8_t)(sw_half_to_float(((const uint16_t *)pixels)[offset])*255.0f);
    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R16G16B16(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint16_t *src = &((const uint16_t *)pixels)[offset*3];
    color[0] = (uint8_t)(sw_half_to_float(src[0])*255.0f);
    color[1] = (uint8_t)(sw_half_to_float(src[1])*255.0f);
    color[2] = (uint8_t)(sw_half_to_float(src[2])*255.0f);
    color[3] = 255;
}

static inline void sw_pixel_read_color8_R16G16B16A16(uint8_t *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint16_t *src = &((const uint16_t *)pixels)[offset*4];
    color[0] = (uint8_t)(sw_half_to_float(src[0])*255.0f);
    color[1] = (uint8_t)(sw_half_to_float(src[1])*255.0f);
    color[2] = (uint8_t)(sw_half_to_float(src[2])*255.0f);
    color[3] = (uint8_t)(sw_half_to_float(src[3])*255.0f);
}

static inline sw_pixel_read_color8_f sw_pixel_get_read_color8_func(sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_COLOR_GRAYSCALE: return sw_pixel_read_color8_GRAYSCALE;
        case SW_PIXELFORMAT_COLOR_GRAYALPHA: return sw_pixel_read_color8_GRAYALPHA;
        case SW_PIXELFORMAT_COLOR_R3G3B2: return sw_pixel_read_color8_R3G3B2;
        case SW_PIXELFORMAT_COLOR_R5G6B5: return sw_pixel_read_color8_R5G6B5;
        case SW_PIXELFORMAT_COLOR_R8G8B8: return sw_pixel_read_color8_R8G8B8;
        case SW_PIXELFORMAT_COLOR_R5G5B5A1: return sw_pixel_read_color8_R5G5B5A1;
        case SW_PIXELFORMAT_COLOR_R4G4B4A4: return sw_pixel_read_color8_R4G4B4A4;
        case SW_PIXELFORMAT_COLOR_R8G8B8A8: return sw_pixel_read_color8_R8G8B8A8;
        case SW_PIXELFORMAT_COLOR_R32: return sw_pixel_read_color8_R32;
        case SW_PIXELFORMAT_COLOR_R32G32B32: return sw_pixel_read_color8_R32G32B32;
        case SW_PIXELFORMAT_COLOR_R32G32B32A32: return sw_pixel_read_color8_R32G32B32A32;
        case SW_PIXELFORMAT_COLOR_R16: return sw_pixel_read_color8_R16;
        case SW_PIXELFORMAT_COLOR_R16G16B16: return sw_pixel_read_color8_R16G16B16;
        case SW_PIXELFORMAT_COLOR_R16G16B16A16: return sw_pixel_read_color8_R16G16B16A16;

        case SW_PIXELFORMAT_UNKNOWN:
        case SW_PIXELFORMAT_DEPTH_D8:
        case SW_PIXELFORMAT_DEPTH_D16:
        case SW_PIXELFORMAT_DEPTH_D32:
        case SW_PIXELFORMAT_COUNT:
        default: break;
    }

    return NULL;
}

static inline void sw_pixel_write_color8_GRAYSCALE(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    ((uint8_t *)pixels)[offset] = sw_luminance8(color);
}

static inline void sw_pixel_write_color8_GRAYALPHA(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint8_t *dst = &((uint8_t *)pixels)[offset*2];
    dst[0] = sw_luminance8(color);
    dst[1] = color[3];
}

static inline void sw_pixel_write_color8_R3G3B2(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint8_t pixel = (sw_compress_8to3(color[0]) << 5)
                  | (sw_compress_8to3(color[1]) << 2)
                  |  sw_compress_8to2(color[2]);
    ((uint8_t *)pixels)[offset] = pixel;
}

static inline void sw_pixel_write_color8_R5G6B5(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint16_t pixel = (sw_compress_8to5(color[0]) << 11)
                   | (sw_compress_8to6(color[1]) <<  5)
                   |  sw_compress_8to5(color[2]);
    ((uint16_t *)pixels)[offset] = pixel;
}

static inline void sw_pixel_write_color8_R8G8B8(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint8_t *dst = &((uint8_t *)pixels)[offset*3];
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
}

static inline void sw_pixel_write_color8_R5G5B5A1(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint16_t pixel = (sw_compress_8to5(color[0]) << 11)
                   | (sw_compress_8to5(color[1]) <<  6)
                   | (sw_compress_8to5(color[2]) <<  1)
                   |  sw_compress_8to1(color[3]);
    ((uint16_t *)pixels)[offset] = pixel;
}

static inline void sw_pixel_write_color8_R4G4B4A4(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint16_t pixel = (sw_compress_8to4(color[0]) << 12)
                   | (sw_compress_8to4(color[1]) <<  8)
                   | (sw_compress_8to4(color[2]) <<  4)
                   |  sw_compress_8to4(color[3]);
    ((uint16_t *)pixels)[offset] = pixel;
}

static inline void sw_pixel_write_color8_R8G8B8A8(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint8_t *dst = &((uint8_t *)pixels)[offset*4];
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
    dst[3] = color[3];
}

static inline void sw_pixel_write_color8_R32(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    ((float *)pixels)[offset] = sw_luminance8(color)*SW_INV_255;
}

static inline void sw_pixel_write_color8_R32G32B32(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    float *dst = &((float *)pixels)[offset*3];
    dst[0] = color[0]*SW_INV_255;
    dst[1] = color[1]*SW_INV_255;
    dst[2] = color[2]*SW_INV_255;
}

static inline void sw_pixel_write_color8_R32G32B32A32(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    float *dst = &((float *)pixels)[offset*4];
    dst[0] = color[0]*SW_INV_255;
    dst[1] = color[1]*SW_INV_255;
    dst[2] = color[2]*SW_INV_255;
    dst[3] = color[3]*SW_INV_255;
}

static inline void sw_pixel_write_color8_R16(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    ((uint16_t *)pixels)[offset] = sw_float_to_half(sw_luminance8(color)*SW_INV_255);
}

static inline void sw_pixel_write_color8_R16G16B16(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint16_t *dst = &((uint16_t *)pixels)[offset*3];
    dst[0] = sw_float_to_half(color[0]*SW_INV_255);
    dst[1] = sw_float_to_half(color[1]*SW_INV_255);
    dst[2] = sw_float_to_half(color[2]*SW_INV_255);
}

static inline void sw_pixel_write_color8_R16G16B16A16(void *SW_RESTRICT pixels, const uint8_t *SW_RESTRICT color, uint32_t offset)
{
    uint16_t *dst = &((uint16_t *)pixels)[offset*4];
    dst[0] = sw_float_to_half(color[0]*SW_INV_255);
    dst[1] = sw_float_to_half(color[1]*SW_INV_255);
    dst[2] = sw_float_to_half(color[2]*SW_INV_255);
    dst[3] = sw_float_to_half(color[3]*SW_INV_255);
}

static inline sw_pixel_write_color8_f sw_pixel_get_write_color8_func(sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_COLOR_GRAYSCALE: return sw_pixel_write_color8_GRAYSCALE;
        case SW_PIXELFORMAT_COLOR_GRAYALPHA: return sw_pixel_write_color8_GRAYALPHA;
        case SW_PIXELFORMAT_COLOR_R3G3B2: return sw_pixel_write_color8_R3G3B2;
        case SW_PIXELFORMAT_COLOR_R5G6B5: return sw_pixel_write_color8_R5G6B5;
        case SW_PIXELFORMAT_COLOR_R8G8B8: return sw_pixel_write_color8_R8G8B8;
        case SW_PIXELFORMAT_COLOR_R5G5B5A1: return sw_pixel_write_color8_R5G5B5A1;
        case SW_PIXELFORMAT_COLOR_R4G4B4A4: return sw_pixel_write_color8_R4G4B4A4;
        case SW_PIXELFORMAT_COLOR_R8G8B8A8: return sw_pixel_write_color8_R8G8B8A8;
        case SW_PIXELFORMAT_COLOR_R32: return sw_pixel_write_color8_R32;
        case SW_PIXELFORMAT_COLOR_R32G32B32: return sw_pixel_write_color8_R32G32B32;
        case SW_PIXELFORMAT_COLOR_R32G32B32A32: return sw_pixel_write_color8_R32G32B32A32;
        case SW_PIXELFORMAT_COLOR_R16: return sw_pixel_write_color8_R16;
        case SW_PIXELFORMAT_COLOR_R16G16B16: return sw_pixel_write_color8_R16G16B16;
        case SW_PIXELFORMAT_COLOR_R16G16B16A16: return sw_pixel_write_color8_R16G16B16A16;

        case SW_PIXELFORMAT_UNKNOWN:
        case SW_PIXELFORMAT_DEPTH_D8:
        case SW_PIXELFORMAT_DEPTH_D16:
        case SW_PIXELFORMAT_DEPTH_D32:
        case SW_PIXELFORMAT_COUNT:
        default: break;
    }

    return NULL;
}

static inline void sw_pixel_read_color_GRAYSCALE(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    float gray = ((const uint8_t *)pixels)[offset]*SW_INV_255;
    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = 1.0f;
}

static inline void sw_pixel_read_color_GRAYALPHA(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint8_t *src = &((const uint8_t *)pixels)[offset*2];
    float gray = src[0]*SW_INV_255;
    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = src[1]*SW_INV_255;
}

static inline void sw_pixel_read_color_R3G3B2(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t unpack[4];
    sw_pixel_read_color8_R3G3B2(unpack, pixels, offset);
    sw_color8_to_color(color, unpack);
}

static inline void sw_pixel_read_color_R5G6B5(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t unpack[4];
    sw_pixel_read_color8_R5G6B5(unpack, pixels, offset);
    sw_color8_to_color(color, unpack);
}

static inline void sw_pixel_read_color_R8G8B8(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t unpack[4];
    sw_pixel_read_color8_R8G8B8(unpack, pixels, offset);
    sw_color8_to_color(color, unpack);
}

static inline void sw_pixel_read_color_R5G5B5A1(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t unpack[4];
    sw_pixel_read_color8_R5G5B5A1(unpack, pixels, offset);
    sw_color8_to_color(color, unpack);
}

static inline void sw_pixel_read_color_R4G4B4A4(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    uint8_t unpack[4];
    sw_pixel_read_color8_R4G4B4A4(unpack, pixels, offset);
    sw_color8_to_color(color, unpack);
}

static inline void sw_pixel_read_color_R8G8B8A8(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    sw_color8_to_color(color, &((const uint8_t *)pixels)[offset*4]);
}

static inline void sw_pixel_read_color_R32(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    float val = ((const float *)pixels)[offset];
    color[0] = val;
    color[1] = val;
    color[2] = val;
    color[3] = 1.0f;
}

static inline void sw_pixel_read_color_R32G32B32(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const float *src = &((const float *)pixels)[offset*3];
    color[0] = src[0];
    color[1] = src[1];
    color[2] = src[2];
    color[3] = 1.0f;
}

static inline void sw_pixel_read_color_R32G32B32A32(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const float *src = &((const float *)pixels)[offset*4];
    color[0] = src[0];
    color[1] = src[1];
    color[2] = src[2];
    color[3] = src[3];
}

static inline void sw_pixel_read_color_R16(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    float val = sw_half_to_float(((const uint16_t *)pixels)[offset]);
    color[0] = val;
    color[1] = val;
    color[2] = val;
    color[3] = 1.0f;
}

static inline void sw_pixel_read_color_R16G16B16(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint16_t *src = &((const uint16_t *)pixels)[offset*3];
    color[0] = sw_half_to_float(src[0]);
    color[1] = sw_half_to_float(src[1]);
    color[2] = sw_half_to_float(src[2]);
    color[3] = 1.0f;
}

static inline void sw_pixel_read_color_R16G16B16A16(float *SW_RESTRICT color, const void *SW_RESTRICT pixels, uint32_t offset)
{
    const uint16_t *src = &((const uint16_t *)pixels)[offset*4];
    color[0] = sw_half_to_float(src[0]);
    color[1] = sw_half_to_float(src[1]);
    color[2] = sw_half_to_float(src[2]);
    color[3] = sw_half_to_float(src[3]);
}

static inline sw_pixel_read_color_f sw_pixel_get_read_color_func(sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_COLOR_GRAYSCALE: return sw_pixel_read_color_GRAYSCALE;
        case SW_PIXELFORMAT_COLOR_GRAYALPHA: return sw_pixel_read_color_GRAYALPHA;
        case SW_PIXELFORMAT_COLOR_R3G3B2: return sw_pixel_read_color_R3G3B2;
        case SW_PIXELFORMAT_COLOR_R5G6B5: return sw_pixel_read_color_R5G6B5;
        case SW_PIXELFORMAT_COLOR_R8G8B8: return sw_pixel_read_color_R8G8B8;
        case SW_PIXELFORMAT_COLOR_R5G5B5A1: return sw_pixel_read_color_R5G5B5A1;
        case SW_PIXELFORMAT_COLOR_R4G4B4A4: return sw_pixel_read_color_R4G4B4A4;
        case SW_PIXELFORMAT_COLOR_R8G8B8A8: return sw_pixel_read_color_R8G8B8A8;
        case SW_PIXELFORMAT_COLOR_R32: return sw_pixel_read_color_R32;
        case SW_PIXELFORMAT_COLOR_R32G32B32: return sw_pixel_read_color_R32G32B32;
        case SW_PIXELFORMAT_COLOR_R32G32B32A32: return sw_pixel_read_color_R32G32B32A32;
        case SW_PIXELFORMAT_COLOR_R16: return sw_pixel_read_color_R16;
        case SW_PIXELFORMAT_COLOR_R16G16B16: return sw_pixel_read_color_R16G16B16;
        case SW_PIXELFORMAT_COLOR_R16G16B16A16: return sw_pixel_read_color_R16G16B16A16;

        case SW_PIXELFORMAT_UNKNOWN:
        case SW_PIXELFORMAT_DEPTH_D8:
        case SW_PIXELFORMAT_DEPTH_D16:
        case SW_PIXELFORMAT_DEPTH_D32:
        case SW_PIXELFORMAT_COUNT:
        default: break;
    }

    return NULL;
}

static inline void sw_pixel_write_color_GRAYSCALE(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    ((uint8_t *)pixels)[offset] = (uint8_t)(sw_luminance(color)*255.0f);
}

static inline void sw_pixel_write_color_GRAYALPHA(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint8_t *dst = &((uint8_t *)pixels)[offset*2];
    dst[0] = (uint8_t)(sw_luminance(color)*255.0f);
    dst[1] = (uint8_t)(color[3]*255.0f);
}

static inline void sw_pixel_write_color_R3G3B2(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint8_t color8[4];
    sw_color_to_color8(color8, color);
    sw_pixel_write_color8_R3G3B2(pixels, color8, offset);
}

static inline void sw_pixel_write_color_R5G6B5(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint8_t color8[4];
    sw_color_to_color8(color8, color);
    sw_pixel_write_color8_R5G6B5(pixels, color8, offset);
}

static inline void sw_pixel_write_color_R8G8B8(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint8_t color8[4];
    sw_color_to_color8(color8, color);
    sw_pixel_write_color8_R8G8B8(pixels, color8, offset);
}

static inline void sw_pixel_write_color_R5G5B5A1(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint8_t color8[4];
    sw_color_to_color8(color8, color);
    sw_pixel_write_color8_R5G5B5A1(pixels, color8, offset);
}

static inline void sw_pixel_write_color_R4G4B4A4(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint8_t color8[4];
    sw_color_to_color8(color8, color);
    sw_pixel_write_color8_R4G4B4A4(pixels, color8, offset);
}

static inline void sw_pixel_write_color_R8G8B8A8(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    sw_color_to_color8(&((uint8_t *)pixels)[offset*4], color);
}

static inline void sw_pixel_write_color_R32(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    ((float *)pixels)[offset] = sw_luminance(color);
}

static inline void sw_pixel_write_color_R32G32B32(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    float *dst = &((float *)pixels)[offset*3];
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
}

static inline void sw_pixel_write_color_R32G32B32A32(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    float *dst = &((float *)pixels)[offset*4];
    dst[0] = color[0];
    dst[1] = color[1];
    dst[2] = color[2];
    dst[3] = color[3];
}

static inline void sw_pixel_write_color_R16(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    ((uint16_t *)pixels)[offset] = sw_float_to_half(sw_luminance(color));
}

static inline void sw_pixel_write_color_R16G16B16(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint16_t *dst = &((uint16_t *)pixels)[offset*3];
    dst[0] = sw_float_to_half(color[0]);
    dst[1] = sw_float_to_half(color[1]);
    dst[2] = sw_float_to_half(color[2]);
}

static inline void sw_pixel_write_color_R16G16B16A16(void *SW_RESTRICT pixels, const float *SW_RESTRICT color, uint32_t offset)
{
    uint16_t *dst = &((uint16_t *)pixels)[offset*4];
    dst[0] = sw_float_to_half(color[0]);
    dst[1] = sw_float_to_half(color[1]);
    dst[2] = sw_float_to_half(color[2]);
    dst[3] = sw_float_to_half(color[3]);
}

static inline sw_pixel_write_color_f sw_pixel_get_write_color_func(sw_pixelformat_t format)
{
    switch (format)
    {
        case SW_PIXELFORMAT_COLOR_GRAYSCALE: return sw_pixel_write_color_GRAYSCALE;
        case SW_PIXELFORMAT_COLOR_GRAYALPHA: return sw_pixel_write_color_GRAYALPHA;
        case SW_PIXELFORMAT_COLOR_R3G3B2: return sw_pixel_write_color_R3G3B2;
        case SW_PIXELFORMAT_COLOR_R5G6B5: return sw_pixel_write_color_R5G6B5;
        case SW_PIXELFORMAT_COLOR_R8G8B8: return sw_pixel_write_color_R8G8B8;
        case SW_PIXELFORMAT_COLOR_R5G5B5A1: return sw_pixel_write_color_R5G5B5A1;
        case SW_PIXELFORMAT_COLOR_R4G4B4A4: return sw_pixel_write_color_R4G4B4A4;
        case SW_PIXELFORMAT_COLOR_R8G8B8A8: return sw_pixel_write_color_R8G8B8A8;
        case SW_PIXELFORMAT_COLOR_R32: return sw_pixel_write_color_R32;
        case SW_PIXELFORMAT_COLOR_R32G32B32: return sw_pixel_write_color_R32G32B32;
        case SW_PIXELFORMAT_COLOR_R32G32B32A32: return sw_pixel_write_color_R32G32B32A32;
        case SW_PIXELFORMAT_COLOR_R16: return sw_pixel_write_color_R16;
        case SW_PIXELFORMAT_COLOR_R16G16B16: return sw_pixel_write_color_R16G16B16;
        case SW_PIXELFORMAT_COLOR_R16G16B16A16: return sw_pixel_write_color_R16G16B16A16;

        case SW_PIXELFORMAT_UNKNOWN:
        case SW_PIXELFORMAT_DEPTH_D8:
        case SW_PIXELFORMAT_DEPTH_D16:
        case SW_PIXELFORMAT_DEPTH_D32:
        case SW_PIXELFORMAT_COUNT:
        default: break;
    }

    return NULL;
}

static inline float sw_pixel_read_depth_D8(const void *pixels, uint32_t offset)
{
    return (float)((uint8_t *)pixels)[offset]*SW_INV_255;
}

static inline float sw_pixel_read_depth_D16(const void *pixels, uint32_t offset)
{
    return (float)((uint16_t *)pixels)[offset]/UINT16_MAX;
}

static inline float sw_pixel_read_depth_D32(const void *pixels, uint32_t offset)
{
    return ((float *)pixels)[offset];
}

static inline void sw_pixel_write_depth_D8(void *pixels, float depth, uint32_t offset)
{
    ((uint8_t *)pixels)[offset] = (uint8_t)(depth*UINT8_MAX);
}

static inline void sw_pixel_write_depth_D16(void *pixels, float depth, uint32_t offset)
{
    ((uint16_t *)pixels)[offset] = (uint16_t)(depth*UINT16_MAX);
}

static inline void sw_pixel_write_depth_D32(void *pixels, float depth, uint32_t offset)
{
    ((float *)pixels)[offset] = depth;
}
//-------------------------------------------------------------------------------------------

// Texture functionality
//-------------------------------------------------------------------------------------------
static inline bool sw_texture_alloc(sw_texture_t *texture, const void *data, int w, int h, sw_pixelformat_t format)
{
    bool isDepth = sw_pixel_is_depth_format(format);
    int bpp = SW_PIXELFORMAT_SIZE[format];
    int newSize = w*h*bpp;

    if (newSize > texture->allocSz)
    {
        void *ptr = SW_REALLOC(texture->pixels, newSize);
        if (!ptr) { RLSW.errCode = SW_OUT_OF_MEMORY; return false; }
        texture->allocSz = newSize;
        texture->pixels = ptr;
    }

    uint8_t *dst = texture->pixels;
    const uint8_t *src = data;

    sw_pixel_read_color8_f readColor8 = NULL;
    sw_pixel_read_color_f readColor = NULL;
    if (!isDepth)
    {
        readColor8 = sw_pixel_get_read_color8_func(format);
        readColor = sw_pixel_get_read_color_func(format);
    }

    sw_pixel_alpha_t pixelAlpha = SW_PIXELFORMAT_ALPHA[format];
    bool alphaFound = !data; // No data: assume transparency

    if (data && !isDepth)
    {
        for (int i = 0; i < newSize; i++) dst[i] = src[i];

        if (pixelAlpha != SW_PIXEL_ALPHA_NONE)
        {
            for (int i = 0; i < newSize; i += bpp)
            {
                uint8_t color[4] = { 0 };
                readColor8(color, &src[i], 0);
                if (color[3] < 255) { alphaFound = true; break; }
            }
        }
    }
    else
    {
        for (int i = 0; i < newSize; i++) dst[i] = 0;
    }

    texture->readColor8 = readColor8;
    texture->readColor = readColor;
    texture->format = format;
    texture->alpha = alphaFound? pixelAlpha : SW_PIXEL_ALPHA_NONE;
    texture->width = w;
    texture->height = h;
    texture->wMinus1 = w - 1;
    texture->hMinus1 = h - 1;
    texture->tx = 1.0f/w;
    texture->ty = 1.0f/h;

    return true;
}

static inline void sw_texture_free(sw_texture_t *texture)
{
    SW_FREE(texture->pixels);
}

static inline void sw_texture_sample_nearest(float *SW_RESTRICT color, const sw_texture_t *SW_RESTRICT tex, float u, float v)
{
    u = (tex->sWrap == SW_REPEAT)? sw_fract(u) : sw_saturate(u);
    v = (tex->tWrap == SW_REPEAT)? sw_fract(v) : sw_saturate(v);

    int x = u*tex->width;
    int y = v*tex->height;

    tex->readColor(color, tex->pixels, y*tex->width + x);
}

static inline void sw_texture_sample_linear(float *SW_RESTRICT color, const sw_texture_t *SW_RESTRICT tex, float u, float v)
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
    tex->readColor(c00, tex->pixels, y0*tex->width + x0);
    tex->readColor(c10, tex->pixels, y0*tex->width + x1);
    tex->readColor(c01, tex->pixels, y1*tex->width + x0);
    tex->readColor(c11, tex->pixels, y1*tex->width + x1);

    for (int i = 0; i < 4; i++)
    {
        float t = c00[i] + fx*(c10[i] - c00[i]);
        float b = c01[i] + fx*(c11[i] - c01[i]);
        color[i] = t + fy*(b - t);
    }
}

static inline void sw_texture_sample(float *SW_RESTRICT color, const sw_texture_t *SW_RESTRICT tex,
                                     float u, float v, float dUdx, float dUdy, float dVdx, float dVdy)
{
    // NOTE: Commented there is the previous method used
    // There was no need to compute the square root because
    // using the squared value, the comparison remains (L2 > 1.0f*1.0f)
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

// Framebuffer management functions
//-------------------------------------------------------------------------------------------
static inline bool sw_default_framebuffer_alloc(sw_default_framebuffer_t *fb, int w, int h)
{
    if (!sw_texture_alloc(&fb->color, NULL, w, h, SW_FRAMEBUFFER_COLOR_FORMAT))
    {
        return false;
    }

    if (!sw_texture_alloc(&fb->depth, NULL, w, h, SW_FRAMEBUFFER_DEPTH_FORMAT))
    {
        return false;
    }

    return true;
}

static inline void sw_default_framebuffer_free(sw_default_framebuffer_t *fb)
{
    sw_texture_free(&fb->color);
    sw_texture_free(&fb->depth);
}

static inline void sw_framebuffer_fill_color(sw_texture_t *colorBuffer, const float color[4])
{
    // NOTE: MSVC doesn't support VLA, so the largest possible size is allocated: 16 bytes
    //uint8_t pixel[SW_FRAMEBUFFER_COLOR_SIZE] = { 0 };
    uint8_t pixel[16] = { 0 };
    SW_FRAMEBUFFER_COLOR_SET(pixel, color, 0);

    uint8_t *dst = (uint8_t *)colorBuffer->pixels;

    if (RLSW.userState & SW_STATE_SCISSOR_TEST)
    {
        int xMin = sw_clamp_int(RLSW.scMin[0], 0, colorBuffer->width - 1);
        int xMax = sw_clamp_int(RLSW.scMax[0], 0, colorBuffer->width - 1);
        int yMin = sw_clamp_int(RLSW.scMin[1], 0, colorBuffer->height - 1);
        int yMax = sw_clamp_int(RLSW.scMax[1], 0, colorBuffer->height - 1);

        int w = xMax - xMin;
        for (int y = yMin; y <= yMax; y++)
        {
            uint8_t *row = dst + (y*colorBuffer->width + xMin)*SW_FRAMEBUFFER_COLOR_SIZE;
            for (int x = 0; x <= w; x++, row += SW_FRAMEBUFFER_COLOR_SIZE)
            {
                for (int b = 0; b < SW_FRAMEBUFFER_COLOR_SIZE; b++) row[b] = pixel[b];
            }
        }
    }
    else
    {
        int size = colorBuffer->width*colorBuffer->height;
        for (int i = 0; i < size; i++)
        {
            for (int b = 0; b < SW_FRAMEBUFFER_COLOR_SIZE; b++)
            {
                dst[i*SW_FRAMEBUFFER_COLOR_SIZE + b] = pixel[b];
            }
        }
    }
}

static inline void sw_framebuffer_fill_depth(sw_texture_t *depthBuffer, float depth)
{
    // NOTE: MSVC doesn't support VLA, so the largest possible size is allocated: 4 bytes
    //uint8_t pixel[SW_FRAMEBUFFER_DEPTH_SIZE] = { 0 };
    uint8_t pixel[4] = { 0 };
    SW_FRAMEBUFFER_DEPTH_SET(pixel, depth, 0);

    uint8_t *dst = (uint8_t *)depthBuffer->pixels;

    if (RLSW.userState & SW_STATE_SCISSOR_TEST)
    {
        int xMin = sw_clamp_int(RLSW.scMin[0], 0, depthBuffer->width - 1);
        int xMax = sw_clamp_int(RLSW.scMax[0], 0, depthBuffer->width - 1);
        int yMin = sw_clamp_int(RLSW.scMin[1], 0, depthBuffer->height - 1);
        int yMax = sw_clamp_int(RLSW.scMax[1], 0, depthBuffer->height - 1);

        int w = xMax - xMin;
        for (int y = yMin; y <= yMax; y++)
        {
            uint8_t *row = dst + (y*depthBuffer->width + xMin)*SW_FRAMEBUFFER_DEPTH_SIZE;
            for (int x = 0; x <= w; x++, row += SW_FRAMEBUFFER_DEPTH_SIZE)
            {
                for (int b = 0; b < SW_FRAMEBUFFER_DEPTH_SIZE; b++) row[b] = pixel[b];
            }
        }
    }
    else
    {
        int size = depthBuffer->width*depthBuffer->height;
        for (int i = 0; i < size; i++)
        {
            for (int b = 0; b < SW_FRAMEBUFFER_DEPTH_SIZE; b++)
            {
                dst[i*SW_FRAMEBUFFER_DEPTH_SIZE + b] = pixel[b];
            }
        }
    }
}

static inline void sw_framebuffer_output_fast(void *dst, const sw_texture_t *buffer)
{
    int width = buffer->width;
    int height = buffer->height;

    uint8_t *d = (uint8_t *)dst;

#if SW_FRAMEBUFFER_OUTPUT_BGRA && (SW_FRAMEBUFFER_COLOR_FORMAT == SW_PIXELFORMAT_COLOR_R8G8B8A8)
    for (int y = height - 1; y >= 0; y--)
    {
        const uint8_t *src = (uint8_t *)(buffer->pixels) + y*width*4;
        for (int x = 0; x < width; x++, src += 4, d += 4)
        {
            d[0] = src[2];
            d[1] = src[1];
            d[2] = src[0];
            d[3] = src[3];
        }
    }
#elif SW_FRAMEBUFFER_OUTPUT_BGRA && (SW_FRAMEBUFFER_COLOR_FORMAT == SW_PIXELFORMAT_COLOR_R8G8B8)
    for (int y = height - 1; y >= 0; y--)
    {
        const uint8_t *src = (uint8_t *)(buffer->pixels) + y*width*3;
        for (int x = 0; x < width; x++, src += 3, d += 3)
        {
            d[0] = src[2];
            d[1] = src[1];
            d[2] = src[0];
        }
    }
#else
    int rowBytes = width*SW_FRAMEBUFFER_COLOR_SIZE;
    for (int y = height - 1; y >= 0; y--)
    {
        const uint8_t *src = (uint8_t *)(buffer->pixels) + y*rowBytes;
        for (int i = 0; i < rowBytes; i++) d[i] = src[i];
        d += rowBytes;
    }
#endif
}

static inline void sw_framebuffer_output_copy(void *dst, const sw_texture_t *buffer, int x, int y, int w, int h, sw_pixelformat_t format)
{
    int stride = buffer->width;
    int dstPixelSize = SW_PIXELFORMAT_SIZE[format];
    sw_pixel_write_color8_f setColor8 = sw_pixel_get_write_color8_func(format);

    const uint8_t *src = (uint8_t *)(buffer->pixels) + ((y + h - 1)*stride + x)*SW_FRAMEBUFFER_COLOR_SIZE;
    uint8_t *d = dst;

    for (int iy = 0; iy < h; iy++)
    {
        const uint8_t *line = src;
        uint8_t *dline = d;

        for (int ix = 0; ix < w; ix++)
        {
            uint8_t color[4];
            SW_FRAMEBUFFER_COLOR8_GET(color, line, 0);

            #if SW_FRAMEBUFFER_OUTPUT_BGRA
            if (format == SW_PIXELFORMAT_COLOR_R8G8B8A8 || format == SW_PIXELFORMAT_COLOR_R8G8B8)
            {
                uint8_t tmp = color[0]; color[0] = color[2]; color[2] = tmp;
            }
            #endif

            setColor8(dline, color, 0);
            line += SW_FRAMEBUFFER_COLOR_SIZE;
            dline += dstPixelSize;
        }

        src -= stride*SW_FRAMEBUFFER_COLOR_SIZE;
        d += w*dstPixelSize;
    }
}

static inline void sw_framebuffer_output_blit(void *dst, const sw_texture_t *buffer,
    int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, sw_pixelformat_t format)
{
    const uint8_t *srcBase = buffer->pixels;

    int fbWidth = buffer->width;
    int dstPixelSize = SW_PIXELFORMAT_SIZE[format];
    sw_pixel_write_color8_f setColor8 = sw_pixel_get_write_color8_func(format);

    uint32_t xScale = ((uint32_t)wSrc << 16)/(uint32_t)wDst;
    uint32_t yScale = ((uint32_t)hSrc << 16)/(uint32_t)hDst;

    int ySrcLast = ySrc + hSrc - 1;
    uint8_t *d = (uint8_t *)dst;

    for (int dy = 0; dy < hDst; dy++)
    {
        int sy = ySrcLast - (int)(dy*yScale >> 16);
        const uint8_t *srcLine = srcBase + (sy*fbWidth + xSrc)*SW_FRAMEBUFFER_COLOR_SIZE;
        uint8_t *dline = d;

        for (int dx = 0; dx < wDst; dx++)
        {
            int sx = (int)(dx*xScale >> 16);
            const uint8_t *pixel = srcLine + sx*SW_FRAMEBUFFER_COLOR_SIZE;

            uint8_t color[4];
            SW_FRAMEBUFFER_COLOR8_GET(color, pixel, 0);

            #if SW_FRAMEBUFFER_OUTPUT_BGRA
            if (format == SW_PIXELFORMAT_COLOR_R8G8B8A8 || format == SW_PIXELFORMAT_COLOR_R8G8B8)
            {
                uint8_t tmp = color[0]; color[0] = color[2]; color[2] = tmp;
            }
            #endif

            setColor8(dline, color, 0);
            dline += dstPixelSize;
        }

        d += wDst*dstPixelSize;
    }
}
//-------------------------------------------------------------------------------------------

// Color blending functionality
//-------------------------------------------------------------------------------------------
// Blend factor component macros: SW_BF_XXX(src, dst, component_index)
// Each expands to the scalar factor value for one RGBA channel
#define SW_BF_ZERO(s,d,i)                0.0f
#define SW_BF_ONE(s,d,i)                 1.0f
#define SW_BF_SRC_COLOR(s,d,i)           (s)[i]
#define SW_BF_ONE_MINUS_SRC_COLOR(s,d,i) (1.0f-(s)[i])
#define SW_BF_SRC_ALPHA(s,d,i)           (s)[3]
#define SW_BF_ONE_MINUS_SRC_ALPHA(s,d,i) (1.0f-(s)[3])
#define SW_BF_DST_ALPHA(s,d,i)           (d)[3]
#define SW_BF_ONE_MINUS_DST_ALPHA(s,d,i) (1.0f-(d)[3])
#define SW_BF_DST_COLOR(s,d,i)           (d)[i]
#define SW_BF_ONE_MINUS_DST_COLOR(s,d,i) (1.0f-(d)[i])
#define SW_BF_SRC_ALPHA_SATURATE(s,d,i)  (((i)<3)? 1.0f : (((s)[3]<1.0f)?(s)[3]:1.0f))

// Generates one specialized blend function for a (sfactor, dfactor) pair
#define DEFINE_BLEND_FUNC(sn, dn, SF, DF)                                               \
static void sw_blend_##sn##_##dn(float *SW_RESTRICT dst, const float *SW_RESTRICT src)  \
{                                                                                       \
    dst[0] = SF(src,dst,0)*src[0] + DF(src,dst,0)*dst[0];                               \
    dst[1] = SF(src,dst,1)*src[1] + DF(src,dst,1)*dst[1];                               \
    dst[2] = SF(src,dst,2)*src[2] + DF(src,dst,2)*dst[2];                               \
    dst[3] = SF(src,dst,3)*src[3] + DF(src,dst,3)*dst[3];                               \
}

// Master factor list: X(c_name, gl_enum, compact_index, SW_BF_macro)
// compact_index is used to index SW_BLEND_TABLE (GL enums are non contiguous)
#define FOREACH_FACTOR(X) \
    X(ZERO,                SW_ZERO,                0,  SW_BF_ZERO               ) \
    X(ONE,                 SW_ONE,                 1,  SW_BF_ONE                ) \
    X(SRC_COLOR,           SW_SRC_COLOR,           2,  SW_BF_SRC_COLOR          ) \
    X(ONE_MINUS_SRC_COLOR, SW_ONE_MINUS_SRC_COLOR, 3,  SW_BF_ONE_MINUS_SRC_COLOR) \
    X(SRC_ALPHA,           SW_SRC_ALPHA,           4,  SW_BF_SRC_ALPHA          ) \
    X(ONE_MINUS_SRC_ALPHA, SW_ONE_MINUS_SRC_ALPHA, 5,  SW_BF_ONE_MINUS_SRC_ALPHA) \
    X(DST_ALPHA,           SW_DST_ALPHA,           6,  SW_BF_DST_ALPHA          ) \
    X(ONE_MINUS_DST_ALPHA, SW_ONE_MINUS_DST_ALPHA, 7,  SW_BF_ONE_MINUS_DST_ALPHA) \
    X(DST_COLOR,           SW_DST_COLOR,           8,  SW_BF_DST_COLOR          ) \
    X(ONE_MINUS_DST_COLOR, SW_ONE_MINUS_DST_COLOR, 9,  SW_BF_ONE_MINUS_DST_COLOR) \
    X(SRC_ALPHA_SATURATE,  SW_SRC_ALPHA_SATURATE,  10, SW_BF_SRC_ALPHA_SATURATE )

// Same list but forwards 3 extra args (A, B, C) to X
#define FOREACH_FACTOR_WITH(X, A, B, C) \
    X(ZERO,                SW_ZERO,                0,  SW_BF_ZERO,               A,B,C) \
    X(ONE,                 SW_ONE,                 1,  SW_BF_ONE,                A,B,C) \
    X(SRC_COLOR,           SW_SRC_COLOR,           2,  SW_BF_SRC_COLOR,          A,B,C) \
    X(ONE_MINUS_SRC_COLOR, SW_ONE_MINUS_SRC_COLOR, 3,  SW_BF_ONE_MINUS_SRC_COLOR,A,B,C) \
    X(SRC_ALPHA,           SW_SRC_ALPHA,           4,  SW_BF_SRC_ALPHA,          A,B,C) \
    X(ONE_MINUS_SRC_ALPHA, SW_ONE_MINUS_SRC_ALPHA, 5,  SW_BF_ONE_MINUS_SRC_ALPHA,A,B,C) \
    X(DST_ALPHA,           SW_DST_ALPHA,           6,  SW_BF_DST_ALPHA,          A,B,C) \
    X(ONE_MINUS_DST_ALPHA, SW_ONE_MINUS_DST_ALPHA, 7,  SW_BF_ONE_MINUS_DST_ALPHA,A,B,C) \
    X(DST_COLOR,           SW_DST_COLOR,           8,  SW_BF_DST_COLOR,          A,B,C) \
    X(ONE_MINUS_DST_COLOR, SW_ONE_MINUS_DST_COLOR, 9,  SW_BF_ONE_MINUS_DST_COLOR,A,B,C) \
    X(SRC_ALPHA_SATURATE,  SW_SRC_ALPHA_SATURATE,  10, SW_BF_SRC_ALPHA_SATURATE, A,B,C)

// Inner loop: receives dst factor + forwarded (sn, si, sf) from outer loop
#define GEN_COMBO(dn, de, di, df, sn, si, sf) DEFINE_BLEND_FUNC(sn, dn, sf, df)

// Outer loop: for each src factor, iterate all dst factors
#define GEN_ROW(sn, se, si, sf) FOREACH_FACTOR_WITH(GEN_COMBO, sn, si, sf)

// Generates all 121 sw_blend_SFACTOR_DFACTOR functions
FOREACH_FACTOR(GEN_ROW)

#undef GEN_COMBO
#undef GEN_ROW

// Inner loop: emits one table entry using compact indices (not GL enum values)
#define GEN_TABLE_ENTRY(dn, de, di, df, sn, si, sf) [si][di] = sw_blend_##sn##_##dn,

// Outer loop: fills one row of the table for a given src factor
#define GEN_TABLE_ROW(sn, se, si, sf) FOREACH_FACTOR_WITH(GEN_TABLE_ENTRY, sn, si, sf)

// 2D dispatch table indexed by compact src/dst factor indices
#define SW_BLEND_FACTOR_COUNT 11
static const sw_blend_f SW_BLEND_TABLE[SW_BLEND_FACTOR_COUNT][SW_BLEND_FACTOR_COUNT] = {
    FOREACH_FACTOR(GEN_TABLE_ROW)
};

#undef GEN_TABLE_ENTRY
#undef GEN_TABLE_ROW

// Maps a GL blend factor enum to its compact table index
static inline int sw_blend_factor_index(SWfactor f)
{
    switch (f)
    {
        case SW_ZERO:                return 0;
        case SW_ONE:                 return 1;
        case SW_SRC_COLOR:           return 2;
        case SW_ONE_MINUS_SRC_COLOR: return 3;
        case SW_SRC_ALPHA:           return 4;
        case SW_ONE_MINUS_SRC_ALPHA: return 5;
        case SW_DST_ALPHA:           return 6;
        case SW_ONE_MINUS_DST_ALPHA: return 7;
        case SW_DST_COLOR:           return 8;
        case SW_ONE_MINUS_DST_COLOR: return 9;
        case SW_SRC_ALPHA_SATURATE:  return 10;
        default:                     return -1;
    }
}

static bool sw_blend_factor_needs_alpha(SWfactor f)
{
    switch (f)
    {
        case SW_SRC_ALPHA:
        case SW_ONE_MINUS_SRC_ALPHA:
        case SW_DST_ALPHA:
        case SW_ONE_MINUS_DST_ALPHA:
        case SW_SRC_ALPHA_SATURATE: return true;
        default: break;
    }

    return false;
}

static uint32_t sw_blend_compute_flags(SWfactor src, SWfactor dst)
{
    uint32_t flags = 0;

    // Blend is a no-op: result = src*1 + dst*0 = src
    if (src == SW_ONE && dst == SW_ZERO) flags |= SW_BLEND_FLAG_NOOP;

    // Factors that depend on the alpha channel
    if (sw_blend_factor_needs_alpha(src) || sw_blend_factor_needs_alpha(dst)) flags |= SW_BLEND_FLAG_NEEDS_ALPHA;

    return flags;
}
//-------------------------------------------------------------------------------------------

// Projection helper functions
//-------------------------------------------------------------------------------------------
static inline void sw_project_ndc_to_screen(float ndc[4])
{
    ndc[0] = RLSW.vpCenter[0] + ndc[0]*RLSW.vpHalf[0] + 0.5f;
    ndc[1] = RLSW.vpCenter[1] + ndc[1]*RLSW.vpHalf[1] + 0.5f;
}
//-------------------------------------------------------------------------------------------

// Polygon clipping management
//-------------------------------------------------------------------------------------------
#define DEFINE_CLIP_FUNC(name, FUNC_IS_INSIDE, FUNC_COMPUTE_T)                          \
static int sw_clip_##name(                                                              \
    sw_vertex_t output[SW_MAX_CLIPPED_POLYGON_VERTICES],                                \
    const sw_vertex_t input[SW_MAX_CLIPPED_POLYGON_VERTICES],                           \
    int n)                                                                              \
{                                                                                       \
    const sw_vertex_t *prev = &input[n - 1];                                            \
    int prevInside = FUNC_IS_INSIDE(prev->position);                                    \
    int outputCount = 0;                                                                \
                                                                                        \
    for (int i = 0; i < n; i++) {                                                       \
        const sw_vertex_t *curr = &input[i];                                            \
        int currInside = FUNC_IS_INSIDE(curr->position);                                \
                                                                                        \
        /* If transition between interior/exterior, calculate intersection point */     \
        if (prevInside != currInside) {                                                 \
            float t = FUNC_COMPUTE_T(prev->position, curr->position);                   \
            sw_lerp_vertex_PCT(&output[outputCount++], prev, curr, t);                  \
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
static bool sw_polygon_clip(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int *vertexCounter)
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

    if (RLSW.userState & SW_STATE_SCISSOR_TEST)
    {
        CLIP_AGAINST_PLANE(sw_clip_scissor_x_min);
        CLIP_AGAINST_PLANE(sw_clip_scissor_x_max);
        CLIP_AGAINST_PLANE(sw_clip_scissor_y_min);
        CLIP_AGAINST_PLANE(sw_clip_scissor_y_max);
    }

    *vertexCounter = n;

    return (n >= 3);
}

// Triangle rasterizer variant dispatch
//-------------------------------------------------------------------------------------------
#ifndef RLSW_TEMPLATE_RASTER_TRIANGLE_EXPANDING
#define RLSW_TEMPLATE_RASTER_TRIANGLE_EXPANDING

    // State mask to apply before indexing the dispatch table
    #define SW_RASTER_TRIANGLE_STATE_MASK \
        (SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Single source of truth for all rasterizer specializations
    // X(NAME, STATE_FLAGS)
    #define SW_RASTER_VARIANTS(X)                                                       \
        X(BASE,            0)                                                           \
        X(TEX,             SW_STATE_TEXTURE_2D)                                         \
        X(DEPTH,           SW_STATE_DEPTH_TEST)                                         \
        X(BLEND,           SW_STATE_BLEND)                                              \
        X(TEX_DEPTH,       SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)                   \
        X(TEX_BLEND,       SW_STATE_TEXTURE_2D | SW_STATE_BLEND)                        \
        X(DEPTH_BLEND,     SW_STATE_DEPTH_TEST | SW_STATE_BLEND)                        \
        X(TEX_DEPTH_BLEND, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Forward declarations because clangd does not follow #include __FILE__ to avoid infinite recursion
    // These declarations make all variants visible to static analysis tools without affecting compilation
    #define SW_FWD_DECL(NAME, _FLAGS) \
        static void sw_raster_triangle_##NAME(const sw_vertex_t*, const sw_vertex_t*, const sw_vertex_t*);
    SW_RASTER_VARIANTS(SW_FWD_DECL) // NOLINT
    #undef SW_FWD_DECL

    // Specialization generation via self-inclusion

    #define RLSW_TEMPLATE_RASTER_TRIANGLE BASE
    #include __FILE__ // IWYU pragma: keep
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE TEX
    #define SW_ENABLE_TEXTURE
    #include __FILE__
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE DEPTH
    #define SW_ENABLE_DEPTH_TEST
    #include __FILE__
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE BLEND
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE TEX_DEPTH
    #define SW_ENABLE_TEXTURE
    #define SW_ENABLE_DEPTH_TEST
    #include __FILE__
    #undef SW_ENABLE_DEPTH_TEST
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE TEX_BLEND
    #define SW_ENABLE_TEXTURE
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE DEPTH_BLEND
    #define SW_ENABLE_DEPTH_TEST
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    #define RLSW_TEMPLATE_RASTER_TRIANGLE TEX_DEPTH_BLEND
    #define SW_ENABLE_TEXTURE
    #define SW_ENABLE_DEPTH_TEST
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_DEPTH_TEST
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_TRIANGLE

    // Dispatch table (auto-generated from SW_RASTER_VARIANTS)
    #define SW_TABLE_ENTRY(NAME, FLAGS) [FLAGS] = sw_raster_triangle_##NAME,
    static const sw_raster_triangle_f SW_RASTER_TRIANGLE_TABLE[] = {
        SW_RASTER_VARIANTS(SW_TABLE_ENTRY)
    };
    #undef SW_TABLE_ENTRY

#undef SW_RASTER_VARIANTS
#undef RLSW_TEMPLATE_RASTER_TRIANGLE_EXPANDING

#endif // RLSW_TEMPLATE_RASTER_TRIANGLE_EXPANDING
//-------------------------------------------------------------------------------------------

// Quad rasterizer variant dispatch
//-------------------------------------------------------------------------------------------
#ifndef RLSW_TEMPLATE_RASTER_QUAD_EXPANDING
#define RLSW_TEMPLATE_RASTER_QUAD_EXPANDING

    // State mask to apply before indexing the dispatch table
    #define SW_RASTER_QUAD_STATE_MASK \
        (SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Single source of truth for all rasterizer specializations
    // X(NAME, STATE_FLAGS)
    #define SW_RASTER_VARIANTS(X)                                                       \
        X(BASE,            0)                                                           \
        X(TEX,             SW_STATE_TEXTURE_2D)                                         \
        X(DEPTH,           SW_STATE_DEPTH_TEST)                                         \
        X(BLEND,           SW_STATE_BLEND)                                              \
        X(TEX_DEPTH,       SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)                   \
        X(TEX_BLEND,       SW_STATE_TEXTURE_2D | SW_STATE_BLEND)                        \
        X(DEPTH_BLEND,     SW_STATE_DEPTH_TEST | SW_STATE_BLEND)                        \
        X(TEX_DEPTH_BLEND, SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Forward declarations because clangd does not follow #include __FILE__ to avoid infinite recursion
    // These declarations make all variants visible to static analysis tools without affecting compilation
    #define SW_FWD_DECL(NAME, _FLAGS) \
        static void sw_raster_quad_##NAME(const sw_vertex_t *v0, const sw_vertex_t *v1, const sw_vertex_t *v2, const sw_vertex_t *v3);
    SW_RASTER_VARIANTS(SW_FWD_DECL) // NOLINT
    #undef SW_FWD_DECL

    // Specialization generation via self-inclusion

    #define RLSW_TEMPLATE_RASTER_QUAD BASE
    #include __FILE__ // IWYU pragma: keep
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD TEX
    #define SW_ENABLE_TEXTURE
    #include __FILE__
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD DEPTH
    #define SW_ENABLE_DEPTH_TEST
    #include __FILE__
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD BLEND
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD TEX_DEPTH
    #define SW_ENABLE_TEXTURE
    #define SW_ENABLE_DEPTH_TEST
    #include __FILE__
    #undef SW_ENABLE_DEPTH_TEST
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD TEX_BLEND
    #define SW_ENABLE_TEXTURE
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD DEPTH_BLEND
    #define SW_ENABLE_DEPTH_TEST
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_QUAD

    #define RLSW_TEMPLATE_RASTER_QUAD TEX_DEPTH_BLEND
    #define SW_ENABLE_TEXTURE
    #define SW_ENABLE_DEPTH_TEST
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_DEPTH_TEST
    #undef SW_ENABLE_TEXTURE
    #undef RLSW_TEMPLATE_RASTER_QUAD

    // Dispatch table (auto-generated from SW_RASTER_VARIANTS)
    #define SW_TABLE_ENTRY(NAME, FLAGS) [FLAGS] = sw_raster_quad_##NAME,
    static const sw_raster_quad_f SW_RASTER_QUAD_TABLE[] = {
        SW_RASTER_VARIANTS(SW_TABLE_ENTRY)
    };
    #undef SW_TABLE_ENTRY

#undef SW_RASTER_VARIANTS
#undef RLSW_TEMPLATE_RASTER_QUAD_EXPANDING

#endif // RLSW_TEMPLATE_RASTER_QUAD_EXPANDING
//-------------------------------------------------------------------------------------------

// Line rasterizer variant dispatch
//-------------------------------------------------------------------------------------------
#ifndef RLSW_TEMPLATE_RASTER_LINE_EXPANDING
#define RLSW_TEMPLATE_RASTER_LINE_EXPANDING

    // State mask to apply before indexing the dispatch table
    #define SW_RASTER_LINE_STATE_MASK \
        (SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Single source of truth for all rasterizer specializations
    // X(NAME, STATE_FLAGS)
    #define SW_RASTER_VARIANTS(X)                                                       \
        X(BASE,            0)                                                           \
        X(DEPTH,           SW_STATE_DEPTH_TEST)                                         \
        X(BLEND,           SW_STATE_BLEND)                                              \
        X(DEPTH_BLEND,     SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Forward declarations because clangd does not follow #include __FILE__ to avoid infinite recursion
    // These declarations make all variants visible to static analysis tools without affecting compilation
    #define SW_FWD_DECL(NAME, _FLAGS)                                                       \
        static void sw_raster_line_##NAME(const sw_vertex_t *v0, const sw_vertex_t *v1);    \
        static void sw_raster_line_thick_##NAME(const sw_vertex_t *v0, const sw_vertex_t *v1);
    SW_RASTER_VARIANTS(SW_FWD_DECL) // NOLINT
    #undef SW_FWD_DECL

    // Specialization generation via self-inclusion

    #define RLSW_TEMPLATE_RASTER_LINE BASE
    #include __FILE__ // IWYU pragma: keep
    #undef RLSW_TEMPLATE_RASTER_LINE

    #define RLSW_TEMPLATE_RASTER_LINE DEPTH
    #define SW_ENABLE_DEPTH_TEST
    #include __FILE__
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_LINE

    #define RLSW_TEMPLATE_RASTER_LINE BLEND
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef RLSW_TEMPLATE_RASTER_LINE

    #define RLSW_TEMPLATE_RASTER_LINE DEPTH_BLEND
    #define SW_ENABLE_DEPTH_TEST
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_LINE

    // Dispatch table (auto-generated from SW_RASTER_VARIANTS)
    #define SW_TABLE_ENTRY0(NAME, FLAGS) [FLAGS] = sw_raster_line_##NAME,
    #define SW_TABLE_ENTRY1(NAME, FLAGS) [FLAGS] = sw_raster_line_thick_##NAME,
    static const sw_raster_line_f SW_RASTER_LINE_TABLE[] = {  SW_RASTER_VARIANTS(SW_TABLE_ENTRY0) };
    static const sw_raster_line_f SW_RASTER_LINE_THICK_TABLE[] = { SW_RASTER_VARIANTS(SW_TABLE_ENTRY1) };
    #undef SW_TABLE_ENTRY0
    #undef SW_TABLE_ENTRY1

#undef SW_RASTER_VARIANTS
#undef RLSW_TEMPLATE_RASTER_LINE_EXPANDING

#endif // RLSW_TEMPLATE_RASTER_LINE_EXPANDING
//-------------------------------------------------------------------------------------------

// Point rasterizer variant dispatch
//-------------------------------------------------------------------------------------------
#ifndef RLSW_TEMPLATE_RASTER_POINT_EXPANDING
#define RLSW_TEMPLATE_RASTER_POINT_EXPANDING

    // State mask to apply before indexing the dispatch table
    #define SW_RASTER_POINT_STATE_MASK \
        (SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Single source of truth for all rasterizer specializations
    // X(NAME, STATE_FLAGS)
    #define SW_RASTER_VARIANTS(X)                                                       \
        X(BASE,            0)                                                           \
        X(DEPTH,           SW_STATE_DEPTH_TEST)                                         \
        X(BLEND,           SW_STATE_BLEND)                                              \
        X(DEPTH_BLEND,     SW_STATE_DEPTH_TEST | SW_STATE_BLEND)

    // Forward declarations because clangd does not follow #include __FILE__ to avoid infinite recursion
    // These declarations make all variants visible to static analysis tools without affecting compilation
    #define SW_FWD_DECL(NAME, _FLAGS) \
        static void sw_raster_point_##NAME(const sw_vertex_t *v);
    SW_RASTER_VARIANTS(SW_FWD_DECL) // NOLINT
    #undef SW_FWD_DECL

    // Specialization generation via self-inclusion

    #define RLSW_TEMPLATE_RASTER_POINT BASE
    #include __FILE__ // IWYU pragma: keep
    #undef RLSW_TEMPLATE_RASTER_POINT

    #define RLSW_TEMPLATE_RASTER_POINT DEPTH
    #define SW_ENABLE_DEPTH_TEST
    #include __FILE__
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_POINT

    #define RLSW_TEMPLATE_RASTER_POINT BLEND
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef RLSW_TEMPLATE_RASTER_POINT

    #define RLSW_TEMPLATE_RASTER_POINT DEPTH_BLEND
    #define SW_ENABLE_DEPTH_TEST
    #define SW_ENABLE_BLEND
    #include __FILE__
    #undef SW_ENABLE_BLEND
    #undef SW_ENABLE_DEPTH_TEST
    #undef RLSW_TEMPLATE_RASTER_POINT

    // Dispatch table (auto-generated from SW_RASTER_VARIANTS)
    #define SW_TABLE_ENTRY(NAME, FLAGS) [FLAGS] = sw_raster_point_##NAME,
    static const sw_raster_point_f SW_RASTER_POINT_TABLE[] = {
        SW_RASTER_VARIANTS(SW_TABLE_ENTRY)
    };
    #undef SW_TABLE_ENTRY

#undef SW_RASTER_VARIANTS
#undef RLSW_TEMPLATE_RASTER_POINT_EXPANDING

#endif // RLSW_TEMPLATE_RASTER_POINT_EXPANDING
//-------------------------------------------------------------------------------------------

// Triangle rendering logic
//-------------------------------------------------------------------------------------------
static inline bool sw_triangle_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations
    // To handle triangles crossing the w=0 plane correctly,
    // the winding order test is performeed in clip space directly,
    // before the perspective division (division by w)
    // This test determines the orientation of the triangle in the (x,y,w) plane,
    // which corresponds to the projected 2D winding order sign,
    // even with negative w values

    // Preload clip coordinates into local variables
    const float *p0 = RLSW.primitive.buffer[0].position;
    const float *p1 = RLSW.primitive.buffer[1].position;
    const float *p2 = RLSW.primitive.buffer[2].position;

    // Compute a value proportional to the signed area in the projected 2D plane,
    // calculated directly using clip coordinates BEFORE division by w
    // This is the determinant of the matrix formed by the (x, y, w) components
    // of the vertices, which correctly captures the winding order in clip
    // space and its relationship to the projected 2D winding order, even with
    // negative w values
    // The determinant formula used here is:
    // p0.x*(p1.y*p2.w - p2.y*p1.w) +
    // p1.x*(p2.y*p0.w - p0.y*p2.w) +
    // p2.x*(p0.y*p1.w - p1.y*p0.w)

    const float sgnArea =
        p0[0]*(p1[1]*p2[3] - p2[1]*p1[3]) +
        p1[0]*(p2[1]*p0[3] - p0[1]*p2[3]) +
        p2[0]*(p0[1]*p1[3] - p1[1]*p0[3]);

    // Discard the triangle if its winding order (determined by the sign
    // of the homogeneous area/determinant) matches the culled direction
    // A positive sgnArea typically corresponds to a counter-clockwise
    // winding in the projected space when all w > 0
    // This test is robust for points with w > 0 or w < 0, correctly
    // capturing the change in orientation when crossing the w=0 plane

    // The culling logic remains the same based on the signed area/determinant
    // A value of 0 for sgnArea means the points are collinear in (x, y, w)
    // space, which corresponds to a degenerate triangle projection
    // Such triangles are typically not culled by this test (0 < 0 is false, 0 > 0 is false)
    // and should be handled by the clipper if necessary
    return (RLSW.cullFace == SW_FRONT)? (sgnArea < 0) : (sgnArea > 0); // Cull if winding is "clockwise" : "counter-clockwise"
}

static void sw_triangle_clip_and_project(void)
{
    sw_vertex_t *polygon = RLSW.primitive.buffer;
    int *vertexCounter = &RLSW.primitive.vertexCount;

    if (sw_polygon_clip(polygon, vertexCounter))
    {
        // Transformation to screen space and normalization
        for (int i = 0; i < *vertexCounter; i++)
        {
            sw_vertex_t *v = &polygon[i];

            // Calculation of the reciprocal of W for normalization
            // as well as perspective-correct attributes
            const float wRcp = 1.0f/v->position[3];

            // Division of XYZ coordinates by weight
            v->position[0] *= wRcp;
            v->position[1] *= wRcp;
            v->position[2] *= wRcp;
            v->position[3] = wRcp;

            // Division of texture coordinates (perspective-correct)
            v->texcoord[0] *= wRcp;
            v->texcoord[1] *= wRcp;

            // Division of colors (perspective-correct)
            v->color[0] *= wRcp;
            v->color[1] *= wRcp;
            v->color[2] *= wRcp;
            v->color[3] *= wRcp;

            // Transformation to screen space
            sw_project_ndc_to_screen(v->position);
        }
    }
}

static void sw_triangle_render(uint32_t state)
{
    if (RLSW.userState & SW_STATE_CULL_FACE)
    {
        if (!sw_triangle_face_culling()) return;
    }

    sw_triangle_clip_and_project();
    if (RLSW.primitive.vertexCount < 3) return;

    state &= SW_RASTER_TRIANGLE_STATE_MASK;

    for (int i = 0; i < RLSW.primitive.vertexCount - 2; i++)
    {
        SW_RASTER_TRIANGLE_TABLE[state](
            &RLSW.primitive.buffer[0],
            &RLSW.primitive.buffer[i + 1],
            &RLSW.primitive.buffer[i + 2]
        );
    }
}
//-------------------------------------------------------------------------------------------

// Quad rendering logic
//-------------------------------------------------------------------------------------------
static inline bool sw_quad_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations
    // To handle quads crossing the w=0 plane correctly,
    // the winding order test is performed in clip space directly,
    // before the perspective division (division by w)
    // For a convex quad with vertices P0, P1, P2, P3 in sequential order,
    // the winding order of the quad is the same as the winding order
    // of the triangle P0 P1 P2. The triangle in clip space is used on
    // winding test on this first triangle

    // Preload clip coordinates into local variables
    const float *p0 = RLSW.primitive.buffer[0].position;
    const float *p1 = RLSW.primitive.buffer[1].position;
    const float *p2 = RLSW.primitive.buffer[2].position;

    // NOTE: h3 is not needed for this test
    // const float *h3 = RLSW.primitive.buffer[3].position;

    // Compute a value proportional to the signed area of the triangle P0 P1 P2
    // in the projected 2D plane, calculated directly using clip coordinates
    // BEFORE division by w
    // This is the determinant of the matrix formed by the (x, y, w) components
    // of the vertices P0, P1, and P2. Its sign correctly indicates the winding order
    // in clip space and its relationship to the projected 2D winding order,
    // even with negative w values
    // The determinant formula used here is:
    // p0.x*(p1.y*p2.w - p2.y*p1.w) +
    // p1.x*(p2.y*p0.w - p0.y*p2.w) +
    // p2.x*(p0.y*p1.w - p1.y*p0.w)

    const float sgnArea =
        p0[0]*(p1[1]*p2[3] - p2[1]*p1[3]) +
        p1[0]*(p2[1]*p0[3] - p0[1]*p2[3]) +
        p2[0]*(p0[1]*p1[3] - p1[1]*p0[3]);

    // Perform face culling based on the winding order determined by the sign
    // of the homogeneous area/determinant of triangle P0 P1 P2
    // This test is robust for points with w > 0 or w < 0 within the triangle,
    // correctly capturing the change in orientation when crossing the w=0 plane

    // A positive sgnArea typically corresponds to a counter-clockwise
    // winding in the projected space when all w > 0
    // A value of 0 for sgnArea means P0, P1, P2 are collinear in (x, y, w)
    // space, which corresponds to a degenerate triangle projection
    // Such quads might also be degenerate or non-planar. They are typically
    // not culled by this test (0 < 0 is false, 0 > 0 is false)
    // and should be handled by the clipper if necessary

    return (RLSW.cullFace == SW_FRONT)? (sgnArea < 0.0f) : (sgnArea > 0.0f); // Cull if winding is "clockwise" : "counter-clockwise"
}

static void sw_quad_clip_and_project(void)
{
    sw_vertex_t *polygon = RLSW.primitive.buffer;
    int *vertexCounter = &RLSW.primitive.vertexCount;

    if (sw_polygon_clip(polygon, vertexCounter))
    {
        // Transformation to screen space and normalization
        for (int i = 0; i < *vertexCounter; i++)
        {
            sw_vertex_t *v = &polygon[i];

            // Calculation of the reciprocal of W for normalization
            // as well as perspective-correct attributes
            const float wRcp = 1.0f/v->position[3];

            // Division of XYZ coordinates by weight
            v->position[0] *= wRcp;
            v->position[1] *= wRcp;
            v->position[2] *= wRcp;
            v->position[3] = wRcp;

            // Division of texture coordinates (perspective-correct)
            v->texcoord[0] *= wRcp;
            v->texcoord[1] *= wRcp;

            // Division of colors (perspective-correct)
            v->color[0] *= wRcp;
            v->color[1] *= wRcp;
            v->color[2] *= wRcp;
            v->color[3] *= wRcp;

            // Transformation to screen space
            sw_project_ndc_to_screen(v->position);
        }
    }
}

static bool sw_quad_is_axis_aligned(void)
{
    // Reject quads with perspective projection
    // The fast path assumes affine (non-perspective) quads,
    // so it's required for all vertices to have homogeneous w = 1.0
    for (int i = 0; i < 4; i++)
    {
        if (RLSW.primitive.buffer[i].position[3] != 1.0f) return false;
    }

    // Epsilon tolerance in screen space (pixels)
    const float epsilon = 0.5f;

    // Fetch screen-space positions for the four quad vertices
    const float *p0 = RLSW.primitive.buffer[0].position;
    const float *p1 = RLSW.primitive.buffer[1].position;
    const float *p2 = RLSW.primitive.buffer[2].position;
    const float *p3 = RLSW.primitive.buffer[3].position;

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

static void sw_quad_render(uint32_t state)
{
    if (RLSW.userState & SW_STATE_CULL_FACE)
    {
        if (!sw_quad_face_culling()) return;
    }

    sw_quad_clip_and_project();
    if (RLSW.primitive.vertexCount < 3) return;

    state &= SW_RASTER_QUAD_STATE_MASK;

    if ((RLSW.primitive.vertexCount == 4) && sw_quad_is_axis_aligned())
    {
        SW_RASTER_QUAD_TABLE[state](
            &RLSW.primitive.buffer[0],
            &RLSW.primitive.buffer[1],
            &RLSW.primitive.buffer[2],
            &RLSW.primitive.buffer[3]
        );
    }
    else
    {
        for (int i = 0; i < RLSW.primitive.vertexCount - 2; i++)
        {
            SW_RASTER_TRIANGLE_TABLE[state](
                &RLSW.primitive.buffer[0],
                &RLSW.primitive.buffer[i + 1],
                &RLSW.primitive.buffer[i + 2]
            );
        }
    }
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

static bool sw_line_clip(sw_vertex_t *v0, sw_vertex_t *v1)
{
    float t0 = 0.0f, t1 = 1.0f;
    float dH[4] = { 0 };
    float dC[4] = { 0 };

    for (int i = 0; i < 4; i++)
    {
        dH[i] = v1->position[i] - v0->position[i];
        dC[i] = v1->color[i] - v0->color[i];
    }

    // Clipping Liang-Barsky
    if (!sw_line_clip_coord(v0->position[3] - v0->position[0], -dH[3] + dH[0], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->position[3] + v0->position[0], -dH[3] - dH[0], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->position[3] - v0->position[1], -dH[3] + dH[1], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->position[3] + v0->position[1], -dH[3] - dH[1], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->position[3] - v0->position[2], -dH[3] + dH[2], &t0, &t1)) return false;
    if (!sw_line_clip_coord(v0->position[3] + v0->position[2], -dH[3] - dH[2], &t0, &t1)) return false;

    // Clipping Scissor
    if (RLSW.userState & SW_STATE_SCISSOR_TEST)
    {
        if (!sw_line_clip_coord(v0->position[0] - RLSW.scClipMin[0]*v0->position[3], RLSW.scClipMin[0]*dH[3] - dH[0], &t0, &t1)) return false;
        if (!sw_line_clip_coord(RLSW.scClipMax[0]*v0->position[3] - v0->position[0], dH[0] - RLSW.scClipMax[0]*dH[3], &t0, &t1)) return false;
        if (!sw_line_clip_coord(v0->position[1] - RLSW.scClipMin[1]*v0->position[3], RLSW.scClipMin[1]*dH[3] - dH[1], &t0, &t1)) return false;
        if (!sw_line_clip_coord(RLSW.scClipMax[1]*v0->position[3] - v0->position[1], dH[1] - RLSW.scClipMax[1]*dH[3], &t0, &t1)) return false;
    }

    // Interpolation of new coordinates
    if (t1 < 1.0f)
    {
        for (int i = 0; i < 4; i++)
        {
            v1->position[i] = v0->position[i] + t1*dH[i];
            v1->color[i] = v0->color[i] + t1*dC[i];
        }
    }

    if (t0 > 0.0f)
    {
        for (int i = 0; i < 4; i++)
        {
            v0->position[i] += t0*dH[i];
            v0->color[i] += t0*dC[i];
        }
    }

    return true;
}

static bool sw_line_clip_and_project(sw_vertex_t *v0, sw_vertex_t *v1)
{
    if (!sw_line_clip(v0, v1)) return false;

    // Convert clip coordinates to NDC
    v0->position[3] = 1.0f/v0->position[3];
    v1->position[3] = 1.0f/v1->position[3];
    for (int i = 0; i < 3; i++)
    {
        v0->position[i] *= v0->position[3];
        v1->position[i] *= v1->position[3];
    }

    // Convert NDC coordinates to screen space
    sw_project_ndc_to_screen(v0->position);
    sw_project_ndc_to_screen(v1->position);

    // NDC +1.0 projects to exactly (width + 0.5f), which truncates out of bounds
    // The clamp is at most 0.5px on a boundary endpoint, it's visually imperceptible
    v0->position[0] = sw_clamp(v0->position[0], 0.0f, (float)(RLSW.colorBuffer->width  - 1) + 0.5f);
    v0->position[1] = sw_clamp(v0->position[1], 0.0f, (float)(RLSW.colorBuffer->height - 1) + 0.5f);
    v1->position[0] = sw_clamp(v1->position[0], 0.0f, (float)(RLSW.colorBuffer->width  - 1) + 0.5f);
    v1->position[1] = sw_clamp(v1->position[1], 0.0f, (float)(RLSW.colorBuffer->height - 1) + 0.5f);

    return true;
}

static void sw_line_render(uint32_t state, sw_vertex_t *vertices)
{
    if (!sw_line_clip_and_project(&vertices[0], &vertices[1])) return;

    state &= SW_RASTER_LINE_STATE_MASK;

    if (RLSW.lineWidth >= 2.0f)
    {
        SW_RASTER_LINE_THICK_TABLE[state](&vertices[0], &vertices[1]);
    }
    else
    {
        SW_RASTER_LINE_TABLE[state](&vertices[0], &vertices[1]);
    }
}
//-------------------------------------------------------------------------------------------

// Point rendering logic
//-------------------------------------------------------------------------------------------
static bool sw_point_clip_and_project(sw_vertex_t *v)
{
    if (v->position[3] != 1.0f)
    {
        for (int_fast8_t i = 0; i < 3; i++)
        {
            if ((v->position[i] < -v->position[3]) || (v->position[i] > v->position[3])) return false;
        }

        v->position[3] = 1.0f/v->position[3];
        v->position[0] *= v->position[3];
        v->position[1] *= v->position[3];
        v->position[2] *= v->position[3];
    }

    sw_project_ndc_to_screen(v->position);

    int min[2] = { 0, 0 };
    int max[2] = { RLSW.colorBuffer->width, RLSW.colorBuffer->height };

    if (RLSW.userState & SW_STATE_SCISSOR_TEST)
    {
        min[0] = sw_clamp_int(RLSW.scMin[0], 0, RLSW.colorBuffer->width);
        min[1] = sw_clamp_int(RLSW.scMin[1], 0, RLSW.colorBuffer->height);
        max[0] = sw_clamp_int(RLSW.scMax[0], 0, RLSW.colorBuffer->width);
        max[1] = sw_clamp_int(RLSW.scMax[1], 0, RLSW.colorBuffer->height);
    }

    bool insideX = (v->position[0] - RLSW.pointRadius < max[0]) && (v->position[0] + RLSW.pointRadius > min[0]);
    bool insideY = (v->position[1] - RLSW.pointRadius < max[1]) && (v->position[1] + RLSW.pointRadius > min[1]);

    return (insideX && insideY);
}

static void sw_point_render(uint32_t state, sw_vertex_t *v)
{
    if (!sw_point_clip_and_project(v)) return;
    state &= SW_RASTER_POINT_STATE_MASK;
    SW_RASTER_POINT_TABLE[state](v);
}
//-------------------------------------------------------------------------------------------

// Polygon modes rendering logic
//-------------------------------------------------------------------------------------------
static inline void sw_poly_point_render(uint32_t state)
{
    for (int i = 0; i < RLSW.primitive.vertexCount; i++) sw_point_render(state, &RLSW.primitive.buffer[i]);
}

static inline void sw_poly_line_render(uint32_t state)
{
    const sw_vertex_t *vertices = RLSW.primitive.buffer;
    int cm1 = RLSW.primitive.vertexCount - 1;

    for (int i = 0; i < cm1; i++)
    {
        sw_vertex_t verts[2] = { vertices[i], vertices[i + 1] };
        sw_line_render(state, verts);
    }

    sw_vertex_t verts[2] = { vertices[cm1], vertices[0] };
    sw_line_render(state, verts);
}

static inline void sw_poly_fill_render(uint32_t state)
{
    switch (RLSW.drawMode)
    {
        case SW_POINTS: sw_point_render(state, &RLSW.primitive.buffer[0]); break;
        case SW_LINES: sw_line_render(state, RLSW.primitive.buffer); break;
        case SW_TRIANGLES: sw_triangle_render(state); break;
        case SW_QUADS: sw_quad_render(state); break;
        default: break;
    }
}
//-------------------------------------------------------------------------------------------

// Immediate rendering logic
//-------------------------------------------------------------------------------------------
static void sw_immediate_begin(SWdraw mode)
{
    // NOTE: Any checks to ensure command recording can start
    //       must be performed before calling this function.

    // Recalculate the MVP if this is needed
    if (RLSW.isDirtyMVP)
    {
        sw_matrix_mul_rst(RLSW.matMVP,
            RLSW.stackModelview[RLSW.stackModelviewCounter - 1],
            RLSW.stackProjection[RLSW.stackProjectionCounter - 1]);

        RLSW.isDirtyMVP = false;
    }

    // Disable pipeline states that are incompatible with the global state
    uint32_t state = RLSW.userState;
    if (!sw_is_texture_complete(RLSW.depthBuffer)) state &= ~SW_STATE_DEPTH_TEST;
    if (!sw_is_texture_complete(RLSW.boundTexture)) state &= ~SW_STATE_TEXTURE_2D;
    else if (sw_pixel_is_depth_format(RLSW.boundTexture->format)) state &= ~SW_STATE_TEXTURE_2D;

    // Initialize required values
    RLSW.primitive.hasColorAlpha = false;
    RLSW.primitive.vertexCount = 0;
    RLSW.rasterState = state;
    RLSW.drawMode = mode;
}

static bool sw_immediate_is_active(void)
{
    return (RLSW.drawMode != SW_DRAW_INVALID);
}

static void sw_immediate_set_color(const float color[4])
{
    RLSW.primitive.color[0] = color[0];
    RLSW.primitive.color[1] = color[1];
    RLSW.primitive.color[2] = color[2];
    RLSW.primitive.color[3] = color[3];

    RLSW.primitive.hasColorAlpha |= (color[3] < 1.0f);
}

static void sw_immediate_set_texcoord(const float texcoord[2])
{
    const float *m = RLSW.stackTexture[RLSW.stackTextureCounter - 1];
    RLSW.primitive.texcoord[0] = m[0]*texcoord[0] + m[4]*texcoord[1] + m[12];
    RLSW.primitive.texcoord[1] = m[1]*texcoord[0] + m[5]*texcoord[1] + m[13];
}

static void sw_immediate_push_vertex(const float position[4])
{
    // Check if the draw mode is valid
    if (!sw_is_draw_mode_valid(RLSW.drawMode))
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    // Gets the current vertex
    sw_vertex_t *vertex = &RLSW.primitive.buffer[RLSW.primitive.vertexCount++];

    // Calculate clip coordinates
    const float *m = RLSW.matMVP;
    vertex->position[0] = m[0]*position[0] + m[4]*position[1] + m[8]*position[2] + m[12]*position[3];
    vertex->position[1] = m[1]*position[0] + m[5]*position[1] + m[9]*position[2] + m[13]*position[3];
    vertex->position[2] = m[2]*position[0] + m[6]*position[1] + m[10]*position[2] + m[14]*position[3];
    vertex->position[3] = m[3]*position[0] + m[7]*position[1] + m[11]*position[2] + m[15]*position[3];

    // Copy the attributes in the current vertex
    for (int i = 0; i < 4; i++) vertex->color[i] = RLSW.primitive.color[i];
    for (int i = 0; i < 2; i++) vertex->texcoord[i] = RLSW.primitive.texcoord[i];

    // Immediate rendering of the primitive if the required number is reached
    if (RLSW.primitive.vertexCount == SW_PRIMITIVE_VERTEX_COUNT[RLSW.drawMode])
    {
        uint32_t state = RLSW.rasterState;

        // Reduces blend mode costs when it's possible
        if (state & SW_STATE_BLEND)
        {
            if (RLSW.blendFlags & SW_BLEND_FLAG_NOOP) state &= ~SW_STATE_BLEND;
            else if ((RLSW.blendFlags & SW_BLEND_FLAG_NEEDS_ALPHA) && (!RLSW.primitive.hasColorAlpha))
            {
                if (!(state & SW_STATE_TEXTURE_2D) || (RLSW.boundTexture->alpha == SW_PIXEL_ALPHA_NONE)) state &= ~SW_STATE_BLEND;
            }
        }

        switch (RLSW.polyMode)
        {
            case SW_FILL: sw_poly_fill_render(state); break;
            case SW_LINE: sw_poly_line_render(state); break;
            case SW_POINT: sw_poly_point_render(state); break;
            default: break;
        }

        RLSW.primitive.hasColorAlpha = false;
        RLSW.primitive.vertexCount = 0;
    }
}

static void sw_immediate_end(void)
{
    RLSW.drawMode = SW_DRAW_INVALID;
}
//-------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
bool swInit(int w, int h)
{
    if (!sw_default_framebuffer_alloc(&RLSW.framebuffer, w, h))
    {
        swClose();
        return false;
    }

    if (!sw_pool_init(&RLSW.framebufferPool, SW_MAX_FRAMEBUFFERS, sizeof(sw_framebuffer_t)))
    {
        swClose();
        return false;
    }

    if (!sw_pool_init(&RLSW.texturePool, SW_MAX_TEXTURES, sizeof(sw_texture_t)))
    {
        swClose();
        return false;
    }

    RLSW.colorBuffer = &RLSW.framebuffer.color;
    RLSW.depthBuffer = &RLSW.framebuffer.depth;

    swViewport(0, 0, w, h);
    swScissor(0, 0, w, h);

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

    RLSW.primitive.texcoord[0] = 0.0f;
    RLSW.primitive.texcoord[1] = 0.0f;

    RLSW.primitive.color[0] = 1.0f;
    RLSW.primitive.color[1] = 1.0f;
    RLSW.primitive.color[2] = 1.0f;
    RLSW.primitive.color[3] = 1.0f;

    RLSW.srcFactor = SW_SRC_ALPHA;
    RLSW.dstFactor = SW_ONE_MINUS_SRC_ALPHA;
    RLSW.blendFunc = sw_blend_SRC_ALPHA_ONE_MINUS_SRC_ALPHA;

    RLSW.drawMode = SW_DRAW_INVALID;
    RLSW.polyMode = SW_FILL;
    RLSW.cullFace = SW_BACK;

#if SW_USE_COLOR_LUT
    for (int i = 0; i < 256; i++) SW_LUT_UINT8_TO_FLOAT[i] = (float)i*SW_INV_255;
#endif

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
    for (int i = 1; i < RLSW.texturePool.watermark; i++)
    {
        if (RLSW.texturePool.gen[i] & SW_POOL_SLOT_LIVE)
        {
            sw_texture_free((sw_texture_t *)RLSW.texturePool.data + i);
        }
    }

    sw_pool_destroy(&RLSW.texturePool);
    sw_pool_destroy(&RLSW.framebufferPool);
    sw_default_framebuffer_free(&RLSW.framebuffer);

    RLSW = SW_CURLY_INIT(sw_context_t) { 0 };
}

bool swResize(int w, int h)
{
    return sw_default_framebuffer_alloc(&RLSW.framebuffer, w, h);
}

void swReadPixels(int x, int y, int w, int h, SWformat format, SWtype type, void *pixels)
{
    // REVIEW: Handle depth buffer copy here or consider it as an error ?
    if (format == SW_DEPTH_COMPONENT) { RLSW.errCode = SW_INVALID_ENUM; return; }

    sw_pixelformat_t pFormat = (sw_pixelformat_t)sw_pixel_get_format(format, type);
    if (pFormat <= SW_PIXELFORMAT_UNKNOWN) { RLSW.errCode = SW_INVALID_ENUM; return; }

    if (w <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }
    if (h <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }

    if (w > RLSW.colorBuffer->width) w = RLSW.colorBuffer->width;
    if (h > RLSW.colorBuffer->height) h = RLSW.colorBuffer->height;

    x = sw_clamp_int(x, 0, w);
    y = sw_clamp_int(y, 0, h);

    if ((x >= w) || (y >= h)) return;

    if ((pFormat == SW_FRAMEBUFFER_COLOR_FORMAT) && (x == 0) && (y == 0) && (w == RLSW.colorBuffer->width) && (h == RLSW.colorBuffer->height))
    {
        sw_framebuffer_output_fast(pixels, RLSW.colorBuffer);
    }
    else
    {
        sw_framebuffer_output_copy(pixels, RLSW.colorBuffer, x, y, w, h, pFormat);
    }
}

void swBlitPixels(int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, SWformat format, SWtype type, void *pixels)
{
    // REVIEW: Handle depth buffer copy here or consider it as an error ?
    if (format == SW_DEPTH_COMPONENT) { RLSW.errCode = SW_INVALID_ENUM; return; }

    sw_pixelformat_t pFormat = (sw_pixelformat_t)sw_pixel_get_format(format, type);
    if (pFormat <= SW_PIXELFORMAT_UNKNOWN) { RLSW.errCode = SW_INVALID_ENUM; return; }

    if (wSrc <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }
    if (hSrc <= 0) { RLSW.errCode = SW_INVALID_VALUE; return; }

    if (wSrc > RLSW.colorBuffer->width) wSrc = RLSW.colorBuffer->width;
    if (hSrc > RLSW.colorBuffer->height) hSrc = RLSW.colorBuffer->height;

    xSrc = sw_clamp_int(xSrc, 0, wSrc);
    ySrc = sw_clamp_int(ySrc, 0, hSrc);

    // Check if the sizes are identical after clamping the source to avoid unexpected issues
    // TODO: REVIEW: This repeats the operations if true, so a copy function can be made without these checks
    if ((xDst == xSrc) && (yDst == ySrc) && (wDst == wSrc) && (hDst == hSrc))
    {
        swReadPixels(xSrc, ySrc, wSrc, hSrc, format, type, pixels);
    }
    else
    {
        sw_framebuffer_output_blit(pixels, RLSW.colorBuffer, xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pFormat);
    }
}

// Get framefuffer pixel data pointer and size
void *swGetColorBuffer(int *width, int *height)
{
    if (width != NULL) *width = RLSW.framebuffer.color.width;
    if (height != NULL) *height = RLSW.framebuffer.color.height;
    return RLSW.framebuffer.color.pixels;
}

void swEnable(SWstate state)
{
    switch (state)
    {
        case SW_SCISSOR_TEST: RLSW.userState |= SW_STATE_SCISSOR_TEST; break;
        case SW_TEXTURE_2D: RLSW.userState |= SW_STATE_TEXTURE_2D; break;
        case SW_DEPTH_TEST: RLSW.userState |= SW_STATE_DEPTH_TEST; break;
        case SW_CULL_FACE: RLSW.userState |= SW_STATE_CULL_FACE; break;
        case SW_BLEND: RLSW.userState |= SW_STATE_BLEND; break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swDisable(SWstate state)
{
    switch (state)
    {
        case SW_SCISSOR_TEST: RLSW.userState &= ~SW_STATE_SCISSOR_TEST; break;
        case SW_TEXTURE_2D: RLSW.userState &= ~SW_STATE_TEXTURE_2D; break;
        case SW_DEPTH_TEST: RLSW.userState &= ~SW_STATE_DEPTH_TEST; break;
        case SW_CULL_FACE: RLSW.userState &= ~SW_STATE_CULL_FACE; break;
        case SW_BLEND: RLSW.userState &= ~SW_STATE_BLEND; break;
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
        case SW_DRAW_FRAMEBUFFER_BINDING: *v = RLSW.boundFramebufferId; break;
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
        case SW_DEPTH_CLEAR_VALUE:
        {
            v[0] = RLSW.clearDepth;
        } break;
        case SW_CURRENT_COLOR:
        {
            v[0] = RLSW.primitive.buffer[RLSW.primitive.vertexCount - 1].color[0];
            v[1] = RLSW.primitive.buffer[RLSW.primitive.vertexCount - 1].color[1];
            v[2] = RLSW.primitive.buffer[RLSW.primitive.vertexCount - 1].color[2];
            v[3] = RLSW.primitive.buffer[RLSW.primitive.vertexCount - 1].color[3];
        } break;
        case SW_CURRENT_TEXTURE_COORDS:
        {
            v[0] = RLSW.primitive.buffer[RLSW.primitive.vertexCount - 1].texcoord[0];
            v[1] = RLSW.primitive.buffer[RLSW.primitive.vertexCount - 1].texcoord[1];
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
        case SW_VENDOR: result = "RLSW"; break;
        case SW_RENDERER: result = "RLSW OpenGL Software Renderer"; break;
        case SW_VERSION: result = RLSW_VERSION; break;
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
}

void swScissor(int x, int y, int width, int height)
{
    if ((width < 0) || (height < 0))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.scMin[0] = x;
    RLSW.scMin[1] = y;
    RLSW.scMax[0] = x + width;
    RLSW.scMax[1] = y + height;

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

void swClearDepth(float depth)
{
    RLSW.clearDepth = depth;
}

void swClear(uint32_t bitmask)
{
    if (!sw_is_ready_to_render()) return;

    if ((bitmask & (SW_COLOR_BUFFER_BIT)) && (RLSW.colorBuffer != NULL) && (RLSW.colorBuffer->pixels != NULL))
    {
        sw_framebuffer_fill_color(RLSW.colorBuffer, RLSW.clearColor);
    }

    if ((bitmask & (SW_DEPTH_BUFFER_BIT)) && (RLSW.depthBuffer != NULL) && (RLSW.depthBuffer->pixels != NULL))
    {
        sw_framebuffer_fill_depth(RLSW.depthBuffer, RLSW.clearDepth);
    }
}

void swBlendFunc(SWfactor sfactor, SWfactor dfactor)
{
    int sIndex = sw_blend_factor_index(sfactor);
    if (sIndex < 0) { RLSW.errCode = SW_INVALID_ENUM; return; }

    int dIndex = sw_blend_factor_index(dfactor);
    if (dIndex < 0) { RLSW.errCode = SW_INVALID_ENUM; return; }

    RLSW.srcFactor = sfactor;
    RLSW.dstFactor = dfactor;
    RLSW.blendFlags = sw_blend_compute_flags(sfactor, dfactor);
    RLSW.blendFunc = SW_BLEND_TABLE[sIndex][dIndex];
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

            RLSW.currentMatrix = &RLSW.stackProjection[(--RLSW.stackProjectionCounter) - 1];
            RLSW.isDirtyMVP = true; //< The MVP is considered to have been changed
        } break;
        case SW_MODELVIEW:
        {
            if (RLSW.stackModelviewCounter <= 0)
            {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackModelview[(--RLSW.stackModelviewCounter) - 1];
            RLSW.isDirtyMVP = true; //< The MVP is considered to have been changed
        } break;
        case SW_TEXTURE:
        {
            if (RLSW.stackTextureCounter <= 0)
            {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackTexture[(--RLSW.stackTextureCounter) - 1];
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
    sw_matrix_t mat = { 0 };
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

    sw_matrix_t mat = { 0 };

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
    sw_matrix_t mat = { 0 };

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
    sw_matrix_t mat = { 0 };

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
    sw_matrix_t mat = { 0 };

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
    // Check if render is possible
    if (!sw_is_ready_to_render())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    // Check if the draw mode is valid
    if (!sw_is_draw_mode_valid(mode))
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    // Ensures that a recording has not already started (spec)
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    sw_immediate_begin(mode);
}

void swEnd(void)
{
    // Ensures that a recording has already started (spec)
    if (!sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    sw_immediate_end();
}

void swVertex2i(int x, int y)
{
    const float v[4] = { (float)x, (float)y, 0.0f, 1.0f };
    sw_immediate_push_vertex(v);
}

void swVertex2f(float x, float y)
{
    const float v[4] = { x, y, 0.0f, 1.0f };
    sw_immediate_push_vertex(v);
}

void swVertex2fv(const float *v)
{
    const float v4[4] = { v[0], v[1], 0.0f, 1.0f };
    sw_immediate_push_vertex(v4);
}

void swVertex3i(int x, int y, int z)
{
    const float v[4] = { (float)x, (float)y, (float)z, 1.0f };
    sw_immediate_push_vertex(v);
}

void swVertex3f(float x, float y, float z)
{
    const float v[4] = { x, y, z, 1.0f };
    sw_immediate_push_vertex(v);
}

void swVertex3fv(const float *v)
{
    const float v4[4] = { v[0], v[1], v[2], 1.0f };
    sw_immediate_push_vertex(v4);
}

void swVertex4i(int x, int y, int z, int w)
{
    const float v[4] = { (float)x, (float)y, (float)z, (float)w };
    sw_immediate_push_vertex(v);
}

void swVertex4f(float x, float y, float z, float w)
{
    const float v[4] = { x, y, z, w };
    sw_immediate_push_vertex(v);
}

void swVertex4fv(const float *v)
{
    sw_immediate_push_vertex(v);
}

void swColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
    float cv[4] = { 0 };
    cv[0] = (float)r*SW_INV_255;
    cv[1] = (float)g*SW_INV_255;
    cv[2] = (float)b*SW_INV_255;
    cv[3] = 1.0f;

    sw_immediate_set_color(cv);
}

void swColor3ubv(const uint8_t *v)
{
    float cv[4] = { 0 };
    cv[0] = (float)v[0]*SW_INV_255;
    cv[1] = (float)v[1]*SW_INV_255;
    cv[2] = (float)v[2]*SW_INV_255;
    cv[3] = 1.0f;

    sw_immediate_set_color(cv);
}

void swColor3f(float r, float g, float b)
{
    float cv[4] = { 0 };
    cv[0] = r;
    cv[1] = g;
    cv[2] = b;
    cv[3] = 1.0f;

    sw_immediate_set_color(cv);
}

void swColor3fv(const float *v)
{
    float cv[4] = { 0 };
    cv[0] = v[0];
    cv[1] = v[1];
    cv[2] = v[2];
    cv[3] = 1.0f;

    sw_immediate_set_color(cv);
}

void swColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    float cv[4] = { 0 };
    cv[0] = (float)r*SW_INV_255;
    cv[1] = (float)g*SW_INV_255;
    cv[2] = (float)b*SW_INV_255;
    cv[3] = (float)a*SW_INV_255;

    sw_immediate_set_color(cv);
}

void swColor4ubv(const uint8_t *v)
{
    float cv[4] = { 0 };
    cv[0] = (float)v[0]*SW_INV_255;
    cv[1] = (float)v[1]*SW_INV_255;
    cv[2] = (float)v[2]*SW_INV_255;
    cv[3] = (float)v[3]*SW_INV_255;

    sw_immediate_set_color(cv);
}

void swColor4f(float r, float g, float b, float a)
{
    float cv[4] = { 0 };
    cv[0] = r;
    cv[1] = g;
    cv[2] = b;
    cv[3] = a;

    sw_immediate_set_color(cv);
}

void swColor4fv(const float *v)
{
    sw_immediate_set_color(v);
}

void swTexCoord2f(float u, float v)
{
    const float texcoord[2] = { u, v };
    sw_immediate_set_texcoord(texcoord);
}

void swTexCoord2fv(const float *v)
{
    sw_immediate_set_texcoord(v);
}

void swBindArray(SWarray type, void *buffer)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

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
    if ((sw_immediate_is_active()) || (!sw_is_ready_to_render()) || (RLSW.array.positions == NULL))
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    sw_immediate_begin(mode);
    {
        const float *positions = RLSW.array.positions;
        const float *texcoords = RLSW.array.texcoords;
        const uint8_t *colors = RLSW.array.colors;

        int end = offset + count;
        for (int i = offset; i < end; i++)
        {
            if (texcoords) sw_immediate_set_texcoord(&texcoords[2*i]);

            if (colors)
            {
                const uint8_t *c = &colors[4*i];
                float color[4] = {
                    (float)c[0]*SW_INV_255,
                    (float)c[1]*SW_INV_255,
                    (float)c[2]*SW_INV_255,
                    (float)c[3]*SW_INV_255,
                };
                sw_immediate_set_color(color);
            }

            const float *p = &positions[3*i];
            float position[4] = { p[0], p[1], p[2], 1.0f };
            sw_immediate_push_vertex(position);
        }
    }
    sw_immediate_end();
}

void swDrawElements(SWdraw mode, int count, int type, const void *indices)
{
    if ((sw_immediate_is_active()) || (!sw_is_ready_to_render()) || (RLSW.array.positions == NULL))
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if ((count < 0) || (indices == NULL))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if ((type != SW_UNSIGNED_BYTE) && (type != SW_UNSIGNED_SHORT) && (type != SW_UNSIGNED_INT))
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    sw_immediate_begin(mode);
    {
        const float *positions = RLSW.array.positions;
        const float *texcoords = RLSW.array.texcoords;
        const uint8_t *colors = RLSW.array.colors;

        const uint8_t *indicesUb = (type == SW_UNSIGNED_BYTE)? indices : NULL;
        const uint16_t *indicesUs = (type == SW_UNSIGNED_SHORT)? indices : NULL;
        const uint32_t *indicesUi = (type == SW_UNSIGNED_INT)? indices : NULL;

        for (int i = 0; i < count; i++)
        {
            uint32_t index = indicesUb? (uint32_t)indicesUb[i] : (indicesUs? (uint32_t)indicesUs[i] : (uint32_t)indicesUi[i]);

            if (texcoords) sw_immediate_set_texcoord(&texcoords[2*index]);

            if (colors)
            {
                const uint8_t *c = &colors[4*index];
                float color[4] = {
                    (float)c[0]*SW_INV_255,
                    (float)c[1]*SW_INV_255,
                    (float)c[2]*SW_INV_255,
                    (float)c[3]*SW_INV_255,
                };
                sw_immediate_set_color(color);
            }

            const float *p = &positions[3*index];
            float position[4] = { p[0], p[1], p[2], 1.0f };
            sw_immediate_push_vertex(position);
        }
    }
    sw_immediate_end();
}

void swGenTextures(int count, sw_handle_t *textures)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (!count || !textures) return;

    for (int i = 0; i < count; i++)
    {
        sw_handle_t h = sw_pool_alloc(&RLSW.texturePool);
        if (h == SW_HANDLE_NULL) { RLSW.errCode = SW_OUT_OF_MEMORY; return; }
        textures[i] = h;
    }
}

void swDeleteTextures(int count, sw_handle_t *textures)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (!count || !textures) return;

    for (int i = 0; i < count; i++)
    {
        sw_texture_t *tex = sw_pool_get(&RLSW.texturePool, textures[i]);
        if (!tex) { RLSW.errCode = SW_INVALID_VALUE; continue; }
        if (tex == RLSW.boundTexture) RLSW.boundTexture = NULL;
        if (tex == RLSW.colorBuffer) RLSW.colorBuffer = NULL;
        if (tex == RLSW.depthBuffer) RLSW.depthBuffer = NULL;

        sw_texture_free(tex);
        *tex = (sw_texture_t) { 0 };

        sw_pool_free(&RLSW.texturePool, textures[i]);
    }
}

void swBindTexture(sw_handle_t id)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (id == SW_HANDLE_NULL)
    {
        RLSW.boundTexture = NULL;
        return;
    }

    if (!sw_is_texture_valid(id))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.boundTexture = sw_pool_get(&RLSW.texturePool, id);
}

void swTexStorage2D(int width, int height, SWinternalformat format)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (RLSW.boundTexture == NULL) return;

    int pixelFormat = SW_PIXELFORMAT_UNKNOWN;
    switch (format)
    {
        case SW_LUMINANCE8: pixelFormat = SW_PIXELFORMAT_COLOR_GRAYSCALE; break;
        case SW_LUMINANCE8_ALPHA8: pixelFormat = SW_PIXELFORMAT_COLOR_GRAYALPHA; break;
        case SW_R3_G3_B2: pixelFormat = SW_PIXELFORMAT_COLOR_R3G3B2; break;
        case SW_RGB8: pixelFormat = SW_PIXELFORMAT_COLOR_R8G8B8; break;
        case SW_RGBA4: pixelFormat = SW_PIXELFORMAT_COLOR_R4G4B4A4; break;
        case SW_RGB5_A1: pixelFormat = SW_PIXELFORMAT_COLOR_R5G5B5A1; break;
        case SW_RGBA8: pixelFormat = SW_PIXELFORMAT_COLOR_R8G8B8A8; break;
        case SW_R16F: pixelFormat = SW_PIXELFORMAT_COLOR_R16; break;
        case SW_RGB16F: pixelFormat = SW_PIXELFORMAT_COLOR_R16G16B16; break;
        case SW_RGBA16F: pixelFormat = SW_PIXELFORMAT_COLOR_R16G16B16A16; break;
        case SW_R32F: pixelFormat = SW_PIXELFORMAT_COLOR_R32; break;
        case SW_RGB32F: pixelFormat = SW_PIXELFORMAT_COLOR_R32G32B32; break;
        case SW_RGBA32F: pixelFormat = SW_PIXELFORMAT_COLOR_R32G32B32A32; break;
        case SW_DEPTH_COMPONENT16: pixelFormat = SW_PIXELFORMAT_DEPTH_D16; break;
        case SW_DEPTH_COMPONENT24: pixelFormat = SW_PIXELFORMAT_DEPTH_D32; break;
        case SW_DEPTH_COMPONENT32: pixelFormat = SW_PIXELFORMAT_DEPTH_D32; break;
        case SW_DEPTH_COMPONENT32F: pixelFormat = SW_PIXELFORMAT_DEPTH_D32; break;
        default: RLSW.errCode = SW_INVALID_ENUM; return;
    }

    (void)sw_texture_alloc(RLSW.boundTexture, NULL, width, height, pixelFormat);
}

void swTexImage2D(int width, int height, SWformat format, SWtype type, const void *data)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (RLSW.boundTexture == NULL) return;

    int pixelFormat = sw_pixel_get_format(format, type);
    if (pixelFormat <= SW_PIXELFORMAT_UNKNOWN) { RLSW.errCode = SW_INVALID_ENUM; return; }

    (void)sw_texture_alloc(RLSW.boundTexture, data, width, height, pixelFormat);
}

void swTexSubImage2D(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (!sw_is_texture_complete(RLSW.boundTexture) || (!pixels) || (width <= 0) || (height <= 0))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if ((x < 0) || (y < 0) || (x + width > RLSW.boundTexture->width) || (y + height > RLSW.boundTexture->height))
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    sw_pixel_write_color_f writeColor = sw_pixel_get_write_color_func(RLSW.boundTexture->format);
    if (writeColor == NULL) // probably a depth format
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    sw_pixelformat_t srcPixelFormat = sw_pixel_get_format(format, type);
    sw_pixel_read_color_f readColor = sw_pixel_get_read_color_func(srcPixelFormat);
    if (readColor == NULL)
    {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    const int srcPixelSize = SW_PIXELFORMAT_SIZE[srcPixelFormat];
    const int dstPixelSize = SW_PIXELFORMAT_SIZE[RLSW.boundTexture->format];

    const uint8_t *srcBytes = (const uint8_t *)pixels;
    uint8_t *dstBytes = (uint8_t *)RLSW.boundTexture->pixels;

    if (srcPixelFormat == RLSW.boundTexture->format)
    {
        const int rowSize = width*dstPixelSize;
        const int dstStride = RLSW.boundTexture->width*dstPixelSize;
        const int srcStride = width*srcPixelSize;

        uint8_t *dstRow = dstBytes + (y*RLSW.boundTexture->width + x)*dstPixelSize;
        const uint8_t *srcRow = srcBytes;

        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < rowSize; ++i) dstRow[i] = srcRow[i];
            dstRow += dstStride;
            srcRow += srcStride;
        }
        return;
    }

    bool alphaFound = false;

    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            float color[4];
            const int srcPixelOffset = ((j*width) + i)*srcPixelSize;
            const int dstPixelOffset = (((y + j)*RLSW.boundTexture->width) + (x + i))*dstPixelSize;

            readColor(color, srcBytes, srcPixelOffset);
            alphaFound |= (color[3] < 1.0f);

            writeColor(dstBytes, color, dstPixelOffset);
        }
    }

    RLSW.boundTexture->alpha = alphaFound? SW_PIXELFORMAT_ALPHA[srcPixelFormat] : SW_PIXEL_ALPHA_NONE;
}

void swTexParameteri(int param, int value)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (RLSW.boundTexture == NULL) return;

    switch (param)
    {
        case SW_TEXTURE_MIN_FILTER:
        {
            if (!sw_is_texture_filter_valid(value)) { RLSW.errCode = SW_INVALID_ENUM; return; }
            RLSW.boundTexture->minFilter = (SWfilter)value;
        } break;
        case SW_TEXTURE_MAG_FILTER:
        {
            if (!sw_is_texture_filter_valid(value)) { RLSW.errCode = SW_INVALID_ENUM; return; }
            RLSW.boundTexture->magFilter = (SWfilter)value;
        } break;
        case SW_TEXTURE_WRAP_S:
        {
            if (!sw_is_texture_wrap_valid(value)) { RLSW.errCode = SW_INVALID_ENUM; return; }
            RLSW.boundTexture->sWrap = (SWwrap)value;
        } break;
        case SW_TEXTURE_WRAP_T:
        {
            if (!sw_is_texture_wrap_valid(value)) { RLSW.errCode = SW_INVALID_ENUM; return; }
            RLSW.boundTexture->tWrap = (SWwrap)value;
        } break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

void swGenFramebuffers(int count, uint32_t *framebuffers)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (!count || !framebuffers) return;

    for (int i = 0; i < count; i++)
    {
        sw_handle_t h = sw_pool_alloc(&RLSW.framebufferPool);
        if (h == SW_HANDLE_NULL) { RLSW.errCode = SW_OUT_OF_MEMORY; return; }
        framebuffers[i] = h;
    }
}

void swDeleteFramebuffers(int count, uint32_t *framebuffers)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (!count || !framebuffers) return;

    for (int i = 0; i < count; i++)
    {
        sw_framebuffer_t *fb = sw_pool_get(&RLSW.framebufferPool, framebuffers[i]);
        if (!fb) { RLSW.errCode = SW_INVALID_VALUE; continue; }

        if (framebuffers[i] == RLSW.boundFramebufferId)
        {
            RLSW.boundFramebufferId = SW_HANDLE_NULL;
            RLSW.colorBuffer = &RLSW.framebuffer.color;
            RLSW.depthBuffer = &RLSW.framebuffer.depth;
        }

        sw_pool_free(&RLSW.framebufferPool, framebuffers[i]);
    }
}

void swBindFramebuffer(uint32_t id)
{
    if (sw_immediate_is_active())
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (id == SW_HANDLE_NULL)
    {
        RLSW.boundFramebufferId = SW_HANDLE_NULL;
        RLSW.colorBuffer = &RLSW.framebuffer.color;
        RLSW.depthBuffer = &RLSW.framebuffer.depth;
        return;
    }

    sw_framebuffer_t *fb = sw_pool_get(&RLSW.framebufferPool, id);
    if (fb == NULL)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.boundFramebufferId = id;
    RLSW.colorBuffer = sw_pool_get(&RLSW.texturePool, fb->colorAttachment);
    RLSW.depthBuffer = sw_pool_get(&RLSW.texturePool, fb->depthAttachment);
}

void swFramebufferTexture2D(SWattachment attach, uint32_t texture)
{
    if ((sw_immediate_is_active()) || (RLSW.boundFramebufferId == SW_HANDLE_NULL))
    {
        RLSW.errCode = SW_INVALID_OPERATION; // not really standard but hey
        return;
    }

    sw_framebuffer_t *fb = sw_pool_get(&RLSW.framebufferPool, RLSW.boundFramebufferId);
    if (fb == NULL) return; // Should never happen

    switch (attach)
    {
        case SW_COLOR_ATTACHMENT:
        {
            fb->colorAttachment = texture;
            RLSW.colorBuffer = sw_pool_get(&RLSW.texturePool, texture);
        } break;
        case SW_DEPTH_ATTACHMENT:
        {
            fb->depthAttachment = texture;
            RLSW.depthBuffer = sw_pool_get(&RLSW.texturePool, texture);
        } break;
        default: RLSW.errCode = SW_INVALID_ENUM; break;
    }
}

SWfbstatus swCheckFramebufferStatus(void)
{
    if (RLSW.boundFramebufferId == SW_HANDLE_NULL) return SW_FRAMEBUFFER_COMPLETE;

    if (RLSW.colorBuffer == NULL) return SW_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;

    if (!sw_is_texture_complete(RLSW.colorBuffer)) return SW_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

    if (RLSW.colorBuffer->format != SW_FRAMEBUFFER_COLOR_FORMAT) return SW_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

    if (RLSW.depthBuffer)
    {
        if (!sw_is_texture_complete(RLSW.depthBuffer)) return SW_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

        if (RLSW.depthBuffer->format != SW_FRAMEBUFFER_DEPTH_FORMAT) return SW_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

        if ((RLSW.colorBuffer->width != RLSW.depthBuffer->width) ||
            (RLSW.colorBuffer->height != RLSW.depthBuffer->height)) return SW_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;
    }

    return SW_FRAMEBUFFER_COMPLETE;
}

void swGetFramebufferAttachmentParameteriv(SWattachment attachment, SWattachget property, int *v)
{
    if (RLSW.boundFramebufferId == SW_HANDLE_NULL)
    {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    if (v == NULL)
    {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    sw_framebuffer_t *fb = sw_pool_get(&RLSW.framebufferPool, RLSW.boundFramebufferId);
    if (fb == NULL) return; // Should never happen

    switch (property)
    {
        case SW_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
        {
            if (attachment == SW_COLOR_ATTACHMENT) *v = fb->colorAttachment;
            else if (attachment == SW_DEPTH_ATTACHMENT) *v = fb->depthAttachment;
        } break;
        case SW_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE: *v = GL_TEXTURE; break;
        default: break;
    }
}

#endif // RLSW_IMPLEMENTATION

//----------------------------------------------------------------------------------
// Template Rasterization Functions
//----------------------------------------------------------------------------------

// Triangle rasterization functions
//-------------------------------------------------------------------------------------------
#ifdef RLSW_TEMPLATE_RASTER_TRIANGLE

// Options:
//  - RLSW_TEMPLATE_RASTER_TRIANGLE     -> Contains the suffix name
//  - SW_ENABLE_DEPTH_TEST
//  - SW_ENABLE_TEXTURE
//  - SW_ENABLE_BLEND

#define SW_RASTER_TRIANGLE_SPAN SW_CONCATX(sw_raster_triangle_span_, RLSW_TEMPLATE_RASTER_TRIANGLE)
#define SW_RASTER_TRIANGLE      SW_CONCATX(sw_raster_triangle_, RLSW_TEMPLATE_RASTER_TRIANGLE)

#ifdef SW_ENABLE_TEXTURE
    #define SW_GET_GRAD         sw_get_vertex_grad_PCT
    #define SW_ADD_GRAD         sw_add_vertex_grad_PCT
    #define SW_ADD_GRAD_SCALED  sw_add_vertex_grad_scaled_PCT
#else
    #define SW_GET_GRAD         sw_get_vertex_grad_PC
    #define SW_ADD_GRAD         sw_add_vertex_grad_PC
    #define SW_ADD_GRAD_SCALED  sw_add_vertex_grad_scaled_PC
#endif

static void SW_RASTER_TRIANGLE_SPAN(const sw_vertex_t *start, const sw_vertex_t *end, float dUdy, float dVdy)
{
    // Gets the start/end coordinates and skip empty lines
    int xStart = (int)start->position[0];
    int xEnd = (int)end->position[0];
    if (xStart == xEnd) return;

    // Compute the inverse horizontal distance along the X axis
    float dxRcp = 1.0f/(end->position[0] - start->position[0]);

    // Compute the interpolation steps along the X axis
    float dWdx = (end->position[3] - start->position[3])*dxRcp;
    float dCdx[4] = {
        (end->color[0] - start->color[0])*dxRcp,
        (end->color[1] - start->color[1])*dxRcp,
        (end->color[2] - start->color[2])*dxRcp,
        (end->color[3] - start->color[3])*dxRcp
    };
#ifdef SW_ENABLE_DEPTH_TEST
    float dZdx = (end->position[2] - start->position[2])*dxRcp;
#endif
#ifdef SW_ENABLE_TEXTURE
    float dUdx = (end->texcoord[0] - start->texcoord[0])*dxRcp;
    float dVdx = (end->texcoord[1] - start->texcoord[1])*dxRcp;
#endif

    // Compute the subpixel distance to traverse before the first pixel
    float xSubstep = 1.0f - sw_fract(start->position[0]);

    // Initializing the interpolation starting values
    float w = start->position[3] + dWdx*xSubstep;
    float color[4] = {
        start->color[0] + dCdx[0]*xSubstep,
        start->color[1] + dCdx[1]*xSubstep,
        start->color[2] + dCdx[2]*xSubstep,
        start->color[3] + dCdx[3]*xSubstep
    };
#ifdef SW_ENABLE_DEPTH_TEST
    float z = start->position[2] + dZdx*xSubstep;
#endif
#ifdef SW_ENABLE_TEXTURE
    float u = start->texcoord[0] + dUdx*xSubstep;
    float v = start->texcoord[1] + dVdx*xSubstep;
#endif

    // Pre-calculate the starting pointers for the framebuffer row
    int y = (int)start->position[1];
    int baseOffset = y*RLSW.colorBuffer->width + xStart;
    uint8_t *cPtr = (uint8_t *)(RLSW.colorBuffer->pixels) + baseOffset*SW_FRAMEBUFFER_COLOR_SIZE;
#ifdef SW_ENABLE_DEPTH_TEST
    uint8_t *dPtr = (uint8_t *)(RLSW.depthBuffer->pixels) + baseOffset*SW_FRAMEBUFFER_DEPTH_SIZE;
#endif

#define SW_AFFINE_BLOCK 16

    int x = xStart;
    while (x < xEnd)
    {
        // Clamp last block to remaining pixels
        int blockEnd = x + SW_AFFINE_BLOCK;
        if (blockEnd > xEnd) blockEnd = xEnd;
        float blockLenF = (float)(blockEnd - x);
        float blockLenRcp = 1.0f/blockLenF;

        // Only 2 '1/w' here; none inside the pixel loop
        float wRcpA = 1.0f/w;
        float wB = w + dWdx*blockLenF;
        float wRcpB = 1.0f/wB;

        // Perspective-correct color at both block endpoints, then affine gradient
        float srcColor[4] = {
            color[0]*wRcpA,
            color[1]*wRcpA,
            color[2]*wRcpA,
            color[3]*wRcpA
        };
        float dSrcColordx[4] = {
            ((color[0] + dCdx[0]*blockLenF)*wRcpB - srcColor[0])*blockLenRcp,
            ((color[1] + dCdx[1]*blockLenF)*wRcpB - srcColor[1])*blockLenRcp,
            ((color[2] + dCdx[2]*blockLenF)*wRcpB - srcColor[2])*blockLenRcp,
            ((color[3] + dCdx[3]*blockLenF)*wRcpB - srcColor[3])*blockLenRcp
        };

    #ifdef SW_ENABLE_TEXTURE
        // Perspective-correct UVs at both endpoints, then affine gradient
        float uAffine  = u*wRcpA;
        float vAffine  = v*wRcpA;
        float dUaffine = ((u + dUdx*blockLenF)*wRcpB - uAffine)*blockLenRcp;
        float dVaffine = ((v + dVdx*blockLenF)*wRcpB - vAffine)*blockLenRcp;
    #endif

        // Inner span pixel loop
        for (; x < blockEnd; x++)
        {
            #ifdef SW_ENABLE_DEPTH_TEST
            {
                float depth = SW_FRAMEBUFFER_DEPTH_GET(dPtr, 0);
                if (z > depth) goto discard;
                SW_FRAMEBUFFER_DEPTH_SET(dPtr, z, 0);
            }
            #endif

            #ifdef SW_ENABLE_TEXTURE
            {
                float texColor[4];
                sw_texture_sample(texColor, RLSW.boundTexture, uAffine, vAffine, dUdx, dUdy, dVdx, dVdy);
                float finalColor[4] = {
                    srcColor[0]*texColor[0],
                    srcColor[1]*texColor[1],
                    srcColor[2]*texColor[2],
                    srcColor[3]*texColor[3]
                };
                #ifdef SW_ENABLE_BLEND
                {
                    float dstColor[4];
                    SW_FRAMEBUFFER_COLOR_GET(dstColor, cPtr, 0);
                    RLSW.blendFunc(dstColor, finalColor);
                    SW_FRAMEBUFFER_COLOR_SET(cPtr, dstColor, 0);
                }
                #else
                    SW_FRAMEBUFFER_COLOR_SET(cPtr, finalColor, 0);
                #endif
            }
            #else
            {
                #ifdef SW_ENABLE_BLEND
                {
                    float dstColor[4];
                    SW_FRAMEBUFFER_COLOR_GET(dstColor, cPtr, 0);
                    RLSW.blendFunc(dstColor, srcColor);
                    SW_FRAMEBUFFER_COLOR_SET(cPtr, dstColor, 0);
                }
                #else
                    SW_FRAMEBUFFER_COLOR_SET(cPtr, srcColor, 0);
                #endif
            }
            #endif

        discard:
            srcColor[0] += dSrcColordx[0];
            srcColor[1] += dSrcColordx[1];
            srcColor[2] += dSrcColordx[2];
            srcColor[3] += dSrcColordx[3];
            cPtr += SW_FRAMEBUFFER_COLOR_SIZE;

            #ifdef SW_ENABLE_DEPTH_TEST
            {
                z += dZdx;
                dPtr += SW_FRAMEBUFFER_DEPTH_SIZE;
            }
            #endif

            #ifdef SW_ENABLE_TEXTURE
            {
                uAffine += dUaffine;
                vAffine += dVaffine;
            }
            #endif
        }

        // Advance perspective-space accumulators by the full block width
        w = wB;
        color[0] += dCdx[0]*blockLenF;
        color[1] += dCdx[1]*blockLenF;
        color[2] += dCdx[2]*blockLenF;
        color[3] += dCdx[3]*blockLenF;
        #ifdef SW_ENABLE_TEXTURE
        u += dUdx*blockLenF;
        v += dVdx*blockLenF;
        #endif
    }

#undef SW_AFFINE_BLOCK
}

static void SW_RASTER_TRIANGLE(const sw_vertex_t *v0, const sw_vertex_t *v1, const sw_vertex_t *v2)
{
    // Swap vertices by increasing Y
    if (v0->position[1] > v1->position[1]) { const sw_vertex_t *tmp = v0; v0 = v1; v1 = tmp; }
    if (v1->position[1] > v2->position[1]) { const sw_vertex_t *tmp = v1; v1 = v2; v2 = tmp; }
    if (v0->position[1] > v1->position[1]) { const sw_vertex_t *tmp = v0; v0 = v1; v1 = tmp; }

    // Extracting coordinates from the sorted vertices
    float x0 = v0->position[0], y0 = v0->position[1];
    float x1 = v1->position[0], y1 = v1->position[1];
    float x2 = v2->position[0], y2 = v2->position[1];

    // Compute height differences
    float h02 = y2 - y0;
    float h01 = y1 - y0;
    float h12 = y2 - y1;

    if (h02 < 1e-6f) return;

    // Inverse edge dy for per-edge dV/dy (scanline interpolation)
    float h02Rcp = 1.0f/h02;
    float h01Rcp = (h01 > 1e-6f)? 1.0f/h01 : 0.0f;
    float h12Rcp = (h12 > 1e-6f)? 1.0f/h12 : 0.0f;

    // Compute gradients for each side of the triangle
    sw_vertex_t dVXdy02, dVXdy01, dVXdy12;
    SW_GET_GRAD(&dVXdy02, v0, v2, h02Rcp);
    SW_GET_GRAD(&dVXdy01, v0, v1, h01Rcp);
    SW_GET_GRAD(&dVXdy12, v1, v2, h12Rcp);

    // Y subpixel correction
    float y0Substep = 1.0f - sw_fract(y0);
    float y1Substep = 1.0f - sw_fract(y1);

    // Get a copy of vertices for interpolation and apply substep correction
    sw_vertex_t lVert = *v0, rVert = *v0;
    SW_ADD_GRAD_SCALED(&lVert, &dVXdy02, y0Substep);
    SW_ADD_GRAD_SCALED(&rVert, &dVXdy01, y0Substep);

    // Y bounds (vertical clipping)
    int yTop = (int)y0;
    int yMid = (int)y1;
    int yBot = (int)y2;

    // Scanline for the upper part of the triangle
    for (int y = yTop; y < yMid; y++)
    {
        lVert.position[1] = rVert.position[1] = y;
        bool longSideIsLeft = (lVert.position[0] < rVert.position[0]);
        const sw_vertex_t *a = longSideIsLeft? &lVert : &rVert;
        const sw_vertex_t *b = longSideIsLeft? &rVert : &lVert;
        SW_RASTER_TRIANGLE_SPAN(a, b, dVXdy02.texcoord[0], dVXdy02.texcoord[1]);
        SW_ADD_GRAD(&lVert, &dVXdy02);
        SW_ADD_GRAD(&rVert, &dVXdy01);
    }

    // Get a copy of next right for interpolation and apply substep correction
    rVert = *v1;
    SW_ADD_GRAD_SCALED(&rVert, &dVXdy12, y1Substep);

    // Scanline for the lower part of the triangle
    for (int y = yMid; y < yBot; y++)
    {
        lVert.position[1] = rVert.position[1] = y;
        bool longSideIsLeft = (lVert.position[0] < rVert.position[0]);
        const sw_vertex_t *a = longSideIsLeft? &lVert : &rVert;
        const sw_vertex_t *b = longSideIsLeft? &rVert : &lVert;
        SW_RASTER_TRIANGLE_SPAN(a, b, dVXdy02.texcoord[0], dVXdy02.texcoord[1]);
        SW_ADD_GRAD(&lVert, &dVXdy02);
        SW_ADD_GRAD(&rVert, &dVXdy12);
    }
}

#undef SW_GET_GRAD
#undef SW_ADD_GRAD
#undef SW_ADD_GRAD_SCALED

#endif // RLSW_TEMPLATE_RASTER_TRIANGLE
//-------------------------------------------------------------------------------------------

// Quad rasterization functions
//-------------------------------------------------------------------------------------------
#ifdef RLSW_TEMPLATE_RASTER_QUAD

// Options:
//  - RLSW_TEMPLATE_RASTER_QUAD     -> Contains the suffix name
//  - SW_ENABLE_DEPTH_TEST
//  - SW_ENABLE_TEXTURE
//  - SW_ENABLE_BLEND

#define SW_RASTER_QUAD SW_CONCATX(sw_raster_quad_, RLSW_TEMPLATE_RASTER_QUAD)

// NOTE: This function should only render affine axis-aligned quads
//       No perspective divide is applied after interpolation

static void SW_RASTER_QUAD(const sw_vertex_t *a, const sw_vertex_t *b,
                           const sw_vertex_t *c, const sw_vertex_t *d)
{
    // Classify corners
    // For axis-aligned quads x+y and x-y uniquely identify each corner
    const sw_vertex_t *verts[4] = { a, b, c, d };
    const sw_vertex_t *tl = verts[0], *tr = verts[0], *br = verts[0], *bl = verts[0];
    for (int i = 1; i < 4; i++)
    {
        float sum  = verts[i]->position[0] + verts[i]->position[1];
        float diff = verts[i]->position[0] - verts[i]->position[1];
        if (sum  < tl->position[0] + tl->position[1]) tl = verts[i];
        if (diff > tr->position[0] - tr->position[1]) tr = verts[i];
        if (sum  > br->position[0] + br->position[1]) br = verts[i];
        if (diff < bl->position[0] - bl->position[1]) bl = verts[i];
    }

    int xMin = (int)tl->position[0];
    int yMin = (int)tl->position[1];
    int xMax = (int)br->position[0];
    int yMax = (int)br->position[1];

    float w = (float)(xMax - xMin);
    float h = (float)(yMax - yMin);
    if ((w <= 0) || (h <= 0)) return;

    float wRcp = 1.0f/w;
    float hRcp = 1.0f/h;

    // Subpixel corrections
    float xSubstep = 1.0f - sw_fract(tl->position[0]);
    float ySubstep = 1.0f - sw_fract(tl->position[1]);

    // Gradients along X (tl->tr) and Y (tl->bl)
    float dCdx[4] = {
        (tr->color[0] - tl->color[0])*wRcp,
        (tr->color[1] - tl->color[1])*wRcp,
        (tr->color[2] - tl->color[2])*wRcp,
        (tr->color[3] - tl->color[3])*wRcp,
    };
    float dCdy[4] = {
        (bl->color[0] - tl->color[0])*hRcp,
        (bl->color[1] - tl->color[1])*hRcp,
        (bl->color[2] - tl->color[2])*hRcp,
        (bl->color[3] - tl->color[3])*hRcp,
    };

#ifdef SW_ENABLE_DEPTH_TEST
    float dZdx = (tr->position[2] - tl->position[2])*wRcp;
    float dZdy = (bl->position[2] - tl->position[2])*hRcp;
    float zRow = tl->position[2] + dZdx*xSubstep + dZdy*ySubstep;
#endif

#ifdef SW_ENABLE_TEXTURE
    float dUdx = (tr->texcoord[0] - tl->texcoord[0])*wRcp;
    float dVdx = (tr->texcoord[1] - tl->texcoord[1])*wRcp;
    float dUdy = (bl->texcoord[0] - tl->texcoord[0])*hRcp;
    float dVdy = (bl->texcoord[1] - tl->texcoord[1])*hRcp;
    float uRow = tl->texcoord[0] + dUdx*xSubstep + dUdy*ySubstep;
    float vRow = tl->texcoord[1] + dVdx*xSubstep + dVdy*ySubstep;
#endif

    float cRow[4] = {
        tl->color[0] + dCdx[0]*xSubstep + dCdy[0]*ySubstep,
        tl->color[1] + dCdx[1]*xSubstep + dCdy[1]*ySubstep,
        tl->color[2] + dCdx[2]*xSubstep + dCdy[2]*ySubstep,
        tl->color[3] + dCdx[3]*xSubstep + dCdy[3]*ySubstep,
    };

    int stride = RLSW.colorBuffer->width;
    uint8_t *cPixels = RLSW.colorBuffer->pixels;
#ifdef SW_ENABLE_DEPTH_TEST
    uint8_t *dPixels = RLSW.depthBuffer->pixels;
#endif

    for (int y = yMin; y < yMax; y++)
    {
        int baseOffset = y*stride + xMin;
        uint8_t *cPtr = cPixels + baseOffset*SW_FRAMEBUFFER_COLOR_SIZE;
    #ifdef SW_ENABLE_DEPTH_TEST
        uint8_t *dPtr = dPixels + baseOffset*SW_FRAMEBUFFER_DEPTH_SIZE;
        float z = zRow;
    #endif
    #ifdef SW_ENABLE_TEXTURE
        float u = uRow;
        float v = vRow;
    #endif
        float color[4] = { cRow[0], cRow[1], cRow[2], cRow[3] };

        for (int x = xMin; x < xMax; x++)
        {
            float srcColor[4] = { color[0], color[1], color[2], color[3] };

            #ifdef SW_ENABLE_DEPTH_TEST
            {
                float depth = SW_FRAMEBUFFER_DEPTH_GET(dPtr, 0);
                if (z > depth) goto discard;
                SW_FRAMEBUFFER_DEPTH_SET(dPtr, z, 0);
            }
            #endif

            #ifdef SW_ENABLE_TEXTURE
            {
                float texColor[4];
                sw_texture_sample(texColor, RLSW.boundTexture, u, v, dUdx, dUdy, dVdx, dVdy);
                srcColor[0] *= texColor[0];
                srcColor[1] *= texColor[1];
                srcColor[2] *= texColor[2];
                srcColor[3] *= texColor[3];
            }
            #endif

            #ifdef SW_ENABLE_BLEND
            {
                float dstColor[4];
                SW_FRAMEBUFFER_COLOR_GET(dstColor, cPtr, 0);
                RLSW.blendFunc(dstColor, srcColor);
                SW_FRAMEBUFFER_COLOR_SET(cPtr, dstColor, 0);
            }
            #else
            {
                SW_FRAMEBUFFER_COLOR_SET(cPtr, srcColor, 0);
            }
            #endif

        discard:
            color[0] += dCdx[0];
            color[1] += dCdx[1];
            color[2] += dCdx[2];
            color[3] += dCdx[3];

            #ifdef SW_ENABLE_DEPTH_TEST
            {
                z += dZdx;
                dPtr += SW_FRAMEBUFFER_DEPTH_SIZE;
            }
            #endif

            #ifdef SW_ENABLE_TEXTURE
            {
                u += dUdx;
                v += dVdx;
            }
            #endif

            cPtr += SW_FRAMEBUFFER_COLOR_SIZE;
        }

        cRow[0] += dCdy[0];
        cRow[1] += dCdy[1];
        cRow[2] += dCdy[2];
        cRow[3] += dCdy[3];

        #ifdef SW_ENABLE_DEPTH_TEST
        {
            zRow += dZdy;
        }
        #endif

        #ifdef SW_ENABLE_TEXTURE
        {
            uRow += dUdy;
            vRow += dVdy;
        }
        #endif
    }
}

#endif // RLSW_TEMPLATE_RASTER_QUAD
//-------------------------------------------------------------------------------------------

// Quad rasterization functions
//-------------------------------------------------------------------------------------------
#ifdef RLSW_TEMPLATE_RASTER_LINE

// Options:
//  - RLSW_TEMPLATE_RASTER_LINE     -> Contains the suffix name
//  - SW_ENABLE_DEPTH_TEST
//  - SW_ENABLE_BLEND

#define SW_RASTER_LINE       SW_CONCATX(sw_raster_line_, RLSW_TEMPLATE_RASTER_LINE)
#define SW_RASTER_LINE_THICK SW_CONCATX(sw_raster_line_thick_, RLSW_TEMPLATE_RASTER_LINE)

static void SW_RASTER_LINE(const sw_vertex_t *v0, const sw_vertex_t *v1)
{
    // Convert from pixel-center convention (n+0.5) to pixel-origin convention (n)
    float x0 = v0->position[0] - 0.5f;
    float y0 = v0->position[1] - 0.5f;
    float x1 = v1->position[0] - 0.5f;
    float y1 = v1->position[1] - 0.5f;

    float dx = x1 - x0;
    float dy = y1 - y0;

    // Compute dominant axis and subpixel offset
    float steps, substep;
    if (fabsf(dx) > fabsf(dy))
    {
        steps = fabsf(dx);
        if (steps < 1.0f) return;
        substep = (dx >= 0.0f)? (1.0f - sw_fract(x0)) : sw_fract(x0);
    }
    else
    {
        steps = fabsf(dy);
        if (steps < 1.0f) return;
        substep = (dy >= 0.0f)? (1.0f - sw_fract(y0)) : sw_fract(y0);
    }

    // Compute per pixel increments
    float xInc = dx/steps;
    float yInc = dy/steps;
    float stepRcp = 1.0f/steps;
#ifdef SW_ENABLE_DEPTH_TEST
    float zInc = (v1->position[2] - v0->position[2])*stepRcp;
#endif
    float rInc = (v1->color[0] - v0->color[0])*stepRcp;
    float gInc = (v1->color[1] - v0->color[1])*stepRcp;
    float bInc = (v1->color[2] - v0->color[2])*stepRcp;
    float aInc = (v1->color[3] - v0->color[3])*stepRcp;

    // Initializing the interpolation starting values
    float x = x0 + xInc*substep;
    float y = y0 + yInc*substep;
#ifdef SW_ENABLE_DEPTH_TEST
    float z = v0->position[2] + zInc*substep;
#endif
    float r = v0->color[0] + rInc*substep;
    float g = v0->color[1] + gInc*substep;
    float b = v0->color[2] + bInc*substep;
    float a = v0->color[3] + aInc*substep;

    // Start line rasterization
    const int fbWidth = RLSW.colorBuffer->width;
    uint8_t *cPixels = RLSW.colorBuffer->pixels;
#ifdef SW_ENABLE_DEPTH_TEST
    uint8_t *dPixels = RLSW.depthBuffer->pixels;
#endif

    int numPixels = (int)(steps - substep) + 1;

    for (int i = 0; i < numPixels; i++)
    {
        int px = x;
        int py = y;

        int baseOffset = py*fbWidth + px;
        uint8_t *cPtr = cPixels + baseOffset*SW_FRAMEBUFFER_COLOR_SIZE;
    #ifdef SW_ENABLE_DEPTH_TEST
        uint8_t *dPtr = dPixels + baseOffset*SW_FRAMEBUFFER_DEPTH_SIZE;
    #endif

        #ifdef SW_ENABLE_DEPTH_TEST
        {
            // TODO: Implement different depth funcs?
            float depth = SW_FRAMEBUFFER_DEPTH_GET(dPtr, 0);
            if (z > depth) goto discard;

            // TODO: Implement depth mask
            SW_FRAMEBUFFER_DEPTH_SET(dPtr, z, 0);
        }
        #endif

        float srcColor[4] = {r, g, b, a};

        #ifdef SW_ENABLE_BLEND
        {
            float dstColor[4];
            SW_FRAMEBUFFER_COLOR_GET(dstColor, cPtr, 0);
            RLSW.blendFunc(dstColor, srcColor);
            SW_FRAMEBUFFER_COLOR_SET(cPtr, dstColor, 0);
        }
        #else
        {
            SW_FRAMEBUFFER_COLOR_SET(cPtr, srcColor, 0);
        }
        #endif

    discard:
        x += xInc;
        y += yInc;
        #ifdef SW_ENABLE_DEPTH_TEST
        {
            z += zInc;
        }
        #endif
        r += rInc;
        g += gInc;
        b += bInc;
        a += aInc;
    }
}

static void SW_RASTER_LINE_THICK(const sw_vertex_t *v0, const sw_vertex_t *v1)
{
    sw_vertex_t tv0, tv1;

    int x0 = (int)v0->position[0];
    int y0 = (int)v0->position[1];
    int x1 = (int)v1->position[0];
    int y1 = (int)v1->position[1];

    int dx = x1 - x0;
    int dy = y1 - y0;

    SW_RASTER_LINE(v0, v1);

    if ((dx != 0) && (abs(dy/dx) < 1))
    {
        int wy = (int)((RLSW.lineWidth - 1.0f)*abs(dx)/sqrtf(dx*dx + dy*dy));
        wy >>= 1;
        for (int i = 1; i <= wy; i++)
        {
            tv0 = *v0, tv1 = *v1;
            tv0.position[1] -= i;
            tv1.position[1] -= i;
            SW_RASTER_LINE(&tv0, &tv1);
            tv0 = *v0, tv1 = *v1;
            tv0.position[1] += i;
            tv1.position[1] += i;
            SW_RASTER_LINE(&tv0, &tv1);
        }
    }
    else if (dy != 0)
    {
        int wx = (int)((RLSW.lineWidth - 1.0f)*abs(dy)/sqrtf(dx*dx + dy*dy));
        wx >>= 1;
        for (int i = 1; i <= wx; i++)
        {
            tv0 = *v0, tv1 = *v1;
            tv0.position[0] -= i;
            tv1.position[0] -= i;
            SW_RASTER_LINE(&tv0, &tv1);
            tv0 = *v0, tv1 = *v1;
            tv0.position[0] += i;
            tv1.position[0] += i;
            SW_RASTER_LINE(&tv0, &tv1);
        }
    }
}

#endif // RLSW_TEMPLATE_RASTER_LINE
//-------------------------------------------------------------------------------------------

// Point rasterization functions
//-------------------------------------------------------------------------------------------
#ifdef RLSW_TEMPLATE_RASTER_POINT

// Options:
//  - RLSW_TEMPLATE_RASTER_POINT    -> Contains the suffix name
//  - SW_ENABLE_DEPTH_TEST
//  - SW_ENABLE_BLEND

#define SW_RASTER_POINT_PIXEL SW_CONCATX(sw_raster_point_pixel_, RLSW_TEMPLATE_RASTER_POINT)
#define SW_RASTER_POINT       SW_CONCATX(sw_raster_point_, RLSW_TEMPLATE_RASTER_POINT)

static void SW_RASTER_POINT_PIXEL(int x, int y, float z, const float color[4])
{
    int offset = y*RLSW.colorBuffer->width + x;

    #ifdef SW_ENABLE_DEPTH_TEST
    {
        uint8_t *dPtr = (uint8_t *)(RLSW.depthBuffer->pixels) + offset*SW_FRAMEBUFFER_DEPTH_SIZE;

        // TODO: Implement different depth funcs?
        float depth = SW_FRAMEBUFFER_DEPTH_GET(dPtr, 0);
        if (z > depth) return;

        // TODO: Implement depth mask
        SW_FRAMEBUFFER_DEPTH_SET(dPtr, z, 0);
    }
    #endif

    uint8_t *cPtr = (uint8_t *)(RLSW.colorBuffer->pixels) + offset*SW_FRAMEBUFFER_COLOR_SIZE;

    #ifdef SW_ENABLE_BLEND
    {
        float dstColor[4];
        SW_FRAMEBUFFER_COLOR_GET(dstColor, cPtr, 0);
        RLSW.blendFunc(dstColor, color);
        SW_FRAMEBUFFER_COLOR_SET(cPtr, dstColor, 0);
    }
    #else
    {
        SW_FRAMEBUFFER_COLOR_SET(cPtr, color, 0);
    }
    #endif
}

static void SW_RASTER_POINT(const sw_vertex_t *v)
{
    int cx = v->position[0];
    int cy = v->position[1];
    float cz = v->position[2];
    int radius = RLSW.pointRadius;
    const float *color = v->color;

    for (int dy = -radius; dy <= radius; dy++)
    {
        for (int dx = -radius; dx <= radius; dx++)
        {
            SW_RASTER_POINT_PIXEL(cx + dx, cy + dy, cz, color);
        }
    }
}

#endif // RLSW_TEMPLATE_RASTER_POINT
//-------------------------------------------------------------------------------------------
