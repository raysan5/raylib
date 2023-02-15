/*******************************************************************************************
*
*   raylib [models] example - Load models vox (MagicaVoxel)
*
*   Example originally created with raylib 4.0, last time updated with raylib 4.0
*
*   Example contributed by Johann Nadalutti (@procfxgen) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2021-2023 Johann Nadalutti (@procfxgen) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"        // Required for: MatrixTranslate()

#define MAX_VOX_FILES  3

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    const char *voxFileNames[] = {
        "resources/models/vox/chr_knight.vox",
        "resources/models/vox/chr_sword.vox",
        "resources/models/vox/monu9.vox"
    };

    InitWindow(screenWidth, screenHeight, "raylib [models] example - magicavoxel loading");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load MagicaVoxel files
    Model models[MAX_VOX_FILES] = { 0 };

    for (int i = 0; i < MAX_VOX_FILES; i++)
    {
        // Load VOX file and measure time
        double t0 = GetTime()*1000.0;
        models[i] = LoadModel(voxFileNames[i]);
        double t1 = GetTime()*1000.0;

        TraceLog(LOG_WARNING, TextFormat("[%s] File loaded in %.3f ms", voxFileNames[i], t1 - t0));

        // Compute model translation matrix to center model on draw position (0, 0 , 0)
        BoundingBox bb = GetModelBoundingBox(models[i]);
        Vector3 center = { 0 };
        center.x = bb.min.x  + (((bb.max.x - bb.min.x)/2));
        center.z = bb.min.z  + (((bb.max.z - bb.min.z)/2));

        Matrix matTranslate = MatrixTranslate(-center.x, 0, -center.z);
        models[i].transform = matTranslate;
    }

    int currentModel = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Cycle between models on mouse click
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) currentModel = (currentModel + 1)%MAX_VOX_FILES;

        // Cycle between models on key pressed
        if (IsKeyPressed(KEY_RIGHT))
        {
            currentModel++;
            if (currentModel >= MAX_VOX_FILES) currentModel = 0;
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            currentModel--;
            if (currentModel < 0) currentModel = MAX_VOX_FILES - 1;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw 3D model
            BeginMode3D(camera);

                DrawModel(models[currentModel], (Vector3){ 0, 0, 0 }, 1.0f, WHITE);
                DrawGrid(10, 1.0);

            EndMode3D();

            // Display info
            DrawRectangle(10, 400, 310, 30, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines(10, 400, 310, 30, Fade(DARKBLUE, 0.5f));
            DrawText("MOUSE LEFT BUTTON to CYCLE VOX MODELS", 40, 410, 10, BLUE);
            DrawText(TextFormat("File: %s", GetFileName(voxFileNames[currentModel])), 10, 10, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload models data (GPU VRAM)
    for (int i = 0; i < MAX_VOX_FILES; i++) UnloadModel(models[i]);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


