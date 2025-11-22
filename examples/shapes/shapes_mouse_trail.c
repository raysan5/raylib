/*******************************************************************************************
*
* raylib [shapes] example - Draw a mouse trail (position history)
*
* Example complexity rating: [★☆☆☆] 1/4
*
* Example originally created with raylib 5.6
*
* Example contributed by Balamurugan R (@Bala050814]) and reviewed by Ramon Santamaria (@raysan5)
*
* Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
* BSD-like license that allows static linking with closed source software
*
* Copyright (c) 2025 Balamurugan R (@Bala050814)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

// Define the maximum number of positions to store in the trail
#define MAX_TRAIL_LENGTH 30

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - mouse trail");

    // Array to store the history of mouse positions (our fixed-size queue)
    Vector2 trailPositions[MAX_TRAIL_LENGTH] = { 0 };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePosition = GetMousePosition();

        // Shift all existing positions backward by one slot in the array
        // The last element (the oldest position) is dropped
        for (int i = MAX_TRAIL_LENGTH - 1; i > 0; i--)
        {
            trailPositions[i] = trailPositions[i - 1];
        }

        // Store the new, current mouse position at the start of the array (Index 0)
        trailPositions[0] = mousePosition;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw the trail by looping through the history array
            for (int i = 0; i < MAX_TRAIL_LENGTH; i++)
            {
                // Ensure we skip drawing if the array hasn't been fully filled on startup
                if ((trailPositions[i].x != 0.0f) || (trailPositions[i].y != 0.0f))
                {
                    // Calculate relative trail strength (ratio is near 1.0 for new, near 0.0 for old)
                    float ratio = (float)(MAX_TRAIL_LENGTH - i)/MAX_TRAIL_LENGTH;

                    // Fade effect: oldest positions are more transparent
                    // Fade (color, alpha) - alpha is 0.5 to 1.0 based on ratio
                    Color trailColor = Fade(SKYBLUE, ratio*0.5f + 0.5f);

                    // Size effect: oldest positions are smaller
                    float trailRadius = 15.0f*ratio;

                    DrawCircleV(trailPositions[i], trailRadius, trailColor);
                }
            }

            // Draw a distinct white circle for the current mouse position (Index 0)
            DrawCircleV(mousePosition, 15.0f, WHITE);

            DrawText("Move the mouse to see the trail effect!", 10, screenHeight - 30, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();         // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}