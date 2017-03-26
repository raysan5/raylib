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

// Decalre monsters
static Monster lamp;
static Monster arc;

static bool monsterHover;
static int monsterCheck;      // Identify checking monster

static const char message[256] = "YOUR PARENTS ARE GONE! TIME TO ESCAPE!\nTHE DOOR IS LOCKED... TURN ON THE LIGHTS! ;)";
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
void InitAtticScreen(void)
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
    
    background = LoadTexture("resources/textures/background_attic.png");
    
    // Initialize doors
    doorRight.position = (Vector2) { 1074, 140 };
    doorRight.facing = 2;
    doorRight.locked = true;
    doorRight.frameRec =(Rectangle) {((doors.width/3)*doorRight.facing), doors.height/2, doors.width/3, doors.height/2};
    doorRight.bound = (Rectangle) { doorRight.position.x, doorRight.position.y, doors.width/3, doors.height/2};
    doorRight.selected = false;

    // Monster init: lamp
    lamp.position = (Vector2){ 50, 316 };
    lamp.texture = LoadTexture("resources/textures/monster_lamp_left.png");
    lamp.currentFrame = 0;
    lamp.framesCounter = 0;
    lamp.numFrames = 4;
    lamp.bounds = (Rectangle){ lamp.position.x + 20, lamp.position.y, 90, 380 };
    lamp.frameRec = (Rectangle) { 0, 0, lamp.texture.width/lamp.numFrames, lamp.texture.height };
    lamp.selected = false;
    lamp.active = false;
    lamp.spooky = false;
    
    // Monster init: arc
    arc.position = (Vector2){ 760, 430 };
    arc.texture = LoadTexture("resources/textures/monster_arc.png");
    arc.currentFrame = 0;
    arc.framesCounter = 0;
    arc.numFrames = 4;
    arc.bounds = (Rectangle){ arc.position.x + 44, arc.position.y + 70, 220, 120 };
    arc.frameRec = (Rectangle) { 0, 0, arc.texture.width/arc.numFrames, arc.texture.height };
    arc.selected = false;
    arc.active = false;
    arc.spooky = true;
}

// Gameplay Screen Update logic
void UpdateAtticScreen(void)
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
        UpdateMonster(&arc);
    }
	
    // Check player hover monsters to interact
    if (((CheckCollisionRecs(player.bounds, lamp.bounds)) && !lamp.active) ||
        ((CheckCollisionRecs(player.bounds, arc.bounds)) && !arc.active)) monsterHover = true;
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
    
    // Monters logic: arc
    if ((CheckCollisionRecs(player.bounds, arc.bounds)) && !arc.active)
    {
        arc.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), arc.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 2;
        }
    }
    else arc.selected = false;
    
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
                if (arc.spooky) 
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                arc.active = true;
                arc.selected = false;
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
    
    if (IsKeyPressed('M'))
    {
        finishScreen = 1;
    }
}

// Gameplay Screen Draw logic
void DrawAtticScreen(void)
{
    DrawTexture(background, 0, 0, WHITE);
    
    // Draw monsters
	DrawMonster(lamp, 0);
    DrawMonster(arc, 0);
    
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
            DrawRectangleRec(arc.bounds, Fade(RED, 0.6f));
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
void UnloadAtticScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
    UnloadTexture(background);
	
    UnloadMonster(lamp);
    UnloadMonster(arc);
}

// Gameplay Screen should finish?
int FinishAtticScreen(void)
{
    return finishScreen;
}
