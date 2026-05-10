/*******************************************************************************************
*
*   raylib [core] example - input gestures
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 1.4, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2016-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_GESTURE_STRINGS   20

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input gestures");

    Vector2 touchPosition = { 0, 0 };
    Rectangle touchArea = { 220, 10, screenWidth - 230.0f, screenHeight - 20.0f };

    int gesturesCount = 0;
    char gestureStrings[MAX_GESTURE_STRINGS][32];

    int currentGesture = GESTURE_NONE;
    int lastGesture = GESTURE_NONE;

    //SetGesturesEnabled(0b0000000000001001);   // Enable only some gestures to be detected

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        lastGesture = currentGesture;
        currentGesture = GetGestureDetected();
        touchPosition = GetTouchPosition(0);

        if (CheckCollisionPointRec(touchPosition, touchArea) && (currentGesture != GESTURE_NONE))
        {
            if (currentGesture != lastGesture)
            {
                // Store gesture string
                switch (currentGesture)
                {
                    case GESTURE_TAP: TextCopy(gestureStrings[gesturesCount], "GESTURE TAP"); break;
                    case GESTURE_DOUBLETAP: TextCopy(gestureStrings[gesturesCount], "GESTURE DOUBLETAP"); break;
                    case GESTURE_HOLD: TextCopy(gestureStrings[gesturesCount], "GESTURE HOLD"); break;
                    case GESTURE_DRAG: TextCopy(gestureStrings[gesturesCount], "GESTURE DRAG"); break;
                    case GESTURE_SWIPE_RIGHT: TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE RIGHT"); break;
                    case GESTURE_SWIPE_LEFT: TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE LEFT"); break;
                    case GESTURE_SWIPE_UP: TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE UP"); break;
                    case GESTURE_SWIPE_DOWN: TextCopy(gestureStrings[gesturesCount], "GESTURE SWIPE DOWN"); break;
                    case GESTURE_PINCH_IN: TextCopy(gestureStrings[gesturesCount], "GESTURE PINCH IN"); break;
                    case GESTURE_PINCH_OUT: TextCopy(gestureStrings[gesturesCount], "GESTURE PINCH OUT"); break;
                    default: break;
                }

                gesturesCount++;

                // Reset gestures strings
                if (gesturesCount >= MAX_GESTURE_STRINGS)
                {
                    for (int i = 0; i < MAX_GESTURE_STRINGS; i++) TextCopy(gestureStrings[i], "\0");

                    gesturesCount = 0;
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawRectangleRec(touchArea, GRAY);
            DrawRectangle(225, 15, screenWidth - 240, screenHeight - 30, RAYWHITE);

            DrawText("GESTURES TEST AREA", screenWidth - 270, screenHeight - 40, 20, Fade(GRAY, 0.5f));

            for (int i = 0; i < gesturesCount; i++)
            {
                if (i%2 == 0) DrawRectangle(10, 30 + 20*i, 200, 20, Fade(LIGHTGRAY, 0.5f));
                else DrawRectangle(10, 30 + 20*i, 200, 20, Fade(LIGHTGRAY, 0.3f));

                if (i < gesturesCount - 1) DrawText(gestureStrings[i], 35, 36 + 20*i, 10, DARKGRAY);
                else DrawText(gestureStrings[i], 35, 36 + 20*i, 10, MAROON);
            }

            DrawRectangleLines(10, 29, 200, screenHeight - 50, GRAY);
            DrawText("DETECTED GESTURES", 50, 15, 10, GRAY);

            if (currentGesture != GESTURE_NONE) DrawCircleV(touchPosition, 30, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}