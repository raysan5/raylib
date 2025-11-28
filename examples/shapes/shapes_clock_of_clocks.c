/*******************************************************************************************
*
*   raylib [shapes] example - clock of clocks
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.6-dev
*
*   Example contributed by JP Mortiboys (@themushroompirates) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 JP Mortiboys (@themushroompirates)
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"    // Required for: Lerp()
#include <time.h>       // Required for: time(), localtime()

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
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - clock of clocks");

    const Color bgColor = ColorLerp(DARKBLUE, BLACK, 0.75f);
    const Color handsColor = ColorLerp(YELLOW, RAYWHITE, .25f);

    const float clockFaceSize = 24;
    const float clockFaceSpacing = 8.0f;
    const float sectionSpacing = 16.0f;

    const Vector2 TL = (Vector2){   0.0f,  90.0f }; // Top-left corner
    const Vector2 TR = (Vector2){  90.0f, 180.0f }; // Top-right corner
    const Vector2 BR = (Vector2){ 180.0f, 270.0f }; // Bottom-right corner
    const Vector2 BL = (Vector2){   0.0f, 270.0f }; // Bottom-left corner
    const Vector2 HH = (Vector2){   0.0f, 180.0f }; // Horizontal line
    const Vector2 VV = (Vector2){  90.0f, 270.0f }; // Vertical line
    const Vector2 ZZ = (Vector2){ 135.0f, 135.0f }; // Not relevant

    const Vector2 digitAngles[10][24] = {
        /* 0 */ { TL,HH,HH,TR, /* */ VV,TL,TR,VV,/* */ VV,VV,VV,VV,/* */ VV,VV,VV,VV,/* */ VV,BL,BR,VV,/* */ BL,HH,HH,BR },
        /* 1 */ { TL,HH,TR,ZZ, /* */ BL,TR,VV,ZZ,/* */ ZZ,VV,VV,ZZ,/* */ ZZ,VV,VV,ZZ,/* */ TL,BR,BL,TR,/* */ BL,HH,HH,BR },
        /* 2 */ { TL,HH,HH,TR, /* */ BL,HH,TR,VV,/* */ TL,HH,BR,VV,/* */ VV,TL,HH,BR,/* */ VV,BL,HH,TR,/* */ BL,HH,HH,BR },
        /* 3 */ { TL,HH,HH,TR, /* */ BL,HH,TR,VV,/* */ TL,HH,BR,VV,/* */ BL,HH,TR,VV,/* */ TL,HH,BR,VV,/* */ BL,HH,HH,BR },
        /* 4 */ { TL,TR,TL,TR, /* */ VV,VV,VV,VV,/* */ VV,BL,BR,VV,/* */ BL,HH,TR,VV,/* */ ZZ,ZZ,VV,VV,/* */ ZZ,ZZ,BL,BR },
        /* 5 */ { TL,HH,HH,TR, /* */ VV,TL,HH,BR,/* */ VV,BL,HH,TR,/* */ BL,HH,TR,VV,/* */ TL,HH,BR,VV,/* */ BL,HH,HH,BR },
        /* 6 */ { TL,HH,HH,TR, /* */ VV,TL,HH,BR,/* */ VV,BL,HH,TR,/* */ VV,TL,TR,VV,/* */ VV,BL,BR,VV,/* */ BL,HH,HH,BR },
        /* 7 */ { TL,HH,HH,TR, /* */ BL,HH,TR,VV,/* */ ZZ,ZZ,VV,VV,/* */ ZZ,ZZ,VV,VV,/* */ ZZ,ZZ,VV,VV,/* */ ZZ,ZZ,BL,BR },
        /* 8 */ { TL,HH,HH,TR, /* */ VV,TL,TR,VV,/* */ VV,BL,BR,VV,/* */ VV,TL,TR,VV,/* */ VV,BL,BR,VV,/* */ BL,HH,HH,BR },
        /* 9 */ { TL,HH,HH,TR, /* */ VV,TL,TR,VV,/* */ VV,BL,BR,VV,/* */ BL,HH,TR,VV,/* */ TL,HH,BR,VV,/* */ BL,HH,HH,BR },
    };

    // Time for the hands to move to the new position (in seconds); this must be <1s
    const float handsMoveDuration = 0.5f;

    int prevSeconds = -1;
    Vector2 currentAngles[6][24] = { 0 };
    Vector2 srcAngles[6][24] = { 0 };
    Vector2 dstAngles[6][24] = { 0 };

    float handsMoveTimer = 0.0f;
    int hourMode = 24;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Get the current time
        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        if (timeinfo->tm_sec != prevSeconds)
        {
            // The time has changed, so we need to move the hands to the new positions
            prevSeconds = timeinfo->tm_sec;

            // Format the current time so we can access the individual digits
            const char *clockDigits = TextFormat("%02d%02d%02d", timeinfo->tm_hour%hourMode, timeinfo->tm_min, timeinfo->tm_sec);

            // Fetch where we want all the hands to be
            for (int digit = 0; digit < 6; digit++)
            {
                for (int cell = 0; cell < 24; cell++)
                {
                    srcAngles[digit][cell] = currentAngles[digit][cell];
                    dstAngles[digit][cell] = digitAngles[clockDigits[digit] - '0'][cell];

                    // Quick exception for 12h mode
                    if ((digit == 0) && (hourMode == 12) && (clockDigits[0] == '0')) dstAngles[digit][cell] = ZZ;
                    if (srcAngles[digit][cell].x > dstAngles[digit][cell].x) srcAngles[digit][cell].x -= 360.0f;
                    if (srcAngles[digit][cell].y > dstAngles[digit][cell].y) srcAngles[digit][cell].y -= 360.0f;
                }
            }

            // Reset the timer
            handsMoveTimer = -GetFrameTime();
        }

        // Now let's animate all the hands if we need to
        if (handsMoveTimer < handsMoveDuration)
        {
            // Increase the timer but don't go above the maximum
            handsMoveTimer = Clamp(handsMoveTimer + GetFrameTime(), 0, handsMoveDuration);

            // Calculate the%completion of the animation
            float t = handsMoveTimer/handsMoveDuration;

            // A little cheeky smoothstep
            t = t*t*(3.0f - 2.0f*t);

            for (int digit = 0; digit < 6; digit++)
            {
                for (int cell = 0; cell < 24; cell++)
                {
                    currentAngles[digit][cell].x = Lerp(srcAngles[digit][cell].x, dstAngles[digit][cell].x, t);
                    currentAngles[digit][cell].y = Lerp(srcAngles[digit][cell].y, dstAngles[digit][cell].y, t);
                }
            }
        }

        // Handle input
        if (IsKeyPressed(KEY_SPACE)) hourMode = 36 - hourMode; // Toggle between 12 and 24 hour mode with space
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(bgColor);

            DrawText(TextFormat("%d-h mode, space to change", hourMode), 10, 30, 20, RAYWHITE);

            float xOffset = 4.0f;

            for (int digit = 0; digit < 6; digit++)
            {
                for (int row = 0; row < 6; row++)
                {
                    for (int col = 0; col < 4; col++)
                    {
                        Vector2 centre = (Vector2){
                            xOffset + col*(clockFaceSize+clockFaceSpacing) + clockFaceSize*0.5f,
                            100 + row*(clockFaceSize+clockFaceSpacing) + clockFaceSize*0.5f
                        };

                        DrawRing(centre, clockFaceSize*0.5f - 2.0f, clockFaceSize*0.5f, 0, 360, 24, DARKGRAY);

                        // Big hand
                        DrawRectanglePro(
                            (Rectangle){centre.x, centre.y, clockFaceSize*0.5f+4.0f, 4.0f},
                            (Vector2){ 2.0f, 2.0f },
                            currentAngles[digit][row*4+col].x,
                            handsColor
                        );

                        // Little hand
                        DrawRectanglePro(
                            (Rectangle){centre.x, centre.y, clockFaceSize*0.5f+2.0f, 4.0f},
                            (Vector2){ 2.0f, 2.0f },
                            currentAngles[digit][row*4+col].y,
                            handsColor
                        );
                    }
                }

                xOffset += (clockFaceSize+clockFaceSpacing)*4;
                if (digit%2 == 1)
                {
                    DrawRing((Vector2){xOffset + 4.0f, 160.0f}, 6.0f, 8.0f, 0.0f, 360.0f, 24, handsColor);
                    DrawRing((Vector2){xOffset + 4.0f, 225.0f}, 6.0f, 8.0f, 0.0f, 360.0f, 24, handsColor);
                    xOffset += sectionSpacing;
                }
            }

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}