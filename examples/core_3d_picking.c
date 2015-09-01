/*******************************************************************************************
*
*   raylib [core] example - Picking in 3d mode
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

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d picking");

    // Define the camera to look into our 3d world
    Camera camera = {{ 0.0, 10.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Vector3 cubePosition = { 0.0, 1.0, 0.0 };
    
    Ray ray;        // Picking line ray
    
    SetCameraMode(CAMERA_FREE);         // Set a free camera mode
    SetCameraPosition(camera.position); // Set internal camera position to match our camera position

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update internal camera and our camera
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // NOTE: This function is NOT WORKING properly!
            ray = GetMouseRay(GetMousePosition(), camera);
            
            // TODO: Check collision between ray and box
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawCube(cubePosition, 2, 2, 2, GRAY);
                DrawCubeWires(cubePosition, 2, 2, 2, DARKGRAY);

                DrawGrid(10.0, 1.0);
                
                DrawRay(ray, MAROON);

            End3dMode();
            
            DrawText("Try selecting the box with mouse!", 240, 10, 20, GRAY);

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