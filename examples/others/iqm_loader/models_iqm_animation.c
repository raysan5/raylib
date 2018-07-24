/*******************************************************************************************
*
*   raylib [models] example - Load IQM 3d model with animations and play them
*
*   This example has been created using raylib 2.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2018 @culacant and @raysan5
*
********************************************************************************************/

#include "raylib.h"

#define RIQM_IMPLEMENTATION
#include "riqm.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - iqm animation");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;                   // Camera mode type

    // Load the animated model mesh and basic data
    AnimatedModel model = LoadAnimatedModel("resources/guy.iqm");

    // Load model texture and set material
    // NOTE: There is only 1 mesh and 1 material (both at index 0), thats what the 2 0's are
    model = AnimatedModelAddTexture(model, "resources/guytex.png");   // REPLACE!
    model = SetMeshMaterial(model, 0, 0);                             // REPLACE!

    // Load animation data
    Animation anim = LoadAnimationFromIQM("resources/guyanim.iqm");

    int animFrameCounter = 0;

    SetCameraMode(camera, CAMERA_FREE);  // Set free camera mode

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);

        // Play animation when spacebar is held down
        if (IsKeyDown(KEY_SPACE))
        {
            animFrameCounter++;
            AnimateModel(model, anim, animFrameCounter); // Animate the model with animation data and frame
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawAnimatedModel(model, Vector3Zero(), 1.0f, WHITE);  // Draw animated model

                DrawGrid(10, 1.0f);         // Draw a grid

            EndMode3D();
            
            DrawText("PRESS SPACE to PLAY IQM MODEL ANIMATION", 10, 10, 20, MAROON);

            DrawText("(c) Guy IQM 3D model by @culacant", screenWidth - 200, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadAnimation(anim);      // Unload animation data
    UnloadAnimatedModel(model); // Unload animated model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
