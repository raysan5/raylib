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

#define MAX_INPUT_CHARS 40


bool isKeyChar(int);

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

    char textField[MAX_INPUT_CHARS + 1] = "\0";
    int usedSize = 0;

    SetTargetFPS(1);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        int letter = GetCharPressed();
        int key = GetKeyPressed();

        while (key > 0 || letter > 0)
        {

            // ingest (key) backspace
            // if posable delete one character from the string
            if (key == KEY_BACKSPACE && usedSize > 0)
            {
                textField[usedSize - 1] = 0;
                usedSize--;
                key = GetKeyPressed();
            }

            // ingest (key) non character
            else if (!isKeyChar(key))
            {
                key = GetKeyPressed();
            }

            // ingest (letter) and (key)
            // if possible wright ascii character to the string
            else if ((letter > 0) && (letter >= 32) && (letter <= 125) && (usedSize < MAX_INPUT_CHARS))
            {
                textField[usedSize] = (char) letter;
                textField[usedSize + 1] = 0;
                usedSize++;
                letter = GetCharPressed();
                key = GetKeyPressed();
            }

            // ingest (letter) and (key)
            else
            {
                letter = GetCharPressed();
                key = GetKeyPressed();
            }
        }


        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // TODO: Draw everything that requires to be drawn at this point:

            DrawText(textField, 40, 200, 20, BLACK);  // Example

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

bool isKeyChar(int key)
{
    switch (key)
    {
    case KEY_SPACE:
    case KEY_APOSTROPHE:
    case KEY_COMMA...KEY_NINE:
    case KEY_SEMICOLON:
    case KEY_EQUAL:
    case KEY_A...KEY_Z:
    case KEY_LEFT_BRACKET...KEY_RIGHT_BRACKET:
    case KEY_GRAVE:
        return true;
    default:
        return false;
    }
}
