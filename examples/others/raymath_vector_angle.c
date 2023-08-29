/*******************************************************************************************
*
*   raylib [shapes] example - Vector Angle
*
*   Example originally created with raylib 1.0, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib [math] example - vector angle");

    Vector2 v0 = { screenWidth/2, screenHeight/2 };
    Vector2 v1 = { 100.0f, 80.0f };
    Vector2 v2 = { 0 };             // Updated with mouse position
    
    float angle = 0.0f;             // Angle in degrees
    int angleMode = 0;              // 0-Vector2Angle(), 1-Vector2LineAngle()

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_SPACE)) angleMode = !angleMode;
        
        if (angleMode == 0)
        {
            // Calculate angle between two vectors, considering a common origin (v0)
            v1 = Vector2Add(v0, (Vector2){ 100.0f, 80.0f });
            v2 = GetMousePosition();
            angle = Vector2Angle(Vector2Normalize(Vector2Subtract(v1, v0)), Vector2Normalize(Vector2Subtract(v2, v0)))*RAD2DEG;
        }
        else if (angleMode == 1)
        {
            // Calculate angle defined by a two vectors line, in reference to horizontal line
            v1 = (Vector2){ screenWidth/2, screenHeight/2 };
            v2 = GetMousePosition();
            angle = Vector2LineAngle(v1, v2)*RAD2DEG;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            if (angleMode == 0) DrawText("v0", v0.x, v0.y, 10, DARKGRAY);
            DrawText("v1", v1.x, v1.y, 10, DARKGRAY);
            DrawText("v2", v2.x, v2.y, 10, DARKGRAY);

            if (angleMode == 0)
            {
                DrawText("MODE: Angle between V1 and V2", 10, 10, 20, BLACK);
                
                DrawLineEx(v0, v1, 2.0f, BLACK);
                DrawLineEx(v0, v2, 2.0f, RED);
                
                float startangle = 90 - Vector2LineAngle(v0, v1)*RAD2DEG;
                DrawCircleSector(v0, 40.0f, startangle, startangle + angle - 360.0f*(angle > 180.0f), 32, Fade(GREEN, 0.6f));
            }
            else if (angleMode == 1)
            {
                DrawText("MODE: Angle formed by line V1 to V2", 10, 10, 20, BLACK);
                
                DrawLine(0, screenHeight/2, screenWidth, screenHeight/2, LIGHTGRAY);
                DrawLineEx(v1, v2, 2.0f, RED);
                
                DrawCircleSector(v1, 40.0f, 90.0f, 180 - angle - 90, 32, Fade(GREEN, 0.6f));
            }
            
            DrawText("Press SPACE to change MODE", 460, 10, 20, DARKGRAY);
            DrawText(TextFormat("ANGLE: %2.2f", angle), 10, 40, 20, LIME);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
