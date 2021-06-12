/*******************************************************************************************
*
*   raylib [shapes] example - Draw Textured Polygon
*
*   This example has been created using raylib 3.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Chris Camacho (@codifies - bedroomcoders.co.uk) and
*   reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2021 Chris Camacho (@codifies) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#define MAX_POINTS  11  // 10 points and back to the start

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    Vector2 texcoords[MAX_POINTS] = {
        (Vector2){ 0.75f, 0.0f },
        (Vector2){ 0.25f, 0.0f },
        (Vector2){ 0.0f, 0.5f },
        (Vector2){ 0.0f, 0.75f },
        (Vector2){ 0.25f, 1.0f},
        (Vector2){ 0.375f, 0.875f},
        (Vector2){ 0.625f, 0.875f},
        (Vector2){ 0.75f, 1.0f},
        (Vector2){ 1.0f, 0.75f},
        (Vector2){ 1.0f, 0.5f},
        (Vector2){ 0.75f, 0.0f}  // Close the poly
    };

    Vector2 points[MAX_POINTS] = { 0 };

    // Create the poly coords from the UV's
    // you don't have to do this you can specify
    // them however you want
    for (int i = 0; i < MAX_POINTS; i++)
    {
        points[i].x = (texcoords[i].x - 0.5f)*256.0f;
        points[i].y = (texcoords[i].y - 0.5f)*256.0f;
    }

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - textured polygon");

    Texture texture = LoadTexture("resources/cat.png");

    float ang = 0;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        ang++;

        Vector2 positions[MAX_POINTS] = { 0 };

        for (int i = 0; i < MAX_POINTS; i++) positions[i] = Vector2Rotate(points[i], ang);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("textured polygon", 20, 20, 20, DARKGRAY);

            DrawTexturePoly(texture, (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 },
                            positions, texcoords, MAX_POINTS, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Unload texture

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
