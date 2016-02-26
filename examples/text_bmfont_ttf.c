/*******************************************************************************************
*
*   raylib [text] example - BMFont and TTF SpriteFonts loading
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

    InitWindow(screenWidth, screenHeight, "raylib [text] example - bmfont and ttf sprite fonts loading");

    const char msgBm[64] = "THIS IS AN AngelCode SPRITE FONT";
    const char msgTtf[64] = "THIS SPRITE FONT has been GENERATED from a TTF";

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
    SpriteFont fontBm = LoadSpriteFont("resources/fonts/bmfont.fnt");       // BMFont (AngelCode)
    SpriteFont fontTtf = LoadSpriteFont("resources/fonts/pixantiqua.ttf");  // TTF font

    Vector2 fontPosition;

    fontPosition.x = screenWidth/2 - MeasureTextEx(fontBm, msgBm, fontBm.size, 0).x/2;
    fontPosition.y = screenHeight/2 - fontBm.size/2 - 80;

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

            DrawTextEx(fontBm, msgBm, fontPosition, fontBm.size, 0, MAROON);
            DrawTextEx(fontTtf, msgTtf, (Vector2){ 75.0f, 240.0f }, fontTtf.size*0.8f, 2, LIME);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(fontBm);     // AngelCode SpriteFont unloading
    UnloadSpriteFont(fontTtf);    // TTF SpriteFont unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}