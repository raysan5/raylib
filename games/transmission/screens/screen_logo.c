/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Logo Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
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

static Texture2D logoCW;

static float fadeValue;
static int showLogoFrames;
static bool fadeOut;

//----------------------------------------------------------------------------------
// Logo Screen Functions Definition
//----------------------------------------------------------------------------------

// Logo Screen Initialization logic
void InitLogoScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    
    logoCW = LoadTexture("resources/textures/cw_logo.png");
    
    showLogoFrames = 60;
    fadeValue = 0;
    
    fadeOut = false;
}

// Logo Screen Update logic
void UpdateLogoScreen(void)
{
    if(!fadeOut)
    {       
        fadeValue += 0.02f;
        if(fadeValue > 1.01f)
        {
            fadeValue = 1.0f;
            framesCounter++;
            
            if(framesCounter % showLogoFrames == 0)
            {
                fadeOut = true;
                finishScreen = true;
            }  
        }        
    }   
    
    if(IsKeyPressed(KEY_ENTER))
    {
        finishScreen = true;
    }
}

// Logo Screen Draw logic
void DrawLogoScreen(void)
{
    DrawTexture(logoCW, GetScreenWidth()/2 - logoCW.width/2, GetScreenHeight()/2 - logoCW.height/2, Fade(WHITE, fadeValue));
}

// Logo Screen Unload logic
void UnloadLogoScreen(void)
{
    UnloadTexture(logoCW);
}

// Logo Screen should finish?
int FinishLogoScreen(void)
{
    return finishScreen;
}