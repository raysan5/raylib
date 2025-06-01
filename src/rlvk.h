#ifndef RLVK_H
#define RLVK_H

#include <vulkan/vulkan.h>

// rlgl data structures replacement for Vulkan
// NOTE: Maximum number of vertex supported in a single batch
// RL_DEFAULT_BATCH_BUFFER_ELEMENTS * 4 vertices by default -> 8192*4 = 32768
// This define is only used on rlgl VULKAN backend to limit arrays size
// It has no meaning on any other backend
#ifndef RLVK_MAX_BATCH_ELEMENTS
    #define RLVK_MAX_BATCH_ELEMENTS 8192
#endif

// Vertex data structure
// NOTE: sizeof(rlvkVertex) is 28 bytes
typedef struct rlvkVertex {
    float position[3];      // Vertex position (x, y, z)
    unsigned char color[4]; // Vertex color (r, g, b, a)
    float texcoord[2];      // Vertex texture coordinates (x, y)
} rlvkVertex;

// Vulkan buffer structure
typedef struct rlvkBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
} rlvkBuffer;

#ifdef __cplusplus
extern "C" {
#endif

// Initialization and Configuration
void rlvkInit(VkInstance instance, VkSurfaceKHR surface, int width, int height); // Simplified, may need more params
void rlvkClose(void);
bool rlvkIsReady(void);

// Drawing
void rlvkBeginDrawing(void);
void rlvkEndDrawing(void);
void rlvkClearBackground(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

// Basic Texture Management (Stubs)
unsigned int rlvkLoadTexture(const void *data, int width, int height, int format, int mipmaps);
void rlvkUnloadTexture(unsigned int id);

// Basic Shader Management (Stubs)
unsigned int rlvkLoadShaderCode(const char *vsCode, const char *fsCode);
void rlvkUnloadShaderProgram(unsigned int id);
int rlvkGetLocationUniform(unsigned int shaderId, const char *uniformName);
int rlvkGetLocationAttrib(unsigned int shaderId, const char *attribName);
void rlvkSetUniform(int locIndex, const void *value, int uniformType, int count);
// ... other rlgl equivalent function declarations as stubs ...

// Vertex Buffer Management
void rlvkInitializeVertexBuffer(void);
void rlvkResizeVertexBuffer(void);
void rlvkResetVertexBuffer(void);
void rlvkDestroyVertexBuffer(void);

// Vertex Data Control
void rlvkSetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void rlvkSetTexCoord(float x, float y);
void rlvkAddVertex(float x, float y, float z);

// Primitive mode setting
void rlvkSetPrimitiveMode(int mode);


#ifdef __cplusplus
}
#endif

#endif // RLVK_H
