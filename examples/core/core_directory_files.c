/*******************************************************************************************
*
*   raylib [core] example - directory files
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Hugo ARNAL (@hugoarnal) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Hugo ARNAL (@hugoarnal)
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - directory files");

    char *directory = ".";
    FilePathList files = LoadDirectoryFiles(directory);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText(TextFormat("Files in directory \"%s\":", directory), 100, 40, 20, DARKGRAY);

            for (int i = 0; i < (int)files.count; i++)
            {
                float alpha = (i % 2 == 0)? 0.5f : 0.3f;
                DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, alpha));

                DrawText(files.paths[i], 120, 100 + 40*i, 10, GRAY);
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadDirectoryFiles(files);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
