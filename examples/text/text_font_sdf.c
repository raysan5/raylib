/*******************************************************************************************
*
*   raylib [text] example - font sdf
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 1.3, last time updated with raylib 4.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#include <stdlib.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - font sdf");

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)

    const char msg[50] = "Signed Distance Fields";

    // Loading file to memory
    int fileSize = 0;
    unsigned char *fileData = LoadFileData("resources/anonymous_pro_bold.ttf", &fileSize);

    // Default font generation from TTF font
    Font fontDefault = { 0 };
    fontDefault.baseSize = 16;
    fontDefault.glyphCount = 95;

    // Loading font data from memory data
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 95 (autogenerate chars array)
    fontDefault.glyphs = LoadFontData(fileData, fileSize, 16, 0, 95, FONT_DEFAULT, &fontDefault.glyphCount);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 4 px, pack method: 0 (default)
    Image atlas = GenImageFontAtlas(fontDefault.glyphs, &fontDefault.recs, 95, 16, 4, 0);
    fontDefault.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);

    // SDF font generation from TTF font
    Font fontSDF = { 0 };
    fontSDF.baseSize = 16;
    fontSDF.glyphCount = 95;
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 0 (defaults to 95)
    fontSDF.glyphs = LoadFontData(fileData, fileSize, 16, 0, 0, FONT_SDF, &
    fontSDF.glyphCount);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 0 px, pack method: 1 (Skyline algorythm)
    atlas = GenImageFontAtlas(fontSDF.glyphs, &fontSDF.recs, 95, 16, 0, 1);
    fontSDF.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);

    UnloadFileData(fileData);      // Free memory from loaded file

    // Load SDF required shader (we use default vertex shader)
    Shader shader = LoadShader(0, TextFormat("resources/shaders/glsl%i/sdf.fs", GLSL_VERSION));
    SetTextureFilter(fontSDF.texture, TEXTURE_FILTER_BILINEAR);    // Required for SDF font

    Vector2 fontPosition = { 40, screenHeight/2.0f - 50 };
    Vector2 textSize = { 0.0f, 0.0f };
    float fontSize = 16.0f;
    int currentFont = 0;            // 0 - fontDefault, 1 - fontSDF

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        fontSize += GetMouseWheelMove()*8.0f;

        if (fontSize < 6) fontSize = 6;

        if (IsKeyDown(KEY_SPACE)) currentFont = 1;
        else currentFont = 0;

        if (currentFont == 0) textSize = MeasureTextEx(fontDefault, msg, fontSize, 0);
        else textSize = MeasureTextEx(fontSDF, msg, fontSize, 0);

        fontPosition.x = GetScreenWidth()/2 - textSize.x/2;
        fontPosition.y = GetScreenHeight()/2 - textSize.y/2 + 80;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (currentFont == 1)
            {
                // NOTE: SDF fonts require a custom SDf shader to compute fragment color
                BeginShaderMode(shader);    // Activate SDF font shader
                    DrawTextEx(fontSDF, msg, fontPosition, fontSize, 0, BLACK);
                EndShaderMode();            // Activate our default shader for next drawings

                DrawTexture(fontSDF.texture, 10, 10, BLACK);
            }
            else
            {
                DrawTextEx(fontDefault, msg, fontPosition, fontSize, 0, BLACK);
                DrawTexture(fontDefault.texture, 10, 10, BLACK);
            }

            if (currentFont == 1) DrawText("SDF!", 320, 20, 80, RED);
            else DrawText("default font", 315, 40, 30, GRAY);

            DrawText("FONT SIZE: 16.0", GetScreenWidth() - 240, 20, 20, DARKGRAY);
            DrawText(TextFormat("RENDER SIZE: %02.02f", fontSize), GetScreenWidth() - 240, 50, 20, DARKGRAY);
            DrawText("Use MOUSE WHEEL to SCALE TEXT!", GetScreenWidth() - 240, 90, 10, DARKGRAY);

            DrawText("HOLD SPACE to USE SDF FONT VERSION!", 340, GetScreenHeight() - 30, 20, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(fontDefault);    // Default font unloading
    UnloadFont(fontSDF);        // SDF font unloading

    UnloadShader(shader);       // Unload SDF shader

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}