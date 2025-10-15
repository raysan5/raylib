/*******************************************************************************************
*
*   raylib [core] example - clipboard text
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
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

#include <stdio.h>

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

    const char* clipboardText = NULL;

    // List of text the user can switch through and copy
    const char* copyableText[] = {"raylib is fun", "hello, clipboard!", "potato chips"};

    unsigned int textIndex = 0;

    const char* popupText = NULL;

    // Initialize timers
    // The amount of time the pop-up text is on screen, before fading
    const float maxTime = 3.0f;
    float textTimer = 0.0f;
    // The length of time text is offset
    const float animMaxTime = 0.1f;
    float pasteAnim = 0.0f;
    float copyAnim = 0.0f;
    int copyAnimMult = 1;
    float textAnim = 0.0f;
    float textAlpha = 0.0f;
    // Offset amount for animations
    const int offsetAmount = -4;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Check if the user has pressed the copy/paste key combinations
        bool pastePressed = (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V));
        bool copyPressed = (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C));

        // Update animation timers
        if (textTimer > 0) textTimer -= GetFrameTime();
        if (pasteAnim > 0) pasteAnim -= GetFrameTime();
        if (copyAnim > 0) copyAnim -= GetFrameTime();
        if (textAnim > 0) textAnim -= GetFrameTime();

        // React to the user pressing paste
        if (pastePressed)
        {
            // Most operating systems hide this information until the user presses Ctrl-V on the window.

            // Check to see if the clipboard contains an image
            // This function does nothing outside of Windows, as it directly calls the Windows API
            Image image = GetClipboardImage();
            
            if (IsImageValid(image))
            {
                // Unload the image
                UnloadImage(image);
                // Update visuals
                popupText = "clipboard contains image";
            }
            else
            {
                // Get text from the user's clipboard
                clipboardText = GetClipboardText();
                
                // Update visuals
                popupText = "text pasted";
                pasteAnim = animMaxTime;
            }

            // Reset animation values
            textTimer = maxTime;
            textAnim = animMaxTime;
            textAlpha = 1;
        }

        // React to the user pressing copy
        if (copyPressed)
        {
            // Set the text on the user's clipboard
            SetClipboardText(copyableText[textIndex]);

            // Reset values
            textTimer = maxTime;
            textAnim = animMaxTime;
            copyAnim = animMaxTime;
            copyAnimMult = 1;
            textAlpha = 1;
            // Update the text that pops up at the bottom of the screen
            popupText = "text copied";
        }

        // Switch to the next item in the list when the user presses up
        if (IsKeyPressed(KEY_UP))
        {
            // Reset animation
            copyAnim = animMaxTime;
            copyAnimMult = 1;

            textIndex += 1;

            if (textIndex >= sizeof(copyableText) / sizeof(const char*)) // Length of array
            { 
                // Loop back to the other end
                textIndex = 0;
            }
        }

        // Switch to the previous item in the list when the user presses down
        if (IsKeyPressed(KEY_DOWN))
        {
            // Reset animation
            copyAnim = animMaxTime;
            copyAnimMult = -1;

            if (textIndex == 0)
            {
                // Loop back to the other end
                textIndex = (sizeof(copyableText) / sizeof(const char*)) - 1; // Length of array minus one
            } 
            else
            {
                textIndex -= 1;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

			ClearBackground(RAYWHITE);

			// Draw the user's pasted text, if there is any yet
			if (clipboardText)
			{
				// Offset animation
				int offset = 0;
				if (pasteAnim > 0) offset = offsetAmount;

				// Draw the pasted text
				DrawText("pasted clipboard:", 10, 10 + offset, 20, DARKGREEN);
				DrawText(clipboardText, 10, 30 + offset, 20, DARKGRAY);
			}

			// Offset animation
			int textOffset = 0;
			if (copyAnim > 0) textOffset = offsetAmount;

			// Draw copyable text and controls
			DrawText(copyableText[textIndex], 10, 330 + (textOffset * copyAnimMult), 20, MAROON);
			DrawText("up/down to change string, ctrl-c to copy, ctrl-v to paste", 10, 355, 20, DARKGRAY);

			// Alpha / Offset animation
			if (textAlpha > 0)
			{
				// Offset animation
				int offset = 0;
				if (textAnim > 0) offset = offsetAmount;
				// Draw pop up text
				DrawText(popupText, 10, 425 + offset, 20, ColorAlpha(DARKGREEN, textAlpha));

				// Fade-out animation
				if (textTimer < 0)
				{
					textAlpha -= GetFrameTime();
				}
			}

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}