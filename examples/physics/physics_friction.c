/*******************************************************************************************
*
*   raylib [physac] example - physics friction
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
    InitWindow(screenWidth, screenHeight, "raylib [physac] example - physics friction");

    // Physac logo drawing position
    int logoX = screenWidth - MeasureText("Physac", 30) - 10;
    int logoY = 15;

    // Initialize physics and default physics bodies
    InitPhysics();

    // Create floor rectangle physics body
    PhysicsBody floor = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2.0f, (float)screenHeight }, (float)screenWidth, 100, 10);
    floor->enabled = false; // Disable body state to convert it to static (no dynamics, but collisions)
    PhysicsBody wall = CreatePhysicsBodyRectangle((Vector2){ screenWidth/2.0f, screenHeight*0.8f }, 10, 80, 10);
    wall->enabled = false; // Disable body state to convert it to static (no dynamics, but collisions)

    // Create left ramp physics body
    PhysicsBody rectLeft = CreatePhysicsBodyRectangle((Vector2){ 25, (float)screenHeight - 5 }, 250, 250, 10);
    rectLeft->enabled = false; // Disable body state to convert it to static (no dynamics, but collisions)
    SetPhysicsBodyRotation(rectLeft, 30*DEG2RAD);

    // Create right ramp  physics body
    PhysicsBody rectRight = CreatePhysicsBodyRectangle((Vector2){ (float)screenWidth - 25, (float)screenHeight - 5 }, 250, 250, 10);
    rectRight->enabled = false; // Disable body state to convert it to static (no dynamics, but collisions)
    SetPhysicsBodyRotation(rectRight, 330*DEG2RAD);

    // Create dynamic physics bodies
    PhysicsBody bodyA = CreatePhysicsBodyRectangle((Vector2){ 35, screenHeight*0.6f }, 40, 40, 10);
    bodyA->staticFriction = 0.1f;
    bodyA->dynamicFriction = 0.1f;
    SetPhysicsBodyRotation(bodyA, 30*DEG2RAD);

    PhysicsBody bodyB = CreatePhysicsBodyRectangle((Vector2){ (float)screenWidth - 35, (float)screenHeight*0.6f }, 40, 40, 10);
    bodyB->staticFriction = 1.0f;
    bodyB->dynamicFriction = 1.0f;
    SetPhysicsBodyRotation(bodyB, 330*DEG2RAD);

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
            // Reset dynamic physics bodies position, velocity and rotation
            bodyA->position = (Vector2){ 35, screenHeight*0.6f };
            bodyA->velocity = (Vector2){ 0, 0 };
            bodyA->angularVelocity = 0;
            SetPhysicsBodyRotation(bodyA, 30*DEG2RAD);

            bodyB->position = (Vector2){ (float)screenWidth - 35, screenHeight * 0.6f };
            bodyB->velocity = (Vector2){ 0, 0 };
            bodyB->angularVelocity = 0;
            SetPhysicsBodyRotation(bodyB, 330*DEG2RAD);
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

            DrawRectangle(0, screenHeight - 49, screenWidth, 49, BLACK);

            DrawText("Friction amount", (screenWidth - MeasureText("Friction amount", 30))/2.0f, 75, 30, WHITE);
            DrawText("0.1", (int)bodyA->position.x - MeasureText("0.1", 20)/2, (int)bodyA->position.y - 7, 20, WHITE);
            DrawText("1", (int)bodyB->position.x - MeasureText("1", 20)/2, (int)bodyB->position.y - 7, 20, WHITE);

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
