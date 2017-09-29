/**********************************************************************************************
*
*   raylib v1.8.0 stripped
*
*   A simple and easy-to-use library to learn videogames programming (www.raylib.com)
*
*   FEATURES:
*       - Library written in plain C code (C99)
*       - Multiple platforms supported: Windows, Linux, Mac, Android, Raspberry Pi, HTML5.
*       - Hardware accelerated with OpenGL (1.1, 2.1, 3.3 or ES 2.0)
*       - Unique OpenGL abstraction layer (usable as standalone module): [rlgl]
*       - Powerful fonts module with SpriteFonts support (XNA bitmap fonts, AngelCode fonts, TTF)
*       - Multiple textures support, including compressed formats and mipmaps generation
*       - Basic 3d support for Shapes, Models, Billboards, Heightmaps and Cubicmaps
*       - Powerful math module for Vector2, Vector3, Matrix and Quaternion operations: [raymath]
*       - Audio loading and playing with streaming support and mixing channels: [audio]
*       - VR stereo rendering support with configurable HMD device parameters
*       - Minimal external dependencies (GLFW3, OpenGL, OpenAL)
*       - Complete bindings for Lua, Go and Pascal
*
*   NOTES:
*       32bit Colors - Any defined Color is always RGBA (4 byte)
*       One custom font is loaded by default when InitWindow() [core]
*       If using OpenGL 3.3 or ES2, one default shader is loaded automatically (internally defined) [rlgl]
*       If using OpenGL 3.3 or ES2, several vertex buffers (VAO/VBO) are created to manage lines-triangles-quads
*
*   DEPENDENCIES:
*       GLFW3 (www.glfw.org) for window/context management and input [core]
*       GLAD for OpenGL extensions loading (3.3 Core profile, only PLATFORM_DESKTOP) [rlgl]
*       OpenAL Soft for audio device/context management [audio]
*
*   OPTIONAL DEPENDENCIES:
*       stb_image (Sean Barret) for images loading (JPEG, PNG, BMP, TGA) [textures]
*       stb_image_write (Sean Barret) for image writting (PNG) [utils]
*       stb_truetype (Sean Barret) for ttf fonts loading [text]
*       stb_vorbis (Sean Barret) for ogg audio loading [audio]
*       jar_xm (Joshua Reisenauer) for XM audio module loading [audio]
*       jar_mod (Joshua Reisenauer) for MOD audio module loading [audio]
*       dr_flac (David Reid) for FLAC audio file loading [audio]
*       tinfl for data decompression (DEFLATE algorithm) [rres]
*
*
*   LICENSE: zlib/libpng
*
*   raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2013-2017 Ramon Santamaria (@raysan5)
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

#ifndef RAYLIB_H
#define RAYLIB_H

// Choose your platform here or just define it at compile time: -DPLATFORM_DESKTOP
//#define PLATFORM_DESKTOP      // Windows, Linux or OSX
//#define PLATFORM_ANDROID      // Android device
//#define PLATFORM_RPI          // Raspberry Pi
//#define PLATFORM_WEB          // HTML5 (emscripten, asm.js)

// Security check in case no PLATFORM_* defined
#if !defined(PLATFORM_DESKTOP) && !defined(PLATFORM_ANDROID) && !defined(PLATFORM_RPI) && !defined(PLATFORM_WEB)
    #define PLATFORM_DESKTOP
#endif

#if defined(_WIN32) && defined(BUILDING_DLL)
    #define RLAPI __declspec(dllexport)         // We are building raylib as a Win32 DLL
#elif defined(_WIN32) && defined(RAYLIB_DLL)
    #define RLAPI __declspec(dllimport)         // We are using raylib as a Win32 DLL
#else
    #define RLAPI   // We are building or using raylib as a static library (or Linux shared library)
#endif

//----------------------------------------------------------------------------------
// Some basic Defines
//----------------------------------------------------------------------------------
#ifndef PI
    #define PI 3.14159265358979323846f
#endif

#define DEG2RAD (PI/180.0f)
#define RAD2DEG (180.0f/PI)

// raylib Config Flags
#define FLAG_SHOW_LOGO              1       // Set to show raylib logo at startup
#define FLAG_FULLSCREEN_MODE        2       // Set to run program in fullscreen
#define FLAG_WINDOW_RESIZABLE       4       // Set to allow resizable window
#define FLAG_WINDOW_DECORATED       8       // Set to show window decoration (frame and buttons)
#define FLAG_WINDOW_TRANSPARENT    16       // Set to allow transparent window
#define FLAG_MSAA_4X_HINT          32       // Set to try enabling MSAA 4X
#define FLAG_VSYNC_HINT            64       // Set to try enabling V-Sync on GPU

// Keyboard Function Keys
#define KEY_SPACE            32
#define KEY_ESCAPE          256
#define KEY_ENTER           257
#define KEY_BACKSPACE       259
#define KEY_RIGHT           262
#define KEY_LEFT            263
#define KEY_DOWN            264
#define KEY_UP              265
#define KEY_F1              290
#define KEY_F2              291
#define KEY_F3              292
#define KEY_F4              293
#define KEY_F5              294
#define KEY_F6              295
#define KEY_F7              296
#define KEY_F8              297
#define KEY_F9              298
#define KEY_F10             299
#define KEY_F11             300
#define KEY_F12             301
#define KEY_LEFT_SHIFT      340
#define KEY_LEFT_CONTROL    341
#define KEY_LEFT_ALT        342
#define KEY_RIGHT_SHIFT     344
#define KEY_RIGHT_CONTROL   345
#define KEY_RIGHT_ALT       346

// Keyboard Alpha Numeric Keys
#define KEY_ZERO             48
#define KEY_ONE              49
#define KEY_TWO              50
#define KEY_THREE            51
#define KEY_FOUR             52
#define KEY_FIVE             53
#define KEY_SIX              54
#define KEY_SEVEN            55
#define KEY_EIGHT            56
#define KEY_NINE             57
#define KEY_A                65
#define KEY_B                66
#define KEY_C                67
#define KEY_D                68
#define KEY_E                69
#define KEY_F                70
#define KEY_G                71
#define KEY_H                72
#define KEY_I                73
#define KEY_J                74
#define KEY_K                75
#define KEY_L                76
#define KEY_M                77
#define KEY_N                78
#define KEY_O                79
#define KEY_P                80
#define KEY_Q                81
#define KEY_R                82
#define KEY_S                83
#define KEY_T                84
#define KEY_U                85
#define KEY_V                86
#define KEY_W                87
#define KEY_X                88
#define KEY_Y                89
#define KEY_Z                90

#if defined(PLATFORM_ANDROID)
    // Android Physical Buttons
    #define KEY_BACK              4
    #define KEY_MENU             82
    #define KEY_VOLUME_UP        24
    #define KEY_VOLUME_DOWN      25
#endif

// Mouse Buttons
#define MOUSE_LEFT_BUTTON     0
#define MOUSE_RIGHT_BUTTON    1
#define MOUSE_MIDDLE_BUTTON   2

// Touch points registered
#define MAX_TOUCH_POINTS     2

// Gamepad Number
#define GAMEPAD_PLAYER1       0
#define GAMEPAD_PLAYER2       1
#define GAMEPAD_PLAYER3       2
#define GAMEPAD_PLAYER4       3

// Gamepad Buttons/Axis

// PS3 USB Controller Buttons
#define GAMEPAD_PS3_BUTTON_TRIANGLE 0
#define GAMEPAD_PS3_BUTTON_CIRCLE   1
#define GAMEPAD_PS3_BUTTON_CROSS    2
#define GAMEPAD_PS3_BUTTON_SQUARE   3
#define GAMEPAD_PS3_BUTTON_L1       6
#define GAMEPAD_PS3_BUTTON_R1       7
#define GAMEPAD_PS3_BUTTON_L2       4
#define GAMEPAD_PS3_BUTTON_R2       5
#define GAMEPAD_PS3_BUTTON_START    8
#define GAMEPAD_PS3_BUTTON_SELECT   9
#define GAMEPAD_PS3_BUTTON_UP      24
#define GAMEPAD_PS3_BUTTON_RIGHT   25
#define GAMEPAD_PS3_BUTTON_DOWN    26
#define GAMEPAD_PS3_BUTTON_LEFT    27
#define GAMEPAD_PS3_BUTTON_PS      12

// PS3 USB Controller Axis
#define GAMEPAD_PS3_AXIS_LEFT_X     0
#define GAMEPAD_PS3_AXIS_LEFT_Y     1
#define GAMEPAD_PS3_AXIS_RIGHT_X    2
#define GAMEPAD_PS3_AXIS_RIGHT_Y    5
#define GAMEPAD_PS3_AXIS_L2         3       // [1..-1] (pressure-level)
#define GAMEPAD_PS3_AXIS_R2         4       // [1..-1] (pressure-level)

// Xbox360 USB Controller Buttons
#define GAMEPAD_XBOX_BUTTON_A       0
#define GAMEPAD_XBOX_BUTTON_B       1
#define GAMEPAD_XBOX_BUTTON_X       2
#define GAMEPAD_XBOX_BUTTON_Y       3
#define GAMEPAD_XBOX_BUTTON_LB      4
#define GAMEPAD_XBOX_BUTTON_RB      5
#define GAMEPAD_XBOX_BUTTON_SELECT  6
#define GAMEPAD_XBOX_BUTTON_START   7
#define GAMEPAD_XBOX_BUTTON_UP      10
#define GAMEPAD_XBOX_BUTTON_RIGHT   11
#define GAMEPAD_XBOX_BUTTON_DOWN    12
#define GAMEPAD_XBOX_BUTTON_LEFT    13
#define GAMEPAD_XBOX_BUTTON_HOME    8

// Xbox360 USB Controller Axis
// NOTE: For Raspberry Pi, axis must be reconfigured
#if defined(PLATFORM_RPI)
    #define GAMEPAD_XBOX_AXIS_LEFT_X    0   // [-1..1] (left->right)
    #define GAMEPAD_XBOX_AXIS_LEFT_Y    1   // [-1..1] (up->down)
    #define GAMEPAD_XBOX_AXIS_RIGHT_X   3   // [-1..1] (left->right)
    #define GAMEPAD_XBOX_AXIS_RIGHT_Y   4   // [-1..1] (up->down)
    #define GAMEPAD_XBOX_AXIS_LT        2   // [-1..1] (pressure-level)
    #define GAMEPAD_XBOX_AXIS_RT        5   // [-1..1] (pressure-level)
#else
    #define GAMEPAD_XBOX_AXIS_LEFT_X    0   // [-1..1] (left->right)
    #define GAMEPAD_XBOX_AXIS_LEFT_Y    1   // [1..-1] (up->down)
    #define GAMEPAD_XBOX_AXIS_RIGHT_X   2   // [-1..1] (left->right)
    #define GAMEPAD_XBOX_AXIS_RIGHT_Y   3   // [1..-1] (up->down)
    #define GAMEPAD_XBOX_AXIS_LT        4   // [-1..1] (pressure-level)
    #define GAMEPAD_XBOX_AXIS_RT        5   // [-1..1] (pressure-level)
#endif

// NOTE: MSC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized from { } initializers.
#ifdef __cplusplus
    #define CLITERAL
#else
    #define CLITERAL    (Color)
#endif

// Some Basic Colors
// NOTE: Custom raylib color palette for amazing visuals on WHITE background
#define LIGHTGRAY  CLITERAL{ 200, 200, 200, 255 }   // Light Gray
#define GRAY       CLITERAL{ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   CLITERAL{ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     CLITERAL{ 253, 249, 0, 255 }     // Yellow
#define GOLD       CLITERAL{ 255, 203, 0, 255 }     // Gold
#define ORANGE     CLITERAL{ 255, 161, 0, 255 }     // Orange
#define PINK       CLITERAL{ 255, 109, 194, 255 }   // Pink
#define RED        CLITERAL{ 230, 41, 55, 255 }     // Red
#define MAROON     CLITERAL{ 190, 33, 55, 255 }     // Maroon
#define GREEN      CLITERAL{ 0, 228, 48, 255 }      // Green
#define LIME       CLITERAL{ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  CLITERAL{ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    CLITERAL{ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       CLITERAL{ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   CLITERAL{ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     CLITERAL{ 200, 122, 255, 255 }   // Purple
#define VIOLET     CLITERAL{ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE CLITERAL{ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      CLITERAL{ 211, 176, 131, 255 }   // Beige
#define BROWN      CLITERAL{ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  CLITERAL{ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      CLITERAL{ 255, 255, 255, 255 }   // White
#define BLACK      CLITERAL{ 0, 0, 0, 255 }         // Black
#define BLANK      CLITERAL{ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    CLITERAL{ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   CLITERAL{ 245, 245, 245, 255 }   // My own White (raylib logo)

// Shader and material limits
#define MAX_SHADER_LOCATIONS        32      // Maximum number of predefined locations stored in shader struct
#define MAX_MATERIAL_MAPS           12      // Maximum number of texture maps stored in shader struct

//----------------------------------------------------------------------------------
// Structures Definition
//----------------------------------------------------------------------------------
#ifndef __cplusplus
// Boolean type
    #if !defined(_STDBOOL_H) || !defined(__STDBOOL_H)   // CLang uses second form
        typedef enum { false, true } bool;
    #endif
#endif

// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

// Vector3 type
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

// Matrix type (OpenGL style 4x4 - right handed, column major)
typedef struct Matrix {
    float m0, m4, m8, m12;
    float m1, m5, m9, m13;
    float m2, m6, m10, m14;
    float m3, m7, m11, m15;
} Matrix;

// Color type, RGBA (32bit)
typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

// Rectangle type
typedef struct Rectangle {
    int x;
    int y;
    int width;
    int height;
} Rectangle;

// Image type, bpp always RGBA (32bit)
// NOTE: Data stored in CPU memory (RAM)
typedef struct Image {
    void *data;             // Image raw data
    int width;              // Image base width
    int height;             // Image base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (TextureFormat type)
} Image;

// Texture2D type
// NOTE: Data stored in GPU memory
typedef struct Texture2D {
    unsigned int id;        // OpenGL texture id
    int width;              // Texture base width
    int height;             // Texture base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (TextureFormat type)
} Texture2D;

// RenderTexture2D type, for texture rendering
typedef struct RenderTexture2D {
    unsigned int id;        // OpenGL Framebuffer Object (FBO) id
    Texture2D texture;      // Color buffer attachment texture
    Texture2D depth;        // Depth buffer attachment texture
} RenderTexture2D;

// SpriteFont character info
typedef struct CharInfo {
    int value;              // Character value (Unicode)
    Rectangle rec;          // Character rectangle in sprite font
    int offsetX;            // Character offset X when drawing
    int offsetY;            // Character offset Y when drawing
    int advanceX;           // Character advance position X
} CharInfo;

// SpriteFont type, includes texture and charSet array data
typedef struct SpriteFont {
    Texture2D texture;      // Font texture
    int baseSize;           // Base size (default chars height)
    int charsCount;         // Number of characters
    CharInfo *chars;        // Characters info data
} SpriteFont;

// Camera type, defines a camera position/orientation in 3d space
typedef struct Camera {
    Vector3 position;       // Camera position
    Vector3 target;         // Camera target it looks-at
    Vector3 up;             // Camera up vector (rotation over its axis)
    float fovy;             // Camera field-of-view apperture in Y (degrees)
} Camera;

// Camera2D type, defines a 2d camera
typedef struct Camera2D {
    Vector2 offset;         // Camera offset (displacement from target)
    Vector2 target;         // Camera target (rotation and zoom origin)
    float rotation;         // Camera rotation in degrees
    float zoom;             // Camera zoom (scaling), should be 1.0f by default
} Camera2D;

// Bounding box type
typedef struct BoundingBox {
    Vector3 min;            // Minimum vertex box-corner
    Vector3 max;            // Maximum vertex box-corner
} BoundingBox;

// Vertex data definning a mesh
// NOTE: Data stored in CPU memory (and GPU)
typedef struct Mesh {
    int vertexCount;        // Number of vertices stored in arrays
    int triangleCount;      // Number of triangles stored (indexed or not)

    float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;      // Vertex second texture coordinates (useful for lightmaps) (shader-location = 5)
    float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    float *tangents;        // Vertex tangents (XYZ - 3 components per vertex) (shader-location = 4)
    unsigned char *colors;  // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    unsigned short *indices;// Vertex indices (in case vertex data comes indexed)

    unsigned int vaoId;     // OpenGL Vertex Array Object id
    unsigned int vboId[7];  // OpenGL Vertex Buffer Objects id (7 types of vertex data)
} Mesh;

// Shader type (generic)
typedef struct Shader {
    unsigned int id;                // Shader program id
    int locs[MAX_SHADER_LOCATIONS]; // Shader locations array
} Shader;

// Material texture map
typedef struct MaterialMap {
    Texture2D texture;      // Material map texture
    Color color;            // Material map color
    float value;            // Material map value
} MaterialMap;

// Material type (generic)
typedef struct Material {
    Shader shader;          // Material shader
    MaterialMap maps[MAX_MATERIAL_MAPS]; // Material maps
    float *params;          // Material generic parameters (if required)
} Material;

// Model type
typedef struct Model {
    Mesh mesh;              // Vertex data buffers (RAM and VRAM)
    Matrix transform;       // Local transform matrix
    Material material;      // Shader and textures data
} Model;

// Ray type (useful for raycast)
typedef struct Ray {
    Vector3 position;       // Ray position (origin)
    Vector3 direction;      // Ray direction
} Ray;

//----------------------------------------------------------------------------------
// Enumerators Definition
//----------------------------------------------------------------------------------
// Trace log type
typedef enum { 
    LOG_INFO = 0,
    LOG_WARNING, 
    LOG_ERROR, 
    LOG_DEBUG, 
    LOG_OTHER 
} LogType;

// Shader location point type
typedef enum {
    LOC_VERTEX_POSITION = 0,
    LOC_VERTEX_TEXCOORD01,
    LOC_VERTEX_TEXCOORD02,
    LOC_VERTEX_NORMAL,
    LOC_VERTEX_TANGENT,
    LOC_VERTEX_COLOR,
    LOC_MATRIX_MVP,
    LOC_MATRIX_MODEL,
    LOC_MATRIX_VIEW,
    LOC_MATRIX_PROJECTION,
    LOC_VECTOR_VIEW,
    LOC_COLOR_DIFFUSE,
    LOC_COLOR_SPECULAR,
    LOC_COLOR_AMBIENT,
    LOC_MAP_ALBEDO,          // LOC_MAP_DIFFUSE
    LOC_MAP_METALNESS,       // LOC_MAP_SPECULAR
    LOC_MAP_NORMAL,
    LOC_MAP_ROUGHNESS,
    LOC_MAP_OCCUSION,
    LOC_MAP_EMISSION,
    LOC_MAP_HEIGHT,
    LOC_MAP_CUBEMAP,
    LOC_MAP_IRRADIANCE,
    LOC_MAP_PREFILTER,
    LOC_MAP_BRDF
} ShaderLocationIndex;

#define LOC_MAP_DIFFUSE      LOC_MAP_ALBEDO
#define LOC_MAP_SPECULAR     LOC_MAP_METALNESS

// Material map type
typedef enum {
    MAP_ALBEDO    = 0,       // MAP_DIFFUSE
    MAP_METALNESS = 1,       // MAP_SPECULAR
    MAP_NORMAL    = 2,
    MAP_ROUGHNESS = 3,
    MAP_OCCLUSION,
    MAP_EMISSION,
    MAP_HEIGHT,
    MAP_CUBEMAP,             // NOTE: Uses GL_TEXTURE_CUBE_MAP
    MAP_IRRADIANCE,          // NOTE: Uses GL_TEXTURE_CUBE_MAP
    MAP_PREFILTER,           // NOTE: Uses GL_TEXTURE_CUBE_MAP
    MAP_BRDF
} TexmapIndex;

#define MAP_DIFFUSE      MAP_ALBEDO
#define MAP_SPECULAR     MAP_METALNESS

// Texture formats
// NOTE: Support depends on OpenGL version and platform
typedef enum {
    UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
    UNCOMPRESSED_GRAY_ALPHA,        // 16 bpp (2 channels)
    UNCOMPRESSED_R5G6B5,            // 16 bpp
    UNCOMPRESSED_R8G8B8,            // 24 bpp
    UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
    UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
    UNCOMPRESSED_R8G8B8A8,          // 32 bpp
    UNCOMPRESSED_R32G32B32,         // 32 bit per channel (float) - HDR
    COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
    COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
    COMPRESSED_DXT3_RGBA,           // 8 bpp
    COMPRESSED_DXT5_RGBA,           // 8 bpp
    COMPRESSED_ETC1_RGB,            // 4 bpp
    COMPRESSED_ETC2_RGB,            // 4 bpp
    COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
    COMPRESSED_PVRT_RGB,            // 4 bpp
    COMPRESSED_PVRT_RGBA,           // 4 bpp
    COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
    COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
} TextureFormat;

// Texture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
typedef enum {
    FILTER_POINT = 0,               // No filter, just pixel aproximation
    FILTER_BILINEAR,                // Linear filtering
    FILTER_TRILINEAR,               // Trilinear filtering (linear with mipmaps)
    FILTER_ANISOTROPIC_4X,          // Anisotropic filtering 4x
    FILTER_ANISOTROPIC_8X,          // Anisotropic filtering 8x
    FILTER_ANISOTROPIC_16X,         // Anisotropic filtering 16x
} TextureFilterMode;

// Texture parameters: wrap mode
typedef enum { 
    WRAP_REPEAT = 0, 
    WRAP_CLAMP, 
    WRAP_MIRROR 
} TextureWrapMode;

// Color blending modes (pre-defined)
typedef enum { 
    BLEND_ALPHA = 0, 
    BLEND_ADDITIVE, 
    BLEND_MULTIPLIED
} BlendMode;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Global Variables Definition
//------------------------------------------------------------------------------------
// It's lonely here...

//------------------------------------------------------------------------------------
// Window and Graphics Device Functions (Module: core)
//------------------------------------------------------------------------------------

// Window-related functions
RLAPI void InitWindow(int width, int height, void *state);        // Initialize Android activity

RLAPI void CloseWindow(void);                                     // Close window and unload OpenGL context
RLAPI bool WindowShouldClose(void);                               // Check if KEY_ESCAPE pressed or Close icon pressed
RLAPI bool IsWindowMinimized(void);                               // Check if window has been minimized (or lost focus)
RLAPI void ToggleFullscreen(void);                                // Toggle fullscreen mode (only PLATFORM_DESKTOP)
RLAPI void SetWindowIcon(Image image);                            // Set icon for window (only PLATFORM_DESKTOP)
RLAPI void SetWindowTitle(const char *title);                     // Set title for window (only PLATFORM_DESKTOP)
RLAPI void SetWindowPosition(int x, int y);                       // Set window position on screen (only PLATFORM_DESKTOP)
RLAPI void SetWindowMonitor(int monitor);                         // Set monitor for the current window (fullscreen mode)
RLAPI void SetWindowMinSize(int width, int height);               // Set window minimum dimensions (for FLAG_WINDOW_RESIZABLE)
RLAPI int GetScreenWidth(void);                                   // Get current screen width
RLAPI int GetScreenHeight(void);                                  // Get current screen height

// Drawing-related functions
RLAPI void ClearBackground(Color color);                          // Set background color (framebuffer clear color)
RLAPI void BeginDrawing(void);                                    // Setup canvas (framebuffer) to start drawing
RLAPI void EndDrawing(void);                                      // End canvas drawing and swap buffers (double buffering)
RLAPI void Begin2dMode(Camera2D camera);                          // Initialize 2D mode with custom camera (2D)
RLAPI void End2dMode(void);                                       // Ends 2D mode with custom camera
RLAPI void Begin3dMode(Camera camera);                            // Initializes 3D mode with custom camera (3D)
RLAPI void End3dMode(void);                                       // Ends 3D mode and returns to default 2D orthographic mode
RLAPI void BeginTextureMode(RenderTexture2D target);              // Initializes render texture for drawing
RLAPI void EndTextureMode(void);                                  // Ends drawing to render texture

// Screen-space-related functions
RLAPI Ray GetMouseRay(Vector2 mousePosition, Camera camera);      // Returns a ray trace from mouse position
RLAPI Vector2 GetWorldToScreen(Vector3 position, Camera camera);  // Returns the screen space position for a 3d world space position
RLAPI Matrix GetCameraMatrix(Camera camera);                      // Returns camera transform matrix (view matrix)

// Timming-related functions
RLAPI void SetTargetFPS(int fps);                                 // Set target FPS (maximum)
RLAPI int GetFPS(void);                                           // Returns current FPS
RLAPI float GetFrameTime(void);                                   // Returns time in seconds for last frame drawn

// Color-related functions
RLAPI int GetHexValue(Color color);                               // Returns hexadecimal value for a Color
RLAPI Color GetColor(int hexValue);                               // Returns a Color struct from hexadecimal value
RLAPI Color Fade(Color color, float alpha);                       // Color fade-in or fade-out, alpha goes from 0.0f to 1.0f
RLAPI float *ColorToFloat(Color color);                           // Converts Color to float array and normalizes

// Math useful functions (available from raymath.h)
RLAPI float *VectorToFloat(Vector3 vec);                          // Returns Vector3 as float array
RLAPI float *MatrixToFloat(Matrix mat);                           // Returns Matrix as float array
RLAPI Vector3 Vector3Zero(void);                                  // Vector with components value 0.0f
RLAPI Vector3 Vector3One(void);                                   // Vector with components value 1.0f
RLAPI Matrix MatrixIdentity(void);                                // Returns identity matrix

// Misc. functions
RLAPI void ShowLogo(void);                                        // Activate raylib logo at startup (can be done with flags)
RLAPI void SetConfigFlags(char flags);                            // Setup window configuration flags (view FLAGS)
RLAPI void TraceLog(int logType, const char *text, ...);          // Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
RLAPI void TakeScreenshot(const char *fileName);                  // Takes a screenshot of current screen (saved a .png)
RLAPI int GetRandomValue(int min, int max);                       // Returns a random value between min and max (both included)

// Files management functions
RLAPI bool IsFileExtension(const char *fileName, const char *ext);// Check file extension
RLAPI const char *GetExtension(const char *fileName);             // Get file extension
RLAPI const char *GetDirectoryPath(const char *fileName);         // Get directory for a given fileName (with path)
RLAPI const char *GetWorkingDirectory(void);                      // Get current working directory
RLAPI bool ChangeDirectory(const char *dir);                      // Change working directory, returns true if success
RLAPI bool IsFileDropped(void);                                   // Check if a file has been dropped into window
RLAPI char **GetDroppedFiles(int *count);                         // Get dropped files names
RLAPI void ClearDroppedFiles(void);                               // Clear dropped files paths buffer

// Persistent storage management
RLAPI void StorageSaveValue(int position, int value);             // Save integer value to storage file (to defined position)
RLAPI int StorageLoadValue(int position);                         // Load integer value from storage file (from defined position)

//------------------------------------------------------------------------------------
// Input Handling Functions (Module: core)
//------------------------------------------------------------------------------------

// Input-related functions: keyboard
RLAPI bool IsKeyPressed(int key);                             // Detect if a key has been pressed once
RLAPI bool IsKeyDown(int key);                                // Detect if a key is being pressed
RLAPI bool IsKeyReleased(int key);                            // Detect if a key has been released once
RLAPI bool IsKeyUp(int key);                                  // Detect if a key is NOT being pressed
RLAPI int GetKeyPressed(void);                                // Get latest key pressed
RLAPI void SetExitKey(int key);                               // Set a custom key to exit program (default is ESC)

// Input-related functions: gamepads
RLAPI bool IsGamepadAvailable(int gamepad);                   // Detect if a gamepad is available
RLAPI bool IsGamepadName(int gamepad, const char *name);      // Check gamepad name (if available)
RLAPI const char *GetGamepadName(int gamepad);                // Return gamepad internal name id
RLAPI bool IsGamepadButtonPressed(int gamepad, int button);   // Detect if a gamepad button has been pressed once
RLAPI bool IsGamepadButtonDown(int gamepad, int button);      // Detect if a gamepad button is being pressed
RLAPI bool IsGamepadButtonReleased(int gamepad, int button);  // Detect if a gamepad button has been released once
RLAPI bool IsGamepadButtonUp(int gamepad, int button);        // Detect if a gamepad button is NOT being pressed
RLAPI int GetGamepadButtonPressed(void);                      // Get the last gamepad button pressed
RLAPI int GetGamepadAxisCount(int gamepad);                   // Return gamepad axis count for a gamepad
RLAPI float GetGamepadAxisMovement(int gamepad, int axis);    // Return axis movement value for a gamepad axis

// Input-related functions: mouse
RLAPI bool IsMouseButtonPressed(int button);                  // Detect if a mouse button has been pressed once
RLAPI bool IsMouseButtonDown(int button);                     // Detect if a mouse button is being pressed
RLAPI bool IsMouseButtonReleased(int button);                 // Detect if a mouse button has been released once
RLAPI bool IsMouseButtonUp(int button);                       // Detect if a mouse button is NOT being pressed
RLAPI int GetMouseX(void);                                    // Returns mouse position X
RLAPI int GetMouseY(void);                                    // Returns mouse position Y
RLAPI Vector2 GetMousePosition(void);                         // Returns mouse position XY
RLAPI void SetMousePosition(Vector2 position);                // Set mouse position XY
RLAPI int GetMouseWheelMove(void);                            // Returns mouse wheel movement Y

// Input-related functions: touch
RLAPI int GetTouchX(void);                                    // Returns touch position X for touch point 0 (relative to screen size)
RLAPI int GetTouchY(void);                                    // Returns touch position Y for touch point 0 (relative to screen size)
RLAPI Vector2 GetTouchPosition(int index);                    // Returns touch position XY for a touch point index (relative to screen size)

//------------------------------------------------------------------------------
// Shaders System Functions (Module: rlgl)
// NOTE: This functions are useless when using OpenGL 1.1
//------------------------------------------------------------------------------------

// Shader loading/unloading functions
RLAPI char *LoadText(const char *fileName);                               // Load chars array from text file
RLAPI Shader LoadShader(char *vsFileName, char *fsFileName);              // Load shader from files and bind default locations
RLAPI void UnloadShader(Shader shader);                                   // Unload shader from GPU memory (VRAM)

RLAPI Shader GetShaderDefault(void);                                      // Get default shader
RLAPI Texture2D GetTextureDefault(void);                                  // Get default texture

// Shader configuration functions
RLAPI int GetShaderLocation(Shader shader, const char *uniformName);              // Get shader uniform location
RLAPI void SetShaderValue(Shader shader, int uniformLoc, float *value, int size); // Set shader uniform value (float)
RLAPI void SetShaderValuei(Shader shader, int uniformLoc, int *value, int size);  // Set shader uniform value (int)
RLAPI void SetShaderValueMatrix(Shader shader, int uniformLoc, Matrix mat);       // Set shader uniform value (matrix 4x4)
RLAPI void SetMatrixProjection(Matrix proj);                              // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void SetMatrixModelview(Matrix view);                               // Set a custom modelview matrix (replaces internal modelview matrix)

// Texture maps generation (PBR)
// NOTE: Required shaders should be provided
RLAPI Texture2D GenTextureCubemap(Shader shader, Texture2D skyHDR, int size);       // Generate cubemap texture from HDR texture
RLAPI Texture2D GenTextureIrradiance(Shader shader, Texture2D cubemap, int size);   // Generate irradiance texture using cubemap data
RLAPI Texture2D GenTexturePrefilter(Shader shader, Texture2D cubemap, int size);    // Generate prefilter texture using cubemap data
RLAPI Texture2D GenTextureBRDF(Shader shader, Texture2D cubemap, int size);         // Generate BRDF texture using cubemap data

// Shading begin/end functions
RLAPI void BeginShaderMode(Shader shader);                                // Begin custom shader drawing
RLAPI void EndShaderMode(void);                                           // End custom shader drawing (use default shader)
RLAPI void BeginBlendMode(int mode);                                      // Begin blending mode (alpha, additive, multiplied)
RLAPI void EndBlendMode(void);                                            // End blending mode (reset to default: alpha blending)

#ifdef __cplusplus
}
#endif

#endif // RAYLIB_H
