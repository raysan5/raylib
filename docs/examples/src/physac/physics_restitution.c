/*******************************************************************************************
*
*   Physac - Physics restitution
*
*   NOTE: Physac requires multi-threading, when InitPhysics() a second thread is created to manage physics calculations.
*   The file pthreadGC2.dll is required to run the program; you can find it in 'src\external'
*
*   Copyright (c) 2016 Victor Fisac
*
********************************************************************************************/

#include "raylib.h"

#define PHYSAC_IMPLEMENTATION
#include "..\src\physac.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Physac [raylib] - Physics restitution");
    SetTargetFPS(60);

    // Physac logo drawing position
    int logoX = screenWidth - MeasureText("Physac", 30) - 10;
    int logoY = 15;

    // Initialize physics and default physics bodies
    InitPhysics();

    // Create floor rectangle physics body
    PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2, screenHeight }, screenWidth, 100, 10);
    floor->enabled = false; // Disable body state to convert it to static (no dynamics, but collisions)
    floor->restitution = 1;

    // Create circles physics body
    PhysicsBody circleA = CreatePhysicsBodyCircle((Vector2){ screenWidth*0.25f, screenHeight/2 }, 30, 10);
    circleA->restitution = 0;
    PhysicsBody circleB = CreatePhysicsBodyCircle((Vector2){ screenWidth*0.5f, screenHeight/2 }, 30, 10);
    circleB->restitution = 0.5f;
    PhysicsBody circleC = CreatePhysicsBodyCircle((Vector2){ screenWidth*0.75f, screenHeight/2 }, 30, 10);
    circleC->restitution = 1;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed('R'))    // Reset physics input
        {
            // Reset circles physics bodies position and velocity
            circleA->position = (Vector2){ screenWidth*0.25f, screenHeight/2 };
            circleA->velocity = (Vector2){ 0, 0 };
            circleB->position = (Vector2){ screenWidth*0.5f, screenHeight/2 };
            circleB->velocity = (Vector2){ 0, 0 };
            circleC->position = (Vector2){ screenWidth*0.75f, screenHeight/2 };
            circleC->velocity = (Vector2){ 0, 0 };
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            DrawFPS(screenWidth - 90, screenHeight - 30);

            // Draw created physics bodies
            int bodiesCount = GetPhysicsBodiesCount();
            for (int i = 0; i < bodiesCount; i++)
            {
                PhysicsBody body = GetPhysicsBody(i);

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

            DrawText("Restitution amount", (screenWidth - MeasureText("Restitution amount", 30))/2, 75, 30, WHITE);
            DrawText("0", circleA->position.x - MeasureText("0", 20)/2, circleA->position.y - 7, 20, WHITE);
            DrawText("0.5", circleB->position.x - MeasureText("0.5", 20)/2, circleB->position.y - 7, 20, WHITE);
            DrawText("1", circleC->position.x - MeasureText("1", 20)/2, circleC->position.y - 7, 20, WHITE);

            DrawText("Press 'R' to reset example", 10, 10, 10, WHITE);

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
