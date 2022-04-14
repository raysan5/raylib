/*******************************************************************************************
*
*   raylib [physac] example - physics demo
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   This example uses physac 1.1 (https://github.com/raysan5/raylib/blob/master/src/physac.h)
*
*   Copyright (c) 2016-2021 Victor Fisac (@victorfisac) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [physac] example - physics demo");

    // Physac logo drawing position
    int logoX = screenWidth - MeasureText("Physac", 30) - 10;
    int logoY = 15;

    // Initialize physics and default physics bodies
    InitPhysics();

    // Create floor rectangle physics body
    PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2.0f, (float)screenHeight }, 500, 100, 10);
    floor->enabled = false;         // Disable body state to convert it to static (no dynamics, but collisions)

    // Create obstacle circle physics body
    PhysicsBody circle = CreatePhysicsBodyCircle((Vector2){ screenWidth/2.0f, screenHeight/2.0f }, 45, 10);
    circle->enabled = false;        // Disable body state to convert it to static (no dynamics, but collisions)

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdatePhysics();            // Update physics system

        if (IsKeyPressed(KEY_R))    // Reset physics system
        {
            ResetPhysics();

            floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2.0f, (float)screenHeight }, 500, 100, 10);
            floor->enabled = false;

            circle = CreatePhysicsBodyCircle((Vector2){ screenWidth/2.0f, screenHeight/2.0f }, 45, 10);
            circle->enabled = false;
        }

        // Physics body creation inputs
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) CreatePhysicsBodyPolygon(GetMousePosition(), (float)GetRandomValue(20, 80), GetRandomValue(3, 8), 10);
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) CreatePhysicsBodyCircle(GetMousePosition(), (float)GetRandomValue(10, 45), 10);

        // Destroy falling physics bodies
        int bodiesCount = GetPhysicsBodiesCount();
        for (int i = bodiesCount - 1; i >= 0; i--)
        {
            PhysicsBody body = GetPhysicsBody(i);
            if (body != NULL && (body->position.y > screenHeight*2)) DestroyPhysicsBody(body);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            DrawFPS(screenWidth - 90, screenHeight - 30);

            // Draw created physics bodies
            bodiesCount = GetPhysicsBodiesCount();
            for (int i = 0; i < bodiesCount; i++)
            {
                PhysicsBody body = GetPhysicsBody(i);

                if (body != NULL)
                {
                    int vertexCount = GetPhysicsShapeVerticesCount(i);
                    for (int j = 0; j < vertexCount; j++)
                    {
                        // Get physics bodies shape vertices to draw lines
                        // Note: GetPhysicsShapeVertex() already calculates rotation transformations
                        Vector2 vertexA = GetPhysicsShapeVertex(body, j);

                        int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
                        Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

                        DrawLineV(vertexA, vertexB, GREEN);     // Draw a line between two vertex positions
                    }
                }
            }

            DrawText("Left mouse button to create a polygon", 10, 10, 10, WHITE);
            DrawText("Right mouse button to create a circle", 10, 25, 10, WHITE);
            DrawText("Press 'R' to reset example", 10, 40, 10, WHITE);

            DrawText("Physac", logoX, logoY, 30, WHITE);
            DrawText("Powered by", logoX + 50, logoY - 7, 10, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    ClosePhysics();       // Unitialize physics

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
