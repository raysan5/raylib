/*******************************************************************************************
*
*   raylib [text] example - Font loading and usage
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - sprite fonts usage");

    const char msg1[50] = "THIS IS A custom SPRITE FONT...";
    const char msg2[50] = "...and this is ANOTHER CUSTOM font...";
    const char msg3[50] = "...and a THIRD one! GREAT! :D";

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
    Font font1 = LoadFont("resources/custom_mecha.png");          // Font loading
    Font font2 = LoadFont("resources/custom_alagard.png");        // Font loading
    Font font3 = LoadFont("resources/custom_jupiter_crash.png");  // Font loading

    Vector2 fontPosition1 = { screenWidth/2 - MeasureTextEx(font1, msg1, font1.baseSize, -3).x/2,
                              screenHeight/2 - font1.baseSize/2 - 80 };

    Vector2 fontPosition2 = { screenWidth/2 - MeasureTextEx(font2, msg2, font2.baseSize, -2).x/2,
                              screenHeight/2 - font2.baseSize/2 - 10 };

    Vector2 fontPosition3 = { screenWidth/2 - MeasureTextEx(font3, msg3, font3.baseSize, 2).x/2,
                              screenHeight/2 - font3.baseSize/2 + 50 };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
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

            DrawTextEx(font1, msg1, fontPosition1, font1.baseSize, -3, WHITE);
            DrawTextEx(font2, msg2, fontPosition2, font2.baseSize, -2, WHITE);
            DrawTextEx(font3, msg3, fontPosition3, font3.baseSize, 2, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(font1);      // Font unloading
    UnloadFont(font2);      // Font unloading
    UnloadFont(font3);      // Font unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}