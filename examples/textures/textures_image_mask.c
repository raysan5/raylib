/*******************************************************************************************
*
*   raylib [textures] example - Retrive image mask from alpha values
*
*   NOTE: Images are loaded in CPU memory (RAM); textures are loaded in GPU memory (VRAM)
*
*   Example originally created with raylib 5.1-dev, last time updated with raylib 5.1-dev
*
*   Example contributed by Bruno Cabral (github.com/brccabral) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2024 Bruno Cabral (github.com/brccabral) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <raylib.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 612;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - draw rectangle rounded");

    Image fudesumi_image = LoadImage("resources/fudesumi.png");
    Texture2D fudesumi_texture = LoadTextureFromImage(fudesumi_image);
    Texture2D fudesumi_mask = LoadTextureMaskFromImage(fudesumi_image, RED, 0.0f);

    Image background_image = GenImageChecked(screenWidth, screenHeight, screenWidth/20, screenHeight/20, BLUE, SKYBLUE);
    Texture2D background_texture = LoadTextureFromImage(background_image);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            DrawTexture(background_texture, 0, 0, WHITE);
            DrawTexture(fudesumi_texture, 50, 50, WHITE);
            DrawTexture(fudesumi_mask, 410, 50, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadImage(fudesumi_image);
    UnloadImage(background_image);
    UnloadTexture(fudesumi_texture);
    UnloadTexture(fudesumi_mask);
    UnloadTexture(background_texture);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
