/*******************************************************************************************
*
*   raylib [shapes] example - double pendulum
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

// Constant for Simulation
#define SIMULATION_STEPS 30
#define G 9.81f

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static Vector2 CalculatePendulumEndPoint(float l, float theta);
static Vector2 CalculateDoublePendulumEndPoint(float l1, float theta1, float l2, float theta2);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - double pendulum");

    // Simulation Parameters
    float l1 = 15.0f, m1 = 0.2f, theta1 = DEG2RAD*170, w1 = 0;
    float l2 = 15.0f, m2 = 0.1f, theta2 = DEG2RAD*0, w2 = 0;
    float lengthScaler = 0.1f;
    float totalM = m1 + m2;

    Vector2 previousPosition = CalculateDoublePendulumEndPoint(l1, theta1, l2, theta2);
    previousPosition.x += (screenWidth/2);
    previousPosition.y += (screenHeight/2 - 100);

    // Scale length
    float L1 = l1*lengthScaler;
    float L2 = l2*lengthScaler;

    // Draw parameters
    float lineThick = 20, trailThick = 2;
    float fateAlpha = 0.01f;

    // Create framebuffer
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float dt = GetFrameTime();
        float step = dt/SIMULATION_STEPS, step2 = step*step;

        // Update Physics - larger steps = better approximation
        for (int i = 0; i < SIMULATION_STEPS; i++)
        {
            float delta = theta1 - theta2;
            float sinD = sinf(delta), cosD = cosf(delta), cos2D = cosf(2*delta);
            float ww1 = w1*w1, ww2 = w2*w2;

            // Calculate a1
            float a1 = (-G*(2*m1 + m2)*sinf(theta1)
                         - m2*G*sinf(theta1 - 2*theta2)
                         - 2*sinD*m2*(ww2*L2 + ww1*L1*cosD))
                        /(L1*(2*m1 + m2 - m2*cos2D));

            // Calculate a2
            float a2 = (2*sinD*(ww1*L1*totalM
                         + G*totalM*cosf(theta1)
                         + ww2*L2*m2*cosD))
                        /(L2*(2*m1 + m2 - m2*cos2D));

            // Update thetas
            theta1 += w1*step + 0.5f*a1*step2;
            theta2 += w2*step + 0.5f*a2*step2;

            // Update omegas
            w1 += a1*step;
            w2 += a2*step;
        }

        // Calculate position
        Vector2 currentPosition = CalculateDoublePendulumEndPoint(l1, theta1, l2, theta2);
        currentPosition.x += screenWidth/2;
        currentPosition.y += screenHeight/2 - 100;

        // Draw to render texture
        BeginTextureMode(target);
            // Draw a transparent rectangle - smaller alpha = longer trails
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, fateAlpha));

            // Draw trail
            DrawCircleV(previousPosition, trailThick, RED);
            DrawLineEx(previousPosition, currentPosition, trailThick*2, RED);
        EndTextureMode();

        // Update previous position
        previousPosition = currentPosition;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            // Draw trails texture
            DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE);

            // Draw double pendulum
            DrawRectanglePro((Rectangle){ screenWidth/2.0f, screenHeight/2.0f - 100, 10*l1, lineThick },
                (Vector2){0, lineThick*0.5f}, 90 - RAD2DEG*theta1, RAYWHITE);

            Vector2 endpoint1 = CalculatePendulumEndPoint(l1, theta1);
            DrawRectanglePro((Rectangle){ screenWidth/2.0f + endpoint1.x, screenHeight/2.0f - 100 + endpoint1.y, 10*l2, lineThick },
                (Vector2){0, lineThick*0.5f}, 90 - RAD2DEG*theta2, RAYWHITE);

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

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Calculate pendulum end point
static Vector2 CalculatePendulumEndPoint(float l, float theta)
{
    return (Vector2){ 10*l*sinf(theta), 10*l*cosf(theta) };
}

// Calculate double pendulum end point
static Vector2 CalculateDoublePendulumEndPoint(float l1, float theta1, float l2, float theta2)
{
    Vector2 endpoint1 = CalculatePendulumEndPoint(l1, theta1);
    Vector2 endpoint2 = CalculatePendulumEndPoint(l2, theta2);
    return (Vector2){ endpoint1.x + endpoint2.x, endpoint1.y + endpoint2.y };
}
