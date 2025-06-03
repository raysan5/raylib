/*******************************************************************************************
*
*   raylib [shapes] example - fancy clock using basic shapes
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
*   Copyright (c) 2025-2025 Hamza RAHAL (@hmz-rhl)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h> // needed for cos & sin functions
#include <time.h> // needed to get machine time

#define DIGIT_SIZE 30

typedef enum
{
    NORMAL_MODE = 0,
    HANDS_FREE_MODE,
} ClockMode;

typedef struct
{
    int value;
    Vector2 origin;
    float angle;
    int length;
    int thickness;
    Color colour;
} Hand;

typedef struct
{
    Hand second;
    Hand minute;
    Hand hour;
    ClockMode cm;
} Clock;


void UpdateClock(Clock *clock)
{
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // updating datas    
    clock->second.value = timeinfo->tm_sec;
    clock->minute.value = timeinfo->tm_min;
    clock->hour.value = timeinfo->tm_hour;

    clock->hour.angle = (timeinfo->tm_hour % 12)*180.0/6.0f;
    clock->hour.angle += (timeinfo->tm_min % 60)*30/60.0f;
    clock->hour.angle -= 90 ;

    clock->minute.angle = (timeinfo->tm_min % 60)*6.0f;
    clock->minute.angle += (timeinfo->tm_sec % 60)*6/60.0f;
    clock->minute.angle -= 90 ;
   
    clock->second.angle = (timeinfo->tm_sec % 60)*6.0f;
    clock->second.angle -= 90 ;

}

void drawClock(Clock clock)
{
    if (clock.cm == HANDS_FREE_MODE)
    {
        DrawText(TextFormat("%i", clock.second.value), clock.second.origin.x + (clock.second.length - 10)*cos(clock.second.angle*(float)(M_PI/180)) - DIGIT_SIZE/2, clock.second.origin.y + clock.second.length*sin(clock.second.angle*(float)(M_PI/180)) - DIGIT_SIZE/2, DIGIT_SIZE, GRAY);

        DrawText(TextFormat("%i", clock.minute.value), clock.minute.origin.x + clock.minute.length*cos(clock.minute.angle*(float)(M_PI/180)) - DIGIT_SIZE/2, clock.minute.origin.y + clock.minute.length*sin(clock.minute.angle*(float)(M_PI/180)) - DIGIT_SIZE/2, DIGIT_SIZE, RED);

        DrawText(TextFormat("%i", clock.hour.value), clock.hour.origin.x + clock.hour.length*cos(clock.hour.angle*(float)(M_PI/180)) - DIGIT_SIZE/2, clock.hour.origin.y + clock.hour.length*sin(clock.hour.angle*(float)(M_PI/180)) - DIGIT_SIZE/2, DIGIT_SIZE, GOLD);
    }
    else
    {
        DrawRectanglePro(
            (Rectangle){   clock.second.origin.x,
                clock.second.origin.y,
                clock.second.length,
                clock.second.thickness,
            }
            , (Vector2){0, clock.second.thickness/2},
            clock.second.angle,
            clock.second.colour
        );

        DrawRectanglePro(
            (Rectangle){   clock.minute.origin.x,
                clock.minute.origin.y,
                clock.minute.length,
                clock.minute.thickness,
            }
            , (Vector2){0, clock.minute.thickness/2},
            clock.minute.angle,
            clock.minute.colour
        );

        DrawRectanglePro(
            (Rectangle){   clock.hour.origin.x,
                clock.hour.origin.y,
                clock.hour.length,
                clock.hour.thickness,
            }
            , (Vector2){0, clock.hour.thickness/2},
            clock.hour.angle,
            clock.hour.colour
        );
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    Clock myClock = {
        .cm = NORMAL_MODE,

        .second.origin = (Vector2){400, 225},
        .second.angle = 45,
        .second.length = 140,        
        .second.thickness = 3,
        .second.colour = BEIGE,

        .minute.origin = (Vector2){400, 225},
        .minute.angle = 10,
        .minute.length = 130,
        .minute.thickness = 7,
        .minute.colour = DARKGRAY,

        .hour.origin = (Vector2){400, 225},
        .hour.angle = 0,
        .hour.length = 100,
        .hour.thickness = 7,
        .hour.colour = BLACK,
    };

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - digital clock");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        if (IsKeyPressed(KEY_SPACE))
        {
            myClock.cm = (myClock.cm == HANDS_FREE_MODE) ? NORMAL_MODE : HANDS_FREE_MODE;
        }

        UpdateClock(&myClock);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
        ClearBackground(RAYWHITE);

            DrawCircle(400, 225, 5, BLACK); // center dot
            drawClock(myClock);

            DrawText("press [SPACE] to switch clock mode", 350, 400, 10, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}