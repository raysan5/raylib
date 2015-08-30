/*******************************************************************************************
*
*   raylib [text] example - Font selector
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - font selector");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    SpriteFont fonts[8];        // SpriteFont array

    fonts[0] = LoadSpriteFont("resources/fonts/alagard.rbmf");       // SpriteFont loading
    fonts[1] = LoadSpriteFont("resources/fonts/pixelplay.rbmf");     // SpriteFont loading
    fonts[2] = LoadSpriteFont("resources/fonts/mecha.rbmf");         // SpriteFont loading
    fonts[3] = LoadSpriteFont("resources/fonts/setback.rbmf");       // SpriteFont loading
    fonts[4] = LoadSpriteFont("resources/fonts/romulus.rbmf");       // SpriteFont loading
    fonts[5] = LoadSpriteFont("resources/fonts/pixantiqua.rbmf");    // SpriteFont loading
    fonts[6] = LoadSpriteFont("resources/fonts/alpha_beta.rbmf");    // SpriteFont loading
    fonts[7] = LoadSpriteFont("resources/fonts/jupiter_crash.rbmf"); // SpriteFont loading

    int currentFont = 0;        // Selected font

    Color colors[8] = { MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, LIME, GOLD, RED };

    const char fontNames[8][20] = { "[0] Alagard", "[1] PixelPlay", "[2] MECHA", "[3] Setback",
                                    "[4] Romulus", "[5] PixAntiqua", "[6] Alpha Beta", "[7] Jupiter Crash" };

    const char text[50] = "THIS is THE FONT you SELECTED!";     // Main text

    Vector2 textSize = MeasureTextEx(fonts[currentFont], text, fonts[currentFont].size*3, 1);

    Vector2 mousePoint;

    Color btnNextOutColor = DARKBLUE;       // Button color (outside line)
    Color btnNextInColor = SKYBLUE;         // Button color (inside)

    int framesCounter = 0;      // Useful to count frames button is 'active' = clicked
    
    int positionY = 180;        // Text selector and button Y position
    
    Rectangle btnNextRec = { 673, positionY, 109, 44 };    // Button rectangle (useful for collision)

    SetTargetFPS(60);           // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Keyboard-based font selection (easy)
        if (IsKeyPressed(KEY_RIGHT))
        {
            if (currentFont < 7) currentFont++;
        }

        if (IsKeyPressed(KEY_LEFT))
        {
            if (currentFont > 0) currentFont--;
        }
        
        if (IsKeyPressed('0')) currentFont = 0;
        else if (IsKeyPressed('1')) currentFont = 1;
        else if (IsKeyPressed('2')) currentFont = 2;
        else if (IsKeyPressed('3')) currentFont = 3;
        else if (IsKeyPressed('4')) currentFont = 4;
        else if (IsKeyPressed('5')) currentFont = 5;
        else if (IsKeyPressed('6')) currentFont = 6;
        else if (IsKeyPressed('7')) currentFont = 7;

        // Mouse-based font selection (NEXT button logic)
        mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, btnNextRec))
        {
            // Mouse hover button logic
            if (framesCounter == 0)
            {
                btnNextOutColor = DARKPURPLE;
                btnNextInColor = PURPLE;
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                framesCounter = 20;         // Frames button is 'active'
                btnNextOutColor = MAROON;
                btnNextInColor = RED;
            }
        }
        else
        {
            // Mouse not hover button
            btnNextOutColor = DARKBLUE;
            btnNextInColor = SKYBLUE;
        }
        
        if (framesCounter > 0) framesCounter--;

        if (framesCounter == 1)     // We change font on frame 1
        {
            currentFont++;
            if (currentFont > 7) currentFont = 0;
        }

        // Text measurement for better positioning on screen
        textSize = MeasureTextEx(fonts[currentFont], text, fonts[currentFont].size*3, 1);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawText("font selector - use arroys, button or numbers", 160, 80, 20, DARKGRAY);
            DrawLine(120, 120, 680, 120, DARKGRAY);

            DrawRectangle(18, positionY, 644, 44, DARKGRAY);
            DrawRectangle(20, positionY + 2, 640, 40, LIGHTGRAY);
            DrawText(fontNames[currentFont], 30, positionY + 13, 20, BLACK);
            DrawText("< >", 610, positionY + 8, 30, BLACK);

            DrawRectangleRec(btnNextRec, btnNextOutColor);
            DrawRectangle(675, positionY + 2, 105, 40, btnNextInColor);
            DrawText("NEXT", 700, positionY + 13, 20, btnNextOutColor);

            DrawTextEx(fonts[currentFont], text, (Vector2){ screenWidth/2 - textSize.x/2,
                       260 + (70 - textSize.y)/2 }, fonts[currentFont].size*3,
                       1, colors[currentFont]);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int i = 0; i < 8; i++) UnloadSpriteFont(fonts[i]);       // SpriteFont(s) unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}