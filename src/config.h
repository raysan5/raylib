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

//------------------------------------------------------------------------------------
// Module selection - Some modules could be avoided
// Mandatory modules: rcore, rlgl, utils
//------------------------------------------------------------------------------------
#define SUPPORT_MODULE_RSHAPES          1
#define SUPPORT_MODULE_RTEXTURES        1
#define SUPPORT_MODULE_RTEXT            1       // WARNING: It requires SUPPORT_MODULE_RTEXTURES to load sprite font textures
#define SUPPORT_MODULE_RMODELS          1
#define SUPPORT_MODULE_RAUDIO           1

//------------------------------------------------------------------------------------
// Module: rcore - Configuration Flags
//------------------------------------------------------------------------------------
// Camera module is included (rcamera.h) and multiple predefined cameras are available: free, 1st/3rd person, orbital
#define SUPPORT_CAMERA_SYSTEM           1
// Gestures module is included (rgestures.h) to support gestures detection: tap, hold, swipe, drag
#define SUPPORT_GESTURES_SYSTEM         1
// Include pseudo-random numbers generator (rprand.h), based on Xoshiro128** and SplitMix64
#define SUPPORT_RPRAND_GENERATOR        1
// Mouse gestures are directly mapped like touches and processed by gestures system
#define SUPPORT_MOUSE_GESTURES          1
// Reconfigure standard input to receive key inputs, works with SSH connection.
#define SUPPORT_SSH_KEYBOARD_RPI        1
// Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
// However, it can also reduce overall system performance, because the thread scheduler switches tasks more often.
#define SUPPORT_WINMM_HIGHRES_TIMER     1
// Use busy wait loop for timing sync, if not defined, a high-resolution timer is set up and used
//#define SUPPORT_BUSY_WAIT_LOOP          1
// Use a partial-busy wait loop, in this case frame sleeps for most of the time, but then runs a busy loop at the end for accuracy
#define SUPPORT_PARTIALBUSY_WAIT_LOOP    1
// Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
#define SUPPORT_SCREEN_CAPTURE          1
// Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()
#define SUPPORT_GIF_RECORDING           1
// Support CompressData() and DecompressData() functions
#define SUPPORT_COMPRESSION_API         1
// Support automatic generated events, loading and recording of those events when required
#define SUPPORT_AUTOMATION_EVENTS       1
// Support custom frame control, only for advanced users
// By default EndDrawing() does this job: draws everything + SwapScreenBuffer() + manage frame timing + PollInputEvents()
// Enabling this flag allows manual control of the frame processes, use at your own risk
//#define SUPPORT_CUSTOM_FRAME_CONTROL    1

// Support for clipboard image loading
// NOTE: Only working on SDL3, GLFW (Windows) and RGFW (Windows)
#define SUPPORT_CLIPBOARD_IMAGE    1

// NOTE: Clipboard image loading requires support for some image file formats
// TODO: Those defines should probably be removed from here, I prefer to let the user manage them
#if defined(SUPPORT_CLIPBOARD_IMAGE)
    #ifndef SUPPORT_MODULE_RTEXTURES
        #define SUPPORT_MODULE_RTEXTURES 1
    #endif
    #ifndef STBI_REQUIRED
        #define STBI_REQUIRED
    #endif
    #ifndef SUPPORT_FILEFORMAT_BMP // For clipboard image on Windows
        #define SUPPORT_FILEFORMAT_BMP 1
    #endif
    #ifndef SUPPORT_FILEFORMAT_PNG // Wayland uses png for prints, at least it was on 22 LTS ubuntu
        #define SUPPORT_FILEFORMAT_PNG 1
    #endif
    #ifndef SUPPORT_FILEFORMAT_JPG
        #define SUPPORT_FILEFORMAT_JPG 1
    #endif
#endif


// rcore: Configuration values
//------------------------------------------------------------------------------------
#define MAX_FILEPATH_CAPACITY        8192       // Maximum file paths capacity
#define MAX_FILEPATH_LENGTH          4096       // Maximum length for filepaths (Linux PATH_MAX default value)

#define MAX_KEYBOARD_KEYS             512       // Maximum number of keyboard keys supported
#define MAX_MOUSE_BUTTONS               8       // Maximum number of mouse buttons supported
#define MAX_GAMEPADS                    4       // Maximum number of gamepads supported
#define MAX_GAMEPAD_AXIS                8       // Maximum number of axis supported (per gamepad)
#define MAX_GAMEPAD_BUTTONS            32       // Maximum number of buttons supported (per gamepad)
#define MAX_GAMEPAD_VIBRATION_TIME      2.0f    // Maximum vibration time in seconds
#define MAX_TOUCH_POINTS                8       // Maximum number of touch points supported
#define MAX_KEY_PRESSED_QUEUE          16       // Maximum number of keys in the key input queue
#define MAX_CHAR_PRESSED_QUEUE         16       // Maximum number of characters in the char input queue

#define MAX_DECOMPRESSION_SIZE         64       // Max size allocated for decompression in MB

#define MAX_AUTOMATION_EVENTS       16384       // Maximum number of automation events to record

//------------------------------------------------------------------------------------
// Module: rlgl - Configuration values
//------------------------------------------------------------------------------------

// Enable OpenGL Debug Context (only available on OpenGL 4.3)
//#define RLGL_ENABLE_OPENGL_DEBUG_CONTEXT       1

// Show OpenGL extensions and capabilities detailed logs on init
//#define RLGL_SHOW_GL_DETAILS_INFO              1

#define RL_SUPPORT_MESH_GPU_SKINNING           1      // GPU skinning, comment if your GPU does not support more than 8 VBOs

//#define RL_DEFAULT_BATCH_BUFFER_ELEMENTS    4096    // Default internal render batch elements limits
#define RL_DEFAULT_BATCH_BUFFERS               1      // Default number of batch buffers (multi-buffering)
#define RL_DEFAULT_BATCH_DRAWCALLS           256      // Default number of batch draw calls (by state changes: mode, texture)
#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS     4      // Maximum number of textures units that can be activated on batch drawing (SetShaderValueTexture())

#define RL_MAX_MATRIX_STACK_SIZE              32      // Maximum size of internal Matrix stack

#define RL_MAX_SHADER_LOCATIONS               32      // Maximum number of shader locations supported

#define RL_CULL_DISTANCE_NEAR              0.001      // Default projection matrix near cull distance
#define RL_CULL_DISTANCE_FAR             10000.0      // Default projection matrix far cull distance

// Default shader vertex attribute locations
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


// Default shader vertex attribute names to set location points
// NOTE: When a new shader is loaded, the following locations are tried to be set for convenience
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)


//------------------------------------------------------------------------------------
// Module: rshapes - Configuration Flags
//------------------------------------------------------------------------------------
// Use QUADS instead of TRIANGLES for drawing when possible
// Some lines-based shapes could still use lines
#define SUPPORT_QUADS_DRAW_MODE         1

// rshapes: Configuration values
//------------------------------------------------------------------------------------
#define SPLINE_SEGMENT_DIVISIONS       24       // Spline segments subdivisions


//------------------------------------------------------------------------------------
// Module: rtextures - Configuration Flags
//------------------------------------------------------------------------------------
// Selecte desired fileformats to be supported for image data loading
#define SUPPORT_FILEFORMAT_PNG      1
//#define SUPPORT_FILEFORMAT_BMP      1
//#define SUPPORT_FILEFORMAT_TGA      1
//#define SUPPORT_FILEFORMAT_JPG      1
#define SUPPORT_FILEFORMAT_GIF      1
#define SUPPORT_FILEFORMAT_QOI      1
//#define SUPPORT_FILEFORMAT_PSD      1
#define SUPPORT_FILEFORMAT_DDS      1
//#define SUPPORT_FILEFORMAT_HDR      1
//#define SUPPORT_FILEFORMAT_PIC          1
//#define SUPPORT_FILEFORMAT_KTX      1
//#define SUPPORT_FILEFORMAT_ASTC     1
//#define SUPPORT_FILEFORMAT_PKM      1
//#define SUPPORT_FILEFORMAT_PVR      1

// Support image export functionality (.png, .bmp, .tga, .jpg, .qoi)
#define SUPPORT_IMAGE_EXPORT            1
// Support procedural image generation functionality (gradient, spot, perlin-noise, cellular)
#define SUPPORT_IMAGE_GENERATION        1
// Support multiple image editing functions to scale, adjust colors, flip, draw on images, crop...
// If not defined, still some functions are supported: ImageFormat(), ImageCrop(), ImageToPOT()
#define SUPPORT_IMAGE_MANIPULATION      1


//------------------------------------------------------------------------------------
// Module: rtext - Configuration Flags
//------------------------------------------------------------------------------------
// Default font is loaded on window initialization to be available for the user to render simple text
// NOTE: If enabled, uses external module functions to load default raylib font
#define SUPPORT_DEFAULT_FONT            1
// Selected desired font fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_TTF          1
#define SUPPORT_FILEFORMAT_FNT          1
//#define SUPPORT_FILEFORMAT_BDF          1

// Support text management functions
// If not defined, still some functions are supported: TextLength(), TextFormat()
#define SUPPORT_TEXT_MANIPULATION       1

// On font atlas image generation [GenImageFontAtlas()], add a 3x3 pixels white rectangle
// at the bottom-right corner of the atlas. It can be useful to for shapes drawing, to allow
// drawing text and shapes with a single draw call [SetShapesTexture()].
#define SUPPORT_FONT_ATLAS_WHITE_REC    1

// rtext: Configuration values
//------------------------------------------------------------------------------------
#define MAX_TEXT_BUFFER_LENGTH       1024       // Size of internal static buffers used on some functions:
                                                // TextFormat(), TextSubtext(), TextToUpper(), TextToLower(), TextToPascal(), TextSplit()
#define MAX_TEXTSPLIT_COUNT           128       // Maximum number of substrings to split: TextSplit()


//------------------------------------------------------------------------------------
// Module: rmodels - Configuration Flags
//------------------------------------------------------------------------------------
// Selected desired model fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_OBJ          1
#define SUPPORT_FILEFORMAT_MTL          1
#define SUPPORT_FILEFORMAT_IQM          1
#define SUPPORT_FILEFORMAT_GLTF         1
#define SUPPORT_FILEFORMAT_VOX          1
#define SUPPORT_FILEFORMAT_M3D          1
// Support procedural mesh generation functions, uses external par_shapes.h library
// NOTE: Some generated meshes DO NOT include generated texture coordinates
#define SUPPORT_MESH_GENERATION         1

// rmodels: Configuration values
//------------------------------------------------------------------------------------
#define MAX_MATERIAL_MAPS              12       // Maximum number of shader maps supported

#ifdef RL_SUPPORT_MESH_GPU_SKINNING
#define MAX_MESH_VERTEX_BUFFERS         9       // Maximum vertex buffers (VBO) per mesh
#else
#define MAX_MESH_VERTEX_BUFFERS         7       // Maximum vertex buffers (VBO) per mesh
#endif

//------------------------------------------------------------------------------------
// Module: raudio - Configuration Flags
//------------------------------------------------------------------------------------
// Desired audio fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_WAV          1
#define SUPPORT_FILEFORMAT_OGG          1
#define SUPPORT_FILEFORMAT_MP3          1
#define SUPPORT_FILEFORMAT_QOA          1
//#define SUPPORT_FILEFORMAT_FLAC         1
#define SUPPORT_FILEFORMAT_XM           1
#define SUPPORT_FILEFORMAT_MOD          1

// raudio: Configuration values
//------------------------------------------------------------------------------------
#define AUDIO_DEVICE_FORMAT    ma_format_f32    // Device output format (miniaudio: float-32bit)
#define AUDIO_DEVICE_CHANNELS              2    // Device output channels: stereo
#define AUDIO_DEVICE_SAMPLE_RATE           0    // Device sample rate (device default)

#define MAX_AUDIO_BUFFER_POOL_CHANNELS    16    // Maximum number of audio pool channels

//------------------------------------------------------------------------------------
// Module: utils - Configuration Flags
//------------------------------------------------------------------------------------
// Standard file io library (stdio.h) included
#define SUPPORT_STANDARD_FILEIO         1
// Show TRACELOG() output messages
// NOTE: By default LOG_DEBUG traces not shown
#define SUPPORT_TRACELOG                1
//#define SUPPORT_TRACELOG_DEBUG          1

// utils: Configuration values
//------------------------------------------------------------------------------------
#define MAX_TRACELOG_MSG_LENGTH       256       // Max length of one trace-log message

#endif // CONFIG_H
