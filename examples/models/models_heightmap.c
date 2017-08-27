/*******************************************************************************************
*
*   raylib [models] example - Heightmap loading and drawing
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");

    // Define our custom camera to look into our 3d world
    Camera camera = {{ 18.0f, 16.0f, 18.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

    Image image = LoadImage("resources/heightmap.png");             // Load heightmap image (RAM)
    Texture2D texture = LoadTextureFromImage(image);                // Convert image to texture (VRAM)
    
    Mesh mesh = GenMeshHeightmap(image, (Vector3){ 16, 8, 16 });    // Generate heightmap mesh (RAM and VRAM)
    Model model = LoadModelFromMesh(mesh);                          // Load model from generated mesh

    model.material.maps[MAP_DIFFUSE].texture = texture;             // Set map diffuse texture
    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };                   // Define model position

    UnloadImage(image);                     // Unload heightmap image from RAM, already uploaded to VRAM
    
    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(model, mapPosition, 1.0f, RED);

                DrawGrid(20, 1.0f);

            End3dMode();
            
            DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE);
            DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadModel(model);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}