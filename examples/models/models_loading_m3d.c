/*******************************************************************************************
*
*   raylib [models] example - Load models M3D
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by bzt (@bztsrc) and reviewed by Ramon Santamaria (@raysan5)
*
*   NOTES:
*     - Model3D (M3D) fileformat specs: https://gitlab.com/bztsrc/model3d
*     - Bender M3D exported: https://gitlab.com/bztsrc/model3d/-/tree/master/blender
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2023 bzt (@bztsrc)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - M3D model loading");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 1.5f, 1.5f, 1.5f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.4f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Vector3 position = { 0.0f, 0.0f, 0.0f };            // Set model position

    char modelFileName[128] = "resources/models/m3d/cesium_man.m3d";
    bool drawMesh = 1;
    bool drawSkeleton = 1;
    bool animPlaying = false;   // Store anim state, what to draw

    // Load model
    Model model = LoadModel(modelFileName); // Load the bind-pose model mesh and basic data

    // Load animations
    int animsCount = 0;
    int animFrameCounter = 0, animId = 0;
    ModelAnimation *anims = LoadModelAnimations(modelFileName, &animsCount); // Load skeletal animation data

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        if (animsCount)
        {
            // Play animation when spacebar is held down (or step one frame with N)
            if (IsKeyDown(KEY_SPACE) || IsKeyPressed(KEY_N))
            {
                animFrameCounter++;

                if (animFrameCounter >= anims[animId].frameCount) animFrameCounter = 0;

                UpdateModelAnimation(model, anims[animId], animFrameCounter);
                animPlaying = true;
            }

            // Select animation by pressing C
            if (IsKeyPressed(KEY_C))
            {
                animFrameCounter = 0;
                animId++;

                if (animId >= (int)animsCount) animId = 0;
                UpdateModelAnimation(model, anims[animId], 0);
                animPlaying = true;
            }
        }

        // Toggle skeleton drawing
        if (IsKeyPressed(KEY_B)) drawSkeleton ^= 1;

        // Toggle mesh drawing
        if (IsKeyPressed(KEY_M)) drawMesh ^= 1;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw 3d model with texture
                if (drawMesh) DrawModel(model, position, 1.0f, WHITE);

                // Draw the animated skeleton
                if (drawSkeleton)
                {
                    // Loop to (boneCount - 1) because the last one is a special "no bone" bone,
                    // needed to workaround buggy models
                    // without a -1, we would always draw a cube at the origin
                    for (int i = 0; i < model.boneCount - 1; i++)
                    {
                        // By default the model is loaded in bind-pose by LoadModel().
                        // But if UpdateModelAnimation() has been called at least once
                        // then the model is already in animation pose, so we need the animated skeleton
                        if (!animPlaying || !animsCount)
                        {
                            // Display the bind-pose skeleton
                            DrawCube(model.bindPose[i].translation, 0.04f, 0.04f, 0.04f, RED);

                            if (model.bones[i].parent >= 0)
                            {
                                DrawLine3D(model.bindPose[i].translation,
                                    model.bindPose[model.bones[i].parent].translation, RED);
                            }
                        }
                        else
                        {
                            // Display the frame-pose skeleton
                            DrawCube(anims[animId].framePoses[animFrameCounter][i].translation, 0.05f, 0.05f, 0.05f, RED);

                            if (anims[animId].bones[i].parent >= 0)
                            {
                                DrawLine3D(anims[animId].framePoses[animFrameCounter][i].translation,
                                    anims[animId].framePoses[animFrameCounter][anims[animId].bones[i].parent].translation, RED);
                            }
                        }
                    }
                }

                DrawGrid(10, 1.0f);         // Draw a grid

            EndMode3D();

            DrawText("PRESS SPACE to PLAY MODEL ANIMATION", 10, GetScreenHeight() - 80, 10, MAROON);
            DrawText("PRESS N to STEP ONE ANIMATION FRAME", 10, GetScreenHeight() - 60, 10, DARKGRAY);
            DrawText("PRESS C to CYCLE THROUGH ANIMATIONS", 10, GetScreenHeight() - 40, 10, DARKGRAY);
            DrawText("PRESS M to toggle MESH, B to toggle SKELETON DRAWING", 10, GetScreenHeight() - 20, 10, DARKGRAY);
            DrawText("(c) CesiumMan model by KhronosGroup", GetScreenWidth() - 210, GetScreenHeight() - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload model animations data
    UnloadModelAnimations(anims, animsCount);

    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
