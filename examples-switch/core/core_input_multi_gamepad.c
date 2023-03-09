/*******************************************************************************************
*
*   raylib [core] example - Gamepad input
*
*   NOTE: This example requires a Gamepad connected to the system
*         raylib is configured to work with the following gamepads:
*                - Xbox 360 Controller (Xbox 360, Xbox One)
*                - PLAYSTATION(R)3 Controller
*         Check raylib.h for buttons configuration
*
*   Example originally created with raylib 1.1, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <switch.h>

// NOTE: Gamepad name ID depends on drivers and OS
#define XBOX360_LEGACY_NAME_ID  "Xbox Controller"
#if defined(PLATFORM_RPI)
    #define XBOX360_NAME_ID     "Microsoft X-Box 360 pad"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#else
    #define XBOX360_NAME_ID     "Xbox 360 Controller"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#endif
#define MAX_GAMEPADS            4

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Set MSAA 4X hint before windows creation

    InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            for(int i = 0; i< MAX_GAMEPADS; i++) {
                int height = 75 * (i + 3);
                if(IsGamepadAvailable(i))
                {
                    if (IsGamepadButtonDown(i, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
                    {
                        DrawRectangle(0, height - 22, 1280, 75, LIGHTGRAY);
                        DrawText(TextFormat("Controller %i", i+1), 300, height, 30, DARKGRAY);
                        DrawText(TextFormat("%s", GetGamepadName(i)), 600, height, 30, DARKGRAY);
                    } else {
                        DrawText(TextFormat("Controller %i", i+1), 300, height, 30, LIGHTGRAY);
                        DrawText(TextFormat("%s", GetGamepadName(i)), 600, height, 30, LIGHTGRAY);
                    }
                } else {
                    DrawText(TextFormat("Controller %i", i+1), 300, height, 30, LIGHTGRAY);
                    DrawText(TextFormat("Not Connected"), 600, height, 30, LIGHTGRAY);
                }
            }
            DrawText("Press", 1110, 670, 20, GRAY);
            DrawCircle(1200, 680, 20, GRAY);
            DrawCircle(1200, 680, 18, DARKGRAY);
            DrawText("A", 1194, 671, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
