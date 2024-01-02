/*******************************************************************************************
*
*   raylib [models] example - Plane rotations (yaw, pitch, roll)
*
*   Example originally created with raylib 1.8, last time updated with raylib 4.0
*
*   Example contributed by Berni (@Berni8k) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2024 Berni (@Berni8k) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"        // Required for: MatrixRotateXYZ()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    //SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - plane rotations (yaw, pitch, roll)");

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 50.0f, -120.0f };// Camera position perspective
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 30.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera type

    Model model = LoadModel("resources/models/obj/plane.obj");                  // Load model
    Texture2D texture = LoadTexture("resources/models/obj/plane_diffuse.png");  // Load model texture
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;            // Set map diffuse texture

    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Plane pitch (x-axis) controls
        if (IsKeyDown(KEY_DOWN)) pitch += 0.6f;
        else if (IsKeyDown(KEY_UP)) pitch -= 0.6f;
        else
        {
            if (pitch > 0.3f) pitch -= 0.3f;
            else if (pitch < -0.3f) pitch += 0.3f;
        }

        // Plane yaw (y-axis) controls
        if (IsKeyDown(KEY_S)) yaw -= 1.0f;
        else if (IsKeyDown(KEY_A)) yaw += 1.0f;
        else
        {
            if (yaw > 0.0f) yaw -= 0.5f;
            else if (yaw < 0.0f) yaw += 0.5f;
        }

        // Plane roll (z-axis) controls
        if (IsKeyDown(KEY_LEFT)) roll -= 1.0f;
        else if (IsKeyDown(KEY_RIGHT)) roll += 1.0f;
        else
        {
            if (roll > 0.0f) roll -= 0.5f;
            else if (roll < 0.0f) roll += 0.5f;
        }

        // Tranformation matrix for rotations
        model.transform = MatrixRotateXYZ((Vector3){ DEG2RAD*pitch, DEG2RAD*yaw, DEG2RAD*roll });
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw 3D model (recomended to draw 3D always before 2D)
            BeginMode3D(camera);

                DrawModel(model, (Vector3){ 0.0f, -8.0f, 0.0f }, 1.0f, WHITE);   // Draw 3d model with texture
                DrawGrid(10, 10.0f);

            EndMode3D();

            // Draw controls info
            DrawRectangle(30, 370, 260, 70, Fade(GREEN, 0.5f));
            DrawRectangleLines(30, 370, 260, 70, Fade(DARKGREEN, 0.5f));
            DrawText("Pitch controlled with: KEY_UP / KEY_DOWN", 40, 380, 10, DARKGRAY);
            DrawText("Roll controlled with: KEY_LEFT / KEY_RIGHT", 40, 400, 10, DARKGRAY);
            DrawText("Yaw controlled with: KEY_A / KEY_S", 40, 420, 10, DARKGRAY);

            DrawText("(c) WWI Plane Model created by GiaHanLam", screenWidth - 240, screenHeight - 20, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);     // Unload model data

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
