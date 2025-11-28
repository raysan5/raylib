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

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                 // Required for GUI controls

#define MAX_FILEPATH_SIZE       1024

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

    char directory[MAX_FILEPATH_SIZE] = { 0 };
    strcpy(directory, GetWorkingDirectory());

    FilePathList files = LoadDirectoryFiles(directory);

    int btnBackPressed = false;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (btnBackPressed)
        {
            TextCopy(directory, GetPrevDirectoryPath(directory));
            UnloadDirectoryFiles(files);
            files = LoadDirectoryFiles(directory);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText(directory, 100, 40, 20, DARKGRAY);

            btnBackPressed = GuiButton((Rectangle){ 40.0f, 38.0f, 48, 24 }, "<");

            for (int i = 0; i < (int)files.count; i++)
            {
                Color color = Fade(LIGHTGRAY, 0.3f);

                if (!IsPathFile(files.paths[i]) && DirectoryExists(files.paths[i]))
                {
                    if (GuiButton((Rectangle){0.0f, 85.0f + 40.0f*(float)i, screenWidth, 40}, ""))
                    {
                        TextCopy(directory, files.paths[i]);
                        UnloadDirectoryFiles(files);
                        files = LoadDirectoryFiles(directory);
                        continue;
                    }
                }

                DrawRectangle(0, 85 + 40*i, screenWidth, 40, color);
                DrawText(GetFileName(files.paths[i]), 120, 100 + 40*i, 10, GRAY);
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
