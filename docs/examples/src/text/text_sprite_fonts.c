/*******************************************************************************************
*
*   raylib [text] example - SpriteFont loading and usage
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - sprite fonts usage");

    const char msg1[50] = "THIS IS A custom SPRITE FONT...";
    const char msg2[50] = "...and this is ANOTHER CUSTOM font...";
    const char msg3[50] = "...and a THIRD one! GREAT! :D";

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
    SpriteFont font1 = LoadSpriteFont("resources/fonts/custom_mecha.png");          // SpriteFont loading
    SpriteFont font2 = LoadSpriteFont("resources/fonts/custom_alagard.png");        // SpriteFont loading
    SpriteFont font3 = LoadSpriteFont("resources/fonts/custom_jupiter_crash.png");  // SpriteFont loading

    Vector2 fontPosition1, fontPosition2, fontPosition3;

    fontPosition1.x = screenWidth/2 - MeasureTextEx(font1, msg1, font1.size, -3).x/2;
    fontPosition1.y = screenHeight/2 - font1.size/2 - 80;

    fontPosition2.x = screenWidth/2 - MeasureTextEx(font2, msg2, font2.size, -2).x/2;
    fontPosition2.y = screenHeight/2 - font2.size/2 - 10;

    fontPosition3.x = screenWidth/2 - MeasureTextEx(font3, msg3, font3.size, 2).x/2;
    fontPosition3.y = screenHeight/2 - font3.size/2 + 50;

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

            DrawTextEx(font1, msg1, fontPosition1, font1.size, -3, WHITE);
            DrawTextEx(font2, msg2, fontPosition2, font2.size, -2, WHITE);
            DrawTextEx(font3, msg3, fontPosition3, font3.size, 2, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(font1);      // SpriteFont unloading
    UnloadSpriteFont(font2);      // SpriteFont unloading
    UnloadSpriteFont(font3);      // SpriteFont unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}