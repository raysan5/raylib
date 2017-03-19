/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level01 Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Level01 screen global variables
static int framesCounter;
static int finishScreen;

static Rectangle innerLeftRec, outerLeftRec;
static Rectangle innerRightRec, outerRightRec;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level01 Screen Functions Definition
//----------------------------------------------------------------------------------

// Level01 Screen Initialization logic
void InitLevel01Screen(void)
{
    // Initialize Level01 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    outerLeftRec = (Rectangle){ 0, 0, GetScreenWidth()/2, GetScreenHeight() };
    outerRightRec = (Rectangle){ GetScreenWidth()/2, 0, GetScreenWidth()/2, GetScreenHeight() };
    
    innerLeftRec = (Rectangle){ GetScreenWidth()/4 - 200, GetScreenHeight()/2 - 200, 400, 400};
    innerRightRec = (Rectangle){ GetScreenWidth()/2 + GetScreenWidth()/4 - 200, GetScreenHeight()/2 - 200, 400, 400};
}

// Level01 Screen Update logic
void UpdateLevel01Screen(void)
{
    // Update Level01 screen
    framesCounter++;
    
    if (!done)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(GetMousePosition(), innerLeftRec))
            {
                if (innerRightRec.width > 0)
                {
                    innerRightRec.x += 20;
                    innerRightRec.y += 20;
                    innerRightRec.width -= 40;
                    innerRightRec.height -= 40;
                }
            }
            else if (CheckCollisionPointRec(GetMousePosition(), innerRightRec))
            {
                if (innerLeftRec.width > 0)
                {
                    innerLeftRec.x += 20;
                    innerLeftRec.y += 20;
                    innerLeftRec.width -= 40;
                    innerLeftRec.height -= 40;
                }
            }
            else if (CheckCollisionPointRec(GetMousePosition(), outerLeftRec))
            {
                innerLeftRec.x -= 20;
                innerLeftRec.y -= 20;
                innerLeftRec.width += 40;
                innerLeftRec.height += 40;
            }
            else if (CheckCollisionPointRec(GetMousePosition(), outerRightRec))
            {
                innerRightRec.x -= 20;
                innerRightRec.y -= 20;
                innerRightRec.width += 40;
                innerRightRec.height += 40;
            }
        }
    
    
        if (((innerRightRec.width == 0) && (innerLeftRec.height >= GetScreenHeight())) ||
            ((innerLeftRec.width == 0) && (innerRightRec.height >= GetScreenHeight())))
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

// Level01 Screen Draw logic
void DrawLevel01Screen(void)
{
    // Draw Level01 screen
    if (!levelFinished) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), LIGHTGRAY);
    else DrawRectangle(60, 60, GetScreenWidth() - 120, GetScreenHeight() - 120, LIGHTGRAY);
    
    DrawRectangleRec(outerLeftRec, GRAY);
    DrawRectangleRec(innerLeftRec, RAYWHITE);
    DrawRectangleRec(outerRightRec, RAYWHITE);
    DrawRectangleRec(innerRightRec, GRAY);
    
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 01", GetScreenWidth()/2 - MeasureText("LEVEL 01", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 01", GetScreenWidth()/2 - MeasureText("LEVEL 01", 30)/2, 20, 30, LIGHTGRAY);
}

// Level01 Screen Unload logic
void UnloadLevel01Screen(void)
{
    // TODO: Unload Level01 screen variables here!
}

// Level01 Screen should finish?
int FinishLevel01Screen(void)
{
    return finishScreen;
}