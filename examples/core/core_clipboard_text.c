/*******************************************************************************************
*
*   raylib [core] example - clipboard text
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Ananth S (@Ananth1839) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Ananth S (@Ananth1839)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define MAX_TEXT_SAMPLES    5

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - clipboard text");

    // Define some sample texts
    const char *sampleTexts[MAX_TEXT_SAMPLES] = {
        "Hello from raylib!",
        "The quick brown fox jumps over the lazy dog",
        "Clipboard operations are useful!",
        "raylib is a simple and easy-to-use library",
        "Copy and paste me!"
    };

    const char *clipboardText = NULL;
    char inputBuffer[256] = "Hello from raylib!"; // Random initial string

    // UI required variables
    bool textBoxEditMode = false;

    bool btnCutPressed = false;
    bool btnCopyPressed = false;
    bool btnPastePressed = false;
    bool btnClearPressed = false;
    bool btnRandomPressed = false;

    // Set UI style
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
    GuiSetIconScale(2);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Handle button interactions
        if (btnCutPressed)
        {
            SetClipboardText(inputBuffer);
            clipboardText = GetClipboardText();
            inputBuffer[0] = '\0'; // Quick solution to clear text
            //memset(inputBuffer, 0, 256); // Clear full buffer properly
        }

        if (btnCopyPressed)
        {
            SetClipboardText(inputBuffer); // Copy text to clipboard
            clipboardText = GetClipboardText(); // Get text from clipboard
        }

        if (btnPastePressed)
        {
            // Paste text from clipboard
            clipboardText = GetClipboardText();
            if (clipboardText != NULL) TextCopy(inputBuffer, clipboardText);
        }

        if (btnClearPressed)
        {
            inputBuffer[0] = '\0'; // Quick solution to clear text
            //memset(inputBuffer, 0, 256); // Clear full buffer properly
        }

        if (btnRandomPressed)
        {
            // Get random text from sample list
            TextCopy(inputBuffer, sampleTexts[GetRandomValue(0, MAX_TEXT_SAMPLES - 1)]);
        }

        // Quick cut/copy/paste with keyboard shortcuts
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        {
            if (IsKeyPressed(KEY_X))
            {
                SetClipboardText(inputBuffer);
                inputBuffer[0] = '\0'; // Quick solution to clear text
            }

            if (IsKeyPressed(KEY_C)) SetClipboardText(inputBuffer);

            if (IsKeyPressed(KEY_V))
            {
                clipboardText = GetClipboardText();
                if (clipboardText != NULL) TextCopy(inputBuffer, clipboardText);
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw instructions
        GuiLabel((Rectangle){ 50, 20, 700, 36 }, "Use the BUTTONS or KEY SHORTCUTS:");
        DrawText("[CTRL+X] - CUT | [CTRL+C] COPY | [CTRL+V] | PASTE", 50, 60, 20, MAROON);

        // Draw text box
        if (GuiTextBox((Rectangle){ 50, 120, 652, 40 }, inputBuffer, 256, textBoxEditMode)) textBoxEditMode = !textBoxEditMode;

        // Random text button
        btnRandomPressed = GuiButton((Rectangle){ 50 + 652 + 8, 120, 40, 40 }, "#77#");

        // Draw buttons
        btnCutPressed = GuiButton((Rectangle){ 50, 180, 158, 40 }, "#17#CUT");
        btnCopyPressed = GuiButton((Rectangle){ 50 + 165, 180, 158, 40 }, "#16#COPY");
        btnPastePressed = GuiButton((Rectangle){ 50 + 165*2, 180, 158, 40 }, "#18#PASTE");
        btnClearPressed = GuiButton((Rectangle){ 50 + 165*3, 180, 158, 40 }, "#143#CLEAR");

        // Draw clipboard status
        GuiSetState(STATE_DISABLED);
        GuiLabel((Rectangle){ 50, 260, 700, 40 }, "Clipboard current text data:");
        GuiSetStyle(TEXTBOX, TEXT_READONLY, 1);
        GuiTextBox((Rectangle){ 50, 300, 700, 40 }, (char *)clipboardText, 256, false);
        GuiSetStyle(TEXTBOX, TEXT_READONLY, 0);
        GuiLabel((Rectangle){ 50, 360, 700, 40 }, "Try copying text from other applications and pasting here!");
        GuiSetState(STATE_NORMAL);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
