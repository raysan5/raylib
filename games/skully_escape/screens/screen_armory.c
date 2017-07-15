/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
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
#include "../player.h"
#include "../monster.h"

#include <string.h>

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Gameplay screen global variables
static int framesCounter;
static int finishScreen;

static Texture2D background;

// Declare doors
static Door doorLeft;
static Door doorRight;

// Decalre monsters
static Monster blazon01;
static Monster blazon02;
static Monster blazon03;

static bool monsterHover = false;
static int monsterCheck = -1;      // Identify checking monster

static const char message[256] = "NO MORE TIPS...\nFOLLOW YOUR INSTINCT!";
static int msgPosX = 100;

static int msgState = 0;   // 0-writting, 1-wait, 2-choose
static int lettersCounter = 0;
static char msgBuffer[256] = { '\0' };
static int msgCounter = 0;

static bool searching = false;

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitArmoryScreen(void)
{
    ResetPlayer();
    
    // Reset Screen variables
    monsterHover = false;
    monsterCheck = -1;
    msgState = 0;
    msgCounter = 0;
    lettersCounter = 0;
    for (int i = 0; i < 256; i++) msgBuffer[i] = '\0';
    
    framesCounter = 0;
    finishScreen = 0;
    
    background = LoadTexture("resources/textures/background_armory.png");
    
    // Initialize doors
    doorLeft.position = (Vector2) { -50, 145 };
    doorLeft.facing = 0;
    doorLeft.locked = true;
    doorLeft.frameRec =(Rectangle) {((doors.width/3)*doorLeft.facing), doors.height/2, doors.width/3, doors.height/2};
    doorLeft.bound = (Rectangle) { doorLeft.position.x, doorLeft.position.y, doors.width/3, doors.height/2};
    doorLeft.selected = false;
    
    doorRight.position = (Vector2) { 1074, 140 };
    doorRight.facing = 2;
    doorRight.locked = true;
    doorRight.frameRec =(Rectangle) {((doors.width/3)*doorRight.facing), doors.height/2, doors.width/3, doors.height/2};
    doorRight.bound = (Rectangle) { doorRight.position.x, doorRight.position.y, doors.width/3, doors.height/2};
    doorRight.selected = false;

    // Monster init: blazon01
    blazon01.position = (Vector2){ 300, 260 };
    blazon01.texture = LoadTexture("resources/textures/monster_blazon01.png");
    blazon01.currentFrame = 0;
    blazon01.framesCounter = 0;
    blazon01.numFrames = 4;
    blazon01.bounds = (Rectangle){ blazon01.position.x, blazon01.position.y + 20, 160, 230 };
    blazon01.frameRec = (Rectangle) { 0, 0, blazon01.texture.width/blazon01.numFrames, blazon01.texture.height };
    blazon01.selected = false;
    blazon01.active = false;
    blazon01.spooky = true;
    
    // Monster init: blazon02
    blazon02.position = (Vector2){ 550, 260 };
    blazon02.texture = LoadTexture("resources/textures/monster_blazon02.png");
    blazon02.currentFrame = 0;
    blazon02.framesCounter = 0;
    blazon02.numFrames = 4;
    blazon02.bounds = (Rectangle){ blazon02.position.x, blazon02.position.y + 20, 160, 230 };
    blazon02.frameRec = (Rectangle) { 0, 0, blazon02.texture.width/blazon02.numFrames, blazon02.texture.height };
    blazon02.selected = false;
    blazon02.active = false;
    blazon02.spooky = true;
    
    // Monster init: blazon03
    blazon03.position = (Vector2){ 800, 260 };
    blazon03.texture = LoadTexture("resources/textures/monster_blazon03.png");
    blazon03.currentFrame = 0;
    blazon03.framesCounter = 0;
    blazon03.numFrames = 4;
    blazon03.bounds = (Rectangle){ blazon03.position.x, blazon03.position.y + 20, 160, 230 };
    blazon03.frameRec = (Rectangle) { 0, 0, blazon03.texture.width/blazon03.numFrames, blazon03.texture.height };
    blazon03.selected = false;
    blazon03.active = false;
    blazon03.spooky = false;
}

// Gameplay Screen Update logic
void UpdateArmoryScreen(void)
{
    if (player.key)
    {
        // Door: left
        if ((CheckCollisionPointRec(GetMousePosition(), doorLeft.bound)) || 
            (CheckCollisionRecs(player.bounds, doorLeft.bound))) doorLeft.selected = true; 
        else doorLeft.selected = false;
        
        if ((doorLeft.selected) && (CheckCollisionRecs(player.bounds, doorLeft.bound)))
        {
            if (((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && CheckCollisionPointRec(GetMousePosition(), doorLeft.bound)) || (IsKeyPressed(KEY_SPACE)))
            {
                if (doorLeft.locked)
                {
                    doorLeft.frameRec.y = 0;
                    doorLeft.locked = false;
                    PlaySound(sndDoor);
                }
                else finishScreen = 1;
            }
        }
        
        // Door: right
        if ((CheckCollisionPointRec(GetMousePosition(), doorRight.bound)) || 
            (CheckCollisionRecs(player.bounds, doorRight.bound))) doorRight.selected = true; 
        else doorRight.selected = false;
        
        if ((doorRight.selected) && (CheckCollisionRecs(player.bounds, doorRight.bound)))
        {
            if (((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && CheckCollisionPointRec(GetMousePosition(), doorRight.bound)) || (IsKeyPressed(KEY_SPACE)))
            {
                if (doorRight.locked)
                {
                    doorRight.frameRec.y = 0;
                    doorRight.locked = false;
                    PlaySound(sndDoor);
                }
                else finishScreen = 2;
            }
        }
    }
        
    if (msgState > 2)
    {
        UpdatePlayer();
	
		// Monsters logic
        UpdateMonster(&blazon01);
        UpdateMonster(&blazon02);
        UpdateMonster(&blazon03);
    }
	
    // Check player hover monsters to interact
    if (((CheckCollisionRecs(player.bounds, blazon01.bounds)) && !blazon01.active) ||
        ((CheckCollisionRecs(player.bounds, blazon02.bounds)) && !blazon02.active) ||
        ((CheckCollisionRecs(player.bounds, blazon03.bounds)) && !blazon03.active)) monsterHover = true;
    else monsterHover = false;
    
    // Monters logic: blazon01
    if ((CheckCollisionRecs(player.bounds, blazon01.bounds)) && !blazon01.active)
    {
        blazon01.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), blazon01.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 1;
        }
    }
    else blazon01.selected = false;
    
    // Monters logic: blazon02
    if ((CheckCollisionRecs(player.bounds, blazon02.bounds)) && !blazon02.active)
    {
        blazon02.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), blazon02.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 2;
        }
    }
    else blazon02.selected = false;
    
    // Monters logic: blazon03
    if ((CheckCollisionRecs(player.bounds, blazon03.bounds)) && !blazon03.active)
    {
        blazon03.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), blazon03.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 3;
        }
    }
    else blazon03.selected = false;
    
    if (searching)
    {
        framesCounter++;
        
        if (framesCounter > 180)
        {
            if (monsterCheck == 1)
            {
                if (blazon01.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                blazon01.active = true;
                blazon01.selected = false;
            }
            else if (monsterCheck == 2)
            {
                if (blazon02.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                blazon02.active = true;
                blazon02.selected = false;
            }
            else if (monsterCheck == 3)
            {
                if (blazon03.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                blazon03.active = true;
                blazon03.selected = false;
            }
  
            searching = false;
            framesCounter = 0;
        }
    }
    
    // Text animation
    framesCounter++;
    
    if ((framesCounter%2) == 0) lettersCounter++;

    if (msgState == 0)
    {
        if (lettersCounter <= (int)strlen(message)) strncpy(msgBuffer, message, lettersCounter);
        else
        {
            for (int i = 0; i < (int)strlen(msgBuffer); i++) msgBuffer[i] = '\0';

            lettersCounter = 0;
            msgState = 1;
        }
        
        if (IsKeyPressed(KEY_ENTER)) msgState = 1;
    }
    else if (msgState == 1)
    {
        msgCounter++;
        
        if ((IsKeyPressed(KEY_ENTER)) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
        {
            msgState = 2;
            msgCounter = 0;
        }
    }
    else if (msgState == 2)
    {
        msgCounter++;
        
        if (msgCounter > 180) msgState = 3;
    }
    else msgCounter++;
}

// Gameplay Screen Draw logic
void DrawArmoryScreen(void)
{
    DrawTexture(background, 0, 0, WHITE);
    
    // Draw monsters
	DrawMonster(blazon01, 0);
    DrawMonster(blazon02, 0);
    DrawMonster(blazon03, 0);
    
    // Draw door
    if (doorLeft.selected) DrawTextureRec(doors, doorLeft.frameRec, doorLeft.position, GREEN);
    else DrawTextureRec(doors, doorLeft.frameRec, doorLeft.position, WHITE);
    
    if (doorRight.selected) DrawTextureRec(doors, doorRight.frameRec, doorRight.position, GREEN);
    else DrawTextureRec(doors, doorRight.frameRec, doorRight.position, WHITE);
    
    // Draw messsages
    if (msgState < 2) DrawRectangle(0, 40, GetScreenWidth(), 200, Fade(LIGHTGRAY, 0.5f));
    else if (msgState == 2) DrawRectangle(0, 80, GetScreenWidth(), 100, Fade(LIGHTGRAY, 0.5f));

    if (msgState == 0)
    {
        DrawTextEx(font, msgBuffer, (Vector2){ msgPosX, 80 }, font.baseSize, 2, WHITE);
    }
    else if (msgState == 1)
    {
        DrawTextEx(font, message, (Vector2){ msgPosX, 80 }, font.baseSize, 2, WHITE);
        
        if ((msgCounter/30)%2) DrawText("PRESS ENTER or CLICK", GetScreenWidth() - 280, 200, 20, BLACK);
    }
    else if (msgState == 2)
    {
        if ((msgCounter/30)%2)
        {
            DrawTextEx(font, "CHOOSE WISELY!", (Vector2){ 300, 95 }, font.baseSize*2, 2, WHITE);
            
            DrawRectangleRec(blazon01.bounds, Fade(RED, 0.6f));
            DrawRectangleRec(blazon02.bounds, Fade(RED, 0.6f));
            DrawRectangleRec(blazon03.bounds, Fade(RED, 0.6f));
        }
    }
    else
    {
        if ((monsterHover) && ((msgCounter/30)%2))
        {
            DrawRectangle(0, 0, GetScreenWidth(), 50, Fade(LIGHTGRAY, 0.5f));
            DrawText("PRESS SPACE or CLICK to INTERACT", 420, 15, 20, BLACK);
        }
    }

    DrawPlayer();       // NOTE: Also draws mouse pointer!
}

// Gameplay Screen Unload logic
void UnloadArmoryScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
    UnloadTexture(background);
	
    UnloadMonster(blazon01);
    UnloadMonster(blazon02);
    UnloadMonster(blazon03);
}

// Gameplay Screen should finish?
int FinishArmoryScreen(void)
{
    return finishScreen;
}
