/*******************************************************************************************
*
*   raylib [text] example - BMFont unordered chars loading and drawing
*
*   This example has been created using raylib 1.4 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;

// NOTE: Using chars outside the [32..127] limits!
// NOTE: If a character is not found in the font, it just renders a space
const char msg[256] = "ASCII extended characters:\n¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆ\nÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæ\nçèéêëìíîïðñòóôõö÷øùúûüýþÿ";

SpriteFont font;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [text] example - bmfont unordered loading and drawing");

    // NOTE: Loaded font has an unordered list of characters (chars in the range 32..255)
    font = LoadSpriteFont("resources/pixantiqua.fnt");       // BMFont (AngelCode)

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(font); // Unload music stream buffers from RAM

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables here...
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Font name:       PixAntiqua", 40, 50, 20, GRAY);
        DrawText(FormatText("Font base size:           %i", font.baseSize), 40, 80, 20, GRAY);
        DrawText(FormatText("Font chars number:     %i", font.charsCount), 40, 110, 20, GRAY);
        
        DrawTextEx(font, msg, (Vector2){ 40, 180 }, font.baseSize, 0, MAROON);

    EndDrawing();
    //----------------------------------------------------------------------------------
}