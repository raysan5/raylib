/*
4. Code should follow raylib conventions: https://github.com/raysan5/raylib/wiki/raylib-coding-conventions
Try to be very organized, using line-breaks appropiately

/*******************************************************************************************
*
*   raylib [core] example - delta time
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5
*
*   Example contributed by Robin (@RobinsAviary) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Robin (@RobinsAviary)
*
********************************************************************************************/

#include "raylib.h"

int currentFps = 60;

// Used only in this example in order to allow finer control of the FPS limit.
void UpdateFPS(int fps)
{
    currentFps = fps;
    SetTargetFPS(currentFps);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - delta time");

    // The x positions for each circle.
    float deltaX = 0;
    float frameX = 0;

    // The speed applied to both circles.
    const float speed = 10.0;
    const int circleRadius = 32;

    // Calculate the visual Y position for both circles.
    const float deltaY = screenHeight / 3.0;
    const float frameY = screenHeight * (2.0/3.0);

    SetTargetFPS(currentFps);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update

        // Update the target FPS based on number key.
        if (IsKeyPressed(KEY_ONE)) UpdateFPS(10);
        if (IsKeyPressed(KEY_TWO)) UpdateFPS(20);
        if (IsKeyPressed(KEY_THREE)) UpdateFPS(30);
        if (IsKeyPressed(KEY_FOUR)) UpdateFPS(40);
        if (IsKeyPressed(KEY_FIVE)) UpdateFPS(50);
        if (IsKeyPressed(KEY_SIX)) UpdateFPS(60);
        if (IsKeyPressed(KEY_SEVEN)) UpdateFPS(70);
        if (IsKeyPressed(KEY_EIGHT)) UpdateFPS(80);
        if (IsKeyPressed(KEY_NINE)) UpdateFPS(90);
        if (IsKeyPressed(KEY_ZERO)) UpdateFPS(0); // Unlimited framerate

        if (IsKeyPressed(KEY_R)) // Reset both circles' positions when you press R.
        {
            deltaX = 0;
            frameX = 0;
        }

        // Adjust the FPS target based on the mouse wheel.
        int mouseWheel = GetMouseWheelMove();
        if (mouseWheel != 0)
        {
            currentFps += mouseWheel;
            SetTargetFPS(currentFps);
        }

        // Use of delta time to make the circle look like it's moving at a "consistent" speed regardless of FPS.
        // Multiply by 6.0 (an arbitrary value) in order to make the speed visually closer to the other circle (at 60 fps), for comparison.
        deltaX += GetFrameTime() * 6.0 * speed;
        // This circle can move faster or slower visually depending on the FPS.
        frameX += .1 * speed;

        // If either circle is off the screen, reset it back to the start.
        if (deltaX > screenWidth)
        {
            deltaX = 0;
        }

        if (frameX > screenWidth)
        {
            frameX = 0;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw both circles to the screen.
        DrawCircle(deltaX, deltaY, circleRadius, RED);
        DrawCircle(frameX, frameY, circleRadius, BLUE);

        // Determine what help text to show depending on the current FPS target.
        char* fpsText;

        if (currentFps <= 0)
        {
            if (currentFps < 0)
            {
                // Clamp values below 0.
                currentFps = 0;
            }

            // Special text for when the FPS target is set to 0 or less, which makes it unlimited.
            fpsText = TextFormat("fps: unlimited (%i)", GetFPS());
        }
        else {
            fpsText = TextFormat("fps: %i", GetFPS());
        }

        // Draw the help text
        DrawText(fpsText, 10, 10, 20, DARKGRAY);
        DrawText(TextFormat("frame time: %02.02f ms", GetFrameTime()), 10, 30, 20, DARKGRAY);
        DrawText("use the scroll wheel/number keys to change the fps limit, r to reset", 10, 50, 20, DARKGRAY);

        // Draw the text above the circles.
        DrawText("x += GetFrameTime() * speed", 10, 90, 20, RED);
        DrawText("x += speed", 10, 240, 20, BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}