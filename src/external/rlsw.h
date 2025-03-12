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
 
 typedef enum {
     SW_PROJECTION,
     SW_MODELVIEW,
     SW_TEXTURE
 } SWmatrix;
 
 typedef enum {
     SW_VERTEX_ARRAY,
     SW_TEXTURE_COORD_ARRAY,
     SW_NORMAL_ARRAY,
     SW_COLOR_ARRAY
 } SWarray;
 
 typedef enum {
     SW_POINTS,
     SW_LINES,
     SW_TRIANGLES,
     SW_QUADS,
 } SWfill;
 
 typedef enum {
     SW_CULL_FRONT,
     SW_CULL_BACK,
 } SWcull;
 
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
     SW_NEAREST,
     SW_LINEAR,
     SW_NEAREST_MIPMAP_NEAREST,
     SW_NEAREST_MIPMAP_LINEAR,
     SW_LINEAR_MIPMAP_NEAREST,
     SW_LINEAR_MIPMAP_LINEAR
 } SWfilter;
 
 typedef enum {
     SW_REPEAT,
     SW_CLAMP_TO_EDGE,
     SW_MIRRORED_REPEAT
 } SWwrap;
 
 typedef enum {
     SW_TEXTURE_MIN_FILTER,
     SW_TEXTURE_MAG_FILTER,
     SW_TEXTURE_WRAP_S,
     SW_TEXTURE_WRAP_T
 } SWtexparam;
 
 typedef enum {
     SW_NO_ERROR,
     SW_INVALID_ENUM,
     SW_INVALID_VALUE,
     SW_STACK_OVERFLOW,
     SW_STACK_UNDERFLOW,
     SW_INVALID_OPERATION,
     SW_ERROR_OUT_OF_MEMORY
 } SWerrcode;
 
 /* === Public API === */
 
 void swInit(int w, int h);
 void swClose(void);
 
 void* swGetColorBuffer(int* w, int* h);
 
 void swMatrixMode(SWmatrix mode);
 void swPushMatrix(void);
 void swPopMatrix(void);
 void swLoadIdentity(void);
 void swTranslatef(float x, float y, float z);
 void swRotatef(float angle, float x, float y, float z);
 void swScalef(float x, float y, float z);
 void swMultMatrixf(const float* mat);
 void swFrustum(float left, float right, float bottom, float top, float znear, float zfar);
 void swOrtho(float left, float right, float bottom, float top, float znear, float zfar);
 
 void swViewport(int x, int y, int width, int height);
 
 void swClearColor(float r, float g, float b, float a);
 void swClear(void);
 
 void swBegin(SWfill mode);
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
 void swDrawArrays(SWfill mode, int offset, int count);
 
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
 
     uint32_t blendFunction;
     uint32_t depthFunction;
 
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
 
     SWfill fillMode;                                            // Current polygon filling mode (e.g., lines, triangles)
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
 
     SWcull cullFace;                                            // Faces to cull
     SWerrcode errCode;                                          // Last error code
 
     sw_texture_t* loadedTextures;
     int loadedTextureCount;
 
     uint32_t* freeTextureIds;
     int freeTextureIdCount;
 
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
 
 static inline void sw_map_repeat(int* out, float in, int max)
 {
     // Upscale to nearest texture coordinates
     // NOTE: We use '(int)(x+0.5)' although this is incorrect
     //       regarding the direction of rounding in case of negative values
     //       and also less accurate than roundf, but it remains so much more
     //       efficient that it is preferable for now to opt for this option.
 
     *out = abs((int)((in - (int)in) * (max - 1) + 0.5f));
 }
 
 static inline void sw_map_clamp_to_edge(int* out, float in, int max)
 {
     in = (in > 1.0f) ? 1.0f : ((in < 0.0f) ? 0.0f : in);
     *out = (int)(in * (max - 1) + 0.5f);
 }
 
 static inline void sw_map_mirrored_repeat(int* out, float in, int max)
 {
     in = fmodf(fabsf(in), 2);
     if (in > 1.0f) in = 1.0f - (in - 1.0f);
     *out = (int)(in * (max - 1) + 0.5f);
 }
 
 static inline void sw_map(int* out, float in, int max, SWwrap mode)
 {
     switch (mode) {
     case SW_REPEAT:
         sw_map_repeat(out, in, max);
         break;
     case SW_CLAMP_TO_EDGE:
         sw_map_clamp_to_edge(out, in, max);
         break;
     case SW_MIRRORED_REPEAT:
         sw_map_mirrored_repeat(out, in, max);
         break;
     }
 }
 
 static inline void sw_sample_texture_nearest(float* color, const sw_texture_t* tex, float u, float v)
 {
     int x, y;
     sw_map(&x, u, tex->width, tex->sWrap);
     sw_map(&y, v, tex->height, tex->tWrap);
     sw_get_pixel(color, tex->pixels, y * tex->width + x, tex->format);
 }
 
 static inline void sw_sample_texture_bilinear(float* color, const sw_texture_t* tex, float u, float v)
 {
     int x0, y0, x1, y1;
     sw_map(&x0, u, tex->width, tex->sWrap);
     sw_map(&y0, v, tex->height, tex->tWrap);
     sw_map(&x1, u + tex->tx, tex->width, tex->sWrap);
     sw_map(&y1, v + tex->ty, tex->height, tex->tWrap);
 
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
 
 static inline void sw_sample_texture(float* color, const sw_texture_t* tex, float u, float v,
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
             sw_sample_texture_nearest(color, tex, u, v);
         } else if (tex->minFilter == SW_LINEAR) {
             sw_sample_texture_bilinear(color, tex, u, v);
         }
     } else {
         // Magnification
         if (tex->magFilter == SW_NEAREST) {
             sw_sample_texture_nearest(color, tex, u, v);
         } else if (tex->magFilter == SW_LINEAR) {
             sw_sample_texture_bilinear(color, tex, u, v);
         }
     }
 }
 
 static inline bool sw_clip_polygon_w(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
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
 
 static inline bool sw_clip_polygon_xyz(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
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
 
 void sw_project_and_clip_triangle(sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES], int* vertexCounter)
 {
     for (int i = 0; i < *vertexCounter; i++) {
         sw_vertex_t *v = polygon + i;
         for (int j = 0; j < 4; j++) v->homogeneous[j] = v->position[j];
         sw_vec4_transform(v->homogeneous, v->homogeneous, RLSW.matMVP);
     }
 
     if (sw_clip_polygon_w(polygon, vertexCounter) && sw_clip_polygon_xyz(polygon, vertexCounter)) {
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
 
             // Transform to screen space
             v->screen[0] = RLSW.vpPos[0] + (v->homogeneous[0] + 1.0f) * 0.5f * RLSW.vpDim[0];
             v->screen[1] = RLSW.vpPos[1] + (v->homogeneous[1] + 1.0f) * 0.5f * RLSW.vpDim[1];
         }
     }
 }
 
 void sw_raster_scanline(const sw_texture_t* tex, const sw_vertex_t* start, const sw_vertex_t* end, float yDu, float yDv)
 {
     // Calculate the horizontal width and avoid division by zero
     float dx = end->screen[0] - start->screen[0];
     if (fabsf(dx) < 1e-4f) return;
     
     // Convert and center the screen coordinates
     int xStart = (int)(start->screen[0] + 0.5f);
     int xEnd   = (int)(end->screen[0] + 0.5f);
     int y      = (int)(start->screen[1] + 0.5f);
     
     // Calculate the initial interpolation parameter and its increment
     float dt = 1.0f / dx;
     float t  = (xStart - start->screen[0]) * dt;
 
     // Calculate the horizontal gradients for UV coordinates
     float xDu = (end->texcoord[0] - start->texcoord[0]) * dt;
     float xDv = (end->texcoord[1] - start->texcoord[1]) * dt;
     
     // Pre-calculate the color differences for interpolation
     float dcol[4];
     for (int i = 0; i < 4; i++) {
         dcol[i] = end->color[i] - start->color[i];
     }
 
     // Pre-calculate the differences in Z and W (for depth testing and perspective correction)
     float dz = end->homogeneous[2] - start->homogeneous[2];
     float dw = end->homogeneous[3] - start->homogeneous[3];
     
     // Initialize the interpolated texture coordinates
     float u = start->texcoord[0] + t * xDu;
     float v = start->texcoord[1] + t * xDv;
     
     // Pre-calculate the starting pointer for the color framebuffer row
     uint8_t* row_ptr = (uint8_t*)((uint32_t*)RLSW.framebuffer.color + y * RLSW.framebuffer.width);
     uint8_t* dst = row_ptr + xStart * 4;
     
     // Pre-calculate the pointer for the depth buffer row
     uint16_t* depth_row = RLSW.framebuffer.depth + y * RLSW.framebuffer.width + xStart;
     uint16_t* dptr = depth_row;
     
     // Scanline rasterization loop
     for (int x = xStart; x < xEnd; x++) {
         // Interpolate Z and W for depth testing and perspective correction
         float w = 1.0f / (start->homogeneous[3] + t * dw);
         float z = start->homogeneous[2] + t * dz;
         
         // Depth testing with direct access to the depth buffer
         // TODO: Implement different depth funcs?
         float depth = (float)(*dptr) / UINT16_MAX;
         if (z > depth) goto discard;
         
         // Update the depth buffer
         *dptr = (uint16_t)(z * UINT16_MAX);
         
         // Sample the texture
         float texColor[4];
         sw_sample_texture(texColor, tex, u * w, v * w, xDu, yDu, xDv, yDv);
         
         // Interpolate the color and modulate by the texture color
         for (int i = 0; i < 4; i++) {
             float lerp = start->color[i] + t * dcol[i];
             float finalColor = texColor[i] * lerp;
             // Inline clamp to keep the value between 0 and 1
             // NOTE: The need for clamp, the colors could be a sign of problem during interpolation (?)
             finalColor = (finalColor < 0.0f) ? 0.0f : (finalColor > 1.0f ? 1.0f : finalColor);
             dst[i] = (uint8_t)(finalColor * 255.0f);
         }
         
         // Increment the interpolation parameter, UVs, and pointers
         discard:
         t += dt;
         u += xDu;
         v += xDv;
         dst += 4;
         dptr++;
     }
 }
 
 void sw_raster_triangle(const sw_vertex_t* v0, const sw_vertex_t* v1, const sw_vertex_t* v2, const sw_texture_t* tex)
 {
     // Swap vertices by increasing y
     if (v0->screen[1] > v1->screen[1]) { const sw_vertex_t* tmp = v0; v0 = v1; v1 = tmp; }
     if (v1->screen[1] > v2->screen[1]) { const sw_vertex_t* tmp = v1; v1 = v2; v2 = tmp; }
     if (v0->screen[1] > v1->screen[1]) { const sw_vertex_t* tmp = v0; v0 = v1; v1 = tmp; }
 
     // Extracting coordinates from the sorted vertices
     float x0 = v0->screen[0], y0 = v0->screen[1];
     float x1 = v1->screen[0], y1 = v1->screen[1];
     float x2 = v2->screen[0], y2 = v2->screen[1];
 
     // Reject degenerate triangles
     float height = y2 - y0;
     if (height < 1e-4f) return;
 
     // Global calculation of vertical texture gradients for the triangle
     float yDu = (v2->texcoord[0] - v0->texcoord[0]) / height;
     float yDv = (v2->texcoord[1] - v0->texcoord[1]) / height;
 
     // Precompute the inverse of the triangle height and
     // edge lengths with checks to avoid division by zero.
     float inv_height = 1.0f / height;
     float inv_y1y0 = (y1 - y0 > 1e-4f) ? 1.0f / (y1 - y0) : 0.0f;
     float inv_y2y1 = (y2 - y1 > 1e-4f) ? 1.0f / (y2 - y1) : 0.0f;
 
     // Pre-calculation of slopes (dx/dy)
     float dx02 = (x2 - x0) * inv_height;
     float dx01 = (x1 - x0) * inv_y1y0;
     float dx12 = (x2 - x1) * inv_y2y1;
 
     // Y bounds (vertical clipping)
     int yTop = (int)(y0 + 0.5f);
     int yMiddle = (int)(y1 + 0.5f);
     int yBottom = (int)(y2 + 0.5f);
 
     // Initializing scanline variables
     float xLeft = x0, xRight = x0;
     sw_vertex_t start, end;
 
     // Scanline for the upper part of the triangle
     for (int y = yTop; y < yMiddle; y++) {
         float dy = (float)y - y0;
         float t1 = dy * inv_height;
         float t2 = dy * inv_y1y0;
 
         // Optimized interpolation
         start = sw_lerp_vertex(v0, v2, t1);
         end   = sw_lerp_vertex(v0, v1, t2);
         start.screen[0] = xLeft;
         start.screen[1] = (float)y;
         end.screen[0] = xRight;
         end.screen[1] = (float)y;
 
         if (xLeft > xRight) { sw_vertex_t tmp = start; start = end; end = tmp; }
         sw_raster_scanline(tex, &start, &end, yDu, yDv);
 
         // Incremental update
         xLeft  += dx02;
         xRight += dx01;
     }
 
     // Scanline for the lower part of the triangle
     xRight = x1; // Restart the right side from the second vertex
     for (int y = yMiddle; y < yBottom; y++) {
         float dy = (float)y - y0;
         float t1 = dy * inv_height;
         float t2 = (float)(y - y1) * inv_y2y1;
 
         // Optimized interpolation
         start = sw_lerp_vertex(v0, v2, t1);
         end   = sw_lerp_vertex(v1, v2, t2);
         start.screen[0] = xLeft;
         start.screen[1] = (float)y;
         end.screen[0] = xRight;
         end.screen[1] = (float)y;
 
         if (xLeft > xRight) { sw_vertex_t tmp = start; start = end; end = tmp; }
         sw_raster_scanline(tex, &start, &end, yDu, yDv);
 
         // Incremental update
         xLeft  += dx02;
         xRight += dx12;
     }
 }
 
 void sw_render_triangle(const sw_vertex_t* v0, const sw_vertex_t* v1, const sw_vertex_t* v2)
 {
     int vertexCounter = 3;
 
     sw_vertex_t polygon[SW_MAX_CLIPPED_POLYGON_VERTICES];
     polygon[0] = *v0;
     polygon[1] = *v1;
     polygon[2] = *v2;
 
     sw_project_and_clip_triangle(polygon, &vertexCounter);
 
     if (vertexCounter < 3) {
         return;
     }
 
     for (int_fast8_t i = 0; i < vertexCounter - 2; i++) {
         sw_raster_triangle(
             &polygon[0], &polygon[i + 1], &polygon[i + 2],
             &RLSW.loadedTextures[RLSW.currentTexture]
         );
     }
 }
 
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
 
     sw_matrix_t mat;
     sw_matrix_id(mat);
 
     float lengthSq = x*x + y*y + z*z;
 
     if (lengthSq != 1.0f && lengthSq != 0.0f) {
         float invLenght = 1.0f / lengthSq;
         x *= invLenght;
         y *= invLenght;
         z *= invLenght;
     }
 
     float sinres = sinf(angle);
     float cosres = cosf(angle);
     float t = 1.0f - cosres;
 
     mat[0]  = x*x*t + cosres;
     mat[1]  = y*x*t + z*sinres;
     mat[2]  = z*x*t - y*sinres;
 
     mat[4]  = x*y*t - z*sinres;
     mat[5]  = y*y*t + cosres;
     mat[6]  = z*y*t + x*sinres;
 
     mat[8]  = x*z*t + y*sinres;
     mat[9]  = y*z*t - x*sinres;
     mat[10] = z*z*t + cosres;
 
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
 
 void swFrustum(float left, float right, float bottom, float top, float znear, float zfar)
 {
     sw_matrix_t mat = { 0 };
 
     float rl = right - left;
     float tb = top - bottom;
     float fn = zfar - znear;
 
     mat[0] = (znear * 2.0f) / rl;
     mat[5] = (znear * 2.0f) / tb;
 
     mat[8] = (right + left) / rl;
     mat[9] = (top + bottom) / tb;
     mat[10] = -(zfar + znear) / fn;
     mat[11] = -1.0f;
 
     mat[14] = -(zfar * znear * 2.0f) / fn;
 
     sw_matrix_mul(*RLSW.currentMatrix, *RLSW.currentMatrix, mat);
 }
 
 void swOrtho(float left, float right, float bottom, float top, float znear, float zfar)
 {
     sw_matrix_t mat = { 0 };
 
     float rl = (right - left);
     float tb = (top - bottom);
     float fn = (zfar - znear);
 
     mat[0] = 2.0f / rl;
     mat[5] = 2.0f / tb;
 
     mat[10] = -2.0f / fn;
     mat[11] = 0.0f;
     mat[12] = -(left + right) / rl;
     mat[13] = -(top + bottom) / tb;
 
     mat[14] = -(zfar + znear) / fn;
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
 
 void swBegin(SWfill mode)
 {
     if (mode < SW_POINTS || mode > SW_QUADS) {
         RLSW.errCode = SW_INVALID_ENUM;
         return;
     }
     RLSW.vertexCounter = 0;
     RLSW.fillMode = mode;
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
     switch (RLSW.fillMode) {
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
 
         switch (RLSW.fillMode) {
         case SW_POINTS:
             break;
         case SW_LINES:
             neededVertices = 2;
             break;
         case SW_TRIANGLES:
             sw_render_triangle(
                 &RLSW.vertexBuffer[0],
                 &RLSW.vertexBuffer[1],
                 &RLSW.vertexBuffer[2]
             );
             break;
         case SW_QUADS:
             sw_render_triangle(
                 &RLSW.vertexBuffer[0],
                 &RLSW.vertexBuffer[1],
                 &RLSW.vertexBuffer[2]
             );
             sw_render_triangle(
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
 
 void swDrawArrays(SWfill mode, int offset, int count)
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
         RLSW.errCode = SW_ERROR_OUT_OF_MEMORY;
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
 