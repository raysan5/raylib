/*******************************************************************************************
*
*   raylib example - Bunnymark
*
*   This example has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: malloc(), free()

#define MAX_BUNNIES     100000       // 100K bunnies

typedef struct Bunny {
    Vector2 position;
    Vector2 speed;
    Color color;
} Bunny;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1280;
    int screenHeight = 960;

    InitWindow(screenWidth, screenHeight, "raylib example - Bunnymark");
    
    Texture2D texBunny = LoadTexture("resources/wabbit_alpha.png");
    
    Bunny *bunnies = (Bunny *)malloc(MAX_BUNNIES*sizeof(Bunny));          // Bunnies array
    int bunniesCount = 0;    // Bunnies counter

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            // Create more bunnies
            for (int i = 0; i < 100; i++)
            {
                bunnies[bunniesCount].position = GetMousePosition();
                bunnies[bunniesCount].speed.x = (float)GetRandomValue(250, 500)/60.0f;
                bunnies[bunniesCount].speed.y = (float)(GetRandomValue(250, 500) - 500)/60.0f;
                bunniesCount++;
            }
        }
        
        // Update bunnies
        for (int i = 0; i < bunniesCount; i++)
        {
            bunnies[i].position.x += bunnies[i].speed.x;
            bunnies[i].position.y += bunnies[i].speed.y;

            if ((bunnies[i].position.x > GetScreenWidth()) || (bunnies[i].position.x < 0)) bunnies[i].speed.x *= -1;
            if ((bunnies[i].position.y > GetScreenHeight()) || (bunnies[i].position.y < 0)) bunnies[i].speed.y *= -1;
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            for (int i = 0; i <= bunniesCount; i++)
            {
                // NOTE: When internal QUADS batch limit is reached, a draw call is launched and
                // batching buffer starts being filled again; before launching the draw call,
                // updated vertex data from internal buffer is send to GPU... it seems it generates
                // a stall and consequently a frame drop, limiting number of bunnies drawn at 60 fps
                DrawTexture(texBunny, bunnies[i].position.x, bunnies[i].position.y, RAYWHITE);
            }
            
            DrawRectangle(0, 0, screenWidth, 40, LIGHTGRAY);
            DrawText("raylib bunnymark", 10, 10, 20, DARKGRAY);
            DrawText(FormatText("bunnies: %i", bunniesCount), 400, 10, 20, RED);
            
            DrawFPS(260, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(bunnies);
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}