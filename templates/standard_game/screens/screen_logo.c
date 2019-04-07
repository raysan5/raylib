/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Logo Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
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

// Logo screen global variables
static int framesCounter;
static int finishScreen;

//----------------------------------------------------------------------------------
// Logo Screen Functions Definition
//----------------------------------------------------------------------------------

// Logo Screen Initialization logic
void InitLogoScreen(void)
{
    // TODO: Initialize LOGO screen variables here!
    framesCounter = 0;
    finishScreen = 0;
}

// Logo Screen Update logic
void UpdateLogoScreen(void)
{
    // TODO: Update LOGO screen variables here!

    framesCounter++;    // Count frames

    // Wait for 2 seconds (120 frames) before jumping to TITLE screen
    if (framesCounter > 120)
    {
        finishScreen = true;
    }
}

// Logo Screen Draw logic
void DrawLogoScreen(void)
{
    // TODO: Draw LOGO screen here!
    DrawText("LOGO SCREEN", 20, 20, 40, LIGHTGRAY);
    DrawText("WAIT for 2 SECONDS...", 290, 220, 20, GRAY);
}

// Logo Screen Unload logic
void UnloadLogoScreen(void)
{
    // TODO: Unload LOGO screen variables here!
}

// Logo Screen should finish?
int FinishLogoScreen(void)
{
    return finishScreen;
}