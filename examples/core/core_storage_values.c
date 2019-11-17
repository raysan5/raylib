/*******************************************************************************************
*
*   raylib [core] example - Storage save/load values
*
*   This example has been created using raylib 1.4 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

// NOTE: Storage positions must start with 0, directly related to file memory layout
typedef enum { STORAGE_SCORE = 0, STORAGE_HISCORE } StorageData;

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - storage save/load values");

    int score = 0;
    int hiscore = 0;
    int framesCounter = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_R))
        {
            score = GetRandomValue(1000, 2000);
            hiscore = GetRandomValue(2000, 4000);
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            StorageSaveValue(STORAGE_SCORE, score);
            StorageSaveValue(STORAGE_HISCORE, hiscore);
        }
        else if (IsKeyPressed(KEY_SPACE))
        {
            // NOTE: If requested position could not be found, value 0 is returned
            score = StorageLoadValue(STORAGE_SCORE);
            hiscore = StorageLoadValue(STORAGE_HISCORE);
        }

        framesCounter++;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText(FormatText("SCORE: %i", score), 280, 130, 40, MAROON);
            DrawText(FormatText("HI-SCORE: %i", hiscore), 210, 200, 50, BLACK);

            DrawText(FormatText("frames: %i", framesCounter), 10, 10, 20, LIME);

            DrawText("Press R to generate random numbers", 220, 40, 20, LIGHTGRAY);
            DrawText("Press ENTER to SAVE values", 250, 310, 20, LIGHTGRAY);
            DrawText("Press SPACE to LOAD values", 252, 350, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}