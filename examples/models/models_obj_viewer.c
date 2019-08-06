/*******************************************************************************************
*
*   raylib [models] example - OBJ models viewer
*
*   This example has been created using raylib 2.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <string.h>        // Required for: strcpy()

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - obj viewer");

    // Define the camera to look into our 3d world
    Camera camera = { { 30.0f, 30.0f, 30.0f }, { 0.0f, 10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, 0 };

    Model model = LoadModel("resources/models/turret.obj");                     // Load default model obj
    Texture2D texture = LoadTexture("resources/models/turret_diffuse.png");     // Load default model texture
    model.materials[0].maps[MAP_DIFFUSE].texture = texture; // Bind texture to model

    Vector3 position = { 0.0, 0.0, 0.0 };                   // Set model position
    BoundingBox bounds = MeshBoundingBox(model.meshes[0]);  // Set model bounds
    bool selected = false;                                  // Selected object flag

    SetCameraMode(camera, CAMERA_FREE);     // Set a free camera mode

    char objFilename[64] = "turret.obj";

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsFileDropped())
        {
            int count = 0;
            char **droppedFiles = GetDroppedFiles(&count);

            if (count == 1)
            {
                if (IsFileExtension(droppedFiles[0], ".obj"))
                {
                    for (int i = 0; i < model.meshCount; i++) UnloadMesh(model.meshes[i]);
                    model.meshes = LoadMeshes(droppedFiles[0], &model.meshCount);
                    bounds = MeshBoundingBox(model.meshes[0]);
                }
                else if (IsFileExtension(droppedFiles[0], ".png"))
                {
                    UnloadTexture(texture);
                    texture = LoadTexture(droppedFiles[0]);
                    model.materials[0].maps[MAP_DIFFUSE].texture = texture;
                }

                strcpy(objFilename, GetFileName(droppedFiles[0]));
            }

            ClearDroppedFiles();    // Clear internal buffers
        }

        UpdateCamera(&camera);

        // Select model on mouse click
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // Check collision between ray and box
            if (CheckCollisionRayBox(GetMouseRay(GetMousePosition(), camera), bounds)) selected = !selected;
            else selected = false;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(model, position, 1.0f, WHITE);   // Draw 3d model with texture

                DrawGrid(20.0, 10.0);        // Draw a grid

                if (selected) DrawBoundingBox(bounds, GREEN);

            EndMode3D();

            DrawText("Free camera default controls:", 10, 20, 10, DARKGRAY);
            DrawText("- Mouse Wheel to Zoom in-out", 20, 40, 10, GRAY);
            DrawText("- Mouse Wheel Pressed to Pan", 20, 60, 10, GRAY);
            DrawText("- Alt + Mouse Wheel Pressed to Rotate", 20, 80, 10, GRAY);
            DrawText("- Alt + Ctrl + Mouse Wheel Pressed for Smooth Zoom", 20, 100, 10, GRAY);

            DrawText("Drag & drop .obj/.png to load mesh/texture.", 10, GetScreenHeight() - 20, 10, DARKGRAY);
            DrawText(FormatText("Current file: %s", objFilename), 250, GetScreenHeight() - 20, 10, GRAY);
            if (selected) DrawText("MODEL SELECTED", GetScreenWidth() - 110, 10, 10, GREEN);

            DrawText("(c) Turret 3D model by Alberto Cano", screenWidth - 200, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);         // Unload model

    ClearDroppedFiles();        // Clear internal buffers

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}