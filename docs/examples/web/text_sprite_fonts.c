/*******************************************************************************************
*
*   raylib [core] example - SpriteFont loading and usage (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
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

const char msg1[50] = "THIS IS A custom SPRITE FONT...";
const char msg2[50] = "...and this is ANOTHER CUSTOM font...";
const char msg3[50] = "...and a THIRD one! GREAT! :D";

SpriteFont font1;
SpriteFont font2;
SpriteFont font3;

Vector2 fontPosition1, fontPosition2, fontPosition3;

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
    InitWindow(screenWidth, screenHeight, "raylib [text] example - sprite fonts usage");
    
    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
    font1 = LoadSpriteFont("resources/fonts/custom_mecha.png");          // SpriteFont loading
    font2 = LoadSpriteFont("resources/fonts/custom_alagard.png");        // SpriteFont loading
    font3 = LoadSpriteFont("resources/fonts/custom_jupiter_crash.png");  // SpriteFont loading
    
    fontPosition1.x = screenWidth/2 - MeasureTextEx(font1, msg1, font1.size, -3).x/2;
    fontPosition1.y = screenHeight/2 - font1.size/2 - 80;

    fontPosition2.x = screenWidth/2 - MeasureTextEx(font2, msg2, font2.size, -2).x/2;
    fontPosition2.y = screenHeight/2 - font2.size/2 - 10;

    fontPosition3.x = screenWidth/2 - MeasureTextEx(font3, msg3, font3.size, 2).x/2;
    fontPosition3.y = screenHeight/2 - font3.size/2 + 50;
    
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
    UnloadSpriteFont(font1);      // SpriteFont unloading
    UnloadSpriteFont(font2);      // SpriteFont unloading
    UnloadSpriteFont(font3);      // SpriteFont unloading
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

        DrawTextEx(font1, msg1, fontPosition1, font1.size, -3, WHITE);
        DrawTextEx(font2, msg2, fontPosition2, font2.size, -2, WHITE);
        DrawTextEx(font3, msg3, fontPosition3, font3.size, 2, WHITE);

    EndDrawing();
    //----------------------------------------------------------------------------------
}