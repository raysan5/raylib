/*******************************************************************************************
*
*   raylib [core] example - Retrieve image data from texture: GetTextureData()  (adapted for HTML5 platform)
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

// NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

Image image;
Texture2D texture;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture to image");
    
    image = LoadImage("resources/raylib_logo.png");  // Load image data into CPU memory (RAM)
    texture = LoadTextureFromImage(image);           // Image converted to texture, GPU memory (RAM -> VRAM)
    UnloadImage(image);                              // Unload image data from CPU memory (RAM)
    
    image = GetTextureData(texture);                 // Retrieve image data from GPU memory (VRAM -> RAM)
    UnloadTexture(texture);                          // Unload texture from GPU memory (VRAM)
    
    texture = LoadTextureFromImage(image);           // Recreate texture from retrieved image data (RAM -> VRAM)
    UnloadImage(image);                              // Unload retrieved image data from CPU memory (RAM)
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);

        DrawText("this IS a texture loaded from an image!", 300, 370, 10, GRAY);


    EndDrawing();
    //----------------------------------------------------------------------------------
}