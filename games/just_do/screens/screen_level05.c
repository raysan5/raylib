/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level05 Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"

#define NUM_CIRCLES 10

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Level05 screen global variables
static int framesCounter;
static int finishScreen;

static Vector2 circleCenter;
static float circleRadius[NUM_CIRCLES];
static bool circleLocked[NUM_CIRCLES];
static Color circleColor[NUM_CIRCLES];

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level05 Screen Functions Definition
//----------------------------------------------------------------------------------
static bool CheckColor(Color col1, Color col2);

// Level05 Screen Initialization logic
void InitLevel05Screen(void)
{
    // Initialize Level05 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    circleCenter = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
    
    for (int i = 0; i < NUM_CIRCLES; i++)
    {
        circleRadius[i] = 760/NUM_CIRCLES*(NUM_CIRCLES - i);
        circleLocked[i] = false;
    }
    
    // That's a dirty hack to give sonme coherence to this puzzle...
    circleColor[9] = RL_GRAY;
    circleColor[8] = RL_RAYWHITE;
    circleColor[7] = RL_RAYWHITE;
    circleColor[6] = RL_GRAY;
    circleColor[5] = RL_RAYWHITE;
    circleColor[4] = RL_GRAY;
    circleColor[3] = RL_GRAY;
    circleColor[2] = RL_GRAY;
    circleColor[1] = RL_RAYWHITE;
    circleColor[0] = RL_GRAY;
}

// Level05 Screen Update logic
void UpdateLevel05Screen(void)
{
    // Update Level05 screen variables here!
    framesCounter++;

    if (!done)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            for (int i = NUM_CIRCLES - 1; i >= 0; i--)
            {
                if (CheckCollisionPointCircle(GetMousePosition(), circleCenter, circleRadius[i]))
                {
                    if (i == 0)
                    {
                        if (CheckColor(circleColor[8], RL_GRAY)) circleColor[8] = RL_RAYWHITE;
                        else circleColor[8] = RL_GRAY;
                    }
                    else if (i == 2)
                    {
                        if (CheckColor(circleColor[5], RL_GRAY)) circleColor[5] = RL_RAYWHITE;
                        else circleColor[5] = RL_GRAY;
                    }
                    else if (i == 3)
                    {
                        if (CheckColor(circleColor[6], RL_GRAY)) circleColor[6] = RL_RAYWHITE;
                        else circleColor[6] = RL_GRAY;
                    }
                    else
                    {
                        if (CheckColor(circleColor[i], RL_GRAY)) circleColor[i] = RL_RAYWHITE;
                        else circleColor[i] = RL_GRAY;
                    }
                    return;
                }
            }
        }
    
        // Check all cicles done
        for (int i = 0; i < NUM_CIRCLES; i++)
        {
            done = true;
            
            if (CheckColor(circleColor[i], RL_RAYWHITE))
            {
                done = false;
                return;
            }
            
            //if (done) PlaySound(levelWin);
        }
    }

    if (done && !levelFinished)
    {
        levelTimeSec = framesCounter/60;
        levelFinished = true;
        framesCounter = 0;
    }
    
    if (levelFinished)
    {
        framesCounter++;
        
        if ((framesCounter > 90) && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) finishScreen = true;
    }
}

// Level05 Screen Draw logic
void DrawLevel05Screen(void)
{
    // Draw Level05 screen
    for (int i = 0; i < NUM_CIRCLES; i++)
    {
        DrawPoly(circleCenter, 64, circleRadius[i], 0.0f, circleColor[i]);
    }
    
    
    
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(RL_LIGHTGRAY, 0.6f));
        DrawText("LEVEL 05", GetScreenWidth()/2 - MeasureText("LEVEL 05", 30)/2, 20, 30, RL_GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, RL_GRAY);
    }
    else DrawText("LEVEL 05", GetScreenWidth()/2 - MeasureText("LEVEL 05", 30)/2, 20, 30, RL_LIGHTGRAY);
}

// Level05 Screen Unload logic
void UnloadLevel05Screen(void)
{
    // TODO: Unload Level05 screen variables here!
}

// Level05 Screen should finish?
int FinishLevel05Screen(void)
{
    return finishScreen;
}

static bool CheckColor(Color col1, Color col2)
{
    return ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a));
}