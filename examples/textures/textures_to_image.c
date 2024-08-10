/*******************************************************************************************
*
*   raylib [textures] example - Retrieve image data from texture: LoadImageFromTexture()
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example originally created with raylib 1.3, last time updated with raylib 4.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture to image");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

    Image image = LoadImage("resources/raylib_logo.png");  // Load image data into CPU memory (RAM)
    Texture2D texture = LoadTextureFromImage(image);       // Image converted to texture, GPU memory (RAM -> VRAM)
    UnloadImage(image);                                    // Unload image data from CPU memory (RAM)

    image = LoadImageFromTexture(texture);                 // Load image from GPU texture (VRAM -> RAM)
    UnloadTexture(texture);                                // Unload texture from GPU memory (VRAM)

    texture = LoadTextureFromImage(image);                 // Recreate texture from retrieved image data (RAM -> VRAM)
    UnloadImage(image);                                    // Unload retrieved image data from CPU memory (RAM)

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
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

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}