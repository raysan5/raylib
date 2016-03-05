/*******************************************************************************************
*
*   raylib [physac] example - Basic rigidbody
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Victor Fisac and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MOVE_VELOCITY    5
#define JUMP_VELOCITY    35

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [physac] example - basic rigidbody");
    InitPhysics();      // Initialize physics module
    
    SetTargetFPS(60);
    
    // Debug variables
    bool isDebug = false;
    
    // Player physic object
    PhysicObject *player = CreatePhysicObject((Vector2){ screenWidth*0.25f, screenHeight/2 }, 0.0f, (Vector2){ 50, 50 });
    player->rigidbody.enabled = true;       // Enable physic object rigidbody behaviour
    player->rigidbody.applyGravity = true;
    player->rigidbody.friction = 0.3f;
    player->collider.enabled = true;        // Enable physic object collisions detection
    
    // Player physic object
    PhysicObject *player2 = CreatePhysicObject((Vector2){ screenWidth*0.75f, screenHeight/2 }, 0.0f, (Vector2){ 50, 50 });
    player2->rigidbody.enabled = true;
    player2->rigidbody.applyGravity = true;
    player2->rigidbody.friction = 0.1f;
    player2->collider.enabled = true;
    
    // Floor physic object
    PhysicObject *floor = CreatePhysicObject((Vector2){ screenWidth/2, screenHeight*0.95f }, 0.0f, (Vector2){ screenWidth*0.9f, 100 });
    floor->collider.enabled = true;         // Enable just physic object collisions detection
    
    // Left wall physic object
    PhysicObject *leftWall = CreatePhysicObject((Vector2){ 0.0f, screenHeight/2 }, 0.0f, (Vector2){ screenWidth*0.1f, screenHeight });
    leftWall->collider.enabled = true;
    
    // Right wall physic object
    PhysicObject *rightWall = CreatePhysicObject((Vector2){ screenWidth, screenHeight/2 }, 0.0f, (Vector2){ screenWidth*0.1f, screenHeight });
    rightWall->collider.enabled = true;
    
    // Platform physic objectdd
    PhysicObject *platform = CreatePhysicObject((Vector2){ screenWidth/2, screenHeight*0.7f }, 0.0f, (Vector2){ screenWidth*0.25f, 20 });
    platform->collider.enabled = true;
    
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdatePhysics();    // Update all created physic objects
        
        // Check debug switch input
        if (IsKeyPressed('P')) isDebug = !isDebug;
        
        // Check player movement inputs
        if (IsKeyDown('W') && player->rigidbody.isGrounded) player->rigidbody.velocity.y = JUMP_VELOCITY;
        
        if (IsKeyDown('A')) player->rigidbody.velocity.x = -MOVE_VELOCITY;
        else if (IsKeyDown('D')) player->rigidbody.velocity.x = MOVE_VELOCITY;
        
        // Check player 2 movement inputs
        if (IsKeyDown(KEY_UP) && player2->rigidbody.isGrounded) player2->rigidbody.velocity.y = JUMP_VELOCITY;
        
        if (IsKeyDown(KEY_LEFT)) player2->rigidbody.velocity.x = -MOVE_VELOCITY;
        else if (IsKeyDown(KEY_RIGHT)) player2->rigidbody.velocity.x = MOVE_VELOCITY;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            if (isDebug)
            {
                DrawRectangleLines(floor->collider.bounds.x, floor->collider.bounds.y, floor->collider.bounds.width, floor->collider.bounds.height, GREEN);
                DrawRectangleLines(leftWall->collider.bounds.x, leftWall->collider.bounds.y, leftWall->collider.bounds.width, leftWall->collider.bounds.height, GREEN);
                DrawRectangleLines(rightWall->collider.bounds.x, rightWall->collider.bounds.y, rightWall->collider.bounds.width, rightWall->collider.bounds.height, GREEN);
                DrawRectangleLines(platform->collider.bounds.x, platform->collider.bounds.y, platform->collider.bounds.width, platform->collider.bounds.height, GREEN);
                DrawRectangleLines(player->collider.bounds.x, player->collider.bounds.y, player->collider.bounds.width, player->collider.bounds.height, GREEN);
                DrawRectangleLines(player2->collider.bounds.x, player2->collider.bounds.y, player2->collider.bounds.width, player2->collider.bounds.height, GREEN);
            }
            else
            {
                // Convert transform values to rectangle data type variable
                DrawRectangleRec(TransformToRectangle(floor->transform), DARKGRAY);
                DrawRectangleRec(TransformToRectangle(leftWall->transform), DARKGRAY);
                DrawRectangleRec(TransformToRectangle(rightWall->transform), DARKGRAY);
                DrawRectangleRec(TransformToRectangle(platform->transform), DARKGRAY);
                DrawRectangleRec(TransformToRectangle(player->transform), RED);
                DrawRectangleRec(TransformToRectangle(player2->transform), BLUE);
            }
            
            // Draw all physic object information in specific screen position and font size
            // DrawPhysicObjectInfo(player, (Vector2){ 10.0f, 10.0f }, 10);    

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    ClosePhysics();       // Unitialize physics module
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}