/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Title Screen Functions Definitions (Init, Update, Draw, Unload)
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

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Title screen global variables
static int framesCounter;
static int finishScreen;
static int state;

static int scrollPositionX;

static int catPosX;
static int roombaPosX;

static float vsAlpha;
static float vsScale;

static Texture2D cat;
static Texture2D vs;
static Texture2D roomba;

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    // TODO: Initialize TITLE screen variables here!
    framesCounter = 0;
    finishScreen = 0;
    
    cat = LoadTexture("resources/title_cat.png");
    vs = LoadTexture("resources/title_vs.png");
    roomba = LoadTexture("resources/title_roomba.png");
    
    state = 0;
    catPosX = 1760;
    roombaPosX = -700;
    scrollPositionX = 0;
    
    vsAlpha = 0.0f;
    vsScale = 10.0f;
    
    PlayMusicStream(music);
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    scrollPositionX -= 5;
    if (scrollPositionX < -GetScreenWidth()) scrollPositionX = 0;

    if (state == 0)
    {
        catPosX -= 4;
        roombaPosX += 3;
        
        if (catPosX < (GetScreenWidth()/2 - cat.width/2)) catPosX = (GetScreenWidth()/2 - cat.width/2);
        if (roombaPosX > (GetScreenWidth()/2 - roomba.width/2)) roombaPosX = (GetScreenWidth()/2 - roomba.width/2);
        
        if ((catPosX == (GetScreenWidth()/2 - cat.width/2)) && (roombaPosX == (GetScreenWidth()/2 - roomba.width/2))) 
        {
            state = 1;
            framesCounter = 0;
        }
    }
    else if (state == 1)
    {
        framesCounter++;
        
        vsScale -= 0.1f;
        vsAlpha += 0.01f;
        
        if (vsScale < 1.0f) vsScale = 1.0f;
        if (vsAlpha > 1.0f) vsAlpha = 1.0f;
        
        if (framesCounter > 160)
        {
            state = 2;
            framesCounter = 0;
        }
    }
    else if (state == 2) framesCounter++;

    // Press enter or tap to change to GAMEPLAY screen
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    {
        //finishScreen = 1;   // OPTIONS
        finishScreen = 2;   // GAMEPLAY
        PlaySound(fxCoin);
    }
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    for (int i = 0; i < 64*2*2; i++)
    {
        DrawRectangle(64*i + scrollPositionX, 0, 64, GetScreenHeight(), (i%2 == 0)? GetColor(0xf3726dff) : GetColor(0xffcf6bff));
    }
    
    DrawTexture(cat, catPosX, 80, WHITE);
    DrawTexture(roomba, roombaPosX, 320, WHITE);
    
    if (state > 0)
    {
        DrawTexturePro(vs, (Rectangle){ 0, 0, vs.width, vs.height }, (Rectangle){ GetScreenWidth()/2, 300, vs.width*vsScale, vs.height*vsScale }, (Vector2){ vs.width/2*vsScale, vs.height/2*vsScale }, 0.0f, Fade(WHITE, vsAlpha));
    }

    if ((state == 2) && ((framesCounter/30)%2)) DrawTextEx(font2, "PRESS ENTER to START", (Vector2){ 340, 550 }, font2.baseSize, 2, WHITE);
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    UnloadTexture(cat);
    UnloadTexture(vs);
    UnloadTexture(roomba);
}

// Title Screen should finish?
int FinishTitleScreen(void)
{
    return finishScreen;
}