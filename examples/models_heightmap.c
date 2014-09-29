/*******************************************************************************************
*
*   raylib [models] example - Heightmap loading and drawing
*
*   This example has been created using raylib 1.1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

    // Define the camera to look into our 3d world
    Camera camera = {{ 10.0, 12.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Image img = LoadImage("resources/heightmap.png");   // Load heightmap image (RAM)
    Texture2D texture = CreateTexture(img, false);      // Convert image to texture (VRAM)
    Model map = LoadHeightmap(img, 4);                  // Load heightmap model
    SetModelTexture(&map, texture);                     // Bind texture to model
    Vector3 mapPosition = { -4, 0.0, -4 };              // Set model position

    UnloadImage(img);       // Unload heightmap image from RAM, already uploaded to VRAM

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(map, mapPosition, 0.5f, MAROON);

                DrawGrid(10.0, 1.0);

                DrawGizmo(mapPosition);

            End3dMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadModel(map);           // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}