/**********************************************************************************************
*
*   raylib configuration flags
*
*   This file defines all the configuration flags for the different raylib modules
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2025 Ahmad Fatoum & Ramon Santamaria (@raysan5)
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

#ifndef CONFIG_H
#define CONFIG_H
#define SUPPORT_MODULE_RSHAPES          1
#define SUPPORT_MODULE_RTEXTURES        1
#define SUPPORT_MODULE_RTEXT            1
#define SUPPORT_MODULE_RMODELS          1
#define SUPPORT_MODULE_RAUDIO           1
#define SUPPORT_CAMERA_SYSTEM           1
#define SUPPORT_GESTURES_SYSTEM         1
#define SUPPORT_RPRAND_GENERATOR        1
#define SUPPORT_MOUSE_GESTURES          1
#define SUPPORT_SSH_KEYBOARD_RPI        1
#define SUPPORT_WINMM_HIGHRES_TIMER     1
#define SUPPORT_PARTIALBUSY_WAIT_LOOP    1
#define SUPPORT_SCREEN_CAPTURE          1
#define SUPPORT_GIF_RECORDING           1
#define SUPPORT_COMPRESSION_API         1
#define SUPPORT_AUTOMATION_EVENTS       1
#define SUPPORT_CLIPBOARD_IMAGE    1
#if defined(SUPPORT_CLIPBOARD_IMAGE)
    #ifndef SUPPORT_MODULE_RTEXTURES
        #define SUPPORT_MODULE_RTEXTURES 1
    #endif
    #ifndef STBI_REQUIRED
        #define STBI_REQUIRED
    #endif
    #ifndef SUPPORT_FILEFORMAT_BMP
        #define SUPPORT_FILEFORMAT_BMP 1
    #endif
    #ifndef SUPPORT_FILEFORMAT_PNG
        #define SUPPORT_FILEFORMAT_PNG 1
    #endif
    #ifndef SUPPORT_FILEFORMAT_JPG
        #define SUPPORT_FILEFORMAT_JPG 1
    #endif
#endif
#define MAX_FILEPATH_CAPACITY        8192
#define MAX_FILEPATH_LENGTH          4096

#define MAX_KEYBOARD_KEYS             512
#define MAX_MOUSE_BUTTONS               8
#define MAX_GAMEPADS                    4
#define MAX_GAMEPAD_AXES                8
#define MAX_GAMEPAD_BUTTONS            32
#define MAX_GAMEPAD_VIBRATION_TIME      2.0f
#define MAX_TOUCH_POINTS                8
#define MAX_KEY_PRESSED_QUEUE          16
#define MAX_CHAR_PRESSED_QUEUE         16

#define MAX_DECOMPRESSION_SIZE         64

#define MAX_AUTOMATION_EVENTS       16384

#define RL_SUPPORT_MESH_GPU_SKINNING           1
#define RL_DEFAULT_BATCH_BUFFERS               1
#define RL_DEFAULT_BATCH_DRAWCALLS           256
#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS     4

#define RL_MAX_MATRIX_STACK_SIZE              32

#define RL_MAX_SHADER_LOCATIONS               32

#define RL_CULL_DISTANCE_NEAR              0.05
#define RL_CULL_DISTANCE_FAR             4000.0
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION    0
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD    1
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL      2
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR       3
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT     4
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2   5
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES     6
#if defined(RL_SUPPORT_MESH_GPU_SKINNING)
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS     7
    #define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS 8
#endif
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCE_TX 9
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"
#define SUPPORT_QUADS_DRAW_MODE         1
#define SPLINE_SEGMENT_DIVISIONS       24
#define SUPPORT_FILEFORMAT_PNG      1
#define SUPPORT_FILEFORMAT_GIF      1
#define SUPPORT_FILEFORMAT_QOI      1
#define SUPPORT_FILEFORMAT_DDS      1
#define SUPPORT_IMAGE_EXPORT            1
#define SUPPORT_IMAGE_GENERATION        1
#define SUPPORT_IMAGE_MANIPULATION      1
#define SUPPORT_DEFAULT_FONT            1
#define SUPPORT_FILEFORMAT_TTF          1
#define SUPPORT_FILEFORMAT_FNT          1
#define SUPPORT_TEXT_MANIPULATION       1
#define SUPPORT_FONT_ATLAS_WHITE_REC    1
#define MAX_TEXT_BUFFER_LENGTH       1024
#define MAX_TEXTSPLIT_COUNT           128
#define SUPPORT_FILEFORMAT_OBJ          1
#define SUPPORT_FILEFORMAT_MTL          1
#define SUPPORT_FILEFORMAT_IQM          1
#define SUPPORT_FILEFORMAT_GLTF         1
#define SUPPORT_FILEFORMAT_VOX          1
#define SUPPORT_FILEFORMAT_M3D          1
#define SUPPORT_MESH_GENERATION         1
#define MAX_MATERIAL_MAPS              12

#ifdef RL_SUPPORT_MESH_GPU_SKINNING
#define MAX_MESH_VERTEX_BUFFERS         9
#else
#define MAX_MESH_VERTEX_BUFFERS         7
#endif
#define SUPPORT_FILEFORMAT_WAV          1
#define SUPPORT_FILEFORMAT_OGG          1
#define SUPPORT_FILEFORMAT_MP3          1
#define SUPPORT_FILEFORMAT_QOA          1
#define SUPPORT_FILEFORMAT_XM           1
#define SUPPORT_FILEFORMAT_MOD          1
#define AUDIO_DEVICE_FORMAT    ma_format_f32
#define AUDIO_DEVICE_CHANNELS              2
#define AUDIO_DEVICE_SAMPLE_RATE           0

#define MAX_AUDIO_BUFFER_POOL_CHANNELS    16
#define SUPPORT_STANDARD_FILEIO         1
#define SUPPORT_TRACELOG                1
#define MAX_TRACELOG_MSG_LENGTH       256

#ifdef PLATFORM_HEADLESS
    #undef SUPPORT_MODULE_RTEXTURES
    #undef SUPPORT_MODULE_RMODELS
    #undef SUPPORT_MODULE_RAUDIO
    #undef SUPPORT_CAMERA_SYSTEM
    #undef SUPPORT_GESTURES_SYSTEM
    #undef SUPPORT_MOUSE_GESTURES
    #undef SUPPORT_SSH_KEYBOARD_RPI
    #undef SUPPORT_PARTIALBUSY_WAIT_LOOP
    #undef SUPPORT_SCREEN_CAPTURE
    #undef SUPPORT_GIF_RECORDING
    #undef SUPPORT_AUTOMATION_EVENTS
    #undef SUPPORT_CLIPBOARD_IMAGE
    #undef SUPPORT_MODULE_RTEXTURES
    #undef STBI_REQUIRED
    #undef SUPPORT_FILEFORMAT_BMP
    #undef SUPPORT_FILEFORMAT_PNG
    #undef SUPPORT_FILEFORMAT_JPG
    #undef SUPPORT_QUADS_DRAW_MODE
    #undef SUPPORT_FILEFORMAT_PNG
    #undef SUPPORT_FILEFORMAT_BMP
    #undef SUPPORT_FILEFORMAT_TGA
    #undef SUPPORT_FILEFORMAT_JPG
    #undef SUPPORT_FILEFORMAT_GIF
    #undef SUPPORT_FILEFORMAT_QOI
    #undef SUPPORT_FILEFORMAT_PSD
    #undef SUPPORT_FILEFORMAT_DDS
    #undef SUPPORT_FILEFORMAT_HDR
    #undef SUPPORT_FILEFORMAT_PIC
    #undef SUPPORT_FILEFORMAT_KTX
    #undef SUPPORT_FILEFORMAT_ASTC
    #undef SUPPORT_FILEFORMAT_PKM
    #undef SUPPORT_FILEFORMAT_PVR
    #undef SUPPORT_IMAGE_EXPORT
    #undef SUPPORT_IMAGE_GENERATION
    #undef SUPPORT_IMAGE_MANIPULATION
    #undef SUPPORT_DEFAULT_FONT
    #undef SUPPORT_FILEFORMAT_TTF
    #undef SUPPORT_FILEFORMAT_FNT
    #undef SUPPORT_FONT_ATLAS_WHITE_REC
    #undef SUPPORT_FILEFORMAT_OBJ
    #undef SUPPORT_FILEFORMAT_MTL
    #undef SUPPORT_FILEFORMAT_IQM
    #undef SUPPORT_FILEFORMAT_GLTF
    #undef SUPPORT_FILEFORMAT_VOX
    #undef SUPPORT_FILEFORMAT_M3D
    #undef SUPPORT_MESH_GENERATION
    #undef SUPPORT_FILEFORMAT_WAV
    #undef SUPPORT_FILEFORMAT_OGG
    #undef SUPPORT_FILEFORMAT_MP3
    #undef SUPPORT_FILEFORMAT_QOA
    #undef SUPPORT_FILEFORMAT_XM
    #undef SUPPORT_FILEFORMAT_MOD


    // Define all rlgl functions to be nothing
    #define rlMatrixMode(mode) ((void)0)
    #define rlPushMatrix() ((void)0)
    #define rlPopMatrix() ((void)0)
    #define rlLoadIdentity() ((void)0)
    #define rlTranslatef(x, y, z) ((void)0)
    #define rlRotatef(angle, x, y, z) ((void)0)
    #define rlScalef(x, y, z) ((void)0)
    #define rlMultMatrixf(matf) ((void)0)
    #define rlFrustum(left, right, bottom, top, znear, zfar) ((void)0)
    #define rlOrtho(left, right, bottom, top, znear, zfar) ((void)0)
    #define rlViewport(x, y, width, height) ((void)0)
    #define rlSetClipPlanes(nearPlane, farPlane) ((void)0)
    #define rlGetCullDistanceNear() ((void)0)
    #define rlGetCullDistanceFar() ((void)0)
    #define rlBegin(mode) ((void)0)
    #define rlEnd() ((void)0)
    #define rlVertex2i(x, y) ((void)0)
    #define rlVertex2f(x, y) ((void)0)
    #define rlVertex3f(x, y, z) ((void)0)
    #define rlTexCoord2f(x, y) ((void)0)
    #define rlNormal3f(x, y, z) ((void)0)
    #define rlColor4ub(r, g, b, a) ((void)0)
    #define rlColor3f(x, y, z) ((void)0)
    #define rlColor4f(x, y, z, w) ((void)0)
    #define rlEnableVertexArray(vaoId) ((void)0)
    #define rlDisableVertexArray() ((void)0)
    #define rlEnableVertexBuffer(id) ((void)0)
    #define rlDisableVertexBuffer() ((void)0)
    #define rlEnableVertexBufferElement(id) ((void)0)
    #define rlDisableVertexBufferElement() ((void)0)
    #define rlEnableVertexAttribute(index) ((void)0)
    #define rlDisableVertexAttribute(index) ((void)0)
    #define rlEnableStatePointer(vertexAttribType, buffer) ((void)0)
    #define rlDisableStatePointer(vertexAttribType) ((void)0)
    #define rlActiveTextureSlot(slot) ((void)0)
    #define rlEnableTexture(id) ((void)0)
    #define rlDisableTexture() ((void)0)
    #define rlEnableTextureCubemap(id) ((void)0)
    #define rlDisableTextureCubemap() ((void)0)
    #define rlTextureParameters(id, param, value) ((void)0)
    #define rlCubemapParameters(id, param, value) ((void)0)
    #define rlEnableShader(id) ((void)0)
    #define rlDisableShader() ((void)0)
    #define rlEnableFramebuffer(id) ((void)0)
    #define rlDisableFramebuffer() ((void)0)
    #define rlGetActiveFramebuffer() ((void)0)
    #define rlActiveDrawBuffers(count) ((void)0)
    #define rlBlitFramebuffer(srcX, srcY, srcWidth, srcHeight, dstX, dstY, dstWidth, dstHeight, bufferMask) ((void)0)
    #define rlBindFramebuffer(target, framebuffer) ((void)0)
    #define rlEnableColorBlend() ((void)0)
    #define rlDisableColorBlend() ((void)0)
    #define rlEnableDepthTest() ((void)0)
    #define rlDisableDepthTest() ((void)0)
    #define rlEnableDepthMask() ((void)0)
    #define rlDisableDepthMask() ((void)0)
    #define rlEnableBackfaceCulling() ((void)0)
    #define rlDisableBackfaceCulling() ((void)0)
    #define rlColorMask(r, g, b, a) ((void)0)
    #define rlSetCullFace(mode) ((void)0)
    #define rlEnableScissorTest() ((void)0)
    #define rlDisableScissorTest() ((void)0)
    #define rlScissor(x, y, width, height) ((void)0)
    #define rlEnablePointMode() ((void)0)
    #define rlDisablePointMode() ((void)0)
    #define rlEnableWireMode() ((void)0)
    #define rlDisableWireMode() ((void)0)
    #define rlSetLineWidth(width) ((void)0)
    #define rlGetLineWidth() ((void)0)
    #define rlEnableSmoothLines() ((void)0)
    #define rlDisableSmoothLines() ((void)0)
    #define rlEnableStereoRender() ((void)0)
    #define rlDisableStereoRender() ((void)0)
    #define rlIsStereoRenderEnabled() ((void)0)
    
    #define rlClearColor(r, g, b, a) ((void)0)
    #define rlClearScreenBuffers() ((void)0)
    #define rlCheckErrors() ((void)0)
    #define rlSetBlendMode(mode) ((void)0)
    #define rlSetBlendFactors(glSrcFactor, glDstFactor, glEquation) ((void)0)
    #define rlSetBlendFactorsSeparate(glSrcRGB, glDstRGB, glSrcAlpha, glDstAlpha, glEqRGB, glEqAlpha) ((void)0)
    #define rlglInit(width, height) ((void)0)
    #define rlglClose() ((void)0)
    #define rlLoadExtensions(loader) ((void)0)
    #define rlGetProcAddress(procName) ((void)0)
    #define rlGetVersion() ((void)0)
    #define rlSetFramebufferWidth(width) ((void)0)
    #define rlGetFramebufferWidth() ((void)0)
    #define rlSetFramebufferHeight(height) ((void)0)
    #define rlGetFramebufferHeight() ((void)0)
    
    #define rlGetTextureIdDefault() ((void)0)
    #define rlGetShaderIdDefault() ((void)0)
    #define rlGetShaderLocsDefault() ((void)0)
    #define rlLoadRenderBatch(numBuffers, bufferElements) ((void)0)
    #define rlUnloadRenderBatch(batch) ((void)0)
    #define rlDrawRenderBatch(batch) ((void)0)
    #define rlSetRenderBatchActive(batch) ((void)0)
    #define rlDrawRenderBatchActive() ((void)0)
    #define rlCheckRenderBatchLimit(vCount) ((void)0)
    
    #define rlSetTexture(id) ((void)0)
    #define rlLoadVertexArray() ((void)0)
    #define rlLoadVertexBuffer(buffer, size, dynamic) ((void)0)
    #define rlLoadVertexBufferElement(buffer, size, dynamic) ((void)0)
    #define rlUpdateVertexBuffer(bufferId, data, dataSize, offset) ((void)0)
    #define rlUpdateVertexBufferElements(id, data, dataSize, offset) ((void)0)
    #define rlUnloadVertexArray(vaoId) ((void)0)
    #define rlUnloadVertexBuffer(vboId) ((void)0)
    #define rlSetVertexAttribute(index, compSize, type, normalized, stride, offset) ((void)0)
    #define rlSetVertexAttributeDivisor(index, divisor) ((void)0)
    #define rlSetVertexAttributeDefault(locIndex, value, attribType, count) ((void)0)
    #define rlDrawVertexArray(offset, count) ((void)0)
    #define rlDrawVertexArrayElements(offset, count, buffer) ((void)0)
    #define rlDrawVertexArrayInstanced(offset, count, instances) ((void)0)
    #define rlDrawVertexArrayElementsInstanced(offset, count, buffer, instances) ((void)0)
    #define rlLoadTexture(data, width, height, format, mipmapCount) ((void)0)
    #define rlLoadTextureDepth(width, height, useRenderBuffer) ((void)0)
    #define rlLoadTextureCubemap(data, size, format, mipmapCount) ((void)0)
    #define rlUpdateTexture(id, offsetX, offsetY, width, height, format, data) ((void)0)
    #define rlGetGlTextureFormats(format, glInternalFormat, glFormat, glType) ((void)0)
    #define rlGetPixelFormatName(format) ((void)0)
    #define rlUnloadTexture(id) ((void)0)
    #define rlGenTextureMipmaps(id, width, height, format, mipmaps) ((void)0)
    #define rlReadTexturePixels(id, width, height, format) ((void)0)
    #define rlReadScreenPixels(width, height) ((void)0)
    #define rlLoadFramebuffer() ((void)0)
    #define rlFramebufferAttach(fboId, texId, attachType, texType, mipLevel) ((void)0)
    #define rlFramebufferComplete(id) ((void)0)
    #define rlUnloadFramebuffer(id) ((void)0)
    #define rlLoadShaderCode(vsCode, fsCode) ((void)0)
    #define rlCompileShader(shaderCode, type) ((void)0)
    #define rlLoadShaderProgram(vShaderId, fShaderId) ((void)0)
    #define rlUnloadShaderProgram(id) ((void)0)
    #define rlGetLocationUniform(shaderId, uniformName) ((void)0)
    #define rlGetLocationAttrib(shaderId, attribName) ((void)0)
    #define rlSetUniform(locIndex, value, uniformType, count) ((void)0)
    #define rlSetUniformMatrix(locIndex, mat) ((void)0)
    #define rlSetUniformMatrices(locIndex, mat, count) ((void)0)
    #define rlSetUniformSampler(locIndex, textureId) ((void)0)
    #define rlSetShader(id, locs) ((void)0)
    #define rlLoadComputeShaderProgram(shaderId) ((void)0)
    #define rlComputeShaderDispatch(groupX, groupY, groupZ) ((void)0)
    #define rlLoadShaderBuffer(size, data, usageHint) ((void)0)
    #define rlUnloadShaderBuffer(ssboId) ((void)0)
    #define rlUpdateShaderBuffer(id, data, dataSize, offset) ((void)0)
    #define rlBindShaderBuffer(id, index) ((void)0)
    #define rlReadShaderBuffer(id, dest, count, offset) ((void)0)
    #define rlCopyShaderBuffer(destId, srcId, destOffset, srcOffset, count) ((void)0)
    #define rlGetShaderBufferSize(id) ((void)0)
    #define rlBindImageTexture(id, index, format, readonly) ((void)0)
    #define rlGetMatrixModelview() ((void)0)
    #define rlGetMatrixProjection() ((void)0)
    #define rlGetMatrixTransform() ((void)0)
    #define rlGetMatrixProjectionStereo(eye) ((void)0)
    #define rlGetMatrixViewOffsetStereo(eye) ((void)0)
    #define rlSetMatrixProjection(proj) ((void)0)
    #define rlSetMatrixModelview(view) ((void)0)
    #define rlSetMatrixProjectionStereo(right, left) ((void)0)
    #define rlSetMatrixViewOffsetStereo(right, left) ((void)0)
    #define rlLoadDrawCube() ((void)0)
    #define rlLoadDrawQuad() ((void)0)

#endif

#endif
