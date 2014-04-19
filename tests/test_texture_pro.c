/*******************************************************************************************
*
*   raylib test - Texture loading and drawing with pro parameters (rotation, origin, scale...)
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

    InitWindow(screenWidth, screenHeight, "raylib test - texture pro");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture2D texture = LoadTexture("resources/raylib_logo.png");        // Texture loading
    
    Vector2 position = { 200, 100 };
    
    Rectangle sourceRec = { 128, 128, 128, 128 };
    Rectangle destRec = { 128, 128, 128, 128 };
    Vector2 origin = { 64, 64 };    // NOTE: origin is relative to destRec size
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
            
            //DrawTextureEx(texture, position, 45, 1, MAROON);
            
            DrawTexturePro(texture, sourceRec, destRec, origin, 45, GREEN);
            
            DrawLine(destRec.x, 0, destRec.x, screenHeight, RED);
            DrawLine(0, destRec.y, screenWidth, destRec.y, RED);
       
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