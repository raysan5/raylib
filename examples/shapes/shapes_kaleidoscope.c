/*******************************************************************************************
*
*   raylib [shapes] example - kaleidoscope
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Hugo ARNAL (@hugoarnal) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Hugo ARNAL (@hugoarnal)
*
********************************************************************************************/

#include "raylib.h"
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

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - kaleidoscope");

    int symmetry = 6;
    float angle = 360.0f/(float)symmetry;
    float thickness = 3.0f;
    Vector2 prevMousePos = { 0 };

    SetTargetFPS(60);
    ClearBackground(BLACK);

    Vector2 offset = { (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0 };
    camera.offset = offset;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Vector2 scaleVector = { 1.0f, -1.0f };
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mousePos = GetMousePosition();
        Vector2 lineStart = Vector2Subtract(mousePos, offset);
        Vector2 lineEnd = Vector2Subtract(prevMousePos, offset);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            BeginMode2D(camera);
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    for (int i = 0; i < symmetry; i++) {
                        lineStart = Vector2Rotate(lineStart, angle*DEG2RAD);
                        lineEnd = Vector2Rotate(lineEnd, angle*DEG2RAD);

                        DrawLineEx(lineStart, lineEnd, thickness, WHITE);

                        Vector2 reflectLineStart = Vector2Multiply(lineStart, scaleVector);
                        Vector2 reflectLineEnd = Vector2Multiply(lineEnd, scaleVector);

                        DrawLineEx(reflectLineStart, reflectLineEnd, thickness, WHITE);
                    }
                }

                prevMousePos = mousePos;
            EndMode2D();
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
