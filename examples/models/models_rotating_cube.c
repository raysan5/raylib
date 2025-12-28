/*******************************************************************************************
*
*   raylib [models] example - rotating cube
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Jopestpe (@jopestpe)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jopestpe (@jopestpe)
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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - rotating cube");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 3.0f, 3.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load image to create texture for the cube
    Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    Image img = LoadImage("resources/cubicmap_atlas.png");
    Image crop = ImageFromImage(img, (Rectangle){0, img.height/2.0f, img.width/2.0f, img.height/2.0f});
    Texture2D texture = LoadTextureFromImage(crop);
    UnloadImage(img);
    UnloadImage(crop);

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    float rotation = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation += 1.0f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Draw model defining: position, size, rotation-axis, rotation (degrees), size, and tint-color
                DrawModelEx(model, (Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ 0.5f, 1.0f, 0.0f },
                    rotation, (Vector3){ 1.0f, 1.0f, 1.0f }, WHITE);

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Unload texture
    UnloadModel(model);     // Unload model

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
