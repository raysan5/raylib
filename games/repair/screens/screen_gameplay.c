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
static bool doMakeup = false;
static bool doGlasses = false;
static bool doPiercing = false;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

const unsigned int headColors[6] = { 0xffe29bff, 0xfed5a8ff, 0xad8962ff, 0xfff1b8ff, 0xffd6c4ff, 0xd49c8dff };
const unsigned int hairColors[10] = { 0xf5bf60ff, 0xaa754aff, 0x974e14ff, 0xf36347ff, 0x87f347ff, 0xfc48d0ff, 0x3b435dff, 0x5f5e60ff, 0xe7e7e7ff, 0xfb386bff };

// Gameplay screen global variables
static int framesCounter = 0;
static int finishScreen = 0;

static Character player = { 0 };
static Character dating = { 0 };

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
    player = GenerateCharacter();
    
    // Generate dating character!
    dating = GenerateCharacter();
    
    // TODO: Generate dating character likes
    // For the different types of properties we assign random like values: 0% (total-dislike) -> 100% (total-like)
    
    // The total match point will be the (like accumulated amount)/(num properties)
    // Some of the elements add points or remove points
    
    // At the end we can show the like percentadge of every element
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    // Update GAMEPLAY screen
    
    // TODO: Check buttons and checkboxes to update player properties
    
    if (IsKeyPressed(KEY_SPACE)) player = GenerateCharacter();
    
    // Press enter or tap to change to ENDING screen
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    {
        finishScreen = 1;
        PlaySound(fxCoin);
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    // Draw background (texture, color, vignette?)
    DrawTexture(background, 0, 0, GetColor(0xf6aa60ff));
    
    // TODO: Draw left menu buttons
    if (GuiButton((Rectangle){ 20, 40, 300, 80 }, "HAIR CUT", doHairCut? 3 : -1))
    {
        doHairCut = true;
        
        if (GetRandomValue(0, 100) > 20) player.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH);
        else 
        {
            player.hair = -1;
            
            // TODO: Play problem sound!
        }
    }
    if (GuiButton((Rectangle){ 20, 40 + 90, 300, 80 }, "HAIR TINT", doHairTint? 3 : -1))
    {
        doHairTint = true;
        player.colHair = hairColors[GetRandomValue(0, 9)];
    }
    if (GuiButton((Rectangle){ 20, 40 + 180, 300, 80 }, "EYELINER", doEyeLiner? 3 : -1))
    {
        doEyeLiner = true;
    }
    if (GuiButton((Rectangle){ 20, 40 + 270, 300, 80 }, "LIPSTICK", doLipStick? 3 : -1))
    {
        doLipStick = true;
    }
    if (GuiButton((Rectangle){ 20, 40 + 360, 300, 80 }, "MAKEUP", doMakeup? 3 : -1))
    {
        doMakeup = true;
        player.makeup = GetRandomValue(1, 2);
    }
    if (GuiButton((Rectangle){ 20, 40 + 540, 300, 80 }, "GLASSES", doGlasses? 3 : -1))
    {
        doGlasses = true;
    }
    if (GuiButton((Rectangle){ 20, 40 + 540, 300, 80 }, "PIERCING", doPiercing? 3 : -1))
    {
        doPiercing = true;
    }
    
    // Draw player
    DrawCharacter(player);

    // Draw dating view
    GuiButton((Rectangle){ 970, 40, 260, 60 }, "DATING...", 2);
    GuiButton((Rectangle){ 970, 40 + 70, 260, 260 }, " ", 0);
    BeginTextureMode(target);
        DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*dating.hair, 240, BASE_HAIR_WIDTH, texHair.height - 240 }, (Vector2){ target.texture.width/2 - BASE_HAIR_WIDTH/2, 80 + 240 }, GetColor(dating.colHair));
        DrawTextureRec(texHead, (Rectangle){ BASE_HEAD_WIDTH*dating.head, 0, BASE_HEAD_WIDTH, texHead.height }, (Vector2){ target.texture.width/2 - BASE_HEAD_WIDTH/2, 140 }, GetColor(dating.colHead));
        DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*dating.hair, 0, BASE_HAIR_WIDTH, 240 }, (Vector2){ target.texture.width/2 - BASE_HAIR_WIDTH/2, 80 }, GetColor(dating.colHair));
        DrawTextureRec(texEyes, (Rectangle){ BASE_EYES_WIDTH*dating.eyes, 0, BASE_EYES_WIDTH, texEyes.height }, (Vector2){ target.texture.width/2 - BASE_EYES_WIDTH/2, 270 }, WHITE);
        DrawTextureRec(texNose, (Rectangle){ BASE_NOSE_WIDTH*dating.nose, 0, BASE_NOSE_WIDTH, texNose.height }, (Vector2){ target.texture.width/2 - BASE_NOSE_WIDTH/2, 355 }, GetColor(dating.colHead));
        DrawTextureRec(texMouth, (Rectangle){ BASE_MOUTH_WIDTH*dating.mouth, 0, BASE_MOUTH_WIDTH, texMouth.height }, (Vector2){ target.texture.width/2 - BASE_MOUTH_WIDTH/2, 450 }, GetColor(dating.colHead));
        
        if (dating.makeup) DrawTextureRec(texMakeup, (Rectangle){ 0, 0, texMakeup.width, texMakeup.height }, (Vector2){ target.texture.width/2 - texMakeup.width/2, 350 }, Fade(RED, 0.3f));
    EndTextureMode();
    
    DrawTexturePro(target.texture, (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height }, (Rectangle){ 970, 40 + 70, 260, 260 }, (Vector2){ 0, 0 }, 0.0f, WHITE);
   
    // Draw left button: date!
    if (GuiButton((Rectangle){ 970, 580, 260, 90 }, "GO DATE!", -1))
    {
        CustomizeCharacter(&dating);

        finishScreen = 1;
        PlaySound(fxCoin);
    }
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // Unload GAMEPLAY screen variables
        
    // Unload required textures
    UnloadTexture(texHead);
    UnloadTexture(texHair);
    UnloadTexture(texNose);
    UnloadTexture(texMouth);
    UnloadTexture(texEyes);
    //UnloadTexture(texComp);
    UnloadTexture(texMakeup);
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

Character GenerateCharacter(void)
{
    Character character = { 0 };
    
    // Generate player character!
    character.head = GetRandomValue(0, texHead.width/BASE_HEAD_WIDTH);
    character.colHead = headColors[GetRandomValue(0, 5)];
    character.hair = GetRandomValue(0, texHair.width/BASE_HAIR_WIDTH);     // Config (decrease value only)
    character.colHair = hairColors[GetRandomValue(0, 9)];                  // Config
    character.eyes = GetRandomValue(0, texEyes.width/BASE_EYES_WIDTH);
    character.nose = GetRandomValue(0, texNose.width/BASE_NOSE_WIDTH);
    character.mouth = GetRandomValue(0, texMouth.width/BASE_MOUTH_WIDTH);
    
    // NOTE: No character customization at this point
    
    return character;
}

void CustomizeCharacter(Character *character)
{
    character->eyeLine = GetRandomValue(0, 2);      // Config
    character->paintLips = GetRandomValue(0, 2);    // Config
    character->makeup = GetRandomValue(0, 2);       // Config
    //character->bear = GetRandomValue(0, 1);       // Config: remove
    //character->moustache = GetRandomValue(0, 1);  // Config: remove
    character->glasses = GetRandomValue(0, 3);      // Config: put/remove
    character->piercing = GetRandomValue(0, 3);     // Config: put/remove
}

void DrawCharacter(Character character)
{
    if (player.hair >= 0) DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*character.hair, 240, BASE_HAIR_WIDTH, texHair.height - 240 }, (Vector2){ GetScreenWidth()/2 - BASE_HAIR_WIDTH/2, 80 + 240 }, GetColor(character.colHair));
    DrawTextureRec(texHead, (Rectangle){ BASE_HEAD_WIDTH*character.head, 0, BASE_HEAD_WIDTH, texHead.height }, (Vector2){ GetScreenWidth()/2 - BASE_HEAD_WIDTH/2, 140 }, GetColor(character.colHead));
    DrawTextureRec(texHair, (Rectangle){ BASE_HAIR_WIDTH*character.hair, 0, BASE_HAIR_WIDTH, 240 }, (Vector2){ GetScreenWidth()/2 - BASE_HAIR_WIDTH/2, 80 }, GetColor(character.colHair));
    DrawTextureRec(texEyes, (Rectangle){ BASE_EYES_WIDTH*character.eyes, 0, BASE_EYES_WIDTH, texEyes.height }, (Vector2){ GetScreenWidth()/2 - BASE_EYES_WIDTH/2, 270 }, WHITE);
    DrawTextureRec(texNose, (Rectangle){ BASE_NOSE_WIDTH*character.nose, 0, BASE_NOSE_WIDTH, texNose.height }, (Vector2){ GetScreenWidth()/2 - BASE_NOSE_WIDTH/2, 355 }, GetColor(character.colHead));
    DrawTextureRec(texMouth, (Rectangle){ BASE_MOUTH_WIDTH*character.mouth, 0, BASE_MOUTH_WIDTH, texMouth.height }, (Vector2){ GetScreenWidth()/2 - BASE_MOUTH_WIDTH/2, 450 }, GetColor(character.colHead));
    
    if (character.makeup) DrawTextureRec(texMakeup, (Rectangle){ 0, 0, texMakeup.width, texMakeup.height }, (Vector2){ GetScreenWidth()/2 - texMakeup.width/2, 350 }, Fade(RED, 0.3f));
}

// Gui Button
bool GuiButton(Rectangle bounds, const char *text, int forcedState)
{
    static const int textColor[4] = { 0xeff6ffff, 0x78e782ff, 0xb04d5fff, 0xd6d6d6ff };
    
    int state = (forcedState >= 0)? forcedState : 0;                // NORMAL
    bool pressed = false;
    Vector2 textSize = MeasureTextEx(font, text, font.baseSize, 1);

    // Update control
    //--------------------------------------------------------------------
    if ((state < 3) && (forcedState < 0))
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = 2;    // PRESSED
            else state = 1;                                         // FOCUSED

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
        }
    }
            
    npInfo.sourceRec.x = 80*state;

    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    //DrawRectangleRec(bounds, GREEN);
    //DrawRectangleLinesEx(bounds, 4, DARKGREEN);
    DrawTextureNPatch(texNPatch, npInfo, bounds, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
    DrawTextEx(font, text, (Vector2){ bounds.x + bounds.width/2 - textSize.x/2, bounds.y + bounds.height/2 - textSize.y/2 + 4 }, font.baseSize, 1, GetColor(textColor[state]));
    //------------------------------------------------------------------
    
    return pressed;
}