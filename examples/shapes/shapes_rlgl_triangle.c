/*******************************************************************************************
*
*   raylib [shapes] example - rlgl triangle
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Robin (@RobinsAviary) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025-2025 Robin (@RobinsAviary)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - rlgl triangle");

    // Starting postions and rendered triangle positions
    Vector2 startingPositions[] = {{ 400.0f, 150.0f }, { 300.0f, 300.0f }, { 500.0f, 300.0f }};
    Vector2 trianglePositions[] = { startingPositions[0], startingPositions[1], startingPositions[2] };

    // Currently selected vertex, -1 means none
    int triangleIndex = -1;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Reset index on release
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            triangleIndex = -1;
        }

        // If the user has selected a vertex, offset it by the mouse's delta this frame
        if (triangleIndex != -1)
        {
            Vector2 *position = &trianglePositions[triangleIndex];

            Vector2 mouseDelta = GetMouseDelta();
            position->x += mouseDelta.x;
            position->y += mouseDelta.y;
        }

        // Enable/disable backface culling (2-sided triangles, slower to render)
        if (IsKeyPressed(KEY_LEFT))
        {
            rlEnableBackfaceCulling();
        }

        if (IsKeyPressed(KEY_RIGHT))
        {
            rlDisableBackfaceCulling();
        }

        // Reset triangle vertices to starting positions and reset backface culling
        if (IsKeyPressed(KEY_R))
        {
            trianglePositions[0] = startingPositions[0];
            trianglePositions[1] = startingPositions[1];
            trianglePositions[2] = startingPositions[2];

            rlEnableBackfaceCulling();
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (IsKeyDown(KEY_SPACE))
        {
            // Draw triangle with lines
            rlBegin(RL_LINES);
				// Three lines, six points
				// Define color for next vertex
				rlColor4ub(255, 0, 0, 255);
				// Define vertex
				rlVertex2f(trianglePositions[0].x, trianglePositions[0].y);
				rlColor4ub(0, 255, 0, 255);
				rlVertex2f(trianglePositions[1].x, trianglePositions[1].y);

				rlColor4ub(0, 255, 0, 255);
				rlVertex2f(trianglePositions[1].x, trianglePositions[1].y);
				rlColor4ub(0, 0, 255, 255);
				rlVertex2f(trianglePositions[2].x, trianglePositions[2].y);

				rlColor4ub(0, 0, 255, 255);
				rlVertex2f(trianglePositions[2].x, trianglePositions[2].y);
				rlColor4ub(255, 0, 0, 255);
				rlVertex2f(trianglePositions[0].x, trianglePositions[0].y);
            rlEnd();
        }
        else
        {
            // Draw triangle as a triangle
            rlBegin(RL_TRIANGLES);
				// One triangle, three points
				// Define color for next vertex
				rlColor4ub(255, 0, 0, 255);
				// Define vertex
				rlVertex2f(trianglePositions[0].x, trianglePositions[0].y);
				rlColor4ub(0, 255, 0, 255);
				rlVertex2f(trianglePositions[1].x, trianglePositions[1].y);
				rlColor4ub(0, 0, 255, 255);
				rlVertex2f(trianglePositions[2].x, trianglePositions[2].y);
            rlEnd();
        }

        // Render the vertex handles, reacting to mouse movement/input
        for (unsigned int i = 0; i < 3; i++)
        {
            Vector2 position = trianglePositions[i];

            float size = 4.0f;

            Vector2 mousePosition = GetMousePosition();

            // If the cursor is within the handle circle
            if (Vector2Distance(mousePosition, position) < size)
            {
                float fillAlpha = 0.0f;
                if (triangleIndex == -1)
                {
                    fillAlpha = 0.5f;
                }

                // If handle selected/clicked
                if (i == triangleIndex)
                {
                    fillAlpha = 1.0f;
                }

                // If clicked, set selected index to handle index
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    triangleIndex = i;
                }

                // If visible, draw DARKGRAY circle with varying alpha.
                if (fillAlpha > 0.0f)
                {
                    Color fillColor = ColorAlpha(DARKGRAY, fillAlpha);

                    DrawCircleV(position, size, fillColor);
                }
            }

            // Draw handle outline
            DrawCircleLinesV(position, size, BLACK);
        }

        // Draw controls
        DrawText("space for lines\nleft for backface culling\nright for no backface culling\nclick and drag points\nr to reset", 10, 10, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}