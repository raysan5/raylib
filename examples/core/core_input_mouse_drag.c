/*******************************************************************************************
*
*   raylib [core] example - input mouse drag
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 tipudeveloper (@tipudeveloper)
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - input mouse drag");
    SetTargetFPS(60);

    Rectangle rect = { screenWidth / 2 - 50, screenHeight / 2 - 50, 100, 100 };
    Vector2 mousePosition = { 0.0f, 0.0f };
    bool dragging = false;

    while (!WindowShouldClose())
    {
        mousePosition = GetMousePosition();

        // Begin dragging if the cursor is inside the rectangle when the left button is pressed
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePosition, rect))
            {
                dragging = true;
            }
        }
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            // Stop dragging when the button is released
            dragging = false;
        }

        if (dragging)
        {
            // Keep the rectangle centered on the cursor while dragging
            rect.x = mousePosition.x - rect.width / 2;
            rect.y = mousePosition.y - rect.height / 2;
        }

        // Use one mouse wheel sample per frame to scale the rectangle
        float wheelMove = GetMouseWheelMove();
        rect.width += wheelMove * 10.0f;
        rect.height += wheelMove * 10.0f;

        // Clamp rectangle size so it remains visible
        if (rect.width < 10) rect.width = 10;
        if (rect.height < 10) rect.height = 10;
        if (rect.width > 500) rect.width = 500;
        if (rect.height > 500) rect.height = 500;
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawRectangleRec(rect, RED);
            DrawText("Drag the rectangle and scale it with the mouse wheel", 10, 10, 20, BLACK);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}