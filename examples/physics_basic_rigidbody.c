/*******************************************************************************************
*
*   raylib [physac] example - Basic rigidbody
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*
*   Compile example using:
*   cmd /c IF NOT EXIST pthreadGC2.dll copy C:\raylib\raylib\src\external\pthread\pthreadGC2.dll $(CURRENT_DIRECTORY) /Y
*
*   Copyright (c) 2016 Victor Fisac and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define PHYSAC_IMPLEMENTATION
#include "physac.h"

#define MOVE_VELOCITY    5
#define JUMP_VELOCITY    30

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [physac] example - basic rigidbody");
    InitPhysics((Vector2){ 0.0f, -9.81f/2 });      // Initialize physics module
    
    // Debug variables
    bool isDebug = false;
    
    // Create rectangle physic object
    PhysicBody rectangle = CreatePhysicBody((Vector2){ screenWidth*0.25f, screenHeight/2 }, 0.0f, (Vector2){ 75, 50 });
    rectangle->rigidbody.enabled = true;       // Enable physic object rigidbody behaviour
    rectangle->rigidbody.applyGravity = true;
    rectangle->rigidbody.friction = 0.1f;
    rectangle->rigidbody.bounciness = 6.0f;
    
    // Create square physic object
    PhysicBody square = CreatePhysicBody((Vector2){ screenWidth*0.75f, screenHeight/2 }, 0.0f, (Vector2){ 50, 50 });
    square->rigidbody.enabled = true;      // Enable physic object rigidbody behaviour
    square->rigidbody.applyGravity = true;
    square->rigidbody.friction = 0.1f;
    
    // Create walls physic objects
    PhysicBody floor = CreatePhysicBody((Vector2){ screenWidth/2, screenHeight*0.95f }, 0.0f, (Vector2){ screenWidth*0.9f, 100 });
    PhysicBody leftWall = CreatePhysicBody((Vector2){ 0.0f, screenHeight/2 }, 0.0f, (Vector2){ screenWidth*0.1f, screenHeight });
    PhysicBody rightWall = CreatePhysicBody((Vector2){ screenWidth, screenHeight/2 }, 0.0f, (Vector2){ screenWidth*0.1f, screenHeight });
    PhysicBody roof = CreatePhysicBody((Vector2){ screenWidth/2, screenHeight*0.05f }, 0.0f, (Vector2){ screenWidth*0.9f, 100 });
    
    // Create pplatform physic object
    PhysicBody platform = CreatePhysicBody((Vector2){ screenWidth/2, screenHeight*0.7f }, 0.0f, (Vector2){ screenWidth*0.25f, 20 });
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------        
        // Check rectangle movement inputs
        if (IsKeyPressed('W')) rectangle->rigidbody.velocity.y = JUMP_VELOCITY;
        if (IsKeyDown('A')) rectangle->rigidbody.velocity.x = -MOVE_VELOCITY;
        else if (IsKeyDown('D')) rectangle->rigidbody.velocity.x = MOVE_VELOCITY;
        
        // Check square movement inputs
        if (IsKeyDown(KEY_UP) && square->rigidbody.isGrounded) square->rigidbody.velocity.y = JUMP_VELOCITY;
        if (IsKeyDown(KEY_LEFT)) square->rigidbody.velocity.x = -MOVE_VELOCITY;
        else if (IsKeyDown(KEY_RIGHT)) square->rigidbody.velocity.x = MOVE_VELOCITY;
        
        // Check debug switch input
        if (IsKeyPressed('P')) isDebug = !isDebug;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw floor, roof and walls rectangles
            DrawRectangleRec(TransformToRectangle(floor->transform), DARKGRAY);         // Convert transform values to rectangle data type variable
            DrawRectangleRec(TransformToRectangle(leftWall->transform), DARKGRAY);
            DrawRectangleRec(TransformToRectangle(rightWall->transform), DARKGRAY);
            DrawRectangleRec(TransformToRectangle(roof->transform), DARKGRAY);
            
            // Draw middle platform rectangle
            DrawRectangleRec(TransformToRectangle(platform->transform), DARKGRAY);
            
            // Draw physic objects
            DrawRectangleRec(TransformToRectangle(rectangle->transform), RED);
            DrawRectangleRec(TransformToRectangle(square->transform), BLUE);
            
            // Draw collider lines if debug is enabled
            if (isDebug)
            {
                DrawRectangleLines(floor->collider.bounds.x, floor->collider.bounds.y, floor->collider.bounds.width, floor->collider.bounds.height, GREEN);
                DrawRectangleLines(leftWall->collider.bounds.x, leftWall->collider.bounds.y, leftWall->collider.bounds.width, leftWall->collider.bounds.height, GREEN);
                DrawRectangleLines(rightWall->collider.bounds.x, rightWall->collider.bounds.y, rightWall->collider.bounds.width, rightWall->collider.bounds.height, GREEN);
                DrawRectangleLines(roof->collider.bounds.x, roof->collider.bounds.y, roof->collider.bounds.width, roof->collider.bounds.height, GREEN);
                DrawRectangleLines(platform->collider.bounds.x, platform->collider.bounds.y, platform->collider.bounds.width, platform->collider.bounds.height, GREEN);
                DrawRectangleLines(rectangle->collider.bounds.x, rectangle->collider.bounds.y, rectangle->collider.bounds.width, rectangle->collider.bounds.height, GREEN);
                DrawRectangleLines(square->collider.bounds.x, square->collider.bounds.y, square->collider.bounds.width, square->collider.bounds.height, GREEN);
            }
            
            // Draw help message
            DrawText("Use WASD to move rectangle and ARROWS to move square", screenWidth/2 - MeasureText("Use WASD to move rectangle and ARROWS to move square", 20)/2, screenHeight*0.075f, 20, LIGHTGRAY);

            DrawFPS(10, 10);
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    ClosePhysics();       // Unitialize physics (including all loaded objects)
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}