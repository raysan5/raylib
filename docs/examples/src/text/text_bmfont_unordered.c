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

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - bmfont unordered loading and drawing");

    // NOTE: Using chars outside the [32..127] limits!
    // NOTE: If a character is not found in the font, it just renders a space
    const char msg[256] = "ASCII extended characters:\n¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆ\nÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæ\nçèéêëìíîïðñòóôõö÷øùúûüýþÿ";

    // NOTE: Loaded font has an unordered list of characters (chars in the range 32..255)
    SpriteFont font = LoadSpriteFont("resources/fonts/pixantiqua.fnt");       // BMFont (AngelCode)

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
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
            DrawText(FormatText("Font base size:           %i", font.size), 40, 80, 20, GRAY);
            DrawText(FormatText("Font chars number:     %i", font.numChars), 40, 110, 20, GRAY);
            
            DrawTextEx(font, msg, (Vector2){ 40, 180 }, font.size, 0, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(font);     // AngelCode SpriteFont unloading
    
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}