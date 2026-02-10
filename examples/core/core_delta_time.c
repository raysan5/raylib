/*******************************************************************************************
*
*   raylib [core] example - delta time
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6-dev
*
*   Example contributed by Robin (@RobinsAviary) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Robin (@RobinsAviary)
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - delta time");

    int currentFps = 60;

    // Store the position for the both of the circles
    Vector2 deltaCircle = { 0, (float)screenHeight/3.0f };
    Vector2 frameCircle = { 0, (float)screenHeight*(2.0f/3.0f) };

    // The speed applied to both circles
    const float speed = 10.0f;
    const float circleRadius = 32.0f;

    SetTargetFPS(currentFps);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Adjust the FPS target based on the mouse wheel
        float mouseWheel = GetMouseWheelMove();
        if (mouseWheel != 0)
        {
            currentFps += (int)mouseWheel;
            if (currentFps < 0) currentFps = 0;
            SetTargetFPS(currentFps);
        }

        // GetFrameTime() returns the time it took to draw the last frame, in seconds (usually called delta time)
        // Uses the delta time to make the circle look like it's moving at a "consistent" speed regardless of FPS

        // Multiply by 6.0 (an arbitrary value) in order to make the speed
        // visually closer to the other circle (at 60 fps), for comparison
        deltaCircle.x += GetFrameTime()*6.0f*speed;
        // This circle can move faster or slower visually depending on the FPS
        frameCircle.x += 0.1f*speed;

        // If either circle is off the screen, reset it back to the start
        if (deltaCircle.x > screenWidth) deltaCircle.x = 0;
        if (frameCircle.x > screenWidth) frameCircle.x = 0;

        // Reset both circles positions
        if (IsKeyPressed(KEY_R))
        {
            deltaCircle.x = 0;
            frameCircle.x = 0;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw both circles to the screen
            DrawCircleV(deltaCircle, circleRadius, RED);
            DrawCircleV(frameCircle, circleRadius, BLUE);

            // Draw the help text
            // Determine what help text to show depending on the current FPS target
            const char *fpsText = 0;
            if (currentFps <= 0) fpsText = TextFormat("FPS: unlimited (%i)", GetFPS());
            else fpsText = TextFormat("FPS: %i (target: %i)", GetFPS(), currentFps);
            DrawText(fpsText, 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Frame time: %02.02f ms", GetFrameTime()), 10, 30, 20, DARKGRAY);
            DrawText("Use the scroll wheel to change the fps limit, r to reset", 10, 50, 20, DARKGRAY);

            // Draw the text above the circles
            DrawText("FUNC: x += GetFrameTime()*speed", 10, 90, 20, RED);
            DrawText("FUNC: x += speed", 10, 240, 20, BLUE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}