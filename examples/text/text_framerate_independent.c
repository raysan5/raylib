/*******************************************************************************************
*
*   raylib [core] example - Backspace input
*
*   Example demonstrates deletion of characters in a string independent of frame rate
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example contributed by benevo (@benev0) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 benevo (@benev0)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_INPUT_CHARS 50

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - backspace input");
    SetTargetFPS(1);

    char textField[MAX_INPUT_CHARS + 1] = "\0";
    int textPos = 0;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // get first letter and key from the appropiate queues
        int letter = GetCharPressed();
        int key = GetKeyPressed();

        while (letter || key)
        {
            // if a the backspace key was pressed and there is a character to be deleted
            // then read backspace in from key queue
            if (key == KEY_BACKSPACE && textPos > 0)
            {
                textField[textPos -1] = 0;
                textPos--;
            }

            // if there is a letter and space in the string
            // then read letter in from character queue
            if (letter && textPos < MAX_INPUT_CHARS)
            {
                textField[textPos] = (char) letter;
                textPos++;
            }

            // get next key press and character from queue
            letter = GetCharPressed();
            key = GetKeyPressed();
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Type something:", 40, 20, 20, BLACK);
            DrawText(textField, 40, 60, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
