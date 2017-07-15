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

static Texture2D texBackground;
static Texture2D texWin;
static Texture2D texLose;
static Texture2D texLogo;

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    // TODO: Initialize ENDING screen variables here!
    framesCounter = 0;
    finishScreen = 0;

    texBackground = LoadTexture("resources/textures/background.png");
    texWin = LoadTexture("resources/textures/win.png");
    texLose = LoadTexture("resources/textures/lose.png");
    texLogo = LoadTexture("resources/textures/logo_raylib.png");
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    // TODO: Update ENDING screen variables here!
    framesCounter++;

    // Press enter to return to TITLE screen
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        finishScreen = 1;
    }
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    DrawTexture(texBackground, 0, 0, WHITE);

    if (endingStatus == 1)          // Win
    {
        DrawTexture(texWin, GetScreenWidth()/2 - texWin.width/2, 90, WHITE);
        DrawTextEx(font, "congrats, you got the wave!", (Vector2){ 200, 335 }, font.baseSize, 0, WHITE);
    }
    else if (endingStatus == 2)     // Lose
    {
        DrawTexture(texLose, GetScreenWidth()/2 - texWin.width/2, 90, WHITE);
        DrawTextEx(font, "it seems you lose the wave...", (Vector2){ 205, 335 }, font.baseSize, 0, WHITE);
    }
    
    DrawRectangle(0, GetScreenHeight() - 70, 560, 40, Fade(RAYWHITE, 0.8f));
    DrawText("(c) Developed by Ramon Santamaria (@raysan5)", 36, GetScreenHeight() - 60, 20, DARKBLUE);
    
    DrawText("powered by", GetScreenWidth() - 162, GetScreenHeight() - 190, 20, DARKGRAY);
    DrawTexture(texLogo, GetScreenWidth() - 128 - 34, GetScreenHeight() - 128 - 36, WHITE);
    
    if ((framesCounter > 80) && ((framesCounter/40)%2)) DrawTextEx(font, "mouse click to return", (Vector2){ 300, 464 }, font.baseSize, 0, SKYBLUE);
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
    // TODO: Unload ENDING screen variables here!
    UnloadTexture(texBackground);
    UnloadTexture(texWin);
    UnloadTexture(texLose);
    UnloadTexture(texLogo);
}

// Ending Screen should finish?
int FinishEndingScreen(void)
{
    return finishScreen;
}