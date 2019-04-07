/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Ending Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Ending screen global variables
static int framesCounter;
static int finishScreen;

static int scrollPositionX;

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    // TODO: Initialize ENDING screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    PlayMusicStream(music);
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    framesCounter++;
    
    scrollPositionX -= 5;
    if (scrollPositionX < -GetScreenWidth()) scrollPositionX = 0;

    // Press enter or tap to return to TITLE screen
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    {
        finishScreen = 1;
        PlaySound(fxCoin);
    }
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    for (int i = 0; i < 64*2*2; i++)
    {
        DrawRectangle(64*i + scrollPositionX, 0, 64, GetScreenHeight(), (i%2 == 0)? GetColor(0xf3726dff) : GetColor(0xffcf6bff));
    }
    
    if (result == 0) DrawTextEx(font2, "YOU LOOSE...", (Vector2){ 350, 200 }, font2.baseSize*2, 2, WHITE);
    else if (result == 1) DrawTextEx(font, "YOU WIN!!!", (Vector2){ 380, 200 }, font.baseSize*2, 2, WHITE);
    
    // Draw score
    DrawTextEx(font, FormatText("FINAL SCORE: %i", score), (Vector2){ 400, 360 }, font2.baseSize, 2, WHITE);
    
    if ((framesCounter/30)%2) DrawTextEx(font2, "PRESS ENTER to TITLE", (Vector2){ 340, 550 }, font2.baseSize, 2, WHITE);
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