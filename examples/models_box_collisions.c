/*******************************************************************************************
*
*   raylib [models] example - Detect basic 3d collisions (box vs sphere vs box)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - box collisions");

    // Define the camera to look into our 3d world
    Camera camera = {{ 0.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    Vector3 playerPosition = { 0, 1, 2 };
    Vector3 playerSize = { 1, 2, 1 };
    Color playerColor = GREEN;
    
    Vector3 enemyBoxPos = { -4, 1, 0 };
    Vector3 enemyBoxSize = { 2, 2, 2 };
    
    Vector3 enemySpherePos = { 4, 0, 0 };
    float enemySphereSize = 1.5f;
    
    bool collision = false;

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Move player
        if (IsKeyDown(KEY_RIGHT)) playerPosition.x += 0.2f;
        else if (IsKeyDown(KEY_LEFT)) playerPosition.x -= 0.2f;
        else if (IsKeyDown(KEY_DOWN)) playerPosition.z += 0.2f;
        else if (IsKeyDown(KEY_UP)) playerPosition.z -= 0.2f;
        
        collision = false;
        
        // Check collisions player vs enemy-box
        if (CheckCollisionBoxes((Vector3){ playerPosition.x - playerSize.x/2, 
                                           playerPosition.y - playerSize.y/2, 
                                           playerPosition.z - playerSize.z/2 }, 
                                (Vector3){ playerPosition.x + playerSize.x/2,
                                           playerPosition.y + playerSize.y/2, 
                                           playerPosition.z + playerSize.z/2 },
                                (Vector3){ enemyBoxPos.x - enemyBoxSize.x/2, 
                                           enemyBoxPos.y - enemyBoxSize.y/2, 
                                           enemyBoxPos.z - enemyBoxSize.z/2 }, 
                                (Vector3){ enemyBoxPos.x + enemyBoxSize.x/2,
                                           enemyBoxPos.y + enemyBoxSize.y/2, 
                                           enemyBoxPos.z + enemyBoxSize.z/2 })) collision = true;
        
        // Check collisions player vs enemy-sphere
        if (CheckCollisionBoxSphere((Vector3){ playerPosition.x - playerSize.x/2, 
                                               playerPosition.y - playerSize.y/2, 
                                               playerPosition.z - playerSize.z/2 }, 
                                    (Vector3){ playerPosition.x + playerSize.x/2,
                                               playerPosition.y + playerSize.y/2, 
                                               playerPosition.z + playerSize.z/2 }, 
                                               enemySpherePos, enemySphereSize)) collision = true;
        
        if (collision) playerColor = RED;
        else playerColor = GREEN;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                // Draw enemy-box
                DrawCube(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, GRAY);
                DrawCubeWires(enemyBoxPos, enemyBoxSize.x, enemyBoxSize.y, enemyBoxSize.z, DARKGRAY);
                
                // Draw enemy-sphere
                DrawSphere(enemySpherePos, enemySphereSize, GRAY);
                DrawSphereWires(enemySpherePos, enemySphereSize, 16, 16, DARKGRAY);
                
                // Draw player
                DrawCubeV(playerPosition, playerSize, playerColor);

                DrawGrid(10.0, 1.0);        // Draw a grid

            End3dMode();
            
            DrawText("Move player with cursors to collide", 220, 40, 20, GRAY);

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