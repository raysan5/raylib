/**********************************************************************************************
*
*   raylib configuration flags
*
*   This file defines the configuration flags for different raylib features per-module
*
*   NOTE: Additional values are configured per-module and can be set on compile time
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2026 Ahmad Fatoum and Ramon Santamaria (@raysan5)
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

#if !defined(EXTERNAL_CONFIG_FLAGS)

//------------------------------------------------------------------------------------
// Module selection - Some modules could be avoided
// Mandatory modules: rcore, rlgl
//------------------------------------------------------------------------------------
#ifndef SUPPORT_MODULE_RSHAPES
    #define SUPPORT_MODULE_RSHAPES      1
#endif
#ifndef SUPPORT_MODULE_RTEXTURES
    #define SUPPORT_MODULE_RTEXTURES    1
#endif
#ifndef SUPPORT_MODULE_RTEXT
    #define SUPPORT_MODULE_RTEXT        1       // WARNING: It requires SUPPORT_MODULE_RTEXTURES to load sprite font textures
#endif
#ifndef SUPPORT_MODULE_RMODELS
    #define SUPPORT_MODULE_RMODELS      1
#endif
#ifndef SUPPORT_MODULE_RAUDIO
    #define SUPPORT_MODULE_RAUDIO       1
#endif

//------------------------------------------------------------------------------------
// Module: rcore - Configuration Flags
//------------------------------------------------------------------------------------
#ifndef SUPPORT_TRACELOG
    // Show TRACELOG() output messages
    #define SUPPORT_TRACELOG                1
#endif
#ifndef SUPPORT_CAMERA_SYSTEM
    // Camera module is included (rcamera.h) and multiple predefined 
    // cameras are available: free, 1st/3rd person, orbital
    #define SUPPORT_CAMERA_SYSTEM           1
#endif
#ifndef SUPPORT_GESTURES_SYSTEM
    // Gestures module is included (rgestures.h) to support gestures detection: tap, hold, swipe, drag
    #define SUPPORT_GESTURES_SYSTEM         1
#endif
#ifndef SUPPORT_RPRAND_GENERATOR
    // Include pseudo-random numbers generator (rprand.h), based on Xoshiro128** and SplitMix64
    #define SUPPORT_RPRAND_GENERATOR        1
#endif
#ifndef SUPPORT_MOUSE_GESTURES
    // Mouse gestures are directly mapped like touches and processed by gestures system
    #define SUPPORT_MOUSE_GESTURES          1
#endif
#ifndef SUPPORT_SSH_KEYBOARD_RPI
    // Reconfigure standard input to receive key inputs, works with SSH connection
    #define SUPPORT_SSH_KEYBOARD_RPI        1
#endif
#ifndef SUPPORT_WINMM_HIGHRES_TIMER
    // Setting a higher resolution can improve the accuracy of time-out intervals in wait functions
    // However, it can also reduce overall system performance, because the thread scheduler switches tasks more often
    #define SUPPORT_WINMM_HIGHRES_TIMER     1
#endif
#if !SUPPORT_BUSY_WAIT_LOOP && !SUPPORT_PARTIALBUSY_WAIT_LOOP
    // Use busy wait loop for timing sync, if not defined, a high-resolution timer is set up and used
    #define SUPPORT_BUSY_WAIT_LOOP          0       // Disabled by default
#endif
#if !SUPPORT_PARTIALBUSY_WAIT_LOOP && !SUPPORT_BUSY_WAIT_LOOP
    // Use a partial-busy wait loop, in this case frame sleeps for most of the time, 
    // but then runs a busy loop at the end for accuracy
    #define SUPPORT_PARTIALBUSY_WAIT_LOOP   1
#endif
#ifndef SUPPORT_SCREEN_CAPTURE
    // Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
    // WARNING: It requires SUPPORT_FILEFORMAT_PNG flag
    #define SUPPORT_SCREEN_CAPTURE          1
#endif
#ifndef SUPPORT_COMPRESSION_API
    // Support CompressData() and DecompressData() functions
    #define SUPPORT_COMPRESSION_API         1
#endif
#ifndef SUPPORT_AUTOMATION_EVENTS
    // Support automatic generated events, loading and recording of those events when required
    #define SUPPORT_AUTOMATION_EVENTS       1
#endif
#ifndef SUPPORT_CUSTOM_FRAME_CONTROL
    // Support custom frame control, only for advanced users
    // By default EndDrawing() does this job: draws everything + SwapScreenBuffer() + manage frame timing + PollInputEvents()
    // Enabling this flag allows manual control of the frame processes, use at your own risk
    #define SUPPORT_CUSTOM_FRAME_CONTROL    0       // Disabled by default
#endif
#ifndef SUPPORT_CLIPBOARD_IMAGE
    // Support for clipboard image loading
    // NOTE: Only working on SDL3, GLFW (Windows) and RGFW (Windows)
    // WARNING: It requires support for some additional flags:
    //  - SUPPORT_MODULE_RTEXTURES
    //  - SUPPORT_FILEFORMAT_BMP (Windows clipboard)
    //  - SUPPORT_FILEFORMAT_PNG (Wayland clipboard)
    //  - SUPPORT_FILEFORMAT_JPG
    #define SUPPORT_CLIPBOARD_IMAGE         1
#endif

#if SUPPORT_TRACELOG
    #define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)
#else
    #define TRACELOG(level, ...) (void)0
#endif

// rcore: Configuration values
// NOTE: Below values are alread defined inside [rcore.c] so there is no need to be
// redefined here, in case it must be done, just uncomment the required line and update
// the value; it can also be done on compilation with -DVALUE_TO_REDEFINE=128
//------------------------------------------------------------------------------------
//#define MAX_TRACELOG_MSG_LENGTH       256       // Max length of one trace-log message
//#define MAX_FILEPATH_CAPACITY        8192       // Maximum file paths capacity
//#define MAX_FILEPATH_LENGTH          4096       // Maximum length for filepaths (Linux PATH_MAX default value)
//#define MAX_KEYBOARD_KEYS             512       // Maximum number of keyboard keys supported
//#define MAX_MOUSE_BUTTONS               8       // Maximum number of mouse buttons supported
//#define MAX_GAMEPADS                    4       // Maximum number of gamepads supported
//#define MAX_GAMEPAD_AXES                8       // Maximum number of axes supported (per gamepad)
//#define MAX_GAMEPAD_BUTTONS            32       // Maximum number of buttons supported (per gamepad)
//#define MAX_GAMEPAD_VIBRATION_TIME      2.0f    // Maximum vibration time in seconds
//#define MAX_TOUCH_POINTS               10       // Maximum number of touch points supported
//#define MAX_KEY_PRESSED_QUEUE          16       // Maximum number of keys in the key input queue
//#define MAX_CHAR_PRESSED_QUEUE         16       // Maximum number of characters in the char input queue
//#define MAX_DECOMPRESSION_SIZE         64       // Max size allocated for decompression in MB
//#define MAX_AUTOMATION_EVENTS       16384       // Maximum number of automation events to record
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Module: rlgl - Configuration values
//------------------------------------------------------------------------------------
#ifndef RLGL_ENABLE_OPENGL_DEBUG_CONTEXT
    // Request OpenGL debug context (only available on OpenGL 4.3)
    #define RLGL_ENABLE_OPENGL_DEBUG_CONTEXT       0
#endif
#ifndef RLGL_SHOW_GL_DETAILS_INFO
    // Show OpenGL detailed info on initialization,
    // supported GL extensions and GL capabilities
    #define RLGL_SHOW_GL_DETAILS_INFO              0
#endif

// rlgl: Configuration values
// NOTE: Below values are alread defined inside [rlgl.h] so there is no need to be
// redefined here, in case it must be done, just uncomment the required line and update
// the value; it can also be done on compilation with -DVALUE_TO_REDEFINE=128
//------------------------------------------------------------------------------------
//#define RL_DEFAULT_BATCH_BUFFER_ELEMENTS    4096      // Default internal render batch elements limits
//#define RL_DEFAULT_BATCH_BUFFERS               1      // Default number of batch buffers (multi-buffering)
//#define RL_DEFAULT_BATCH_DRAWCALLS           256      // Default number of batch draw calls (by state changes: mode, texture)
//#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS     4      // Maximum number of textures units that can be activated on batch drawing (SetShaderValueTexture())
//#define RL_MAX_MATRIX_STACK_SIZE              32      // Maximum size of internal Matrix stack
//#define RL_MAX_SHADER_LOCATIONS               32      // Maximum number of shader locations supported
//#define RL_CULL_DISTANCE_NEAR                  0.05   // Default projection matrix near cull distance
//#define RL_CULL_DISTANCE_FAR                4000.0    // Default projection matrix far cull distance

// Default shader vertex attribute locations
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION    0
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD    1
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL      2
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR       3
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT     4
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2   5
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES     6
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES 7
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS 8
//#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM 9

// Default shader vertex attribute/uniform names to set location points
// NOTE: When a new shader is loaded, locations are tried to be set for convenience, looking for the names defined here
// In case custom shader names are used, it's up to the user to set locations with GetShaderLocation*() functions
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION       "vertexPosition"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD       "vertexTexCoord"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL         "vertexNormal"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR          "vertexColor"        // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT        "vertexTangent"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2      "vertexTexCoord2"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEINDICES    "vertexBoneIndices"  // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS    "vertexBoneWeights"  // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
//#define RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCETRANSFORM "instanceTransform" // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_INSTANCETRANSFORM
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP           "mvp"                // model-view-projection matrix
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW          "matView"            // view matrix
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION    "matProjection"      // projection matrix
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL         "matModel"           // model matrix
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL        "matNormal"          // normal matrix (transpose(inverse(matModelView))
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR         "colDiffuse"         // color diffuse (tint color, multiplied by texture color)
//#define RL_DEFAULT_SHADER_UNIFORM_NAME_BONEMATRICES  "boneMatrices"       // bone matrices
//#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0    "texture0"           // texture0 (texture slot active 0)
//#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1    "texture1"           // texture1 (texture slot active 1)
//#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2    "texture2"           // texture2 (texture slot active 2)
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Module: rshapes - Configuration Flags
//------------------------------------------------------------------------------------
#ifndef SUPPORT_QUADS_DRAW_MODE
    // Use QUADS instead of TRIANGLES for drawing when possible
    // Some lines-based shapes could still use lines
    #define SUPPORT_QUADS_DRAW_MODE         1
#endif

//------------------------------------------------------------------------------------
// Module: rtextures - Configuration Flags
//------------------------------------------------------------------------------------
// Selected desired fileformats to be supported for image data loading
#ifndef SUPPORT_FILEFORMAT_PNG
    #define SUPPORT_FILEFORMAT_PNG      1
#endif
#ifndef SUPPORT_FILEFORMAT_BMP
    // NOTE: BMP support required for clipboard images on Windows
    #define SUPPORT_FILEFORMAT_BMP      1
#endif
#ifndef SUPPORT_FILEFORMAT_TGA
    #define SUPPORT_FILEFORMAT_TGA      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_JPG
    #define SUPPORT_FILEFORMAT_JPG      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_GIF
    #define SUPPORT_FILEFORMAT_GIF      1
#endif
#ifndef SUPPORT_FILEFORMAT_QOI
    #define SUPPORT_FILEFORMAT_QOI      1
#endif
#ifndef SUPPORT_FILEFORMAT_PSD
    #define SUPPORT_FILEFORMAT_PSD      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_DDS
    #define SUPPORT_FILEFORMAT_DDS      1
#endif
#ifndef SUPPORT_FILEFORMAT_HDR
    #define SUPPORT_FILEFORMAT_HDR      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_PIC
    #define SUPPORT_FILEFORMAT_PIC      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_KTX
    #define SUPPORT_FILEFORMAT_KTX      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_ASTC
    #define SUPPORT_FILEFORMAT_ASTC     0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_PKM
    #define SUPPORT_FILEFORMAT_PKM      0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_PVR
    #define SUPPORT_FILEFORMAT_PVR      0       // Disabled by default
#endif

#ifndef SUPPORT_IMAGE_EXPORT
    // Support image export functionality (.png, .bmp, .tga, .jpg, .qoi)
    // NOTE: Image export requires stb_image_write.h library 
    #define SUPPORT_IMAGE_EXPORT        1
#endif
#ifndef SUPPORT_IMAGE_GENERATION
    // Support procedural image generation functionality: gradient, spot, perlin-noise, cellular...
    // NOTE: Perlin noise requires stb_perlin.h library
    #define SUPPORT_IMAGE_GENERATION    1
#endif

//------------------------------------------------------------------------------------
// Module: rtext - Configuration Flags
//------------------------------------------------------------------------------------
// Selected desired font fileformats to be supported for loading
#ifndef SUPPORT_FILEFORMAT_TTF
    #define SUPPORT_FILEFORMAT_TTF      1
#endif
#ifndef SUPPORT_FILEFORMAT_FNT
    #define SUPPORT_FILEFORMAT_FNT      1
#endif
#ifndef SUPPORT_FILEFORMAT_BDF
    #define SUPPORT_FILEFORMAT_BDF      0       // Disabled by default
#endif

//------------------------------------------------------------------------------------
// Module: rmodels - Configuration Flags
//------------------------------------------------------------------------------------
// Selected desired model fileformats to be supported for loading
#ifndef SUPPORT_FILEFORMAT_OBJ
    #define SUPPORT_FILEFORMAT_OBJ      1
#endif
#ifndef SUPPORT_FILEFORMAT_MTL
    #define SUPPORT_FILEFORMAT_MTL      1
#endif
#ifndef SUPPORT_FILEFORMAT_IQM
    #define SUPPORT_FILEFORMAT_IQM      1
#endif
#ifndef SUPPORT_FILEFORMAT_GLTF
    #define SUPPORT_FILEFORMAT_GLTF     1
#endif
#ifndef SUPPORT_FILEFORMAT_VOX
    #define SUPPORT_FILEFORMAT_VOX      1
#endif
#ifndef SUPPORT_FILEFORMAT_M3D
    #define SUPPORT_FILEFORMAT_M3D      1
#endif
#ifndef SUPPORT_MESH_GENERATION
    // Support procedural mesh generation functions, uses external par_shapes.h library
    // NOTE: Some generated meshes DO NOT include generated texture coordinates
    #define SUPPORT_MESH_GENERATION     1
#endif
#ifndef SUPPORT_GPU_SKINNING
    // GPU skinning disabled by default, some GPUs do not support more than 8 VBOs
    #define SUPPORT_GPU_SKINNING        0    
#endif

//------------------------------------------------------------------------------------
// Module: raudio - Configuration Flags
//------------------------------------------------------------------------------------
// Desired audio fileformats to be supported for loading
#ifndef SUPPORT_FILEFORMAT_WAV
    #define SUPPORT_FILEFORMAT_WAV      1
#endif
#ifndef SUPPORT_FILEFORMAT_OGG
    #define SUPPORT_FILEFORMAT_OGG      1
#endif
#ifndef SUPPORT_FILEFORMAT_MP3
    #define SUPPORT_FILEFORMAT_MP3      1
#endif
#ifndef SUPPORT_FILEFORMAT_QOA
    #define SUPPORT_FILEFORMAT_QOA      1
#endif
#ifndef SUPPORT_FILEFORMAT_FLAC
    #define SUPPORT_FILEFORMAT_FLAC     0       // Disabled by default
#endif
#ifndef SUPPORT_FILEFORMAT_XM
    #define SUPPORT_FILEFORMAT_XM       1
#endif
#ifndef SUPPORT_FILEFORMAT_MOD
    #define SUPPORT_FILEFORMAT_MOD      1
#endif

// raudio: Configuration values
// NOTE: Below values are alread defined inside [rlgl.h] so there is no need to be
// redefined here, in case it must be done, just uncomment the required line and update
// the value; it can also be done on compilation with -DVALUE_TO_REDEFINE=128
//------------------------------------------------------------------------------------
//#define AUDIO_DEVICE_FORMAT     ma_format_f32    // Device output format (miniaudio: float-32bit)
//#define AUDIO_DEVICE_CHANNELS               2    // Device output channels: stereo
//#define AUDIO_DEVICE_SAMPLE_RATE            0    // Device sample rate (device default)
//#define AUDIO_DEVICE_PERIOD_SIZE_IN_FRAMES  0    // Device period size (controls latency, 0 defaults to 10ms)
//#define MAX_AUDIO_BUFFER_POOL_CHANNELS     16    // Maximum number of audio pool channels
//------------------------------------------------------------------------------------

#endif // !EXTERNAL_CONFIG_FLAGS

#endif // CONFIG_H
