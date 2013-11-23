/*******************************************************************************************
*
*   raylib example 02c - Draw raylib custom color palette
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
    int screenWidth = 800;
    int screenHeight = 450;
    
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example 02c - raylib color palette");
    //----------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
        // TODO: Update your variables here
        //-----------------------------------------------------
        
        // Draw
        //-----------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawText("raylib color palette", 28, 42, 20, 2, BLACK);
            
            DrawRectangle(26, 80, 100, 100, DARKGRAY);
            DrawRectangle(26, 188, 100, 100, GRAY);
            DrawRectangle(26, 296, 100, 100, LIGHTGRAY);
            DrawRectangle(134, 80, 100, 100, MAROON);
            DrawRectangle(134, 188, 100, 100, RED);
            DrawRectangle(134, 296, 100, 100, PINK);
            DrawRectangle(242, 80, 100, 100, ORANGE);
            DrawRectangle(242, 188, 100, 100, GOLD);
            DrawRectangle(242, 296, 100, 100, YELLOW);
            DrawRectangle(350, 80, 100, 100, DARKGREEN);
            DrawRectangle(350, 188, 100, 100, LIME);
            DrawRectangle(350, 296, 100, 100, GREEN);
            DrawRectangle(458, 80, 100, 100, DARKBLUE);
            DrawRectangle(458, 188, 100, 100, BLUE);
            DrawRectangle(458, 296, 100, 100, SKYBLUE);
            DrawRectangle(566, 80, 100, 100, DARKPURPLE);
            DrawRectangle(566, 188, 100, 100, VIOLET);
            DrawRectangle(566, 296, 100, 100, PURPLE);
            DrawRectangle(674, 80, 100, 100, DARKBROWN);
            DrawRectangle(674, 188, 100, 100, BROWN);
            DrawRectangle(674, 296, 100, 100, BEIGE);

            
            DrawText("DARKGRAY", 57, 166, 10, 2, BLACK);
            DrawText("GRAY", 89, 274, 10, 2, BLACK);
            DrawText("LIGHTGRAY", 51, 382, 10, 2, BLACK);
            DrawText("MAROON", 180, 166, 10, 2, BLACK);
            DrawText("RED", 207, 274, 10, 2, BLACK);
            DrawText("PINK", 200, 382, 10, 2, BLACK);
            DrawText("ORANGE", 290, 166, 10, 2, BLACK);
            DrawText("GOLD", 306, 274, 10, 2, BLACK);
            DrawText("YELLOW", 290, 382, 10, 2, BLACK);
            DrawText("DARKGREEN", 374, 166, 10, 2, BLACK);
            DrawText("LIME", 417, 274, 10, 2, BLACK);
            DrawText("GREEN", 407, 382, 10, 2, BLACK);
            DrawText("DARKBLUE", 491, 166, 10, 2, BLACK);
            DrawText("BLUE", 523, 274, 10, 2, BLACK);
            DrawText("SKYBLUE", 499, 382, 10, 2, BLACK);
            DrawText("DARKPURPLE", 582, 166, 10, 2, BLACK);
            DrawText("VIOLET", 617, 274, 10, 2, BLACK);
            DrawText("PURPLE", 615, 382, 10, 2, BLACK);
            DrawText("DARKBROWN", 695, 166, 10, 2, BLACK);
            DrawText("BROWN", 728, 274, 10, 2, BLACK);
            DrawText("BEIGE", 733, 382, 10, 2, BLACK);
        
        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------
    
    return 0;
}