/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Level03 Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Level03 screen global variables
static int framesCounter;
static int finishScreen;

static Rectangle holeRec, pieceRec;
static bool showPiece = false;
static bool pieceSelected = false;

static bool done = false;
static int levelTimeSec = 0;
static bool levelFinished = false;

//----------------------------------------------------------------------------------
// Level03 Screen Functions Definition
//----------------------------------------------------------------------------------

// Level03 Screen Initialization logic
void InitLevel03Screen(void)
{
    // Initialize Level03 screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    holeRec = (Rectangle){ GetScreenWidth()/2 - 50, GetScreenHeight()/2 - 50, 100, 100 };
    pieceRec = (Rectangle){ 200, 400, 100, 100 };
}

// Level03 Screen Update logic
void UpdateLevel03Screen(void)
{
    // Update Level03 screen variables here!
    framesCounter++;
    
    Vector2 mousePos = GetMousePosition();
        
    if (!done)
    {
        if (CheckCollisionPointRec(mousePos, holeRec)) showPiece = true;
        else showPiece = false;
            
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePos, pieceRec))
            {
                pieceSelected = true;
                
                pieceRec.x = ((int)mousePos.x - 50);
                pieceRec.y = ((int)mousePos.y - 50);
            }
        }
    
        if ((pieceRec.x == holeRec.x) && !(CheckCollisionPointRec(mousePos, holeRec)))
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

// Level03 Screen Draw logic
void DrawLevel03Screen(void)
{
    // Draw Level03 screen
    DrawRectangleRec(holeRec, GRAY);
    DrawRectangleRec(pieceRec, RAYWHITE);
    
    if (showPiece) DrawRectangleLines(pieceRec.x, pieceRec.y, pieceRec.width, pieceRec.height, Fade(LIGHTGRAY, 0.8f));
    
    if (levelFinished)
    {
        DrawRectangleBordersRec((Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 0, 0, 60, Fade(LIGHTGRAY, 0.6f));
        DrawText("LEVEL 03", GetScreenWidth()/2 - MeasureText("LEVEL 03", 30)/2, 20, 30, GRAY);
        DrawText(FormatText("DONE! (Seconds: %03i)", levelTimeSec), GetScreenWidth()/2 - MeasureText("DONE! (Seconds: 000)", 30)/2, GetScreenHeight() - 40, 30, GRAY);
    }
    else DrawText("LEVEL 03", GetScreenWidth()/2 - MeasureText("LEVEL 03", 30)/2, 20, 30, LIGHTGRAY);
}

// Level03 Screen Unload logic
void UnloadLevel03Screen(void)
{
    // TODO: Unload Level03 screen variables here!
}

// Level03 Screen should finish?
int FinishLevel03Screen(void)
{
    return finishScreen;
}