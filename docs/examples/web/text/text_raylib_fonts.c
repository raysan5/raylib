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

#define MAX_FONTS   8

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

SpriteFont fonts[MAX_FONTS];
const int spacings[MAX_FONTS] = { 2, 4, 8, 4, 3, 4, 4, 1 };
Vector2 positions[MAX_FONTS];

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
    InitWindow(screenWidth, screenHeight, "raylib [text] example - raylib fonts");
    
    Color tempColors[MAX_FONTS] = { MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, LIME, GOLD, RED };
    
    colors = tempColors;
    
    const char tempMessages[MAX_FONTS][64] = { "ALAGARD FONT designed by Hewett Tsoi", 
                               "PIXELPLAY FONT designed by Aleksander Shevchuk",
                               "MECHA FONT designed by Captain Falcon",  
                               "SETBACK FONT designed by Brian Kent (AEnigma)", 
                               "ROMULUS FONT designed by Hewett Tsoi", 
                               "PIXANTIQUA FONT designed by Gerhard Grossmann",
                               "ALPHA_BETA FONT designed by Brian Kent (AEnigma)",
                               "JUPITER_CRASH FONT designed by Brian Kent (AEnigma)" };
    
    messages = tempMessages;
    
    fonts[0] = LoadSpriteFont("resources/fonts/alagard.png");
    fonts[1] = LoadSpriteFont("resources/fonts/pixelplay.png");
    fonts[2] = LoadSpriteFont("resources/fonts/mecha.png");
    fonts[3] = LoadSpriteFont("resources/fonts/setback.png");
    fonts[4] = LoadSpriteFont("resources/fonts/romulus.png");
    fonts[5] = LoadSpriteFont("resources/fonts/pixantiqua.png");
    fonts[6] = LoadSpriteFont("resources/fonts/alpha_beta.png");
    fonts[7] = LoadSpriteFont("resources/fonts/jupiter_crash.png"); 

    for (int i = 0; i < MAX_FONTS; i++)
    {
        positions[i].x = screenWidth/2 - MeasureTextEx(fonts[i], messages[i], fonts[i].baseSize*2, spacings[i]).x/2;
        positions[i].y = 60 + fonts[i].baseSize + 45*i;
    }
    
    // Small Y position corrections
    positions[3].y += 8;
    positions[4].y += 2;
    positions[7].y -= 8;
    
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
    
    // SpriteFont unloading
    for (int i = 0; i < MAX_FONTS; i++) UnloadSpriteFont(fonts[i]);

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
            DrawTextEx(fonts[i], messages[i], positions[i], fonts[i].baseSize*2, spacings[i], colors[i]);
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}