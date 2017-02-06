/*******************************************************************************************
*
*   raylib [core] example - raylib bitmap font (rbmf) loading and usage (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
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

SpriteFont fonts[8];
const int spacings[8] = { 2, 4, 8, 4, 3, 4, 4, 1 };
Vector2 positions[8];

Color *colors;
const char (*messages)[64];

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
    InitWindow(screenWidth, screenHeight, "raylib [text] example - rBMF fonts");
    
    Color tempColors[8] = { MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, LIME, GOLD };
    
    colors = tempColors;
    
    const char tempMessages[8][64] = { "ALAGARD FONT designed by Hewett Tsoi", 
                               "PIXELPLAY FONT designed by Aleksander Shevchuk",
                               "MECHA FONT designed by Captain Falcon",  
                               "SETBACK FONT designed by Brian Kent (AEnigma)", 
                               "ROMULUS FONT designed by Hewett Tsoi", 
                               "PIXANTIQUA FONT designed by Gerhard Grossmann",
                               "ALPHA_BETA FONT designed by Brian Kent (AEnigma)",
                               "JUPITER_CRASH FONT designed by Brian Kent (AEnigma)" };
    
    messages = tempMessages;
    
    fonts[0] = LoadSpriteFont("resources/fonts/alagard.rbmf");       // rBMF font loading
    fonts[1] = LoadSpriteFont("resources/fonts/pixelplay.rbmf");     // rBMF font loading
    fonts[2] = LoadSpriteFont("resources/fonts/mecha.rbmf");         // rBMF font loading
    fonts[3] = LoadSpriteFont("resources/fonts/setback.rbmf");       // rBMF font loading
    fonts[4] = LoadSpriteFont("resources/fonts/romulus.rbmf");       // rBMF font loading
    fonts[5] = LoadSpriteFont("resources/fonts/pixantiqua.rbmf");    // rBMF font loading
    fonts[6] = LoadSpriteFont("resources/fonts/alpha_beta.rbmf");    // rBMF font loading
    fonts[7] = LoadSpriteFont("resources/fonts/jupiter_crash.rbmf"); // rBMF font loading    
        
    for (int i = 0; i < 8; i++)
    {
        positions[i].x = screenWidth/2 - MeasureTextEx(fonts[i], messages[i], fonts[i].size*2, spacings[i]).x/2;
        positions[i].y = 60 + fonts[i].size + 50*i;
    }
    
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
    for (int i = 0; i < 8; i++)
    {
        UnloadSpriteFont(fonts[i]);       // SpriteFont unloading
    }
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
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("free fonts included with raylib", 250, 20, 20, DARKGRAY);
        DrawLine(220, 50, 590, 50, DARKGRAY);
        
        for (int i = 0; i < 8; i++)
        {
            DrawTextEx(fonts[i], messages[i], positions[i], fonts[i].size*2, spacings[i], colors[i]);
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}