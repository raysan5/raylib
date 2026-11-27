/*******************************************************************************************
*
*   raylib [textures] example - background scrolling
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 2.0, last time updated with raylib 2.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2025 Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - background scrolling");

    // NOTE: Be careful, background width must be equal or bigger than screen width
    // if not, texture should be draw more than two times for scrolling effect
    Texture2D texBackground = LoadTexture("resources/cyberpunk_street_background.png");
    Texture2D texMidground = LoadTexture("resources/cyberpunk_street_midground.png");
    Texture2D texForeground = LoadTexture("resources/cyberpunk_street_foreground.png");

    float scrollingBackground = 0.0f;
    float scrollingMidground = 0.0f;
    float scrollingForeground = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        scrollingBackground -= 0.1f;
        scrollingMidground -= 0.5f;
        scrollingForeground -= 1.0f;

        // NOTE: Texture is scaled twice its size, so it sould be considered on scrolling
        if (scrollingBackground <= -texBackground.width*2) scrollingBackground = 0;
        if (scrollingMidground <= -texMidground.width*2) scrollingMidground = 0;
        if (scrollingForeground <= -texForeground.width*2) scrollingForeground = 0;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(GetColor(0x052c46ff));

            // Draw background image twice
            // NOTE: Texture is scaled twice its size
            DrawTextureEx(texBackground, (Vector2){ scrollingBackground, 20 }, 0.0f, 2.0f, WHITE);
            DrawTextureEx(texBackground, (Vector2){ texBackground.width*2 + scrollingBackground, 20 }, 0.0f, 2.0f, WHITE);

            // Draw midground image twice
            DrawTextureEx(texMidground, (Vector2){ scrollingMidground, 20 }, 0.0f, 2.0f, WHITE);
            DrawTextureEx(texMidground, (Vector2){ texMidground.width*2 + scrollingMidground, 20 }, 0.0f, 2.0f, WHITE);

            // Draw foreground image twice
            DrawTextureEx(texForeground, (Vector2){ scrollingForeground, 70 }, 0.0f, 2.0f, WHITE);
            DrawTextureEx(texForeground, (Vector2){ texForeground.width*2 + scrollingForeground, 70 }, 0.0f, 2.0f, WHITE);

            DrawText("BACKGROUND SCROLLING & PARALLAX", 10, 10, 20, RED);
            DrawText("(c) Cyberpunk Street Environment by Luis Zuno (@ansimuz)", screenWidth - 330, screenHeight - 20, 10, RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texBackground);  // Unload background texture
    UnloadTexture(texMidground);   // Unload midground texture
    UnloadTexture(texForeground);  // Unload foreground texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}