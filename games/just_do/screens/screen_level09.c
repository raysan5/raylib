/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level09 Screen Functions Definitions (Init, Update, Draw, Unload)
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

#define NUM_BOXES   21

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Level09 screen global variables
static int framesCounter;
static int finishScreen;

static Rectangle bwRecs[NUM_BOXES];
static Color bwColors[NUM_BOXES];
static bool activated[NUM_BOXES];
static int resetCounter = 0;
static bool enableCounter = 0;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level09 Screen Functions Definition
//----------------------------------------------------------------------------------
static bool CheckColor(Color col1, Color col2);

// Level09 Screen Initialization logic
void InitLevel09Screen(void)
{
    // Initialize Level09 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    for (int i = 0; i < NUM_BOXES; i++)
    {
        bwRecs[i].x = GetScreenWidth()/7*(i%7);
        bwRecs[i].y = GetScreenHeight()/3*(i/7);
        bwRecs[i].width = GetScreenWidth()/7;
        bwRecs[i].height = GetScreenHeight()/3;
        
        activated[i] = false;
        
        if (i%2 == 0) bwColors[i] = LIGHTGRAY;
        else bwColors[i] = GRAY;
    }
    
    bwColors[10] = RAYWHITE;
}

// Level09 Screen Update logic
void UpdateLevel09Screen(void)
{
    // Update Level09 screen variables here!
    framesCounter++;
    if (enableCounter) resetCounter++;
    
    if (!done)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            for (int i = 0; i < NUM_BOXES; i++)
            {
                if (CheckCollisionPointRec(GetMousePosition(), bwRecs[i]))
                {               
                    if (i == 10)
                    {
                        if (CheckColor(bwColors[i], RAYWHITE))
                        {
                            bwColors[i] = LIGHTGRAY;
                            enableCounter = true;
                            resetCounter = 0;
                            activated[1] = true;
                        }
                        else
                        {
                            bwColors[i] = RAYWHITE;
                            enableCounter = false;
                            resetCounter = 5*60;
                            
                            for (int i = 0; i < NUM_BOXES; i++) activated[i] = false;
                        }
                    }
                    else if ((i%2 == 1) && enableCounter)
                    {
                        if (activated[i])
                        {
                            bwColors[i] = LIGHTGRAY;
                            if (i != 19) activated[i + 2] = true;
                        }
                    }
                }
            }
        }
        
        if (resetCounter > (4*60 + 10))
        {
            for (int i = 0; i < NUM_BOXES; i++)
            {
                if (i%2 == 0) bwColors[i] = LIGHTGRAY;
                else bwColors[i] = GRAY;
                
                activated[i] = false;
            }
        
            bwColors[10] = RAYWHITE;
            enableCounter = false;
            resetCounter = 0;
        }
    
        for (int i = 0; i < NUM_BOXES; i++)
        {
            done = true;
            
            if (!CheckColor(bwColors[i], LIGHTGRAY))
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

// Level09 Screen Draw logic
void DrawLevel09Screen(void)
{
    // Draw Level09 screen
    for (int i = 0; i < NUM_BOXES; i++) 
    {
        DrawRectangleRec(bwRecs[i], bwColors[i]);
    }
    
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(RAYWHITE, 0.6f));
        DrawText("LEVEL 09", GetScreenWidth()/2 - MeasureText("LEVEL 09", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 09", GetScreenWidth()/2 - MeasureText("LEVEL 09", 30)/2, 20, 30, LIGHTGRAY);
}

// Level09 Screen Unload logic
void UnloadLevel09Screen(void)
{
    // TODO: Unload Level09 screen variables here!
}

// Level09 Screen should finish?
int FinishLevel09Screen(void)
{
    return finishScreen;
}

static bool CheckColor(Color col1, Color col2)
{
    return ((col1.r == col2.r) && (col1.g == col2.g) && (col1.b == col2.b) && (col1.a == col2.a));
}