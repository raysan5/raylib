/**********************************************************************************************
*
*   raylib configuration flags
*
*   This file defines all the configuration flags for the different raylib modules
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2024 Ahmad Fatoum & Ramon Santamaria (@raysan5)
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
#ifndef SUPPORT_MODULE_RSHAPES
#define SUPPORT_MODULE_RSHAPES          1
#endif

#ifndef SUPPORT_MODULE_RTEXTURES
#define SUPPORT_MODULE_RTEXTURES        1
#endif 

#ifndef SUPPORT_MODULE_RTEXT
#define SUPPORT_MODULE_RTEXT            1       // WARNING: It requires SUPPORT_MODULE_RTEXTURES to load sprite font textures
#endif

#ifndef SUPPORT_MODULE_RMODELS
#define SUPPORT_MODULE_RMODELS          1
#endif

#ifndef SUPPORT_MODULE_RAUDIO
#define SUPPORT_MODULE_RAUDIO           1
#endif

//------------------------------------------------------------------------------------
// Module: rcore - Configuration Flags
//------------------------------------------------------------------------------------
// Camera module is included (rcamera.h) and multiple predefined cameras are available: free, 1st/3rd person, orbital
#ifndef SUPPORT_CAMERA_SYSTEM
#define SUPPORT_CAMERA_SYSTEM           1
#endif

// Gestures module is included (rgestures.h) to support gestures detection: tap, hold, swipe, drag
#ifndef SUPPORT_GESTURES_SYSTEM
#define SUPPORT_GESTURES_SYSTEM         1
#endif

// Include pseudo-random numbers generator (rprand.h), based on Xoshiro128** and SplitMix64
#ifndef SUPPORT_RPRAND_GENERATOR
#define SUPPORT_RPRAND_GENERATOR        1
#endif

// Mouse gestures are directly mapped like touches and processed by gestures system
#ifndef SUPPORT_MOUSE_GESTURES
#define SUPPORT_MOUSE_GESTURES          1
#endif

// Reconfigure standard input to receive key inputs, works with SSH connection.
#ifndef SUPPORT_SSH_KEYBOARD_RPI
#define SUPPORT_SSH_KEYBOARD_RPI        1
#endif 

// Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
// However, it can also reduce overall system performance, because the thread scheduler switches tasks more often.
#ifndef SUPPORT_WINMM_HIGHRES_TIMER
#define SUPPORT_WINMM_HIGHRES_TIMER     1
#endif

// Use busy wait loop for timing sync, if 0, a high-resolution timer is set up and used (Default off)
#ifndef SUPPORT_BUSY_WAIT_LOOP
#define SUPPORT_BUSY_WAIT_LOOP          0
#endif

// Use a partial-busy wait loop, in this case frame sleeps for most of the time, but then runs a busy loop at the end for accuracy
#ifndef SUPPORT_PARTIALBUSY_WAIT_LOOP
#define SUPPORT_PARTIALBUSY_WAIT_LOOP    1
#endif

// Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
#ifndef SUPPORT_SCREEN_CAPTURE
#define SUPPORT_SCREEN_CAPTURE          1
#endif 

// Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()
#ifndef SUPPORT_GIF_RECORDING
#define SUPPORT_GIF_RECORDING           1
#endif

// Support CompressData() and DecompressData() functions
#ifndef SUPPORT_COMPRESSION_API
#define SUPPORT_COMPRESSION_API         1
#endif

// Support automatic generated events, loading and recording of those events when required
#ifndef SUPPORT_AUTOMATION_EVENTS
#define SUPPORT_AUTOMATION_EVENTS       1
#endif

// Support custom frame control, only for advanced users (Default off)
// By default EndDrawing() does this job: draws everything + SwapScreenBuffer() + manage frame timing + PollInputEvents()
// Enabling this flag allows manual control of the frame processes, use at your own risk
#ifndef SUPPORT_CUSTOM_FRAME_CONTROL
#define SUPPORT_CUSTOM_FRAME_CONTROL    0
#endif

// Support for clipboard image loading
// NOTE: Only working on SDL3, GLFW (Windows) and RGFW (Windows)
#ifndef SUPPORT_CLIPBOARD_IMAGE
#define SUPPORT_CLIPBOARD_IMAGE    1
#endif

// NOTE: Clipboard image loading requires support for some image file formats
// TODO: Those defines should probably be removed from here, I prefer to let the user manage them

#if SUPPORT_CLIPBOARD_IMAGE
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
#ifndef MAX_FILEPATH_CAPACITY
#define MAX_FILEPATH_CAPACITY        8192       // Maximum file paths capacity
#endif

#ifndef MAX_FILEPATH_LENGTH
#define MAX_FILEPATH_LENGTH          4096       // Maximum length for filepaths (Linux PATH_MAX default value)
#endif

#ifndef MAX_KEYBOARD_KEYS
#define MAX_KEYBOARD_KEYS             512       // Maximum number of keyboard keys supported
#endif 

#ifndef MAX_MOUSE_BUTTONS
#define MAX_MOUSE_BUTTONS               8       // Maximum number of mouse buttons supported
#endif

#ifndef MAX_GAMEPADS
#define MAX_GAMEPADS                    4       // Maximum number of gamepads supported
#endif

#ifndef MAX_GAMEPAD_AXIS
#define MAX_GAMEPAD_AXIS                8       // Maximum number of axis supported (per gamepad)
#endif

#ifndef MAX_GAMEPAD_BUTTONS
#define MAX_GAMEPAD_BUTTONS            32       // Maximum number of buttons supported (per gamepad)
#endif

#ifndef MAX_GAMEPAD_VIBRATION_TIME
#define MAX_GAMEPAD_VIBRATION_TIME      2.0f    // Maximum vibration time in seconds
#endif

#ifndef MAX_TOUCH_POINTS
#define MAX_TOUCH_POINTS                8       // Maximum number of touch points supported
#endif

#ifndef MAX_KEY_PRESSED_QUEUE
#define MAX_KEY_PRESSED_QUEUE          16       // Maximum number of keys in the key input queue
#endif

#ifndef MAX_CHAR_PRESSED_QUEUE
#define MAX_CHAR_PRESSED_QUEUE         16       // Maximum number of characters in the char input queue
#endif

#ifndef MAX_DECOMPRESSION_SIZE
#define MAX_DECOMPRESSION_SIZE         64       // Max size allocated for decompression in MB
#endif

#ifndef MAX_AUTOMATION_EVENTS
#define MAX_AUTOMATION_EVENTS       16384       // Maximum number of automation events to record
#endif

//------------------------------------------------------------------------------------
// Module: rlgl - Configuration values
//------------------------------------------------------------------------------------

// Enable OpenGL Debug Context (only available on OpenGL 4.3)
//#define RLGL_ENABLE_OPENGL_DEBUG_CONTEXT       1

// Show OpenGL extensions and capabilities detailed logs on init
//#define RLGL_SHOW_GL_DETAILS_INFO              1

#ifndef RL_SUPPORT_MESH_GPU_SKINNING
#define RL_SUPPORT_MESH_GPU_SKINNING           1      // GPU skinning, comment if your GPU does not support more than 8 VBOs
#endif

#ifndef RL_DEFAULT_BATCH_BUFFERS
#define RL_DEFAULT_BATCH_BUFFERS               1      // Default number of batch buffers (multi-buffering)
#endif

#ifndef RL_DEFAULT_BATCH_DRAWCALLS
#define RL_DEFAULT_BATCH_DRAWCALLS           256      // Default number of batch draw calls (by state changes: mode, texture)
#endif

#ifndef RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS
#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS     4      // Maximum number of textures units that can be activated on batch drawing (SetShaderValueTexture())
#endif

#ifndef RL_MAX_MATRIX_STACK_SIZE
#define RL_MAX_MATRIX_STACK_SIZE              32      // Maximum size of internal Matrix stack
#endif

#ifndef RL_MAX_SHADER_LOCATIONS
#define RL_MAX_SHADER_LOCATIONS               32      // Maximum number of shader locations supported
#endif

#ifndef RL_CULL_DISTANCE_NEAR
#define RL_CULL_DISTANCE_NEAR               0.01      // Default projection matrix near cull distance
#endif

#ifndef RL_CULL_DISTANCE_FAR
#define RL_CULL_DISTANCE_FAR              1000.0      // Default projection matrix far cull distance
#endif

// Default shader vertex attribute locations
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION    0
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD    1
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL      2
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR       3
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT     4
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2   5
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES     6
#endif

#if RL_SUPPORT_MESH_GPU_SKINNING
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS     7
#endif

#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS 8
#endif
#endif

// Default shader vertex attribute names to set location points
// NOTE: When a new shader is loaded, the following locations are tried to be set for convenience
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_MVP
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#endif

#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#endif

#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#endif

#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)
#endif

//------------------------------------------------------------------------------------
// Module: rshapes - Configuration Flags
//------------------------------------------------------------------------------------
// Use QUADS instead of TRIANGLES for drawing when possible
// Some lines-based shapes could still use lines
#ifndef SUPPORT_QUADS_DRAW_MODE
#define SUPPORT_QUADS_DRAW_MODE         1
#endif

// rshapes: Configuration values
//------------------------------------------------------------------------------------
#ifndef SPLINE_SEGMENT_DIVISIONS
#define SPLINE_SEGMENT_DIVISIONS       24       // Spline segments subdivisions
#endif

//------------------------------------------------------------------------------------
// Module: rtextures - Configuration Flags
//------------------------------------------------------------------------------------
// Select desired fileformats to be supported for image data loading
#ifndef SUPPORT_FILEFORMAT_PNG
#define SUPPORT_FILEFORMAT_PNG      1
#endif

#ifndef SUPPORT_FILEFORMAT_BMP
#define SUPPORT_FILEFORMAT_BMP      0
#endif

#ifndef SUPPORT_FILEFORMAT_TGA
#define SUPPORT_FILEFORMAT_TGA      0
#endif

#ifndef SUPPORT_FILEFORMAT_JPG
#define SUPPORT_FILEFORMAT_JPG      0
#endif

#ifndef SUPPORT_FILEFORMAT_GIF
#define SUPPORT_FILEFORMAT_GIF      1
#endif

#ifndef SUPPORT_FILEFORMAT_QOI
#define SUPPORT_FILEFORMAT_QOI      1
#endif

#ifndef SUPPORT_FILEFORMAT_PSD
#define SUPPORT_FILEFORMAT_PSD      0
#endif

#ifndef SUPPORT_FILEFORMAT_DDS
#define SUPPORT_FILEFORMAT_DDS      1
#endif

#ifndef SUPPORT_FILEFORMAT_HDR
#define SUPPORT_FILEFORMAT_HDR      0
#endif

#ifndef SUPPORT_FILEFORMAT_PIC
#define SUPPORT_FILEFORMAT_PIC      0
#endif

#ifndef SUPPORT_FILEFORMAT_KTX
#define SUPPORT_FILEFORMAT_KTX      0
#endif

#ifndef SUPPORT_FILEFORMAT_ASTC
#define SUPPORT_FILEFORMAT_ASTC     0
#endif

#ifndef SUPPORT_FILEFORMAT_PNM
#define SUPPORT_FILEFORMAT_PNM      0
#endif

#ifndef SUPPORT_FILEFORMAT_PVR
#define SUPPORT_FILEFORMAT_PVR      0
#endif

// Support image export functionality (.png, .bmp, .tga, .jpg, .qoi)
#ifndef SUPPORT_IMAGE_EXPORT
#define SUPPORT_IMAGE_EXPORT            1
#endif

// Support procedural image generation functionality (gradient, spot, perlin-noise, cellular)
#ifndef SUPPORT_IMAGE_GENERATION
#define SUPPORT_IMAGE_GENERATION        1
#endif

// Support multiple image editing functions to scale, adjust colors, flip, draw on images, crop...
// If not defined, still some functions are supported: ImageFormat(), ImageCrop(), ImageToPOT()
#ifndef SUPPORT_IMAGE_MANIPULATION
#define SUPPORT_IMAGE_MANIPULATION      1
#endif

//------------------------------------------------------------------------------------
// Module: rtext - Configuration Flags
//------------------------------------------------------------------------------------
// Default font is loaded on window initialization to be available for the user to render simple text
// NOTE: If enabled, uses external module functions to load default raylib font
#ifndef SUPPORT_DEFAULT_FONT
#define SUPPORT_DEFAULT_FONT            1
#endif

// Selected desired font fileformats to be supported for loading
#ifndef SUPPORT_FILEFORMAT_TTF
#define SUPPORT_FILEFORMAT_TTF          1
#endif

#ifndef SUPPORT_FILEFORMAT_FNT
#define SUPPORT_FILEFORMAT_FNT          1
#endif

#ifndef SUPPORT_FILEFORMAT_BDF
#define SUPPORT_FILEFORMAT_BDF          0
#endif

// Support text management functions
// If not defined, still some functions are supported: TextLength(), TextFormat()
#ifndef SUPPORT_TEXT_MANIPULATION
#define SUPPORT_TEXT_MANIPULATION       1
#endif

// On font atlas image generation [GenImageFontAtlas()], add a 3x3 pixels white rectangle
// at the bottom-right corner of the atlas. It can be useful to for shapes drawing, to allow
// drawing text and shapes with a single draw call [SetShapesTexture()].
#ifndef SUPPORT_FONT_ATLAS_WHITE_REC
#define SUPPORT_FONT_ATLAS_WHITE_REC    1
#endif

// rtext: Configuration values
//------------------------------------------------------------------------------------
// Size of internal static buffers used on some functions:
// TextFormat(), TextSubtext(), TextToUpper(), TextToLower(), TextToPascal(), TextSplit()
#ifndef MAX_TEXT_BUFFER_LENGTH
#define MAX_TEXT_BUFFER_LENGTH       1024
#endif

// Maximum number of substrings to split: TextSplit()                              
#ifndef MAX_TEXTSPLIT_COUNT
#define MAX_TEXTSPLIT_COUNT           128       
#endif

//------------------------------------------------------------------------------------
// Module: rmodels - Configuration Flags
//------------------------------------------------------------------------------------
// Selected desired model fileformats to be supported for loading
#ifndef SUPPORT_FILEFORMAT_OBJ
#define SUPPORT_FILEFORMAT_OBJ          1       
#endif

#ifndef SUPPORT_FILEFORMAT_MTL
#define SUPPORT_FILEFORMAT_MTL          1       
#endif

#ifndef SUPPORT_FILEFORMAT_IQM
#define SUPPORT_FILEFORMAT_IQM          1       
#endif

#ifndef SUPPORT_FILEFORMAT_GLTF
#define SUPPORT_FILEFORMAT_GLTF         1       
#endif

#ifndef SUPPORT_FILEFORMAT_VOX
#define SUPPORT_FILEFORMAT_VOX          1       
#endif

#ifndef SUPPORT_FILEFORMAT_M3D
#define SUPPORT_FILEFORMAT_M3D          1       
#endif

// Support procedural mesh generation functions, uses external par_shapes.h library
// NOTE: Some generated meshes DO NOT include generated texture coordinates
#ifndef SUPPORT_MESH_GENERATION
#define SUPPORT_MESH_GENERATION         1       
#endif

// rmodels: Configuration values
//------------------------------------------------------------------------------------
#ifndef MAX_MATERIAL_MAPS
#define MAX_MATERIAL_MAPS              12       // Maximum number of shader maps supported       
#endif

#ifndef MAX_MESH_VERTEX_BUFFERS
#if RL_SUPPORT_MESH_GPU_SKINNING
#define MAX_MESH_VERTEX_BUFFERS         9       // Maximum vertex buffers (VBO) per mesh
#else
#define MAX_MESH_VERTEX_BUFFERS         7       // Maximum vertex buffers (VBO) per mesh
#endif
#endif

//------------------------------------------------------------------------------------
// Module: raudio - Configuration Flags
//------------------------------------------------------------------------------------
// Desired audio fileformats to be supported for loading
#ifndef SUPPORT_FILEFORMAT_WAV
#define SUPPORT_FILEFORMAT_WAV          1       
#endif

#ifndef SUPPORT_FILEFORMAT_OGG
#define SUPPORT_FILEFORMAT_OGG          1       
#endif

#ifndef SUPPORT_FILEFORMAT_MP3
#define SUPPORT_FILEFORMAT_MP3          1       
#endif

#ifndef SUPPORT_FILEFORMAT_QOA
#define SUPPORT_FILEFORMAT_QOA          1       
#endif

#ifndef SUPPORT_FILEFORMAT_FLAC
#define SUPPORT_FILEFORMAT_FLAC         0       
#endif

#ifndef SUPPORT_FILEFORMAT_XM
#define SUPPORT_FILEFORMAT_XM           1       
#endif

#ifndef SUPPORT_FILEFORMAT_MOD
#define SUPPORT_FILEFORMAT_MOD          1       
#endif

// raudio: Configuration values
//------------------------------------------------------------------------------------
#ifndef AUDIO_DEVICE_FORMAT
#define AUDIO_DEVICE_FORMAT    ma_format_f32    // Device output format (miniaudio: float-32bit)       
#endif

#ifndef AUDIO_DEVICE_CHANNELS
#define AUDIO_DEVICE_CHANNELS              2    // Device output channels: stereo       
#endif

#ifndef AUDIO_DEVICE_SAMPLE_RATE
#define AUDIO_DEVICE_SAMPLE_RATE           0    // Device sample rate (device default)       
#endif

#ifndef MAX_AUDIO_BUFFER_POOL_CHANNELS
#define MAX_AUDIO_BUFFER_POOL_CHANNELS    16    // Maximum number of audio pool channels       
#endif

//------------------------------------------------------------------------------------
// Module: utils - Configuration Flags
//------------------------------------------------------------------------------------
// Standard file io library (stdio.h) included
#ifndef SUPPORT_STANDARD_FILEIO
#define SUPPORT_STANDARD_FILEIO         1    
#endif

// Show TRACELOG() output messages
// NOTE: By default LOG_DEBUG traces not shown
#ifndef SUPPORT_TRACELOG
#define SUPPORT_TRACELOG                1    
#endif

#ifndef SUPPORT_TRACELOG_DEBUG
#define SUPPORT_TRACELOG_DEBUG          0    
#endif


// utils: Configuration values
//------------------------------------------------------------------------------------
#ifndef MAX_TRACELOG_MSG_LENGTH
#define MAX_TRACELOG_MSG_LENGTH       256       // Max length of one trace-log message    
#endif


#endif // CONFIG_H
