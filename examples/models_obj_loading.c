/*******************************************************************************************
*
*   raylib [models] example - Load and draw a 3d model (OBJ)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - obj model loading");

    // Define the camera to look into our 3d world
    Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Texture2D texture = LoadTexture("resources/catsham.png");   // Load model texture
    Model cat = LoadModel("resources/cat.obj");                 // Load OBJ model
    SetModelTexture(&cat, texture);                             // Bind texture to model
    Vector3 catPosition = { 0.0, 0.0, 0.0 };                    // Set model position

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT)) catPosition.x -= 0.2;
        if (IsKeyDown(KEY_RIGHT)) catPosition.x += 0.2;
        if (IsKeyDown(KEY_UP)) catPosition.z -= 0.2;
        if (IsKeyDown(KEY_DOWN)) catPosition.z += 0.2;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(cat, catPosition, 0.1f, WHITE);   // Draw 3d model with texture

                DrawGrid(10.0, 1.0);        // Draw a grid

                DrawGizmo(catPosition);     // Draw gizmo

            End3dMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadModel(cat);           // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}