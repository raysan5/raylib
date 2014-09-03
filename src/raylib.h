/*********************************************************************************************
*
*   raylib 1.1 (www.raylib.com)
*
*   A simple and easy-to-use library to learn videogames programming
*
*   Features:
*     Library written in plain C code (C99)
*     Uses C# PascalCase/camelCase notation
*     Hardware accelerated with OpenGL (1.1, 3.3+ or ES2)
*     Unique OpenGL abstraction layer [rlgl]
*     Powerful fonts module with SpriteFonts support
*     Multiple textures support, including DDS and mipmaps generation
*     Basic 3d support for Shapes, Models, Heightmaps and Billboards
*     Powerful math module for Vector and Matrix operations [raymath]
*     Audio loading and playing with streaming support
*
*   Used external libs:
*     GLFW3 (www.glfw.org) for window/context management and input
*     GLEW for OpenGL extensions loading (3.3+ and ES2)
*     stb_image (Sean Barret) for images loading (JPEG, PNG, BMP, TGA, PSD, GIF, HDR, PIC)
*     stb_image_write (Sean Barret) for image writting (PNG)
*     stb_vorbis (Sean Barret) for ogg audio loading
*     OpenAL Soft for audio device/context management
*     tinfl for data decompression (DEFLATE algorithm)
*
*   Some design decisions:
*     32bit Colors - All defined color are always RGBA
*     32bit Textures - All loaded images are converted automatically to RGBA textures
*     SpriteFonts - All loaded sprite-font images are converted to RGBA and POT textures
*     One custom default font is loaded automatically when InitWindow()
*     If using OpenGL 3.3+ or ES2, one default shader is loaded automatically (internally defined)
*
*   -- LICENSE (raylib v1.1, April 2014) --
*
*   raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

//----------------------------------------------------------------------------------
// Some basic Defines
//----------------------------------------------------------------------------------
#ifndef PI
#define PI 3.14159265358979323846
#endif

#define DEG2RAD (PI / 180.0)
#define RAD2DEG (180.0 / PI)

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
#define KEY_LEFT_SHIFT      340
#define KEY_LEFT_CONTROL    341
#define KEY_LEFT_ALT        342
#define KEY_RIGHT_SHIFT     344
#define KEY_RIGHT_CONTROL   345
#define KEY_RIGHT_ALT       346

// Mouse Buttons
#define MOUSE_LEFT_BUTTON     0
#define MOUSE_RIGHT_BUTTON    1
#define MOUSE_MIDDLE_BUTTON   2

// Gamepad Number
#define GAMEPAD_PLAYER1       0
#define GAMEPAD_PLAYER2       1
#define GAMEPAD_PLAYER3       2
#define GAMEPAD_PLAYER4       3

// Gamepad Buttons
// NOTE: Adjusted for a PS3 USB Controller
#define GAMEPAD_BUTTON_A      2
#define GAMEPAD_BUTTON_B      1
#define GAMEPAD_BUTTON_X      3
#define GAMEPAD_BUTTON_Y      4
#define GAMEPAD_BUTTON_R1     7
#define GAMEPAD_BUTTON_R2     5
#define GAMEPAD_BUTTON_L1     6
#define GAMEPAD_BUTTON_L2     8
#define GAMEPAD_BUTTON_SELECT 9
#define GAMEPAD_BUTTON_START 10

// TODO: Review Xbox360 USB Controller Buttons

// Some Basic Colors
// NOTE: Custom raylib color palette for amazing visuals on WHITE background
#define LIGHTGRAY  (Color){ 200, 200, 200, 255 }   // Light Gray
#define GRAY       (Color){ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     (Color){ 253, 249, 0, 255 }     // Yellow
#define GOLD       (Color){ 255, 203, 0, 255 }     // Gold
#define ORANGE     (Color){ 255, 161, 0, 255 }     // Orange
#define PINK       (Color){ 255, 109, 194, 255 }   // Pink
#define RED        (Color){ 230, 41, 55, 255 }     // Red
#define MAROON     (Color){ 190, 33, 55, 255 }     // Maroon
#define GREEN      (Color){ 0, 228, 48, 255 }      // Green
#define LIME       (Color){ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  (Color){ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    (Color){ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       (Color){ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   (Color){ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     (Color){ 200, 122, 255, 255 }   // Purple
#define VIOLET     (Color){ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE (Color){ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      (Color){ 211, 176, 131, 255 }   // Beige
#define BROWN      (Color){ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  (Color){ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      (Color){ 255, 255, 255, 255 }   // White
#define BLACK      (Color){ 0, 0, 0, 255 }         // Black
#define BLANK      (Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    (Color){ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Boolean type
typedef enum { false, true } bool;

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
    Color *pixels;
    int width;
    int height;
} Image;

// Texture2D type, bpp always RGBA (32bit)
// NOTE: Data stored in GPU memory
typedef struct Texture2D {
    unsigned int id;        // OpenGL id
    int width;
    int height;
} Texture2D;

// Character type (one font glyph)
// NOTE: Defined in module: text
typedef struct Character Character;

// SpriteFont type, includes texture and charSet array data
typedef struct SpriteFont {
    Texture2D texture;
    int numChars;
    Character *charSet;
} SpriteFont;

// Camera type, defines a camera position/orientation in 3d space
typedef struct Camera {
    Vector3 position;
    Vector3 target;
    Vector3 up;
} Camera;

// Vertex data definning a mesh
typedef struct VertexData {
    int vertexCount;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    float *normals;             // 3 components per vertex
    unsigned char *colors;      // 4 components per vertex
} VertexData;

// 3d Model type
// NOTE: If using OpenGL 1.1, loaded in CPU (mesh); if OpenGL 3.3+ loaded in GPU (vaoId)
typedef struct Model {
    VertexData mesh;
    unsigned int vaoId;
    unsigned int textureId;
    //Matrix transform;
} Model;

// Sound source type
typedef struct Sound {
    unsigned int source;
    unsigned int buffer;
} Sound;

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
void InitWindow(int width, int height, const char *title);  // Initialize Window and Graphics Context (OpenGL)
void InitWindowEx(int width, int height, const char* title, // Initialize Window and Graphics Context (OpenGL),...
                  bool resizable, const char *cursorImage); // ...define if windows-resizable and custom cursor
void CloseWindow(void);                                     // Close Window and Terminate Context
bool WindowShouldClose(void);                               // Detect if KEY_ESCAPE pressed or Close icon pressed
void ToggleFullscreen(void);                                // Fullscreen toggle (by default F11)
void SetCustomCursor(const char *cursorImage);              // Set a custom cursor icon/image
void SetExitKey(int key);                                   // Set a custom key to exit program (default is ESC)

void ClearBackground(Color color);                          // Sets Background Color
void BeginDrawing(void);                                    // Setup drawing canvas to start drawing
void EndDrawing(void);                                      // End canvas drawing and Swap Buffers (Double Buffering)

void Begin3dMode(Camera cam);                               // Initializes 3D mode for drawing (Camera setup)
void End3dMode(void);                                       // Ends 3D mode and returns to default 2D orthographic mode

void SetTargetFPS(int fps);                                 // Set target FPS (maximum)
float GetFPS(void);                                         // Returns current FPS
float GetFrameTime(void);                                   // Returns time in seconds for one frame

Color GetColor(int hexValue);                               // Returns a Color struct from hexadecimal value
int GetHexValue(Color color);                               // Returns hexadecimal value for a Color

int GetRandomValue(int min, int max);                       // Returns a random value between min and max (both included)
Color Fade(Color color, float alpha);                       // Color fade-in or fade-out, alpha goes from 0.0 to 1.0

void ShowLogo(void);                                        // Activates raylib logo at startup

//------------------------------------------------------------------------------------
// Input Handling Functions (Module: core)
//------------------------------------------------------------------------------------
bool IsKeyPressed(int key);                             // Detect if a key has been pressed once
bool IsKeyDown(int key);                                // Detect if a key is being pressed
bool IsKeyReleased(int key);                            // Detect if a key has been released once
bool IsKeyUp(int key);                                  // Detect if a key is NOT being pressed

bool IsMouseButtonPressed(int button);                  // Detect if a mouse button has been pressed once
bool IsMouseButtonDown(int button);                     // Detect if a mouse button is being pressed
bool IsMouseButtonReleased(int button);                 // Detect if a mouse button has been released once
bool IsMouseButtonUp(int button);                       // Detect if a mouse button is NOT being pressed
int GetMouseX(void);                                    // Returns mouse position X
int GetMouseY(void);                                    // Returns mouse position Y
Vector2 GetMousePosition(void);                         // Returns mouse position XY
int GetMouseWheelMove(void);                            // Returns mouse wheel movement Y

bool IsGamepadAvailable(int gamepad);                   // Detect if a gamepad is available
Vector2 GetGamepadMovement(int gamepad);                // Return axis movement vector for a gamepad
bool IsGamepadButtonPressed(int gamepad, int button);   // Detect if a gamepad button has been pressed once
bool IsGamepadButtonDown(int gamepad, int button);      // Detect if a gamepad button is being pressed
bool IsGamepadButtonReleased(int gamepad, int button);  // Detect if a gamepad button has been released once
bool IsGamepadButtonUp(int gamepad, int button);        // Detect if a gamepad button is NOT being pressed

//------------------------------------------------------------------------------------
// Basic Shapes Drawing Functions (Module: shapes)
//------------------------------------------------------------------------------------
void DrawPixel(int posX, int posY, Color color);                                                   // Draw a pixel
void DrawPixelV(Vector2 position, Color color);                                                    // Draw a pixel (Vector version)
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                // Draw a line
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                     // Draw a line (Vector version)
void DrawCircle(int centerX, int centerY, float radius, Color color);                              // Draw a color-filled circle
void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);       // Draw a gradient-filled circle
void DrawCircleV(Vector2 center, float radius, Color color);                                       // Draw a color-filled circle (Vector version)
void DrawCircleLines(int centerX, int centerY, float radius, Color color);                         // Draw circle outline
void DrawRectangle(int posX, int posY, int width, int height, Color color);                        // Draw a color-filled rectangle
void DrawRectangleRec(Rectangle rec, Color color);                                                 // Draw a color-filled rectangle
void DrawRectangleGradient(int posX, int posY, int width, int height, Color color1, Color color2); // Draw a gradient-filled rectangle
void DrawRectangleV(Vector2 position, Vector2 size, Color color);                                  // Draw a color-filled rectangle (Vector version)
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);                   // Draw rectangle outline
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                                // Draw a color-filled triangle
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);                           // Draw triangle outline
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);               // Draw a regular polygon (Vector version)
void DrawPolyEx(Vector2 *points, int numPoints, Color color);                                      // Draw a closed polygon defined by points
void DrawPolyExLines(Vector2 *points, int numPoints, Color color);                                 // Draw polygon lines

bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);                                           // Check collision between two rectangles
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);        // Check collision between two circles
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);                         // Check collision between circle and rectangle
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);                                         // Get collision rectangle for two rectangles collision
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);                                         // Check if point is inside rectangle
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);                       // Check if point is inside circle
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);               // Check if point is inside a triangle

//------------------------------------------------------------------------------------
// Texture Loading and Drawing Functions (Module: textures)
//------------------------------------------------------------------------------------
Image LoadImage(const char *fileName);                                                             // Load an image into CPU memory (RAM)
Image LoadImageFromRES(const char *rresName, int resId);                                           // Load an image from rRES file (raylib Resource)
Texture2D LoadTexture(const char *fileName);                                                       // Load an image as texture into GPU memory
Texture2D LoadTextureFromRES(const char *rresName, int resId);                                     // Load an image as texture from rRES file (raylib Resource)
Texture2D CreateTexture(Image image, bool genMipmaps);                                             // Create a Texture2D from Image data (and generate mipmaps)
void UnloadImage(Image image);                                                                     // Unload image from CPU memory (RAM)
void UnloadTexture(Texture2D texture);                                                             // Unload texture from GPU memory

void DrawTexture(Texture2D texture, int posX, int posY, Color tint);                               // Draw a Texture2D
void DrawTextureV(Texture2D texture, Vector2 position, Color tint);                                // Draw a Texture2D with position defined as Vector2
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);  // Draw a Texture2D with extended parameters
void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint);         // Draw a part of a texture defined by a rectangle
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin,     // Draw a part of a texture defined by a rectangle with 'pro' parameters
                    float rotation, Color tint);

//------------------------------------------------------------------------------------
// Font Loading and Text Drawing Functions (Module: text)
//------------------------------------------------------------------------------------
SpriteFont GetDefaultFont(void);                                                                   // Get the default SpriteFont
SpriteFont LoadSpriteFont(const char *fileName);                                                   // Load a SpriteFont image into GPU memory
void UnloadSpriteFont(SpriteFont spriteFont);                                                      // Unload SpriteFont from GPU memory

void DrawText(const char *text, int posX, int posY, int fontSize, Color color);                    // Draw text (using default font)
void DrawTextEx(SpriteFont spriteFont, const char* text, Vector2 position,                         // Draw text using SpriteFont and additional parameters
                int fontSize, int spacing, Color tint);
int MeasureText(const char *text, int fontSize);                                                   // Measure string width for default font
Vector2 MeasureTextEx(SpriteFont spriteFont, const char *text, int fontSize, int spacing);         // Measure string size for SpriteFont
int GetFontBaseSize(SpriteFont spriteFont);                                                        // Returns the base size for a SpriteFont (chars height)
void DrawFPS(int posX, int posY);                                                                  // Shows current FPS on top-left corner
const char *FormatText(const char *text, ...);                                                     // Formatting of text with variables to 'embed'

//------------------------------------------------------------------------------------
// Basic 3d Shapes Drawing Functions (Module: models)
//------------------------------------------------------------------------------------
void DrawCube(Vector3 position, float width, float height, float lenght, Color color);             // Draw cube
void DrawCubeV(Vector3 position, Vector3 size, Color color);                                       // Draw cube (Vector version)
void DrawCubeWires(Vector3 position, float width, float height, float lenght, Color color);        // Draw cube wires
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float lenght, Color color); // Draw cube textured
void DrawSphere(Vector3 centerPos, float radius, Color color);                                     // Draw sphere
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);            // Draw sphere with extended parameters
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);         // Draw sphere wires
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone wires
void DrawPlane(Vector3 centerPos, Vector2 size, Vector3 rotation, Color color);                    // Draw a plane
void DrawPlaneEx(Vector3 centerPos, Vector2 size, Vector3 rotation, int slicesX, int slicesZ, Color color); // Draw a plane with divisions
void DrawGrid(int slices, float spacing);                                                          // Draw a grid (centered at (0, 0, 0))
void DrawGizmo(Vector3 position);                                                                  // Draw simple gizmo
void DrawGizmoEx(Vector3 position, Vector3 rotation, float scale);                                 // Draw gizmo with extended parameters
//DrawTorus(), DrawTeapot() are useless...

//------------------------------------------------------------------------------------
// Model 3d Loading and Drawing Functions (Module: models)
//------------------------------------------------------------------------------------
Model LoadModel(const char *fileName);                                                             // Load a 3d model (.OBJ)
//Model LoadModelFromRES(const char *rresName, int resId);                                         // TODO: Load a 3d model from rRES file (raylib Resource)
Model LoadHeightmap(Image heightmap, float maxHeight);                                             // Load a heightmap image as a 3d model
Model LoadCubesmap(Image cubesmap);                                                                // Load a map image as a 3d model (cubes based)
void UnloadModel(Model model);                                                                     // Unload 3d model from memory
void SetModelTexture(Model *model, Texture2D texture);                                             // Link a texture to a model

void DrawModel(Model model, Vector3 position, float scale, Color tint);                            // Draw a model (with texture if set)
void DrawModelEx(Model model, Vector3 position, Vector3 rotation, Vector3 scale, Color tint);      // Draw a model with extended parameters
void DrawModelWires(Model model, Vector3 position, float scale, Color color);                      // Draw a model wires (with texture if set)

void DrawBillboard(Camera camera, Texture2D texture, Vector3 center, float size, Color tint);                         // Draw a billboard texture
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 center, float size, Color tint); // Draw a billboard texture defined by sourceRec

//------------------------------------------------------------------------------------
// Audio Loading and Playing Functions (Module: audio)
//------------------------------------------------------------------------------------
void InitAudioDevice(void);                                     // Initialize audio device and context
void CloseAudioDevice(void);                                    // Close the audio device and context (and music stream)

Sound LoadSound(char *fileName);                                // Load sound to memory
Sound LoadSoundFromRES(const char *rresName, int resId);        // Load sound to memory from rRES file (raylib Resource)
void UnloadSound(Sound sound);                                  // Unload sound
void PlaySound(Sound sound);                                    // Play a sound
void PauseSound(Sound sound);                                   // Pause a sound
void StopSound(Sound sound);                                    // Stop playing a sound
bool SoundIsPlaying(Sound sound);                               // Check if a sound is currently playing
void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)
void SetSoundPitch(Sound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)

void PlayMusicStream(char *fileName);                           // Start music playing (open stream)
void StopMusicStream(void);                                     // Stop music playing (close stream)
void PauseMusicStream(void);                                    // Pause music playing
bool MusicIsPlaying(void);                                      // Check if music is playing
void SetMusicVolume(float volume);                              // Set volume for music (1.0 is max level)
float GetMusicTimeLength(void);                                 // Get current music time length (in seconds)
float GetMusicTimePlayed(void);                                 // Get current music time played (in seconds)

#ifdef __cplusplus
}
#endif

#endif // RAYLIB_H
