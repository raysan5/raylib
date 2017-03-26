/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level08 Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Level08 screen global variables
static int framesCounter;
static int finishScreen;

static Rectangle leftColumnRec, middleColumnRec, rightColumnRec;
static Rectangle movingBox;
static int moveSpeed = 4;

static bool leftColumnActive, middleColumnActive, rightColumnActive;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level08 Screen Functions Definition
//----------------------------------------------------------------------------------

// Level08 Screen Initialization logic
void InitLevel08Screen(void)
{
    // TODO: Initialize Level08 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    movingBox = (Rectangle){ 20, GetScreenHeight()/2 - 20, 40, 40 };
    
    leftColumnRec = (Rectangle){ 240, 0, 100, GetScreenHeight() };
    middleColumnRec = (Rectangle){ GetScreenWidth()/2 - 50, 0, 100, GetScreenHeight() };
    rightColumnRec = (Rectangle){ 920, 0, 100, GetScreenHeight() };
    
    leftColumnActive = true;
    middleColumnActive = false;
    rightColumnActive = true;
}

// Level08 Screen Update logic
void UpdateLevel08Screen(void)
{
    // Update Level08 screen variables here!
    framesCounter++;
    
    if (!done)
    {
        movingBox.x += moveSpeed;
        
        if (movingBox.x <= 0) moveSpeed *= -1;
            
        if ((leftColumnActive && (CheckCollisionRecs(leftColumnRec, movingBox))) ||
            (middleColumnActive && (CheckCollisionRecs(middleColumnRec, movingBox))) ||
            (rightColumnActive && (CheckCollisionRecs(rightColumnRec, movingBox)))) moveSpeed *= -1;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(GetMousePosition(), leftColumnRec))
            {
                middleColumnActive = false;
                rightColumnActive = true;
            }
            else if (CheckCollisionPointRec(GetMousePosition(), middleColumnRec))
            {
                rightColumnActive = false;
                leftColumnActive = true;
            }
            else if (CheckCollisionPointRec(GetMousePosition(), rightColumnRec))
            {
                leftColumnActive = false;
                middleColumnActive = true;
            }
        }
    
        if (movingBox.x >= 1100)
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

// Level08 Screen Draw logic
void DrawLevel08Screen(void)
{
    // Draw Level08 screen
    DrawRectangle(1100, GetScreenHeight()/2 - 20, 40, 40, GRAY);

    DrawRectangleRec(movingBox, LIGHTGRAY);
    
    if (leftColumnActive) DrawRectangleRec(leftColumnRec, GRAY);
    if (middleColumnActive) DrawRectangleRec(middleColumnRec, GRAY);
    if (rightColumnActive) DrawRectangleRec(rightColumnRec, GRAY);

    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 08", GetScreenWidth()/2 - MeasureText("LEVEL 08", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 08", GetScreenWidth()/2 - MeasureText("LEVEL 08", 30)/2, 20, 30, LIGHTGRAY);
}

// Level08 Screen Unload logic
void UnloadLevel08Screen(void)
{
    // TODO: Unload Level08 screen variables here!
}

// Level08 Screen should finish?
int FinishLevel08Screen(void)
{
    return finishScreen;
}