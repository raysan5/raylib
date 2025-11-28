/*******************************************************************************************
*
*   raylib [shapes] example - starfield effect
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6-dev
*
*   Example contributed by JP Mortiboys (@themushroompirates) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 JP Mortiboys (@themushroompirates)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"    // Required for: Lerp()

#define STAR_COUNT 420

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - starfield effect");

    Color bgColor = ColorLerp(DARKBLUE, BLACK, 0.69f);

    // Speed at which we fly forward
    float speed = 10.0f/9.0f;

    // We're either drawing lines or circles
    bool drawLines = true;

    Vector3 stars[STAR_COUNT] = { 0 };
    Vector2 starsScreenPos[STAR_COUNT] = { 0 };

    // Setup the stars with a random position
    for (int i = 0; i < STAR_COUNT; i++)
    {
        stars[i].x = GetRandomValue(-screenWidth*0.5f, screenWidth*0.5f);
        stars[i].y = GetRandomValue(-screenHeight*0.5f, screenHeight*0.5f);
        stars[i].z = 1.0f;
    }

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Change speed based on mouse
        float mouseMove = GetMouseWheelMove();
        if ((int)mouseMove != 0) speed += 2.0f*mouseMove/9.0f;
        if (speed < 0.0f) speed = 0.1f;
        else if (speed > 2.0f) speed = 2.0f;

        // Toggle lines / points with space bar
        if (IsKeyPressed(KEY_SPACE)) drawLines = !drawLines;

        float dt = GetFrameTime();
        for (int i = 0; i < STAR_COUNT; i++)
        {
            // Update star's timer
            stars[i].z -= dt*speed;

            // Calculate the screen position
            starsScreenPos[i] = (Vector2){
                screenWidth*0.5f + stars[i].x/stars[i].z,
                screenHeight*0.5f + stars[i].y/stars[i].z,
            };

            // If the star is too old, or offscreen, it dies and we make a new random one
            if ((stars[i].z < 0.0f) || (starsScreenPos[i].x < 0) || (starsScreenPos[i].y < 0.0f) ||
                (starsScreenPos[i].x > screenWidth) || (starsScreenPos[i].y > screenHeight))
            {
                stars[i].x = GetRandomValue(-screenWidth*0.5f, screenWidth*0.5f);
                stars[i].y = GetRandomValue(-screenHeight*0.5f, screenHeight*0.5f);
                stars[i].z = 1.0f;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(bgColor);

            for (int i = 0; i < STAR_COUNT; i++)
            {
                if (drawLines)
                {
                    // Get the time a little while ago for this star, but clamp it
                    float t = Clamp(stars[i].z + 1.0f/32.0f, 0.0f, 1.0f);

                    // If it's different enough from the current time, we proceed
                    if ((t - stars[i].z) > 1e-3)
                    {
                        // Calculate the screen position of the old point
                        Vector2 startPos = (Vector2){
                            screenWidth*0.5f + stars[i].x/t,
                            screenHeight*0.5f + stars[i].y/t,
                        };

                        // Draw a line connecting the old point to the current point
                        DrawLineV(startPos, starsScreenPos[i], RAYWHITE);
                    }
                }
                else
                {
                    // Make the radius grow as the star ages
                    float radius = Lerp(stars[i].z, 1.0f, 5.0f);

                    // Draw the circle
                    DrawCircleV(starsScreenPos[i], radius, RAYWHITE);
                }
            }

            DrawText(TextFormat("[MOUSE WHEEL] Current Speed: %.0f", 9.0f*speed/2.0f), 10, 40, 20, RAYWHITE);
            DrawText(TextFormat("[SPACE] Current draw mode: %s", drawLines ? "Lines" : "Circles"), 10, 70, 20, RAYWHITE);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}