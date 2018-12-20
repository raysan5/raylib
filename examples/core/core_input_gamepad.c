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
*   This example has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

// NOTE: Gamepad name ID depends on drivers and OS
#if defined(PLATFORM_RPI)
    #define XBOX360_NAME_ID     "Microsoft X-Box 360 pad"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#else
    #define XBOX360_NAME_ID     "Xbox 360 Controller"
    #define PS3_NAME_ID         "PLAYSTATION(R)3 Controller"
#endif

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Set MSAA 4X hint before windows creation  
    
    InitWindow(screenWidth, screenHeight, "raylib [core] example - gamepad input");
    
    Texture2D texPs3Pad = LoadTexture("resources/ps3.png");
    Texture2D texXboxPad = LoadTexture("resources/xbox.png");

    SetTargetFPS(60);
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

            ClearBackground(RL_RAYWHITE);
            
            if (IsGamepadAvailable(GAMEPAD_PLAYER1))
            {
                DrawText(FormatText("GP1: %s", GetGamepadName(GAMEPAD_PLAYER1)), 10, 10, 10, RL_BLACK);

                if (IsGamepadName(GAMEPAD_PLAYER1, XBOX360_NAME_ID))
                {
                    DrawTexture(texXboxPad, 0, 0, RL_DARKGRAY);
                    
                    // Draw buttons: xbox home
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_HOME)) DrawCircle(394, 89, 19, RL_RED);

                    // Draw buttons: basic
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_START)) DrawCircle(436, 150, 9, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_SELECT)) DrawCircle(352, 150, 9, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_X)) DrawCircle(501, 151, 15, RL_BLUE);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_A)) DrawCircle(536, 187, 15, RL_LIME);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_B)) DrawCircle(572, 151, 15, RL_MAROON);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_Y)) DrawCircle(536, 115, 15, RL_GOLD);
                    
                    // Draw buttons: d-pad
                    DrawRectangle(317, 202, 19, 71, RL_BLACK);
                    DrawRectangle(293, 228, 69, 19, RL_BLACK);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_UP)) DrawRectangle(317, 202, 19, 26, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_DOWN)) DrawRectangle(317, 202 + 45, 19, 26, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_LEFT)) DrawRectangle(292, 228, 25, 19, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_RIGHT)) DrawRectangle(292 + 44, 228, 26, 19, RL_RED);
                    
                    // Draw buttons: left-right back
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_LB)) DrawCircle(259, 61, 20, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_XBOX_BUTTON_RB)) DrawCircle(536, 61, 20, RL_RED);

                    // Draw axis: left joystick
                    DrawCircle(259, 152, 39, RL_BLACK);
                    DrawCircle(259, 152, 34, RL_LIGHTGRAY);
                    DrawCircle(259 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_LEFT_X)*20), 
                               152 - (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_LEFT_Y)*20), 25, RL_BLACK);
                    
                    // Draw axis: right joystick
                    DrawCircle(461, 237, 38, RL_BLACK);
                    DrawCircle(461, 237, 33, RL_LIGHTGRAY);
                    DrawCircle(461 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_RIGHT_X)*20), 
                               237 - (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_RIGHT_Y)*20), 25, RL_BLACK);

                    // Draw axis: left-right triggers
                    DrawRectangle(170, 30, 15, 70, RL_GRAY);
                    DrawRectangle(604, 30, 15, 70, RL_GRAY);              
                    DrawRectangle(170, 30, 15, (((1.0f + GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_LT))/2.0f)*70), RL_RED);
                    DrawRectangle(604, 30, 15, (((1.0f + GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_RT))/2.0f)*70), RL_RED);
                    
                    //DrawText(FormatText("Xbox axis LT: %02.02f", GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_LT)), 10, 40, 10, RL_BLACK);
                    //DrawText(FormatText("Xbox axis RT: %02.02f", GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_XBOX_AXIS_RT)), 10, 60, 10, RL_BLACK);
                }
                else if (IsGamepadName(GAMEPAD_PLAYER1, PS3_NAME_ID))
                {
                    DrawTexture(texPs3Pad, 0, 0, RL_DARKGRAY);

                    // Draw buttons: ps
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_PS)) DrawCircle(396, 222, 13, RL_RED);
                    
                    // Draw buttons: basic
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_SELECT)) DrawRectangle(328, 170, 32, 13, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_START)) DrawTriangle((Vector2){ 436, 168 }, (Vector2){ 436, 185 }, (Vector2){ 464, 177 }, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_TRIANGLE)) DrawCircle(557, 144, 13, RL_LIME);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_CIRCLE)) DrawCircle(586, 173, 13, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_CROSS)) DrawCircle(557, 203, 13, RL_VIOLET);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_SQUARE)) DrawCircle(527, 173, 13, RL_PINK);

                    // Draw buttons: d-pad
                    DrawRectangle(225, 132, 24, 84, RL_BLACK);
                    DrawRectangle(195, 161, 84, 25, RL_BLACK);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_UP)) DrawRectangle(225, 132, 24, 29, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_DOWN)) DrawRectangle(225, 132 + 54, 24, 30, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_LEFT)) DrawRectangle(195, 161, 30, 25, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_RIGHT)) DrawRectangle(195 + 54, 161, 30, 25, RL_RED);
                    
                    // Draw buttons: left-right back buttons
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_L1)) DrawCircle(239, 82, 20, RL_RED);
                    if (IsGamepadButtonDown(GAMEPAD_PLAYER1, GAMEPAD_PS3_BUTTON_R1)) DrawCircle(557, 82, 20, RL_RED);

                    // Draw axis: left joystick
                    DrawCircle(319, 255, 35, RL_BLACK);
                    DrawCircle(319, 255, 31, RL_LIGHTGRAY);
                    DrawCircle(319 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_LEFT_X)*20), 
                               255 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_LEFT_Y)*20), 25, RL_BLACK);
                    
                    // Draw axis: right joystick
                    DrawCircle(475, 255, 35, RL_BLACK);
                    DrawCircle(475, 255, 31, RL_LIGHTGRAY);
                    DrawCircle(475 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_RIGHT_X)*20), 
                               255 + (GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_RIGHT_Y)*20), 25, RL_BLACK);

                    // Draw axis: left-right triggers
                    DrawRectangle(169, 48, 15, 70, RL_GRAY);
                    DrawRectangle(611, 48, 15, 70, RL_GRAY);              
                    DrawRectangle(169, 48, 15, (((1.0f - GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_L2))/2.0f)*70), RL_RED);
                    DrawRectangle(611, 48, 15, (((1.0f - GetGamepadAxisMovement(GAMEPAD_PLAYER1, GAMEPAD_PS3_AXIS_R2))/2.0f)*70), RL_RED);
                }
                else
                {
                    DrawText("- GENERIC GAMEPAD -", 280, 180, 20, RL_GRAY);
                    
                    // TODO: Draw generic gamepad
                }
                
                DrawText(FormatText("DETECTED AXIS [%i]:", GetGamepadAxisCount(GAMEPAD_PLAYER1)), 10, 50, 10, RL_MAROON); 
                
                for (int i = 0; i < GetGamepadAxisCount(GAMEPAD_PLAYER1); i++)
                {
                    DrawText(FormatText("AXIS %i: %.02f", i, GetGamepadAxisMovement(GAMEPAD_PLAYER1, i)), 20, 70 + 20*i, 10, RL_DARKGRAY);
                }
                
                if (GetGamepadButtonPressed() != -1) DrawText(FormatText("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 430, 10, RL_RED);
                else DrawText("DETECTED BUTTON: NONE", 10, 430, 10, RL_GRAY);
            }
            else
            {
                DrawText("GP1: NOT DETECTED", 10, 10, 10, RL_GRAY);
                
                DrawTexture(texXboxPad, 0, 0, RL_LIGHTGRAY);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texPs3Pad);
    UnloadTexture(texXboxPad);
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}