/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Logo Screen Functions Definitions (Init, Update, Draw, Unload)
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

// Logo screen global variables
static int framesCounter = 0;
static int finishScreen;

static Texture2D logo;
static float logoAlpha = 0;

static int state = 0;

//----------------------------------------------------------------------------------
// Logo Screen Functions Definition
//----------------------------------------------------------------------------------

// Logo Screen Initialization logic
void InitLogoScreen(void)
{
    // Initialize LOGO screen variables here!
    finishScreen = 0;
    
    logo = LoadTexture("resources/textures/skully_logo.png");
}

// Logo Screen Update logic
void UpdateLogoScreen(void)
{
    // Update LOGO screen variables here!
    if (state == 0)
    {
        logoAlpha += 0.04f;
        
        if (logoAlpha >= 1.0f) state = 1;
    }
    else if (state == 1)
    {
        framesCounter++;
        
        if (framesCounter > 180) state = 2;
    }
    else if (state == 2)
    {
        logoAlpha -= 0.04f;
        
        if (logoAlpha <= 0.0f) 
        {
            framesCounter = 0;
            state = 3;
        }
    }
    else if (state == 3)
    {
        finishScreen = 1;
    }
}

// Logo Screen Draw logic
void DrawLogoScreen(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), RAYWHITE);

    DrawTexture(logo, GetScreenWidth()/2 - logo.width/2, 130, Fade(WHITE, logoAlpha));
    
    DrawText("GRAY TEAM", 340, 450, 100, Fade(DARKGRAY, logoAlpha));
}

// Logo Screen Unload logic
void UnloadLogoScreen(void)
{
    // Unload LOGO screen variables here!
    UnloadTexture(logo);
}

// Logo Screen should finish?
int FinishLogoScreen(void)
{
    return finishScreen;
}