/*******************************************************************************************
*
*   raylib [shapes] example - digital clock
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Hamza RAHAL (@hmz-rhl) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Hamza RAHAL (@hmz-rhl)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>       // Required for: cosf(), sinf()
#include <time.h>       // Required for: time(), localtime()

#define DIGIT_SIZE 30

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    MODE_NORMAL = 0,
    MODE_HANDS_FREE,
} ClockMode;

typedef struct {
    int value;
    Vector2 origin;
    float angle;
    int length;
    int thickness;
    Color color;
} ClockHand;

typedef struct {
    ClockMode mode;
    ClockHand second;
    ClockHand minute;
    ClockHand hour;
} Clock;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateClock(Clock *clock); // Update clock time
static void DrawClock(Clock clock, Vector2 centerPos); // Draw clock at desired position

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - digital clock");

    // Initialize clock
    Clock myClock = {
        .mode = MODE_NORMAL,

        .second.angle = 45,
        .second.length = 140,
        .second.thickness = 3,
        .second.color = BEIGE,

        .minute.angle = 10,
        .minute.length = 130,
        .minute.thickness = 7,
        .minute.color = DARKGRAY,

        .hour.angle = 0,
        .hour.length = 100,
        .hour.thickness = 7,
        .hour.color = BLACK,
    };

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_SPACE))
        {
            if (myClock.mode == MODE_HANDS_FREE) myClock.mode = MODE_NORMAL;
            else if (myClock.mode == MODE_NORMAL) myClock.mode = MODE_HANDS_FREE;
        }

        UpdateClock(&myClock);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawCircle(400, 225, 5, BLACK); // Clock center dot

            DrawClock(myClock, (Vector2){ 400, 225 }); // Clock in selected mode

            DrawText("Press [SPACE] to switch clock mode", 10, 10, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Update clock time
static void UpdateClock(Clock *clock)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Updating time data
    clock->second.value = timeinfo->tm_sec;
    clock->minute.value = timeinfo->tm_min;
    clock->hour.value = timeinfo->tm_hour;

    clock->hour.angle = (timeinfo->tm_hour%12)*180.0/6.0f;
    clock->hour.angle += (timeinfo->tm_min%60)*30/60.0f;
    clock->hour.angle -= 90;

    clock->minute.angle = (timeinfo->tm_min%60)*6.0f;
    clock->minute.angle += (timeinfo->tm_sec%60)*6/60.0f;
    clock->minute.angle -= 90;

    clock->second.angle = (timeinfo->tm_sec%60)*6.0f;
    clock->second.angle -= 90;
}

// Draw clock
static void DrawClock(Clock clock, Vector2 centerPosition)
{
    if (clock.mode == MODE_HANDS_FREE)
    {
        DrawCircleLinesV(centerPosition, clock.minute.length, LIGHTGRAY);

        DrawText(TextFormat("%i", clock.second.value), centerPosition.x + (clock.second.length - 10)*cosf(clock.second.angle*(float)(PI/180)) - DIGIT_SIZE/2, centerPosition.y + clock.second.length*sinf(clock.second.angle*(float)(PI/180)) - DIGIT_SIZE/2, DIGIT_SIZE, GRAY);

        DrawText(TextFormat("%i", clock.minute.value), centerPosition.x + clock.minute.length*cosf(clock.minute.angle*(float)(PI/180)) - DIGIT_SIZE/2, centerPosition.y + clock.minute.length*sinf(clock.minute.angle*(float)(PI/180)) - DIGIT_SIZE/2, DIGIT_SIZE, RED);

        DrawText(TextFormat("%i", clock.hour.value), centerPosition.x + clock.hour.length*cosf(clock.hour.angle*(float)(PI/180)) - DIGIT_SIZE/2, centerPosition.y + clock.hour.length*sinf(clock.hour.angle*(float)(PI/180)) - DIGIT_SIZE/2, DIGIT_SIZE, GOLD);
    }
    else if (clock.mode == MODE_NORMAL)
    {
        // Draw hand seconds
        DrawRectanglePro((Rectangle){ centerPosition.x, centerPosition.y, clock.second.length, clock.second.thickness },
            (Vector2){ 0.0f, clock.second.thickness/2.0f }, clock.second.angle, clock.second.color);

        // Draw hand minutes
        DrawRectanglePro((Rectangle){ centerPosition.x, centerPosition.y, clock.minute.length, clock.minute.thickness },
            (Vector2){ 0.0f, clock.minute.thickness/2.0f }, clock.minute.angle, clock.minute.color);

        // Draw hand hours
        DrawRectanglePro((Rectangle){ centerPosition.x, centerPosition.y, clock.hour.length, clock.hour.thickness },
            (Vector2){ 0.0f, clock.hour.thickness/2.0f }, clock.hour.angle, clock.hour.color);
    }
}
