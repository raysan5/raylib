/*******************************************************************************************
*
*   raylib example 06a - Color selection by mouse (collision detection)
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
    int screenHeight = 400;
    
    Color colors[21] = { DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
                         GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
                         GREEN, SKYBLUE, PURPLE, BEIGE };
    
    Rectangle recs[21];             // Rectangles array
    
    // Fills recs data (for every rectangle)
    for (int i = 0; i < 21; i++)
    {
        recs[i].x = 20 + 100*(i%7) + 10*(i%7);
        recs[i].y = 40 + 100*(i/7) + 10*(i/7);
        recs[i].width = 100;
        recs[i].height = 100;
    }
    
    bool selected[21] = { false };   // Selected rectangles indicator
    
    Vector2 mousePoint;
    
    InitWindow(screenWidth, screenHeight, "raylib example 06a - color selection");
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        mousePoint = GetMousePosition();
        
        for (int i = 0; i < 21; i++)    // Iterate along all the rectangles
        {
            if (CheckCollisionPointRec(mousePoint, recs[i]))
            {   
                colors[i].a = 120;
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selected[i] = !selected[i];
            }
            else colors[i].a = 255;
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            for (int i = 0; i < 21; i++)    // Draw all rectangles
            {
                DrawRectangleRec(recs[i], colors[i]);
                
                // Draw four rectangles around selected rectangle
                if (selected[i])
                {
                    DrawRectangle(recs[i].x, recs[i].y, 100, 10, RAYWHITE);        // Square top rectangle
                    DrawRectangle(recs[i].x, recs[i].y, 10, 100, RAYWHITE);        // Square left rectangle
                    DrawRectangle(recs[i].x + 90, recs[i].y, 10, 100, RAYWHITE);   // Square right rectangle
                    DrawRectangle(recs[i].x, recs[i].y + 90, 100, 10, RAYWHITE);   // Square bottom rectangle
                }
            }
       
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------  
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}