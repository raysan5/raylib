/*******************************************************************************************
*
*   raylib [models] example - Drawing billboards
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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - drawing billboards");

    // Define the camera to look into our 3d world
    Camera camera = {{ 5.0, 4.0, 5.0 }, { 0.0, 2.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Texture2D bill = LoadTexture("resources/billboard.png");     // Our texture billboard
    Vector3 billPosition = { 0.0, 2.0, 0.0 };                   // Position where draw billboard
    
    SetCameraMode(CAMERA_ORBITAL);      // Set an orbital camera mode
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
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawBillboard(camera, bill, billPosition, 2.0f, WHITE);

                DrawGrid(10.0, 1.0);        // Draw a grid

            End3dMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(bill);        // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}