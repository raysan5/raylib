/*******************************************************************************************
*
*   raylib [physac] physics example - Rigidbody forces
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_OBJECTS           5
#define OBJECTS_OFFSET      150

#define FORCE_INTENSITY  250.0f     // Customize by user
#define FORCE_RADIUS        100     // Customize by user    

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib [physics] example - rigidbodies forces");

    InitPhysics();      // Initialize internal physics values   (max rigidbodies/colliders available: 1024)
    
    // Physics initialization
    Physics worldPhysics = {true, false, (Vector2){0, -9.81f}};
    
    // Set internal physics settings
    SetPhysics(worldPhysics);
    
    // Objects initialization
    Transform objects[MAX_OBJECTS];
    
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        objects[i] = (Transform){(Vector2){75 + OBJECTS_OFFSET * i, (screenHeight - 50) / 2}, 0.0f, (Vector2){50, 50}};
        AddCollider(i, (Collider){true, COLLIDER_RECTANGLE, (Rectangle){objects[i].position.x, objects[i].position.y, objects[i].scale.x, objects[i].scale.y}, 0});
        AddRigidbody(i, (Rigidbody){true, 1.0f, (Vector2){0, 0}, (Vector2){0, 0}, false, false, true, 0.5f, 0.5f});
    }
    
    // Floor initialization 
    // NOTE: floor doesn't need a rigidbody because it's a static physic object, just a collider to collide with other dynamic colliders (with rigidbody)
    Transform floor = (Transform){(Vector2){0, screenHeight * 0.8f}, 0.0f, (Vector2){screenWidth, screenHeight * 0.2f}};
    AddCollider(MAX_OBJECTS, (Collider){true, COLLIDER_RECTANGLE, (Rectangle){floor.position.x, floor.position.y, floor.scale.x, floor.scale.y}, 0});

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Update object physics 
        // NOTE: all physics detections and reactions are calculated in ApplyPhysics() function (You will live happier :D)
        for (int i = 0; i < MAX_OBJECTS; i++)
        {
            ApplyPhysics(i, &objects[i].position);
        }
        
        // Check foce button input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            AddForceAtPosition(GetMousePosition(), FORCE_INTENSITY, FORCE_RADIUS);
        }
        
        // Check debug mode toggle button input
        if (IsKeyPressed(KEY_P))
        {
            // Update program physics value
            worldPhysics.debug = !worldPhysics.debug;
            
            // Update internal physics value
            SetPhysics(worldPhysics);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            // Check if debug mode is enabled
            if (worldPhysics.debug)
            {
                // Draw every internal physics stored collider if it is active (floor included)
                for (int i = 0; i < MAX_OBJECTS + 1; i++)
                {
                    if (GetCollider(i).enabled)
                    {
                        // Draw collider bounds
                        DrawRectangleLines(GetCollider(i).bounds.x, GetCollider(i).bounds.y, GetCollider(i).bounds.width, GetCollider(i).bounds.height, GREEN);
                        
                        // Check if current collider is not floor
                        if (i < MAX_OBJECTS)
                        {
                            // Draw lines between mouse position and objects if they are in force range
                            if (CheckCollisionPointCircle(GetMousePosition(), (Vector2){GetCollider(i).bounds.x + GetCollider(i).bounds.width / 2, GetCollider(i).bounds.y + GetCollider(i).bounds.height / 2}, FORCE_RADIUS))
                            {
                                DrawLineV(GetMousePosition(), (Vector2){GetCollider(i).bounds.x + GetCollider(i).bounds.width / 2, GetCollider(i).bounds.y + GetCollider(i).bounds.height / 2}, RED);
                            }
                        }
                    }
                }
                
                // Draw radius circle
                DrawCircleLines(GetMousePosition().x, GetMousePosition().y, FORCE_RADIUS, RED);
            }
            else
            {
                // Draw objects
                for (int i = 0; i < MAX_OBJECTS; i++)
                {
                    DrawRectangleRec((Rectangle){objects[i].position.x, objects[i].position.y, objects[i].scale.x, objects[i].scale.y}, GRAY);
                }
                
                // Draw floor
                DrawRectangleRec((Rectangle){floor.position.x, floor.position.y, floor.scale.x, floor.scale.y}, BLACK);
            }
            
                        
            // Draw help messages
            DrawText("Use LEFT MOUSE BUTTON to create a force in mouse position", (screenWidth - MeasureText("Use LEFT MOUSE BUTTON to create a force in mouse position", 20)) / 2, screenHeight * 0.20f, 20, LIGHTGRAY);
            DrawText("Use P to switch DEBUG MODE", (screenWidth - MeasureText("Use P to switch DEBUG MODE", 20)) / 2, screenHeight * 0.3f, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------   
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
