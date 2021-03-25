/*******************************************************************************************
*
*   raylib [shapes] example - Draw Textured Polygon
*
*   This example has been created using raylib 99.98 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*   Copyright (c) 2021 Chris Camacho (codifies - bedroomcoders.co.uk)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    int numPnts = 11;  // 10 points and back to the start

    Vector2 tPnts[] = {
        (Vector2){.75,  0},
        (Vector2){.25,  0},
        (Vector2){0,    .5},
        (Vector2){0,    .75},
        (Vector2){.25,  1},
        (Vector2){.375, .875},
        (Vector2){.625, .875},
        (Vector2){.75,  1},
        (Vector2){1,    .75},
        (Vector2){1,    .5},
        (Vector2){.75,  0}  // close the poly
    };

    Vector2 pnts[numPnts];

    // create the poly coords from the UV's
    // you don't have to do this you can specify
    // them however you want
    for (int i=0; i < numPnts; i++)
    {
        pnts[i].x = (tPnts[i].x - 0.5) * 256.0;
        pnts[i].y = (tPnts[i].y - 0.5) * 256.0;
    }

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - Textured Polygon");

    Texture tex = LoadTexture("resources/cat.png");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    float ang = 0;
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Update your variables here
        //----------------------------------------------------------------------------------
        ang++;
        
        Vector2 dPnts[numPnts];
        for (int i = 0; i < numPnts; i++)
        {
            dPnts[i] = Vector2Rotate(pnts[i], ang);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Textured Polygon", 20, 20, 20, DARKGRAY);
            
            DrawTexturePoly(tex, screenWidth/2, screenHeight/2,
                                dPnts, tPnts, numPnts, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    
    UnloadTexture(tex);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
