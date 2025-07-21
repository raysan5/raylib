/*******************************************************************************************
*
*   raylib [shapes] example - Double Pendulum
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by JoeCheong (@Joecheong2006) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 JoeCheong (@Joecheong2006)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>       // Required for: sin(), cos(), PI

//----------------------------------------------------------------------------------
// Macro Helpers
//----------------------------------------------------------------------------------
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define CENTER_X SCREEN_WIDTH * 0.5
#define CENTER_Y SCREEN_HEIGHT * 0.5 - 100

// Constant for Simulation
#define SIMULATION_STEPS 30
#define G 9.81

// Helpers for Angles Conversion
#define RAD(x) x * PI / 180.0
#define DEG(x) x * 180.0 / PI

#define scalar float

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static Vector2 CalculatePendulumEndPoint(scalar l, scalar theta);
static Vector2 CalculateDoublePendulumEndPoint(scalar l1, scalar theta1, scalar l2, scalar theta2);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
	SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib [shapes] example - Double Pendulum");

    // Simulation Paramters
    //--------------------------------------------------------------------------------------
    scalar l1 = 15, m1 = 0.2, theta1 = RAD(170), w1 = 0;
    scalar l2 = 15, m2 = 0.1, theta2 = RAD(0), w2 = 0;
    scalar lengthScaler = 0.1;
    scalar totalM = m1 + m2;

    Vector2 previousPosition = CalculateDoublePendulumEndPoint(l1, theta1, l2, theta2);
    previousPosition.x += CENTER_X;
    previousPosition.y += CENTER_Y;

    // Scale length
    scalar L1 = l1 * lengthScaler;
    scalar L2 = l2 * lengthScaler;

    // Draw Parameters
    //--------------------------------------------------------------------------------------
    int lineThick = 20, trailThick = 2;
    float fateAlpha = 0.01;
    
    // Create Framebuffer
    //--------------------------------------------------------------------------------------
    RenderTexture2D target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        scalar dt = GetFrameTime();
        scalar step = dt / SIMULATION_STEPS, step2 = step * step;

        // Update Physics - larger steps = better approximation
        //----------------------------------------------------------------------------------
        for (int i = 0; i < SIMULATION_STEPS; ++i)
        {
            scalar delta = theta1 - theta2;
            scalar sinD = sin(delta), cosD = cos(delta), cos2D = cos(2 * delta);
            scalar ww1 = w1 * w1, ww2 = w2 * w2;

            // Calculate a1
            scalar a1 = (-G * (2 * m1 + m2) * sin(theta1) 
                         - m2 * G * sin(theta1 - 2 * theta2) 
                         - 2 * sinD * m2 * (ww2 * L2 + ww1 * L1 * cosD))
                         / (L1 * (2 * m1 + m2 - m2 * cos2D));

            // Calculate a2
            scalar a2 = (2 * sinD * (ww1 * L1 * totalM
                         + G * totalM * cos(theta1) 
                         + ww2 * L2 * m2 * cosD))
                         / (L2 * (2 * m1 + m2 - m2 * cos2D));

            // Update thetas
            theta1 += w1 * step + 0.5 * a1 * step2;
            theta2 += w2 * step + 0.5 * a2 * step2;

            // Update omegas
            w1 += a1 * step;
            w2 += a2 * step;
        }
        //----------------------------------------------------------------------------------

        // Calculate position
        Vector2 currentPosition = CalculateDoublePendulumEndPoint(l1, theta1, l2, theta2);
        currentPosition.x += CENTER_X;
        currentPosition.y += CENTER_Y;

        // Draw to framebuffer
        //----------------------------------------------------------------------------------
        BeginTextureMode(target);

            // Draw a transparent rectangle - smaller alpha = longer trails
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, fateAlpha));
            
            // Draw trail
            DrawCircleV(previousPosition, trailThick, RED);
            DrawLineEx(previousPosition, currentPosition, trailThick * 2, RED);

        EndTextureMode();
        //----------------------------------------------------------------------------------

        // Update previous position
        previousPosition = currentPosition;

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw Trails Texture
            DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height },
                             (Vector2){ 0, 0 }, WHITE);

            // Draw Double Pendulum
            DrawRectanglePro((Rectangle){ CENTER_X, CENTER_Y, 10 * l1, lineThick },
                             (Vector2){0, lineThick * 0.5}, 90 - DEG(theta1), RAYWHITE);

            Vector2 endpoint1 = CalculatePendulumEndPoint(l1, theta1);
            DrawRectanglePro((Rectangle){ CENTER_X + endpoint1.x, CENTER_Y + endpoint1.y, 10 * l2, lineThick },
                             (Vector2){0, lineThick * 0.5}, 90 - DEG(theta2), RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Calculate Pendulum End Point
static Vector2 CalculatePendulumEndPoint(scalar l, scalar theta)
{
    return (Vector2){ 10 * l * sin(theta), 10 * l * cos(theta) };
}

// Calculate Double Pendulum End Point
static Vector2 CalculateDoublePendulumEndPoint(scalar l1, scalar theta1, scalar l2, scalar theta2)
{
    Vector2 endpoint1 = CalculatePendulumEndPoint(l1, theta1);
    Vector2 endpoint2 = CalculatePendulumEndPoint(l2, theta2);
    return (Vector2){ endpoint1.x + endpoint2.x, endpoint1.y + endpoint2.y };
}
