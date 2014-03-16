/*******************************************************************************************
*
*   raylib test - Testing FormatText() function
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
    
    int score = 100020;
    int hiscore = 200450;
    int lives = 5;
    
    InitWindow(screenWidth, screenHeight, "raylib test - FormatText()");
    
    SetTargetFPS(60);
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
            
            DrawText(FormatText("Score: %08i", score), 80, 80, 20, RED);
        
            DrawText(FormatText("HiScore: %08i", hiscore), 80, 120, 20, GREEN);
            
            DrawText(FormatText("Lives: %02i", lives), 80, 160, 40, BLUE);
            
            DrawText(FormatText("Elapsed Time: %02.02f ms", GetFrameTime()*1000), 80, 220, 20, BLACK);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}