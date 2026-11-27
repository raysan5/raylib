/*******************************************************************************************
*
*   raylib [shapes] example - easings box
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 2.5, last time updated with raylib 2.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2014-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "reasings.h"            // Required for easing functions

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - easings box");

    // Box variables to be animated with easings
    Rectangle rec = { GetScreenWidth()/2.0f, -100, 100, 100 };
    float rotation = 0.0f;
    float alpha = 1.0f;

    int state = 0;
    int framesCounter = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        switch (state)
        {
            case 0:     // Move box down to center of screen
            {
                framesCounter++;

                // NOTE: Remember that 3rd parameter of easing function refers to
                // desired value variation, do not confuse it with expected final value!
                rec.y = EaseElasticOut((float)framesCounter, -100, GetScreenHeight()/2.0f + 100, 120);

                if (framesCounter >= 120)
                {
                    framesCounter = 0;
                    state = 1;
                }
            } break;
            case 1:     // Scale box to an horizontal bar
            {
                framesCounter++;
                rec.height = EaseBounceOut((float)framesCounter, 100, -90, 120);
                rec.width = EaseBounceOut((float)framesCounter, 100, (float)GetScreenWidth(), 120);

                if (framesCounter >= 120)
                {
                    framesCounter = 0;
                    state = 2;
                }
            } break;
            case 2:     // Rotate horizontal bar rectangle
            {
                framesCounter++;
                rotation = EaseQuadOut((float)framesCounter, 0.0f, 270.0f, 240);

                if (framesCounter >= 240)
                {
                    framesCounter = 0;
                    state = 3;
                }
            } break;
            case 3:     // Increase bar size to fill all screen
            {
                framesCounter++;
                rec.height = EaseCircOut((float)framesCounter, 10, (float)GetScreenWidth(), 120);

                if (framesCounter >= 120)
                {
                    framesCounter = 0;
                    state = 4;
                }
            } break;
            case 4:     // Fade out animation
            {
                framesCounter++;
                alpha = EaseSineOut((float)framesCounter, 1.0f, -1.0f, 160);

                if (framesCounter >= 160)
                {
                    framesCounter = 0;
                    state = 5;
                }
            } break;
            default: break;
        }

        // Reset animation at any moment
        if (IsKeyPressed(KEY_SPACE))
        {
            rec = (Rectangle){ GetScreenWidth()/2.0f, -100, 100, 100 };
            rotation = 0.0f;
            alpha = 1.0f;
            state = 0;
            framesCounter = 0;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawRectanglePro(rec, (Vector2){ rec.width/2, rec.height/2 }, rotation, Fade(BLACK, alpha));

            DrawText("PRESS [SPACE] TO RESET BOX ANIMATION!", 10, GetScreenHeight() - 25, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}