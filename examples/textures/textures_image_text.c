/*******************************************************************************************
*
*   raylib [textures] example - image text
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 1.8, last time updated with raylib 6.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2017-2025 Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - image text");

    Image parrots = LoadImage("resources/parrots.png"); // Load image in CPU memory (RAM)

    // When no codepoints are provided, LoadFontEx() loads a default set of codepoints
    // This set includes '@', which "KAISG.ttf" doesn't have a glyph for, causing a warning to be logged
    // We avoid this by just excluding it from the list of codepoints we load
    int codepointCount = 0;
    int *codepoints = LoadCodepoints(" !\"#$%&'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", &codepointCount);
    // TTF Font loading with custom generation parameters
    Font font = LoadFontEx("resources/KAISG.ttf", 64, codepoints, codepointCount);
    UnloadCodepoints(codepoints);

    // Draw over image using custom font
    ImageDrawTextEx(&parrots, font, "[Parrots font drawing]", (Vector2){ 20.0f, 20.0f }, (float)font.baseSize, 0.0f, RED);

    Texture2D texture = LoadTextureFromImage(parrots);  // Image converted to texture, uploaded to GPU memory (VRAM)
    UnloadImage(parrots);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM

    Vector2 position = { (float)screenWidth/2 - (float)texture.width/2, (float)screenHeight/2 - (float)texture.height/2 - 20 };

    bool showFont = false;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_SPACE)) showFont = true;
        else showFont = false;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (!showFont)
            {
                // Draw texture with text already drawn inside
                DrawTextureV(texture, position, WHITE);

                // Draw text directly using sprite font
                DrawTextEx(font, "[Parrots font drawing]", (Vector2){ position.x + 20,
                           position.y + 20 + 280 }, (float)font.baseSize, 0.0f, WHITE);
            }
            else
            {
                // Make the font atlas texture fit the screen, so we can see the whole thing
                float scale = 1.0f;

                float atlasRatio = (float)font.texture.width/(float)font.texture.height;
                float screenRatio = (float)screenWidth/(float)screenHeight;
                if (atlasRatio >= screenRatio) scale = (float)screenWidth/(float)font.texture.width;
                else scale = (float)screenHeight/(float)font.texture.height;

                float width = (float)font.texture.width * scale;
                float height = (float)font.texture.height * scale;

                DrawTextureEx(font.texture, (Vector2){ ((float)screenWidth - width)/2, ((float)screenHeight - height)/2 }, 0, scale, BLACK);
            }

            DrawText("PRESS SPACE to SHOW FONT ATLAS USED", 290, 420, 10, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Texture unloading

    UnloadFont(font);           // Unload custom font

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}