/*******************************************************************************************
*
*   raylib [models] example - Load and draw a 3d model (OBJ)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
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
    Camera camera = {{ 3.0, 3.0, 3.0 }, { 0.0, 1.5, 0.0 }, { 0.0, 1.0, 0.0 }};

    Texture2D texture = LoadTexture("resources/model/dwarf_diffuse.png");   // Load model texture
    Model dwarf = LoadModel("resources/model/dwarf.obj");                   // Load OBJ model
    SetModelTexture(&dwarf, texture);                             // Bind texture to model
    Vector3 position = { 0.0, 0.0, 0.0 };                         // Set model position

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT)) position.x -= 0.2;
        if (IsKeyDown(KEY_RIGHT)) position.x += 0.2;
        if (IsKeyDown(KEY_UP)) position.z -= 0.2;
        if (IsKeyDown(KEY_DOWN)) position.z += 0.2;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                DrawModel(dwarf, position, 2.0f, WHITE);   // Draw 3d model with texture

                DrawGrid(10.0, 1.0);        // Draw a grid

                DrawGizmo(position);     // Draw gizmo

            End3dMode();
            
            DrawText("(c) Dwarf 3D model by David Moreno", screenWidth - 200, screenHeight - 20, 10, GRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadModel(dwarf);         // Unload model

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}