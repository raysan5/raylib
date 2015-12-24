/*******************************************************************************************
*
*   raylib [physics] example - Basic rigidbody
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define OBJECT_SIZE 50
#define PLAYER_INDEX 0

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib [physics] example - basic rigidbody");
    SetTargetFPS(60);   // Enable v-sync
    InitPhysics();      // Initialize internal physics values   (max rigidbodies/colliders available: 1024)
    
    // Physics initialization
    Physics worldPhysics = {true, false, (Vector2){0, -9.81f}};
    
    // Set internal physics settings
    SetPhysics(worldPhysics);
    
    // Object initialization
    Transform player = (Transform){(Vector2){(screenWidth - OBJECT_SIZE) / 2, (screenHeight - OBJECT_SIZE) / 2}, 0.0f, (Vector2){OBJECT_SIZE, OBJECT_SIZE}};
    AddCollider(PLAYER_INDEX, (Collider){true, RectangleCollider, (Rectangle){player.position.x, player.position.y, player.scale.x, player.scale.y}, 0});
    AddRigidbody(PLAYER_INDEX, (Rigidbody){true, 1.0f, (Vector2){0, 0}, (Vector2){0, 0}, false, false, true, 0.5f, 1.0f});
    
    // Floor initialization 
    // NOTE: floor doesn't need a rigidbody because it's a static physic object, just a collider to collide with other dynamic colliders (with rigidbody)
    Transform floor = (Transform){(Vector2){0, screenHeight * 0.8f}, 0.0f, (Vector2){screenWidth, screenHeight * 0.2f}};
    AddCollider(PLAYER_INDEX + 1, (Collider){true, RectangleCollider, (Rectangle){floor.position.x, floor.position.y, floor.scale.x, floor.scale.y}, 0});
    
    // Object properties initialization
    float moveSpeed = 6.0f;
    float jumpForce = 4.5f;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Update object physics 
        // NOTE: all physics detections and reactions are calculated in ApplyPhysics() function (You will live happier :D)
        ApplyPhysics(PLAYER_INDEX, &player.position);
        
        // Check jump button input
        if(IsKeyDown(KEY_SPACE) && GetRigidbody(PLAYER_INDEX).isGrounded)
        {
            // Reset object Y velocity to avoid double jumping cases but keep the same X velocity that it already has
            SetRigidbodyVelocity(PLAYER_INDEX, (Vector2){GetRigidbody(PLAYER_INDEX).velocity.x, 0});
            
            // Add jumping force in Y axis
            AddRigidbodyForce(PLAYER_INDEX, (Vector2){0, jumpForce});
        }
        
        // Check movement buttons input
        if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            // Set rigidbody velocity in X based on moveSpeed value and apply the same Y velocity that it already has
            SetRigidbodyVelocity(PLAYER_INDEX, (Vector2){moveSpeed, GetRigidbody(PLAYER_INDEX).velocity.y});
        }
        else if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            // Set rigidbody velocity in X based on moveSpeed negative value and apply the same Y velocity that it already has
            SetRigidbodyVelocity(PLAYER_INDEX, (Vector2){-moveSpeed, GetRigidbody(PLAYER_INDEX).velocity.y});
        }
        
        // Check debug mode toggle button input
        if(IsKeyPressed(KEY_P))
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
            
            // Draw information
            DrawText("Use LEFT / RIGHT to MOVE and SPACE to JUMP", (screenWidth - MeasureText("Use LEFT / RIGHT to MOVE and SPACE to JUMP", 20)) / 2, screenHeight * 0.20f, 20, LIGHTGRAY);
            DrawText("Use P to switch DEBUG MODE", (screenWidth - MeasureText("Use P to switch DEBUG MODE", 20)) / 2, screenHeight * 0.3f, 20, LIGHTGRAY);
            
            // Check if debug mode is enabled
            if(worldPhysics.debug)
            {
                // Draw every internal physics stored collider if it is active
                for(int i = 0; i < 2; i++)
                {
                    if(GetCollider(i).enabled)
                    {
                        DrawRectangleLines(GetCollider(i).bounds.x, GetCollider(i).bounds.y, GetCollider(i).bounds.width, GetCollider(i).bounds.height, GREEN);
                    }
                }
                
            }
            else
            {
                // Draw player
                DrawRectangleRec((Rectangle){player.position.x, player.position.y, player.scale.x, player.scale.y}, GRAY);
                
                // Draw floor
                DrawRectangleRec((Rectangle){floor.position.x, floor.position.y, floor.scale.x, floor.scale.y}, BLACK);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------   
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}