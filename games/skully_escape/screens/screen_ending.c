/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Ending Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Ending screen global variables
static int framesCounter;
static int finishScreen;

static float alpha = 0.0f;

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    // TODO: Initialize ENDING screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    alpha = 0.0f;
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    // TODO: Update ENDING screen variables here!
    framesCounter++;
    
    alpha += 0.005f;
    
    if (alpha >= 1.0f) alpha = 1.0f;

    // Press enter to change to ATTIC screen
    if ((IsKeyPressed(KEY_ENTER)) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
    {
        finishScreen = 1;
    }
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), DARKGRAY);

    DrawTextEx(font, "CONGRATULATIONS!", (Vector2){ 50, 160 }, font.baseSize*3, 2, Fade(WHITE, alpha));
    DrawTextEx(font, "SKULLY ESCAPED!", (Vector2){ 100, 300 }, font.baseSize*3, 2, Fade(WHITE, alpha));
    
    if ((framesCounter > 180) && ((framesCounter/40)%2)) DrawText("PRESS ENTER or CLICK", 380, 545, 40, BLACK);
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
    // TODO: Unload ENDING screen variables here!
}

// Ending Screen should finish?
int FinishEndingScreen(void)
{
    return finishScreen;
}
