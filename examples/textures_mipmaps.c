/*******************************************************************************************
*
*   raylib [textures] example - Texture loading with mipmaps, mipmaps generation
*
*   NOTE: On OpenGL 1.1, mipmaps are calculated 'manually', original image must be power-of-two
*         On OpenGL 3.3 and ES2, mipmaps are generated automatically
*
*   This example has been created using raylib 1.1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - texture mipmaps generation");

    // NOTE: To generate mipmaps for an image, image must be loaded first and converted to texture
    // with mipmaps option set to true on CreateTexture()

    Image image = LoadImage("resources/raylib_logo.png");   // Load image to CPU memory (RAM)
    Texture2D texture = LoadTextureFromImage(image, true);  // Create texture and generate mipmaps

    UnloadImage(image);     // Once texture has been created, we can unload image data from RAM
    //--------------------------------------------------------------------------------------

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

            DrawTexture(texture, screenWidth/2 - texture.width/2,
                        screenHeight/2 - texture.height/2 - 30, WHITE);

            DrawText("this IS a texture with mipmaps! really!", 210, 360, 20, GRAY);

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