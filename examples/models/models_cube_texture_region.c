/*******************************************************************************************
*
*   raylib [core] example - Draw cube with texture regions
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Timon de Groot (@tdgroot) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d cube texture region");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

    // Load texture and define texture regions
    Texture2D texture = LoadTexture("resources/sheet.png");       // Load texture sheet
    Rectangle sprite1 = (Rectangle){ 0.0f, 0.0f, 32.0f, 32.0f };  // Define texture region for sprite 1
    Rectangle sprite2 = (Rectangle){ 32.0f, 0.0f, 32.0f, 32.0f }; // Define texture region for sprite 2

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);          // Update camera

        if (IsKeyDown('Z')) camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawCubeTextureRec(texture, sprite1, (Vector3){4.0f, 0.0f, -1.0f }, 2.0f, 2.0f, 2.0f, WHITE);
                DrawCubeTextureRec(texture, sprite2, (Vector3){-4.0f, 0.0f, 1.0f }, 2.0f, 2.0f, 2.0f, BLUE);

            EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
