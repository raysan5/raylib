/*******************************************************************************************
*
*   raylib [textures] example - srcrec dstrec
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 1.3, last time updated with raylib 6.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2025 Ramon Santamaria (@raysan5)
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

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - srcrec dstrec");

    // Load texture, spritesheet of character containing 6 frames
    Texture2D texture = LoadTexture("resources/scarfy.png");

    // NOTE: Spritesheet contains 6 frames in horizontal, 
    // so dividing by 6 to get the width of one frame
    int frameWidth = texture.width/6;
    int frameHeight = texture.height;

    // Source rectangle: rectangle piece of the texture to use for drawing
    Rectangle sourceRec = { 0.0f, 0.0f, (float)frameWidth, (float)frameHeight };

    // Destination rectangle: screen-coordinates rectangle to draw the source part of texture
    // NOTE: Defines position on screen and scaling, set to center of screen with a scale x2
    Rectangle destRec = { screenWidth/2.0f, screenHeight/2.0f, frameWidth*2.0f, frameHeight*2.0f };

    // Origin of the texture (rotation/scale point)
    // NOTE: Relative to destination rectangle size, set at the center of it
    Vector2 origin = { (float)destRec.width/2, (float)destRec.height/2 };

    int rotation = 0; // Rotation in degrees

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation++;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // NOTE: Using DrawTexturePro() we can easily rotate and scale the part of the texture we draw
            // sourceRec defines the part of the texture we use for drawing
            // destRec defines the rectangle where our texture part will fit (scaling it to fit)
            // origin defines the point of the texture used as reference for rotation and scaling
            // rotation defines the texture rotation (using origin as rotation point)
            DrawTexturePro(texture, sourceRec, destRec, origin, (float)rotation, WHITE);

            DrawLine((int)destRec.x, 0, (int)destRec.x, screenHeight, GRAY);
            DrawLine(0, (int)destRec.y, screenWidth, (int)destRec.y, GRAY);

            DrawText("(c) Scarfy spritesheet by Eiden Marsal", screenWidth - 220, screenHeight - 20, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading

    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
