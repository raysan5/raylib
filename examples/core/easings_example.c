/*******************************************************************************************
*
*   raylib [easings] example
*
*   This example has been created using raylib 2.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2019 Juan Miguel López
*
********************************************************************************************/


#include <raylib.h>
#include "easings.h"


// Application constants
#define SCR_WIDTH 800
#define SCR_HEIGHT 450
#define BALL_RADIUS 16.0f
#define BALL_COLOR MAROON
#define PAD 80.0f
#define START_X (0.0f + (BALL_RADIUS) + (PAD))
#define END_X ((SCR_WIDTH) - ((BALL_RADIUS) + (PAD)))
#define START_Y (0.0f + (BALL_RADIUS) + (PAD))
#define END_Y ((SCR_HEIGHT) - ((BALL_RADIUS) + (PAD)))
#define T_ADVANCE 1.0f
#define D_DFT 300.0f
#define TARGET_FPS 60
#define BG_COLOR RAYWHITE
#define TEXT_COLOR LIGHTGRAY
#define FONT_SIZE 20
#define D_STEP 20.0f
#define D_STEP_FINE 2.0f
#define D_MIN 1.0f
#define D_MAX 10000.0f

// Application control keys
#define KEY_NEXT_EASE_X KEY_RIGHT
#define KEY_PREV_EASE_X KEY_LEFT
#define KEY_NEXT_EASE_Y KEY_DOWN
#define KEY_PREV_EASE_Y KEY_UP
#define KEY_INCR_D_STEP KEY_W
#define KEY_DECR_D_STEP KEY_Q
#define KEY_INCR_D_FINE KEY_S
#define KEY_DECR_D_FINE KEY_A
#define KEY_PLAY_PAUSE KEY_ENTER
#define KEY_RESTART KEY_SPACE
#define KEY_TOGGLE_UNBOUNDED_T KEY_T


// Easing types
enum EasingTypes {
    EASE_LINEAR_NONE,
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
    [EASE_LINEAR_NONE] = {
        .name = "EaseLinearNone",
        .func = EaseLinearNone,
    },
    [EASE_LINEAR_IN] = {
        .name = "EaseLinearIn",
        .func = EaseLinearIn,
    },
    [EASE_LINEAR_OUT] = {
        .name = "EaseLinearOut",
        .func = EaseLinearOut,
    },
    [EASE_LINEAR_IN_OUT] = {
        .name = "EaseLinearInOut",
        .func = EaseLinearInOut,
    },
    [EASE_SINE_IN] = {
        .name = "EaseSineIn",
        .func = EaseSineIn,
    },
    [EASE_SINE_OUT] = {
        .name = "EaseSineOut",
        .func = EaseSineOut,
    },
    [EASE_SINE_IN_OUT] = {
        .name = "EaseSineInOut",
        .func = EaseSineInOut,
    },
    [EASE_CIRC_IN] = {
        .name = "EaseCircIn",
        .func = EaseCircIn,
    },
    [EASE_CIRC_OUT] = {
        .name = "EaseCircOut",
        .func = EaseCircOut,
    },
    [EASE_CIRC_IN_OUT] = {
        .name = "EaseCircInOut",
        .func = EaseCircInOut,
    },
    [EASE_CUBIC_IN] = {
        .name = "EaseCubicIn",
        .func = EaseCubicIn,
    },
    [EASE_CUBIC_OUT] = {
        .name = "EaseCubicOut",
        .func = EaseCubicOut,
    },
    [EASE_CUBIC_IN_OUT] = {
        .name = "EaseCubicInOut",
        .func = EaseCubicInOut,
    },
    [EASE_QUAD_IN] = {
        .name = "EaseQuadIn",
        .func = EaseQuadIn,
    },
    [EASE_QUAD_OUT] = {
        .name = "EaseQuadOut",
        .func = EaseQuadOut,
    },
    [EASE_QUAD_IN_OUT] = {
        .name = "EaseQuadInOut",
        .func = EaseQuadInOut,
    },
    [EASE_EXPO_IN] = {
        .name = "EaseExpoIn",
        .func = EaseExpoIn,
    },
    [EASE_EXPO_OUT] = {
        .name = "EaseExpoOut",
        .func = EaseExpoOut,
    },
    [EASE_EXPO_IN_OUT] = {
        .name = "EaseExpoInOut",
        .func = EaseExpoInOut,
    },
    [EASE_BACK_IN] = {
        .name = "EaseBackIn",
        .func = EaseBackIn,
    },
    [EASE_BACK_OUT] = {
        .name = "EaseBackOut",
        .func = EaseBackOut,
    },
    [EASE_BACK_IN_OUT] = {
        .name = "EaseBackInOut",
        .func = EaseBackInOut,
    },
    [EASE_BOUNCE_OUT] = {
        .name = "EaseBounceOut",
        .func = EaseBounceOut,
    },
    [EASE_BOUNCE_IN] = {
        .name = "EaseBounceIn",
        .func = EaseBounceIn,
    },
    [EASE_BOUNCE_IN_OUT] = {
        .name = "EaseBounceInOut",
        .func = EaseBounceInOut,
    },
    [EASE_ELASTIC_IN] = {
        .name = "EaseElasticIn",
        .func = EaseElasticIn,
    },
    [EASE_ELASTIC_OUT] = {
        .name = "EaseElasticOut",
        .func = EaseElasticOut,
    },
    [EASE_ELASTIC_IN_OUT] = {
        .name = "EaseElasticInOut",
        .func = EaseElasticInOut,
    },
    [EASING_NONE] = {
        .name = "None",
        .func = NoEase,
    },
};


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    Vector2 ballPos = { .x = START_X, .y = START_Y };
    float t = 0.0f;        // Current time (in any unit measure, but same unit as duration)
    float d = D_DFT;       // Total time it should take to complete (duration)
    bool paused = true;
    bool boundedT = true;  // If true, t will stop when d >= td, otherwise t will keep adding td to its value every loop

    enum EasingTypes easingX = EASING_NONE;  // Easing selected for x axis
    enum EasingTypes easingY = EASING_NONE;  // Easing selected for y axis

    InitWindow(SCR_WIDTH, SCR_HEIGHT, "raylib [easings] example");
    SetTargetFPS(TARGET_FPS);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_TOGGLE_UNBOUNDED_T))
            boundedT = 1 - boundedT;

        // Choose easing for the X axis
        if (IsKeyPressed(KEY_NEXT_EASE_X))
        {
            ++easingX;

            if (easingX > EASING_NONE)
                easingX = 0;
        }
        else if (IsKeyPressed(KEY_PREV_EASE_X))
        {
            if (easingX == 0)
                easingX = EASING_NONE;
            else
                --easingX;
        }

        // Choose easing for the Y axis
        if (IsKeyPressed(KEY_NEXT_EASE_Y))
        {
            ++easingY;

            if (easingY > EASING_NONE)
                easingY = 0;
        }
        else if (IsKeyPressed(KEY_PREV_EASE_Y))
        {
            if (easingY == 0)
                easingY = EASING_NONE;
            else
                --easingY;
        }

        // Change d (duration) value
        if (IsKeyPressed(KEY_INCR_D_STEP) && d < D_MAX - D_STEP)
            d += D_STEP;
        else if (IsKeyPressed(KEY_DECR_D_STEP) && d > D_MIN + D_STEP)
            d -= D_STEP;

        if (IsKeyDown(KEY_INCR_D_FINE) && d < D_MAX - D_STEP_FINE)
            d += D_STEP_FINE;
        else if (IsKeyDown(KEY_DECR_D_FINE) && d > D_MIN + D_STEP_FINE)
            d -= D_STEP_FINE;

        // Play, pause and restart controls
        if (IsKeyPressed(KEY_RESTART) ||
            IsKeyPressed(KEY_NEXT_EASE_X) || IsKeyPressed(KEY_PREV_EASE_X) ||
            IsKeyPressed(KEY_NEXT_EASE_Y) || IsKeyPressed(KEY_PREV_EASE_Y) ||
            IsKeyPressed(KEY_INCR_D_STEP) || IsKeyPressed(KEY_DECR_D_STEP) ||
            IsKeyPressed(KEY_TOGGLE_UNBOUNDED_T) ||
            IsKeyDown(KEY_INCR_D_FINE)  || IsKeyDown(KEY_DECR_D_FINE) ||
            (IsKeyPressed(KEY_PLAY_PAUSE) && boundedT == true && t >= d))
        {
            t = 0.0f;
            ballPos.x = START_X;
            ballPos.y = START_Y;
            paused = true;
        }

        if (IsKeyPressed(KEY_PLAY_PAUSE))
            paused = 1 - paused;

        // Movement computation
        if ((paused == false) &&
            ((boundedT == true && t < d) || boundedT == false))
        {
            ballPos.x = Easings[easingX].func(t, START_X, END_X - START_X, d);
            ballPos.y = Easings[easingY].func(t, START_Y, END_Y - START_Y, d);
            t += T_ADVANCE;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BG_COLOR);

            // Draw information text
            int line = 0;

            DrawText(TextFormat("Easing x: %s", Easings[easingX].name), 0, FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);
            DrawText(TextFormat("Easing y: %s", Easings[easingY].name), 0, FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);
            DrawText(TextFormat("t (%c) = %.2f d = %.2f", (boundedT == true)? 'b' : 'u', t, d),
                     0, FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);

            // Draw instructions text
            line = 1;
            DrawText("Use ENTER to play or pause movement, use SPACE to restart", 0, SCR_HEIGHT - FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);
            DrawText("Use D and W or A and S keys to change duration", 0, SCR_HEIGHT - FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);
            DrawText("Use LEFT or RIGHT keys to choose easing for the x axis", 0, SCR_HEIGHT - FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);
            DrawText("Use UP or DOWN keys to choose easing for the y axis", 0, SCR_HEIGHT - FONT_SIZE * (line++), FONT_SIZE, TEXT_COLOR);

            // Draw ball
            DrawCircleV(ballPos, BALL_RADIUS, BALL_COLOR);

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