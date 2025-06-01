#ifndef RLVK_H
#define RLVK_H

#include <vulkan/vulkan.h>

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

#ifdef __cplusplus
}
#endif

#endif // RLVK_H
