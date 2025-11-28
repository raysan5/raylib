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
*   Copyright (c) 2025 Robin (@RobinsAviary)
*
********************************************************************************************/

#include "raylib.h"

#include "rlgl.h"

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
    Vector2 startingPositions[3] = {{ 400.0f, 150.0f }, { 300.0f, 300.0f }, { 500.0f, 300.0f }};
    Vector2 trianglePositions[3] = { startingPositions[0], startingPositions[1], startingPositions[2] };

    // Currently selected vertex, -1 means none
    int triangleIndex = -1;
    bool linesMode = false;
    float handleRadius = 8.0f;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_SPACE)) linesMode = !linesMode;

        // Check selected vertex
        for (unsigned int i = 0; i < 3; i++)
        {
            // If the mouse is within the handle circle
            if (CheckCollisionPointCircle(GetMousePosition(), trianglePositions[i], handleRadius) &&
                IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                triangleIndex = i;
                break;
            }
        }

        // If the user has selected a vertex, offset it by the mouse's delta this frame
        if (triangleIndex != -1)
        {
            Vector2 *position = &trianglePositions[triangleIndex];

            Vector2 mouseDelta = GetMouseDelta();
            position->x += mouseDelta.x;
            position->y += mouseDelta.y;
        }

        // Reset index on release
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) triangleIndex = -1;

        // Enable/disable backface culling (2-sided triangles, slower to render)
        if (IsKeyPressed(KEY_LEFT)) rlEnableBackfaceCulling();
        if (IsKeyPressed(KEY_RIGHT)) rlDisableBackfaceCulling();

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

            if (linesMode)
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
                // Draw handle fill focused by mouse
                if (CheckCollisionPointCircle(GetMousePosition(), trianglePositions[i], handleRadius))
                    DrawCircleV(trianglePositions[i], handleRadius, ColorAlpha(DARKGRAY, 0.5f));

                // Draw handle fill selected
                if (i == triangleIndex) DrawCircleV(trianglePositions[i], handleRadius, DARKGRAY);

                // Draw handle outline
                DrawCircleLinesV(trianglePositions[i], handleRadius, BLACK);
            }

            // Draw controls
            DrawText("SPACE: Toggle lines mode", 10, 10, 20, DARKGRAY);
            DrawText("LEFT-RIGHT: Toggle backface culling", 10, 40, 20, DARKGRAY);
            DrawText("MOUSE: Click and drag vertex points", 10, 70, 20, DARKGRAY);
            DrawText("R: Reset triangle to start positions", 10, 100, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}