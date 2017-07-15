/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level02 Screen Functions Definitions (Init, Update, Draw, Unload)
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

#include <math.h>

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Level02 screen global variables
static int framesCounter;
static int finishScreen;

static Vector2 bouncingBallPos;
static float bouncingBallRadius = 40;
static Vector2 bouncingBallSpeed;

static Vector2 holeCirclePos;
static float holeCircleRadius = 50;

static bool ballOnHole = false;

static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level02 Screen Functions Definition
//----------------------------------------------------------------------------------
float Vector2Distance(Vector2 v1, Vector2 v2);

// Level02 Screen Initialization logic
void InitLevel02Screen(void)
{
    // TODO: Initialize Level02 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    bouncingBallPos = (Vector2){ 120, 80 };
    bouncingBallSpeed = (Vector2){ 6, 8 };
    holeCirclePos = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
}

// Level02 Screen Update logic
void UpdateLevel02Screen(void)
{
    // Update Level02 screen
    framesCounter++;
    
    if (!ballOnHole)
    {
        bouncingBallPos.x += bouncingBallSpeed.x;
        bouncingBallPos.y += bouncingBallSpeed.y;

        if (((bouncingBallPos.x - bouncingBallRadius) <= 0) || ((bouncingBallPos.x + bouncingBallRadius) >= GetScreenWidth())) bouncingBallSpeed.x *= -1;
        if (((bouncingBallPos.y - bouncingBallRadius) <= 0) || ((bouncingBallPos.y + bouncingBallRadius) >= GetScreenHeight())) bouncingBallSpeed.y *= -1;
        
        Vector2 mousePos = GetMousePosition();
        
        if (CheckCollisionPointCircle(mousePos, bouncingBallPos, 120))
        {
            bouncingBallPos.x = GetRandomValue(80, 1200);
            bouncingBallPos.y = GetRandomValue(80, 650);
        }
        
        if (CheckCollisionPointCircle(mousePos, holeCirclePos, 120))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                holeCirclePos = mousePos;
                
                if ((holeCirclePos.x - holeCircleRadius) <= 0) holeCirclePos.x = holeCircleRadius;
                else if ((holeCirclePos.x + holeCircleRadius) >= GetScreenWidth()) holeCirclePos.x = GetScreenWidth() - holeCircleRadius;
                
                if ((holeCirclePos.y - holeCircleRadius) <= 0) holeCirclePos.y = holeCircleRadius;
                else if ((holeCirclePos.y + holeCircleRadius) >= GetScreenHeight()) holeCirclePos.y = GetScreenHeight() - holeCircleRadius;
            }
        }
        
        if (Vector2Distance(bouncingBallPos, holeCirclePos) < 20)
        {
            ballOnHole = true;
            PlaySound(levelWin);
        }
    }
    
    if (ballOnHole && !levelFinished)
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

// Level02 Screen Draw logic
void DrawLevel02Screen(void)
{
    // Draw Level02 screen

    DrawCircleV(holeCirclePos, holeCircleRadius, LIGHTGRAY);
    DrawCircleV(bouncingBallPos, bouncingBallRadius, DARKGRAY);

    DrawCircleLines(bouncingBallPos.x, bouncingBallPos.y, 120, Fade(LIGHTGRAY, 0.8f));
    
    
    
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 02", GetScreenWidth()/2 - MeasureText("LEVEL 02", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 02", GetScreenWidth()/2 - MeasureText("LEVEL 02", 30)/2, 20, 30, LIGHTGRAY);
}

// Level02 Screen Unload logic
void UnloadLevel02Screen(void)
{
    // TODO: Unload Level02 screen variables here!
}

// Level02 Screen should finish?
int FinishLevel02Screen(void)
{
    return finishScreen;
}