/*******************************************************************************************
*
*   raylib [models] example - Load M3D model (with optional animations) and play them
*
*   Example static mesh Suzanne from Blender
*   Example animated seagull model from Scorched 3D, licensed GPLv2
*
*   Copyright (c) 2019-2022 Culacant (@culacant) and Ramon Santamaria (@raysan5)
*   Copyright (c) 2022 bzt (@bztsrc)
*
********************************************************************************************
*
*   NOTE: To export a model from blender, just use https://gitlab.com/bztsrc/model3d/-/tree/master/blender
*         and make sure to add "(action)" markers to the timeline if you want multiple animations.
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    char *model_fn = argc > 1 ? argv[1] : "resources/models/m3d/suzanne.m3d";

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - M3D model");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
    Vector3 position = { 0.0f, 0.0f, 0.0f };            // Set model position

    // Load model
    Model model = LoadModel(model_fn); // Load the animated model mesh and basic data

    // Load animation data
    unsigned int animsCount = 0;
    ModelAnimation *anims = LoadModelAnimations(model_fn, &animsCount);
    int animFrameCounter = 0, animId = 0;

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
        if (animsCount)
        {
            if (IsKeyDown(KEY_SPACE))
            {
                animFrameCounter++;
                UpdateModelAnimation(model, anims[animId], animFrameCounter);
                if (animFrameCounter >= anims[animId].frameCount) animFrameCounter = 0;
            }

            // Select animation on mouse click
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                animFrameCounter = 0;
                animId++;
                if (animId >= animsCount) animId = 0;
                UpdateModelAnimation(model, anims[animId], 0);
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, position, 1.0f, WHITE);        // Draw 3d model with texture
                if(anims)
                    for (int i = 0; i < model.boneCount; i++)
                    {
                        DrawCube(anims[animId].framePoses[animFrameCounter][i].translation, 0.2f, 0.2f, 0.2f, RED);
                    }

                DrawGrid(10, 1.0f);         // Draw a grid

            EndMode3D();

            DrawText("PRESS SPACE to PLAY MODEL ANIMATION", 10, GetScreenHeight() - 30, 10, MAROON);
            DrawText("MOUSE LEFT BUTTON to CYCLE THROUGH ANIMATIONS", 10, GetScreenHeight() - 20, 10, DARKGRAY);
            DrawText("(c) Suzanne 3D model by blender", screenWidth - 200, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload model animations data
    for (unsigned int i = 0; i < animsCount; i++) UnloadModelAnimation(anims[i]);
    RL_FREE(anims);

    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
