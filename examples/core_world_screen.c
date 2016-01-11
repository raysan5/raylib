/*******************************************************************************************
*
*   raylib [core] example - World to screen
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera free");

    // Define the camera to look into our 3d world
    Camera camera = {{ 0.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Vector3 cubePosition = { 0.0, 0.0, 0.0 };
    
    Vector2 cubeScreenPosition;
    
    SetCameraMode(CAMERA_FREE);         // Set a free camera mode
    SetCameraPosition(camera.position); // Set internal camera position to match our camera position
    SetCameraTarget(camera.target);     // Set internal camera target to match our camera target

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update internal camera and our camera
        
        // Calculate cube screen space position (with a little offset to be in top)
        cubeScreenPosition = WorldToScreen((Vector3){cubePosition.x, cubePosition.y + 2.5f, cubePosition.z}, camera);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawCube(cubePosition, 2, 2, 2, RED);
                DrawCubeWires(cubePosition, 2, 2, 2, MAROON);

                DrawGrid(10.0, 1.0);

            End3dMode();
            
            DrawText("Enemy: 100 / 100", cubeScreenPosition.x - MeasureText("Enemy: 100 / 100", 20) / 2, cubeScreenPosition.y, 20, BLACK);
            DrawText("Text is always on top of the cube", (screenWidth - MeasureText("Text is always on top of the cube", 20)) / 2, 25, 20, GRAY);
            

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}