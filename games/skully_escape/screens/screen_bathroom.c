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
static Door doorRight;

// Decalre monst
static Monster lamp;
static Monster chair;
static Monster mirror;

static bool monsterHover = false;
static int monsterCheck = -1;      // Identify checking monster

static const char message[256] = "TRICK OR TREAT! WHO IS THE MOST BEAUTIFUL\nSKELETON IN THE WORLD?";
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
void InitBathroomScreen(void)
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
    
    background = LoadTexture("resources/textures/background_bathroom.png");
    
    // Reset Screen variables
    monsterHover = false;
    monsterCheck = -1;
    msgState = 0;
    msgCounter = 0;
    lettersCounter = 0;
    for (int i = 0; i < 256; i++) msgBuffer[i] = '\0';
    
    // Initialize doors
    doorRight.position = (Vector2) { 1070, 135 };
    doorRight.facing = 2;
    doorRight.locked = true;
    doorRight.frameRec =(Rectangle) {((doors.width/3)*doorRight.facing), doors.height/2, doors.width/3, doors.height/2};
    doorRight.bound = (Rectangle) { doorRight.position.x, doorRight.position.y, doors.width/3, doors.height/2};
    doorRight.selected = false;

    // Monster init: lamp
    lamp.position = (Vector2){ 35, 334 };
    lamp.texture = LoadTexture("resources/textures/monster_lamp_left.png");
    lamp.currentFrame = 0;
    lamp.framesCounter = 0;
    lamp.numFrames = 4;
    lamp.bounds = (Rectangle){ lamp.position.x + 20, lamp.position.y + 0, 90, 380};
    lamp.frameRec = (Rectangle) { 0, 0, lamp.texture.width/lamp.numFrames, lamp.texture.height };
    lamp.selected = false;
    lamp.active = false;
    lamp.spooky = true;
    
    // Monster init: mirror
    mirror.position = (Vector2){ 300, 200 };
    mirror.texture = LoadTexture("resources/textures/monster_mirror.png");
    mirror.currentFrame = 0;
    mirror.framesCounter = 0;
    mirror.numFrames = 4;
    mirror.bounds = (Rectangle){ mirror.position.x + 40, mirror.position.y + 20, 190, 200 };
    mirror.frameRec = (Rectangle) { 0, 0, mirror.texture.width/mirror.numFrames, mirror.texture.height };
    mirror.selected = false;
    mirror.active = false;
    mirror.spooky = false;
    
    // Monster init: chair
    chair.position = (Vector2){ 760, 430 };
    chair.texture = LoadTexture("resources/textures/monster_chair_right.png");
    chair.currentFrame = 0;
    chair.framesCounter = 0;
    chair.numFrames = 4;
    chair.bounds = (Rectangle){ chair.position.x + 30, chair.position.y + 30, 120, 190 };
    chair.frameRec = (Rectangle) { 0, 0, chair.texture.width/chair.numFrames, chair.texture.height };
    chair.selected = false;
    chair.active = false;
    chair.spooky = true;
}

// Gameplay Screen Update logic
void UpdateBathroomScreen(void)
{
    if (player.key)
    {
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
                else finishScreen = 1;
            }
        }
    }
        
    if (msgState > 2)
    {
        UpdatePlayer();
	
		// Monsters logic
        UpdateMonster(&lamp);
        UpdateMonster(&mirror);
        UpdateMonster(&chair);
    }
	
    // Check player hover monsters to interact
    if (((CheckCollisionRecs(player.bounds, lamp.bounds)) && !lamp.active) ||
        ((CheckCollisionRecs(player.bounds, mirror.bounds)) && !mirror.active) ||
        ((CheckCollisionRecs(player.bounds, chair.bounds)) && !chair.active)) monsterHover = true;
    else monsterHover = false;
    
    
    // Monters logic: lamp
    if ((CheckCollisionRecs(player.bounds, lamp.bounds)) && !lamp.active)
    {
        lamp.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), lamp.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 1;
        }
    }
    else lamp.selected = false;
    
    // Monters logic: mirror
    if ((CheckCollisionRecs(player.bounds, mirror.bounds)) && !mirror.active)
    {
        mirror.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), mirror.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 2;
        }
    }
    else mirror.selected = false;
        
    // Monters logic: chair
    if ((CheckCollisionRecs(player.bounds, chair.bounds)) && !chair.active)
    {
        chair.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), chair.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 3;
        }
    }
    else chair.selected = false;
    
    if (searching)
    {
        framesCounter++;
        
        if (framesCounter > 180)
        {
            if (monsterCheck == 1)
            {
                if (lamp.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                lamp.active = true;
                lamp.selected = false;
            }
            else if (monsterCheck == 2)
            {
                if (mirror.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                mirror.active = true;
                mirror.selected = false;
            }
            else if (monsterCheck == 3)
            {
                if (chair.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                chair.active = true;
                chair.selected = false;
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
void DrawBathroomScreen(void)
{
    DrawTexture(background, 0, 0, WHITE);
    
    // Draw monsters
	DrawMonster(lamp, 0);
    DrawMonster(mirror, 0);
    DrawMonster(chair, 0);
    
    // Draw door
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
            
            DrawRectangleRec(lamp.bounds, Fade(RED, 0.6f));
            DrawRectangleRec(mirror.bounds, Fade(RED, 0.6f));
            DrawRectangleRec(chair.bounds, Fade(RED, 0.6f));
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
void UnloadBathroomScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
    UnloadTexture(background);
	
    UnloadMonster(lamp);
    UnloadMonster(chair);
    UnloadMonster(mirror);
}

// Gameplay Screen should finish?
int FinishBathroomScreen(void)
{
    return finishScreen;
}
