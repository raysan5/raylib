/*******************************************************************************************
*
*   raylib [models] example - loading m3d
*
*   Example complexity rating: [★★☆☆] 2/4
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
*   Copyright (c) 2022-2025 bzt (@bztsrc)
*
********************************************************************************************/

#include "raylib.h"

static void DrawModelSkeleton(ModelSkeleton skeleton, ModelAnimPose pose, float scale, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - loading m3d");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 1.5f, 1.5f, 1.5f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.4f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load model
    Model model = LoadModel("resources/models/m3d/cesium_man.m3d");             // Load the animated model mesh and basic data
    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position

    // Load animation data
    int animCount = 0;
    ModelAnimation *anims = LoadModelAnimations("resources/models/m3d/cesium_man.m3d", &animCount);

    // Animation playing variables
    unsigned int animIndex = 0;         // Current animation playing
    float animCurrentFrame = 0.0f;      // Current animation frame (supporting interpolated frames)

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Select current animation
        if (IsKeyPressed(KEY_RIGHT)) animIndex = (animIndex + 1)%animCount;
        else if (IsKeyPressed(KEY_LEFT)) animIndex = (animIndex + animCount - 1)%animCount;

        // Update model animation
        animCurrentFrame += 1.0f;
        if (animCurrentFrame >= anims[animIndex].keyframeCount) animCurrentFrame = 0.0f;
        UpdateModelAnimation(model, anims[animIndex], animCurrentFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw 3d model with texture
                if (!IsKeyDown(KEY_SPACE)) DrawModel(model, position, 1.0f, WHITE);
                else
                {
                    // Draw the animated skeleton
                    DrawModelSkeleton(model.skeleton, anims[animIndex].keyframePoses[(int)animCurrentFrame], 1.0f, RED);
                }

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawText(TextFormat("Current animation: %s", anims[animIndex].name), 10, 10, 20, LIGHTGRAY);
            DrawText("Press SPACE to draw skeleton", 10, 40, 20, MAROON);
            DrawText("(c) CesiumMan model by KhronosGroup", GetScreenWidth() - 210, GetScreenHeight() - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModelAnimations(anims, animCount);   // Unload model animations data
    UnloadModel(model);                        // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Draw model skeleton
static void DrawModelSkeleton(ModelSkeleton skeleton, ModelAnimPose pose, float scale, Color color)
{
    // Loop to (boneCount - 1) because the last one is a special "no bone" bone,
    // needed to workaround buggy models without a -1, a cube is always drawn at the origin
    for (int i = 0; i < skeleton.boneCount - 1; i++)
    {
        // Display the frame-pose skeleton
        DrawCube(pose[i].translation, scale*0.05f, scale*0.05f, scale*0.05f, color);

        if (skeleton.bones[i].parent >= 0)
        {
            DrawLine3D(pose[i].translation, pose[skeleton.bones[i].parent].translation, color);
        }
    }
}