/*******************************************************************************************
*
*   raylib [models] example - Load 3d model with animations and play them
*
*   This example has been created using raylib 3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Tyler Bezera (@gamerfiend) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Tyler Bezera (@gamerfiend) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <stdlib.h>
#include "raylib.h"
#define RGLTFANIMATION_IMPLEMENTATION
#include "rgltfanim.h"



int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - model animation gltf");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.type = CAMERA_PERSPECTIVE;                   // Camera mode type

    // Load the animated model mesh and basic data
    Model model = LoadModel("resources/models/RiggedFigure.glb");               

    Vector3 position = { 0.0f, 0.0f, 0.0f };            // Set model position

    // Load animation data
    // int animsCount = 0;
    ModelAnimationsGLTF anims = LoadModelGLTFAnimations("resources/models/RiggedFigure.glb");
    // int animFrameCounter = 0;

    SetCameraMode(camera, CAMERA_FREE); // Set free camera mode

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);

        // Play animation when spacebar is held down
        if (IsKeyDown(KEY_SPACE))
        {
            // animFrameCounter++;
            UpdateModelAnimationGLTF(model, 0, 0.5);
            // if (animFrameCounter >= anims[0].frameCount) animFrameCounter = 0;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModelEx(model, position, (Vector3){ 1.0f, 0.0f, 0.0f }, -90.0f, (Vector3){ 1.0f, 1.0f, 1.0f }, WHITE);

                for (int i = 0; i < model.boneCount; i++)
                {
                    //DrawCube(anims[0].framePoses[animFrameCounter][i].translation, 0.2f, 0.2f, 0.2f, RED);
                }

                DrawGrid(10, 1.0f);         // Draw a grid

            EndMode3D();

            DrawText("PRESS SPACE to PLAY MODEL ANIMATION", 10, 10, 20, MAROON);
            DrawText("(c) Guy IQM 3D model by @culacant", screenWidth - 200, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    
    // Unload model animations data
    // for (int i = 0; i < animsCount; i++) UnloadModelAnimation(anims[i]);
    // RL_FREE(anims);

    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
