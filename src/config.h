/**********************************************************************************************
*
*   raylib configuration flags
*
*   This file defines all the configuration flags for the different raylib modules
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018-2021 Ahmad Fatoum & Ramon Santamaria (@raysan5)
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

//------------------------------------------------------------------------------------
// Module: core - Configuration Flags
//------------------------------------------------------------------------------------
// Camera module is included (camera.h) and multiple predefined cameras are available: free, 1st/3rd person, orbital
#define SUPPORT_CAMERA_SYSTEM       1
// Gestures module is included (gestures.h) to support gestures detection: tap, hold, swipe, drag
#define SUPPORT_GESTURES_SYSTEM     1
// Mouse gestures are directly mapped like touches and processed by gestures system
#define SUPPORT_MOUSE_GESTURES      1
// Reconfigure standard input to receive key inputs, works with SSH connection.
#define SUPPORT_SSH_KEYBOARD_RPI    1
// Draw a mouse pointer on screen
//#define SUPPORT_MOUSE_CURSOR_POINT   1
// Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
// However, it can also reduce overall system performance, because the thread scheduler switches tasks more often.
#define SUPPORT_WINMM_HIGHRES_TIMER 1
// Use busy wait loop for timing sync, if not defined, a high-resolution timer is setup and used
//#define SUPPORT_BUSY_WAIT_LOOP      1
// Use a partial-busy wait loop, in this case frame sleeps for most of the time, but then runs a busy loop at the end for accuracy
#define SUPPORT_PARTIALBUSY_WAIT_LOOP
// Wait for events passively (sleeping while no events) instead of polling them actively every frame
//#define SUPPORT_EVENTS_WAITING      1
// Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
#define SUPPORT_SCREEN_CAPTURE      1
// Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()
#define SUPPORT_GIF_RECORDING       1
// Support CompressData() and DecompressData() functions
#define SUPPORT_COMPRESSION_API     1
// Support saving binary data automatically to a generated storage.data file. This file is managed internally.
#define SUPPORT_DATA_STORAGE        1
// Support automatic generated events, loading and recording of those events when required
//#define SUPPORT_EVENTS_AUTOMATION     1
// Support custom frame control, only for advance users
// By default EndDrawing() does this job: draws everything + SwapScreenBuffer() + manage frame timming + PollInputEvents()
// Enabling this flag allows manual control of the frame processes, use at your own risk
//#define SUPPORT_CUSTOM_FRAME_CONTROL   1

// core: Configuration values
//------------------------------------------------------------------------------------
#if defined(__linux__)
    #define MAX_FILEPATH_LENGTH     4096        // Maximum length for filepaths (Linux PATH_MAX default value)
#else
    #define MAX_FILEPATH_LENGTH      512        // Maximum length supported for filepaths
#endif

#define MAX_GAMEPADS                   4        // Max number of gamepads supported
#define MAX_GAMEPAD_AXIS               8        // Max number of axis supported (per gamepad)
#define MAX_GAMEPAD_BUTTONS           32        // Max bumber of buttons supported (per gamepad)
#define MAX_TOUCH_POINTS              10        // Maximum number of touch points supported
#define MAX_KEY_PRESSED_QUEUE         16        // Max number of characters in the key input queue

#define STORAGE_DATA_FILE  "storage.data"       // Automatic storage filename

#define MAX_DECOMPRESSION_SIZE        64        // Max size allocated for decompression in MB


//------------------------------------------------------------------------------------
// Module: rlgl - Configuration values
//------------------------------------------------------------------------------------
// Show OpenGL extensions and capabilities detailed logs on init
//#define SUPPORT_GL_DETAILS_INFO        1

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    #define DEFAULT_BATCH_BUFFER_ELEMENTS   8192    // Default internal render batch limits
#elif defined(GRAPHICS_API_OPENGL_ES2)
    #define DEFAULT_BATCH_BUFFER_ELEMENTS   2048    // Default internal render batch limits
#endif

#define DEFAULT_BATCH_BUFFERS            1      // Default number of batch buffers (multi-buffering)
#define DEFAULT_BATCH_DRAWCALLS        256      // Default number of batch draw calls (by state changes: mode, texture)

#define MAX_MATRIX_STACK_SIZE           32      // Maximum size of internal Matrix stack
#define MAX_MESH_VERTEX_BUFFERS          7      // Maximum vertex buffers (VBO) per mesh
#define MAX_SHADER_LOCATIONS            32      // Maximum number of shader locations supported
#define MAX_MATERIAL_MAPS               12      // Maximum number of shader maps supported

#define RL_CULL_DISTANCE_NEAR         0.01      // Default projection matrix near cull distance
#define RL_CULL_DISTANCE_FAR        1000.0      // Default projection matrix far cull distance

// Default shader vertex attribute names to set location points
// NOTE: When a new shader is loaded, the following locations are tried to be set for convenience
#define DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Binded by default to shader location: 0
#define DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Binded by default to shader location: 1
#define DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Binded by default to shader location: 2
#define DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Binded by default to shader location: 3
#define DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Binded by default to shader location: 4
#define DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Binded by default to shader location: 5

#define DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#define DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#define DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#define DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#define DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#define DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#define DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)


//------------------------------------------------------------------------------------
// Module: shapes - Configuration Flags
//------------------------------------------------------------------------------------
// Use QUADS instead of TRIANGLES for drawing when possible
// Some lines-based shapes could still use lines
#define SUPPORT_QUADS_DRAW_MODE     1


//------------------------------------------------------------------------------------
// Module: textures - Configuration Flags
//------------------------------------------------------------------------------------
// Selecte desired fileformats to be supported for image data loading
#define SUPPORT_FILEFORMAT_PNG      1
//#define SUPPORT_FILEFORMAT_BMP      1
//#define SUPPORT_FILEFORMAT_TGA      1
//#define SUPPORT_FILEFORMAT_JPG      1
#define SUPPORT_FILEFORMAT_GIF      1
//#define SUPPORT_FILEFORMAT_PSD      1
#define SUPPORT_FILEFORMAT_DDS      1
#define SUPPORT_FILEFORMAT_HDR      1
//#define SUPPORT_FILEFORMAT_KTX      1
//#define SUPPORT_FILEFORMAT_ASTC     1
//#define SUPPORT_FILEFORMAT_PKM      1
//#define SUPPORT_FILEFORMAT_PVR      1

// Support image export functionality (.png, .bmp, .tga, .jpg)
#define SUPPORT_IMAGE_EXPORT        1
// Support procedural image generation functionality (gradient, spot, perlin-noise, cellular)
#define SUPPORT_IMAGE_GENERATION    1
// Support multiple image editing functions to scale, adjust colors, flip, draw on images, crop...
// If not defined, still some functions are supported: ImageFormat(), ImageCrop(), ImageToPOT()
#define SUPPORT_IMAGE_MANIPULATION  1


//------------------------------------------------------------------------------------
// Module: text - Configuration Flags
//------------------------------------------------------------------------------------
// Default font is loaded on window initialization to be available for the user to render simple text
// NOTE: If enabled, uses external module functions to load default raylib font
#define SUPPORT_DEFAULT_FONT        1
// Selected desired font fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_FNT      1
#define SUPPORT_FILEFORMAT_TTF      1

// Support text management functions
// If not defined, still some functions are supported: TextLength(), TextFormat()
#define SUPPORT_TEXT_MANIPULATION   1

// text: Configuration values
//------------------------------------------------------------------------------------
#define MAX_TEXT_BUFFER_LENGTH      1024        // Size of internal static buffers used on some functions:
                                                // TextFormat(), TextSubtext(), TextToUpper(), TextToLower(), TextToPascal(), TextSplit()
#define MAX_TEXTSPLIT_COUNT          128        // Maximum number of substrings to split: TextSplit()


//------------------------------------------------------------------------------------
// Module: models - Configuration Flags
//------------------------------------------------------------------------------------
// Selected desired model fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_OBJ      1
#define SUPPORT_FILEFORMAT_MTL      1
#define SUPPORT_FILEFORMAT_IQM      1
#define SUPPORT_FILEFORMAT_GLTF     1
// Support procedural mesh generation functions, uses external par_shapes.h library
// NOTE: Some generated meshes DO NOT include generated texture coordinates
#define SUPPORT_MESH_GENERATION     1


//------------------------------------------------------------------------------------
// Module: audio - Configuration Flags
//------------------------------------------------------------------------------------
// Desired audio fileformats to be supported for loading
#define SUPPORT_FILEFORMAT_WAV      1
#define SUPPORT_FILEFORMAT_OGG      1
#define SUPPORT_FILEFORMAT_XM       1
#define SUPPORT_FILEFORMAT_MOD      1
#define SUPPORT_FILEFORMAT_MP3      1
//#define SUPPORT_FILEFORMAT_FLAC     1

// audio: Configuration values
//------------------------------------------------------------------------------------
#define AUDIO_DEVICE_FORMAT    ma_format_f32    // Device output format (miniaudio: float-32bit)
#define AUDIO_DEVICE_CHANNELS              2    // Device output channels: stereo
#define AUDIO_DEVICE_SAMPLE_RATE           0    // Device sample rate (device default)

#define MAX_AUDIO_BUFFER_POOL_CHANNELS    16    // Maximum number of audio pool channels

//------------------------------------------------------------------------------------
// Module: utils - Configuration Flags
//------------------------------------------------------------------------------------
// Standard file io library (stdio.h) included
#define SUPPORT_STANDARD_FILEIO
// Show TRACELOG() output messages
// NOTE: By default LOG_DEBUG traces not shown
#define SUPPORT_TRACELOG            1
//#define SUPPORT_TRACELOG_DEBUG      1

// utils: Configuration values
//------------------------------------------------------------------------------------
#define MAX_TRACELOG_MSG_LENGTH          128    // Max length of one trace-log message
#define MAX_UWP_MESSAGES                 512    // Max UWP messages to process
