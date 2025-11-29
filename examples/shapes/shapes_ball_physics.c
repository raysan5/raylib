/*******************************************************************************************
*
*   raylib [shapes] example - physics bouncing balls
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5
*
*   Example contributed by David Buzatto (@davidbuzatto) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 David Buzatto (@davidbuzatto)
*
********************************************************************************************/

#include <stdlib.h>
#include <math.h>
#include "raylib.h"

#define MAX_BALLS 5000 // Maximum quantity of balls

typedef struct Ball {
    Vector2 pos;       // Position
    Vector2 vel;       // Velocity
    Vector2 ppos;      // Previous position
    float radius;
    float friction;   
    float elasticity;
    Color color;
    bool grabbed;
} Ball;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - physics bouncing balls");

    Ball balls[MAX_BALLS] = {{
        .pos = {GetScreenWidth()/2, GetScreenHeight()/2},
        .vel = {200, 200},
        .ppos = {0},
        .radius = 40,
        .friction = 0.99,
        .elasticity = 0.9,
        .color = BLUE,
        .grabbed = false
    }};
    
    int ballQuantity = 1;
    Ball *grabbedBall = NULL;   // A pointer to the current ball that is grabbed
    Vector2 pressOffset = {0};  // Mouse press offset relative to the ball that grabbedd

    float gravity = 100;        // World gravity

    SetTargetFPS(60);           // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        float delta = GetFrameTime();
        Vector2 mousePos = GetMousePosition();

        // Checks if a ball was grabbed
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            for (int i = ballQuantity - 1; i >= 0; i--) {

                Ball *ball = &balls[i];
                pressOffset.x = mousePos.x - ball->pos.x;
                pressOffset.y = mousePos.y - ball->pos.y;

                // If the distance between the ball position and the mouse press position
                // is less or equal the ball radius, the event occured inside the ball
                if (hypot(pressOffset.x, pressOffset.y) <= ball->radius)
                {
                    ball->grabbed = true;
                    grabbedBall = ball;
                    break;
                }

            }
        }

        // Releases any ball the was grabbed
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            if (grabbedBall != NULL)
            {
                grabbedBall->grabbed = false;
                grabbedBall = NULL;
            }
        }

        // Creates a new ball
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))) {
            if (ballQuantity < MAX_BALLS) {
                balls[ballQuantity++] = (Ball) {
                    .pos = mousePos,
                    .vel = {GetRandomValue(-300, 300), GetRandomValue(-300, 300)},
                    .ppos = {0},
                    .radius = 20 + GetRandomValue(0, 30),
                    .friction = 0.99,
                    .elasticity = 0.9,
                    .color = {GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255},
                    .grabbed = false
                };
            }
        }

        // Shake balls
        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            for (int i = 0; i < ballQuantity; i++) {
                Ball *ball = &balls[i];
                if (!ball->grabbed) {
                    ball->vel = (Vector2) {GetRandomValue(-2000, 2000), GetRandomValue(-2000, 2000)};
                }
            }
        }

        // Changes gravity
        gravity += GetMouseWheelMove() * 5;

        // Updates each ball state
        for (int i = 0; i < ballQuantity; i++) {

            Ball *ball = &balls[i];

            // The ball is not grabbed
            if (!ball->grabbed) 
            {
                // Ball repositioning using the velocity
                ball->pos.x += ball->vel.x * delta;
                ball->pos.y += ball->vel.y * delta;

                // Does the ball hit the screen right boundary?
                if (ball->pos.x + ball->radius >= screenWidth) 
                {
                    ball->pos.x = screenWidth - ball->radius; // Ball repositioning
                    ball->vel.x = -ball->vel.x * ball->elasticity;  // Elasticity makes the ball lose 10% of its velocity on hit
                } 
                // Does the ball hit the screen left boundary?
                else if (ball->pos.x - ball->radius <= 0)
                { 
                    ball->pos.x = ball->radius;
                    ball->vel.x = -ball->vel.x * ball->elasticity;
                }

                // The same for y axis
                if (ball->pos.y + ball->radius >= screenHeight) 
                {
                    ball->pos.y = screenHeight - ball->radius;
                    ball->vel.y = -ball->vel.y * ball->elasticity;
                } 
                else if (ball->pos.y - ball->radius <= 0) 
                { 
                    ball->pos.y = ball->radius;
                    ball->vel.y = -ball->vel.y * ball->elasticity;
                }

                // Friction makes the ball lose 1% of its velocity each frame
                ball->vel.x = ball->vel.x * ball->friction;
                // Gravity affects only the y axis
                ball->vel.y = ball->vel.y * ball->friction + gravity;

            }
            else
            {
                // Ball repositioning using the mouse position
                ball->pos.x = mousePos.x - pressOffset.x;
                ball->pos.y = mousePos.y - pressOffset.y;
                // While the ball is grabbed, recalculates its velocity
                ball->vel.x = (ball->pos.x - ball->ppos.x) / delta;
                ball->vel.y = (ball->pos.y - ball->ppos.y) / delta;
                ball->ppos = ball->pos;
            }
        }
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            for (int i = 0; i < ballQuantity; i++)
            {
                Ball *ball = &balls[i];
                DrawCircleV(ball->pos, ball->radius, ball->color);
                DrawCircleLinesV(ball->pos, ball->radius, BLACK);
            }

            DrawText("grab a ball by pressing with the mouse and throw it by releasing", 10, 10, 20, DARKGRAY);
            DrawText("right click to create new balls (keep left control pressed to create a lot)", 10, 30, 20, DARKGRAY);
            DrawText("use mouse wheel to change gravity", 10, 50, 20, DARKGRAY);
            DrawText("middle click to shake", 10, 70, 20, DARKGRAY);
            DrawText(TextFormat("ball quantity: %d", ballQuantity), 10, GetScreenHeight() - 55, 20, BLACK);
            DrawText(TextFormat("gravity: %.2f", gravity), 10, GetScreenHeight() - 35, 20, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}