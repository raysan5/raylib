/**********************************************************************************************
*
*   raylib - transmission mission
*
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

#include <string.h>

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Title screen global variables
static int framesCounter;
static int finishScreen;

static Texture2D texBackground;
static Font fontTitle;
static Sound fxTyping;

static float titleSize;
static Vector2 transmissionPosition;
static Vector2 missionPositon;

static const char textTitle[20] = "transmissionmission";

static Color titleColor;
static int speedText;

static int transmissionLenght;
static int missionLenght;
static int transmissionMaxLenght;
static int missionMaxLenght;

static bool writeTransmission;
static bool writeMission;
static bool writeEnd;

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------
static void MissionScreen();

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    // TODO: Initialize TITLE screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    texBackground = LoadTexture("resources/textures/title_background.png");
    fxTyping = LoadSound("resources/audio/fx_typing.ogg");
    fontTitle = LoadFontEx("resources/fonts/mom_typewritter.ttf", 96, 0, 0);
    
    titleSize = 44;
    transmissionPosition = (Vector2){519, 221};
    missionPositon = (Vector2){580, 261};
    
    titleColor = BLACK;
    speedText = 15;
    
    missionLenght = 0;
    transmissionLenght = 0;
    
    missionMaxLenght = 7;
    transmissionMaxLenght = 12;
    
    writeTransmission = true;
    writeMission = false;
    writeEnd = false;
    
    currentMission = 0;
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    if (!writeEnd)
    {
        framesCounter ++;
        
        if (framesCounter%speedText == 0)
        {
            framesCounter = 0;
            if (writeTransmission)
            {
                transmissionLenght++;   
                if (transmissionLenght == transmissionMaxLenght)
                {
                    writeTransmission = false;
                    writeMission = true;
                }
            }
            else if (writeMission)
            {
                missionLenght++;
                if (missionLenght == missionMaxLenght)
                {
                    writeMission = false;
                    writeEnd = true;
                }
            }
            
            PlaySound(fxTyping);
        }
    }

    if(IsButtonPressed())
    {
        MissionScreen();
    }    
    else if (IsKeyPressed(KEY_ENTER)) MissionScreen();
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    DrawTexture(texBackground, 0,0, WHITE);
    DrawTextEx(fontTitle, TextSubtext(textTitle, 0, transmissionLenght), transmissionPosition, titleSize, 0, titleColor);
    DrawTextEx(fontTitle, TextSubtext(textTitle, 12, missionLenght), missionPositon, titleSize, 0, titleColor);      

    DrawButton("start");
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    UnloadTexture(texBackground);
    UnloadSound(fxTyping);
    UnloadFont(fontTitle);
}

// Title Screen should finish?
int FinishTitleScreen(void)
{
    return finishScreen;
}

static void MissionScreen()
{
    transmissionLenght = transmissionMaxLenght;
    missionLenght = missionMaxLenght;
    writeEnd = true;
    //finishScreen = 1;   // OPTIONS
    finishScreen = true;   // GAMEPLAY
    //PlaySound(fxCoin);
}