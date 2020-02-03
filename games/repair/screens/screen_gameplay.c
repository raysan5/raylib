/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
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

static bool doHairCut = false;
static bool doHairTint = false;
static bool doEyeLiner = false;
static bool doLipStick = false;
static bool doNose = false;
static bool doGlasses = false;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

const unsigned int headColors[6] = { 0xffe29bff, 0xfed5a8ff, 0xad8962ff, 0xfff1b8ff, 0xffd6c4ff, 0xd49c8dff };
const unsigned int hairColors[10] = { 0xf5bf60ff, 0xaa754aff, 0x974e14ff, 0xf36347ff, 0x87f347ff, 0xfc48d0ff, 0x3b435dff, 0x5f5e60ff, 0xe7e7e7ff, 0xfb386bff };

// Gameplay screen global variables
static int framesCounter = 0;
static int finishScreen = 0;

static RenderTexture target = { 0 };

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    // Initialize GAMEPLAY screen variables
    framesCounter = 0;
    finishScreen = 0;
       
    target = LoadRenderTexture(720, 720);
    SetTextureFilter(target.texture, FILTER_BILINEAR);
       
    // Generate player character!
    //player = GenerateCharacter();
    playerBase = player;
    
    // Generate dating character!
    dating = GenerateCharacter();
    datingBase = dating;
    
    // TODO: Generate dating character likes
    // For the different types of properties we assign random like values: 0% (total-dislike) -> 100% (total-like)
    
    // The total match point will be the (like accumulated amount)/(num properties)
    // Some of the elements add points or remove points
    
    // At the end we can show the like percentadge of every element
    
    doHairCut = false;
    doHairTint = false;
    doEyeLiner = false;
    doLipStick = false;
    doNose = false;
    doGlasses = false;
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    if (IsKeyPressed(KEY_SPACE))
    {
        player = GenerateCharacter();
        playerBase = player;
    }
    
    if (IsKeyPressed(KEY_ENTER)) finishScreen = 1;
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    // Draw background
    DrawTexture(background, 0, 0, GetColor(0xf6aa60ff));
    
    // Draw left menu buttons
    GuiButton((Rectangle){ 20, 40, 300, 60 }, "RE-TOUCH:", 2);
    
    if (GuiButton((Rectangle){ 20, 40 + 90, 300, 80 }, "HAIR TINT", doHairTint? 3 : -1))
    {
        doHairTint = true;
        player.colHair = hairColors[GetRandomValue(0, 9)];
    }
    if (GuiButton((Rectangle){ 20, 40 + 180, 300, 80 }, "HAIR", doHairCut? 3 : -1))
    {
        doHairCut = true;
        player.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH);

    }
    if (GuiButton((Rectangle){ 20, 40 + 270, 300, 80 }, "EYES", doEyeLiner? 3 : -1))
    {
        doEyeLiner = true;
        player.eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH - 1);
    }
    if (GuiButton((Rectangle){ 20, 40 + 360, 300, 80 }, "NOSE", doNose? 3 : -1))
    {
        doNose = true;
        player.nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH - 1);
    }
    if (GuiButton((Rectangle){ 20, 40 + 450, 300, 80 }, "LIPS", doLipStick? 3 : -1))
    {
        doLipStick = true;
        player.mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH - 1);
    }
    if (GuiButton((Rectangle){ 20, 40 + 540, 300, 80 }, "GLASSES", 3))
    {
        doGlasses = true;
    }
    
    // Draw player
    DrawCharacter(player, (Vector2){ GetScreenWidth()/2 - 125, 80 });

    // Draw dating view
    GuiButton((Rectangle){ 970, 40, 260, 60 }, "DATING:", 2);
    GuiButton((Rectangle){ 970, 40 + 70, 260, 260 }, " ", 0);
    
    BeginTextureMode(target);
        DrawCharacter(dating, (Vector2){ (720 - 250)/2, (720 - 500)/2 });
    EndTextureMode();
    
    DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height }, (Rectangle){ 970, 40 + 70, 260, 260 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
   
    // Draw left button: date!
    if (GuiButton((Rectangle){ 970, 580, 260, 90 }, "GO DATE!", -1))
    {
        finishScreen = 1;
    }
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // Unload required textures
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}
