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
#include <stdlib.h>

#define MISSION_MAX_LENGTH 256
#define KEYWORD_MAX_LENGTH 32
#define MAX_LINE_CHAR 75

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Mission screen global variables
static int framesCounter;
static int finishScreen;

static Texture2D texBackground;

static Texture2D texBackline; //mission_backline
static Rectangle sourceRecBackLine;
static Rectangle destRecBackLine;
static float fadeBackLine;

static Vector2 numberPosition;
static Color numberColor;

//static char textMission[MISSION_MAX_LENGTH];
static Vector2 missionPosition;
static int missionSize;
static Color missionColor;
static int missionLenght;
static bool missionMaxLength;
static int missionSpeed;

//static char textKeyword[KEYWORD_MAX_LENGTH];
static Vector2 keywordPosition;
static Color keywordColor;

static int showMissionWaitFrames;
static int showNumberWaitFrames;
static int showKeywordWaitFrames;

static bool startWritting;
static bool writeMission;
static bool writeNumber;
static bool writeKeyword;
static bool writeEnd;

static bool writtingMission;

static int blinkFrames;
static bool blinkKeyWord = true;

static bool showButton = false;

static Mission *missions = NULL;

static Sound fxTransmit;
static Music musMission;

//----------------------------------------------------------------------------------
// Mission Screen Functions Definition
//----------------------------------------------------------------------------------
static void WriteMissionText();


// Mission Screen Initialization logic
void InitMissionScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;

    fadeButton = 0.80f;

    texBackground = LoadTexture("resources/textures/mission_background.png");

    texBackline = LoadTexture("resources/textures/mission_backline.png");
    sourceRecBackLine = (Rectangle){0,0,GetScreenWidth(), texBackline.height};
    destRecBackLine = (Rectangle){0,0,sourceRecBackLine.width, sourceRecBackLine.height};
    fadeBackLine = 0;

    fxTransmit = LoadSound("resources/audio/fx_message.ogg");
    musMission = LoadMusicStream("resources/audio/music_mission.ogg");

    PlayMusicStream(musMission);

    // Initialize missions
    missions = LoadMissions("resources/missions.txt");

    missionMaxLength = strlen(missions[currentMission].brief);

    // Insert line breaks every MAX_LINE_CHAR
    int currentLine = 1;
    int i = currentLine * MAX_LINE_CHAR;

    while (i < missionMaxLength)
    {
        if (missions[currentMission].brief[i] == ' ')
        {
            missions[currentMission].brief[i] = '\n';
            currentLine++;
            i = currentLine*MAX_LINE_CHAR;
        }
        else i++;
    }

    missionSize = 30;
    missionLenght = 0;
    missionSpeed = 1;

    numberColor = RAYWHITE;
    missionColor = LIGHTGRAY;
    keywordColor = (Color){198, 49, 60, 255}; //RED

    numberPosition = (Vector2){150, 185};
    missionPosition = (Vector2){numberPosition.x, numberPosition.y + 60};
    keywordPosition = (Vector2){missionPosition.x, missionPosition.y + MeasureTextEx(fontMission, missions[currentMission].brief, missionSize, 0).y + 60};

    startWritting = false;
    writeNumber = false;
    writeMission = false;
    writeKeyword = false;
    writeEnd = false;

    writtingMission = false;

    showNumberWaitFrames = 30;
    showMissionWaitFrames = 60;
    showKeywordWaitFrames = 60;

    blinkKeyWord = true;
    blinkFrames = 15;

    PlaySound(fxTransmit);
}

// Mission Screen Update logic
void UpdateMissionScreen(void)
{
    UpdateMusicStream(musMission);

    if (!writeEnd) WriteMissionText();
    else
    {
        framesCounter++;

        if ((framesCounter%blinkFrames) == 0)
        {
            framesCounter = 0;
            blinkKeyWord = !blinkKeyWord;
        }
    }

    if (showButton)
    {
        if (IsKeyPressed(KEY_ENTER) || IsButtonPressed())
        {
            if (!writeEnd)
            {
                writeEnd = true;
                writeKeyword = true;
                writeNumber = true;
                missionLenght = missionMaxLength;
            }
            else
            {
                finishScreen = true;
                showButton = false;
            }
        }
    }
}

// Mission Screen Draw logic
void DrawMissionScreen(void)
{
    // Draw MISSION screen here!
    DrawTexture(texBackground, 0,0, WHITE);
    DrawTexturePro(texBackline, sourceRecBackLine, destRecBackLine, (Vector2){0,0},0, Fade(WHITE, fadeBackLine));

    if (writeNumber) DrawTextEx(fontMission, FormatText("Filtración #%02i ", currentMission + 1), numberPosition, missionSize + 10, 0, numberColor);
    DrawTextEx(fontMission, TextSubtext(missions[currentMission].brief, 0, missionLenght), missionPosition, missionSize, 0, missionColor);
    if (writeKeyword && blinkKeyWord) DrawTextEx(fontMission, FormatText("Keyword: %s", missions[currentMission].key), keywordPosition, missionSize + 10, 0, keywordColor);

    if (showButton)
    {
        if (!writeEnd) DrawButton("saltar");
        else DrawButton("codificar");
    }
}

// Mission Screen Unload logic
void UnloadMissionScreen(void)
{
    // Unload MISSION screen variables here!
    UnloadTexture(texBackground);
    UnloadTexture(texBackline);
    UnloadSound(fxTransmit);
    UnloadMusicStream(musMission);
    free(missions);
}

// Mission Screen should finish?
int FinishMissionScreen(void)
{
    return finishScreen;
}

static void WriteMissionText()
{
    if (!startWritting)
    {
        framesCounter++;
        if (framesCounter % 60 == 0)
        {
            framesCounter = 0;
            startWritting = true;
        }
    }
    else if (!writeNumber)
    {
        framesCounter++;
        fadeBackLine += 0.020f;
        if (framesCounter % showNumberWaitFrames == 0)
        {
            framesCounter = 0;
            writeNumber = true;
            showButton = true;
        }
    }
    else if (!writeMission)
    {
        framesCounter ++;
        if (framesCounter % showMissionWaitFrames == 0)
        {
            framesCounter = 0;
            writeMission = true;
            writtingMission = true;
        }
    }
    else if (writeMission && writtingMission)
    {
        framesCounter++;
        if (framesCounter % missionSpeed == 0)
        {
            framesCounter = 0;
            missionLenght++;

            if (missionLenght == missionMaxLength)
            {
                writtingMission = false;
            }
        }
    }
    else if (!writeKeyword)
    {
        framesCounter++;
        if (framesCounter % showKeywordWaitFrames == 0)
        {
            framesCounter = 0;
            writeKeyword = true;
            writeEnd = true;
        }
    }
}
