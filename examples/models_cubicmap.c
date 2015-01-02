/*******************************************************************************************
*
*   raylib [models] example - Cubicmap loading and drawing
*
*   This example has been created using raylib 1.2 (www.raylib.com)
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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - cubesmap loading and drawing");

    // Define the camera to look into our 3d world
    Camera camera = {{ 7.0, 7.0, 7.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Image img = LoadImage("resources/cubicmap.png");        // Load cubesmap image (RAM)
    Texture2D texture = LoadTextureFromImage(img, false);   // Convert image to texture (VRAM)
    Model map = LoadCubicmap(img);                          // Load cubicmap model
    SetModelTexture(&map, texture);                         // Bind texture to model
    Vector3 mapPosition = { -1, 0.0, -1 };                  // Set model position

    UnloadImage(img);       // Unload cubesmap image from RAM, already uploaded to VRAM

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_UP)) camera.position.y += 0.2f;
        else if (IsKeyDown(KEY_DOWN)) camera.position.y -= 0.2f;

        if (IsKeyDown(KEY_RIGHT)) camera.position.z += 0.2f;
        else if (IsKeyDown(KEY_LEFT)) camera.position.z -= 0.2f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(map, mapPosition, 1.0f, MAROON);

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
