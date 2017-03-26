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

// Decalre monsters
static Monster lamp;
static Monster chair;
static Monster picture;
static Monster arc;

static bool monsterHover = false;
static int monsterCheck = -1;      // Identify checking monster

static const char message[256] = "HAS LEGS BUT CAN NOT WALK...\nSEARCH FOR IT TO OPEN THE DOOR!";
static int msgPosX = 100;

static int msgState = 0;   // 0-writting, 1-wait, 2-choose
static int lettersCounter = 0;
static char msgBuffer[256] = { '\0' };
static int msgCounter = 0;

static bool searching = false;

static int scroll = 0;

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitAisle02Screen(void)
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
    
    background = LoadTexture("resources/textures/background_aisle02.png");
    
    scroll = player.position.x - 200;
    
    // Initialize doors
    doorLeft.position = (Vector2) { -10, 136 };
    doorLeft.facing = 0;
    doorLeft.locked = true;
    doorLeft.frameRec =(Rectangle) {((doors.width/3)*doorLeft.facing), doors.height/2, doors.width/3, doors.height/2};
    doorLeft.bound = (Rectangle) { doorLeft.position.x, doorLeft.position.y, doors.width/3, doors.height/2};
    doorLeft.selected = false;

    // Monster init: lamp
    lamp.position = (Vector2){ 1520, 300 };
    lamp.texture = LoadTexture("resources/textures/monster_lamp_right.png");
    lamp.currentFrame = 0;
    lamp.framesCounter = 0;
    lamp.numFrames = 4;
    lamp.bounds = (Rectangle){ lamp.position.x + 200, lamp.position.y, 90, 380 };
    lamp.frameRec = (Rectangle) { 0, 0, lamp.texture.width/lamp.numFrames, lamp.texture.height };
    lamp.selected = false;
    lamp.active = false;
    lamp.spooky = true;
    
    // Monster init: chair
    chair.position = (Vector2){ 1400, 404 };
    chair.texture = LoadTexture("resources/textures/monster_chair_right.png");
    chair.currentFrame = 0;
    chair.framesCounter = 0;
    chair.numFrames = 4;
    chair.bounds = (Rectangle){ chair.position.x + 50, chair.position.y + 30, 120, 190 };
    chair.frameRec = (Rectangle) { 0, 0, chair.texture.width/chair.numFrames, chair.texture.height };
    chair.selected = false;
    chair.active = false;
    chair.spooky = false;
    
    // Monster init: picture
    picture.position = (Vector2){ 837, 162 };
    picture.texture = LoadTexture("resources/textures/monster_picture.png");
    picture.currentFrame = 0;
    picture.framesCounter = 0;
    picture.numFrames = 4;
    picture.bounds = (Rectangle){ picture.position.x + 44, picture.position.y, 174, 264 };
    picture.frameRec = (Rectangle) { 0, 0, picture.texture.width/picture.numFrames, picture.texture.height };
    picture.selected = false;
    picture.active = false;
    picture.spooky = true;
    
    // Monster init: arc
    arc.position = (Vector2){ 388, 423 };
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
void UpdateAisle02Screen(void)
{
    // Update doors bounds
    doorLeft.bound.x = doorLeft.position.x - scroll;

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
    }
        
    if (msgState > 2)
    {
        UpdatePlayer();
	
		// Monsters logic
        UpdateMonster(&lamp);
        UpdateMonster(&chair);
        UpdateMonster(&picture);
        UpdateMonster(&arc);
    }
    
    // Update monster bounds
    lamp.bounds.x = lamp.position.x + 200 - scroll;
    chair.bounds.x = chair.position.x + 50 - scroll;
    picture.bounds.x = picture.position.x + 44 - scroll;
    arc.bounds.x = arc.position.x + 44 - scroll;
	
    // Check player hover monsters to interact
    if (((CheckCollisionRecs(player.bounds, lamp.bounds)) && !lamp.active) ||
        ((CheckCollisionRecs(player.bounds, chair.bounds)) && !chair.active) ||
        ((CheckCollisionRecs(player.bounds, picture.bounds)) && !picture.active) ||
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
            
            monsterCheck = 2;
        }
    }
    else chair.selected = false;
    
    // Monters logic: picture
    if ((CheckCollisionRecs(player.bounds, picture.bounds)) && !picture.active)
    {
        picture.selected = true;
        
        if ((IsKeyPressed(KEY_SPACE)) || 
            ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && (CheckCollisionPointRec(GetMousePosition(), picture.bounds))))
        {
            SearchKeyPlayer();
            searching = true;
            framesCounter = 0;
            
            monsterCheck = 3;
        }
    }
    else picture.selected = false;
    
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
            
            monsterCheck = 4;
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
                if (chair.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                chair.active = true;
                chair.selected = false;
            }
            else if (monsterCheck == 3)
            {
                if (picture.spooky)
                {
                    ScarePlayer();
                    PlaySound(sndScream);
                }
                else FindKeyPlayer();
                
                picture.active = true;
                picture.selected = false;
            }
            else if (monsterCheck == 4)
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
        
    if (player.position.x > 200)
    {
        scroll = player.position.x - 200;
        
        if (scroll > 620) scroll = 620;
    }
}

// Gameplay Screen Draw logic
void DrawAisle02Screen(void)
{
    DrawTexture(background, -scroll, 0, WHITE);
    
    // Draw monsters
	DrawMonster(lamp, scroll);
    DrawMonster(arc, scroll);
    DrawMonster(picture, scroll);
    DrawMonster(chair, scroll);
    
    // Draw door
    Vector2 doorScrollPos = { doorLeft.position.x - scroll, doorLeft.position.y };
    
    if (doorLeft.selected) DrawTextureRec(doors, doorLeft.frameRec, doorScrollPos, GREEN);
    else DrawTextureRec(doors, doorLeft.frameRec, doorScrollPos, WHITE);
    
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
            DrawRectangleRec(chair.bounds, Fade(RED, 0.6f));
            DrawRectangleRec(picture.bounds, Fade(RED, 0.6f));
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
void UnloadAisle02Screen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
    UnloadTexture(background);
	
    UnloadMonster(lamp);
    UnloadMonster(chair);
    UnloadMonster(picture);
    UnloadMonster(arc);
}

// Gameplay Screen should finish?
int FinishAisle02Screen(void)
{
    return finishScreen;
}
