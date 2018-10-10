/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level06 Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Level06 screen global variables
static int framesCounter;
static int finishScreen;

static Rectangle centerRec;

static Rectangle movingRecs[4];
static int speedRecs[4];
static bool stoppedRec[4];
static int mouseOverNum = -1;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level06 Screen Functions Definition
//----------------------------------------------------------------------------------

// Level06 Screen Initialization logic
void InitLevel06Screen(void)
{
    // Initialize Level06 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    centerRec = (Rectangle){ GetScreenWidth()/2 - 100, 0, 200, GetScreenHeight() };
    
    for (int i = 0; i < 4; i++)
    {
        movingRecs[i] = (Rectangle){ GetRandomValue(0, 5)*150, (i*150) + 90, 100, 100 };
        stoppedRec[i] = false;
        speedRecs[i] = GetRandomValue(4, 8);
    }
}

// Level06 Screen Update logic
void UpdateLevel06Screen(void)
{
    // Update Level06 screen variables here!
    framesCounter++;

    if (!done)
    {
        for (int i = 0; i < 4; i++)
        {
            if (!stoppedRec[i]) movingRecs[i].x += speedRecs[i];
            
            if (movingRecs[i].x >= GetScreenWidth()) movingRecs[i].x = -movingRecs[i].width;
            
            if (CheckCollisionPointRec(GetMousePosition(), movingRecs[i]))
            {
                mouseOverNum = i;
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (i == 0) stoppedRec[3] = !stoppedRec[3];
                    else if (i == 1) stoppedRec[2] = !stoppedRec[2];
                    else if (i == 2) stoppedRec[0] = !stoppedRec[0];
                    else if (i == 3) stoppedRec[1] = !stoppedRec[1];
                }
            }
        }

        // Check if all boxes are aligned
        if (((movingRecs[0].x > centerRec.x) && ((movingRecs[0].x + movingRecs[0].width) < (centerRec.x + centerRec.width))) &&
            ((movingRecs[1].x > centerRec.x) && ((movingRecs[1].x + movingRecs[1].width) < (centerRec.x + centerRec.width))) &&
            ((movingRecs[2].x > centerRec.x) && ((movingRecs[2].x + movingRecs[2].width) < (centerRec.x + centerRec.width))) &&
            ((movingRecs[3].x > centerRec.x) && ((movingRecs[3].x + movingRecs[3].width) < (centerRec.x + centerRec.width))))
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

// Level06 Screen Draw logic
void DrawLevel06Screen(void)
{
    // Draw Level06 screen
    DrawRectangleRec(centerRec, LIGHTGRAY);
    
    for (int i = 0; i < 4; i++)
    {
        DrawRectangleRec(movingRecs[i], GRAY);
    }
    
    if (!done && (mouseOverNum >= 0)) DrawRectangleLines(movingRecs[mouseOverNum].x - 5, movingRecs[mouseOverNum].y - 5, movingRecs[mouseOverNum].width + 10, movingRecs[mouseOverNum].height + 10, Fade(LIGHTGRAY, 0.8f));
        
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 06", GetScreenWidth()/2 - MeasureText("LEVEL 06", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 06", GetScreenWidth()/2 - MeasureText("LEVEL 06", 30)/2, 20, 30, LIGHTGRAY);
}

// Level06 Screen Unload logic
void UnloadLevel06Screen(void)
{
    // TODO: Unload Level06 screen variables here!
}

// Level06 Screen should finish?
int FinishLevel06Screen(void)
{
    return finishScreen;
}