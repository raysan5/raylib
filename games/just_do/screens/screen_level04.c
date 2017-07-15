/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level04 Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Level04 screen global variables
static int framesCounter;
static int finishScreen;

static Vector2 circlesCenter;
static float innerCircleRadius = 40;
static float outerCircleRadius = 300;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level04 Screen Functions Definition
//----------------------------------------------------------------------------------

// Level04 Screen Initialization logic
void InitLevel04Screen(void)
{
    // Initialize Level04 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    circlesCenter = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
}

// Level04 Screen Update logic
void UpdateLevel04Screen(void)
{
    // Update Level04 screen variables here!
    framesCounter++;

    if (!done)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) 
        {
            if (CheckCollisionPointCircle(GetMousePosition(), circlesCenter, innerCircleRadius))
            {
                innerCircleRadius += 2;
            }
            else if (CheckCollisionPointCircle(GetMousePosition(), circlesCenter, outerCircleRadius))
            {
                outerCircleRadius += 2;
            }
            else
            {
                outerCircleRadius -= 2;
                
                if (outerCircleRadius <= 260) outerCircleRadius = 260;  
            }
        }
        else
        {
            if (!done)
            {
                innerCircleRadius -= 2;
                if (outerCircleRadius > 300) outerCircleRadius -= 2;
            }
        }

        if (innerCircleRadius >= 270) innerCircleRadius = 270;
        else if (innerCircleRadius <= 40) innerCircleRadius = 40;
        
        if (outerCircleRadius >= 600) outerCircleRadius = 600;

        if (innerCircleRadius >= outerCircleRadius)
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

// Level04 Screen Draw logic
void DrawLevel04Screen(void)
{
    // Draw Level04 screen here!
    //DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GRAY);
    DrawCircleV(circlesCenter, outerCircleRadius, GRAY);
    DrawCircleV(circlesCenter, innerCircleRadius, RAYWHITE);

    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 04", GetScreenWidth()/2 - MeasureText("LEVEL 04", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 04", GetScreenWidth()/2 - MeasureText("LEVEL 04", 30)/2, 20, 30, LIGHTGRAY);
}

// Level04 Screen Unload logic
void UnloadLevel04Screen(void)
{
    // TODO: Unload Level04 screen variables here!
}

// Level04 Screen should finish?
int FinishLevel04Screen(void)
{
    return finishScreen;
}