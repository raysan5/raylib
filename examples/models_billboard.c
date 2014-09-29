/*******************************************************************************************
*
*   raylib [models] example - Drawing billboards
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

    InitWindow(screenWidth, screenHeight, "raylib [models] example - drawing billboards");

    // Define the camera to look into our 3d world
    Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};

    Texture2D lena = LoadTexture("resources/lena.png");     // Our texture for billboard
    Rectangle eyesRec = { 225, 240, 155, 50 };              // Part of the texture to draw
    Vector3 billPosition = { 0.0, 0.0, 0.0 };               // Position where draw billboard

    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT)) camera.position.x -= 0.2;
        if (IsKeyDown(KEY_RIGHT)) camera.position.x += 0.2;
        if (IsKeyDown(KEY_UP)) camera.position.y -= 0.2;
        if (IsKeyDown(KEY_DOWN)) camera.position.y += 0.2;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            Begin3dMode(camera);

                //DrawBillboard(camera, lena, billPosition, 1.0, WHITE);
                DrawBillboardRec(camera, lena, eyesRec, billPosition, 4.0, WHITE);

                DrawGrid(10.0, 1.0);        // Draw a grid

            End3dMode();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(lena);        // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}