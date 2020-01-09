/*******************************************************************************************
*
*   Physac - Physics restitution
*
*   NOTE 1: Physac requires multi-threading, when InitPhysics() a second thread is created to manage physics calculations.
*   NOTE 2: Physac requires static C library linkage to avoid dependency on MinGW DLL (-static -lpthread)
*
*   Use the following line to compile:
*
*   gcc -o $(NAME_PART).exe $(FILE_NAME) -s -static  /
*       -lraylib -lpthread -lglfw3 -lopengl32 -lgdi32 -lopenal32 -lwinmm /
*       -std=c99 -Wl,--subsystem,windows -Wl,-allow-multiple-definition
*
*   Copyright (c) 2016-2018 Victor Fisac
*
********************************************************************************************/

#include "raylib.h"

#define PHYSAC_IMPLEMENTATION
#define PHYSAC_NO_THREADS
#include "physac.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Physac [raylib] - Physics restitution");

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

    // Restitution demo needs a very tiny physics time step for a proper simulation
    SetPhysicsTimeStep(1.0/60.0/100*1000);
    
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        RunPhysicsStep();

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
    DestroyPhysicsBody(circleA);
    DestroyPhysicsBody(circleB);
    DestroyPhysicsBody(circleC);
    DestroyPhysicsBody(floor);
    ClosePhysics();       // Unitialize physics

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
