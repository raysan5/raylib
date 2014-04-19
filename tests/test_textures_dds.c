/*******************************************************************************************
*
*   raylib test - DDS Texture loading and drawing (compressed and uncompressed)
*
*   This test has been created using raylib 1.1 (www.raylib.com)
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

    InitWindow(screenWidth, screenHeight, "raylib test - DDS texture loading and drawing");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    //Texture2D texture = LoadTexture("resources/raylib_logo.dds");               // Texture loading
    //Texture2D texture = LoadTexture("resources/raylib_logo_uncompressed.dds");  // Texture loading
    
    Image image = LoadImage("resources/raylib_logo_uncompressed.dds");
    Texture2D texture = CreateTexture(image, false);
    
    // NOTE: With OpenGL 3.3 mipmaps generation works great
    
    SetTargetFPS(60);
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
            
            DrawText("this IS a texture!", 360, 370, 10, GRAY);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    //UnloadTexture(texture);       // Texture unloading
    
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}