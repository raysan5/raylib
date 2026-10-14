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

#include "reasings.h"       // Required for: Easing functions
#include "raymath.h"        // Required for: Clamp()

#define MAX_TEXT_LENGTH     64
#define TEXT_SPACING         4

// Animated text type
typedef struct {
    char str[MAX_TEXT_LENGTH]; // Text to animate
    Vector2 position;       // Text position on screen
    float delta;            // Intensity of the effect (does not apply for flashing)
    float offset;           // Time offset (in frames) between each char
    float duration;         // Time duration (in frames) of the animation
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

    char animations[3][MAX_TEXT_LENGTH] = { "Waving", "Flashing", "Shaking" };
    int animationsSize = sizeof(animations)/sizeof(animations[0]);
    int currentAnimation = 0;

    AnimatedText text = { "This is an animated text!", { 200, 250 }, 24.0f, 6, 60 };
    int frameCount = 0;

    Vector2 glyphPosition[MAX_TEXT_LENGTH] = { 0 };
    unsigned char glyphAlpha[MAX_TEXT_LENGTH] = { 0 };

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frameCount++;

        if (IsKeyPressed(KEY_S)) text.delta = Clamp(text.delta - 2.0f, 10.0f, 60.0f);
        else if (IsKeyPressed(KEY_W)) text.delta = Clamp(text.delta + 2.0f, 10.0f, 60.0f);
        if (IsKeyPressed(KEY_A)) text.duration = (int) Clamp((float) text.duration - 5.0f, 20.0f, 120.0f);
        else if (IsKeyPressed(KEY_D)) text.duration = (int) Clamp((float) text.duration + 5.0f, 20.0f, 120.0f);
        if (IsKeyPressed(KEY_Z)) text.offset = Clamp(text.offset - 1.0f, 0.0f, 15.0f);
        else if (IsKeyPressed(KEY_X)) text.offset = Clamp(text.offset + 1.0f, 0.0f, 15.0f);
        if (IsKeyPressed(KEY_Q)) currentAnimation = currentAnimation? currentAnimation - 1 : animationsSize - 1;
        else if (IsKeyPressed(KEY_E)) currentAnimation = (currentAnimation + 1)%animationsSize;

        for (unsigned int i = 0, offsetX = 0; i < TextLength(text.str); i++)
        {
            // NOTE: Calculating every glyph position by its width
            glyphPosition[i] = (Vector2) { text.position.x + offsetX, text.position.y };
            glyphAlpha[i] = 255U;

            offsetX += (MeasureText(TextSubtext(text.str, i, 1), 30) + TEXT_SPACING);
        }

        switch (currentAnimation)
        {
            case 0: // Waving animation
            {
                for (unsigned int i = 0; i < TextLength(text.str); i++)
                {
                    float time = (int) (frameCount + i*text.offset)%(int)text.duration;

                    if (time < text.duration/2) glyphPosition[i].y = EaseSineIn(time, text.position.y, text.delta, text.duration);
                    else glyphPosition[i].y = EaseSineOut(time, text.position.y + text.delta, -text.delta, text.duration);
                }
            } break;
            case 1: // Flashing animation
            {
                for (unsigned int i = 0; i < TextLength(text.str); i++)
                {
                    float time = (int) (frameCount + i*text.offset)%(int)text.duration;
                    glyphAlpha[i] = (unsigned char) EaseSineInOut(time, 255.0f, -255.0f, text.duration);
                }
            } break;
            case 2: // Shaking animation
            {
                for (unsigned int i = 0; i < TextLength(text.str); i++)
                {
                    float time = (int) (frameCount + i*text.offset)%(int)text.duration;
                    float deltaX = (float) GetRandomValue(-text.delta/10, text.delta/10);
                    float deltaY = (float) GetRandomValue(-text.delta/10, text.delta/10);

                    if (time < (text.duration/2))
                    {
                        glyphPosition[i].x = EaseLinearNone(time, glyphPosition[i].x, deltaX, text.duration/2);
                        glyphPosition[i].y = EaseLinearNone(time, text.position.y, deltaY, text.duration/2);
                    }
                    else
                    {
                        glyphPosition[i].x = EaseLinearNone(time - text.duration/2, glyphPosition[i].x + deltaX, -deltaX, text.duration/2);
                        glyphPosition[i].y = EaseLinearNone(time - text.duration/2, text.position.y + deltaY, -deltaY, text.duration/2);
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

        DrawText(TextFormat("Duration (A | D): %d", (int)text.duration), 20, 20, 20, GRAY);
        DrawText(TextFormat("Delta (W | S): %.1f", text.delta), 20, 50, 20, GRAY);
        DrawText(TextFormat("Offset (Z | X): %.1f", text.offset), 20, 80, 20, GRAY);
        DrawText(TextFormat("Curent Animation (Q | E): %s", animations[currentAnimation]), 20, 110, 20, GRAY);

        for (unsigned int i = 0; i < TextLength(text.str); i++)
        {
            // NOTE: Drawing text glyph by glyph, so, spacing is actually not used
            DrawTextEx(GetFontDefault(), TextSubtext(text.str, i, 1), glyphPosition[i],
                GetFontDefault().baseSize*3.0f, 0, (Color) { 200U, 122U, 255U, glyphAlpha[i] });
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
