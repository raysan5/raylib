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

#ifndef SW_FREE
#   define SW_FREE(ptr) free(ptr)
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


/* === OpenGL Definitions === */

#define GL_TEXTURE_2D               0x0DE1
#define GL_DEPTH_TEST               0x0B71
#define GL_CULL_FACE                0x0B44
#define GL_BLEND                    0x0BE2

#define GL_MODELVIEW                0x1700
#define GL_PROJECTION               0x1701
#define GL_TEXTURE                  0x1702

#define GL_VERTEX_ARRAY             0x8074
#define GL_NORMAL_ARRAY             0x8075
#define GL_COLOR_ARRAY              0x8076
//#define GL_INDEX_ARRAY            0x8077
#define GL_TEXTURE_COORD_ARRAY      0x8078

#define GL_POINTS                   0x0000
#define GL_LINES                    0x0001
//#define GL_LINE_LOOP              0x0002
//#define GL_LINE_STRIP             0x0003
#define GL_TRIANGLES                0x0004
//#define GL_TRIANGLE_STRIP         0x0005
//#define GL_TRIANGLE_FAN           0x0006
#define GL_QUADS                    0x0007
//#define GL_QUAD_STRIP             0x0008
//#define GL_POLYGON                0x0009

//#define GL_CW                     0x0900
//#define GL_CCW                    0x0901

#define GL_FRONT                    0x0404
#define GL_BACK                     0x0405

#define GL_ZERO                     0
#define GL_ONE                      1
#define GL_SRC_COLOR                0x0300
#define GL_ONE_MINUS_SRC_COLOR      0x0301
#define GL_SRC_ALPHA                0x0302
#define GL_ONE_MINUS_SRC_ALPHA      0x0303
#define GL_DST_ALPHA                0x0304
#define GL_ONE_MINUS_DST_ALPHA      0x0305
#define GL_DST_COLOR                0x0306
#define GL_ONE_MINUS_DST_COLOR      0x0307
#define GL_SRC_ALPHA_SATURATE       0x0308

#define GL_NEAREST                  0x2600
#define GL_LINEAR                   0x2601

#define GL_REPEAT                   0x2901
#define GL_CLAMP_TO_EDGE            0x812F  //< (OpenGL 1.2)
#define GL_MIRRORED_REPEAT          0x8370  //< (OpenGL 2.0)

#define GL_TEXTURE_MAG_FILTER       0x2800
#define GL_TEXTURE_MIN_FILTER       0x2801

#define GL_TEXTURE_WRAP_S           0x2802
#define GL_TEXTURE_WRAP_T           0x2803

#define GL_NO_ERROR                 0
#define GL_INVALID_ENUM             0x0500
#define GL_INVALID_VALUE            0x0501
#define GL_INVALID_OPERATION        0x0502
#define GL_STACK_OVERFLOW           0x0503
#define GL_STACK_UNDERFLOW          0x0504
#define GL_OUT_OF_MEMORY            0x0505


/* === RLSW Enums === */

typedef enum {
    SW_TEXTURE_2D = GL_TEXTURE_2D,
    SW_DEPTH_TEST = GL_DEPTH_TEST,
    SW_CULL_FACE = GL_CULL_FACE,
    SW_BLEND = GL_BLEND
} SWstate;

typedef enum {
    SW_PROJECTION = GL_PROJECTION,
    SW_MODELVIEW = GL_MODELVIEW,
    SW_TEXTURE = GL_TEXTURE
} SWmatrix;

typedef enum {
    SW_VERTEX_ARRAY = GL_VERTEX_ARRAY,
    SW_TEXTURE_COORD_ARRAY = GL_TEXTURE_COORD_ARRAY,
    SW_NORMAL_ARRAY = GL_NORMAL_ARRAY,
    SW_COLOR_ARRAY = GL_COLOR_ARRAY
} SWarray;

typedef enum {
    SW_POINTS = GL_POINTS,
    SW_LINES = GL_LINES,
    SW_TRIANGLES = GL_TRIANGLES,
    SW_QUADS = GL_QUADS,
} SWdraw;

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
    SW_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
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
    SW_PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
    SW_PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
    SW_PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
    SW_PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
    SW_PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
    SW_PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
    SW_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
    SW_PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
    SW_PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
    SW_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
    SW_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
} SWpixelformat;

typedef enum {
    SW_NEAREST = GL_NEAREST,
    SW_LINEAR = GL_LINEAR
} SWfilter;

typedef enum {
    SW_REPEAT = GL_REPEAT,
    SW_CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    SW_MIRRORED_REPEAT = GL_MIRRORED_REPEAT
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

void swInit(int w, int h);
void swClose(void);

void swEnable(SWstate state);
void swDisable(SWstate state);

void* swGetColorBuffer(int* w, int* h);

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

void swViewport(int x, int y, int width, int height);

void swClearColor(float r, float g, float b, float a);
void swClear(void);

void swBlendFunc(SWfactor sfactor, SWfactor dfactor);
void swCullFace(SWface face);

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

void swColor1ui(uint32_t color);
void swColor3ub(uint8_t r, uint8_t g, uint8_t b);
void swColor3ubv(const uint8_t* v);
void swColor3us(uint16_t r, uint16_t g, uint16_t b);
void swColor3usv(const uint16_t* v);
void swColor3ui(uint32_t r, uint32_t g, uint32_t b);
void swColor3uiv(const uint32_t* v);
void swColor3f(float r, float g, float b);
void swColor3fv(const float* v);
void swColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void swColor4ubv(const uint8_t* v);
void swColor4us(uint16_t r, uint16_t g, uint16_t b, uint16_t a);
void swColor4usv(const uint16_t* v);
void swColor4ui(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
void swColor4uiv(const uint32_t* v);
void swColor4f(float r, float g, float b, float a);
void swColor4fv(const float* v);

void swTexCoord2f(float u, float v);
void swTexCoordfv(const float* v);

void swNormal3f(float x, float y, float z);
void swNormal3fv(const float* v);

void swBindArray(SWarray type, void *buffer);
void swDrawArrays(SWdraw mode, int offset, int count);

uint32_t swLoadTexture(const void *data, int width, int height, int format, int mipmapCount);
void swUnloadTexture(uint32_t id);

void swTextureParameters(uint32_t id, int param, int value);
void swBindTexture(uint32_t id);

#endif // RLSW_H


#ifdef RLSW_IMPL

#include <stdlib.h>
#include <math.h>

/* === Defines and Macros === */

#define SW_PI 3.14159265358979323846f
#define SW_DEG2RAD (SW_PI/180.0f)
#define SW_RAD2DEG (180.0f/SW_PI)

#define SW_STATE_CHECK(flags) ((RLSW.stateFlags & (flags)) == (flags))

#define SW_STATE_TEXTURE_2D     (1 << 0)
#define SW_STATE_DEPTH_TEST     (1 << 1)
#define SW_STATE_CULL_FACE      (1 << 2)
#define SW_STATE_BLEND          (1 << 3)

#define SW_CLIP_INSIDE  (0x00) // 0000
#define SW_CLIP_LEFT    (0x01) // 0001
#define SW_CLIP_RIGHT   (0x02) // 0010
#define SW_CLIP_BOTTOM  (0x04) // 0100
#define SW_CLIP_TOP     (0x08) // 1000

/* === Internal Structs === */

typedef float sw_matrix_t[4*4];
typedef uint16_t sw_half_t;

typedef struct {

    float position[4];                  // Position coordinates
    float normal[3];                    // Normal vector
    float texcoord[2];                  // Texture coordinates
    float color[4];                     // Color

    float homogeneous[4];               // Homogeneous coordinates
    float screen[2];                    // Screen coordinates

} sw_vertex_t;

typedef struct {

    const void* pixels;
    int width;
    int height;
    int format;

    SWfilter minFilter;
    SWfilter magFilter;

    SWwrap sWrap;
    SWwrap tWrap;

    float tx;
    float ty;

} sw_texture_t;

typedef struct {
    uint8_t *color;     // 32-bit RGBA color buffer
    uint16_t *depth;    // 16-bit fixed fract buffer
    int width, height;
} sw_framebuffer_t;

typedef struct {

    sw_framebuffer_t framebuffer;
    uint8_t clearColor[4];                                // Color used to clear the screen
    uint16_t clearDepth;                                  // Depth value used to clear the screen

    uint32_t currentTexture;
    sw_matrix_t *currentMatrix;

    int vpPos[2];                                               // Represents the top-left corner of the viewport
    int vpDim[2];                                               // Represents the dimensions of the viewport (minus one)
    int vpMin[2];                                               // Represents the minimum renderable point of the viewport (top-left)
    int vpMax[2];                                               // Represents the maximum renderable point of the viewport (bottom-right)

    struct {
        float* positions;
        float* texcoords;
        float* normals;
        uint8_t* colors;
    } array;

    sw_vertex_t vertexBuffer[4];                                // Buffer used for storing primitive vertices, used for processing and rendering
    int vertexCounter;                                          // Number of vertices in 'ctx.vertexBuffer'

    SWdraw drawMode;                                            // Current polygon filling mode (e.g., lines, triangles)
    float pointSize;                                            // Rasterized point size
    float lineWidth;                                            // Rasterized line width

    sw_matrix_t matProjection;                                  // Projection matrix, user adjustable
    sw_matrix_t matTexture;                                     // Texture matrix, user adjustable
    sw_matrix_t matModel;                                       // Model matrix, user adjustable (the one used if we push in SW_MODELVIEW mode)
    sw_matrix_t matView;                                        // View matrix, user adjustable (the default one used in SW_MODELVIEW mode)
    sw_matrix_t matMVP;                                         // Model view projection matrix, calculated and used internally

    sw_matrix_t stackProjection[SW_MAX_PROJECTION_STACK_SIZE];  // Projection matrix stack for push/pop operations
    sw_matrix_t stackModelview[SW_MAX_MODELVIEW_STACK_SIZE];    // Modelview matrix stack for push/pop operations
    sw_matrix_t stackTexture[SW_MAX_TEXTURE_STACK_SIZE];        // Texture matrix stack for push/pop operations
    uint32_t stackProjectionCounter;                            // Counter for matrix stack operations
    uint32_t stackModelviewCounter;                             // Counter for matrix stack operations
    uint32_t stackTextureCounter;                               // Counter for matrix stack operations

    SWmatrix currentMatrixMode;                                 // Current matrix mode (e.g., sw_MODELVIEW, sw_PROJECTION)
    bool modelMatrixUsed;                                       // Flag indicating if the model matrix is used

    SWfactor srcFactor;
    SWfactor dstFactor;

    SWface cullFace;                                            // Faces to cull
    SWerrcode errCode;                                          // Last error code

    sw_texture_t* loadedTextures;
    int loadedTextureCount;

    uint32_t* freeTextureIds;
    int freeTextureIdCount;

    uint32_t stateFlags;

} sw_data_t;


/* === Global Data === */

static sw_data_t RLSW = { 0 };


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

static inline float sw_saturate(float x)
{
    // After several comparisons, this saturation method
    // seems to be the most optimized by GCC and Clang,
    // and it does not produce any conditional branching.

    // However, it is possible that a clamp could be
    // more efficient on certain platforms.
    // Comparisons will need to be made.

    // SEE: https://godbolt.org/z/5qYznK5zj

    // Saturation from below: max(0, x)
    float y = 0.5f * (x + fabsf(x));

    // Saturation from above: min(1, y)
    return y - 0.5f * ((y - 1.0f) + fabsf(y - 1.0f));

    // return (x < 0.0f) ? 0.0f : ((x > 1.0f) ? 1.0f : x);
}

static inline float sw_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static inline sw_vertex_t sw_lerp_vertex(const sw_vertex_t* a, const sw_vertex_t* b, float t)
{
    sw_vertex_t result;
    for (int i = 0; i < sizeof(sw_vertex_t) / sizeof(float); i++) {
        ((float*)&result)[i] = sw_lerp(((float*)a)[i], ((float*)b)[i], t);
    }
    return result;
}


/* === Pixel Format Conversion Part === */

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

static inline void sw_get_pixel_grayscale(float* color, const void* pixels, uint32_t offset)
{
    float gray = (float)((uint8_t*)pixels)[offset] / 255;

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
    float gray = (float)((uint8_t*)pixels)[2 * offset] / 255;
    float alpha = (float)((uint8_t*)pixels)[2 * offset + 1] / 255;

    color[0] = gray;
    color[1] = gray;
    color[2] = gray;
    color[3] = alpha;
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
    const uint8_t* pixel = (uint8_t*)pixels + 3 * offset;

    color[0] = (float)pixel[0] / 255;
    color[1] = (float)pixel[1] / 255;
    color[2] = (float)pixel[2] / 255;
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

    color[0] = (float)pixel[0] / 255;
    color[1] = (float)pixel[1] / 255;
    color[2] = (float)pixel[2] / 255;
    color[3] = (float)pixel[3] / 255;
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

static inline void sw_get_pixel(float* color, const void* pixels, uint32_t offset, SWpixelformat format)
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

    case SW_PIXELFORMAT_COMPRESSED_DXT1_RGB:
    case SW_PIXELFORMAT_COMPRESSED_DXT1_RGBA:
    case SW_PIXELFORMAT_COMPRESSED_DXT3_RGBA:
    case SW_PIXELFORMAT_COMPRESSED_DXT5_RGBA:
    case SW_PIXELFORMAT_COMPRESSED_ETC1_RGB:
    case SW_PIXELFORMAT_COMPRESSED_ETC2_RGB:
    case SW_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
    case SW_PIXELFORMAT_COMPRESSED_PVRT_RGB:
    case SW_PIXELFORMAT_COMPRESSED_PVRT_RGBA:
    case SW_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA:
    case SW_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA:
        break;

    }
}


/* === Texture Sampling Part === */

static inline void sw_texture_map_repeat(int* out, float in, int max)
{
    // Upscale to nearest texture coordinates
    // NOTE: We use '(int)(x+0.5)' although this is incorrect
    //       regarding the direction of rounding in case of negative values
    //       and also less accurate than roundf, but it remains so much more
    //       efficient that it is preferable for now to opt for this option.

    *out = abs((int)((in - (int)in) * (max - 1) + 0.5f));
}

static inline void sw_texture_map_clamp_to_edge(int* out, float in, int max)
{
    *out = (int)(sw_saturate(in) * (max - 1) + 0.5f);
}

static inline void sw_texture_map_mirrored_repeat(int* out, float in, int max)
{
    in = fmodf(fabsf(in), 2);
    if (in > 1.0f) in = 1.0f - (in - 1.0f);
    *out = (int)(in * (max - 1) + 0.5f);
}

static inline void sw_texture_map(int* out, float in, int max, SWwrap mode)
{
    switch (mode) {
    case SW_REPEAT:
        sw_texture_map_repeat(out, in, max);
        break;
    case SW_CLAMP_TO_EDGE:
        sw_texture_map_clamp_to_edge(out, in, max);
        break;
    case SW_MIRRORED_REPEAT:
        sw_texture_map_mirrored_repeat(out, in, max);
        break;
    }
}

static inline void sw_texture_sample_nearest(float* color, const sw_texture_t* tex, float u, float v)
{
    int x, y;
    sw_texture_map(&x, u, tex->width, tex->sWrap);
    sw_texture_map(&y, v, tex->height, tex->tWrap);
    sw_get_pixel(color, tex->pixels, y * tex->width + x, tex->format);
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
    sw_get_pixel(c00, tex->pixels, y0 * tex->width + x0, tex->format);
    sw_get_pixel(c10, tex->pixels, y0 * tex->width + x1, tex->format);
    sw_get_pixel(c01, tex->pixels, y1 * tex->width + x0, tex->format);
    sw_get_pixel(c11, tex->pixels, y1 * tex->width + x1, tex->format);

    float c0[4], c1[4];
    for (int i = 0; i < 4; i++) {
        float a = sw_lerp(c00[i], c10[i], fx);
        float b = sw_lerp(c01[i], c11[i], fx);
        color[i] = sw_lerp(a, b, fy);
    }
}

static inline void sw_texture_sample(float* color, const sw_texture_t* tex, float u, float v,
                                     float xDu, float yDu, float xDv, float yDv)
{
    // TODO: It seems there are some incorrect detections depending on the context
    //       This is probably due to the fact that the fractions are obtained
    //       at the wrong moment during rasterization. It would be worth reviewing
    //       this, although the scanline method complicates things.

    // Calculate the derivatives for each axis
    float du = sqrtf(xDu * xDu + yDu * yDu);
    float dv = sqrtf(xDv * xDv + yDv * yDv);
    float L = (du > dv) ? du : dv;
    
    // Select the filter based on the size of the footprint
    if (L > 1.0f) {
        // Minification
        if (tex->minFilter == SW_NEAREST) {
            sw_texture_sample_nearest(color, tex, u, v);
        } else if (tex->minFilter == SW_LINEAR) {
            sw_texture_sample_linear(color, tex, u, v);
        }
    } else {
        // Magnification
        if (tex->magFilter == SW_NEAREST) {
            sw_texture_sample_nearest(color, tex, u, v);
        } else if (tex->magFilter == SW_LINEAR) {
            sw_texture_sample_linear(color, tex, u, v);
        }
    }
}


/* === Color Blending Functions === */

static inline void sw_blend_colors(float dst[4], float src[4])
{
    float src_factor[4] = { 0 };
    float dst_factor[4] = { 0 };

    switch (RLSW.srcFactor) {
    case SW_ZERO:
        src_factor[0] = src_factor[1] = src_factor[2] = src_factor[3] = 0.0f;
        break;
    case SW_ONE:
        src_factor[0] = src_factor[1] = src_factor[2] = src_factor[3] = 1.0f;
        break;
    case SW_SRC_COLOR:
        src_factor[0] = src[0];
        src_factor[1] = src[1];
        src_factor[2] = src[2];
        src_factor[3] = src[3];
        break;
    case SW_ONE_MINUS_SRC_COLOR:
        src_factor[0] = 1.0f - src[0];
        src_factor[1] = 1.0f - src[1];
        src_factor[2] = 1.0f - src[2];
        src_factor[3] = 1.0f - src[3];
        break;
    case SW_SRC_ALPHA:
        src_factor[0] = src[3];
        src_factor[1] = src[3];
        src_factor[2] = src[3];
        src_factor[3] = src[3];
        break;
    case SW_ONE_MINUS_SRC_ALPHA:
        src_factor[0] = 1.0f - src[3];
        src_factor[1] = 1.0f - src[3];
        src_factor[2] = 1.0f - src[3];
        src_factor[3] = 1.0f - src[3];
        break;
    case SW_DST_ALPHA:
        src_factor[0] = dst[3];
        src_factor[1] = dst[3];
        src_factor[2] = dst[3];
        src_factor[3] = dst[3];
        break;
    case SW_ONE_MINUS_DST_ALPHA:
        src_factor[0] = 1.0f - dst[3];
        src_factor[1] = 1.0f - dst[3];
        src_factor[2] = 1.0f - dst[3];
        src_factor[3] = 1.0f - dst[3];
        break;
    case SW_DST_COLOR:
        src_factor[0] = dst[0];
        src_factor[1] = dst[1];
        src_factor[2] = dst[2];
        src_factor[3] = dst[3];
        break;
    case SW_ONE_MINUS_DST_COLOR:
        src_factor[0] = 1.0f - dst[0];
        src_factor[1] = 1.0f - dst[1];
        src_factor[2] = 1.0f - dst[2];
        src_factor[3] = 1.0f - dst[3];
        break;
    case SW_SRC_ALPHA_SATURATE:
        src_factor[0] = 1.0f;
        src_factor[1] = 1.0f;
        src_factor[2] = 1.0f;
        src_factor[3] = fminf(src[3], 1.0f);
        break;
    }

    switch (RLSW.dstFactor) {
    case SW_ZERO:
        dst_factor[0] = dst_factor[1] = dst_factor[2] = dst_factor[3] = 0.0f;
        break;
    case SW_ONE:
        dst_factor[0] = dst_factor[1] = dst_factor[2] = dst_factor[3] = 1.0f;
        break;
    case SW_SRC_COLOR:
        dst_factor[0] = src[0];
        dst_factor[1] = src[1];
        dst_factor[2] = src[2];
        dst_factor[3] = src[3];
        break;
    case SW_ONE_MINUS_SRC_COLOR:
        dst_factor[0] = 1.0f - src[0];
        dst_factor[1] = 1.0f - src[1];
        dst_factor[2] = 1.0f - src[2];
        dst_factor[3] = 1.0f - src[3];
        break;
    case SW_SRC_ALPHA:
        dst_factor[0] = src[3];
        dst_factor[1] = src[3];
        dst_factor[2] = src[3];
        dst_factor[3] = src[3];
        break;
    case SW_ONE_MINUS_SRC_ALPHA:
        dst_factor[0] = 1.0f - src[3];
        dst_factor[1] = 1.0f - src[3];
        dst_factor[2] = 1.0f - src[3];
        dst_factor[3] = 1.0f - src[3];
        break;
    case SW_DST_ALPHA:
        dst_factor[0] = dst[3];
        dst_factor[1] = dst[3];
        dst_factor[2] = dst[3];
        dst_factor[3] = dst[3];
        break;
    case SW_ONE_MINUS_DST_ALPHA:
        dst_factor[0] = 1.0f - dst[3];
        dst_factor[1] = 1.0f - dst[3];
        dst_factor[2] = 1.0f - dst[3];
        dst_factor[3] = 1.0f - dst[3];
        break;
    case SW_DST_COLOR:
        dst_factor[0] = dst[0];
        dst_factor[1] = dst[1];
        dst_factor[2] = dst[2];
        dst_factor[3] = dst[3];
        break;
    case SW_ONE_MINUS_DST_COLOR:
        dst_factor[0] = 1.0f - dst[0];
        dst_factor[1] = 1.0f - dst[1];
        dst_factor[2] = 1.0f - dst[2];
        dst_factor[3] = 1.0f - dst[3];
        break;
    case SW_SRC_ALPHA_SATURATE:
        // NOTE: This case is only available for the source.  
        //       Since the factors are validated before assignment,  
        //       we should never reach this point.  
        break;
    }

    for (int i = 0; i < 4; ++i) {
        dst[i] = src_factor[i] * src[i] + dst_factor[i] * dst[i];
    }
}


/* === Projection Helper Functions === */

static inline void sw_project_ndc_to_screen(float screen[2], const float ndc[4])
{
    screen[0] = RLSW.vpPos[0] + (ndc[0] + 1.0f) * 0.5f * RLSW.vpDim[0];
    screen[1] = RLSW.vpPos[1] + (ndc[1] + 1.0f) * 0.5f * RLSW.vpDim[1];
}


/* === Triangle Rendering Part === */

static inline bool sw_triangle_clip_w(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
{
    sw_vertex_t input[SW_MAX_CLIPPED_POLYGON_VERTICES];
    for (int i = 0; i < SW_MAX_CLIPPED_POLYGON_VERTICES; i++) {
        input[i] = polygon[i];
    }

    int inputCounter = *vertexCounter;
    *vertexCounter = 0;

    const sw_vertex_t *prevVt = &input[inputCounter-1];
    char prevDot = (prevVt->homogeneous[3] < SW_CLIP_EPSILON) ? -1 : 1;

    for (int i = 0; i < inputCounter; i++) {
        char currDot = (input[i].homogeneous[3] < SW_CLIP_EPSILON) ? -1 : 1;
        if (prevDot*currDot < 0) {
            polygon[(*vertexCounter)++] = sw_lerp_vertex(prevVt, &input[i], 
                (SW_CLIP_EPSILON - prevVt->homogeneous[3]) / (input[i].homogeneous[3] - prevVt->homogeneous[3]));
        }
        if (currDot > 0) {
            polygon[(*vertexCounter)++] = input[i];
        }
        prevDot = currDot;
        prevVt = &input[i];
    }

    return *vertexCounter > 0;
}

static inline bool sw_triangle_clip_xyz(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
{
    for (int iAxis = 0; iAxis < 3; iAxis++)
    {
        if (*vertexCounter == 0) return false;

        sw_vertex_t input[SW_MAX_CLIPPED_POLYGON_VERTICES];
        int inputCounter;

        const sw_vertex_t *prevVt;
        char prevDot;

        // Clip against first plane

        for (int i = 0; i < SW_MAX_CLIPPED_POLYGON_VERTICES; i++) {
            input[i] = polygon[i];
        }
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int i = 0; i < inputCounter; i++) {
            char currDot = (input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;
            if (prevDot * currDot <= 0) {
                polygon[(*vertexCounter)++] = sw_lerp_vertex(prevVt, &input[i], (prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] - input[i].homogeneous[iAxis])));
            }
            if (currDot > 0) {
                polygon[(*vertexCounter)++] = input[i];
            }
            prevDot = currDot;
            prevVt = &input[i];
        }

        if (*vertexCounter == 0) return false;

        // Clip against opposite plane

        for (int i = 0; i < SW_MAX_CLIPPED_POLYGON_VERTICES; i++) {
            input[i] = polygon[i];
        }
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (-prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int i = 0; i < inputCounter; i++) {
            char currDot = (-input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;
            if (prevDot*currDot <= 0) {
                polygon[(*vertexCounter)++] = sw_lerp_vertex(prevVt, &input[i], (prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] + input[i].homogeneous[iAxis])));
            }
            if (currDot > 0) {
                polygon[(*vertexCounter)++] = input[i];
            }
            prevDot = currDot;
            prevVt = &input[i];
        }
    }

    return *vertexCounter > 0;
}

static inline void sw_triangle_project_and_clip(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
{
    for (int i = 0; i < *vertexCounter; i++) {
        sw_vertex_t *v = polygon + i;
        sw_vec4_transform(v->homogeneous, v->position, RLSW.matMVP);
    }

    if (RLSW.stateFlags & SW_STATE_CULL_FACE) {
        float x0 = polygon[0].homogeneous[0], y0 = polygon[0].homogeneous[1];
        float x1 = polygon[1].homogeneous[0], y1 = polygon[1].homogeneous[1];
        float x2 = polygon[2].homogeneous[0], y2 = polygon[2].homogeneous[1];

        float sgnArea = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
        if ((RLSW.cullFace == SW_FRONT && sgnArea >= 0) || (RLSW.cullFace == SW_BACK  && sgnArea <= 0)) {
            *vertexCounter = 0;
            return;
        }
    }

    if (sw_triangle_clip_w(polygon, vertexCounter) && sw_triangle_clip_xyz(polygon, vertexCounter)) {
        for (int i = 0; i < *vertexCounter; i++) {
            sw_vertex_t *v = polygon + i;

            // Calculation of the reciprocal of W for normalization
            // as well as perspective correct attributes
            v->homogeneous[3] = 1.0f / v->homogeneous[3];

            // Division of XYZ coordinates by weight
            v->homogeneous[0] *= v->homogeneous[3];
            v->homogeneous[1] *= v->homogeneous[3];
            v->homogeneous[2] *= v->homogeneous[3];

            // Division of texture coordinates (perspective correct)
            v->texcoord[0] *= v->homogeneous[3];
            v->texcoord[1] *= v->homogeneous[3];

            // Division of colors (perspective correct)
            v->color[0] *= v->homogeneous[3];
            v->color[1] *= v->homogeneous[3];
            v->color[2] *= v->homogeneous[3];
            v->color[3] *= v->homogeneous[3];

            // Transform to screen space
            sw_project_ndc_to_screen(v->screen, v->homogeneous);
        }
    }
}

#define DEFINE_TRIANGLE_RASTER_SCANLINE(FUNC_NAME, ENABLE_TEXTURE, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
static inline void FUNC_NAME(const sw_texture_t* tex, const sw_vertex_t* start,     \
                             const sw_vertex_t* end, float yDu, float yDv)          \
{                                                                                   \
    /* Calculate the horizontal width and avoid division by zero */                 \
    float dx = end->screen[0] - start->screen[0];                                   \
    if (fabsf(dx) < 1e-4f) return;                                                  \
                                                                                    \
    /* Convert and center the screen coordinates */                                 \
    int xStart = (int)(start->screen[0] + 0.5f);                                    \
    int xEnd   = (int)(end->screen[0] + 0.5f);                                      \
    int y      = (int)(start->screen[1] + 0.5f);                                    \
                                                                                    \
    /* Calculate the initial interpolation parameter and its increment */           \
    float dt = 1.0f / dx;                                                           \
    float t  = (xStart - start->screen[0]) * dt;                                    \
                                                                                    \
    float xDu, xDv;                                                                 \
    if (ENABLE_TEXTURE) {                                                           \
        /* Calculate the horizontal gradients for UV coordinates */                 \
        xDu = (end->texcoord[0] - start->texcoord[0]) * dt;                         \
        xDv = (end->texcoord[1] - start->texcoord[1]) * dt;                         \
    }                                                                               \
                                                                                    \
    /* Pre-calculate the color differences for interpolation */                     \
    float dcol[4];                                                                  \
    for (int i = 0; i < 4; i++) {                                                   \
        dcol[i] = end->color[i] - start->color[i];                                  \
    }                                                                               \
                                                                                    \
    /* Pre-calculate the differences in Z and W                                     \
       (for depth testing and perspective correction) */                            \
    float dz = end->homogeneous[2] - start->homogeneous[2];                         \
    float dw = end->homogeneous[3] - start->homogeneous[3];                         \
                                                                                    \
    float u, v;                                                                     \
    if (ENABLE_TEXTURE) {                                                           \
        /* Initialize the interpolated texture coordinates */                       \
        u = start->texcoord[0] + t * xDu;                                           \
        v = start->texcoord[1] + t * xDv;                                           \
    }                                                                               \
                                                                                    \
    /* Pre-calculate the starting pointer for the color framebuffer row */          \
    uint8_t* cptrRow = (uint8_t*)((uint32_t*)RLSW.framebuffer.color + y * RLSW.framebuffer.width); \
    uint8_t* cptr = cptrRow + xStart * 4;                                           \
                                                                                    \
    /* Pre-calculate the pointer for the depth buffer row */                        \
    uint16_t* dptrRow = RLSW.framebuffer.depth + y * RLSW.framebuffer.width + xStart; \
    uint16_t* dptr = dptrRow;                                                       \
                                                                                    \
    /* Scanline rasterization loop */                                               \
    for (int x = xStart; x < xEnd; x++) {                                           \
        /* Interpolate Z and W for depth testing and perspective correction */      \
        float w = 1.0f / (start->homogeneous[3] + t * dw);                          \
        float z = start->homogeneous[2] + t * dz;                                   \
                                                                                    \
        if (ENABLE_DEPTH_TEST) {                                                    \
            /* Depth testing with direct access to the depth buffer */              \
            /* TODO: Implement different depth funcs? */                            \
            float depth = (float)(*dptr) / UINT16_MAX;                              \
            if (z > depth) goto discard;                                            \
        }                                                                           \
                                                                                    \
        /* Update the depth buffer */                                               \
        *dptr = (uint16_t)(z * UINT16_MAX);                                         \
                                                                                    \
        if (ENABLE_COLOR_BLEND)                                                     \
        {                                                                           \
            float dstColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };                         \
            float srcColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };                         \
                                                                                    \
            if (ENABLE_TEXTURE) {                                                   \
                sw_texture_sample(srcColor, tex, u * w, v * w, xDu, yDu, xDv, yDv); \
            }                                                                       \
                                                                                    \
            for (int i = 0; i < 4; i++) {                                           \
                dstColor[i] = (float)cptr[i] / 255;                                 \
                srcColor[i] *= (start->color[i] + t * dcol[i]) * w;                 \
            }                                                                       \
                                                                                    \
            sw_blend_colors(dstColor, srcColor);                                    \
                                                                                    \
            for (int i = 0; i < 4; i++) {                                           \
                cptr[i] = (uint8_t)(sw_saturate(dstColor[i]) * 255);                \
            }                                                                       \
        }                                                                           \
        else                                                                        \
        {                                                                           \
            if (ENABLE_TEXTURE)                                                     \
            {                                                                       \
                /* Sample the texture */                                            \
                float texColor[4];                                                  \
                sw_texture_sample(texColor, tex, u * w, v * w, xDu, yDu, xDv, yDv); \
                                                                                    \
                /* Interpolate the color and modulate by the texture color */       \
                for (int i = 0; i < 4; i++) {                                       \
                    float finalColor = texColor[i];                                 \
                    finalColor *= (start->color[i] + t * dcol[i]) * w;              \
                    cptr[i] = (uint8_t)(sw_saturate(finalColor) * 255.0f);          \
                }                                                                   \
            }                                                                       \
            else                                                                    \
            {                                                                       \
                /* Interpolate the color */                                         \
                for (int i = 0; i < 4; i++) {                                       \
                    float finalColor = (start->color[i] + t * dcol[i]) * w;         \
                    cptr[i] = (uint8_t)(sw_saturate(finalColor) * 255.0f);          \
                }                                                                   \
            }                                                                       \
        }                                                                           \
                                                                                    \
        /* Increment the interpolation parameter, UVs, and pointers */              \
        discard:                                                                    \
        t += dt;                                                                    \
        cptr += 4;                                                                  \
        dptr++;                                                                     \
        if (ENABLE_TEXTURE) {                                                       \
            u += xDu;                                                               \
            v += xDv;                                                               \
        }                                                                           \
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
    /* Reject degenerate triangles */                                               \
    float height = y2 - y0;                                                         \
    if (height < 1e-4f) return;                                                     \
                                                                                    \
    /* Precompute the inverse of the triangle height and */                         \
    /* edge lengths with checks to avoid division by zero. */                       \
    float inv_height = 1.0f / height;                                               \
    float inv_y1y0 = (y1 - y0 > 1e-4f) ? 1.0f / (y1 - y0) : 0.0f;                   \
    float inv_y2y1 = (y2 - y1 > 1e-4f) ? 1.0f / (y2 - y1) : 0.0f;                   \
                                                                                    \
    /* Pre-calculation of slopes (dx/dy) */                                         \
    float dx02 = (x2 - x0) * inv_height;                                            \
    float dx01 = (x1 - x0) * inv_y1y0;                                              \
    float dx12 = (x2 - x1) * inv_y2y1;                                              \
                                                                                    \
    /* Y bounds (vertical clipping) */                                              \
    int yTop = (int)(y0 + 0.5f);                                                    \
    int yMiddle = (int)(y1 + 0.5f);                                                 \
    int yBottom = (int)(y2 + 0.5f);                                                 \
                                                                                    \
    /* Global calculation of vertical texture gradients for the triangle */         \
    float yDu, yDv;                                                                 \
    if (ENABLE_TEXTURE) {                                                           \
        yDu = (v2->texcoord[0] - v0->texcoord[0]) * inv_height;                     \
        yDv = (v2->texcoord[1] - v0->texcoord[1]) * inv_height;                     \
    }                                                                               \
                                                                                    \
    /* Initializing scanline variables */                                           \
    float xLeft = x0, xRight = x0;                                                  \
    sw_vertex_t start, end;                                                         \
                                                                                    \
    /* Scanline for the upper part of the triangle */                               \
    for (int y = yTop; y < yMiddle; y++) {                                          \
        float dy = (float)y - y0;                                                   \
        float t1 = dy * inv_height;                                                 \
        float t2 = dy * inv_y1y0;                                                   \
                                                                                    \
        /* Vertex interpolation */                                                  \
        start = sw_lerp_vertex(v0, v2, t1);                                         \
        end   = sw_lerp_vertex(v0, v1, t2);                                         \
        start.screen[0] = xLeft;                                                    \
        start.screen[1] = (float)y;                                                 \
        end.screen[0] = xRight;                                                     \
        end.screen[1] = (float)y;                                                   \
                                                                                    \
        if (xLeft > xRight) { sw_vertex_t tmp = start; start = end; end = tmp; }    \
        FUNC_SCANLINE(tex, &start, &end, yDu, yDv);                                 \
                                                                                    \
        /* Incremental update */                                                    \
        xLeft  += dx02;                                                             \
        xRight += dx01;                                                             \
    }                                                                               \
                                                                                    \
    /* Scanline for the lower part of the triangle */                               \
    xRight = x1; /* Restart the right side from the second vertex */                \
    for (int y = yMiddle; y < yBottom; y++) {                                       \
        float dy = (float)y - y0;                                                   \
        float t1 = dy * inv_height;                                                 \
        float t2 = (float)(y - y1) * inv_y2y1;                                      \
                                                                                    \
        /* Vertex interpolation */                                                  \
        start = sw_lerp_vertex(v0, v2, t1);                                         \
        end   = sw_lerp_vertex(v1, v2, t2);                                         \
        start.screen[0] = xLeft;                                                    \
        start.screen[1] = (float)y;                                                 \
        end.screen[0] = xRight;                                                     \
        end.screen[1] = (float)y;                                                   \
                                                                                    \
        if (xLeft > xRight) { sw_vertex_t tmp = start; start = end; end = tmp; }    \
        FUNC_SCANLINE(tex, &start, &end, yDu, yDv);                                 \
                                                                                    \
        /* Incremental update */                                                    \
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

static inline void sw_triangle_render(const sw_vertex_t* v0, const sw_vertex_t* v1, const sw_vertex_t* v2)
{
    int vertexCounter = 3;

    sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES];
    polygon[0] = *v0;
    polygon[1] = *v1;
    polygon[2] = *v2;

    sw_triangle_project_and_clip(polygon, &vertexCounter);

    if (vertexCounter < 3) {
        return;
    }

#   define TRIANGLE_RASTER(RASTER_FUNC)                         \
    {                                                           \
        for (int i = 0; i < vertexCounter - 2; i++) {           \
            RASTER_FUNC(                                        \
                &polygon[0], &polygon[i + 1], &polygon[i + 2],  \
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
}


/* === Line Rendering Part === */

uint8_t sw_line_clip_encode_2d(const float screen[2], int xMin, int yMin, int xMax, int yMax)
{
    uint8_t code = SW_CLIP_INSIDE;
    if (screen[0] < xMin) code |= SW_CLIP_LEFT;
    if (screen[0] > xMax) code |= SW_CLIP_RIGHT;
    if (screen[1] < yMin) code |= SW_CLIP_TOP;
    if (screen[1] > yMax) code |= SW_CLIP_BOTTOM;
    return code;
}

bool sw_line_clip_2d(sw_vertex_t* v1, sw_vertex_t* v2)
{
    int xMin = RLSW.vpMin[0];
    int yMin = RLSW.vpMin[1];
    int xMax = RLSW.vpMax[0];
    int yMax = RLSW.vpMax[1];

    bool accept = false;
    uint8_t code0, code1;
    float m = 0;

    if (v1->screen[0] != v2->screen[0]) {
        m = (v2->screen[1] - v1->screen[1]) / (v2->screen[0] - v1->screen[0]);
    }

    for (;;) {
        code0 = sw_line_clip_encode_2d(v1->screen, xMin, yMin, xMax, yMax);
        code1 = sw_line_clip_encode_2d(v2->screen, xMin, yMin, xMax, yMax);

        // Accepted if both endpoints lie within rectangle
        if ((code0 | code1) == 0) {
            accept = true;
            break;
        }

        // Rejected if both endpoints are outside rectangle, in same region
        if (code0 & code1) break;

        if (code0 == SW_CLIP_INSIDE) {
            uint8_t ctmp = code0; code0 = code1; code1 = ctmp;
            sw_vertex_t vtmp = *v1; *v1 = *v2; *v2 = vtmp;
        }

        if (code0 & SW_CLIP_LEFT) {
            v1->screen[1] += (RLSW.vpMin[0] - v1->screen[0])*m;
            v1->screen[0] = (float)RLSW.vpMin[0];
        }
        else if (code0 & SW_CLIP_RIGHT) {
            v1->screen[1] += (RLSW.vpMax[0] - v1->screen[0])*m;
            v1->screen[0] = (float)RLSW.vpMax[0];
        }
        else if (code0 & SW_CLIP_BOTTOM) {
            if (m) v1->screen[0] += (RLSW.vpMin[1] - v1->screen[1]) / m;
            v1->screen[1] = (float)RLSW.vpMin[1];
        }
        else if (code0 & SW_CLIP_TOP) {
            if (m) v1->screen[0] += (RLSW.vpMax[1] - v1->screen[1]) / m;
            v1->screen[1] = (float)RLSW.vpMax[1];
        }
    }

    return accept;
}

bool sw_line_clip_coord_3d(float q, float p, float* t1, float* t2)
{
    if (fabsf(p) < SW_CLIP_EPSILON) {
        // Check if the line is entirely outside the window
        if (q < -SW_CLIP_EPSILON) return 0; // Completely outside
        return 1;                           // Completely inside or on the edges
    }

    const float r = q / p;

    if (p < 0) {
        if (r > *t2) return 0;
        if (r > *t1) *t1 = r;
    } else {
        if (r < *t1) return 0;
        if (r < *t2) *t2 = r;
    }

    return 1;
}

bool sw_line_clip_3d(sw_vertex_t* v1, sw_vertex_t* v2)
{
    // TODO: Lerp all vertices here, not just homogeneous coordinates

    float t1 = 0, t2 = 1;

    float delta[4];
    for (int i = 0; i < 4; i++) {
        delta[i] = v2->homogeneous[i] - v1->homogeneous[i];
    }

    if (!sw_line_clip_coord_3d(v1->homogeneous[3] - v1->homogeneous[0], -delta[3] + delta[0], &t1, &t2)) return false;
    if (!sw_line_clip_coord_3d(v1->homogeneous[3] + v1->homogeneous[0], -delta[3] - delta[0], &t1, &t2)) return false;

    if (!sw_line_clip_coord_3d(v1->homogeneous[3] - v1->homogeneous[1], -delta[3] + delta[1], &t1, &t2)) return false;
    if (!sw_line_clip_coord_3d(v1->homogeneous[3] + v1->homogeneous[1], -delta[3] - delta[1], &t1, &t2)) return false;

    if (!sw_line_clip_coord_3d(v1->homogeneous[3] - v1->homogeneous[2], -delta[3] + delta[2], &t1, &t2)) return false;
    if (!sw_line_clip_coord_3d(v1->homogeneous[3] + v1->homogeneous[2], -delta[3] - delta[2], &t1, &t2)) return false;

    if (t2 < 1) {
        for (int i = 0; i < 4; i++) {
            v2->homogeneous[i] = v1->homogeneous[i] + t2 * delta[i];
        }
    }

    if (t1 > 0) {
        for (int i = 0; i < 4; i++) {
            v1->homogeneous[i] = v1->homogeneous[i] + t1 * delta[i];
        }
    }

    return true;
}

bool sw_line_project_and_clip(sw_vertex_t* v0, sw_vertex_t* v1)
{
    sw_vec4_transform(v0->homogeneous, v0->position, RLSW.matMVP);
    sw_vec4_transform(v1->homogeneous, v1->position, RLSW.matMVP);

    if (v0->homogeneous[3] == 1.0f && v1->homogeneous[3] == 1.0f) {
        sw_project_ndc_to_screen(v0->screen, v0->homogeneous);
        sw_project_ndc_to_screen(v1->screen, v1->homogeneous);
        if (!sw_line_clip_2d(v0, v1)) {
            return false;
        }
    }
    else {
        if (!sw_line_clip_3d(v0, v1)) {
            return false;
        }
        // Convert XYZ coordinates to NDC
        v0->homogeneous[3] = 1.0f / v0->homogeneous[3];
        v1->homogeneous[3] = 1.0f / v1->homogeneous[3];
        for (int i = 0; i < 3; i++) {
            v0->homogeneous[i] *= v0->homogeneous[3];
            v1->homogeneous[i] *= v1->homogeneous[3];
        }
        // Convert NDC coordinates to screen space
        sw_project_ndc_to_screen(v0->screen, v0->homogeneous);
        sw_project_ndc_to_screen(v1->screen, v1->homogeneous);
    }

    return true;
}

#define DEFINE_LINE_RASTER(FUNC_NAME, ENABLE_DEPTH_TEST, ENABLE_COLOR_BLEND) \
void FUNC_NAME(const sw_vertex_t* v0, const sw_vertex_t* v1)            \
{                                                                       \
    int x1 = (int)v0->screen[0];                                        \
    int y1 = (int)v0->screen[1];                                        \
    int x2 = (int)v1->screen[0];                                        \
    int y2 = (int)v1->screen[1];                                        \
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
            int pixel_index = y * fbWidth + x;                          \
                                                                        \
            uint16_t* dptr = &depthBuffer[pixel_index];                 \
            if (ENABLE_DEPTH_TEST) {                                    \
                float depth = (float)(*dptr) / UINT16_MAX;              \
                if (z > depth) continue;                                \
            }                                                           \
                                                                        \
            *dptr = (uint16_t)(z * UINT16_MAX);                         \
                                                                        \
            int color_index = 4 * pixel_index;                          \
            uint8_t* cptr = &colorBuffer[color_index];                  \
                                                                        \
            if (ENABLE_COLOR_BLEND)                                     \
            {                                                           \
                float dstColor[4];                                      \
                float srcColor[4];                                      \
                for (int j = 0; j < 4; j++) {                           \
                    dstColor[j] = (float)cptr[i] / 255;                 \
                    srcColor[j] = sw_lerp(v0->color[j], v1->color[j], t); \
                }                                                       \
                sw_blend_colors(dstColor, srcColor);                    \
                for (int j = 0; j < 4; j++) {                           \
                    cptr[j] = (uint8_t)(dstColor[j] * 255);             \
                }                                                       \
            }                                                           \
            else                                                        \
            {                                                           \
                for (int j = 0; j < 4; j++) {                           \
                    float finalColor = sw_lerp(v0->color[j], v1->color[j], t); \
                    cptr[j] = (uint8_t)(finalColor * 255);              \
                }                                                       \
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
            int pixel_index = y * fbWidth + x;                          \
                                                                        \
            uint16_t* dptr = &depthBuffer[pixel_index];                 \
            if (ENABLE_DEPTH_TEST) {                                    \
                float depth = (float)(*dptr) / UINT16_MAX;              \
                if (z > depth) continue;                                \
            }                                                           \
                                                                        \
            *dptr = (uint16_t)(z * UINT16_MAX);                         \
                                                                        \
            int color_index = 4 * pixel_index;                          \
            uint8_t* cptr = &colorBuffer[color_index];                  \
                                                                        \
            if (ENABLE_COLOR_BLEND)                                     \
            {                                                           \
                float dstColor[4];                                      \
                float srcColor[4];                                      \
                for (int j = 0; j < 4; j++) {                           \
                    dstColor[j] = (float)cptr[i] / 255;                 \
                    srcColor[j] = sw_lerp(v0->color[j], v1->color[j], t); \
                }                                                       \
                sw_blend_colors(dstColor, srcColor);                    \
                for (int j = 0; j < 4; j++) {                           \
                    cptr[j] = (uint8_t)(dstColor[j] * 255);             \
                }                                                       \
            }                                                           \
            else                                                        \
            {                                                           \
                for (int j = 0; j < 4; j++) {                           \
                    float finalColor = sw_lerp(v0->color[j], v1->color[j], t); \
                    cptr[j] = (uint8_t)(finalColor * 255);              \
                }                                                       \
            }                                                           \
        }                                                               \
    }                                                                   \
}

DEFINE_LINE_RASTER(sw_line_raster, 0, 0)
DEFINE_LINE_RASTER(sw_line_raster_DEPTH, 1, 0)
DEFINE_LINE_RASTER(sw_line_raster_BLEND, 0, 1)
DEFINE_LINE_RASTER(sw_line_raster_DEPTH_BLEND, 1, 1)

static inline void sw_line_render(sw_vertex_t* v0, sw_vertex_t* v1)
{
    if (!sw_line_project_and_clip(v0, v1)) {
        return;
    }

    if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST | SW_STATE_BLEND)) {
        sw_line_raster_DEPTH_BLEND(v0, v1);
    }
    else if (SW_STATE_CHECK(SW_STATE_BLEND)) {
        sw_line_raster_BLEND(v0, v1);
    }
    else if (SW_STATE_CHECK(SW_STATE_DEPTH_TEST)) {
        sw_line_raster_DEPTH(v0, v1);
    }
    else {
        sw_line_raster(v0, v1);
    }
}

/* === Some Validity Check Helper === */

static inline bool sw_is_texture_id_valid(uint32_t id)
{
    bool valid = true;

    if (id == 0) valid = false;
    else if (id >= SW_MAX_TEXTURES) valid = false;
    else if (RLSW.loadedTextures[id].pixels == 0) valid = false;

    return true;
}

static inline bool sw_is_texture_filter_valid(int filter)
{
    return (filter == SW_NEAREST || filter == SW_LINEAR);
}

static inline bool sw_is_texture_wrap_valid(int wrap)
{
    return (wrap == SW_REPEAT || wrap == SW_CLAMP_TO_EDGE || SW_MIRRORED_REPEAT);
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

void swInit(int w, int h)
{
    swViewport(0, 0, w, h);

    RLSW.framebuffer.color = SW_MALLOC(4 * w * h);
    RLSW.framebuffer.depth = SW_MALLOC(2 * w * h);

    RLSW.framebuffer.width = w;
    RLSW.framebuffer.height = h;

    RLSW.loadedTextures = SW_MALLOC(SW_MAX_TEXTURES);
    RLSW.freeTextureIds = SW_MALLOC(SW_MAX_TEXTURES);

    RLSW.clearColor[0] = 0;
    RLSW.clearColor[1] = 0;
    RLSW.clearColor[2] = 0;
    RLSW.clearColor[3] = 255;
    RLSW.clearDepth = UINT16_MAX;

    RLSW.currentMatrixMode = SW_MODELVIEW;
    RLSW.currentMatrix = &RLSW.matView;

    sw_matrix_id(RLSW.matProjection);
    sw_matrix_id(RLSW.matTexture);
    sw_matrix_id(RLSW.matModel);
    sw_matrix_id(RLSW.matView);

    RLSW.vertexBuffer[0].color[0] = 1.0f;
    RLSW.vertexBuffer[0].color[1] = 1.0f;
    RLSW.vertexBuffer[0].color[2] = 1.0f;
    RLSW.vertexBuffer[0].color[3] = 1.0f;

    RLSW.vertexBuffer[0].texcoord[0] = 0.0f;
    RLSW.vertexBuffer[0].texcoord[1] = 0.0f;

    RLSW.vertexBuffer[0].normal[0] = 0.0f;
    RLSW.vertexBuffer[0].normal[1] = 0.0f;
    RLSW.vertexBuffer[0].normal[2] = 1.0f;

    RLSW.srcFactor = SW_SRC_ALPHA;
    RLSW.dstFactor = SW_ONE_MINUS_SRC_ALPHA;

    RLSW.cullFace = SW_BACK;

    static const float defTex[3*2*2] =
    {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    RLSW.loadedTextures[0].pixels = defTex;
    RLSW.loadedTextures[0].width = 2;
    RLSW.loadedTextures[0].height = 2;
    RLSW.loadedTextures[0].format = SW_PIXELFORMAT_UNCOMPRESSED_R32G32B32;
    RLSW.loadedTextures[0].minFilter = SW_NEAREST;
    RLSW.loadedTextures[0].magFilter = SW_NEAREST;
    RLSW.loadedTextures[0].sWrap = SW_REPEAT;
    RLSW.loadedTextures[0].tWrap = SW_REPEAT;
    RLSW.loadedTextures[0].tx = 0.5f;
    RLSW.loadedTextures[0].ty = 0.5f;

    RLSW.loadedTextureCount = 1;
}

void swClose(void)
{
    SW_FREE(RLSW.framebuffer.color);
    SW_FREE(RLSW.framebuffer.depth);

    SW_FREE(RLSW.loadedTextures);
    SW_FREE(RLSW.freeTextureIds);
}

void swEnable(SWstate state)
{
    switch (state) {
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

void* swGetColorBuffer(int* w, int* h)
{
    if (w) *w = RLSW.framebuffer.width;
    if (h) *h = RLSW.framebuffer.height;

    return RLSW.framebuffer.color;
}

void swMatrixMode(SWmatrix mode)
{
    switch (mode) {
    case SW_PROJECTION:
        RLSW.currentMatrix = &RLSW.matProjection;
        break;
    case SW_MODELVIEW:
        RLSW.currentMatrix = RLSW.modelMatrixUsed
            ? &RLSW.matModel : &RLSW.matView;
        break;
    case SW_TEXTURE:
        RLSW.currentMatrix = &RLSW.matTexture;
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
        if (RLSW.stackProjectionCounter >= SW_MAX_PROJECTION_STACK_SIZE) {
            RLSW.errCode = SW_STACK_OVERFLOW;
            return;
        }
        for (int i = 0; i < 16; i++) {
            RLSW.stackProjection[RLSW.stackProjectionCounter][i] = RLSW.matProjection[i];
        }
        RLSW.stackProjectionCounter++;
        break;

    case SW_MODELVIEW:
        if (RLSW.stackModelviewCounter >= SW_MAX_MODELVIEW_STACK_SIZE) {
            RLSW.errCode = SW_STACK_OVERFLOW;
            return;
        }
        if (RLSW.modelMatrixUsed) {
            for (int i = 0; i < 16; i++) {
                RLSW.stackModelview[RLSW.stackModelviewCounter][i] = RLSW.matModel[i];
            }
            RLSW.stackModelviewCounter++;
        } else {
            RLSW.currentMatrix = &RLSW.matModel;
            RLSW.modelMatrixUsed = true;
        }
        break;

    case SW_TEXTURE:
        if (RLSW.stackTextureCounter >= SW_MAX_TEXTURE_STACK_SIZE) {
            RLSW.errCode = SW_STACK_OVERFLOW;
            return;
        }
        for (int i = 0; i < 16; i++) {
            RLSW.stackTexture[RLSW.stackTextureCounter][i] = RLSW.matTexture[i];
        }
        RLSW.stackTextureCounter++;
        break;

    }
}

void swPopMatrix(void)
{
    switch (RLSW.currentMatrixMode) {

    case SW_PROJECTION:
        if (RLSW.stackProjectionCounter <= 0) {
            RLSW.errCode = SW_STACK_UNDERFLOW;
            return;
        }
        RLSW.stackProjectionCounter--;
        for (int i = 0; i < 16; i++) {
            RLSW.matProjection[i] = RLSW.stackProjection[RLSW.stackProjectionCounter][i];
        }
        break;

    case SW_MODELVIEW:
        if (RLSW.stackModelviewCounter == 0)  {
            if (!RLSW.modelMatrixUsed) {
                RLSW.errCode = SW_STACK_UNDERFLOW;
                return;
            }
            sw_matrix_id(RLSW.matModel);
            RLSW.currentMatrix = &RLSW.matView;
            RLSW.modelMatrixUsed = false;
        } else {
            RLSW.stackModelviewCounter--;
            for (int i = 0; i < 16; i++) {
                RLSW.matModel[i] = RLSW.stackModelview[RLSW.stackModelviewCounter][i];
            }
        }
        break;

    case SW_TEXTURE:
        if (RLSW.stackTextureCounter <= 0) {
            RLSW.errCode = SW_STACK_UNDERFLOW;
            return;
        }
        RLSW.stackTextureCounter--;
        for (int i = 0; i < 16; i++) {
            RLSW.matTexture[i] = RLSW.stackTexture[RLSW.stackTextureCounter][i];
        }
        break;

    }
}

void swLoadIdentity(void)
{
    sw_matrix_id(*RLSW.currentMatrix);
}

void swTranslatef(float x, float y, float z)
{
    sw_matrix_t mat;
    sw_matrix_id(mat);

    mat[12] = x;
    mat[13] = y;
    mat[14] = z;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);
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
}

void swScalef(float x, float y, float z)
{
    sw_matrix_t mat;

    mat[0]  = x, mat[1]  = 0, mat[2]  = 0, mat[3]  = 0;
    mat[4]  = 0, mat[5]  = y, mat[6]  = 0, mat[7]  = 0;
    mat[8]  = 0, mat[9]  = 0, mat[10] = z, mat[11] = 0;
    mat[12] = 0, mat[13] = 0, mat[14] = 0, mat[15] = 1;

    sw_matrix_mul(*RLSW.currentMatrix, mat, *RLSW.currentMatrix);
}

void swMultMatrixf(const float* mat)
{
    sw_matrix_mul(*RLSW.currentMatrix, *RLSW.currentMatrix, mat);
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
}

void swViewport(int x, int y, int width, int height)
{
    if (x <= -width || y <= -height) {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    RLSW.vpPos[0] = x;
    RLSW.vpPos[1] = y;

    RLSW.vpDim[0] = width - 1;
    RLSW.vpDim[1] = height - 1;

    RLSW.vpMin[0] = (x < 0) ? 0 : x;
    RLSW.vpMin[1] = (y < 0) ? 0 : y;

    int fbW = RLSW.framebuffer.width - 1;
    int fbH = RLSW.framebuffer.height - 1;

    int vpMaxX = x + width;
    int vpMaxY = y + height;

    RLSW.vpMax[0] = (vpMaxX < fbW) ? vpMaxX : fbW;
    RLSW.vpMax[1] = (vpMaxY < fbH) ? vpMaxY : fbH;
}

void swClearColor(float r, float g, float b, float a)
{
    RLSW.clearColor[0] = r * 255;
    RLSW.clearColor[1] = g * 255;
    RLSW.clearColor[2] = b * 255;
    RLSW.clearColor[3] = a * 255;
}

void swClear(void)
{
    int size = RLSW.framebuffer.width * RLSW.framebuffer.height;

    for (int i = 0; i < size; i++) {
        ((uint32_t*)RLSW.framebuffer.color)[i] = *((uint32_t*)RLSW.clearColor);
        RLSW.framebuffer.depth[i] = RLSW.clearDepth;
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
}

void swCullFace(SWface face)
{
    if (!sw_is_face_valid(face)) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }
    RLSW.cullFace = face;
}

void swBegin(SWdraw mode)
{
    if (mode < SW_POINTS || mode > SW_QUADS) {
        RLSW.errCode = SW_INVALID_ENUM;
        return;
    }
    RLSW.vertexCounter = 0;
    RLSW.drawMode = mode;
}

void swEnd(void)
{
    RLSW.vertexCounter = 0;
}

void swVertex2i(int x, int y)
{
    float v[4] = { (float)x, (float)y, 0.0f, 1.0f };
    swVertex4fv(v);
}

void swVertex2f(float x, float y)
{
    float v[4] = { x, y, 0.0f, 1.0f };
    swVertex4fv(v);
}

void swVertex2fv(const float* v)
{
    float v4[4] = { v[0], v[1], 0.0f, 1.0f };
    swVertex4fv(v4);
}

void swVertex3i(int x, int y, int z)
{
    float v[4] = { (float)x, (float)y, (float)z, 1.0f };
    swVertex4fv(v);
}

void swVertex3f(float x, float y, float z)
{
    float v[4] = { x, y, z, 1.0f };
    swVertex4fv(v);
}

void swVertex3fv(const float* v)
{
    float v4[4] = { v[0], v[1], v[2], 1.0f };
    swVertex4fv(v4);
}

void swVertex4i(int x, int y, int z, int w)
{
    float v[4] = { (float)x, (float)y, (float)z, (float)w };
    swVertex4fv(v);
}

void swVertex4f(float x, float y, float z, float w)
{
    float v[4] = { x, y, z, w };
    swVertex4fv(v);
}

void swVertex4fv(const float* v)
{
    for (int i = 0; i < 4; i++) {
        RLSW.vertexBuffer[RLSW.vertexCounter].position[i] = v[i];
    }
    RLSW.vertexCounter++;

    int neededVertices = 0;
    switch (RLSW.drawMode) {
    case SW_POINTS:
        neededVertices = 1;
        break;
    case SW_LINES:
        neededVertices = 2;
        break;
    case SW_TRIANGLES:
        neededVertices = 3;
        break;
    case SW_QUADS:
        neededVertices = 4;
        break;
    }

    if (RLSW.vertexCounter == neededVertices) {

        // TODO: Optimize MVP calculation
        sw_matrix_mul(RLSW.matMVP, RLSW.matModel, RLSW.matView);
        sw_matrix_mul(RLSW.matMVP, RLSW.matMVP, RLSW.matProjection);

        switch (RLSW.drawMode) {
        case SW_POINTS:
            break;
        case SW_LINES:
            sw_line_render(
                &RLSW.vertexBuffer[0],
                &RLSW.vertexBuffer[1]
            );
            break;
        case SW_TRIANGLES:
            sw_triangle_render(
                &RLSW.vertexBuffer[0],
                &RLSW.vertexBuffer[1],
                &RLSW.vertexBuffer[2]
            );
            break;
        case SW_QUADS:
            sw_triangle_render(
                &RLSW.vertexBuffer[0],
                &RLSW.vertexBuffer[1],
                &RLSW.vertexBuffer[2]
            );
            sw_triangle_render(
                &RLSW.vertexBuffer[2],
                &RLSW.vertexBuffer[3],
                &RLSW.vertexBuffer[0]
            );
            break;
        }

        RLSW.vertexBuffer[0] = RLSW.vertexBuffer[neededVertices - 1];
        RLSW.vertexCounter = 0;
    }
    else {
        RLSW.vertexBuffer[RLSW.vertexCounter] = RLSW.vertexBuffer[RLSW.vertexCounter - 1];
    }
}

void swColor1ui(uint32_t color)
{
    union {
        uint32_t v;
        uint8_t a[4];
    } c = { .v = color };

    float cv[4];
    cv[0] = (float)c.a[0] / 255;
    cv[1] = (float)c.a[1] / 255;
    cv[2] = (float)c.a[2] / 255;
    cv[3] = (float)c.a[3] / 255;

    swColor4fv(cv);
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

void swColor3us(uint16_t r, uint16_t g, uint16_t b)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(r >> 8)) / 255;
    cv[1] = (float)((uint8_t)(g >> 8)) / 255;
    cv[2] = (float)((uint8_t)(b >> 8)) / 255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3usv(const uint16_t* v)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(v[0] >> 8)) / 255;
    cv[1] = (float)((uint8_t)(v[1] >> 8)) / 255;
    cv[2] = (float)((uint8_t)(v[2] >> 8)) / 255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3ui(uint32_t r, uint32_t g, uint32_t b)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(r >> 24)) / 255;
    cv[1] = (float)((uint8_t)(g >> 24)) / 255;
    cv[2] = (float)((uint8_t)(b >> 24)) / 255;
    cv[3] = 1.0f;

    swColor4fv(cv);
}

void swColor3uiv(const uint32_t* v)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(v[0] >> 24)) / 255;
    cv[1] = (float)((uint8_t)(v[1] >> 24)) / 255;
    cv[2] = (float)((uint8_t)(v[2] >> 24)) / 255;
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

void swColor4us(uint16_t r, uint16_t g, uint16_t b, uint16_t a)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(r >> 8)) / 255;
    cv[1] = (float)((uint8_t)(g >> 8)) / 255;
    cv[2] = (float)((uint8_t)(b >> 8)) / 255;
    cv[3] = (float)((uint8_t)(a >> 8)) / 255;

    swColor4fv(cv);
}

void swColor4usv(const uint16_t* v)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(v[0] >> 8)) / 255;
    cv[1] = (float)((uint8_t)(v[1] >> 8)) / 255;
    cv[2] = (float)((uint8_t)(v[2] >> 8)) / 255;
    cv[3] = (float)((uint8_t)(v[3] >> 8)) / 255;

    swColor4fv(cv);
}

void swColor4ui(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(r >> 24)) / 255;
    cv[1] = (float)((uint8_t)(g >> 24)) / 255;
    cv[2] = (float)((uint8_t)(b >> 24)) / 255;
    cv[3] = (float)((uint8_t)(a >> 24)) / 255;

    swColor4fv(cv);
}

void swColor4uiv(const uint32_t* v)
{
    float cv[4];
    cv[0] = (float)((uint8_t)(v[0] >> 24)) / 255;
    cv[1] = (float)((uint8_t)(v[1] >> 24)) / 255;
    cv[2] = (float)((uint8_t)(v[2] >> 24)) / 255;
    cv[3] = (float)((uint8_t)(v[3] >> 24)) / 255;

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
    float s = RLSW.matTexture[0]*u + RLSW.matTexture[4]*v + RLSW.matTexture[12];
    float t = RLSW.matTexture[1]*u + RLSW.matTexture[5]*v + RLSW.matTexture[13];

    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[0] = s;
    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[1] = t;
}

void swTexCoordfv(const float* v)
{
    float s = RLSW.matTexture[0]*v[0] + RLSW.matTexture[4]*v[1] + RLSW.matTexture[12];
    float t = RLSW.matTexture[1]*v[0] + RLSW.matTexture[5]*v[1] + RLSW.matTexture[13];

    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[0] = s;
    RLSW.vertexBuffer[RLSW.vertexCounter].texcoord[1] = t;
}

void swNormal3f(float x, float y, float z)
{
    RLSW.vertexBuffer[RLSW.vertexCounter].normal[0] = x;
    RLSW.vertexBuffer[RLSW.vertexCounter].normal[1] = y;
    RLSW.vertexBuffer[RLSW.vertexCounter].normal[2] = z;
}

void swNormal3fv(const float* v)
{
    RLSW.vertexBuffer[RLSW.vertexCounter].normal[0] = v[0];
    RLSW.vertexBuffer[RLSW.vertexCounter].normal[1] = v[1];
    RLSW.vertexBuffer[RLSW.vertexCounter].normal[2] = v[2];
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
    case SW_NORMAL_ARRAY:
        RLSW.array.normals = buffer;
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

    for (int i = offset; i < count; i++) {
        if (RLSW.array.texcoords) {
            swTexCoordfv(RLSW.array.texcoords + 2 * i);
        }
        if (RLSW.array.normals) {
            swNormal3fv(RLSW.array.normals + 3 * i);
        }
        if (RLSW.array.colors) {
            swColor4ubv(RLSW.array.colors + 4 * i);
        }
        swVertex3fv(RLSW.array.positions + 3 * i);
    }

    swEnd();
}

uint32_t swLoadTexture(const void *data, int width, int height, int format, int mipmapCount)
{
    if (RLSW.loadedTextureCount >= SW_MAX_TEXTURES) {
        RLSW.errCode = SW_STACK_OVERFLOW; //< Out of memory, not really stack overflow
        return 0;
    }

    sw_texture_t texture = { 0 };
    texture.pixels = data;
    texture.width = width;
    texture.height = height;
    texture.format = format;
    texture.minFilter = SW_NEAREST;
    texture.magFilter = SW_NEAREST;
    texture.sWrap = SW_REPEAT;
    texture.tWrap = SW_REPEAT;
    texture.tx = 1.0f / width;
    texture.ty = 1.0f / height;
    (void)mipmapCount;

    uint32_t id = 0;
    if (RLSW.freeTextureIdCount > 0) {
        id = RLSW.freeTextureIds[--RLSW.freeTextureIdCount];
    } 
    else {
        id = RLSW.loadedTextureCount++;
    }

    RLSW.loadedTextures[id] = texture;

    return id;
}

void swUnloadTexture(uint32_t id)
{
    if (!sw_is_texture_id_valid(id)) {
        RLSW.errCode = SW_INVALID_VALUE;
        return;
    }

    RLSW.loadedTextures[id].pixels = 0;
    RLSW.freeTextureIds[RLSW.freeTextureIdCount++] = id;
}

void swTextureParameters(uint32_t id, int param, int value)
{
    if (!sw_is_texture_id_valid(id)) {
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

    if (id > 0 && RLSW.loadedTextures[id].pixels == 0) {
        RLSW.errCode = SW_INVALID_OPERATION;
        return;
    }

    RLSW.currentTexture = id;
}

#endif // RLSW_IMPL
