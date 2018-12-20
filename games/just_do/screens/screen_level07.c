/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level07 Screen Functions Definitions (Init, Update, Draw, Unload)
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

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Level07 screen global variables
static int framesCounter;
static int finishScreen;

static Vector2 leftCirclePos, middleCirclePos, rightCirclePos;
static Vector2 leftBtnPos, middleBtnPos, rightBtnPos;
static float circleRadius = 100;
static float btnRadius = 80;

static bool leftCircleActive, middleCircleActive, rightCircleActive;
static Color leftCircleColor, middleCircleColor, rightCircleColor;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level07 Screen Functions Definition
//----------------------------------------------------------------------------------
static bool CheckColor(Color col1, Color col2);

// Level07 Screen Initialization logic
void InitLevel07Screen(void)
{
    // Initialize Level07 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    leftCirclePos = (Vector2){ GetScreenWidth()/2 - 340, GetScreenHeight()/2 - 100 };
    middleCirclePos = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 - 100 };
    rightCirclePos = (Vector2){ GetScreenWidth()/2 + 340, GetScreenHeight()/2 - 100 };

    leftBtnPos = (Vector2){ GetScreenWidth()/2 - 340, GetScreenHeight()/2 + 120 };
    middleBtnPos = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 + 120 };
    rightBtnPos = (Vector2){ GetScreenWidth()/2 + 340, GetScreenHeight()/2 + 120 };
    
    leftCircleActive = false;
    middleCircleActive = true;
    rightCircleActive = false;
    
    leftCircleColor = RL_GRAY;
    middleCircleColor = RL_GRAY;
    rightCircleColor = RL_GRAY;
}

// Level07 Screen Update logic
void UpdateLevel07Screen(void)
{
    // Update Level07 screen variables here!
    framesCounter++;
    
    if (!done)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointCircle(GetMousePosition(), leftBtnPos, btnRadius)) leftCircleActive = !leftCircleActive;
            else if (CheckCollisionPointCircle(GetMousePosition(), middleBtnPos, btnRadius)) middleCircleActive = !middleCircleActive;
            else if (CheckCollisionPointCircle(GetMousePosition(), rightBtnPos, btnRadius)) rightCircleActive = !rightCircleActive;
            
            if (rightCircleActive && CheckCollisionPointCircle(GetMousePosition(), leftCirclePos, circleRadius))
            {
                if (CheckColor(leftCircleColor, RL_GRAY)) leftCircleColor = RL_LIGHTGRAY;
                else leftCircleColor = RL_GRAY;
            }
            
            if (middleCircleActive && CheckCollisionPointCircle(GetMousePosition(), middleCirclePos, circleRadius))
            {
                if (CheckColor(middleCircleColor, RL_GRAY)) middleCircleColor = RL_LIGHTGRAY;
                else middleCircleColor = RL_GRAY;
            }
            
            if (rightCircleActive && leftCircleActive && CheckCollisionPointCircle(GetMousePosition(), rightCirclePos, circleRadius))
            {
                if (CheckColor(rightCircleColor, RL_GRAY)) rightCircleColor = RL_LIGHTGRAY;
                else rightCircleColor = RL_GRAY;
            }
        }
    
        // Check all cicles done
        if (CheckColor(leftCircleColor, RL_LIGHTGRAY) && CheckColor(middleCircleColor, RL_LIGHTGRAY) && CheckColor(rightCircleColor, RL_LIGHTGRAY) &&
            !leftCircleActive && !middleCircleActive && !rightCircleActive)
        {
            done = true;
            PlaySound(levelWin);
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

// Level07 Screen Draw logic
void DrawLevel07Screen(void)
{
    // Draw Level07 screen here!
    DrawCircleV(leftCirclePos, circleRadius, leftCircleColor);
    DrawCircleV(middleCirclePos, circleRadius, middleCircleColor);
    DrawCircleV(rightCirclePos, circleRadius, rightCircleColor);
    
    if (leftCircleActive) DrawCircleV(leftBtnPos, btnRadius, RL_GRAY);
    else DrawCircleV(leftBtnPos, btnRadius, RL_LIGHTGRAY);
    
    if (middleCircleActive) DrawCircleV(middleBtnPos, btnRadius, RL_GRAY);
    else DrawCircleV(middleBtnPos, btnRadius, RL_LIGHTGRAY);
    
    if (rightCircleActive) DrawCircleV(rightBtnPos, btnRadius, RL_GRAY);
    else DrawCircleV(rightBtnPos, btnRadius, RL_LIGHTGRAY);
    
    
    
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(RL_LIGHTGRAY, 0.6f));
        DrawText("LEVEL 07", GetScreenWidth()/2 - MeasureText("LEVEL 07", 30)/2, 20, 30, RL_GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, RL_GRAY);
    }
    else DrawText("LEVEL 07", GetScreenWidth()/2 - MeasureText("LEVEL 07", 30)/2, 20, 30, RL_LIGHTGRAY);
}

// Level07 Screen Unload logic
void UnloadLevel07Screen(void)
{
    // TODO: Unload Level07 screen variables here!
}

// Level07 Screen should finish?
int FinishLevel07Screen(void)
{
    return finishScreen;
}

static bool CheckColor(Color col1, Color col2)
{
    return ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a));
}