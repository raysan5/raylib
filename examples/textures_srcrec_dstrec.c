/*******************************************************************************************
*
*   raylib [textures] example - Texture source and destination rectangles
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

    InitWindow(screenWidth, screenHeight, "raylib [textures] examples - texture source and destination rectangles");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture2D texture = LoadTexture("resources/raylib_logo.png");        // Texture loading

    // NOTE: Source rectangle (part of the texture to use for drawing)    
    Rectangle sourceRec = { 128, 128, 128, 128 };

    // NOTE: Destination rectangle (screen rectangle where drawing part of texture)
    Rectangle destRec = { screenWidth/2, screenHeight/2, 256, 256 };

    // NOTE: Origin of the texture in case of rotation, it's relative to destination rectangle size
    Vector2 origin = { 128, 128 };
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
            
            // NOTE: Using DrawTexturePro() we can easily rotate and scale the part of the texture we draw
            DrawTexturePro(texture, sourceRec, destRec, origin, 45, LIGHTGRAY);
            
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