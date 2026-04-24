/*******************************************************************************************
*
*   raylib [shapes] example - moving circle
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   This example demonstrates basic animation by moving a circle horizontally.
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - moving circle");

    float x = 100.0f;
    float speed = 200.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        // Move circle based on frame time (smooth movement)
        x += speed * GetFrameTime();

        if (x > screenWidth - 50 || x < 50) speed *= -1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

    	    // Title
    	    DrawText("Moving Circle Example", 10, 10, 20, DARKGRAY);

    	    // Subtitle
    	    DrawText("Circle moves horizontally using frame time", 10, 40, 10, GRAY);

    	    // FPS (separate, non-overlapping)
            DrawFPS(10, 70);

            // Circle (main visual focus)
            DrawCircle((int)x, screenHeight/2, 50, BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
