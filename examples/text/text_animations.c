/*******************************************************************************************
*
*   raylib [text] example - animations
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example contributed by Gabriel Piangers (@gabriel-piangers) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Gabriel Piangers (@gabriel-piangers)
*
********************************************************************************************/

#include "raylib.h"
#include "../shapes/reasings.h" // Required for Easing functions
#include "raymath.h" // Required for Clamp
#include <stdio.h>

typedef struct {
    char str[100];
    Vector2 position;
    float delta; // Intensity of the effect (does not apply for flashing)
    float offset; // Time offset (in frames) between each char 
    int duration; // Time duration (in frames) of the animation
} AnimatedText;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - animations");

    char animations[3][51] = {
      "Waving",
      "Flashing",
      "Shaking"
    };
    int curAnimation = 0;

    AnimatedText text = { "This is an animated text!", { 150, 250 }, 24.0f, 6, 60 };
    int frameCount = 0;

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frameCount++;
        if (IsKeyPressed(KEY_S)) text.delta = Clamp(text.delta - 2.0f, 10.0f, 60.0f);
        else if (IsKeyPressed(KEY_W)) text.delta = Clamp(text.delta + 2.0, 10.0f, 60.0f);
        if (IsKeyPressed(KEY_A)) text.duration = (int) Clamp((float) text.duration - 5.0f, 20.0f, 120.0f);
        else if (IsKeyPressed(KEY_D)) text.duration = (int) Clamp((float) text.duration + 5.0f, 20.0f, 120.0f);
        if (IsKeyPressed(KEY_Z)) text.offset = Clamp(text.offset - 1.0f, 0.0f, 15.0f);
        else if (IsKeyPressed(KEY_X)) text.offset = Clamp(text.offset + 1.0f, 0.0f, 15.0f);
        if (IsKeyPressed(KEY_Q)) curAnimation = curAnimation ? curAnimation - 1 : 2;
        else if (IsKeyPressed(KEY_E)) curAnimation = (curAnimation + 1)%3;
        
        Vector2 charPos[51] = { 0 };
        unsigned char charAlpha[51] = { 0 }; 

        for (int i=0; i<TextLength(text.str); i++) 
        {
            charPos[i] = (Vector2) { text.position.x + 20*i, text.position.y };   
            charAlpha[i] = 255U;
        }    

        switch (curAnimation)
        {
            case 0: // Waving animation
            {
                for (int i=0; i<TextLength(text.str); i++)
                {
                    float time = (int) (frameCount + i*text.offset)%text.duration;
                    if (time < text.duration/2) charPos[i].y = EaseSineIn(time, text.position.y, text.delta, text.duration);
                    else charPos[i].y = EaseSineOut(time, text.position.y + text.delta, -text.delta, text.duration);
                }
            } break;
            case 1: // Flashing animation
            {
                for (int i=0; i<TextLength(text.str); i++)
                {
                    float time = (int) (frameCount + i*text.offset)%text.duration;
                    charAlpha[i] = (unsigned char) EaseSineInOut(time, 255.0f, -255.0f, text.duration);
                }
            } break;
            case 2: // Shaking animation
            {
                for (int i=0; i<TextLength(text.str); i++)
                {
                    float time = (int) (frameCount + i*text.offset)%text.duration;
                    float deltaX = (float) GetRandomValue(-text.delta/10, text.delta/10);
                    float deltaY = (float) GetRandomValue(-text.delta/10, text.delta/10);
                    if (time < text.duration/2) 
                    {
                        charPos[i].x = EaseLinearNone(time, text.position.x + 20*i, deltaX, text.duration/2);
                        charPos[i].y = EaseLinearNone(time, text.position.y, deltaY, text.duration/2);
                    }
                    else 
                    {
                        charPos[i].x = EaseLinearNone(time - text.duration/2, text.position.x + 20*i + deltaX, -deltaX, text.duration/2);
                        charPos[i].y = EaseLinearNone(time - text.duration/2, text.position.y + deltaY, -deltaY, text.duration/2);
                    }
                }
            } break;
            default: break;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText(TextFormat("Duration (A | D): %d", text.duration), 20, 20, 20, GRAY);
        DrawText(TextFormat("Delta (W | S): %.1f", text.delta), 20, 50, 20, GRAY);
        DrawText(TextFormat("Offset (Z | X): %.1f", text.offset), 20, 80, 20, GRAY);
        DrawText(TextFormat("Curent Animation (Q | E): %s", animations[curAnimation]), 20, 110, 20, GRAY);

        for (int i=0; i<TextLength(text.str); i++)
        {
             char str[2] = { text.str[i], 0 };
             DrawText(str, charPos[i].x, charPos[i].y, 24, (Color) { 200U, 122U, 255U, charAlpha[i] });
        }
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
