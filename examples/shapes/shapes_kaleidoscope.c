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
*   Copyright (c) 2025 Hugo ARNAL (@hugoarnal) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#define MAX_DRAW_LINES  8192

// Line data type
typedef struct {
    Vector2 start;
    Vector2 end;
} Line;

// Lines array as a global static variable to be stored
// in heap and avoid potential stack overflow (on Web platform)
static Line lines[MAX_DRAW_LINES] = { 0 };

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

    // Line drawing properties
    int symmetry = 6;
    float angle = 360.0f/(float)symmetry;
    float thickness = 3.0f;
    Vector2 mousePos = { 0 };
    Vector2 prevMousePos = { 0 };
    Vector2 scaleVector = { 1.0f, -1.0f };
    Vector2 offset = { (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    
    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0 };
    camera.offset = offset;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    int lineCounter = 0;

    SetTargetFPS(20);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        prevMousePos = mousePos;
        mousePos = GetMousePosition();
        
        Vector2 lineStart = Vector2Subtract(mousePos, offset);
        Vector2 lineEnd = Vector2Subtract(prevMousePos, offset);
        
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            for (int s = 0; (s < symmetry) && (lineCounter < (MAX_DRAW_LINES - 1)); s++)
            {
                lineStart = Vector2Rotate(lineStart, angle*DEG2RAD);
                lineEnd = Vector2Rotate(lineEnd, angle*DEG2RAD);

                // Store mouse line
                lines[lineCounter].start = lineStart;
                lines[lineCounter].end = lineEnd;

                // Store reflective line
                lines[lineCounter + 1].start = Vector2Multiply(lineStart, scaleVector);
                lines[lineCounter + 1].end = Vector2Multiply(lineEnd, scaleVector);
                
                lineCounter += 2;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
    
            BeginMode2D(camera);
                for (int s = 0; s < symmetry; s++)
                {
                    for (int i = 0; i < lineCounter; i += 2)
                    {
                        DrawLineEx(lines[i].start, lines[i].end, thickness, BLACK);
                        DrawLineEx(lines[i + 1].start, lines[i + 1].end, thickness, BLACK);
                    }
                }
            EndMode2D();
            
            DrawText(TextFormat("LINES: %i/%i", lineCounter, MAX_DRAW_LINES), 10, screenHeight - 30, 20, MAROON);
            DrawFPS(10, 10);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
