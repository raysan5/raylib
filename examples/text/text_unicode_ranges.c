/*******************************************************************************************
*
*   raylib [text] example - unicode ranges
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Vlad Adrian (@demizdor) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()

//--------------------------------------------------------------------------------------
// Module Functions Declaration
//--------------------------------------------------------------------------------------
// Add codepoint range to existing font
static void AddCodepointRange(Font *font, const char *fontPath, int start, int stop);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - unicode ranges");

    // Load font with default Unicode range: Basic ASCII [32-127]
    Font font = LoadFont("resources/NotoSansTC-Regular.ttf");
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    int unicodeRange = 0;           // Track the ranges of codepoints added to font
    int prevUnicodeRange = 0;       // Previous Unicode range to avoid reloading every frame

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (unicodeRange != prevUnicodeRange)
        {
            UnloadFont(font);

            // Load font with default Unicode range: Basic ASCII [32-127]
            font = LoadFont("resources/NotoSansTC-Regular.ttf");
            
            // Add required ranges to loaded font
            switch (unicodeRange)
            {
                /*
                case 5:
                {
                    // Unicode range: Devanari, Arabic, Hebrew
                    // WARNING: Glyphs not available on provided font!
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x900, 0x97f);  // Devanagari
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x600, 0x6ff);  // Arabic
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x5d0, 0x5ea);  // Hebrew
                }
                */
                case 4:
                {
                    // Unicode range: CJK (Japanese and Chinese)
                    // WARNING: Loading thousands of codepoints requires lot of time!
                    // A better strategy is prefilter the required codepoints for the text
                    // in the game and just load the required ones
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x4e00, 0x9fff);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x3400, 0x4dbf);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x3000, 0x303f);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x3040, 0x309f);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x30A0, 0x30ff);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x31f0, 0x31ff);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0xff00, 0xffef);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0xac00, 0xd7af);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x1100, 0x11ff);
                }
                case 3:
                {
                    // Unicode range: Cyrillic
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x400, 0x4ff);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x500, 0x52f);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x2de0, 0x2Dff);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0xa640, 0xA69f);
                }
                case 2:
                {
                    // Unicode range: Greek
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x370, 0x3ff);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x1f00, 0x1fff);
                }
                case 1:
                {
                    // Unicode range: European Languages
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0xc0, 0x17f);
                    AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x180, 0x24f);
                    //AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x1e00, 0x1eff);
                    //AddCodepointRange(&font, "resources/NotoSansTC-Regular.ttf", 0x2c60, 0x2c7f);
                }
                default: break;
            }

            prevUnicodeRange = unicodeRange;
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR); // Set font atlas scale filter
        }

        if (IsKeyPressed(KEY_ZERO)) unicodeRange = 0;
        else if (IsKeyPressed(KEY_ONE)) unicodeRange = 1;
        else if (IsKeyPressed(KEY_TWO)) unicodeRange = 2;
        else if (IsKeyPressed(KEY_THREE)) unicodeRange = 3;
        else if (IsKeyPressed(KEY_FOUR)) unicodeRange = 4;
        //else if (IsKeyPressed(KEY_FIVE)) unicodeRange = 5;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);

            DrawText("ADD CODEPOINTS: [1][2][3][4]", 20, 20, 20, MAROON);
            
            // Render test strings in different languages
            DrawTextEx(font, "> English: Hello World!", (Vector2){ 50, 70 }, 32, 1, DARKGRAY); // English
            DrawTextEx(font, "> Español: Hola mundo!", (Vector2){ 50, 120 }, 32, 1, DARKGRAY); // Spanish
            DrawTextEx(font, "> Ελληνικά: Γειά σου κόσμε!", (Vector2){ 50, 170 }, 32, 1, DARKGRAY); // Greek
            DrawTextEx(font, "> Русский: Привет мир!", (Vector2){ 50, 220 }, 32, 0, DARKGRAY); // Russian
            DrawTextEx(font, "> 中文: 你好世界!", (Vector2){ 50, 270 }, 32, 1, DARKGRAY);        // Chinese
            DrawTextEx(font, "> 日本語: こんにちは世界!", (Vector2){ 50, 320 }, 32, 1, DARKGRAY); // Japanese
            //DrawTextEx(font, "देवनागरी: होला मुंडो!", (Vector2){ 50, 350 }, 32, 1, DARKGRAY);     // Devanagari (glyphs not available in font)
            
            // Draw font texture scaled to screen
            float atlasScale = 380.0f/font.texture.width;
            DrawRectangle(400, 16, font.texture.width*atlasScale, font.texture.height*atlasScale, BLACK);
            DrawTexturePro(font.texture, (Rectangle){ 0, 0, font.texture.width, font.texture.height },
                (Rectangle){ 400, 16, font.texture.width*atlasScale, font.texture.height*atlasScale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
            DrawRectangleLines(400, 16, 380, 380, RED);

            DrawText(TextFormat("ATLAS SIZE: %ix%i px (x%02.2f)", font.texture.width, font.texture.height, atlasScale), 20, 380, 20, BLUE);
            DrawText(TextFormat("CODEPOINTS GLYPHS LOADED: %i", font.glyphCount), 20, 410, 20, LIME);

            // Display font attribution
            DrawText("Font: Noto Sans TC. License: SIL Open Font License 1.1", screenWidth - 300, screenHeight - 20, 10, GRAY);

            if (prevUnicodeRange != unicodeRange)
            {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(WHITE, 0.8f));
                DrawRectangle(0, 125, screenWidth, 200, GRAY);
                DrawText("GENERATING FONT ATLAS...", 120, 210, 40, BLACK);
            }
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(font);        // Unload font resource

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------
// Add codepoint range to existing font
static void AddCodepointRange(Font *font, const char *fontPath, int start, int stop)
{
    int rangeSize = stop - start + 1;
    int currentRangeSize = font->glyphCount;
    
    // TODO: Load glyphs from provided vector font (if available),
    // add them to existing font, regenerating font image and texture
    
    int updatedCodepointCount = currentRangeSize + rangeSize;
    int *updatedCodepoints = (int *)RL_CALLOC(updatedCodepointCount, sizeof(int));

    // Get current codepoint list
    for (int i = 0; i < currentRangeSize; i++) updatedCodepoints[i] = font->glyphs[i].value;

    // Add new codepoints to list (provided range)
    for (int i = currentRangeSize; i < updatedCodepointCount; i++)
        updatedCodepoints[i] = start + (i - currentRangeSize);

    UnloadFont(*font);
    *font = LoadFontEx(fontPath, 32, updatedCodepoints, updatedCodepointCount);
    RL_FREE(updatedCodepoints);
}

