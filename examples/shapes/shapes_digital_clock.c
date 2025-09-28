/*******************************************************************************************
*
*   raylib [shapes] example - digital clock
*
*   Example complexity rating: [★★★★] 4/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6
*
*   Example contributed by Hamza RAHAL (@hmz-rhl) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Hamza RAHAL (@hmz-rhl) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <math.h>       // Required for: cosf(), sinf()
#include <time.h>       // Required for: time(), localtime()

#define CLOCK_ANALOG    0
#define CLOCK_DIGITAL   1

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Clock hand type
typedef struct {
    int value;          // Time value

    // Visual elements
    float angle;        // Hand angle
    int length;         // Hand length
    int thickness;      // Hand thickness
    Color color;        // Hand color
} ClockHand;

// Clock hands
typedef struct {
    ClockHand second;   // Clock hand for seconds
    ClockHand minute;   // Clock hand for minutes
    ClockHand hour;     // Clock hand for hours
} Clock;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateClock(Clock *clock); // Update clock time
static void DrawClockAnalog(Clock clock, Vector2 position); // Draw analog clock at desired center position
static void DrawClockDigital(Clock clock, Vector2 position); // Draw digital clock at desired position

static void DrawDisplayValue(Vector2 position, int value, Color colorOn, Color colorOff);
static void Draw7SDisplay(Vector2 position, char segments, Color colorOn, Color colorOff);
static void DrawDisplaySegment(Vector2 center, int length, int thick, bool vertical, Color color);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - digital clock");

    int clockMode = CLOCK_DIGITAL;

    // Initialize clock
    // NOTE: Includes visual info for anlaog clock
    Clock clock = {
        .second.angle = 45,
        .second.length = 140,
        .second.thickness = 3,
        .second.color = MAROON,

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
            // Toggle clock mode
            if (clockMode == CLOCK_DIGITAL) clockMode = CLOCK_ANALOG;
            else if (clockMode == CLOCK_ANALOG) clockMode = CLOCK_DIGITAL;
        }

        UpdateClock(&clock); // Update clock required data: value and angle
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw clock in selected mode
            if (clockMode == CLOCK_ANALOG) DrawClockAnalog(clock, (Vector2){ 400, 240 }); 
            else if (clockMode == CLOCK_DIGITAL)
            {
                DrawClockDigital(clock, (Vector2){ 30, 60 });

                // Draw clock using default raylib font
                // Get pointer to formated clock time string
                // WARNING: Pointing to an internal static string that is reused between TextFormat() calls
                const char *clockTime = TextFormat("%02i:%02i:%02i", clock.hour.value, clock.minute.value, clock.second.value);
                DrawText(clockTime, GetScreenWidth()/2 - MeasureText(clockTime, 150)/2, 300, 150, BLACK);
            }

            DrawText(TextFormat("Press [SPACE] to switch clock mode: %s", 
                (clockMode == CLOCK_DIGITAL)? "DIGITAL CLOCK" : "ANALOGUE CLOCK"), 10, 10, 20, DARKGRAY);

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

// Draw analog clock
// Parameter: position, refers to center position
static void DrawClockAnalog(Clock clock, Vector2 position)
{
        // Draw clock base
        DrawCircleV(position, clock.second.length + 40, LIGHTGRAY);
        DrawCircleV(position, 12, GRAY);

        // Draw clock minutes/seconds lines
        for (int i = 0; i < 60; i++)
        {
            DrawLineEx((Vector2){ position.x + (clock.second.length + ((i%5)? 10 : 6))*cosf((6.0f*i - 90.0f)*DEG2RAD), 
                position.y + (clock.second.length + ((i%5)? 10 : 6))*sinf((6.0f*i - 90.0f)*DEG2RAD) }, 
                (Vector2){ position.x + (clock.second.length + 20)*cosf((6.0f*i - 90.0f)*DEG2RAD), 
                position.y + (clock.second.length + 20)*sinf((6.0f*i - 90.0f)*DEG2RAD) }, ((i%5)? 1.0f : 3.0f), DARKGRAY);
            
            // Draw seconds numbers
            //DrawText(TextFormat("%02i", i), centerPosition.x + (clock.second.length + 50)*cosf((6.0f*i - 90.0f)*DEG2RAD) - 10/2, 
            //    centerPosition.y + (clock.second.length + 50)*sinf((6.0f*i - 90.0f)*DEG2RAD) - 10/2, 10, GRAY);
        }

        // Draw hand seconds
        DrawRectanglePro((Rectangle){ position.x, position.y, clock.second.length, clock.second.thickness },
            (Vector2){ 0.0f, clock.second.thickness/2.0f }, clock.second.angle, clock.second.color);

        // Draw hand minutes
        DrawRectanglePro((Rectangle){ position.x, position.y, clock.minute.length, clock.minute.thickness },
            (Vector2){ 0.0f, clock.minute.thickness/2.0f }, clock.minute.angle, clock.minute.color);

        // Draw hand hours
        DrawRectanglePro((Rectangle){ position.x, position.y, clock.hour.length, clock.hour.thickness },
            (Vector2){ 0.0f, clock.hour.thickness/2.0f }, clock.hour.angle, clock.hour.color);
}

// Draw digital clock
// PARAM: position, refers to top-left corner
static void DrawClockDigital(Clock clock, Vector2 position)
{
    // Draw clock using custom 7-segments display (made of shapes)
    DrawDisplayValue((Vector2){ position.x, position.y }, clock.hour.value/10, RED, Fade(LIGHTGRAY, 0.3f));
    DrawDisplayValue((Vector2){ position.x + 120, position.y }, clock.hour.value%10, RED, Fade(LIGHTGRAY, 0.3f));

    DrawCircle(position.x + 240, position.y + 70, 12, (clock.second.value%2)? RED : Fade(LIGHTGRAY, 0.3f));
    DrawCircle(position.x + 240, position.y + 150, 12, (clock.second.value%2)? RED : Fade(LIGHTGRAY, 0.3f));

    DrawDisplayValue((Vector2){ position.x + 260, position.y }, clock.minute.value/10, RED, Fade(LIGHTGRAY, 0.3f));
    DrawDisplayValue((Vector2){ position.x + 380, position.y }, clock.minute.value%10, RED, Fade(LIGHTGRAY, 0.3f));

    DrawCircle(position.x + 500, position.y + 70, 12, (clock.second.value%2)? RED : Fade(LIGHTGRAY, 0.3f));
    DrawCircle(position.x + 500, position.y + 150, 12, (clock.second.value%2)? RED : Fade(LIGHTGRAY, 0.3f));

    DrawDisplayValue((Vector2){ position.x + 520, position.y }, clock.second.value/10, RED, Fade(LIGHTGRAY, 0.3f));
    DrawDisplayValue((Vector2){ position.x + 640, position.y }, clock.second.value%10, RED, Fade(LIGHTGRAY, 0.3f));
}

// Draw 7-segment display with value
static void DrawDisplayValue(Vector2 position, int value, Color colorOn, Color colorOff)
{
    switch (value)
    {
        case 0: Draw7SDisplay(position, 0b00111111, colorOn, colorOff); break;
        case 1: Draw7SDisplay(position, 0b00000110, colorOn, colorOff); break;
        case 2: Draw7SDisplay(position, 0b01011011, colorOn, colorOff); break;
        case 3: Draw7SDisplay(position, 0b01001111, colorOn, colorOff); break;
        case 4: Draw7SDisplay(position, 0b01100110, colorOn, colorOff); break;
        case 5: Draw7SDisplay(position, 0b01101101, colorOn, colorOff); break;
        case 6: Draw7SDisplay(position, 0b01111101, colorOn, colorOff); break;
        case 7: Draw7SDisplay(position, 0b00000111, colorOn, colorOff); break;
        case 8: Draw7SDisplay(position, 0b01111111, colorOn, colorOff); break;
        case 9: Draw7SDisplay(position, 0b01101111, colorOn, colorOff); break;
        default: break;
    }
}

// Draw seven segments display
// Parameter: position, refers to top-left corner of display
// Parameter: segments, defines in binary the segments to be activated
static void Draw7SDisplay(Vector2 position, char segments, Color colorOn, Color colorOff)
{
    int segmentLen = 60;
    int segmentThick = 20;
    float offsetYAdjust = segmentThick*0.3f; // HACK: Adjust gap space between segment limits

    // Segment A
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen/2.0f, position.y + segmentThick }, 
        segmentLen, segmentThick, false, (segments & 0b00000001)? colorOn : colorOff);
    // Segment B
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen + segmentThick/2.0f, position.y + 2*segmentThick + segmentLen/2.0f - offsetYAdjust }, 
        segmentLen, segmentThick, true, (segments & 0b00000010)? colorOn : colorOff);
    // Segment C
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen + segmentThick/2.0f, position.y + 4*segmentThick + segmentLen + segmentLen/2.0f - 3*offsetYAdjust }, 
        segmentLen, segmentThick, true, (segments & 0b00000100)? colorOn : colorOff);
    // Segment D
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen/2.0f, position.y + 5*segmentThick + 2*segmentLen - 4*offsetYAdjust }, 
        segmentLen, segmentThick, false, (segments & 0b00001000)? colorOn : colorOff);
    // Segment E
    DrawDisplaySegment((Vector2){ position.x + segmentThick/2.0f, position.y + 4*segmentThick + segmentLen + segmentLen/2.0f - 3*offsetYAdjust }, 
        segmentLen, segmentThick, true, (segments & 0b00010000)? colorOn : colorOff);
    // Segment F
    DrawDisplaySegment((Vector2){ position.x + segmentThick/2.0f, position.y + 2*segmentThick + segmentLen/2.0f - offsetYAdjust }, 
        segmentLen, segmentThick, true, (segments & 0b00100000)? colorOn : colorOff);
    // Segment G
    DrawDisplaySegment((Vector2){ position.x + segmentThick + segmentLen/2.0f, position.y + 3*segmentThick + segmentLen - 2*offsetYAdjust }, 
        segmentLen, segmentThick, false, (segments & 0b01000000)? colorOn : colorOff);
}

// Draw one 7-segment display segment, horizontal or vertical
static void DrawDisplaySegment(Vector2 center, int length, int thick, bool vertical, Color color)
{
    if (!vertical)
    {
        // Horizontal segment points
        //   3___________________________5
        //  /                             \
        // /1             x               6\
        // \                               /
        //  \2___________________________4/
        Vector2 segmentPointsH[6] = {
            (Vector2){ center.x - length/2.0f - thick/2.0f,  center.y },  // Point 1
            (Vector2){ center.x - length/2.0f,  center.y + thick/2.0f },  // Point 2
            (Vector2){ center.x - length/2.0f, center.y - thick/2.0f },   // Point 3
            (Vector2){ center.x + length/2.0f,  center.y + thick/2.0f },  // Point 4
            (Vector2){ center.x + length/2.0f,  center.y - thick/2.0f },  // Point 5
            (Vector2){ center.x + length/2.0f + thick/2.0f,  center.y },  // Point 6
        };

        DrawTriangleStrip(segmentPointsH, 6, color);
    }
    else
    {
        // Vertical segment points
        Vector2 segmentPointsV[6] = {
            (Vector2){ center.x,  center.y - length/2.0f - thick/2.0f },  // Point 1
            (Vector2){ center.x - thick/2.0f,  center.y - length/2.0f },  // Point 2
            (Vector2){ center.x + thick/2.0f, center.y - length/2.0f },   // Point 3
            (Vector2){ center.x - thick/2.0f,  center.y + length/2.0f },  // Point 4
            (Vector2){ center.x + thick/2.0f,  center.y + length/2.0f },  // Point 5
            (Vector2){ center.x,  center.y + length/2 + thick/2.0f },     // Point 6
        };

        DrawTriangleStrip(segmentPointsV, 6, color);
    }
}
