/**
 * MIT License
 * 
 * Copyright (c) 2025 Le Juez Victor
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef RLSW_H
#define RLSW_H

#include <stdbool.h>
#include <stdint.h>

/* === RLSW Definition And Macros === */

#ifndef SW_MALLOC
#   define SW_MALLOC(sz) malloc(sz)
#endif

#ifndef SW_REALLOC
#   define SW_REALLOC(ptr, newSz) realloc(ptr, newSz)
#endif

#ifndef SW_FREE
#   define SW_FREE(ptr) free(ptr)
#endif

#ifndef SW_RESTRICT
#   ifdef _MSC_VER
#       define SW_RESTRICT __restrict
#   else
#       define SW_RESTRICT restrict
#   endif
#endif

#ifndef SW_GL_BINDING_COPY_TEXTURE
#   define SW_GL_BINDING_COPY_TEXTURE true
#endif

#ifndef SW_COLOR_BUFFER_BITS
#   define SW_COLOR_BUFFER_BITS 24
#endif

#ifndef SW_DEPTH_BUFFER_BITS
#   define SW_DEPTH_BUFFER_BITS 16
#endif

#ifndef SW_MAX_PROJECTION_STACK_SIZE
#   define SW_MAX_PROJECTION_STACK_SIZE 2
#endif

#ifndef SW_MAX_MODELVIEW_STACK_SIZE
#   define SW_MAX_MODELVIEW_STACK_SIZE 8
#endif

#ifndef SW_MAX_TEXTURE_STACK_SIZE
#   define SW_MAX_TEXTURE_STACK_SIZE 4
#endif

#ifndef SW_MAX_TEXTURES
#   define SW_MAX_TEXTURES 128
#endif

#ifndef SW_MAX_CLIPPED_POLYGON_VERTICES
#   define SW_MAX_CLIPPED_POLYGON_VERTICES 12
#endif

#ifndef SW_CLIP_EPSILON
#   define SW_CLIP_EPSILON 1e-4f
#endif


/* === OpenGL Compatibility Types === */

typedef unsigned int    GLenum;
typedef unsigned char   GLboolean;
typedef unsigned int    GLbitfield;
typedef void            GLvoid;
typedef signed char     GLbyte;
typedef short           GLshort;
typedef int             GLint;
typedef unsigned char   GLubyte;
typedef unsigned short  GLushort;
typedef unsigned int    GLuint;
typedef int             GLsizei;
typedef float           GLfloat;
typedef float           GLclampf;
typedef double          GLdouble;
typedef double          GLclampd;


/* === OpenGL Definitions === */

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
//#define GL_CURRENT_NORMAL                   0x0B02
//#define GL_CURRENT_RASTER_COLOR           0x0B04
//#define GL_CURRENT_RASTER_DISTANCE        0x0B09
//#define GL_CURRENT_RASTER_INDEX           0x0B05
//#define GL_CURRENT_RASTER_POSITION        0x0B07
//#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
//#define GL_CURRENT_RASTER_POSITION_VALID  0x0B08
#define GL_CURRENT_TEXTURE_COORDS           0x0B03
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
#define GL_NORMAL_ARRAY                     0x8075  //< WARNING: Not implemented (defined for RLGL)
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


/* === Not Used Definitions === */

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


/* === OpenGL Binding === */

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


/* === Not Implemented === */

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


/* === RLSW Enums === */

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


/* === Public API === */

bool swInit(int w, int h);
void swClose(void);

bool swResizeFramebuffer(int w, int h);
void swCopyFramebuffer(int x, int y, int w, int h, SWformat format, SWtype type, void* pixels);
void swBlitFramebuffer(int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, SWformat format, SWtype type, void* pixels);
void* swGetColorBuffer(int* w, int* h);

void swEnable(SWstate state);
void swDisable(SWstate state);

void swGetFloatv(SWget name, float* v);
const char* swGetString(SWget name);
SWerrcode swGetError(void);

void swViewport(int x, int y, int width, int height);
void swScissor(int x, int y, int width, int height);

void swClearColor(float r, float g, float b, float a);
void swClear(uint32_t bitmask);

void swBlendFunc(SWfactor sfactor, SWfactor dfactor);
void swPolygonMode(SWpoly mode);
void swCullFace(SWface face);

void swPointSize(float size);
void swLineWidth(float width);

void swMatrixMode(SWmatrix mode);
void swPushMatrix(void);
void swPopMatrix(void);
void swLoadIdentity(void);
void swTranslatef(float x, float y, float z);
void swRotatef(float angle, float x, float y, float z);
void swScalef(float x, float y, float z);
void swMultMatrixf(const float* mat);
void swFrustum(double left, double right, double bottom, double top, double znear, double zfar);
void swOrtho(double left, double right, double bottom, double top, double znear, double zfar);

void swBegin(SWdraw mode);
void swEnd(void);

void swVertex2i(int x, int y);
void swVertex2f(float x, float y);
void swVertex2fv(const float* v);
void swVertex3i(int x, int y, int z);
void swVertex3f(float x, float y, float z);
void swVertex3fv(const float* v);
void swVertex4i(int x, int y, int z, int w);
void swVertex4f(float x, float y, float z, float w);
void swVertex4fv(const float* v);

void swColor3ub(uint8_t r, uint8_t g, uint8_t b);
void swColor3ubv(const uint8_t* v);
void swColor3f(float r, float g, float b);
void swColor3fv(const float* v);
void swColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void swColor4ubv(const uint8_t* v);
void swColor4f(float r, float g, float b, float a);
void swColor4fv(const float* v);

void swTexCoord2f(float u, float v);
void swTexCoord2fv(const float* v);

void swBindArray(SWarray type, void *buffer);
void swDrawArrays(SWdraw mode, int offset, int count);

void swGenTextures(int count, uint32_t* textures);
void swDeleteTextures(int count, uint32_t* textures);

void swTexImage2D(int width, int height, SWformat format, SWtype type, bool copy, const void* data);
void swTexParameteri(int param, int value);
void swBindTexture(uint32_t id);

#endif // RLSW_H

#define RLSW_IMPL
#ifdef RLSW_IMPL

#include <stdlib.h>
#include <stddef.h>
#include <math.h>

/* === Defines and Macros === */

#define SW_PI 3.14159265358979323846f
#define SW_DEG2RAD (SW_PI/180.0f)
#define SW_RAD2DEG (180.0f/SW_PI)

#define SW_COLOR_PIXEL_SIZE (SW_COLOR_BUFFER_BITS / 8)
#define SW_DEPTH_PIXEL_SIZE (SW_DEPTH_BUFFER_BITS / 8)

#define SW_STATE_CHECK(flags) ((RLSW.stateFlags & (flags)) == (flags))

#define SW_STATE_SCISSOR_TEST   (1 << 0)
#define SW_STATE_TEXTURE_2D     (1 << 1)
#define SW_STATE_DEPTH_TEST     (1 << 2)
#define SW_STATE_CULL_FACE      (1 << 3)
#define SW_STATE_BLEND          (1 << 4)

/* === Internal Structs === */

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
} sw_pixelformat_e;

typedef void (*sw_factor_f)(
    float* SW_RESTRICT factor,
    const float* SW_RESTRICT src,
    const float* SW_RESTRICT dst
);

typedef float sw_matrix_t[4*4];
typedef uint16_t sw_half_t;

typedef struct {

    float position[4];                  // Position coordinates
    float texcoord[2];                  // Texture coordinates
    float color[4];                     // Color

    float homogeneous[4];               // Homogeneous coordinates
    float screen[2];                    // Screen coordinates

} sw_vertex_t;

typedef struct {

    // Dirty hack for copied data  
    // TODO: Rework copied image handling  

    union {
        const void* cptr;   //< NOTE: Is used for all data reads
        void* ptr;          //< WARNING: Should only be used to allocate and free data
    } pixels;

    int width;
    int height;
    int format;

    SWfilter minFilter;
    SWfilter magFilter;

    SWwrap sWrap;
    SWwrap tWrap;

    float tx;
    float ty;

    bool copy;

} sw_texture_t;

typedef struct {
    void *color;
    void *depth;
    int width;
    int height;
    int allocSz;
} sw_framebuffer_t;

typedef struct {

    sw_framebuffer_t framebuffer;
    float clearColor[4];                                // Color used to clear the screen
    float clearDepth;                                   // Depth value used to clear the screen

    int vpPos[2];                                               // Represents the top-left corner of the viewport
    int vpDim[2];                                               // Represents the dimensions of the viewport (minus one)
    int vpMin[2];                                               // Represents the minimum renderable point of the viewport (top-left)
    int vpMax[2];                                               // Represents the maximum renderable point of the viewport (bottom-right)

    int scMin[2];                                               // Represents the minimum renderable point of the scissor rect (top-left)
    int scMax[2];                                               // Represents the maximum renderable point of the scissor rect (bottom-right)
    float scHMin[2];                                            // Represents the minimum renderable point of the scissor rect in clip space
    float scHMax[2];                                            // Represents the maximum renderable point of the scissor rect in clip space

    uint32_t currentTexture;

    struct {
        float* positions;
        float* texcoords;
        uint8_t* colors;
    } array;

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
    sw_matrix_t* currentMatrix;                                 // Pointer to the currently used matrix according to the mode
    sw_matrix_t matMVP;                                         // Model view projection matrix, calculated and used internally
    bool isDirtyMVP;                                            // Indicates if the MVP matrix should be rebuilt

    SWfactor srcFactor;
    SWfactor dstFactor;

    sw_factor_f srcFactorFunc;
    sw_factor_f dstFactorFunc;

    SWface cullFace;                                            // Faces to cull
    SWerrcode errCode;                                          // Last error code

    sw_texture_t* loadedTextures;
    int loadedTextureCount;

    uint32_t* freeTextureIds;
    int freeTextureIdCount;

    uint32_t stateFlags;

} sw_context_t;


/* === Global Data === */

static sw_context_t RLSW = { 0 };


/* === Helper Functions === */

static inline void sw_matrix_id(sw_matrix_t dst)
{
    dst[0]  = 1, dst[1]  = 0, dst[2]  = 0, dst[3]  = 0;
    dst[4]  = 0, dst[5]  = 1, dst[6]  = 0, dst[7]  = 0;
    dst[8]  = 0, dst[9]  = 0, dst[10] = 1, dst[11] = 0;
    dst[12] = 0, dst[13] = 0, dst[14] = 0, dst[15] = 1;
}

static inline void sw_matrix_mul(sw_matrix_t dst, const sw_matrix_t left, const sw_matrix_t right)
{
    sw_matrix_t result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0;
            for (int k = 0; k < 4; k++) {
                sum += left[i * 4 + k] * right[k * 4 + j];
            }
            result[i * 4 + j] = sum;
        }
    }
    for (int i = 0; i < 16; i++) {
        dst[i] = result[i];
    }
}

/*
static inline void sw_vec4_transform(float dst[4], const float v[4], const sw_matrix_t mat)
{
    float tmp[4] = {
        mat[0] * v[0] + mat[4] * v[1] + mat[8] * v[2] + mat[12] * v[3],
        mat[1] * v[0] + mat[5] * v[1] + mat[9] * v[2] + mat[13] * v[3],
        mat[2] * v[0] + mat[6] * v[1] + mat[10] * v[2] + mat[14] * v[3],
        mat[3] * v[0] + mat[7] * v[1] + mat[11] * v[2] + mat[15] * v[3]
    };

    for (int i = 0; i < 4; i++) {
        dst[i] = tmp[i];
    }
}
*/

static inline float sw_saturate(float x)
{
    // Clamps a floating point value between 0.0 and 1.0

    // This implementation uses IEEE 754 bit manipulation:
    // - Uses the sign bit to detect negative values
    // - Directly compares with binary representation of 1.0f to detect values > 1.0

    // Use union to access the bits of the float as an unsigned int
    union { float f; uint32_t u; } v;
    v.f = x;

    // Check sign bit (bit 31): if set, x is negative, return 0.0f
    if (v.u & 0x80000000) return 0.0f;

    // Extract the unsigned magnitude (exponent + mantissa bits)
    uint32_t expMantissa = v.u & 0x7FFFFFFF;

    // If magnitude > binary representation of 1.0f (0x3F800000), return 1.0f
    // This efficiently handles all values > 1.0f without additional computation
    if (expMantissa > 0x3F800000) return 1.0f;

    // Value is between 0.0f and 1.0f inclusive, return unchanged
    return x;
}

static inline float sw_fract(float x)
{
    // Computes the positive fractional part of a float.
    // Equivalent to fabs(x) - floorf(fabs(x)).
    // Uses IEEE 754 bit tricks for efficiency and edge case handling.

    union { float f; uint32_t u; } v;
    v.f = x;

    // Get absolute value bits (clear sign bit)
    uint32_t abs_bits = v.u & 0x7FFFFFFF;

    // Case 1: |x| < 1.0f -> integer part is 0, return |x|
    if (abs_bits < 0x3F800000) {
        v.u = abs_bits; // Ensure positive result
        return v.f;
    }

    // Case 2: |x| ≥ 2^24 -> float is an exact integer, return 0.0f
    // Also handles Inf and NaN as 0.0f
    if (abs_bits >= 0x4B000000) {
        return 0.0f;
    }

    // Case 3: 1.0f ≤ |x| < 2^24 -> compute |x| - floor(|x|)
    v.u = abs_bits;
    float abs_x = v.f;

    return abs_x - floorf(abs_x);
}

static inline int sw_clampi(int v, int min, int max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

static inline float sw_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static inline void sw_lerp_vertex_PNTCH(sw_vertex_t* SW_RESTRICT out, const sw_vertex_t* SW_RESTRICT a, const sw_vertex_t* SW_RESTRICT b, float t)
{
    const float tInv = 1.0f - t;

    // Position interpolation (4 components)
    out->position[0] = a->position[0] * tInv + b->position[0] * t;
    out->position[1] = a->position[1] * tInv + b->position[1] * t;
    out->position[2] = a->position[2] * tInv + b->position[2] * t;
    out->position[3] = a->position[3] * tInv + b->position[3] * t;

    // Texture coordinate interpolation (2 components)
    out->texcoord[0] = a->texcoord[0] * tInv + b->texcoord[0] * t;
    out->texcoord[1] = a->texcoord[1] * tInv + b->texcoord[1] * t;

    // Color interpolation (4 components)
    out->color[0] = a->color[0] * tInv + b->color[0] * t;
    out->color[1] = a->color[1] * tInv + b->color[1] * t;
    out->color[2] = a->color[2] * tInv + b->color[2] * t;
    out->color[3] = a->color[3] * tInv + b->color[3] * t;

    // Homogeneous coordinate interpolation (4 components)
    out->homogeneous[0] = a->homogeneous[0] * tInv + b->homogeneous[0] * t;
    out->homogeneous[1] = a->homogeneous[1] * tInv + b->homogeneous[1] * t;
    out->homogeneous[2] = a->homogeneous[2] * tInv + b->homogeneous[2] * t;
    out->homogeneous[3] = a->homogeneous[3] * tInv + b->homogeneous[3] * t;
}


/* === Half Floating Point === */

static inline uint32_t sw_cvt_hf_ui(uint16_t h)
{
    uint32_t s = (uint32_t)(h & 0x8000) << 16;
    int32_t em = h & 0x7fff;

    // bias exponent and pad mantissa with 0; 112 is relative exponent bias (127-15)
    int32_t r = (em + (112 << 10)) << 13;

    // denormal: flush to zero
    r = (em < (1 << 10)) ? 0 : r;

    // infinity/NaN; note that we preserve NaN payload as a byproduct of unifying inf/nan cases
    // 112 is an exponent bias fixup; since we already applied it once, applying it twice converts 31 to 255
    r += (em >= (31 << 10)) ? (112 << 23) : 0;

    return s | r;
}

static inline float sw_cvt_hf(sw_half_t y)
{
    union { float f; uint32_t i; } v = {
        .i = sw_cvt_hf_ui(y)
    };
    return v.f;
}

static inline uint16_t sw_cvt_fh_ui(uint32_t ui)
{
    int32_t s = (ui >> 16) & 0x8000;
    int32_t em = ui & 0x7fffffff;

    // bias exponent and round to nearest; 112 is relative exponent bias (127-15)
    int32_t h = (em - (112 << 23) + (1 << 12)) >> 13;

    // underflow: flush to zero; 113 encodes exponent -14
    h = (em < (113 << 23)) ? 0 : h;

    // overflow: infinity; 143 encodes exponent 16
    h = (em >= (143 << 23)) ? 0x7c00 : h;

    // NaN; note that we convert all types of NaN to qNaN
    h = (em > (255 << 23)) ? 0x7e00 : h;

    return (uint16_t)(s | h);
}

static inline sw_half_t sw_cvt_fh(float i)
{
    union { float f; uint32_t i; } v;
    v.f = i;
    return sw_cvt_fh_ui(v.i);
}


/* === Framebuffer Part === */

static inline bool sw_framebuffer_load(int w, int h)
{
    int size = w * h;

    RLSW.framebuffer.color = SW_MALLOC(SW_COLOR_PIXEL_SIZE * size);
    if (RLSW.framebuffer.color == NULL) return false;

    RLSW.framebuffer.depth = SW_MALLOC(SW_DEPTH_PIXEL_SIZE * size);
    if (RLSW.framebuffer.depth == NULL) return false;

    RLSW.framebuffer.width = w;
    RLSW.framebuffer.height = h;
    RLSW.framebuffer.allocSz = w * h;

    return true;
}

static inline bool sw_framebuffer_resize(int w, int h)
{
    int newSize = w * h;

    if (newSize <= RLSW.framebuffer.allocSz) {
        RLSW.framebuffer.width = w;
        RLSW.framebuffer.height = h;
        return true;
    }

    void* newColor = SW_REALLOC(RLSW.framebuffer.color, newSize);
    if (newColor == NULL) return false;

    void* newDepth = SW_REALLOC(RLSW.framebuffer.depth, newSize);
    if (newDepth == NULL) return false;

    RLSW.framebuffer.color = newColor;
    RLSW.framebuffer.depth = newDepth;

    RLSW.framebuffer.width = w;
    RLSW.framebuffer.height = h;
    RLSW.framebuffer.allocSz = newSize;

    return true;
}

static inline void* sw_framebuffer_get_color_addr(const void* ptr, uint32_t offset)
{
    return (uint8_t*)ptr + offset * SW_COLOR_PIXEL_SIZE;
}

static inline void sw_framebuffer_inc_color_addr(void** ptr)
{
    *ptr = (void*)(((uint8_t*)*ptr) + SW_COLOR_PIXEL_SIZE);
}

static inline void sw_framebuffer_inc_const_color_addr(const void** ptr)
{
    *ptr = (const void*)(((const uint8_t*)*ptr) + SW_COLOR_PIXEL_SIZE);
}

static inline void* sw_framebuffer_get_depth_addr(const void* ptr, uint32_t offset)
{
    return (uint8_t*)ptr + offset * SW_DEPTH_PIXEL_SIZE;
}

static inline void sw_framebuffer_inc_depth_addr(void** ptr)
{
    *ptr = (void*)(((uint8_t*)*ptr) + SW_DEPTH_PIXEL_SIZE);
}

#if (SW_COLOR_BUFFER_BITS == 8) // RGB - 332

static inline void sw_framebuffer_read_color(float dst[4], const void* src)
{
    uint8_t pixel = ((uint8_t*)src)[0];

    dst[0] = ((pixel >> 5) & 0x07) * (1.0f / 7.0f);
    dst[1] = ((pixel >> 2) & 0x07) * (1.0f / 7.0f);
    dst[2] = (pixel & 0x03) * (1.0f / 3.0f);
    dst[3] = 1.0f;
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const void* src)
{
    uint8_t pixel = ((const uint8_t*)src)[0];

    uint8_t r = (pixel >> 5) & 0x07;
    uint8_t g = (pixel >> 2) & 0x07;
    uint8_t b = pixel & 0x03;

    dst[0] = (r * 255 + 3) / 7;
    dst[1] = (g * 255 + 3) / 7;
    dst[2] = (b * 255 + 1) / 3;
    dst[3] = 255;
}

static inline void sw_framebuffer_write_color(void* dst, float color[3])
{
    uint8_t r = ((uint8_t)(color[0] * UINT8_MAX) >> 5) & 0x07;
    uint8_t g = ((uint8_t)(color[1] * UINT8_MAX) >> 5) & 0x07;
    uint8_t b = ((uint8_t)(color[2] * UINT8_MAX) >> 6) & 0x03;

    ((uint8_t*)dst)[0] = (r << 5) | (g << 2) | b;
}

static inline void sw_framebuffer_fill_color(void* ptr, int size, float color[4])
{
    uint8_t r8 = (uint8_t)(color[0] * 7.0f + 0.5f);
    uint8_t g8 = (uint8_t)(color[1] * 7.0f + 0.5f);
    uint8_t b8 = (uint8_t)(color[2] * 3.0f + 0.5f);

    uint8_t packedColor = ((r8 & 0x07) << 5) | ((g8 & 0x07) << 2) | (b8 & 0x03);

    uint8_t* p = (uint8_t*)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            uint8_t* curPtr = p + y * RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) {
                *curPtr++ = packedColor;
            }
        }
    } else {
        for (int i = 0; i < size; i++) {
            *p++ = packedColor;
        }
    }
}

#elif (SW_COLOR_BUFFER_BITS == 16) // RGB - 565

static inline void sw_framebuffer_read_color(float dst[4], const void* src)
{
    uint16_t pixel = ((uint16_t*)src)[0];

    dst[0] = ((pixel >> 11) & 0x1F) * (1.0f / 31.0f);
    dst[1] = ((pixel >> 5) & 0x3F) * (1.0f / 63.0f);
    dst[2] = (pixel & 0x1F) * (1.0f / 31.0f);
    dst[3] = 1.0f;
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const void* src)
{
    uint16_t pixel = ((const uint16_t*)src)[0];

    uint8_t r = (pixel >> 11) & 0x1F;
    uint8_t g = (pixel >> 5)  & 0x3F;
    uint8_t b = pixel & 0x1F;

    dst[0] = (r * 255 + 15) / 31;
    dst[1] = (g * 255 + 31) / 63;
    dst[2] = (b * 255 + 15) / 31;
    dst[3] = 255;
}

static inline void sw_framebuffer_write_color(void* dst, float color[3])
{
    uint8_t r = (uint8_t)(color[0] * 31.0f + 0.5f) & 0x1F;
    uint8_t g = (uint8_t)(color[1] * 63.0f + 0.5f) & 0x3F;
    uint8_t b = (uint8_t)(color[2] * 31.0f + 0.5f) & 0x1F;

    ((uint16_t*)dst)[0] = (r << 11) | (g << 5) | b;
}

static inline void sw_framebuffer_fill_color(void* ptr, int size, float color[4])
{
    uint16_t r16 = (uint16_t)(color[0] * 31.0f + 0.5f);
    uint16_t g_16 = (uint16_t)(color[1] * 63.0f + 0.5f);
    uint16_t b_16 = (uint16_t)(color[2] * 31.0f + 0.5f);

    uint16_t packedColor = ((r16 & 0x1F) << 11) | ((g_16 & 0x3F) << 5) | (b_16 & 0x1F);

    uint16_t* p = (uint16_t*)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            uint16_t* curPtr = p + y * RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) {
                *curPtr++ = packedColor;
            }
        }
    } else {
        for (int i = 0; i < size; i++) {
            *p++ = packedColor;
        }
    }
}

#elif (SW_COLOR_BUFFER_BITS == 24) // RGB - 888

static inline void sw_framebuffer_read_color(float dst[4], const void* src)
{
    dst[0] = ((uint8_t*)src)[0] * (1.0f / 255.0f);
    dst[1] = ((uint8_t*)src)[1] * (1.0f / 255.0f);
    dst[2] = ((uint8_t*)src)[2] * (1.0f / 255.0f);
    dst[3] = 1.0f;
}

static inline void sw_framebuffer_read_color8(uint8_t dst[4], const void* src)
{
    dst[0] = ((uint8_t*)src)[0];
    dst[1] = ((uint8_t*)src)[1];
    dst[2] = ((uint8_t*)src)[2];
    dst[3] = 255;
}

static inline void sw_framebuffer_write_color(void* dst, float color[3])
{
    ((uint8_t*)dst)[0] = (uint8_t)(color[0] * UINT8_MAX);
    ((uint8_t*)dst)[1] = (uint8_t)(color[1] * UINT8_MAX);
    ((uint8_t*)dst)[2] = (uint8_t)(color[2] * UINT8_MAX);
}

static inline void sw_framebuffer_fill_color(void* ptr, int size, float color[4])
{
    uint8_t r = (uint8_t)(color[0] * 255.0f);
    uint8_t g = (uint8_t)(color[1] * 255.0f);
    uint8_t b = (uint8_t)(color[2] * 255.0f);

    uint8_t* p = (uint8_t*)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            uint8_t* curPtr = p + 3 * (y * RLSW.framebuffer.width + RLSW.scMin[0]);
            for (int xCount = 0; xCount < wScissor; xCount++) {
                *curPtr++ = r;
                *curPtr++ = g;
                *curPtr++ = b;
            }
        }
    } else {
        for (int i = 0; i < size; i++) {
            *p++ = r;
            *p++ = g;
            *p++ = b;
        }
    }
}

#endif // SW_COLOR_BUFFER_BITS

#if (SW_DEPTH_BUFFER_BITS == 8)

static inline float sw_framebuffer_read_depth(const void* src)
{
    return (float)((uint8_t*)src)[0] * (1.0f / UINT8_MAX);
}

static inline void sw_framebuffer_write_depth(void* dst, float depth)
{
    ((uint8_t*)dst)[0] = (uint8_t)(depth * UINT8_MAX);
}

static inline void sw_framebuffer_fill_depth(void* ptr, int size, float value)
{
    uint8_t d8  = (uint8_t)(value * UINT8_MAX);
    uint8_t* p = (uint8_t*)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            uint8_t* curPtr = p + y * RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) {
                *curPtr++ = d8;
            }
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            *p++ = d8;
        }
    }
}

#elif (SW_DEPTH_BUFFER_BITS == 16)

static inline float sw_framebuffer_read_depth(const void* src)
{
    return (float)((uint16_t*)src)[0] * (1.0f / UINT16_MAX);
}

static inline void sw_framebuffer_write_depth(void* dst, float depth)
{
    ((uint16_t*)dst)[0] = (uint16_t)(depth * UINT16_MAX);
}

static inline void sw_framebuffer_fill_depth(void* ptr, int size, float value)
{
    uint16_t d16  = (uint16_t)(value * UINT16_MAX);
    uint16_t* p = (uint16_t*)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            uint16_t* curPtr = p + y * RLSW.framebuffer.width + RLSW.scMin[0];
            for (int xCount = 0; xCount < wScissor; xCount++) {
                *curPtr++ = d16;
            }
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            *p++ = d16;
        }
    }
}

#elif (SW_DEPTH_BUFFER_BITS == 24)

static inline float sw_framebuffer_read_depth(const void* src)
{
    uint32_t depth24 = (((uint8_t*)src)[0] << 16) |
                       (((uint8_t*)src)[1] << 8) |
                       ((uint8_t*)src)[2];

    return depth24 / (float)0xFFFFFF;
}

static inline void sw_framebuffer_write_depth(void* dst, float depth)
{
    uint32_t depth24 = (uint32_t)(depth * 0xFFFFFF);

    ((uint8_t*)dst)[0] = (depth24 >> 16) & 0xFF;
    ((uint8_t*)dst)[1] = (depth24 >> 8) & 0xFF;
    ((uint8_t*)dst)[2] = depth24 & 0xFF;
}

static inline void sw_framebuffer_fill_depth(void* ptr, int size, float value)
{
    uint32_t d32 = (uint32_t)(value * UINT32_MAX);
    uint8_t d_byte0 = (uint8_t)((d32 >> 16) & 0xFF);
    uint8_t d_byte1 = (uint8_t)((d32 >> 8) & 0xFF);
    uint8_t d_byte2 = (uint8_t)(d32 & 0xFF);

    uint8_t* p = (uint8_t*)ptr;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            uint8_t* curPtr = p + 3 * (y * RLSW.framebuffer.width + RLSW.scMin[0]);
            for (int xCount = 0; xCount < wScissor; xCount++) {
                *curPtr++ = d_byte0;
                *curPtr++ = d_byte1;
                *curPtr++ = d_byte2;
            }
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            *p++ = d_byte0;
            *p++ = d_byte1;
            *p++ = d_byte2;
        }
    }
}

#endif // SW_DEPTH_BUFFER_BITS

static inline void sw_framebuffer_fill(void* colorPtr, void* depthPtr, int size, float color[4], float depth_value)
{
#if (SW_COLOR_BUFFER_BITS == 8)
    // Calculate and pack 3:3:2 color
    // Scale color components to the max value for each bit depth and round
    uint8_t r8 = (uint8_t)(color[0] * 7.0f + 0.5f);
    uint8_t g8 = (uint8_t)(color[1] * 7.0f + 0.5f);
    uint8_t b8 = (uint8_t)(color[2] * 3.0f + 0.5f);
    // Pack the components into a single byte
    uint8_t packedColor = ((r8 & 0x07) << 5) | ((g8 & 0x07) << 2) | (b8 & 0x03);
    uint8_t* cptr = (uint8_t*)colorPtr;
#elif (SW_COLOR_BUFFER_BITS == 16)
    // Calculate and pack 5:6:5 color
    // Scale color components to the max value for each bit depth and round
    uint16_t r16 = (uint16_t)(color[0] * 31.0f + 0.5f);
    uint16_t r16 = (uint16_t)(color[1] * 63.0f + 0.5f);
    uint16_t b16 = (uint16_t)(color[2] * 31.0f + 0.5f);
    // Pack the components into a 16-bit value
    uint16_t packedColor = ((r16 & 0x1F) << 11) | ((r16 & 0x3F) << 5) | (b16 & 0x1F);
    uint16_t* cptr = (uint16_t*)colorPtr;
#elif (SW_COLOR_BUFFER_BITS == 24)
    // Calculate 8:8:8 color components
    uint8_t r24 = (uint8_t)(color[0] * 255.0f);
    uint8_t g24 = (uint8_t)(color[1] * 255.0f);
    uint8_t b24 = (uint8_t)(color[2] * 255.0f);
    uint8_t* cptr = (uint8_t*)colorPtr;
#endif

#if (SW_DEPTH_BUFFER_BITS == 8)
    // Calculate 8-bit depth
    uint8_t d8  = (uint8_t)(depth_value * UINT8_MAX);
    uint8_t* dptr = (uint8_t*)depthPtr;
#elif (SW_DEPTH_BUFFER_BITS == 16)
    // Calculate 16-bit depth
    uint16_t d16  = (uint16_t)(depth_value * UINT16_MAX);
    uint16_t* dptr = (uint16_t*)depthPtr;
#elif (SW_DEPTH_BUFFER_BITS == 24)
    // Calculate 24-bit depth and pre-calculate bytes
    uint32_t d32 = (uint32_t)(depth_value * UINT32_MAX);
    uint8_t dByte0 = (uint8_t)((d32 >> 16) & 0xFF);
    uint8_t dByte1 = (uint8_t)((d32 >> 8) & 0xFF);
    uint8_t dByte2 = (uint8_t)(d32 & 0xFF);
    uint8_t* dptr = (uint8_t*)depthPtr;
#endif

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        int wScissor = RLSW.scMax[0] - RLSW.scMin[0] + 1;
        for (int y = RLSW.scMin[1]; y <= RLSW.scMax[1]; y++) {
            int row_start_index = y * RLSW.framebuffer.width + RLSW.scMin[0];

            // Calculate starting pointers for the current row within the scissor rectangle
#           if (SW_COLOR_BUFFER_BITS == 8)
            uint8_t* curCPtr = cptr + row_start_index;
#           elif (SW_COLOR_BUFFER_BITS == 16)
            uint16_t* curCPtr = cptr + row_start_index;
#           elif (SW_COLOR_BUFFER_BITS == 24)
            uint8_t* curCPtr = cptr + 3 * row_start_index;
#           endif

#           if (SW_DEPTH_BUFFER_BITS == 8)
            uint8_t* curDPtr = dptr + row_start_index;
#           elif (SW_DEPTH_BUFFER_BITS == 16)
            uint16_t* curDPtr = dptr + row_start_index;
#           elif (SW_DEPTH_BUFFER_BITS == 24)
            uint8_t* curDPtr = dptr + 3 * row_start_index;
#           endif

            // Fill the current row within the scissor rectangle
            for (int xCount = 0; xCount < wScissor; xCount++)
            {
                // Write color
#           if (SW_COLOR_BUFFER_BITS == 8)
                *curCPtr++ = packedColor;
#           elif (SW_COLOR_BUFFER_BITS == 16)
                *curCPtr++ = packedColor;
#           elif (SW_COLOR_BUFFER_BITS == 24)
                *curCPtr++ = r24;
                *curCPtr++ = g24;
                *curCPtr++ = b24;
#           endif

                // Write depth
#           if (SW_DEPTH_BUFFER_BITS == 8)
                *curDPtr++ = d8;
#           elif (SW_DEPTH_BUFFER_BITS == 16)
                *curDPtr++ = d16;
#           elif (SW_DEPTH_BUFFER_BITS == 24)
                *curDPtr++ = dByte0;
                *curDPtr++ = dByte1;
                *curDPtr++ = dByte2;
#           endif
            }
        }
        return;
    }

    for (int i = 0; i < size; i++)
    {
        // Write color
#   if (SW_COLOR_BUFFER_BITS == 8)
        *cptr++ = packedColor;
#   elif (SW_COLOR_BUFFER_BITS == 16)
        *cptr++ = packedColor;
#   elif (SW_COLOR_BUFFER_BITS == 24)
        *cptr++ = r24;
        *cptr++ = g24;
        *cptr++ = b24;
#   endif

        // Write depth
#   if (SW_DEPTH_BUFFER_BITS == 8)
        *dptr++ = d8;
#   elif (SW_DEPTH_BUFFER_BITS == 16)
        *dptr++ = d16;
#   elif (SW_DEPTH_BUFFER_BITS == 24)
        *dptr++ = dByte0;
        *dptr++ = dByte1;
        *dptr++ = dByte2;
#   endif
    }
}

#define DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_copy_to_##name(int x, int y, int w, int h, DST_PTR_T* dst)   \
{                                                                               \
    const void* src = RLSW.framebuffer.color;                                   \
                                                                                \
    for (int iy = y; iy < h; iy++) {                                            \
        for (int ix = x; ix < w; ix++) {                                        \
            uint8_t color[4];                                                   \
            sw_framebuffer_read_color8(color, src);                             \

#define DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_copy_to_##name(int x, int y, int w, int h, DST_PTR_T* dst)   \
{                                                                               \
    const void* src = RLSW.framebuffer.color;                                   \
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
    uint8_t gray = (uint8_t)((color[0] * 299 + color[1] * 587 + color[2] * 114 + 500) / 1000);
    *dst++ = gray;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(GRAYALPHA, uint8_t)
{
    // Convert RGB to grayscale using NTSC formula
    uint8_t gray = (uint8_t)((color[0] * 299 + color[1] * 587 + color[2] * 114 + 500) / 1000);

    dst[0] = gray;
    dst[1] = color[3]; // alpha

    dst += 2;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R5G6B5, uint16_t)
{
    // Convert 8-bit RGB to 5:6:5 format
    uint8_t r5 = (color[0] * 31 + 127) / 255;
    uint8_t g6 = (color[1] * 63 + 127) / 255;
    uint8_t b5 = (color[2] * 31 + 127) / 255;

    uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;

    *dst++ = rgb565;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R8G8B8, uint8_t)
{
    dst[0] = color[0]; // R
    dst[1] = color[1]; // G
    dst[2] = color[2]; // B

    dst += 3;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R5G5B5A1, uint16_t)
{
    uint8_t r5 = (color[0] * 31 + 127) / 255;
    uint8_t g5 = (color[1] * 31 + 127) / 255;
    uint8_t b5 = (color[2] * 31 + 127) / 255;
    uint8_t a1 = color[3] >= 128 ? 1 : 0;

    uint16_t pixel = (r5 << 11) | (g5 << 6) | (b5 << 1) | a1;

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R4G4B4A4, uint16_t)
{
    uint8_t r4 = (color[0] * 15 + 127) / 255;
    uint8_t g4 = (color[1] * 15 + 127) / 255;
    uint8_t b4 = (color[2] * 15 + 127) / 255;
    uint8_t a4 = (color[3] * 15 + 127) / 255;

    uint16_t pixel = (r4 << 12) | (g4 << 8) | (b4 << 4) | a4;

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_U32_BEGIN(R8G8B8A8, uint8_t)
{
    dst[0] = color[0]; // R
    dst[1] = color[1]; // G
    dst[2] = color[2]; // B
    dst[3] = color[3]; // A

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
    dst[0] = color[0];  // R
    dst[1] = color[1];  // G
    dst[2] = color[2];  // B

    dst += 3;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R32G32B32A32, float)
{
    dst[0] = color[0];  // R
    dst[1] = color[1];  // G
    dst[2] = color[2];  // B
    dst[3] = color[3];  // A

    dst += 4;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R16, sw_half_t)
{
    dst[0] = sw_cvt_fh(color[0]);
    dst++;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R16G16B16, sw_half_t)
{
    dst[0] = sw_cvt_fh(color[0]);  // R
    dst[1] = sw_cvt_fh(color[1]);  // G
    dst[2] = sw_cvt_fh(color[2]);  // B

    dst += 3;
}
DEFINE_FRAMEBUFFER_COPY_END()

DEFINE_FRAMEBUFFER_COPY_F32_BEGIN(R16G16B16A16, sw_half_t)
{
    dst[0] = sw_cvt_fh(color[0]);  // R
    dst[1] = sw_cvt_fh(color[1]);  // G
    dst[2] = sw_cvt_fh(color[2]);  // B
    dst[3] = sw_cvt_fh(color[3]);  // A

    dst += 4;
}
DEFINE_FRAMEBUFFER_COPY_END()

#define DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_blit_to_##name(                               \
    int xDst, int yDst, int wDst, int hDst,                                     \
    int xSrc, int ySrc, int wSrc, int hSrc,                                     \
    DST_PTR_T* dst)                                                             \
{                                                                               \
    const uint8_t* srcBase = RLSW.framebuffer.color;                            \
    int fbWidth = RLSW.framebuffer.width;                                       \
                                                                                \
    uint32_t xScale = ((uint32_t)wSrc << 16) / (uint32_t)wDst;                  \
    uint32_t yScale = ((uint32_t)hSrc << 16) / (uint32_t)hDst;                  \
                                                                                \
    for (int dy = 0; dy < hDst; dy++) {                                         \
        uint32_t yFix = ((uint32_t)ySrc << 16) + dy * yScale;                   \
        int sy = yFix >> 16;                                                    \
                                                                                \
        for (int dx = 0; dx < wDst; dx++) {                                     \
            uint32_t xFix = dx * xScale;                                        \
            int sx = xFix >> 16;                                                \
            const void* srcPtr = sw_framebuffer_get_color_addr(srcBase, sy * fbWidth + sx); \
            uint8_t color[4];                                                   \
            sw_framebuffer_read_color8(color, srcPtr);                          \

#define DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(name, DST_PTR_T)                      \
static inline void sw_framebuffer_blit_to_##name(                               \
    int xDst, int yDst, int wDst, int hDst,                                     \
    int xSrc, int ySrc, int wSrc, int hSrc,                                     \
    DST_PTR_T* dst)                                                             \
{                                                                               \
    const uint8_t* srcBase = RLSW.framebuffer.color;                            \
    int fbWidth = RLSW.framebuffer.width;                                       \
                                                                                \
    uint32_t xScale = ((uint32_t)wSrc << 16) / (uint32_t)wDst;                  \
    uint32_t yScale = ((uint32_t)hSrc << 16) / (uint32_t)hDst;                  \
                                                                                \
    for (int dy = 0; dy < hDst; dy++) {                                         \
        uint32_t yFix = ((uint32_t)ySrc << 16) + dy * yScale;                   \
        int sy = yFix >> 16;                                                    \
                                                                                \
        for (int dx = 0; dx < wDst; dx++) {                                     \
            uint32_t xFix = dx * xScale;                                        \
            int sx = xFix >> 16;                                                \
            const void* srcPtr = sw_framebuffer_get_color_addr(srcBase, sy * fbWidth + sx); \
            float color[4];                                                     \
            sw_framebuffer_read_color(color, srcPtr);                           \

#define DEFINE_FRAMEBUFFER_BLIT_END()                                           \
        }                                                                       \
    }                                                                           \
}

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(GRAYSCALE, uint8_t)
{
    uint8_t gray = (uint8_t)((color[0] * 299 + color[1] * 587 + color[2] * 114 + 500) / 1000);
    *dst++ = gray;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(GRAYALPHA, uint8_t)
{
    uint8_t gray = (uint8_t)((color[0] * 299 + color[1] * 587 + color[2] * 114 + 500) / 1000);

    dst[0] = gray;
    dst[1] = color[3]; // alpha

    dst += 2;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R5G6B5, uint16_t)
{
    uint8_t r5 = (color[0] * 31 + 127) / 255;
    uint8_t g6 = (color[1] * 63 + 127) / 255;
    uint8_t b5 = (color[2] * 31 + 127) / 255;

    uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;

    *dst++ = rgb565;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R8G8B8, uint8_t)
{
    dst[0] = color[0]; // R
    dst[1] = color[1]; // G
    dst[2] = color[2]; // B

    dst += 3;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R5G5B5A1, uint16_t)
{
    uint8_t r5 = (color[0] * 31 + 127) / 255;
    uint8_t g5 = (color[1] * 31 + 127) / 255;
    uint8_t b5 = (color[2] * 31 + 127) / 255;
    uint8_t a1 = color[3] >= 128 ? 1 : 0;

    uint16_t pixel = (r5 << 11) | (g5 << 6) | (b5 << 1) | a1;

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R4G4B4A4, uint16_t)
{
    uint8_t r4 = (color[0] * 15 + 127) / 255;
    uint8_t g4 = (color[1] * 15 + 127) / 255;
    uint8_t b4 = (color[2] * 15 + 127) / 255;
    uint8_t a4 = (color[3] * 15 + 127) / 255;

    uint16_t pixel = (r4 << 12) | (g4 << 8) | (b4 << 4) | a4;

    *dst++ = pixel;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_U32_BEGIN(R8G8B8A8, uint8_t)
{
    dst[0] = color[0]; // R
    dst[1] = color[1]; // G
    dst[2] = color[2]; // B
    dst[3] = color[3]; // A

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
    dst[0] = color[0];  // R
    dst[1] = color[1];  // G
    dst[2] = color[2];  // B

    dst += 3;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R32G32B32A32, float)
{
    dst[0] = color[0];  // R
    dst[1] = color[1];  // G
    dst[2] = color[2];  // B
    dst[3] = color[3];  // A

    dst += 4;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R16, sw_half_t)
{
    dst[0] = sw_cvt_fh(color[0]);
    dst++;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R16G16B16, sw_half_t)
{
    dst[0] = sw_cvt_fh(color[0]);  // R
    dst[1] = sw_cvt_fh(color[1]);  // G
    dst[2] = sw_cvt_fh(color[2]);  // B

    dst += 3;
}
DEFINE_FRAMEBUFFER_BLIT_END()

DEFINE_FRAMEBUFFER_BLIT_F32_BEGIN(R16G16B16A16, sw_half_t)
{
    dst[0] = sw_cvt_fh(color[0]);  // R
    dst[1] = sw_cvt_fh(color[1]);  // G
    dst[2] = sw_cvt_fh(color[2]);  // B
    dst[3] = sw_cvt_fh(color[3]);  // A

    dst += 4;
}
DEFINE_FRAMEBUFFER_BLIT_END()


/* === Pixel Format Part === */

static inline int sw_get_pixel_format(SWformat format, SWtype type)
{
    int channels = 0;
    int bitsPerChannel = 8; // Default: 8 bits per channel
    
    // Determine the number of channels (format)
    switch (format) {
        case SW_LUMINANCE:        channels = 1; break;
        case SW_LUMINANCE_ALPHA:  channels = 2; break;
        case SW_RGB:              channels = 3; break;
        case SW_RGBA:             channels = 4; break;
        default:
            return SW_PIXELFORMAT_UNKNOWN;
    }

    // Determine the depth of each channel (type)
    switch (type) {
        case SW_UNSIGNED_BYTE:    bitsPerChannel = 8;  break;
        case SW_BYTE:             bitsPerChannel = 8;  break;
        case SW_UNSIGNED_SHORT:   bitsPerChannel = 16; break;
        case SW_SHORT:            bitsPerChannel = 16; break;
        case SW_UNSIGNED_INT:     bitsPerChannel = 32; break;
        case SW_INT:              bitsPerChannel = 32; break;
        case SW_FLOAT:            bitsPerChannel = 32; break;
        default:
            return SW_PIXELFORMAT_UNKNOWN;
    }

    // Map the format and type to the correct internal format
    if (bitsPerChannel == 8) {
        if (channels == 1) return SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
        if (channels == 2) return SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
        if (channels == 3) return SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8;
        if (channels == 4) return SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    }
    else if (bitsPerChannel == 16) {
        if (channels == 1) return SW_PIXELFORMAT_UNCOMPRESSED_R16;
        if (channels == 3) return SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16;
        if (channels == 4) return SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;
    }
    else if (bitsPerChannel == 32) {
        if (channels == 1) return SW_PIXELFORMAT_UNCOMPRESSED_R32;
        if (channels == 3) return SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32;
        if (channels == 4) return SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
    }

    return SW_PIXELFORMAT_UNKNOWN;
}

int sw_get_pixel_bytes(sw_pixelformat_e format)
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

static inline void sw_get_pixel_grayscale(float* color, const void* pixels, uint32_t offset)
{
    float gray = (float)((uint8_t*)pixels)[offset] * (1.0f / 255);

    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_red_16(float* color, const void* pixels, uint32_t offset)
{
    float value = sw_cvt_hf(((sw_half_t*)pixels)[offset]);

    color[0] = value;
    color[1] = value;
    color[2] = value;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_red_32(float* color, const void* pixels, uint32_t offset)
{
    float value = ((float*)pixels)[offset];

    color[0] = value;
    color[1] = value;
    color[2] = value;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_grayscale_alpha(float* color, const void* pixels, uint32_t offset)
{
    const uint8_t* pixelData = (const uint8_t*)pixels + 2 * offset;

    color[0] = color[1] = color[2] = (float)pixelData[0] * (1.0f / 255);
    color[3] = (float)pixelData[1] * (1.0f / 255);
}

static inline void sw_get_pixel_rgb_565(float* color, const void* pixels, uint32_t offset)
{
    uint16_t pixel = ((uint16_t*)pixels)[offset];

    color[0] = (float)((pixel & 0xF800) >> 11) / 31;
    color[1] = (float)((pixel & 0x7E0) >> 5) / 63;
    color[2] = (float)(pixel & 0x1F) / 31;
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgb_888(float* color, const void* pixels, uint32_t offset)
{
    const uint8_t* pixel = (const uint8_t*)pixels + 3 * offset;

    color[0] = (float)pixel[0] * (1.0f / 255);
    color[1] = (float)pixel[1] * (1.0f / 255);
    color[2] = (float)pixel[2] * (1.0f / 255);
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgb_161616(float* color, const void* pixels, uint32_t offset)
{
    const sw_half_t *pixel = (sw_half_t*)pixels + 3 * offset;

    color[0] = sw_cvt_hf(pixel[0]);
    color[1] = sw_cvt_hf(pixel[1]);
    color[2] = sw_cvt_hf(pixel[2]);
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgb_323232(float* color, const void* pixels, uint32_t offset)
{
    const float *pixel = (float*)pixels + 3 * offset;

    color[0] = pixel[0];
    color[1] = pixel[1];
    color[2] = pixel[2];
    color[3] = 1.0f;
}

static inline void sw_get_pixel_rgba_5551(float* color, const void* pixels, uint32_t offset)
{
    uint16_t pixel = ((uint16_t*)pixels)[offset];

    color[0] = (float)((pixel & 0xF800) >> 11) / 31;
    color[1] = (float)((pixel & 0x7C0) >> 6) / 31;
    color[2] = (float)((pixel & 0x3E) >> 1) / 31;
    color[3] = (float)(pixel & 0x1);
}

static inline void sw_get_pixel_rgba_4444(float* color, const void* pixels, uint32_t offset)
{
    uint16_t pixel = ((uint16_t*)pixels)[offset];

    color[0] = (float)((pixel & 0xF000) >> 12) / 15;
    color[1] = (float)((pixel & 0xF00) >> 8) / 15;
    color[2] = (float)((pixel & 0xF0) >> 4) / 15;
    color[3] = (float)(pixel & 0xF) / 15;
}

static inline void sw_get_pixel_rgba_8888(float* color, const void* pixels, uint32_t offset)
{
    const uint8_t *pixel = (uint8_t*)pixels + 4 * offset;

    color[0] = (float)pixel[0] * (1.0f / 255);
    color[1] = (float)pixel[1] * (1.0f / 255);
    color[2] = (float)pixel[2] * (1.0f / 255);
    color[3] = (float)pixel[3] * (1.0f / 255);
}

static inline void sw_get_pixel_rgba_16161616(float* color, const void* pixels, uint32_t offset)
{
    const sw_half_t *pixel = (sw_half_t*)pixels + 4 * offset;

    color[0] = sw_cvt_hf(pixel[0]);
    color[1] = sw_cvt_hf(pixel[1]);
    color[2] = sw_cvt_hf(pixel[2]);
    color[3] = sw_cvt_hf(pixel[3]);
}

static inline void sw_get_pixel_rgba_32323232(float* color, const void* pixels, uint32_t offset)
{
    const float *pixel = (float*)pixels + 4 * offset;

    color[0] = pixel[0];
    color[1] = pixel[1];
    color[2] = pixel[2];
    color[3] = pixel[3];
}

static inline void sw_get_pixel(float* color, const void* pixels, uint32_t offset, sw_pixelformat_e format)
{
    switch (format) {

    case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        sw_get_pixel_grayscale(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        sw_get_pixel_grayscale_alpha(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        sw_get_pixel_rgb_565(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        sw_get_pixel_rgb_888(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        sw_get_pixel_rgba_5551(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        sw_get_pixel_rgba_4444(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        sw_get_pixel_rgba_8888(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R32:
        sw_get_pixel_red_32(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        sw_get_pixel_rgb_323232(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        sw_get_pixel_rgba_32323232(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R16:
        sw_get_pixel_red_16(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        sw_get_pixel_rgb_161616(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        sw_get_pixel_rgba_16161616(color, pixels, offset);
        break;

    case SW_PIXELFORMAT_UNKNOWN:
        break;

    }
}


/* === Texture Sampling Part === */

static inline void sw_texture_map(int* out, float in, int max, SWwrap mode)
{
    switch (mode) {
    case SW_REPEAT:
        *out = (int)(sw_fract(in) * max + 0.5f);
        break;
    case SW_CLAMP:
        *out = (int)(sw_saturate(in) * (max - 1) + 0.5f);
        break;
    }
}

static inline void sw_texture_sample_nearest(float* color, const sw_texture_t* tex, float u, float v)
{
    int x, y;
    sw_texture_map(&x, u, tex->width, tex->sWrap);
    sw_texture_map(&y, v, tex->height, tex->tWrap);
    sw_get_pixel(color, tex->pixels.cptr, y * tex->width + x, tex->format);
}

static inline void sw_texture_sample_linear(float* color, const sw_texture_t* tex, float u, float v)
{
    int x0, y0, x1, y1;
    sw_texture_map(&x0, u, tex->width, tex->sWrap);
    sw_texture_map(&y0, v, tex->height, tex->tWrap);
    sw_texture_map(&x1, u + tex->tx, tex->width, tex->sWrap);
    sw_texture_map(&y1, v + tex->ty, tex->height, tex->tWrap);

    float fx = u * (tex->width - 1) - x0;
    float fy = v * (tex->height - 1) - y0;

    float c00[4], c10[4], c01[4], c11[4];
    sw_get_pixel(c00, tex->pixels.cptr, y0 * tex->width + x0, tex->format);
    sw_get_pixel(c10, tex->pixels.cptr, y0 * tex->width + x1, tex->format);
    sw_get_pixel(c01, tex->pixels.cptr, y1 * tex->width + x0, tex->format);
    sw_get_pixel(c11, tex->pixels.cptr, y1 * tex->width + x1, tex->format);

    float c0[4], c1[4];
    for (int i = 0; i < 4; i++) {
        float a = sw_lerp(c00[i], c10[i], fx);
        float b = sw_lerp(c01[i], c11[i], fx);
        color[i] = sw_lerp(a, b, fy);
    }
}

static inline void sw_texture_sample(float* color, const sw_texture_t* tex, float u, float v,
                                     float duDx, float duDy, float dvDx, float dvDy)
{
    // TODO: It seems there are some incorrect detections depending on the context
    //       This is probably due to the fact that the fractions are obtained
    //       at the wrong moment during rasterization. It would be worth reviewing
    //       this, although the scanline method complicates things.

    // Previous method: There is no need to compute the square root
    // because using the squared value, the comparison remains `L2 > 1.0f * 1.0f`
    //float du = sqrtf(duDx * duDx + duDy * duDy);
    //float dv = sqrtf(dvDx * dvDx + dvDy * dvDy);
    //float L = (du > dv) ? du : dv;

    // Calculate the derivatives for each axis
    float du2 = duDx * duDx + duDy * duDy;
    float dv2 = dvDx * dvDx + dvDy * dvDy;
    float L2 = (du2 > dv2) ? du2 : dv2;

    SWfilter filter = (L2 > 1.0f)
        ? tex->minFilter : tex->magFilter;

    switch (filter) {
    case SW_NEAREST:
        sw_texture_sample_nearest(color, tex, u, v);
        break;
    case SW_LINEAR:
        sw_texture_sample_linear(color, tex, u, v);
        break;
    }
}


/* === Color Blending Functions === */

static inline void sw_factor_zero(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = 0.0f;
}

static inline void sw_factor_one(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = 1.0f;
}

static inline void sw_factor_src_color(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = src[0]; factor[1] = src[1]; factor[2] = src[2]; factor[3] = src[3];
}

static inline void sw_factor_one_minus_src_color(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = 1.0f - src[0]; factor[1] = 1.0f - src[1]; 
    factor[2] = 1.0f - src[2]; factor[3] = 1.0f - src[3];
}

static inline void sw_factor_src_alpha(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = src[3];
}

static inline void sw_factor_one_minus_src_alpha(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    float inv_alpha = 1.0f - src[3];
    factor[0] = factor[1] = factor[2] = factor[3] = inv_alpha;
}

static inline void sw_factor_dst_alpha(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = factor[3] = dst[3];
}

static inline void sw_factor_one_minus_dst_alpha(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    float inv_alpha = 1.0f - dst[3];
    factor[0] = factor[1] = factor[2] = factor[3] = inv_alpha;
}

static inline void sw_factor_dst_color(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = dst[0]; factor[1] = dst[1]; factor[2] = dst[2]; factor[3] = dst[3];
}

static inline void sw_factor_one_minus_dst_color(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = 1.0f - dst[0]; factor[1] = 1.0f - dst[1]; 
    factor[2] = 1.0f - dst[2]; factor[3] = 1.0f - dst[3];
}

static inline void sw_factor_src_alpha_saturate(float* SW_RESTRICT factor, const float* SW_RESTRICT src, const float* SW_RESTRICT dst)
{
    factor[0] = factor[1] = factor[2] = 1.0f;
    factor[3] = (src[3] < 1.0f) ? src[3] : 1.0f;
}

static inline void sw_blend_colors(float* SW_RESTRICT dst/*[4]*/, float* SW_RESTRICT src/*[4]*/)
{
    float srcFactor[4], dstFactor[4];

    RLSW.srcFactorFunc(srcFactor, src, dst);
    RLSW.dstFactorFunc(dstFactor, src, dst);

    dst[0] = srcFactor[0] * src[0] + dstFactor[0] * dst[0];
    dst[1] = srcFactor[1] * src[1] + dstFactor[1] * dst[1];
    dst[2] = srcFactor[2] * src[2] + dstFactor[2] * dst[2];
    dst[3] = srcFactor[3] * src[3] + dstFactor[3] * dst[3];
}

/* === Projection Helper Functions === */

static inline void sw_project_ndc_to_screen(float screen[2], const float ndc[4])
{
    screen[0] = RLSW.vpPos[0] + (ndc[0] + 1.0f) * 0.5f * RLSW.vpDim[0];
    screen[1] = RLSW.vpPos[1] + (1.0f - ndc[1]) * 0.5f * RLSW.vpDim[1];
}


/* === Polygon Clipping Part === */

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
            sw_lerp_vertex_PNTCH(&output[outputCount++], prev, curr, t);                \
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

// Frustum clip functions

#define IS_INSIDE_PLANE_W(h) ((h)[3] >= SW_CLIP_EPSILON)
#define IS_INSIDE_PLANE_X_POS(h) ((h)[0] <= (h)[3])
#define IS_INSIDE_PLANE_X_NEG(h) (-(h)[0] <= (h)[3])
#define IS_INSIDE_PLANE_Y_POS(h) ((h)[1] <= (h)[3])
#define IS_INSIDE_PLANE_Y_NEG(h) (-(h)[1] <= (h)[3])
#define IS_INSIDE_PLANE_Z_POS(h) ((h)[2] <= (h)[3])
#define IS_INSIDE_PLANE_Z_NEG(h) (-(h)[2] <= (h)[3])

#define COMPUTE_T_PLANE_W(hPrev, hCurr) ((SW_CLIP_EPSILON - (hPrev)[3]) / ((hCurr)[3] - (hPrev)[3]))
#define COMPUTE_T_PLANE_X_POS(hPrev, hCurr) (((hPrev)[3] - (hPrev)[0]) / (((hPrev)[3] - (hPrev)[0]) - ((hCurr)[3] - (hCurr)[0])))
#define COMPUTE_T_PLANE_X_NEG(hPrev, hCurr) (((hPrev)[3] + (hPrev)[0]) / (((hPrev)[3] + (hPrev)[0]) - ((hCurr)[3] + (hCurr)[0])))
#define COMPUTE_T_PLANE_Y_POS(hPrev, hCurr) (((hPrev)[3] - (hPrev)[1]) / (((hPrev)[3] - (hPrev)[1]) - ((hCurr)[3] - (hCurr)[1])))
#define COMPUTE_T_PLANE_Y_NEG(hPrev, hCurr) (((hPrev)[3] + (hPrev)[1]) / (((hPrev)[3] + (hPrev)[1]) - ((hCurr)[3] + (hCurr)[1])))
#define COMPUTE_T_PLANE_Z_POS(hPrev, hCurr) (((hPrev)[3] - (hPrev)[2]) / (((hPrev)[3] - (hPrev)[2]) - ((hCurr)[3] - (hCurr)[2])))
#define COMPUTE_T_PLANE_Z_NEG(hPrev, hCurr) (((hPrev)[3] + (hPrev)[2]) / (((hPrev)[3] + (hPrev)[2]) - ((hCurr)[3] + (hCurr)[2])))

DEFINE_CLIP_FUNC(w, IS_INSIDE_PLANE_W, COMPUTE_T_PLANE_W)
DEFINE_CLIP_FUNC(x_pos, IS_INSIDE_PLANE_X_POS, COMPUTE_T_PLANE_X_POS)
DEFINE_CLIP_FUNC(x_neg, IS_INSIDE_PLANE_X_NEG, COMPUTE_T_PLANE_X_NEG)
DEFINE_CLIP_FUNC(y_pos, IS_INSIDE_PLANE_Y_POS, COMPUTE_T_PLANE_Y_POS)
DEFINE_CLIP_FUNC(y_neg, IS_INSIDE_PLANE_Y_NEG, COMPUTE_T_PLANE_Y_NEG)
DEFINE_CLIP_FUNC(z_pos, IS_INSIDE_PLANE_Z_POS, COMPUTE_T_PLANE_Z_POS)
DEFINE_CLIP_FUNC(z_neg, IS_INSIDE_PLANE_Z_NEG, COMPUTE_T_PLANE_Z_NEG)

// Scissor clip functions

#define COMPUTE_T_SCISSOR_X_MIN(hPrev, hCurr) (((RLSW.scHMin[0]) * (hPrev)[3] - (hPrev)[0]) / (((hCurr)[0] - (RLSW.scHMin[0]) * (hCurr)[3]) - ((hPrev)[0] - (RLSW.scHMin[0]) * (hPrev)[3])))
#define COMPUTE_T_SCISSOR_X_MAX(hPrev, hCurr) (((RLSW.scHMax[0]) * (hPrev)[3] - (hPrev)[0]) / (((hCurr)[0] - (RLSW.scHMax[0]) * (hCurr)[3]) - ((hPrev)[0] - (RLSW.scHMax[0]) * (hPrev)[3])))
#define COMPUTE_T_SCISSOR_Y_MIN(hPrev, hCurr) (((RLSW.scHMin[1]) * (hPrev)[3] - (hPrev)[1]) / (((hCurr)[1] - (RLSW.scHMin[1]) * (hCurr)[3]) - ((hPrev)[1] - (RLSW.scHMin[1]) * (hPrev)[3])))
#define COMPUTE_T_SCISSOR_Y_MAX(hPrev, hCurr) (((RLSW.scHMax[1]) * (hPrev)[3] - (hPrev)[1]) / (((hCurr)[1] - (RLSW.scHMax[1]) * (hCurr)[3]) - ((hPrev)[1] - (RLSW.scHMax[1]) * (hPrev)[3])))

#define IS_INSIDE_SCISSOR_X_MIN(h) ((h)[0] >= (RLSW.scHMin[0]) * (h)[3])
#define IS_INSIDE_SCISSOR_X_MAX(h) ((h)[0] <= (RLSW.scHMax[0]) * (h)[3])
#define IS_INSIDE_SCISSOR_Y_MIN(h) ((h)[1] >= (RLSW.scHMin[1]) * (h)[3])
#define IS_INSIDE_SCISSOR_Y_MAX(h) ((h)[1] <= (RLSW.scHMax[1]) * (h)[3])

DEFINE_CLIP_FUNC(scissor_x_min, IS_INSIDE_SCISSOR_X_MIN, COMPUTE_T_SCISSOR_X_MIN)
DEFINE_CLIP_FUNC(scissor_x_max, IS_INSIDE_SCISSOR_X_MAX, COMPUTE_T_SCISSOR_X_MAX)
DEFINE_CLIP_FUNC(scissor_y_min, IS_INSIDE_SCISSOR_Y_MIN, COMPUTE_T_SCISSOR_Y_MIN)
DEFINE_CLIP_FUNC(scissor_y_max, IS_INSIDE_SCISSOR_Y_MAX, COMPUTE_T_SCISSOR_Y_MAX)

// Main clip function

static inline bool sw_polygon_clip(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
{
    sw_vertex_t tmp[SW_MAX_CLIPPED_POLYGON_VERTICES];
    int n = *vertexCounter;

    #define CLIP_AGAINST_PLANE(FUNC_CLIP)                       \
    {                                                           \
        n = FUNC_CLIP(tmp, polygon, n);                         \
        if (n == 0) return false;                               \
        for (int i = 0; i < n; i++) {                           \
            polygon[i] = tmp[i];                                \
        }                                                       \
    }

    CLIP_AGAINST_PLANE(sw_clip_w);
    CLIP_AGAINST_PLANE(sw_clip_x_pos);
    CLIP_AGAINST_PLANE(sw_clip_x_neg);
    CLIP_AGAINST_PLANE(sw_clip_y_pos);
    CLIP_AGAINST_PLANE(sw_clip_y_neg);
    CLIP_AGAINST_PLANE(sw_clip_z_pos);
    CLIP_AGAINST_PLANE(sw_clip_z_neg);

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        CLIP_AGAINST_PLANE(sw_clip_scissor_x_min);
        CLIP_AGAINST_PLANE(sw_clip_scissor_x_max);
        CLIP_AGAINST_PLANE(sw_clip_scissor_y_min);
        CLIP_AGAINST_PLANE(sw_clip_scissor_y_max);
    }

    *vertexCounter = n;

    return n > 0;
}


/* === Triangle Rendering Part === */

static inline bool sw_triangle_face_culling(void)
{
    // NOTE: Face culling is done before clipping to avoid unnecessary computations.
    //       However, culling requires NDC coordinates, while clipping must be done 
    //       in homogeneous space to correctly interpolate newly generated vertices.
    //       This means we need to compute 1/W twice: 
    //       - Once before clipping for face culling.
    //       - Again after clipping for the new vertices.

    // Preload homogeneous coordinates into local variables
    const float* h0 = RLSW.vertexBuffer[0].homogeneous;
    const float* h1 = RLSW.vertexBuffer[1].homogeneous;
    const float* h2 = RLSW.vertexBuffer[2].homogeneous;

    // Compute 1/w once and delay divisions
    const float invW0 = 1.0f / h0[3];
    const float invW1 = 1.0f / h1[3];
    const float invW2 = 1.0f / h2[3];

    // Compute the signed 2D area (cross product in Z)
    const float x0 = h0[0] * invW0, y0 = h0[1] * invW0;
    const float x1 = h1[0] * invW1, y1 = h1[1] * invW1;
    const float x2 = h2[0] * invW2, y2 = h2[1] * invW2;
    const float sgnArea = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);

    // Discard the triangle if it faces the culled direction
    return (RLSW.cullFace == SW_FRONT)
        ? (sgnArea < 0) : (sgnArea > 0);
}

static inline void sw_triangle_clip_and_project(void)
{
    sw_vertex_t* polygon = RLSW.vertexBuffer;
    int* vertexCounter = &RLSW.vertexCounter;

    if (sw_polygon_clip(polygon, vertexCounter) && *vertexCounter >= 3) {

        // Transformation to screen space and normalization
        for (int i = 0; i < *vertexCounter; i++) {
            sw_vertex_t *v = &polygon[i];

            // Calculation of the reciprocal of W for normalization
            // as well as perspective-correct attributes
            const float invW = 1.0f / v->homogeneous[3];
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
static inline void FUNC_NAME(const sw_texture_t* tex, const sw_vertex_t* start,     \
                             const sw_vertex_t* end, float duDy, float dvDy)        \
{                                                                                   \
    /* Convert and center the screen coordinates */                                 \
                                                                                    \
    int xStart = (int)(start->screen[0] + 0.5f);                                    \
    int xEnd   = (int)(end->screen[0] + 0.5f);                                      \
    int y      = (int)(start->screen[1] + 0.5f);                                    \
                                                                                    \
    /* Compute the inverse horizontal distance along the X axis */                  \
                                                                                    \
    float dx = end->screen[0] - start->screen[0];                                   \
    if (fabsf(dx) < 1e-6f) return;                                                  \
    float dxRcp = 1.0f / dx;                                                        \
                                                                                    \
    /* Compute the interpolation steps along the X axis */                          \
                                                                                    \
    float dzDx = (end->homogeneous[2] - start->homogeneous[2]) * dxRcp;             \
    float dwDx = (end->homogeneous[3] - start->homogeneous[3]) * dxRcp;             \
                                                                                    \
    float dcDx[4];                                                                  \
    dcDx[0] = (end->color[0] - start->color[0]) * dxRcp;                            \
    dcDx[1] = (end->color[1] - start->color[1]) * dxRcp;                            \
    dcDx[2] = (end->color[2] - start->color[2]) * dxRcp;                            \
    dcDx[3] = (end->color[3] - start->color[3]) * dxRcp;                            \
                                                                                    \
    float duDx, dvDx;                                                               \
    if (ENABLE_TEXTURE) {                                                           \
        duDx = (end->texcoord[0] - start->texcoord[0]) * dxRcp;                     \
        dvDx = (end->texcoord[1] - start->texcoord[1]) * dxRcp;                     \
    }                                                                               \
                                                                                    \
    /* Initializing the interpolation starting values */                            \
                                                                                    \
    float z = start->homogeneous[2];                                                \
    float w = start->homogeneous[3];                                                \
                                                                                    \
    float color[4];                                                                 \
    color[0] = start->color[0];                                                     \
    color[1] = start->color[1];                                                     \
    color[2] = start->color[2];                                                     \
    color[3] = start->color[3];                                                     \
                                                                                    \
    float u, v;                                                                     \
    if (ENABLE_TEXTURE) {                                                           \
        u = start->texcoord[0];                                                     \
        v = start->texcoord[1];                                                     \
    }                                                                               \
                                                                                    \
    /* Pre-calculate the starting pointers for the framebuffer row */               \
                                                                                    \
    void* cptr = sw_framebuffer_get_color_addr(                                     \
        RLSW.framebuffer.color, y * RLSW.framebuffer.width + xStart);               \
                                                                                    \
    void* dptr = sw_framebuffer_get_depth_addr(                                     \
        RLSW.framebuffer.depth, y * RLSW.framebuffer.width + xStart);               \
                                                                                    \
    /* Scanline rasterization */                                                    \
                                                                                    \
    for (int x = xStart; x < xEnd; x++)                                             \
    {                                                                               \
        /* Test and write depth */                                                  \
                                                                                    \
        if (ENABLE_DEPTH_TEST)                                                      \
        {                                                                           \
            /* TODO: Implement different depth funcs? */                            \
            float depth =  sw_framebuffer_read_depth(dptr);                         \
            if (z > depth) goto discard;                                            \
        }                                                                           \
                                                                                    \
        sw_framebuffer_write_depth(dptr, z);                                        \
                                                                                    \
        /* Pixel color computation */                                               \
                                                                                    \
        float wRcp = 1.0f / w;                                                      \
                                                                                    \
        float srcColor[4] = {                                                       \
            color[0] * wRcp,                                                        \
            color[1] * wRcp,                                                        \
            color[2] * wRcp,                                                        \
            color[3] * wRcp                                                         \
        };                                                                          \
                                                                                    \
        if (ENABLE_TEXTURE)                                                         \
        {                                                                           \
            float texColor[4];                                                      \
            float s = u * wRcp;                                                     \
            float t = v * wRcp;                                                     \
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
                                                                                    \
    discard:                                                                        \
                                                                                    \
        z += dzDx;                                                                  \
        w += dwDx;                                                                  \
        color[0] += dcDx[0];                                                        \
        color[1] += dcDx[1];                                                        \
        color[2] += dcDx[2];                                                        \
        color[3] += dcDx[3];                                                        \
        if (ENABLE_TEXTURE) {                                                       \
            u += duDx;                                                              \
            v += dvDx;                                                              \
        }                                                                           \
                                                                                    \
        sw_framebuffer_inc_color_addr(&cptr);                                       \
        sw_framebuffer_inc_depth_addr(&dptr);                                       \
    }                                                                               \
}

#define DEFINE_TRIANGLE_RASTER(FUNC_NAME, FUNC_SCANLINE, ENABLE_TEXTURE)            \
static inline void FUNC_NAME(const sw_vertex_t* v0, const sw_vertex_t* v1, const sw_vertex_t* v2, \
                             const sw_texture_t* tex)                               \
{                                                                                   \
    /* Swap vertices by increasing y */                                             \
    if (v0->screen[1] > v1->screen[1]) { const sw_vertex_t* tmp = v0; v0 = v1; v1 = tmp; }  \
    if (v1->screen[1] > v2->screen[1]) { const sw_vertex_t* tmp = v1; v1 = v2; v2 = tmp; }  \
    if (v0->screen[1] > v1->screen[1]) { const sw_vertex_t* tmp = v0; v0 = v1; v1 = tmp; }  \
                                                                                    \
    /* Extracting coordinates from the sorted vertices */                           \
    float x0 = v0->screen[0], y0 = v0->screen[1];                                   \
    float x1 = v1->screen[0], y1 = v1->screen[1];                                   \
    float x2 = v2->screen[0], y2 = v2->screen[1];                                   \
                                                                                    \
    /* Compute height differences */                                                \
    float h20 = y2 - y0;                                                            \
    float h10 = y1 - y0;                                                            \
    float h21 = y2 - y1;                                                            \
                                                                                    \
    /* Precompute the inverse values without additional checks */                   \
    float invH20 = (h20 > 1e-6f) ? 1.0f / h20 : 0.0f;                               \
    float invH10 = (h10 > 1e-6f) ? 1.0f / h10 : 0.0f;                               \
    float invH21 = (h21 > 1e-6f) ? 1.0f / h21 : 0.0f;                               \
                                                                                    \
    /* Pre-calculation of slopes (dx/dy) */                                         \
    float dx02 = (x2 - x0) * invH20;                                                \
    float dx01 = (x1 - x0) * invH10;                                                \
    float dx12 = (x2 - x1) * invH21;                                                \
                                                                                    \
    /* Y bounds (vertical clipping) */                                              \
    int yTop = (int)(y0 + 0.5f);                                                    \
    int yMiddle = (int)(y1 + 0.5f);                                                 \
    int yBottom = (int)(y2 + 0.5f);                                                 \
                                                                                    \
    /* Global calculation of vertical texture gradients for the triangle */         \
    float duDy, dvDy;                                                               \
    if (ENABLE_TEXTURE) {                                                           \
        duDy = (v2->texcoord[0] - v0->texcoord[0]) * invH20;                        \
        dvDy = (v2->texcoord[1] - v0->texcoord[1]) * invH20;                        \
    }                                                                               \
                                                                                    \
    /* Initializing scanline variables */                                           \
    float xLeft = x0, xRight = x0;                                                  \
    sw_vertex_t start, end;                                                         \
                                                                                    \
    /* Scanline for the upper part of the triangle */                               \
    for (int y = yTop; y < yMiddle; y++) {                                          \
                                                                                    \
        /* Discard the lines that are degenerate */                                 \
        if (fabsf(xRight - xLeft) <= 1e-6f) {                                       \
            goto discardTL;                                                         \
        }                                                                           \
                                                                                    \
        /* Calculation of interpolation factors */                                  \
        float dy = (float)y - y0;                                                   \
        float t1 = dy * invH20;                                                     \
        float t2 = dy * invH10;                                                     \
                                                                                    \
        /* Vertex interpolation */                                                  \
        sw_lerp_vertex_PNTCH(&start, v0, v2, t1);                                   \
        sw_lerp_vertex_PNTCH(&end, v0, v1, t2);                                     \
        start.screen[0] = xLeft;                                                    \
        start.screen[1] = (float)y;                                                 \
        end.screen[0] = xRight;                                                     \
        end.screen[1] = (float)y;                                                   \
                                                                                    \
        if (xLeft > xRight) {                                                       \
            sw_vertex_t tmp = start;                                                \
            start = end;                                                            \
            end = tmp;                                                              \
        }                                                                           \
                                                                                    \
        FUNC_SCANLINE(tex, &start, &end, duDy, dvDy);                               \
                                                                                    \
        /* Incremental update */                                                    \
        discardTL:                                                                  \
        xLeft  += dx02;                                                             \
        xRight += dx01;                                                             \
    }                                                                               \
                                                                                    \
    /* Scanline for the lower part of the triangle */                               \
    xRight = x1; /* Restart the right side from the second vertex */                \
    for (int y = yMiddle; y < yBottom; y++) {                                       \
                                                                                    \
        /* Discard the lines that are degenerate */                                 \
        if (fabsf(xRight - xLeft) <= 1e-6f) {                                       \
            goto discardBL;                                                         \
        }                                                                           \
                                                                                    \
        /* Calculation of interpolation factors */                                  \
        float dy = (float)y - y0;                                                   \
        float t1 = dy * invH20;                                                     \
        float t2 = (float)(y - y1) * invH21;                                        \
                                                                                    \
        /* Vertex interpolation */                                                  \
        sw_lerp_vertex_PNTCH(&start, v0, v2, t1);                                   \
        sw_lerp_vertex_PNTCH(&end, v1, v2, t2);                                     \
        start.screen[0] = xLeft;                                                    \
        start.screen[1] = (float)y;                                                 \
        end.screen[0] = xRight;                                                     \
        end.screen[1] = (float)y;                                                   \
                                                                                    \
        if (xLeft > xRight) {                                                       \
            sw_vertex_t tmp = start;                                                \
            start = end;                                                            \
            end = tmp;                                                              \
        }                                                                           \
                                                                                    \
        FUNC_SCANLINE(tex, &start, &end, duDy, dvDy);                               \
                                                                                    \
        /* Incremental update */                                                    \
        discardBL:                                                                  \
        xLeft  += dx02;                                                             \
        xRight += dx12;                                                             \
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
    if (RLSW.stateFlags & SW_STATE_CULL_FACE) {
        if (!sw_triangle_face_culling()) {
            return;
        }
    }

    sw_triangle_clip_and_project();

    if (RLSW.vertexCounter < 3) {
        return;
    }

#   define TRIANGLE_RASTER(RASTER_FUNC)                         \
    {                                                           \
        for (int i = 0; i < RLSW.vertexCounter - 2; i++) {      \
            RASTER_FUNC(                                        \
                &RLSW.vertexBuffer[0],                          \
                &RLSW.vertexBuffer[i + 1],                      \
                &RLSW.vertexBuffer[i + 2],                      \
                &RLSW.loadedTextures[RLSW.currentTexture]       \
            );                                                  \
        }                                                       \
    }

    if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH)
    }
    else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
        TRIANGLE_RASTER(sw_triangle_raster_DEPTH)
    }
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX)
    }
    else {
        TRIANGLE_RASTER(sw_triangle_raster)
    }

#   undef TRIANGLE_RASTER
}


/* === Quad Rendering Part === */

static inline bool sw_quad_face_culling(void)
{
    // NOTE: We use Green's theorem (signed polygon area) instead of triangulation.
    // This is faster but only reliable if the quad is convex and not self-intersecting.
    // For face culling purposes, this approximation is acceptable.

    // Preload homogeneous coordinates into local variables
    const float* h0 = RLSW.vertexBuffer[0].homogeneous;
    const float* h1 = RLSW.vertexBuffer[1].homogeneous;
    const float* h2 = RLSW.vertexBuffer[2].homogeneous;
    const float* h3 = RLSW.vertexBuffer[3].homogeneous;

    // Compute 1/w once and delay divisions
    const float invW0 = 1.0f / h0[3];
    const float invW1 = 1.0f / h1[3];
    const float invW2 = 1.0f / h2[3];
    const float invW3 = 1.0f / h3[3];

    // Pre-multiply to get x/w and y/w coordinates
    const float x0 = h0[0] * invW0, y0 = h0[1] * invW0;
    const float x1 = h1[0] * invW1, y1 = h1[1] * invW1;
    const float x2 = h2[0] * invW2, y2 = h2[1] * invW2;
    const float x3 = h3[0] * invW3, y3 = h3[1] * invW3;

    // Use Green's theorem (signed polygon area)
    // area = 0.5 * sum of (xi * yi+1 - xi+1 * yi)
    // The factor 0.5 is not needed here, only the sign matters.
    const float sgnArea = 
        (x0 * y1 - x1 * y0)
        + (x1 * y2 - x2 * y1)
        + (x2 * y3 - x3 * y2)
        + (x3 * y0 - x0 * y3);

    // Perform face culling based on area sign
    return (RLSW.cullFace == SW_FRONT)
        ? (sgnArea < 0.0f) : (sgnArea > 0.0f);
}

static inline void sw_quad_clip_and_project(void)
{
    sw_vertex_t* polygon = RLSW.vertexBuffer;
    int* vertexCounter = &RLSW.vertexCounter;

    if (sw_polygon_clip(polygon, vertexCounter) && *vertexCounter >= 3) {

        // Transformation to screen space and normalization
        for (int i = 0; i < *vertexCounter; i++) {
            sw_vertex_t *v = &polygon[i];

            // Calculation of the reciprocal of W for normalization
            // as well as perspective-correct attributes
            const float invW = 1.0f / v->homogeneous[3];
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

    for (int i = 0; i < 4; i++) {
        const float* v0 = RLSW.vertexBuffer[i].position;
        const float* v1 = RLSW.vertexBuffer[(i + 1) % 4].position;

        float dx = v1[0] - v0[0];
        float dy = v1[1] - v0[1];

        if (fabsf(dx) > 1e-6f && fabsf(dy) < 1e-6f) {
            horizontal++;
        }
        else if (fabsf(dy) > 1e-6f && fabsf(dx) < 1e-6f) {
            vertical++;
        }
        else {
            // Diagonal edge -> not axis-aligned
            return false;
        }
    }

    return (horizontal == 2 && vertical == 2);
}

static inline void sw_quad_sort_cw(const sw_vertex_t** output)
{
    // Sort 4 quad vertices into clockwise order with fixed layout:
    //
    //   v0 -- v1
    //   |      |
    //   v3 -- v2
    //
    // The goal is:
    // - v0: top-left (minimum Y, then minimum X)
    // - v1: top-right (same Y row as v0, maximum X)
    // - v3: bottom-left (maximum Y, minimum X)
    // - v2: bottom-right (maximum Y, maximum X)

    const sw_vertex_t* input = RLSW.vertexBuffer;

    // Find indices of vertices with min Y (top) and max Y (bottom)
    int minYIndex = 0, maxYIndex = 0;
    for (int i = 1; i < 4; i++) {
        if (input[i].screen[1] < input[minYIndex].screen[1]) minYIndex = i;
        if (input[i].screen[1] > input[maxYIndex].screen[1]) maxYIndex = i;
    }

    // Find indices of the remaining two vertices
    int others[2], idx = 0;
    for (int i = 0; i < 4; i++) {
        if (i != minYIndex && i != maxYIndex) {
            others[idx++] = i;
        }
    }

    // Determine left/right among the middle vertices by X coordinate
    int leftMidIndex = (input[others[0]].screen[0] < input[others[1]].screen[0]) ? others[0] : others[1];
    int rightMidIndex = (leftMidIndex == others[0]) ? others[1] : others[0];

    // Assign top vertices: v0 = top-left, v1 = top-right
    if (input[minYIndex].screen[0] < input[leftMidIndex].screen[0]) {
        output[0] = &input[minYIndex];      // v0: top-left
        output[1] = &input[leftMidIndex];   // v1: top-right
    } else {
        output[0] = &input[leftMidIndex];
        output[1] = &input[minYIndex];
    }

    // Assign bottom vertices: v3 = bottom-left, v2 = bottom-right
    if (input[maxYIndex].screen[0] < input[rightMidIndex].screen[0]) {
        output[3] = &input[maxYIndex];      // v3: bottom-left
        output[2] = &input[rightMidIndex];  // v2: bottom-right
    } else {
        output[3] = &input[rightMidIndex];
        output[2] = &input[maxYIndex];
    }
}

// REVIEW: Could a perfectly aligned quad, where one of the four points has a different depth, still appear perfectly aligned from a certain point of view?
//         Because in that case, we would still need to perform perspective division for textures and colors...
#define DEFINE_QUAD_RASTER_AXIS_ALIGNED(FUNC_NAME, ENABLE_TEXTURE, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(void)                                              \
{                                                                               \
    const sw_vertex_t* sortedVerts[4];                                          \
    sw_quad_sort_cw(sortedVerts);                                               \
                                                                                \
    const sw_vertex_t* v0 = sortedVerts[0];                                     \
    const sw_vertex_t* v1 = sortedVerts[1];                                     \
    const sw_vertex_t* v2 = sortedVerts[2];                                     \
    const sw_vertex_t* v3 = sortedVerts[3];                                     \
                                                                                \
    /* Screen bounds (axis-aligned) */                                          \
                                                                                \
    int xMin = (int)v0->screen[0];                                              \
    int yMin = (int)v0->screen[1];                                              \
    int xMax = (int)v2->screen[0];                                              \
    int yMax = (int)v2->screen[1];                                              \
                                                                                \
    float width = (float)(xMax - xMin);                                         \
    float height = (float)(yMax - yMin);                                        \
                                                                                \
    float invWidth = 1.0f / width;                                              \
    float invHeight = 1.0f / height;                                            \
                                                                                \
    /* Precomputed coefficients for bilinear interpolation */                   \
                                                                                \
    float zA, zB, zC, zD;                                                       \
                                                                                \
    zA = v0->homogeneous[2];                                                    \
    zB = v1->homogeneous[2] - v0->homogeneous[2];                               \
    zC = v3->homogeneous[2] - v0->homogeneous[2];                               \
    zD = v2->homogeneous[2] - v3->homogeneous[2] - v1->homogeneous[2] + v0->homogeneous[2]; \
                                                                                \
    float colorA[4];                                                            \
    float colorB[4];                                                            \
    float colorC[4];                                                            \
    float colorD[4];                                                            \
                                                                                \
    for (int c = 0; c < 4; c++) {                                               \
        colorA[c] = v0->color[c];                                               \
        colorB[c] = v1->color[c] - v0->color[c];                                \
        colorC[c] = v3->color[c] - v0->color[c];                                \
        colorD[c] = v2->color[c] - v3->color[c] - v1->color[c] + v0->color[c];  \
    }                                                                           \
                                                                                \
    float texA[2];                                                              \
    float texB[2];                                                              \
    float texC[2];                                                              \
    float texD[2];                                                              \
                                                                                \
    if (ENABLE_TEXTURE)                                                         \
    {                                                                           \
        for (int uv = 0; uv < 2; uv++) {                                        \
            texA[uv] = v0->texcoord[uv];                                        \
            texB[uv] = v1->texcoord[uv] - v0->texcoord[uv];                     \
            texC[uv] = v3->texcoord[uv] - v0->texcoord[uv];                     \
            texD[uv] = v2->texcoord[uv] - v3->texcoord[uv] - v1->texcoord[uv] + v0->texcoord[uv]; \
        }                                                                       \
    }                                                                           \
                                                                                \
    /* Precomputed UV gradients (constant across the entire quad) */            \
                                                                                \
    float duDx, dvDx, duDy, dvDy;                                               \
                                                                                \
    if (ENABLE_TEXTURE)                                                         \
    {                                                                           \
        duDx = ((v1->texcoord[0] - v0->texcoord[0]) + (v2->texcoord[0] - v3->texcoord[0])) * 0.5f * invWidth; \
        dvDx = ((v1->texcoord[1] - v0->texcoord[1]) + (v2->texcoord[1] - v3->texcoord[1])) * 0.5f * invWidth; \
        duDy = ((v3->texcoord[0] - v0->texcoord[0]) + (v2->texcoord[0] - v1->texcoord[0])) * 0.5f * invHeight; \
        dvDy = ((v3->texcoord[1] - v0->texcoord[1]) + (v2->texcoord[1] - v1->texcoord[1])) * 0.5f * invHeight; \
    }                                                                           \
                                                                                \
    const sw_texture_t* tex = &RLSW.loadedTextures[RLSW.currentTexture];        \
    void* cDstBase = RLSW.framebuffer.color;                                    \
    void* dDstBase = RLSW.framebuffer.depth;                                    \
    int wDst = RLSW.framebuffer.width;                                          \
                                                                                \
    for (int y = yMin; y < yMax; y++)                                           \
    {                                                                           \
        float ty = (y - yMin) * invHeight;                                      \
        void* cptr = sw_framebuffer_get_color_addr(cDstBase, y * wDst + xMin);  \
        void* dptr = sw_framebuffer_get_depth_addr(dDstBase, y * wDst + xMin);  \
                                                                                \
        /* Compute starting values for this scanline (for x = xMin) */          \
                                                                                \
        float z = zA + zC * ty;                                                 \
                                                                                \
        float srcColor[4];                                                      \
        srcColor[0] = colorA[0] + colorC[0] * ty;                               \
        srcColor[1] = colorA[1] + colorC[1] * ty;                               \
        srcColor[2] = colorA[2] + colorC[2] * ty;                               \
        srcColor[3] = colorA[3] + colorC[3] * ty;                               \
                                                                                \
        float u, v;                                                             \
        if (ENABLE_TEXTURE) {                                                   \
            u = texA[0] + texC[0] * ty;                                         \
            v = texA[1] + texC[1] * ty;                                         \
        }                                                                       \
                                                                                \
        /* Compute per-pixel increments along X (constant for a scanline) */    \
                                                                                \
        float zIncX = (zB + zD * ty) * invWidth;                                \
                                                                                \
        float colorIncX[4];                                                     \
        colorIncX[0] = (colorB[0] + colorD[0] * ty) * invWidth;                 \
        colorIncX[1] = (colorB[1] + colorD[1] * ty) * invWidth;                 \
        colorIncX[2] = (colorB[2] + colorD[2] * ty) * invWidth;                 \
        colorIncX[3] = (colorB[3] + colorD[3] * ty) * invWidth;                 \
                                                                                \
        float uvIncX[2];                                                        \
        if (ENABLE_TEXTURE) {                                                   \
            uvIncX[0] = (texB[0] + texD[0] * ty) * invWidth;                    \
            uvIncX[1] = (texB[1] + texD[1] * ty) * invWidth;                    \
        }                                                                       \
                                                                                \
        /* Scanline rasterization */                                            \
                                                                                \
        for (int x = xMin; x < xMax; x++)                                       \
        {                                                                       \
            /* Test and write depth */                                          \
                                                                                \
            if (ENABLE_DEPTH_TEST)                                              \
            {                                                                   \
                /* TODO: Implement different depth funcs? */                    \
                float depth =  sw_framebuffer_read_depth(dptr);                 \
                if (z > depth) goto discard;                                    \
            }                                                                   \
                                                                                \
            sw_framebuffer_write_depth(dptr, z);                                \
                                                                                \
            /* Pixel color computation */                                       \
                                                                                \
            float fragColor[4] = {                                              \
                srcColor[0],                                                    \
                srcColor[1],                                                    \
                srcColor[2],                                                    \
                srcColor[3]                                                     \
            };                                                                  \
                                                                                \
            if (ENABLE_TEXTURE)                                                 \
            {                                                                   \
                float texColor[4];                                              \
                sw_texture_sample(texColor, tex, u, v, duDx, duDy, dvDx, dvDy); \
                fragColor[0] *= texColor[0];                                    \
                fragColor[1] *= texColor[1];                                    \
                fragColor[2] *= texColor[2];                                    \
                fragColor[3] *= texColor[3];                                    \
            }                                                                   \
                                                                                \
            if (ENABLE_COLOR_BLEND)                                             \
            {                                                                   \
                float dstColor[4];                                              \
                sw_framebuffer_read_color(dstColor, cptr);                      \
                                                                                \
                sw_blend_colors(dstColor, fragColor);                           \
                dstColor[0] = sw_saturate(dstColor[0]);                         \
                dstColor[1] = sw_saturate(dstColor[1]);                         \
                dstColor[2] = sw_saturate(dstColor[2]);                         \
                                                                                \
                sw_framebuffer_write_color(cptr, dstColor);                     \
            }                                                                   \
            else                                                                \
            {                                                                   \
                sw_framebuffer_write_color(cptr, fragColor);                    \
            }                                                                   \
                                                                                \
            /* Increment values for the next pixel */                           \
                                                                                \
        discard:                                                                \
                                                                                \
            z += zIncX;                                                         \
                                                                                \
            srcColor[0] += colorIncX[0];                                        \
            srcColor[1] += colorIncX[1];                                        \
            srcColor[2] += colorIncX[2];                                        \
            srcColor[3] += colorIncX[3];                                        \
                                                                                \
            if (ENABLE_TEXTURE) {                                               \
                u += uvIncX[0];                                                 \
                v += uvIncX[1];                                                 \
            }                                                                   \
                                                                                \
            /* Advance the pointers along the line */                           \
                                                                                \
            sw_framebuffer_inc_color_addr(&cptr);                               \
            sw_framebuffer_inc_depth_addr(&dptr);                               \
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
    if (RLSW.stateFlags & SW_STATE_CULL_FACE) {
        if (!sw_quad_face_culling()) {
            return;
        }
    }

    sw_quad_clip_and_project();

    if (RLSW.vertexCounter < 3) {
        return;
    }

    if (RLSW.vertexCounter == 4 && sw_quad_is_axis_aligned())
    {
        if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
            sw_quad_raster_axis_aligned_TEX_DEPTH_BLEND();
        }
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
            sw_quad_raster_axis_aligned_DEPTH_BLEND();
        }
        else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) {
            sw_quad_raster_axis_aligned_TEX_BLEND();
        }
        else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) {
            sw_quad_raster_axis_aligned_TEX_DEPTH();
        }
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
            sw_quad_raster_axis_aligned_BLEND();
        }
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
            sw_quad_raster_axis_aligned_DEPTH();
        }
        else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D)) {
            sw_quad_raster_axis_aligned_TEX();
        }
        else {
            sw_quad_raster_axis_aligned();
        }
        return;
    }

#   define TRIANGLE_RASTER(RASTER_FUNC)                         \
    {                                                           \
        for (int i = 0; i < RLSW.vertexCounter - 2; i++) {      \
            RASTER_FUNC(                                        \
                &RLSW.vertexBuffer[0],                          \
                &RLSW.vertexBuffer[i + 1],                      \
                &RLSW.vertexBuffer[i + 2],                      \
                &RLSW.loadedTextures[RLSW.currentTexture]       \
            );                                                  \
        }                                                       \
    }

    if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_DEPTH_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D | SW_STATE_DEPTH_TEST)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX_DEPTH)
    }
    else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
        TRIANGLE_RASTER(sw_triangle_raster_BLEND)
    }
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
        TRIANGLE_RASTER(sw_triangle_raster_DEPTH)
    }
    else if (SW_STATE_CHECK(SW_STATE_TEXTURE_2D)) {
        TRIANGLE_RASTER(sw_triangle_raster_TEX)
    }
    else {
        TRIANGLE_RASTER(sw_triangle_raster)
    }

#   undef TRIANGLE_RASTER
}


/* === Line Rendering Part === */

static inline bool sw_line_clip_coord(float q, float p, float* t0, float* t1)
{
    if (fabsf(p) < SW_CLIP_EPSILON) {
        // Check if the line is entirely outside the window
        if (q < -SW_CLIP_EPSILON) return 0; // Completely outside
        return 1;                           // Completely inside or on the edges
    }

    const float r = q / p;

    if (p < 0) {
        if (r > *t1) return 0;
        if (r > *t0) *t0 = r;
    } else {
        if (r < *t0) return 0;
        if (r < *t1) *t1 = r;
    }

    return 1;
}

static inline bool sw_line_clip(sw_vertex_t* v0, sw_vertex_t* v1)
{
    float t0 = 0.0f, t1 = 1.0f;
    float dH[4], dC[4];

    for (int i = 0; i < 4; i++) {
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
    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        if (!sw_line_clip_coord(v0->homogeneous[0] - RLSW.scHMin[0] * v0->homogeneous[3], RLSW.scHMin[0] * dH[3] - dH[0], &t0, &t1)) return false;
        if (!sw_line_clip_coord(RLSW.scHMax[0] * v0->homogeneous[3] - v0->homogeneous[0], dH[0] - RLSW.scHMax[0] * dH[3], &t0, &t1)) return false;
        if (!sw_line_clip_coord(v0->homogeneous[1] - RLSW.scHMin[1] * v0->homogeneous[3], RLSW.scHMin[1] * dH[3] - dH[1], &t0, &t1)) return false;
        if (!sw_line_clip_coord(RLSW.scHMax[1] * v0->homogeneous[3] - v0->homogeneous[1], dH[1] - RLSW.scHMax[1] * dH[3], &t0, &t1)) return false;
    }

    // Interpolation of new coordinates
    if (t1 < 1.0f) {
        for (int i = 0; i < 4; i++) {
            v1->homogeneous[i] = v0->homogeneous[i] + t1 * dH[i];
            v1->color[i] = v0->color[i] + t1 * dC[i];
        }
    }
    if (t0 > 0.0f) {
        for (int i = 0; i < 4; i++) {
            v0->homogeneous[i] += t0 * dH[i];
            v0->color[i] += t0 * dC[i];
        }
    }

    return true;
}

static inline bool sw_line_clip_and_project(sw_vertex_t* v0, sw_vertex_t* v1)
{
    if (!sw_line_clip(v0, v1)) {
        return false;
    }

    // Convert homogeneous coordinates to NDC
    v0->homogeneous[3] = 1.0f / v0->homogeneous[3];
    v1->homogeneous[3] = 1.0f / v1->homogeneous[3];
    for (int i = 0; i < 3; i++) {
        v0->homogeneous[i] *= v0->homogeneous[3];
        v1->homogeneous[i] *= v1->homogeneous[3];
    }

    // Convert NDC coordinates to screen space
    sw_project_ndc_to_screen(v0->screen, v0->homogeneous);
    sw_project_ndc_to_screen(v1->screen, v1->homogeneous);

    return true;
}

#define DEFINE_LINE_RASTER(FUNC_NAME, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(const sw_vertex_t* v0, const sw_vertex_t* v1) \
{                                                                       \
    int x1 = (int)(v0->screen[0] + 0.5f);                               \
    int y1 = (int)(v0->screen[1] + 0.5f);                               \
    int x2 = (int)(v1->screen[0] + 0.5f);                               \
    int y2 = (int)(v1->screen[1] + 0.5f);                               \
                                                                        \
    float z1 = v0->homogeneous[2];                                      \
    float z2 = v1->homogeneous[2];                                      \
                                                                        \
    int shortLen = y2 - y1;                                             \
    int longLen = x2 - x1;                                              \
    bool yLonger = 0;                                                   \
                                                                        \
    if (abs(shortLen) > abs(longLen)) {                                 \
        int tmp = shortLen;                                             \
        shortLen = longLen;                                             \
        longLen = tmp;                                                  \
        yLonger = 1;                                                    \
    }                                                                   \
                                                                        \
    float invEndVal = 1.0f / longLen;                                   \
    int endVal = longLen;                                               \
    int sgnInc = 1;                                                     \
                                                                        \
    if (longLen < 0) {                                                  \
        longLen = -longLen;                                             \
        sgnInc = -1;                                                    \
    }                                                                   \
                                                                        \
    int decInc = (longLen == 0) ? 0                                     \
        : (shortLen << 16) / longLen;                                   \
                                                                        \
    const int fbWidth = RLSW.framebuffer.width;                         \
    const float zDiff = z2 - z1;                                        \
                                                                        \
    uint8_t* colorBuffer = RLSW.framebuffer.color;                      \
    uint16_t* depthBuffer = RLSW.framebuffer.depth;                     \
                                                                        \
    int j = 0;                                                          \
    if (yLonger) {                                                      \
        for (int i = 0; i != endVal; i += sgnInc, j += decInc) {        \
            float t = (float)i * invEndVal;                             \
                                                                        \
            int x = x1 + (j >> 16);                                     \
            int y = y1 + i;                                             \
            float z = z1 + t * zDiff;                                   \
            int offset = y * fbWidth + x;                               \
                                                                        \
            void* dptr = sw_framebuffer_get_depth_addr(                 \
                depthBuffer, offset                                     \
            );                                                          \
                                                                        \
            if (ENABLE_DEPTH_TEST) {                                    \
                float depth = sw_framebuffer_read_depth(dptr);          \
                if (z > depth) continue;                                \
            }                                                           \
                                                                        \
            sw_framebuffer_write_depth(dptr, z);                        \
                                                                        \
            void* cptr = sw_framebuffer_get_color_addr(                 \
                colorBuffer, offset                                     \
            );                                                          \
                                                                        \
            if (ENABLE_COLOR_BLEND)                                     \
            {                                                           \
                float dstColor[4];                                      \
                sw_framebuffer_read_color(dstColor, cptr);              \
                                                                        \
                float srcColor[4];                                      \
                srcColor[0] = sw_lerp(v0->color[0], v1->color[0], t);   \
                srcColor[1] = sw_lerp(v0->color[1], v1->color[1], t);   \
                srcColor[2] = sw_lerp(v0->color[2], v1->color[2], t);   \
                srcColor[3] = sw_lerp(v0->color[3], v1->color[3], t);   \
                                                                        \
                sw_blend_colors(dstColor, srcColor);                    \
                sw_framebuffer_write_color(cptr, dstColor);             \
            }                                                           \
            else                                                        \
            {                                                           \
                float color[3];                                         \
                color[0] = sw_lerp(v0->color[0], v1->color[0], t);      \
                color[1] = sw_lerp(v0->color[1], v1->color[1], t);      \
                color[2] = sw_lerp(v0->color[2], v1->color[2], t);      \
                sw_framebuffer_write_color(cptr, color);                \
            }                                                           \
        }                                                               \
    }                                                                   \
    else {                                                              \
        for (int i = 0; i != endVal; i += sgnInc, j += decInc) {        \
            float t = (float)i * invEndVal;                             \
                                                                        \
            int x = x1 + i;                                             \
            int y = y1 + (j >> 16);                                     \
            float z = z1 + t * zDiff;                                   \
            int offset = y * fbWidth + x;                               \
                                                                        \
            void* dptr = sw_framebuffer_get_depth_addr(                 \
                depthBuffer, offset                                     \
            );                                                          \
                                                                        \
            if (ENABLE_DEPTH_TEST) {                                    \
                float depth = sw_framebuffer_read_depth(dptr);          \
                if (z > depth) continue;                                \
            }                                                           \
                                                                        \
            sw_framebuffer_write_depth(dptr, z);                        \
                                                                        \
            void* cptr = sw_framebuffer_get_color_addr(                 \
                colorBuffer, offset                                     \
            );                                                          \
                                                                        \
            if (ENABLE_COLOR_BLEND)                                     \
            {                                                           \
                float dstColor[4];                                      \
                sw_framebuffer_read_color(dstColor, cptr);              \
                                                                        \
                float srcColor[4];                                      \
                srcColor[0] = sw_lerp(v0->color[0], v1->color[0], t);   \
                srcColor[1] = sw_lerp(v0->color[1], v1->color[1], t);   \
                srcColor[2] = sw_lerp(v0->color[2], v1->color[2], t);   \
                srcColor[3] = sw_lerp(v0->color[3], v1->color[3], t);   \
                                                                        \
                sw_blend_colors(dstColor, srcColor);                    \
                sw_framebuffer_write_color(cptr, dstColor);             \
            }                                                           \
            else                                                        \
            {                                                           \
                float color[3];                                         \
                color[0] = sw_lerp(v0->color[0], v1->color[0], t);      \
                color[1] = sw_lerp(v0->color[1], v1->color[1], t);      \
                color[2] = sw_lerp(v0->color[2], v1->color[2], t);      \
                sw_framebuffer_write_color(cptr, color);                \
            }                                                           \
        }                                                               \
    }                                                                   \
}

#define DEFINE_LINE_THICK_RASTER(FUNC_NAME, RASTER_FUNC)                \
void FUNC_NAME(const sw_vertex_t* v1, const sw_vertex_t* v2)            \
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
    if (dx != 0 && abs(dy / dx) < 1) {                                  \
        int wy = (int)((RLSW.lineWidth - 1.0f) * abs(dx) / sqrtf(dx * dx + dy * dy)); \
        wy >>= 1; /* Division by 2 via bit shift */                     \
        for (int i = 1; i <= wy; i++) {                                 \
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
    else if (dy != 0) {                                                 \
        int wx = (int)((RLSW.lineWidth - 1.0f) * abs(dy) / sqrtf(dx * dx + dy * dy)); \
        wx >>= 1; /* Division by 2 via bit shift */                     \
        for (int i = 1; i <= wx; i++) {                                 \
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

static inline void sw_line_render(sw_vertex_t* vertices)
{
    if (!sw_line_clip_and_project(&vertices[0], &vertices[1])) {
        return;
    }

    if (RLSW.lineWidth >= 2.0f) {
        if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
            sw_line_thick_raster_DEPTH_BLEND(&vertices[0], &vertices[1]);
        }
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
            sw_line_thick_raster_BLEND(&vertices[0], &vertices[1]);
        }
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
            sw_line_thick_raster_DEPTH(&vertices[0], &vertices[1]);
        }
        else {
            sw_line_thick_raster(&vertices[0], &vertices[1]);
        }
    }
    else {
        if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
            sw_line_raster_DEPTH_BLEND(&vertices[0], &vertices[1]);
        }
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
            sw_line_raster_BLEND(&vertices[0], &vertices[1]);
        }
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
            sw_line_raster_DEPTH(&vertices[0], &vertices[1]);
        }
        else {
            sw_line_raster(&vertices[0], &vertices[1]);
        }
    }
}


/* === Point Rendering Part === */

static inline bool sw_point_clip_and_project(sw_vertex_t* v)
{
    if (v->homogeneous[3] != 1.0f) {
        for (int_fast8_t i = 0; i < 3; i++) {
            if (v->homogeneous[i] < -v->homogeneous[3] || v->homogeneous[i] > v->homogeneous[3]) {
                return false;
            }
        }
        v->homogeneous[3] = 1.0f / v->homogeneous[3];
        v->homogeneous[0] *= v->homogeneous[3];
        v->homogeneous[1] *= v->homogeneous[3];
        v->homogeneous[2] *= v->homogeneous[3];
    }

    sw_project_ndc_to_screen(v->screen, v->homogeneous);

    const int *min, *max;

    if (RLSW.stateFlags & SW_STATE_SCISSOR_TEST) {
        min = RLSW.scMin;
        max = RLSW.scMax;
    } else {
        min = RLSW.vpMin;
        max = RLSW.vpMax;
    }

    bool insideX = v->screen[0] - RLSW.pointRadius < max[0]
                && v->screen[0] + RLSW.pointRadius > min[0];

    bool insideY = v->screen[1] - RLSW.pointRadius < max[1]
                && v->screen[1] + RLSW.pointRadius > min[1];

    return insideX && insideY; 
}

#define DEFINE_POINT_RASTER(FUNC_NAME, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND, CHECK_BOUNDS) \
static inline void FUNC_NAME(int x, int y, float z, float color[4])         \
{                                                                           \
    if (CHECK_BOUNDS == 1)                                                  \
    {                                                                       \
        if (x < RLSW.vpMin[0] || x >= RLSW.vpMax[0]) return;                \
        if (y < RLSW.vpMin[1] || y >= RLSW.vpMax[1]) return;                \
    }                                                                       \
    else if (CHECK_BOUNDS == SW_SCISSOR_TEST)                               \
    {                                                                       \
        if (x < RLSW.scMin[0] || x >= RLSW.scMax[0]) return;                \
        if (y < RLSW.scMin[1] || y >= RLSW.scMax[1]) return;                \
    }                                                                       \
                                                                            \
    int offset = y * RLSW.framebuffer.width + x;                            \
                                                                            \
    void* dptr = sw_framebuffer_get_depth_addr(                             \
        RLSW.framebuffer.depth, offset                                      \
    );                                                                      \
                                                                            \
    if (ENABLE_DEPTH_TEST)                                                  \
    {                                                                       \
        float depth = sw_framebuffer_read_depth(dptr);                      \
        if (z > depth) return;                                              \
    }                                                                       \
                                                                            \
    sw_framebuffer_write_depth(dptr, z);                                    \
                                                                            \
    void* cptr = sw_framebuffer_get_color_addr(                             \
        RLSW.framebuffer.color, offset                                      \
    );                                                                      \
                                                                            \
    if (ENABLE_COLOR_BLEND)                                                 \
    {                                                                       \
        float dstColor[4];                                                  \
        sw_framebuffer_read_color(dstColor, cptr);                          \
                                                                            \
        sw_blend_colors(dstColor, color);                                   \
        sw_framebuffer_write_color(cptr, dstColor);                         \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        sw_framebuffer_write_color(cptr, color);                            \
    }                                                                       \
}

#define DEFINE_POINT_THICK_RASTER(FUNC_NAME, RASTER_FUNC)                   \
static inline void FUNC_NAME(sw_vertex_t* v)                                \
{                                                                           \
    int cx = v->screen[0];                                                  \
    int cy = v->screen[1];                                                  \
    float cz = v->homogeneous[2];                                           \
    int radius = RLSW.pointRadius;                                          \
    float* color = v->color;                                                \
                                                                            \
    int x = 0;                                                              \
    int y = radius;                                                         \
    int d = 3 - 2 * radius;                                                 \
                                                                            \
    while (x <= y) {                                                        \
        for (int i = -x; i <= x; i++) {                                     \
            RASTER_FUNC(cx + i, cy + y, cz, color);                         \
            RASTER_FUNC(cx + i, cy - y, cz, color);                         \
        }                                                                   \
        for (int i = -y; i <= y; i++) {                                     \
            RASTER_FUNC(cx + i, cy + x, cz, color);                         \
            RASTER_FUNC(cx + i, cy - x, cz, color);                         \
        }                                                                   \
        if (d > 0) {                                                        \
            y--;                                                            \
            d = d + 4 * (x - y) + 10;                                       \
        } else {                                                            \
            d = d + 4 * x + 6;                                              \
        }                                                                   \
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

static inline void sw_point_render(sw_vertex_t* v)
{
    if (!sw_point_clip_and_project(v)) {
        return;
    }

    if (RLSW.pointRadius >= 1.0f) {
        if (SW_STATE_CHECK(SW_STATE_SCISSOR_TEST)) {
            if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
                sw_point_thick_raster_DEPTH_BLEND_SCISSOR(v);
            }
            else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
                sw_point_thick_raster_BLEND_SCISSOR(v);
            }
            else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
                sw_point_thick_raster_DEPTH_SCISSOR(v);
            }
            else {
                sw_point_thick_raster_SCISSOR(v);
            }
        }
        else {
            if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
                sw_point_thick_raster_DEPTH_BLEND(v);
            }
            else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
                sw_point_thick_raster_BLEND(v);
            }
            else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
                sw_point_thick_raster_DEPTH(v);
            }
            else {
                sw_point_thick_raster(v);
            }
        }
    }
    else {
        if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
            sw_point_raster_DEPTH_BLEND(
                v->screen[0], v->screen[1],
                v->homogeneous[2], v->color
            );
        }
        else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
            sw_point_raster_BLEND(
                v->screen[0], v->screen[1],
                v->homogeneous[2], v->color
            );
        }
        else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
            sw_point_raster_DEPTH(
                v->screen[0], v->screen[1],
                v->homogeneous[2], v->color
            );
        }
        else {
            sw_point_raster(
                v->screen[0], v->screen[1],
                v->homogeneous[2], v->color
            );
        }
    }
}


/* === Polygon Modes Rendering Part === */

static inline void sw_poly_point_render(void)
{
    for (int i = 0; i < RLSW.vertexCounter; i++) {
        sw_point_render(&RLSW.vertexBuffer[i]);
    }
}

static inline void sw_poly_line_render(void)
{
    const sw_vertex_t* vertices = RLSW.vertexBuffer;
    int cm1 = RLSW.vertexCounter - 1;

    for (int i = 0; i < cm1; i++) {
        sw_line_render((sw_vertex_t[2]){
            vertices[i], vertices[i + 1]
        });
    }

    sw_line_render((sw_vertex_t[2]){
        vertices[cm1], vertices[0]
    });
}

static inline void sw_poly_fill_render(void)
{
    switch (RLSW.drawMode) {
    case SW_POINTS:
        sw_point_render(&RLSW.vertexBuffer[0]);
        break;
    case SW_LINES:
        sw_line_render(RLSW.vertexBuffer);
        break;
    case SW_TRIANGLES:
        sw_triangle_render();
        break;
    case SW_QUADS:
        sw_quad_render();
        break;
    }
}

/* === Some Validity Check Helper === */

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
    return (filter == SW_NEAREST || filter == SW_LINEAR);
}

static inline bool sw_is_texture_wrap_valid(int wrap)
{
    return (wrap == SW_REPEAT || wrap == SW_CLAMP);
}

static inline bool sw_is_draw_mode_valid(int mode)
{
    bool result = false;

    switch (mode) {
    case SW_POINTS:
    case SW_LINES:
    case SW_TRIANGLES:
    case SW_QUADS:
        result = true;
      break;
    default:
        break;
    }

    return result;
}

static inline bool sw_is_poly_mode_valid(int mode)
{
    bool result = false;

    switch (mode) {
    case SW_POINT:
    case SW_LINE:
    case SW_FILL:
        result = true;
      break;
    default:
        break;
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

    switch (blend) {
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
    case SW_SRC_ALPHA_SATURATE:
        result = true;
        break;
    default:
        break;
    }

    return result;
}

static inline bool sw_is_blend_dst_factor_valid(int blend)
{
    bool result = false;

    switch (blend) {
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
        result = true;
        break;
    default:
        break;
    }

    return result;
}

/* === Public Implementation === */

bool swInit(int w, int h)
{
    if (!sw_framebuffer_load(w, h)) {
        swClose(); return false;
    }

    swViewport(0, 0, w, h);
    swScissor(0, 0, w, h);

    RLSW.loadedTextures = SW_MALLOC(SW_MAX_TEXTURES * sizeof(sw_texture_t));
    if (RLSW.loadedTextures == NULL) { swClose(); return false; }

    RLSW.freeTextureIds = SW_MALLOC(SW_MAX_TEXTURES * sizeof(uint32_t));
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

    RLSW.vertexBuffer[0].color[0] = 1.0f;
    RLSW.vertexBuffer[0].color[1] = 1.0f;
    RLSW.vertexBuffer[0].color[2] = 1.0f;
    RLSW.vertexBuffer[0].color[3] = 1.0f;

    RLSW.vertexBuffer[0].texcoord[0] = 0.0f;
    RLSW.vertexBuffer[0].texcoord[1] = 0.0f;

    RLSW.srcFactor = SW_SRC_ALPHA;
    RLSW.dstFactor = SW_ONE_MINUS_SRC_ALPHA;

    RLSW.srcFactorFunc = sw_factor_src_alpha;
    RLSW.dstFactorFunc = sw_factor_one_minus_src_alpha;

    RLSW.polyMode = SW_FILL;
    RLSW.cullFace = SW_BACK;

    static const float defTex[3*2*2] =
    {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    RLSW.loadedTextures[0].pixels.cptr = defTex;
    RLSW.loadedTextures[0].width = 2;
    RLSW.loadedTextures[0].height = 2;
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
    for (int i = 1; i < RLSW.loadedTextureCount; i++) {
        sw_texture_t* texture = &RLSW.loadedTextures[i];
        if (sw_is_texture_valid(i) && texture->copy) {
            SW_FREE(texture->pixels.ptr);
        }
    }

    SW_FREE(RLSW.framebuffer.color);
    SW_FREE(RLSW.framebuffer.depth);
    SW_FREE(RLSW.loadedTextures);
    SW_FREE(RLSW.freeTextureIds);

    RLSW = (sw_context_t) { 0 };
}

bool swResizeFramebuffer(int w, int h)
{
    return sw_framebuffer_resize(w, h);
}

void swCopyFramebuffer(int x, int y, int w, int h, SWformat format, SWtype type, void* pixels)
{
    sw_pixelformat_e pFormat = sw_get_pixel_format(format, type);

    if (w <= 0) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (h <= 0) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (w > RLSW.framebuffer.width)
        w = RLSW.framebuffer.width;

    if (h > RLSW.framebuffer.height)
        h = RLSW.framebuffer.height;

    x = sw_clampi(x, 0, w);
    y = sw_clampi(y, 0, h);

    switch (pFormat) {

    case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        sw_framebuffer_copy_to_GRAYALPHA(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        sw_framebuffer_copy_to_GRAYALPHA(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        sw_framebuffer_copy_to_R5G6B5(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        sw_framebuffer_copy_to_R8G8B8(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        sw_framebuffer_copy_to_R5G5B5A1(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        sw_framebuffer_copy_to_R4G4B4A4(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        sw_framebuffer_copy_to_R8G8B8A8(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R32:
        sw_framebuffer_copy_to_R32(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        sw_framebuffer_copy_to_R32G32B32(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        sw_framebuffer_copy_to_R32G32B32A32(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R16:
        sw_framebuffer_copy_to_R16(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        sw_framebuffer_copy_to_R16G16B16(x, y, w, h, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        sw_framebuffer_copy_to_R16G16B16A16(x, y, w, h, pixels);
        break;

    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;

    }
}

void swBlitFramebuffer(int xDst, int yDst, int wDst, int hDst,
                       int xSrc, int ySrc, int wSrc, int hSrc,
                       SWformat format, SWtype type, void* pixels)
{
    sw_pixelformat_e pFormat = sw_get_pixel_format(format, type);

    if (wSrc <= 0) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (hSrc <= 0) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (wSrc > RLSW.framebuffer.width)
        wSrc = RLSW.framebuffer.width;

    if (hSrc > RLSW.framebuffer.height)
        hSrc = RLSW.framebuffer.height;

    xSrc = sw_clampi(xSrc, 0, wSrc);
    ySrc = sw_clampi(ySrc, 0, hSrc);

    switch (pFormat) {

    case SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        sw_framebuffer_blit_to_GRAYALPHA(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        sw_framebuffer_blit_to_GRAYALPHA(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        sw_framebuffer_blit_to_R5G6B5(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        sw_framebuffer_blit_to_R8G8B8(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        sw_framebuffer_blit_to_R5G5B5A1(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        sw_framebuffer_blit_to_R4G4B4A4(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        sw_framebuffer_blit_to_R8G8B8A8(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R32:
        sw_framebuffer_blit_to_R32(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        sw_framebuffer_blit_to_R32G32B32(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        sw_framebuffer_blit_to_R32G32B32A32(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R16:
        sw_framebuffer_blit_to_R16(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        sw_framebuffer_blit_to_R16G16B16(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;
    case SW_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        sw_framebuffer_blit_to_R16G16B16A16(xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc, pixels);
        break;

    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;

    }
}

void* swGetColorBuffer(int* w, int* h)
{
    if (w) *w = RLSW.framebuffer.width;
    if (h) *h = RLSW.framebuffer.height;

    return RLSW.framebuffer.color;
}

void swEnable(SWstate state)
{
    switch (state) {
    case SW_SCISSOR_TEST:
        RLSW.stateFlags |= SW_STATE_SCISSOR_TEST;
        break;
    case SW_TEXTURE_2D:
        RLSW.stateFlags |= SW_STATE_TEXTURE_2D;
        break;
    case SW_DEPTH_TEST:
        RLSW.stateFlags |= SW_STATE_DEPTH_TEST;
        break;
    case SW_CULL_FACE:
        RLSW.stateFlags |= SW_STATE_CULL_FACE;
        break;
    case SW_BLEND:
        RLSW.stateFlags |= SW_STATE_BLEND;
        break;
    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;
    }
}

void swDisable(SWstate state)
{
    switch (state) {
    case SW_SCISSOR_TEST:
        RLSW.stateFlags &= ~SW_STATE_SCISSOR_TEST;
        break;
    case SW_TEXTURE_2D:
        RLSW.stateFlags &= ~SW_STATE_TEXTURE_2D;
        break;
    case SW_DEPTH_TEST:
        RLSW.stateFlags &= ~SW_STATE_DEPTH_TEST;
        break;
    case SW_CULL_FACE:
        RLSW.stateFlags &= ~SW_STATE_CULL_FACE;
        break;
    case SW_BLEND:
        RLSW.stateFlags &= ~SW_STATE_BLEND;
        break;
    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;
    }
}

void swGetIntegerv(SWget name, int* v)
{
    switch (name) {
    case SW_MODELVIEW_STACK_DEPTH:
        *v = SW_MODELVIEW_STACK_DEPTH;
        break;
    case SW_PROJECTION_STACK_DEPTH:
        *v = SW_PROJECTION_STACK_DEPTH;
        break;
    case SW_TEXTURE_STACK_DEPTH:
        *v = SW_TEXTURE_STACK_DEPTH;
        break;
    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;
    }
}

void swGetFloatv(SWget name, float* v)
{
    switch (name) {
    case SW_COLOR_CLEAR_VALUE:
        v[0] = RLSW.clearColor[0];
        v[1] = RLSW.clearColor[1];
        v[2] = RLSW.clearColor[2];
        v[3] = RLSW.clearColor[3];
        break;
    case SW_CURRENT_COLOR:
        v[0] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[0];
        v[1] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[1];
        v[2] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[2];
        v[3] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].color[3];
        break;
    case SW_CURRENT_TEXTURE_COORDS:
        v[0] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].texcoord[0];
        v[1] = RLSW.vertexBuffer[RLSW.vertexCounter - 1].texcoord[1];
        break;
    case SW_MODELVIEW_MATRIX:
        for (int i = 0; i < 16; i++) {
            v[i] = RLSW.stackModelview[RLSW.stackModelviewCounter - 1][i];
        }
        break;
    case SW_PROJECTION_MATRIX:
        for (int i = 0; i < 16; i++) {
            v[i] = RLSW.stackProjection[RLSW.stackProjectionCounter - 1][i];
        }
        break;
    case SW_TEXTURE_MATRIX:
        for (int i = 0; i < 16; i++) {
            v[i] = RLSW.stackTexture[RLSW.stackTextureCounter - 1][i];
        }
        break;
    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;
    }
}

const char* swGetString(SWget name)
{
    const char* result = NULL;

    switch (name) {
    case SW_VENDOR:
        result = "RLSW Header";
        break;
    case SW_RENDERER:
        result = "RLSW Software Renderer";
        break;
    case SW_VERSION:
        result = "RLSW 1.0";
        break;
    case SW_EXTENSIONS:
        result = "None";
        break;
    default:
        RLSW.errCode = SW_INVALID_ENUM;
        break;
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
    if (width < 0 || height < 0) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.vpPos[0] = x;
    RLSW.vpPos[1] = y;
    RLSW.vpDim[0] = width;
    RLSW.vpDim[1] = height;

    RLSW.vpMin[0] = sw_clampi(x, 0, RLSW.framebuffer.width - 1);
    RLSW.vpMin[1] = sw_clampi(y, 0, RLSW.framebuffer.height - 1);
    RLSW.vpMax[0] = sw_clampi(x + width, 0, RLSW.framebuffer.width - 1);
    RLSW.vpMax[1] = sw_clampi(y + height, 0, RLSW.framebuffer.height - 1);
}

void swScissor(int x, int y, int width, int height)
{
    if (width < 0 || height < 0) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.scMin[0] = sw_clampi(x, 0, RLSW.framebuffer.width - 1);
    RLSW.scMin[1] = sw_clampi(y, 0, RLSW.framebuffer.height - 1);
    RLSW.scMax[0] = sw_clampi(x + width, 0, RLSW.framebuffer.width - 1);
    RLSW.scMax[1] = sw_clampi(y + height, 0, RLSW.framebuffer.height - 1);

    RLSW.scHMin[0] = (2.0f * (float)RLSW.scMin[0] / (float)RLSW.vpDim[0]) - 1.0f;
    RLSW.scHMax[0] = (2.0f * (float)RLSW.scMax[0] / (float)RLSW.vpDim[0]) - 1.0f;
    RLSW.scHMax[1] = 1.0f - (2.0f * (float)RLSW.scMin[1] / (float)RLSW.vpDim[1]);
    RLSW.scHMin[1] = 1.0f - (2.0f * (float)RLSW.scMax[1] / (float)RLSW.vpDim[1]);
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
    int size = RLSW.framebuffer.width * RLSW.framebuffer.height;

    if ((bitmask & (SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT)) == (SW_COLOR_BUFFER_BIT | SW_DEPTH_BUFFER_BIT)) {
        sw_framebuffer_fill(
            RLSW.framebuffer.color, RLSW.framebuffer.depth,
            size, RLSW.clearColor, RLSW.clearDepth
        );
    }
    else if (bitmask & (SW_COLOR_BUFFER_BIT)) {
        sw_framebuffer_fill_color(RLSW.framebuffer.color, size, RLSW.clearColor);
    }
    else if (bitmask & SW_DEPTH_BUFFER_BIT) {
        sw_framebuffer_fill_depth(RLSW.framebuffer.depth, size, RLSW.clearDepth);
    }
}

void swBlendFunc(SWfactor sfactor, SWfactor dfactor)
{
    if (!sw_is_blend_src_factor_valid(sfactor)
     || !sw_is_blend_dst_factor_valid(dfactor)) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }
    RLSW.srcFactor = sfactor;
    RLSW.dstFactor = dfactor;

    switch (sfactor) {
    case SW_ZERO:
        RLSW.srcFactorFunc = sw_factor_zero;
        break;
    case SW_ONE:
        RLSW.srcFactorFunc = sw_factor_one;
        break;
    case SW_SRC_COLOR:
        RLSW.srcFactorFunc = sw_factor_src_color;
        break;
    case SW_ONE_MINUS_SRC_COLOR:
        RLSW.srcFactorFunc = sw_factor_one_minus_src_color;
        break;
    case SW_SRC_ALPHA:
        RLSW.srcFactorFunc = sw_factor_src_alpha;
        break;
    case SW_ONE_MINUS_SRC_ALPHA:
        RLSW.srcFactorFunc = sw_factor_one_minus_src_alpha;
        break;
    case SW_DST_ALPHA:
        RLSW.srcFactorFunc = sw_factor_dst_alpha;
        break;
    case SW_ONE_MINUS_DST_ALPHA:
        RLSW.srcFactorFunc = sw_factor_one_minus_dst_alpha;
        break;
    case SW_DST_COLOR:
        RLSW.srcFactorFunc = sw_factor_dst_color;
        break;
    case SW_ONE_MINUS_DST_COLOR:
        RLSW.srcFactorFunc = sw_factor_one_minus_dst_color;
        break;
    case SW_SRC_ALPHA_SATURATE:
        RLSW.srcFactorFunc = sw_factor_src_alpha_saturate;
        break;
    }

    switch (dfactor) {
    case SW_ZERO:
        RLSW.srcFactorFunc = sw_factor_zero;
        break;
    case SW_ONE:
        RLSW.srcFactorFunc = sw_factor_one;
        break;
    case SW_SRC_COLOR:
        RLSW.srcFactorFunc = sw_factor_src_color;
        break;
    case SW_ONE_MINUS_SRC_COLOR:
        RLSW.srcFactorFunc = sw_factor_one_minus_src_color;
        break;
    case SW_SRC_ALPHA:
        RLSW.srcFactorFunc = sw_factor_src_alpha;
        break;
    case SW_ONE_MINUS_SRC_ALPHA:
        RLSW.srcFactorFunc = sw_factor_one_minus_src_alpha;
        break;
    case SW_DST_ALPHA:
        RLSW.srcFactorFunc = sw_factor_dst_alpha;
        break;
    case SW_ONE_MINUS_DST_ALPHA:
        RLSW.srcFactorFunc = sw_factor_one_minus_dst_alpha;
        break;
    case SW_DST_COLOR:
        RLSW.srcFactorFunc = sw_factor_dst_color;
        break;
    case SW_ONE_MINUS_DST_COLOR:
        RLSW.srcFactorFunc = sw_factor_one_minus_dst_color;
        break;
    case SW_SRC_ALPHA_SATURATE:
        // NOTE: Should never be reached
        break;
    }
}

void swPolygonMode(SWpoly mode)
{
    if (!sw_is_poly_mode_valid(mode)) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }
    RLSW.polyMode = mode;
}

void swCullFace(SWface face)
{
    if (!sw_is_face_valid(face)) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }
    RLSW.cullFace = face;
}

void swPointSize(float size)
{
    RLSW.pointRadius = floorf(size * 0.5f);
}

void swLineWidth(float width)
{
    RLSW.lineWidth = roundf(width);
}

void swMatrixMode(SWmatrix mode)
{
    switch (mode) {
    case SW_PROJECTION:
        RLSW.currentMatrix = &RLSW.stackProjection[RLSW.stackProjectionCounter - 1];
        break;
    case SW_MODELVIEW:
        RLSW.currentMatrix = &RLSW.stackModelview[RLSW.stackModelviewCounter - 1];
        break;
    case SW_TEXTURE:
        RLSW.currentMatrix = &RLSW.stackTexture[RLSW.stackTextureCounter - 1];
        break;
    default:
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    RLSW.currentMatrixMode = mode;
}

void swPushMatrix(void)
{
    switch (RLSW.currentMatrixMode) {

    case SW_PROJECTION:
        {
            if (RLSW.stackProjectionCounter >= SW_MAX_PROJECTION_STACK_SIZE) {
                RLSW.errCode = SW_STACK_OVERFLOW;
                return;
            }

            int iOld = RLSW.stackProjectionCounter - 1;
            int iNew = RLSW.stackProjectionCounter++;

            for (int i = 0; i < 16; i++) {
                RLSW.stackProjection[iNew][i] = RLSW.stackProjection[iOld][i];
            }

            RLSW.currentMatrix = &RLSW.stackProjection[iNew];
        }
        break;

    case SW_MODELVIEW:
        {
            if (RLSW.stackModelviewCounter >= SW_MAX_MODELVIEW_STACK_SIZE) {
                RLSW.errCode = SW_STACK_OVERFLOW;
                return;
            }

            int iOld = RLSW.stackModelviewCounter - 1;
            int iNew = RLSW.stackModelviewCounter++;

            for (int i = 0; i < 16; i++) {
                RLSW.stackModelview[iNew][i] = RLSW.stackModelview[iOld][i];
            }

            RLSW.currentMatrix = &RLSW.stackModelview[iNew];
        }
        break;

    case SW_TEXTURE:
        {
            if (RLSW.stackTextureCounter >= SW_MAX_TEXTURE_STACK_SIZE) {
                RLSW.errCode = SW_STACK_OVERFLOW;
                return;
            }

            int iOld = RLSW.stackTextureCounter - 1;
            int iNew = RLSW.stackTextureCounter++;

            for (int i = 0; i < 16; i++) {
                RLSW.stackTexture[iNew][i] = RLSW.stackTexture[iOld][i];
            }

            RLSW.currentMatrix = &RLSW.stackTexture[iNew];
        }
        break;

    }
}

void swPopMatrix(void)
{
    switch (RLSW.currentMatrixMode) {

    case SW_PROJECTION:
        {
            if (RLSW.stackProjectionCounter <= 0) {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackProjection[--RLSW.stackProjectionCounter];
            RLSW.isDirtyMVP = true; //< The MVP is considered to have been changed
        }
        break;

    case SW_MODELVIEW:
        {
            if (RLSW.stackModelviewCounter <= 0) {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackModelview[--RLSW.stackModelviewCounter];
            RLSW.isDirtyMVP = true; //< The MVP is considered to have been changed
        }
        break;

    case SW_TEXTURE:
        {
            if (RLSW.stackTextureCounter <= 0) {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }

            RLSW.currentMatrix = &RLSW.stackTexture[--RLSW.stackTextureCounter];
        }
        break;

    }
}

void swLoadIdentity(void)
{
    sw_matrix_id(*RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swTranslatef(float x, float y, float z)
{
    sw_matrix_t mat;
    sw_matrix_id(mat);

    mat[12] = x;
    mat[13] = y;
    mat[14] = z;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swRotatef(float angle, float x, float y, float z)
{
    angle *= SW_DEG2RAD;

    float lengthSq = x*x + y*y + z*z;

    if (lengthSq != 1.0f && lengthSq != 0.0f) {
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

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swScalef(float x, float y, float z)
{
    sw_matrix_t mat;

    mat[0]  = x, mat[1]  = 0, mat[2]  = 0, mat[3]  = 0;
    mat[4]  = 0, mat[5]  = y, mat[6]  = 0, mat[7]  = 0;
    mat[8]  = 0, mat[9]  = 0, mat[10] = z, mat[11] = 0;
    mat[12] = 0, mat[13] = 0, mat[14] = 0, mat[15] = 1;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swMultMatrixf(const float* mat)
{
    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    sw_matrix_t mat;

    double rl = right - left;
    double tb = top - bottom;
    double fn = zfar - znear;

    mat[0] = (znear*2.0)/rl;
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    mat[3] = 0.0f;

    mat[4] = 0.0f;
    mat[5] = (znear*2.0)/tb;
    mat[6] = 0.0f;
    mat[7] = 0.0f;

    mat[8] = (right + left)/rl;
    mat[9] = (top + bottom)/tb;
    mat[10] = -(zfar + znear)/fn;
    mat[11] = -1.0f;

    mat[12] = 0.0f;
    mat[13] = 0.0f;
    mat[14] = -(zfar*znear*2.0)/fn;
    mat[15] = 0.0f;

    sw_matrix_mul(*RLSW.currentMatrix, *RLSW.currentMatrix, mat);

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    sw_matrix_t mat;

    double rl = right - left;
    double tb = top - bottom;
    double fn = zfar - znear;

    mat[0] = 2.0f/rl;
    mat[1] = 0.0f;
    mat[2] = 0.0f;
    mat[3] = 0.0f;

    mat[4] = 0.0f;
    mat[5] = 2.0f/tb;
    mat[6] = 0.0f;
    mat[7] = 0.0f;

    mat[8] = 0.0f;
    mat[9] = 0.0f;
    mat[10] = -2.0f/fn;
    mat[11] = 0.0f;

    mat[12] = -(left + right)/rl;
    mat[13] = -(top + bottom)/tb;
    mat[14] = -(zfar + znear)/fn;
    mat[15] = 1.0f;

    sw_matrix_mul(*RLSW.currentMatrix, *RLSW.currentMatrix, mat);

    if (RLSW.currentMatrixMode != SW_TEXTURE) {
        RLSW.isDirtyMVP = true;
    }
}

void swBegin(SWdraw mode)
{
    /* --- Check if the draw mode is valid --- */

    if (!sw_is_draw_mode_valid(mode)) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    /* --- Recalculate the MVP if this is needed --- */

    if (RLSW.isDirtyMVP) {
        sw_matrix_mul(
            RLSW.matMVP,
            RLSW.stackModelview[RLSW.stackModelviewCounter - 1],
            RLSW.stackProjection[RLSW.stackProjectionCounter - 1]
        );
        RLSW.isDirtyMVP = false;
    }

    /* --- Obtaining the number of vertices needed for this primitive --- */

    switch (mode) {
    case SW_POINTS:
        RLSW.reqVertices = 1;
        break;
    case SW_LINES:
        RLSW.reqVertices = 2;
        break;
    case SW_TRIANGLES:
        RLSW.reqVertices = 3;
        break;
    case SW_QUADS:
        RLSW.reqVertices = 4;
        break;
    }
    
    /* --- Initialize some values --- */

    RLSW.vertexCounter = 0;
    RLSW.drawMode = mode;
}

void swEnd(void)
{
    RLSW.drawMode = 0;
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

void swVertex2fv(const float* v)
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

void swVertex3fv(const float* v)
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

void swVertex4fv(const float* v)
{
    /* --- Copy the position in the current vertex --- */

    sw_vertex_t* vertex = &RLSW.vertexBuffer[RLSW.vertexCounter++];

    for (int i = 0; i < 4; i++) {
        vertex->position[i] = v[i];
    }

    /* --- Calculation of homogeneous coordinates --- */

    const sw_matrix_t* mat = &RLSW.matMVP;

    vertex->homogeneous[0] = (*mat)[0] * v[0] + (*mat)[4] * v[1] + (*mat)[8] * v[2] + (*mat)[12] * v[3];
    vertex->homogeneous[1] = (*mat)[1] * v[0] + (*mat)[5] * v[1] + (*mat)[9] * v[2] + (*mat)[13] * v[3];
    vertex->homogeneous[2] = (*mat)[2] * v[0] + (*mat)[6] * v[1] + (*mat)[10] * v[2] + (*mat)[14] * v[3];
    vertex->homogeneous[3] = (*mat)[3] * v[0] + (*mat)[7] * v[1] + (*mat)[11] * v[2] + (*mat)[15] * v[3];

    /* --- Immediate rendering of the primitive if the required number is reached --- */

    if (RLSW.vertexCounter == RLSW.reqVertices) {
        switch (RLSW.polyMode) {
        case SW_FILL:
            sw_poly_fill_render();
            break;
        case SW_LINE:
            sw_poly_line_render();
            break;
        case SW_POINT:
            sw_poly_point_render();
            break;
        }
        RLSW.vertexBuffer[0] = RLSW.vertexBuffer[RLSW.reqVertices - 1];
        RLSW.vertexCounter = 0;
    }
    else {
        RLSW.vertexBuffer[RLSW.vertexCounter] = RLSW.vertexBuffer[RLSW.vertexCounter - 1];
    }
}

void swColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
    float cv[4];
    cv[0] = (float)r / 255;
    cv[1] = (float)g / 255;
    cv[2] = (float)b / 255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3ubv(const uint8_t* v)
{
    float cv[4];
    cv[0] = (float)v[0] / 255;
    cv[1] = (float)v[1] / 255;
    cv[2] = (float)v[2] / 255;
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

void swColor3fv(const float* v)
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
    cv[0] = (float)r / 255;
    cv[1] = (float)g / 255;
    cv[2] = (float)b / 255;
    cv[3] = (float)a / 255;

    swColor4fv(cv);
}

void swColor4ubv(const uint8_t* v)
{
    float cv[4];
    cv[0] = (float)v[0] / 255;
    cv[1] = (float)v[1] / 255;
    cv[2] = (float)v[2] / 255;
    cv[3] = (float)v[3] / 255;

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

void swColor4fv(const float* v)
{
    for (int i = 0; i < 4; i++) {
        RLSW.vertexBuffer[RLSW.vertexCounter].color[i] = v[i];
    }
}

void swTexCoord2f(float u, float v)
{
    const sw_matrix_t* mat = &RLSW.stackTexture[RLSW.stackTextureCounter - 1];

    float s = (*mat)[0]*u + (*mat)[4]*v + (*mat)[12];
    float t = (*mat)[1]*u + (*mat)[5]*v + (*mat)[13];

    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[0] = s;
    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[1] = t;
}

void swTexCoord2fv(const float* v)
{
    const sw_matrix_t* mat = &RLSW.stackTexture[RLSW.stackTextureCounter - 1];

    float s = (*mat)[0]*v[0] + (*mat)[4]*v[1] + (*mat)[12];
    float t = (*mat)[1]*v[0] + (*mat)[5]*v[1] + (*mat)[13];

    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[0] = s;
    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[1] = t;
}

void swBindArray(SWarray type, void *buffer)
{
    switch (type) {
    case SW_VERTEX_ARRAY:
        RLSW.array.positions = buffer;
        break;
    case SW_TEXTURE_COORD_ARRAY:
        RLSW.array.texcoords = buffer;
        break;
    case SW_COLOR_ARRAY:
        RLSW.array.colors = buffer;
        break;
    default:
        break;
    }
}

void swDrawArrays(SWdraw mode, int offset, int count)
{
    if (RLSW.array.positions == 0) {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    swBegin(mode);
    {
        swTexCoord2f(0.0f, 0.0f);
        swColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        for (int i = offset; i < count; i++) {
            if (RLSW.array.texcoords) {
                swTexCoord2fv(RLSW.array.texcoords + 2 * i);
            }
            if (RLSW.array.colors) {
                swColor4ubv(RLSW.array.colors + 4 * i);
            }
            swVertex3fv(RLSW.array.positions + 3 * i);
        }
    }
    swEnd();
}

void swGenTextures(int count, uint32_t* textures)
{
    if (count == 0 || textures == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (RLSW.loadedTextureCount >= SW_MAX_TEXTURES) {
            RLSW.errCode = SW_STACK_OVERFLOW; //< Out of memory, not really stack overflow
            return;
        }
        uint32_t id = 0;
        if (RLSW.freeTextureIdCount > 0) {
            id = RLSW.freeTextureIds[--RLSW.freeTextureIdCount];
        } 
        else {
            id = RLSW.loadedTextureCount++;
        }
        RLSW.loadedTextures[id] = RLSW.loadedTextures[0];
        textures[i] = id;
    }
}

void swDeleteTextures(int count, uint32_t* textures)
{
    if (count == 0 || textures == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (!sw_is_texture_valid(textures[i])) {
            RLSW.errCode = SW_INVALID_VALUE;
            continue;
        }
        if (RLSW.loadedTextures[textures[i]].copy) {
            SW_FREE(RLSW.loadedTextures[textures[i]].pixels.ptr);
        }
        RLSW.loadedTextures[textures[i]].pixels.cptr = NULL;
        RLSW.freeTextureIds[RLSW.freeTextureIdCount++] = textures[i];
    }
}

void swTexImage2D(int width, int height, SWformat format, SWtype type, bool copy, const void* data)
{
    uint32_t id = RLSW.currentTexture;

    if (!sw_is_texture_valid(id)) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    int pixelFormat = sw_get_pixel_format(format, type);

    if (pixelFormat <= SW_PIXELFORMAT_UNKNOWN) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }

    sw_texture_t* texture = &RLSW.loadedTextures[id];

    if (copy) {
        int bytes = sw_get_pixel_bytes(pixelFormat);
        int size = bytes * width * height;
        texture->pixels.ptr = SW_MALLOC(size);
        if (texture->pixels.ptr == NULL) {
            RLSW.errCode = SW_STACK_OVERFLOW;   //< Out of memory...
            return;
        }
        for (int i = 0; i < size; i++) {
            ((uint8_t*)texture->pixels.ptr)[i] = ((uint8_t*)data)[i];
        }
    }
    else {
        texture->pixels.cptr = data;
    }

    texture->width = width;
    texture->height = height;
    texture->format = pixelFormat;
    texture->tx = 1.0f / width;
    texture->ty = 1.0f / height;
    texture->copy = copy;
}

void swTexParameteri(int param, int value)
{
    uint32_t id = RLSW.currentTexture;

    if (!sw_is_texture_valid(id)) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    sw_texture_t* texture = &RLSW.loadedTextures[id];

    switch (param) {

    case SW_TEXTURE_MIN_FILTER:
        if (!sw_is_texture_filter_valid(value)) {
            RLSW.errCode = SW_INVALID_ENUM;
            return;
        }
        texture->minFilter = value;
        break;

    case SW_TEXTURE_MAG_FILTER:
        if (!sw_is_texture_filter_valid(value)) {
            RLSW.errCode = SW_INVALID_ENUM;
            return;
        }
        texture->magFilter = value;
        break;

    case SW_TEXTURE_WRAP_S:
        if (!sw_is_texture_wrap_valid(value)) {
            RLSW.errCode = SW_INVALID_ENUM;
            return;
        }
        texture->sWrap = value;
        break;

    case SW_TEXTURE_WRAP_T:
        if (!sw_is_texture_wrap_valid(value)) {
            RLSW.errCode = SW_INVALID_ENUM;
            return;
        }
        texture->tWrap = value;
        break;

    default:
        RLSW.errCode = SW_INVALID_ENUM;
        return;

    }
}

void swBindTexture(uint32_t id)
{
    if (id >= SW_MAX_TEXTURES) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    if (RLSW.loadedTextures[id].pixels.cptr == NULL) {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    RLSW.currentTexture = id;
}

#endif // RLSW_IMPL
