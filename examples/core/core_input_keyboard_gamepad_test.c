/*******************************************************************************************
*
*   raylib [core] example - Keyboard vs Gamepad Input Test
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   This example is a diagnostic tool to verify that keyboard input is not
*   incorrectly detected as gamepad input on Android devices.
*
*   Issue reference: https://github.com/raysan5/raylib/issues/5387
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 raylib contributors
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard vs gamepad test");

    Vector2 ballPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    int lastKeyPressed = 0;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------

        // Track keyboard input
        if (IsKeyDown(KEY_RIGHT)) ballPosition.x += 4.0f;
        if (IsKeyDown(KEY_LEFT)) ballPosition.x -= 4.0f;
        if (IsKeyDown(KEY_UP)) ballPosition.y -= 4.0f;
        if (IsKeyDown(KEY_DOWN)) ballPosition.y += 4.0f;

        // Keep ball on screen
        if (ballPosition.x < 25) ballPosition.x = 25;
        if (ballPosition.x > screenWidth - 25) ballPosition.x = screenWidth - 25;
        if (ballPosition.y < 25) ballPosition.y = 25;
        if (ballPosition.y > screenHeight - 25) ballPosition.y = screenHeight - 25;

        // Track last key pressed
        int key = GetKeyPressed();
        if (key != 0) lastKeyPressed = key;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Title
            DrawText("KEYBOARD vs GAMEPAD INPUT TEST", 180, 10, 20, DARKGRAY);
            DrawText("Issue #5387: Keyboard detected as gamepad on some Android devices", 120, 35, 14, GRAY);

            // Divider
            DrawLine(0, 60, screenWidth, 60, LIGHTGRAY);

            // Keyboard section
            DrawText("KEYBOARD INPUT", 20, 75, 18, DARKBLUE);
            DrawRectangle(20, 100, 360, 80, Fade(BLUE, 0.1f));

            DrawText(TextFormat("Arrow Keys: [%s] [%s] [%s] [%s]",
                IsKeyDown(KEY_UP) ? "UP" : "--",
                IsKeyDown(KEY_DOWN) ? "DN" : "--",
                IsKeyDown(KEY_LEFT) ? "LT" : "--",
                IsKeyDown(KEY_RIGHT) ? "RT" : "--"), 30, 110, 16, BLACK);

            DrawText(TextFormat("Last Key Pressed: %d", lastKeyPressed), 30, 135, 16, DARKGRAY);
            DrawText(TextFormat("Any Key Down: %s", (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) ||
                IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) ? "YES" : "NO"), 30, 155, 16, DARKGRAY);

            // Gamepad section
            DrawText("GAMEPAD STATUS", 420, 75, 18, DARKGREEN);
            DrawRectangle(420, 100, 360, 80, Fade(GREEN, 0.1f));

            bool gamepadReady = IsGamepadAvailable(0);
            DrawText(TextFormat("Gamepad 0 Available: %s", gamepadReady ? "YES" : "NO"),
                430, 110, 16, gamepadReady ? RED : DARKGREEN);

            if (gamepadReady)
            {
                DrawText(TextFormat("D-Pad: [%s] [%s] [%s] [%s]",
                    IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP) ? "UP" : "--",
                    IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) ? "DN" : "--",
                    IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) ? "LT" : "--",
                    IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) ? "RT" : "--"),
                    430, 135, 16, RED);

                DrawText(TextFormat("Gamepad Name: %.20s", GetGamepadName(0)), 430, 155, 14, DARKGRAY);
            }
            else
            {
                DrawText("No gamepad detected", 430, 135, 16, DARKGREEN);
            }

            // Divider
            DrawLine(0, 190, screenWidth, 190, LIGHTGRAY);

            // Test result section
            DrawText("TEST RESULT", 20, 200, 18, MAROON);

            bool keyboardActive = IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) ||
                                  IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT);

            if (keyboardActive && gamepadReady)
            {
                // BUG DETECTED: Keyboard is triggering gamepad detection
                DrawRectangle(20, 225, 760, 50, Fade(RED, 0.3f));
                DrawText("BUG DETECTED: Keyboard input is being detected as gamepad!", 30, 235, 18, RED);
                DrawText("The fix for issue #5387 may not be working correctly.", 30, 258, 14, DARKGRAY);
            }
            else if (keyboardActive && !gamepadReady)
            {
                // CORRECT: Keyboard works without triggering gamepad
                DrawRectangle(20, 225, 760, 50, Fade(GREEN, 0.3f));
                DrawText("PASS: Keyboard input detected correctly (no phantom gamepad)", 30, 235, 18, DARKGREEN);
                DrawText("Issue #5387 fix is working as expected.", 30, 258, 14, DARKGRAY);
            }
            else if (!keyboardActive && gamepadReady)
            {
                // Gamepad is connected (might be real or might be bug on idle)
                DrawRectangle(20, 225, 760, 50, Fade(ORANGE, 0.3f));
                DrawText("INFO: Gamepad detected - press keyboard keys to test", 30, 235, 18, ORANGE);
                DrawText("If gamepad stays active while pressing keyboard = BUG", 30, 258, 14, DARKGRAY);
            }
            else
            {
                // Idle state
                DrawRectangle(20, 225, 760, 50, Fade(GRAY, 0.1f));
                DrawText("WAITING: Press arrow keys to test keyboard input", 30, 235, 18, GRAY);
                DrawText("Gamepad should NOT become available when pressing keyboard keys", 30, 258, 14, DARKGRAY);
            }

            // Ball controlled by keyboard
            DrawText("Ball Control (Arrow Keys):", 20, 295, 16, DARKGRAY);
            DrawCircleV(ballPosition, 25, MAROON);
            DrawCircleLines((int)ballPosition.x, (int)ballPosition.y, 25, DARKGRAY);

            // Instructions
            DrawRectangle(0, screenHeight - 45, screenWidth, 45, Fade(BLACK, 0.05f));
            DrawText("Instructions: Press keyboard arrow keys - the ball should move and gamepad should stay 'NO'",
                20, screenHeight - 35, 14, DARKGRAY);
            DrawText("If gamepad becomes 'YES' while pressing keyboard = issue #5387 is NOT fixed",
                20, screenHeight - 18, 14, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
