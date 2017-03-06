/*******************************************************************************************
*
*   raylib [text] example - BMFont and TTF SpriteFonts loading (adapted for HTML5 platform)
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
int screenWidth = 800;
int screenHeight = 450;

const char msgBm[64] = "THIS IS AN AngelCode SPRITE FONT";
const char msgTtf[64] = "THIS SPRITE FONT has been GENERATED from a TTF";

// NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
SpriteFont fontBm;      // BMFont (AngelCode)
SpriteFont fontTtf;     // TTF font

Vector2 fontPosition;

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
    InitWindow(screenWidth, screenHeight, "raylib [text] example - bmfont and ttf sprite fonts loading");
    
    fontBm = LoadSpriteFont("resources/fonts/bmfont.fnt");       // BMFont (AngelCode)
    fontTtf = LoadSpriteFont("resources/fonts/pixantiqua.ttf");  // TTF font
    
    fontPosition.x = screenWidth/2 - MeasureTextEx(fontBm, msgBm, fontBm.size, 0).x/2;
    fontPosition.y = screenHeight/2 - fontBm.size/2 - 80;
    
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
    UnloadSpriteFont(fontBm);     // AngelCode SpriteFont unloading
    UnloadSpriteFont(fontTtf);    // TTF SpriteFont unloading
    
    CloseWindow();        // Close window and OpenGL context
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

        DrawTextEx(fontBm, msgBm, fontPosition, fontBm.size, 0, MAROON);
        DrawTextEx(fontTtf, msgTtf, (Vector2){ 75.0f, 240.0f }, fontTtf.size*0.8f, 2, LIME);

    EndDrawing();
    //----------------------------------------------------------------------------------
}