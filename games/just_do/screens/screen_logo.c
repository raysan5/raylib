/**********************************************************************************************
*
*   raylib - Standard Game template
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

#include <string.h>

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Logo screen global variables
static int framesCounter;
static int finishScreen;

const char msgLogoA[64] = "A simple and easy-to-use library";
const char msgLogoB[64] = "to enjoy videogames programming";

int logoPositionX;
int logoPositionY;

int raylibLettersCount = 0;

int topSideRecWidth = 16;
int leftSideRecHeight = 16;

int bottomSideRecWidth = 16;
int rightSideRecHeight = 16;

char raylib[8] = "       \0";   // raylib text array, max 8 letters

int logoScreenState = 0;        // Tracking animation states (State Machine)
bool msgLogoADone = false;
bool msgLogoBDone = false;

int lettersCounter = 0;
char msgBuffer[128] = { ' ' };

//----------------------------------------------------------------------------------
// Logo Screen Functions Definition
//----------------------------------------------------------------------------------

// Logo Screen Initialization logic
void InitLogoScreen(void)
{
    // Initialize LOGO screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    logoPositionX = GetScreenWidth()/2 - 128;
    logoPositionY = GetScreenHeight()/2 - 128;
}

// Logo Screen Update logic
void UpdateLogoScreen(void)
{
    // Update LOGO screen
    framesCounter++;    // Count frames
    
    // Update LOGO screen variables
    if (logoScreenState == 0)                 // State 0: Small box blinking
    {
        framesCounter++;

        if (framesCounter == 120)
        {
            logoScreenState = 1;
            framesCounter = 0;      // Reset counter... will be used later...
        }
    }
    else if (logoScreenState == 1)            // State 1: Top and left bars growing
    {
        topSideRecWidth += 4;
        leftSideRecHeight += 4;

        if (topSideRecWidth == 256) logoScreenState = 2;
    }
    else if (logoScreenState == 2)            // State 2: Bottom and right bars growing
    {
        bottomSideRecWidth += 4;
        rightSideRecHeight += 4;

        if (bottomSideRecWidth == 256)
        {
            lettersCounter = 0;
            for (int i = 0; i < strlen(msgBuffer); i++) msgBuffer[i] = ' ';

            logoScreenState = 3;
        }
    }
    else if (logoScreenState == 3)            // State 3: Letters appearing (one by one)
    {
        framesCounter++;

        // Every 12 frames, one more letter!
        if ((framesCounter%12) == 0) raylibLettersCount++;

        switch (raylibLettersCount)
        {
            case 1: raylib[0] = 'r'; break;
            case 2: raylib[1] = 'a'; break;
            case 3: raylib[2] = 'y'; break;
            case 4: raylib[3] = 'l'; break;
            case 5: raylib[4] = 'i'; break;
            case 6: raylib[5] = 'b'; break;
            default: break;
        }

        if (raylibLettersCount >= 10)
        {
            // Write raylib description messages
            if ((framesCounter%2) == 0) lettersCounter++;

            if (!msgLogoADone)
            {
                if (lettersCounter <= strlen(msgLogoA)) strncpy(msgBuffer, msgLogoA, lettersCounter);
                else
                {
                    for (int i = 0; i < strlen(msgBuffer); i++) msgBuffer[i] = ' ';

                    lettersCounter = 0;
                    msgLogoADone = true;
                }
            }
            else if (!msgLogoBDone)
            {
                if (lettersCounter <= strlen(msgLogoB)) strncpy(msgBuffer, msgLogoB, lettersCounter);
                else
                {
                    msgLogoBDone = true;
                    framesCounter = 0;
                    PlaySound(levelWin);
                }
            }
        }
    }

    // Wait for 2 seconds (60 frames) before jumping to TITLE screen
    if (msgLogoBDone)
    {
        framesCounter++;
        
        if (framesCounter > 90) finishScreen = true;
    }
}

// Logo Screen Draw logic
void DrawLogoScreen(void)
{
    // Draw LOGO screen
    if (logoScreenState == 0)
    {
        if ((framesCounter/15)%2) DrawRectangle(logoPositionX, logoPositionY - 60, 16, 16, BLACK);
    }
    else if (logoScreenState == 1)
    {
        DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
        DrawRectangle(logoPositionX, logoPositionY - 60, 16, leftSideRecHeight, BLACK);
    }
    else if (logoScreenState == 2)
    {
        DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
        DrawRectangle(logoPositionX, logoPositionY - 60, 16, leftSideRecHeight, BLACK);

        DrawRectangle(logoPositionX + 240, logoPositionY - 60, 16, rightSideRecHeight, BLACK);
        DrawRectangle(logoPositionX, logoPositionY + 240 - 60, bottomSideRecWidth, 16, BLACK);
    }
    else if (logoScreenState == 3)
    {         
        DrawRectangle(logoPositionX, logoPositionY - 60, topSideRecWidth, 16, BLACK);
        DrawRectangle(logoPositionX, logoPositionY + 16 - 60, 16, leftSideRecHeight - 32, BLACK);

        DrawRectangle(logoPositionX + 240, logoPositionY + 16 - 60, 16, rightSideRecHeight - 32, BLACK);
        DrawRectangle(logoPositionX, logoPositionY + 240 - 60, bottomSideRecWidth, 16, BLACK);

        DrawRectangle(GetScreenWidth()/2 - 112, GetScreenHeight()/2 - 112 - 60, 224, 224, RAYWHITE);

        DrawText(raylib, GetScreenWidth()/2 - 44, GetScreenHeight()/2 + 48 - 60, 50, BLACK);

        if (!msgLogoADone) DrawText(msgBuffer, GetScreenWidth()/2 - MeasureText(msgLogoA, 30)/2, logoPositionY + 230, 30, GRAY);
        else
        {
            DrawText(msgLogoA, GetScreenWidth()/2 - MeasureText(msgLogoA, 30)/2, logoPositionY + 230, 30, GRAY);

            if (!msgLogoBDone) DrawText(msgBuffer, GetScreenWidth()/2 - MeasureText(msgLogoB, 30)/2, logoPositionY + 280, 30, GRAY);
            else
            {
                DrawText(msgLogoB, GetScreenWidth()/2 - MeasureText(msgLogoA, 30)/2, logoPositionY + 280, 30, GRAY);
            }
        }
    }
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