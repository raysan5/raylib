/*******************************************************************************************
*
*   raylib [models] example - Load 3d gltf model
*
*   This example has been created using raylib 3.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Hristo Stamenov (@object71) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2021 Hristo Stamenov (@object71) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
* To export a model from blender, make sure it is not posed, the vertices need to be in the
* same position as they would be in edit mode.
* and that the scale of your models is set to 0. Scaling can be done from the export menu.
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>

#define MAX_MODELS  6

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - model animation");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    Model model[MAX_MODELS] = { 0 };

    model[0] = LoadModel("resources/gltf/raylib_32x32.glb");
    model[1] = LoadModel("resources/gltf/rigged_figure.glb");
    model[2] = LoadModel("resources/gltf/GearboxAssy.glb");
    model[3] = LoadModel("resources/gltf/BoxAnimated.glb");
    model[4] = LoadModel("resources/gltf/AnimatedTriangle.gltf");
    model[5] = LoadModel("resources/gltf/AnimatedMorphCube.glb");

    int currentModel = 0;

    Vector3 position = { 0.0f, 0.0f, 0.0f };    // Set model position

    SetCameraMode(camera, CAMERA_FREE); // Set free camera mode

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);

        if (IsKeyReleased(KEY_RIGHT))
        {
            currentModel++;
            if (currentModel == MAX_MODELS) currentModel = 0;
        }

        if (IsKeyReleased(KEY_LEFT))
        {
            currentModel--;
            if (currentModel < 0) currentModel = MAX_MODELS - 1;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(SKYBLUE);

            BeginMode3D(camera);

                DrawModelEx(model[currentModel], position, (Vector3){ 0.0f, 1.0f, 0.0f }, 180.0f, (Vector3){ 2.0f, 2.0f, 2.0f }, WHITE);

                DrawGrid(10, 1.0f);         // Draw a grid

            EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for(int i = 0; i < MAX_MODELS; i++) UnloadModel(model[i]);  // Unload models

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
