/*******************************************************************************************
*
*   raylib [shape] example - Translation, rotation and scaling
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by zerasul (@zerasul) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 zerasul (@zerasul) - All rights reserved.
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib shape example - Translation, rotation and scaling");

    // Rectangle variables
    int rectWidth = 100;
    int rectHeight = 100;
    Rectangle rec = { 300,400, rectWidth, rectHeight };
    Vector2 origin = { rectWidth/2.0f, rectHeight/2.0f };
    //rotation angle and scale factors
    float rotation = 0.0f;
    float scale = 1.0f;


    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
       //----------------------------------------------------------------------------------

       //if an arrow key is pressed, move the rectangle in the corresponding direction
       if(IsKeyDown(KEY_RIGHT)) rec.x += 2.0f;
       if(IsKeyDown(KEY_LEFT)) rec.x -= 2.0f;
       if(IsKeyDown(KEY_UP)) rec.y -= 2.0f;
       if(IsKeyDown(KEY_DOWN)) rec.y += 2.0f;

       //if R or T keys are pressed, rotate the rectangle
       if(IsKeyDown(KEY_R)) rotation += 2.0f;
       if(IsKeyDown(KEY_T)) rotation -= 2.0f;

       //if + or - keys are pressed, scale the rectangle
       if(IsKeyDown(KEY_KP_ADD)) scale += 0.02f;
       if(IsKeyDown(KEY_KP_SUBTRACT)) scale -= 0.02f;

       //scale limits
       if(scale < 0.1f) scale = 0.1f;
       if(scale > 3.0f) scale = 3.0f;

       //recalcuate rectangle size and origin based on scale
       rec.width = rectWidth * scale;
       rec.height = rectHeight * scale;
       origin = (Vector2){ rec.width/2.0f, rec.height/2.0f };

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawText("Translation, Rotation and Scale Example", 10, 10, 30, LIGHTGRAY);
            DrawText("Use arrow keys to move rectangle", 10, 50, 20, DARKGRAY);
            DrawText("Use R and T keys to rotate rectangle", 10, 80, 20, DARKGRAY);
            DrawText("Use + and - keys to scale rectangle", 10, 110, 20, DARKGRAY);

            DrawRectanglePro(rec, origin, rotation, RED); // Rotation around rectangle center

        EndDrawing();
        //----------------------------------------------------------------------------------
    }


    // De-Initialization
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}