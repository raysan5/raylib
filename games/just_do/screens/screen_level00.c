/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level00 Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Level00 screen global variables
static int framesCounter;
static int finishScreen;

static Rectangle boundsU, boundsO;

static bool mouseOverU = false;
static bool mouseOverO = false;
static bool placedU = false;
static bool placedO = false;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level00 Screen Functions Definition
//----------------------------------------------------------------------------------

// Level00 Screen Initialization logic
void InitLevel00Screen(void)
{
    // Initialize Level00 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    boundsU = (Rectangle){GetScreenWidth()/2 - 265, -200, MeasureText("U", 160) + 40, 160 };
    boundsO = (Rectangle){GetScreenWidth() - 370, -30, MeasureText("O", 160) + 40, 160 };
}

// Level00 Screen Update logic
void UpdateLevel00Screen(void)
{
    // Update Level00 screen variables here!
    if (!done) framesCounter++;
    
    if (!done)
    {
        if (!placedU) boundsU.y += 2;
        
        if (boundsU.y >= GetScreenHeight()) boundsU.y = -boundsU.height;
        
        Vector2 mousePos = GetMousePosition();

        if (CheckCollisionPointRec(mousePos, boundsU))
        {
            mouseOverU = true;
            
            if (!placedU && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if ((boundsU.y > GetScreenHeight()/2 - 110) && ((boundsU.y + boundsU.height) < (GetScreenHeight()/2 + 100)))
                {
                    placedU = true;
                }
            }
        }
        else mouseOverU = false;
        
        if (CheckCollisionPointRec(mousePos, boundsO))
        {
            mouseOverO = true;
            
            if (!placedO && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) boundsO.y += 100;
            
            if (boundsO.y >= (GetScreenHeight()/2 - 130)) placedO = true;
        }
        else mouseOverO = false;
        
        if (placedO && placedU)
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
        
        if ((framesCounter > 30) && (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) finishScreen = true;
    }
}

// Level00 Screen Draw logic
void DrawLevel00Screen(void)
{
    // Draw Level00 screen
    DrawText("U", boundsU.x, boundsU.y + 10, 160, GRAY);
    DrawText("J", GetScreenWidth()/2 - MeasureText("JUST DO", 160)/2, GetScreenHeight()/2 - 80, 160, GRAY);
    DrawText("ST D", GetScreenWidth()/2 - MeasureText("JUST DO", 160)/2 + 210, GetScreenHeight()/2 - 80, 160, GRAY);
    DrawText("O", boundsO.x, boundsO.y + 10, 160, GRAY);
    
    DrawText("by RAMON SANTAMARIA (@raysan5)", 370, GetScreenHeight()/2 + 100, 30, Fade(LIGHTGRAY, 0.4f));
    
    if (mouseOverU && !placedU) DrawRectangleLines(boundsU.x - 20, boundsU.y, boundsU.width, boundsU.height, Fade(LIGHTGRAY, 0.8f));
    //DrawRectangleBordersRec(boundsU, -20, 0, 20, Fade(RED, 0.3f));
    
    if (mouseOverO && !placedO) DrawRectangleLines(boundsO.x - 20, boundsO.y, boundsO.width, boundsO.height, Fade(LIGHTGRAY, 0.8f));
    //DrawRectangleBordersRec(boundsO, -20, 0, 20, Fade(RED, 0.3f));
        
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 00", GetScreenWidth()/2 - MeasureText("LEVEL 00", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 00", GetScreenWidth()/2 - MeasureText("LEVEL 00", 30)/2, 20, 30, LIGHTGRAY);
}

// Level00 Screen Unload logic
void UnloadLevel00Screen(void)
{
    // TODO: Unload Level00 screen variables here!
}

// Level00 Screen should finish?
int FinishLevel00Screen(void)
{
    return finishScreen;
}

void DrawRectangleBordersRec(Rectangle rec, int offsetX, int offsetY, int borderSize, Color col)
{
    DrawRectangle(rec.x + offsetX, rec.y + offsetY, rec.width, borderSize, col);
    DrawRectangle(rec.x + offsetX, rec.y + borderSize + offsetY, borderSize, rec.height - borderSize*2, col);
    DrawRectangle(rec.x + rec.width - borderSize + offsetX, rec.y + borderSize + offsetY, borderSize, rec.height - borderSize*2, col);
    DrawRectangle(rec.x + offsetX, rec.y + rec.height - borderSize + offsetY, rec.width, borderSize, col);
}