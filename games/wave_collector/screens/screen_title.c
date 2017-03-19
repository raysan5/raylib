/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Title Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Title screen global variables
static int framesCounter;
static int finishScreen;

static Texture2D texBackground;
static Texture2D texTitle;
static Texture2D texLogo;

static float titleAlpha = 0.0f;

static Sound fxStart;

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    // Initialize TITLE screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    texBackground = LoadTexture("resources/textures/background_title.png");
    texTitle = LoadTexture("resources/textures/title.png");
    texLogo = LoadTexture("resources/textures/logo_raylib.png");
    
    fxStart = LoadSound("resources/audio/start.wav");
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    // Update TITLE screen variables here!
    framesCounter++;
    
    titleAlpha += 0.005f;
    
    if (titleAlpha >= 1.0f) titleAlpha = 1.0f;

    // Press enter to change to ATTIC screen
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        PlaySound(fxStart);
        StopMusicStream(music);
        finishScreen = 1;
    }
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    DrawTexture(texBackground, 0, 0, WHITE);
    DrawTexture(texTitle, GetScreenWidth()/2 - texTitle.width/2, -25, Fade(WHITE, titleAlpha));
    
    DrawRectangle(0, GetScreenHeight() - 70, 560, 40, Fade(RAYWHITE, 0.8f));
    DrawText("(c) Developed by Ramon Santamaria (@raysan5)", 36, GetScreenHeight() - 60, 20, DARKBLUE); 
    
    DrawText("powered by", GetScreenWidth() - 162, GetScreenHeight() - 190, 20, DARKGRAY);
    DrawTexture(texLogo, GetScreenWidth() - 128 - 34, GetScreenHeight() - 128 - 36, WHITE);
    
    if ((framesCounter > 160) && ((framesCounter/40)%2)) DrawTextEx(font, "mouse click to start", (Vector2){ 325, 500 }, font.baseSize, 0, SKYBLUE);
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    // Unload TITLE screen variables here!
    UnloadTexture(texBackground);
    UnloadTexture(texTitle);
    UnloadTexture(texLogo);
    
    UnloadSound(fxStart);
}

// Title Screen should finish?
int FinishTitleScreen(void)
{
    return finishScreen;
}