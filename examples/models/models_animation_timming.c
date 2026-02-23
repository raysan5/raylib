/*******************************************************************************************
*
*   raylib [models] example - animation timming
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"                         // Required for: UI controls

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - animation timming");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 6.0f, 6.0f, 6.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load model
    Model model = LoadModel("resources/models/gltf/robot.glb");
    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model world position

    // Load model animations
    int animsCount = 0;
    ModelAnimation *modelAnimations = LoadModelAnimations("resources/models/gltf/robot.glb", &animsCount);

    // Animation playing variables
    unsigned int animIndex = 0;         // Current animation playing
    float animCurrentFrame = 0.0f;      // Current animation frame (supporting interpolated frames)
    float animFrameSpeed = 0.1f;        // Animation play speed

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Select current animation
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) animIndex = (animIndex + 1)%animsCount;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) animIndex = (animIndex + animsCount - 1)%animsCount;

        // Select animation playing speed
        if (IsKeyPressed(KEY_RIGHT)) animFrameSpeed += 0.1f;
        else if (IsKeyPressed(KEY_LEFT)) animFrameSpeed -= 0.1f;

        // Update model animation
        animCurrentFrame += animFrameSpeed;
        UpdateModelAnimation(model, modelAnimations[animIndex], animCurrentFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, position, 1.0f, WHITE);
            
                DrawGrid(10, 1.0f);
            
            EndMode3D();

            // Draw UI
            //GuiDropdownBox((Rectangle){ 10, 20, 240, 30 }, "text", &animIndex, editMode);

            DrawText(TextFormat("FRAME SPEED: x%.1f", animFrameSpeed), 10, 40, 20, RED);

            DrawText("Use the LEFT/RIGHT mouse buttons to switch animation", 10, 10, 20, GRAY);
            DrawText(TextFormat("Animation: %s", modelAnimations[animIndex].name), 10, GetScreenHeight() - 20, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);         // Unload model and meshes/material

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}



