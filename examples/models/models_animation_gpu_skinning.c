/*******************************************************************************************
*
*   raylib [models] example - animation gpu skinning
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Daniel Holden (@orangeduck) and reviewed by Ramon Santamaria (@raysan5)
*
*   WARNING: GPU skinning must be enabled in raylib with a compilation flag,
*   if not enabled, CPU skinning will be used instead
*   NOTE: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2025 Daniel Holden (@orangeduck)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - animation gpu skinning");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    // Load gltf model
    Model model = LoadModel("resources/models/gltf/greenman.glb"); // Load character model
    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position

    // Load skinning shader
    // WARNING: GPU skinning must be enabled in raylib with a compilation flag,
    // if not enabled, CPU skinning will be used instead
    Shader skinningShader = LoadShader(TextFormat("resources/shaders/glsl%i/skinning.vs", GLSL_VERSION),
                                       TextFormat("resources/shaders/glsl%i/skinning.fs", GLSL_VERSION));
    model.materials[1].shader = skinningShader;

    // Load gltf model animations
    int animCount = 0;
    ModelAnimation *anims = LoadModelAnimations("resources/models/gltf/greenman.glb", &animCount);

    // Animation playing variables
    unsigned int animIndex = 0;         // Current animation playing
    unsigned int animCurrentFrame = 0;  // Current animation frame

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
        animCurrentFrame = (animCurrentFrame + 1)%anims[animIndex].keyframeCount;
        UpdateModelAnimation(model, anims[animIndex], (float)animCurrentFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, position, 1.0f, WHITE);

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawText(TextFormat("Current animation: %s", anims[animIndex].name), 10, 40, 20, MAROON);
            DrawText("Use the LEFT/RIGHT keys to switch animation", 10, 10, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModelAnimations(anims, animCount); // Unload model animation
    UnloadModel(model);             // Unload model and meshes/material
    UnloadShader(skinningShader);   // Unload GPU skinning shader

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}