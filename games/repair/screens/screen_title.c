/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Title Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2020 Ramon Santamaria (@raysan5)
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
static int framesCounter = 0;
static int finishScreen = 0;

static Texture2D texTitle = { 0 };
static Texture2D texLogo = { 0 };

static int titlePositionY = 0;
static int titleCounter = 0;

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    
    texTitle = LoadTexture("resources/title.png");
    texLogo = LoadTexture("resources/raylib_logo.png");
    
    player = GenerateCharacter();
    
    titlePositionY = -200;
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    framesCounter++;

    if (framesCounter > 5)
    {
        int partToChange = GetRandomValue(0, 4);
        
        if (partToChange == 0)
        {
            player.head = GetRandomValue(0, texHead.width/BASE_HEAD_WIDTH - 1);
            player.colHead = headColors[GetRandomValue(0, 5)];
        }
        else if (partToChange == 1) player.eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
        else if (partToChange == 2) player.nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
        else if (partToChange == 3) player.mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
        else if (partToChange == 4)
        {
            player.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH - 1);
            player.colHair = hairColors[GetRandomValue(0, 9)];
        }

        framesCounter = 0;
    }
    
    titlePositionY += 3;
    if (titlePositionY > 40) titlePositionY = 40;
    
    titleCounter++;
    
    if (IsKeyPressed(KEY_ENTER)) finishScreen = 1;
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    DrawTexture(background, 0, 0, GetColor(0xf6aa60ff));
    
    // Draw face, parts keep changing ranomly
    DrawCharacter(player, (Vector2){ GetScreenWidth()/2 - 125, 80 });
    
    // Draw face rectangles
    //DrawRectangleRec((Rectangle){ GetScreenWidth()/2 - BASE_EYES_WIDTH/2, 270, BASE_EYES_WIDTH, texEyes.height }, Fade(GREEN, 0.3f));
    //DrawRectangleRec((Rectangle){ GetScreenWidth()/2 - BASE_NOSE_WIDTH/2, 355, BASE_NOSE_WIDTH, texNose.height }, Fade(SKYBLUE, 0.3f));
    //DrawRectangleRec((Rectangle){ GetScreenWidth()/2 - BASE_MOUTH_WIDTH/2, 450, BASE_MOUTH_WIDTH, texMouth.height }, Fade(RED, 0.3f));
    
    DrawTexture(texTitle, GetScreenWidth()/2 - texTitle.width/2, titlePositionY, WHITE);

    if (titleCounter > 180)
    {
        if (GuiButton((Rectangle){ GetScreenWidth()/2 - 440/2, 580, 440, 80 }, "START DATE!", -1)) finishScreen = 1;   // GAMEPLAY
    }
    
    DrawText("powered by", 20, GetScreenHeight() - texLogo.height - 35, 10, BLACK);
    DrawTexture(texLogo, 20, GetScreenHeight() - texLogo.height - 20, WHITE);
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    UnloadTexture(texTitle);
    UnloadTexture(texLogo);
}

// Title Screen should finish?
int FinishTitleScreen(void)
{
    return finishScreen;
}