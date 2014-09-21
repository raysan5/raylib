/*******************************************************************************************
*
*   raylib example 03a - Keyboard input 
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{    
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    Vector2 ballPosition = { screenWidth/2, screenHeight/2 };
    
    InitWindow(screenWidth, screenHeight, "raylib example 05 - keyboard input");
    
    SetTargetFPS(60);       // Set target frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 0.8;
        if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 0.8;
        if (IsKeyDown(KEY_UP)) ballPosition.y -= 0.8;
        if (IsKeyDown(KEY_DOWN)) ballPosition.y += 0.8;
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);
            
            DrawCircleV(ballPosition, 50, MAROON);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}
