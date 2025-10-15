/*******************************************************************************************
*
*   raylib [core] example - Clipboard Text Operations
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example contributed by [Your Name] and reviewed by the raylib community
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 [ANANTH S] (@Ananth1836)
*
********************************************************************************************/

#include "raylib.h"
#include <cstddef>

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
    const char *sampleTexts[] = {
        "Hello from raylib!",
        "The quick brown fox jumps over the lazy dog",
        "Clipboard operations are useful!",
        "raylib is a simple and easy-to-use library",
        "Copy and paste me!"
    };
    const int sampleTextsCount = sizeof(sampleTexts) / sizeof(sampleTexts[0]);
    
    int currentTextIndex = 0;
    char inputBuffer[256] = "Type here to copy to clipboard...";
    bool textEdited = false;
    
    // UI state
    Rectangle copyButton = { 50, 350, 150, 40 };
    Rectangle pasteButton = { 220, 350, 150, 40 };
    Rectangle clearButton = { 390, 350, 150, 40 };
    Rectangle cycleButton = { 560, 350, 150, 40 };
    
    Rectangle textBox = { 50, 250, 700, 40 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePoint = GetMousePosition();
        
        // Handle text input
        if (CheckCollisionPointRec(mousePoint, textBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (TextIsEqual(inputBuffer, "Type here to copy to clipboard..."))
            {
                inputBuffer[0] = '\0'; // Clear the default text
            }
        }
        
        // Get key presses and update input buffer
        if (textEdited) textEdited = false;
        
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125))
            {
                int length = TextLength(inputBuffer);
                if (length < 255)
                {
                    inputBuffer[length] = (char)key;
                    inputBuffer[length + 1] = '\0';
                    textEdited = true;
                }
            }
            key = GetCharPressed();
        }
        
        // Handle backspace
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            int length = TextLength(inputBuffer);
            if (length > 0)
            {
                inputBuffer[length - 1] = '\0';
                textEdited = true;
            }
        }
        
        // Handle button interactions
        if (CheckCollisionPointRec(mousePoint, copyButton))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Copy text to clipboard
                SetClipboardText(inputBuffer);
            }
        }
        
        if (CheckCollisionPointRec(mousePoint, pasteButton))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Paste text from clipboard
                const char *clipboardText = GetClipboardText();
                if (clipboardText != NULL)
                {
                    TextCopy(inputBuffer, clipboardText);
                }
            }
        }
        
        if (CheckCollisionPointRec(mousePoint, clearButton))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Clear input buffer
                inputBuffer[0] = '\0';
            }
        }
        
        if (CheckCollisionPointRec(mousePoint, cycleButton))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Cycle through sample texts
                currentTextIndex = (currentTextIndex + 1) % sampleTextsCount;
                TextCopy(inputBuffer, sampleTexts[currentTextIndex]);
            }
        }
        
        // Quick copy/paste with keyboard shortcuts
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
        {
            if (IsKeyPressed(KEY_C))
            {
                SetClipboardText(inputBuffer);
            }
            if (IsKeyPressed(KEY_V))
            {
                const char *clipboardText = GetClipboardText();
                if (clipboardText != NULL)
                {
                    TextCopy(inputBuffer, clipboardText);
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawText("Clipboard Text Operations", 20, 20, 32, DARKBLUE);
            
            // Draw instructions
            DrawText("Use the buttons below or keyboard shortcuts:", 20, 70, 20, DARKGRAY);
            DrawText("CTRL+C to copy, CTRL+V to paste", 20, 100, 20, DARKGRAY);
            
            // Draw text box
            DrawRectangleRec(textBox, LIGHTGRAY);
            DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
            DrawText(inputBuffer, (int)textBox.x + 5, (int)textBox.y + 10, 20, MAROON);
            
            // Draw cursor in text box
            if (((int)(GetTime() * 2) % 2) == 0 && textEdited)
            {
                DrawText("_", (int)textBox.x + 5 + MeasureText(inputBuffer, 20), (int)textBox.y + 10, 20, MAROON);
            }
            
            // Draw buttons
            DrawRectangleRec(copyButton, CheckCollisionPointRec(mousePoint, copyButton) ? SKYBLUE : BLUE);
            DrawRectangleLines((int)copyButton.x, (int)copyButton.y, (int)copyButton.width, (int)copyButton.height, DARKBLUE);
            DrawText("Copy", (int)copyButton.x + 45, (int)copyButton.y + 10, 20, WHITE);
            
            DrawRectangleRec(pasteButton, CheckCollisionPointRec(mousePoint, pasteButton) ? SKYBLUE : BLUE);
            DrawRectangleLines((int)pasteButton.x, (int)pasteButton.y, (int)pasteButton.width, (int)pasteButton.height, DARKBLUE);
            DrawText("Paste", (int)pasteButton.x + 40, (int)pasteButton.y + 10, 20, WHITE);
            
            DrawRectangleRec(clearButton, CheckCollisionPointRec(mousePoint, clearButton) ? SKYBLUE : BLUE);
            DrawRectangleLines((int)clearButton.x, (int)clearButton.y, (int)clearButton.width, (int)clearButton.height, DARKBLUE);
            DrawText("Clear", (int)clearButton.x + 40, (int)clearButton.y + 10, 20, WHITE);
            
            DrawRectangleRec(cycleButton, CheckCollisionPointRec(mousePoint, cycleButton) ? SKYBLUE : BLUE);
            DrawRectangleLines((int)cycleButton.x, (int)cycleButton.y, (int)cycleButton.width, (int)cycleButton.height, DARKBLUE);
            DrawText("Sample Text", (int)cycleButton.x + 15, (int)cycleButton.y + 10, 20, WHITE);
            
            // Draw clipboard status
            DrawText("Try copying text from other applications and pasting here!", 50, 420, 18, DARKGREEN);
            
            // Draw current sample text info
            DrawText(TextFormat("Sample Text %d/%d", currentTextIndex + 1, sampleTextsCount), 
                     600, 300, 18, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}