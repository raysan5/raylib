/*******************************************************************************************
*
*   raylib [core] example - input virtual controls
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.0, last time updated with raylib 5.0
*
*   Example contributed by GreenSnakeLinux (@GreenSnakeLinux),
*   reviewed by Ramon Santamaria (@raysan5), oblerion (@oblerion) and danilwhale (@danilwhale)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2025 GreenSnakeLinux (@GreenSnakeLinux) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>

typedef enum {
    BUTTON_NONE = -1,
    BUTTON_UP,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_DOWN,
    BUTTON_MAX
} PadButton;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input virtual controls");

    Vector2 padPosition = { 100, 350 };
    float buttonRadius = 30;

    Vector2 buttonPositions[BUTTON_MAX] = {
        { padPosition.x,padPosition.y - buttonRadius*1.5f },  // Up
        { padPosition.x - buttonRadius*1.5f, padPosition.y }, // Left
        { padPosition.x + buttonRadius*1.5f, padPosition.y }, // Right
        { padPosition.x, padPosition.y + buttonRadius*1.5f }  // Down
    };

    const char *buttonLabels[BUTTON_MAX] = {
        "Y",    // Up
        "X",    // Left
        "B",    // Right
        "A"     // Down
    };

    Color buttonLabelColors[BUTTON_MAX] = {
        YELLOW, // Up
        BLUE,   // Left
        RED,    // Right
        GREEN   // Down
    };

    int pressedButton = BUTTON_NONE;
    Vector2 inputPosition = { 0, 0 };

    Vector2 playerPosition = { (float)screenWidth/2, (float)screenHeight/2 };
    float playerSpeed = 75;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //--------------------------------------------------------------------------
        if ((GetTouchPointCount() > 0)) inputPosition = GetTouchPosition(0); // Use touch position
        else inputPosition = GetMousePosition(); // Use mouse position

        // Reset pressed button to none
        pressedButton = BUTTON_NONE;

        // Make sure user is pressing left mouse button if they're from desktop
        if ((GetTouchPointCount() > 0) ||
            ((GetTouchPointCount() == 0) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)))
        {
            // Find nearest D-Pad button to the input position
            for (int i = 0; i < BUTTON_MAX; i++)
            {
                float distX = fabsf(buttonPositions[i].x - inputPosition.x);
                float distY = fabsf(buttonPositions[i].y - inputPosition.y);

                if ((distX + distY < buttonRadius))
                {
                    pressedButton = i;
                    break;
                }
            }
        }

        // Move player according to pressed button
        switch (pressedButton)
        {
            case BUTTON_UP: playerPosition.y -= playerSpeed*GetFrameTime(); break;
            case BUTTON_LEFT: playerPosition.x -= playerSpeed*GetFrameTime(); break;
            case BUTTON_RIGHT: playerPosition.x += playerSpeed*GetFrameTime(); break;
            case BUTTON_DOWN: playerPosition.y += playerSpeed*GetFrameTime(); break;
            default: break;
        };
        //--------------------------------------------------------------------------

        // Draw
        //--------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw world
            DrawCircleV(playerPosition, 50, MAROON);

            // Draw GUI
            for (int i = 0; i < BUTTON_MAX; i++)
            {
                DrawCircleV(buttonPositions[i], buttonRadius, (i == pressedButton)? DARKGRAY : BLACK);

                DrawText(buttonLabels[i],
                    (int)buttonPositions[i].x - 7, (int)buttonPositions[i].y - 8,
                    20, buttonLabelColors[i]);
            }

            DrawText("move the player with D-Pad buttons", 10, 10, 20, DARKGRAY);

        EndDrawing();
        //--------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
