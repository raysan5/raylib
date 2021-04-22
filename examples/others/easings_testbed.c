/*******************************************************************************************
*
*   raylib [easings] example - Easings Testbed
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Juan Miguel López (@flashback-fx) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Juan Miguel López (@flashback-fx ) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <raylib.h>

#include "easings.h"

#define FONT_SIZE         20

#define D_STEP         20.0f
#define D_STEP_FINE     2.0f
#define D_MIN           1.0f
#define D_MAX       10000.0f

// Easing types
enum EasingTypes {
    EASE_LINEAR_NONE = 0,
    EASE_LINEAR_IN,
    EASE_LINEAR_OUT,
    EASE_LINEAR_IN_OUT,
    EASE_SINE_IN,
    EASE_SINE_OUT,
    EASE_SINE_IN_OUT,
    EASE_CIRC_IN,
    EASE_CIRC_OUT,
    EASE_CIRC_IN_OUT,
    EASE_CUBIC_IN,
    EASE_CUBIC_OUT,
    EASE_CUBIC_IN_OUT,
    EASE_QUAD_IN,
    EASE_QUAD_OUT,
    EASE_QUAD_IN_OUT,
    EASE_EXPO_IN,
    EASE_EXPO_OUT,
    EASE_EXPO_IN_OUT,
    EASE_BACK_IN,
    EASE_BACK_OUT,
    EASE_BACK_IN_OUT,
    EASE_BOUNCE_OUT,
    EASE_BOUNCE_IN,
    EASE_BOUNCE_IN_OUT,
    EASE_ELASTIC_IN,
    EASE_ELASTIC_OUT,
    EASE_ELASTIC_IN_OUT,
    NUM_EASING_TYPES,
    EASING_NONE = NUM_EASING_TYPES
};


static float NoEase(float t, float b, float c, float d);  // NoEase function declaration, function used when "no easing" is selected for any axis


// Easing functions reference data
static const struct {
    const char *name;
    float (*func)(float, float, float, float);
} Easings[] = {
    [EASE_LINEAR_NONE] = { .name = "EaseLinearNone", .func = EaseLinearNone },
    [EASE_LINEAR_IN] = { .name = "EaseLinearIn", .func = EaseLinearIn },
    [EASE_LINEAR_OUT] = { .name = "EaseLinearOut", .func = EaseLinearOut },
    [EASE_LINEAR_IN_OUT] = { .name = "EaseLinearInOut", .func = EaseLinearInOut },
    [EASE_SINE_IN] = { .name = "EaseSineIn", .func = EaseSineIn },
    [EASE_SINE_OUT] = { .name = "EaseSineOut", .func = EaseSineOut },
    [EASE_SINE_IN_OUT] = { .name = "EaseSineInOut", .func = EaseSineInOut },
    [EASE_CIRC_IN] = { .name = "EaseCircIn", .func = EaseCircIn },
    [EASE_CIRC_OUT] = { .name = "EaseCircOut", .func = EaseCircOut },
    [EASE_CIRC_IN_OUT] = { .name = "EaseCircInOut", .func = EaseCircInOut },
    [EASE_CUBIC_IN] = { .name = "EaseCubicIn", .func = EaseCubicIn },
    [EASE_CUBIC_OUT] = { .name = "EaseCubicOut", .func = EaseCubicOut },
    [EASE_CUBIC_IN_OUT] = { .name = "EaseCubicInOut", .func = EaseCubicInOut },
    [EASE_QUAD_IN] = { .name = "EaseQuadIn", .func = EaseQuadIn },
    [EASE_QUAD_OUT] = { .name = "EaseQuadOut", .func = EaseQuadOut },
    [EASE_QUAD_IN_OUT] = { .name = "EaseQuadInOut", .func = EaseQuadInOut },
    [EASE_EXPO_IN] = { .name = "EaseExpoIn", .func = EaseExpoIn },
    [EASE_EXPO_OUT] = { .name = "EaseExpoOut", .func = EaseExpoOut },
    [EASE_EXPO_IN_OUT] = { .name = "EaseExpoInOut", .func = EaseExpoInOut },
    [EASE_BACK_IN] = { .name = "EaseBackIn", .func = EaseBackIn },
    [EASE_BACK_OUT] = { .name = "EaseBackOut", .func = EaseBackOut },
    [EASE_BACK_IN_OUT] = { .name = "EaseBackInOut", .func = EaseBackInOut },
    [EASE_BOUNCE_OUT] = { .name = "EaseBounceOut", .func = EaseBounceOut },
    [EASE_BOUNCE_IN] = { .name = "EaseBounceIn", .func = EaseBounceIn },
    [EASE_BOUNCE_IN_OUT] = { .name = "EaseBounceInOut", .func = EaseBounceInOut },
    [EASE_ELASTIC_IN] = { .name = "EaseElasticIn", .func = EaseElasticIn },
    [EASE_ELASTIC_OUT] = { .name = "EaseElasticOut", .func = EaseElasticOut },
    [EASE_ELASTIC_IN_OUT] = { .name = "EaseElasticInOut", .func = EaseElasticInOut },
    [EASING_NONE] = { .name = "None", .func = NoEase },
};


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [easings] example - easings testbed");

    Vector2 ballPosition = { 100.0f, 200.0f };

    float t = 0.0f;             // Current time (in any unit measure, but same unit as duration)
    float d = 300.0f;           // Total time it should take to complete (duration)
    bool paused = true;
    bool boundedT = true;       // If true, t will stop when d >= td, otherwise t will keep adding td to its value every loop

    int easingX = EASING_NONE;  // Easing selected for x axis
    int easingY = EASING_NONE;  // Easing selected for y axis

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_T)) boundedT = !boundedT;

        // Choose easing for the X axis
        if (IsKeyPressed(KEY_RIGHT))
        {
            easingX++;

            if (easingX > EASING_NONE) easingX = 0;
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            if (easingX == 0) easingX = EASING_NONE;
            else easingX--;
        }

        // Choose easing for the Y axis
        if (IsKeyPressed(KEY_DOWN))
        {
            easingY++;

            if (easingY > EASING_NONE) easingY = 0;
        }
        else if (IsKeyPressed(KEY_UP))
        {
            if (easingY == 0) easingY = EASING_NONE;
            else easingY--;
        }

        // Change d (duration) value
        if (IsKeyPressed(KEY_W) && d < D_MAX - D_STEP) d += D_STEP;
        else if (IsKeyPressed(KEY_Q) && d > D_MIN + D_STEP) d -= D_STEP;

        if (IsKeyDown(KEY_S) && d < D_MAX - D_STEP_FINE) d += D_STEP_FINE;
        else if (IsKeyDown(KEY_A) && d > D_MIN + D_STEP_FINE) d -= D_STEP_FINE;

        // Play, pause and restart controls
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_T) ||
            IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT) ||
            IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP) ||
            IsKeyPressed(KEY_W) || IsKeyPressed(KEY_Q) ||
            IsKeyDown(KEY_S)  || IsKeyDown(KEY_A) ||
            (IsKeyPressed(KEY_ENTER) && (boundedT == true) && (t >= d)))
        {
            t = 0.0f;
            ballPosition.x = 100.0f;
            ballPosition.y = 100.0f;
            paused = true;
        }

        if (IsKeyPressed(KEY_ENTER)) paused = !paused;

        // Movement computation
        if (!paused && ((boundedT && t < d) || !boundedT))
        {
            ballPosition.x = Easings[easingX].func(t, 100.0f, 700.0f - 100.0f, d);
            ballPosition.y = Easings[easingY].func(t, 100.0f, 400.0f - 100.0f, d);
            t += 1.0f;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw information text
            DrawText(TextFormat("Easing x: %s", Easings[easingX].name), 0, FONT_SIZE*2, FONT_SIZE, LIGHTGRAY);
            DrawText(TextFormat("Easing y: %s", Easings[easingY].name), 0, FONT_SIZE*3, FONT_SIZE, LIGHTGRAY);
            DrawText(TextFormat("t (%c) = %.2f d = %.2f", (boundedT == true)? 'b' : 'u', t, d), 0, FONT_SIZE*4, FONT_SIZE, LIGHTGRAY);

            // Draw instructions text
            DrawText("Use ENTER to play or pause movement, use SPACE to restart", 0, GetScreenHeight() - FONT_SIZE*2, FONT_SIZE, LIGHTGRAY);
            DrawText("Use D and W or A and S keys to change duration", 0, GetScreenHeight() - FONT_SIZE*3, FONT_SIZE, LIGHTGRAY);
            DrawText("Use LEFT or RIGHT keys to choose easing for the x axis", 0, GetScreenHeight() - FONT_SIZE*4, FONT_SIZE, LIGHTGRAY);
            DrawText("Use UP or DOWN keys to choose easing for the y axis", 0, GetScreenHeight() - FONT_SIZE*5, FONT_SIZE, LIGHTGRAY);

            // Draw ball
            DrawCircleV(ballPosition, 16.0f, MAROON);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}


// NoEase function, used when "no easing" is selected for any axis. It just ignores all parameters besides b.
static float NoEase(float t, float b, float c, float d)
{
    float burn = t + b + c + d;  // Hack to avoid compiler warning (about unused variables)
    d += burn;

    return b;
}