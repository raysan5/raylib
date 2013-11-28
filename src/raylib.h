/*********************************************************************************************
* 
*   raylib 1.0.0 (www.raylib.com)
*    
*   A simple and easy-to-use library to learn C videogames programming
*
*   Features:
*     Library written in plain C code (C99)
*     Uses C# PascalCase/camelCase notation
*     Hardware accelerated with OpenGL 1.1
*     Powerful fonts module with SpriteFonts support
*     Basic 3d support for Shapes and Models
*     Audio loading and playing
*    
*   Used external libs:
*     GLFW3 (www.glfw.org) for window/context management and input
*     stb_image (Sean Barret) for images loading (JPEG, PNG, BMP, TGA, PSD, GIF, HDR, PIC)
*     OpenAL Soft for audio device/context management
*
*   Some design decisions:
*     32bit Colors - All defined color are always RGBA
*     32bit Textures - All loaded images are converted automatically to RGBA textures
*     SpriteFonts - All loaded sprite-font images are converted to RGBA and POT textures
*     One custom default font is loaded automatically when InitWindow()
* 
*   -- LICENSE (raylib v1.0, November 2013) --
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

#define NO_AUDIO    // Audio is still being tested, deactivated by default

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
    unsigned int glId;
    int width;
    int height;
} Texture2D;

// SpriteFont one Character (Glyph) data, defined in text module
typedef struct Character Character;

// SpriteFont type, includes texture and charSet array data
typedef struct SpriteFont {
    Texture2D texture;
    int numChars;
    Character *charSet;
} SpriteFont;

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

// Camera type, defines a camera position/orientation in 3d space
typedef struct Camera {
    Vector3 position;
    Vector3 target;
    Vector3 up;
} Camera;

// Basic 3d Model type
typedef struct Model {
    int numVertices;
    Vector3 *vertices;
    Vector2 *texcoords;
    Vector3 *normals;
} Model;

// Basic Sound source and buffer
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
void InitWindow(int width, int height, char* title);    // Initialize Window and Graphics Context (OpenGL)
void CloseWindow();                                     // Close Window and Terminate Context
bool WindowShouldClose();                               // Detect if KEY_ESCAPE pressed or Close icon pressed
void ToggleFullscreen();                                // Fullscreen toggle (by default F11)

void ClearBackground(Color color);                      // Sets Background Color
void BeginDrawing();                                    // Setup drawing canvas to start drawing
void EndDrawing();                                      // End canvas drawing and Swap Buffers (Double Buffering)

void Begin3dMode(Camera cam);                           // Initializes 3D mode for drawing (Camera setup)
void End3dMode();                                       // Ends 3D mode and returns to default 2D orthographic mode

void SetTargetFPS(int fps);                             // Set target FPS (maximum)
float GetFPS();                                         // Returns current FPS
float GetFrameTime();                                   // Returns time in seconds for one frame

Color GetColor(int hexValue);                           // Returns a Color struct from hexadecimal value
int GetHexValue(Color color);                           // Returns hexadecimal value for a Color

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
int GetMouseX();                                        // Returns mouse position X
int GetMouseY();                                        // Returns mouse position Y
Vector2 GetMousePosition();                             // Returns mouse position XY

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
void DrawPoly(Vector2 *points, int numPoints, Color color);                                        // Draw a closed polygon defined by points
void DrawPolyLine(Vector2 *points, int numPoints, Color color);                                    // Draw polygon lines

//------------------------------------------------------------------------------------
// Texture Loading and Drawing Functions (Module: textures)
//------------------------------------------------------------------------------------
Image LoadImage(const char *fileName);                                                             // Load an image into CPU memory (RAM)
void UnloadImage(Image image);                                                                     // Unload image from CPU memory (RAM)
Texture2D LoadTexture(const char *fileName);                                                       // Load an image as texture into GPU memory
//Texture2D LoadTextureEx(const char *fileName, bool createPOT, bool mipmaps);                     // Load an image as texture (and convert to POT with mipmaps) (raylib 1.x)
void UnloadTexture(Texture2D texture);                                                             // Unload texture from GPU memory
void DrawTexture(Texture2D texture, int posX, int posY, Color tint);                               // Draw a Texture2D
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);  // Draw a Texture2D with extended parameters
void DrawTextureRec(Texture2D texture, Rectangle sourceRec, Vector2 position, Color tint);         // Draw a part of a texture defined by a rectangle
void DrawTexturePro(Texture2D texture, Rectangle sourceRec, Rectangle destRec, Vector2 origin, float rotation, Color tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters

//------------------------------------------------------------------------------------
// Font Loading and Text Drawing Functions (Module: text)
//------------------------------------------------------------------------------------
SpriteFont LoadSpriteFont(const char *fileName);                                                   // Load a SpriteFont image into GPU memory
void UnloadSpriteFont(SpriteFont spriteFont);                                                      // Unload SpriteFont from GPU memory
void DrawText(const char *text, int posX, int posY, int fontSize, Color color);                    // Draw text (using default font)
void DrawTextEx(SpriteFont spriteFont, const char* text, Vector2 position, int fontSize, int spacing, Color tint); // Draw text using SpriteFont
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
void DrawSphere(Vector3 centerPos, float radius, Color color);                                     // Draw sphere
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);            // Draw sphere with extended parameters
void DrawSphereWires(Vector3 centerPos, float radius, Color color);                                // Draw sphere wires
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color); // Draw a cylinder/cone wires
void DrawPlane(Vector3 centerPos, Vector2 size, Vector3 rotation, Color color);                    // Draw a plane
void DrawPlaneEx(Vector3 centerPos, Vector2 size, Vector3 rotation, int slicesX, int slicesZ, Color color); // Draw a plane with divisions
void DrawGrid(int slices, float spacing);                                                          // Draw a grid (centered at (0, 0, 0))
void DrawGizmo(Vector3 position, bool orbits);                                                     // Draw gizmo (with or without orbits)
//DrawTorus(), DrawTeapot() are useless...

//------------------------------------------------------------------------------------
// Model 3d Loading and Drawing Functions (Module: models)
//------------------------------------------------------------------------------------
Model LoadModel(const char *fileName);                                                             // Load a 3d model (.OBJ)
void UnloadModel(Model model);                                                                     // Unload 3d model from memory
void DrawModel(Model model, Vector3 position, float scale, Color color);                           // Draw a model
void DrawModelEx(Model model, Texture2D texture, Vector3 position, float scale, Color tint);       // Draw a textured model
void DrawModelWires(Model model, Vector3 position, float scale, Color color);                      // Draw a model wires

// NOTE: The following functions work but are incomplete or require some revision
// DrawHeightmap is extremely inefficient and can impact performance up to 60%
void DrawBillboard(Camera camera, Texture2D texture, Vector3 basePos, float size, Color tint);                         // REVIEW: Draw a billboard (raylib 1.x)
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle sourceRec, Vector3 basePos, float size, Color tint); // REVIEW: Draw a billboard (raylib 1.x)
void DrawHeightmap(Image heightmap, Vector3 centerPos, Vector3 scale, Color color);                                    // REVIEW: Draw heightmap using image map (raylib 1.x)
void DrawHeightmapEx(Image heightmap, Texture2D texture, Vector3 centerPos, Vector3 scale, Color tint);                // REVIEW: Draw textured heightmap (raylib 1.x)

#ifndef NO_AUDIO

//------------------------------------------------------------------------------------
// Audio Loading and Playing Functions (Module: audio)
//------------------------------------------------------------------------------------
void InitAudioDevice();                                         // Initialize audio device and context
void CloseAudioDevice();                                        // Close the audio device and context
Sound LoadSound(char *fileName);                                // Load sound to memory
void UnloadSound(Sound sound);                                  // Unload sound
void PlaySound(Sound sound);                                    // Play a sound
void PlaySoundEx(Sound sound, float timePosition, bool loop);   // Play a sound with extended parameters
void PauseSound(Sound sound);                                   // Pause a sound
void StopSound(Sound sound);                                    // Stop playing a sound

#endif    // NO_AUDIO

#ifdef __cplusplus
}
#endif

#endif // RAYLIB_H
