/**********************************************************************************************
*
*   raylib - Koala Seasons game
*
*   Ending Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
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

#include <stdio.h>

#include "atlas01.h"
#include "atlas02.h"

typedef enum { DELAY, SEASONS, LEAVES, KILLS, REPLAY } EndingCounter;

typedef struct {
    Vector2 position;
    Vector2 speed;
    float rotation;
    float size;
    Color color;
    float alpha;
    bool active;
} Particle;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Ending screen global variables
static EndingCounter endingCounter;

static int framesCounter;
static int finishScreen;
static int framesKillsCounter;

static Rectangle playButton; 
static Rectangle shopButton; 
static Rectangle trophyButton; 
static Rectangle shareButton; 

static Color buttonPlayColor;
static Color buttonShopColor;
static Color buttonTrophyColor;
static Color buttonShareColor;
static Color backgroundColor;

static int currentScore;
static int seasonsCounter;
static int currentLeavesEnding;
static int finalYears;
static int replayTimer;
static int yearsElapsed;
static int initRotation;

static float clockRotation;
static float finalRotation;

static bool replaying;
static bool active[MAX_KILLS];

static char initMonthText[32];
static char finalMonthText[32];

static Particle leafParticles[20];

static int drawTimer;

// Death texts
const char textOwl01[32] = "Turned into a pretty";
const char textOwl02[32] = "owl pellet";
const char textDingo01[32] = "A dingo took your life";
const char textFire01[32] = "Kissed by fire";
const char textSnake01[32] = "Digested alive by a";
const char textSnake02[32] = "big snake";
const char textNaturalDeath01[32] = "LIFE KILLED YOU";
const char textBee01[32] = "You turn out to be";
const char textBee02[32] = "allergic to bee sting";
const char textEagle[32] = "KOALA IS DEAD :(";

static float LinearEaseIn(float t, float b, float c, float d) { return c*t/d + b; }

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    framesCounter = -10;
    finishScreen = 0;
    drawTimer = 15;
    replayTimer = 0;
    replaying = false;
    finalYears = initYears + (seasons/4);
    yearsElapsed = seasons/4;
    
    playButton = (Rectangle){ GetScreenWidth()*0.871, GetScreenHeight()*0.096, 123, 123};
    shopButton = (Rectangle){ GetScreenWidth()*0.871, GetScreenHeight()*0.303, 123, 123};
    trophyButton = (Rectangle){ GetScreenWidth()*0.871, GetScreenHeight()*0.513, 123, 123};
    shareButton = (Rectangle){ GetScreenWidth()*0.871, GetScreenHeight()*0.719, 123, 123};
    
    buttonPlayColor = WHITE;
    buttonShopColor = WHITE;
    buttonTrophyColor = WHITE;
    buttonShareColor = WHITE;
    
    currentScore = 0;
    seasonsCounter = 0;
    currentLeavesEnding = 0;
    
    endingCounter = DELAY;
    
    backgroundColor = (Color){ 176, 167, 151, 255};
    
    for (int j = 0; j < 20; j++)
    {
        leafParticles[j].active = false;
        leafParticles[j].position = (Vector2){ GetRandomValue(-20, 20), GetRandomValue(-20, 20) };
        leafParticles[j].speed = (Vector2){ (float)GetRandomValue(-500, 500)/100, (float)GetRandomValue(-500, 500)/100 };
        leafParticles[j].size = (float)GetRandomValue(3, 10)/5;
        leafParticles[j].rotation = GetRandomValue(0, 360);
        leafParticles[j].color = WHITE;
        leafParticles[j].alpha = 1;
    }
    
    // Seasons death texts
    if (initSeason == 0)
    {
        sprintf(initMonthText, "SUMMER");
        clockRotation = 225;
        initRotation = 225;
    }
    else if (initSeason == 1)
    {
        sprintf(initMonthText, "AUTUMN");
        clockRotation = 135;
        initRotation = 135;
    }
    else if (initSeason == 2)
    {
        sprintf(initMonthText, "WINTER");
        clockRotation = 45;
        initRotation = 45;  
    }
    else if (initSeason == 3)
    {
        sprintf(initMonthText, "SPRING");
        clockRotation = 315;
        initRotation = 315; 
    }
    
    if (currentSeason == 0)
    {
        sprintf(finalMonthText, "SUMMER");
        finalRotation = 225 + 360*yearsElapsed;
    }
    else if (currentSeason == 1)
    {    
        sprintf(finalMonthText, "AUTUMN");
        finalRotation = 135 + 360*yearsElapsed;
    }
    else if (currentSeason == 2)
    {
        sprintf(finalMonthText, "WINTER");
        finalRotation = 45 + 360*yearsElapsed;
    }
    else if (currentSeason == 3)
    {
        sprintf(finalMonthText, "SPRING");
        finalRotation = 315 + 360*yearsElapsed;
    }

    for (int i = 0; i < MAX_KILLS; i++) active[i] = false;
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    framesCounter += 1*TIME_FACTOR;
  
    switch (endingCounter)
    {
        case DELAY:
        {
            if(framesCounter >= 10)
            {
                endingCounter = SEASONS;
                framesCounter = 0;
            }
            
        } break;
        case SEASONS:
        {
            if (seasons > 0)
            {
                seasonsCounter = (int)LinearEaseIn((float)framesCounter, 0.0f, (float)(seasons), 90.0f);
                clockRotation = LinearEaseIn((float)framesCounter, (float)initRotation, (float)-(finalRotation - initRotation), 90.0f);
                
                if (framesCounter >= 90)
                {
                    endingCounter = LEAVES;
                    framesCounter = 0;
                }
            }
            else endingCounter = LEAVES;
            
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
            if (IsGestureDetected(GESTURE_TAP))
            {
                seasonsCounter = seasons;
                clockRotation = finalRotation;
                framesCounter = 0;
                endingCounter = LEAVES;
            }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
            if (IsKeyPressed(KEY_ENTER))
            {
                seasonsCounter = seasons;
                clockRotation = finalRotation;
                framesCounter = 0;
                endingCounter = LEAVES;
            }
#endif
        } break;
        case LEAVES: 
        {
            if (currentLeaves > 0)
            {
                if (currentLeavesEnding == currentLeaves)
                {
                    endingCounter = KILLS;
                    framesCounter = 0;
                }
                else if (currentLeavesEnding < currentLeaves)
                {
                    if (framesCounter >= 4)
                    {
                        currentLeavesEnding += 1;
                        framesCounter = 0;
                    }
                    
                    for (int i = 0; i < 20; i++)
                    {
                        if (!leafParticles[i].active)
                        {
                            leafParticles[i].position = (Vector2){ GetScreenWidth()*0.46, GetScreenHeight()*0.32};
                            leafParticles[i].alpha = 1.0f;
                            leafParticles[i].active = true;
                        }
                    }
                }
            }
            else endingCounter = KILLS;       

#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
            if (IsGestureDetected(GESTURE_TAP))
            {
                currentLeavesEnding = currentLeaves;
                framesCounter = 0;
                endingCounter = KILLS;
            }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
            if (IsKeyPressed(KEY_ENTER))
            {
                currentLeavesEnding = currentLeaves;
                framesCounter = 0;
                endingCounter = KILLS;
            }  
#endif
        } break;
        case KILLS:
        {
            if (score > 0)
            {
                if (framesCounter <= 90 && !replaying)
                {
                    currentScore = (int)LinearEaseIn((float)framesCounter, 0.0f, (float)(score), 90.0f);
                }
                
                framesKillsCounter += 1*TIME_FACTOR;
                
                for (int i = 0; i < MAX_KILLS; i++)
                {
                    if (framesKillsCounter >= drawTimer && active[i] == false)
                    {
                        active[i] = true;
                        framesKillsCounter = 0;
                    }
                }
                
                if (framesCounter >= 90)
                {
                    endingCounter = REPLAY;
                    framesCounter = 0;
                }
            }
            else endingCounter = REPLAY;   
            
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
            if (IsGestureDetected(GESTURE_TAP))
            {
               currentScore = score;
               framesCounter = 0;
               for (int i = 0; i < MAX_KILLS; i++) active[i] = true;
               endingCounter = REPLAY;
            }
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
            if (IsKeyPressed(KEY_ENTER))
            {
               currentScore = score;
               framesCounter = 0;
               for (int i = 0; i < MAX_KILLS; i++) active[i] = true;
               endingCounter = REPLAY;
            }
#endif
        } break;
        case REPLAY:
        {
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
            if (IsGestureDetected(GESTURE_TAP)) replaying = true;
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
            if (IsKeyPressed(KEY_ENTER)) replaying = true;
#endif
            if (replaying)
            {
                replayTimer += 1*TIME_FACTOR;
                
                if (replayTimer >= 30)
                {
                    finishScreen = 1;
                    initSeason = GetRandomValue(0, 3);
                }
                
                buttonPlayColor = GOLD;
            }
        } break;
    }
    
    for (int i = 0; i < 20; i++)
    {
        if (leafParticles[i].active == true)
        {
            leafParticles[i].position.x +=  leafParticles[i].speed.x;
            leafParticles[i].position.y +=  leafParticles[i].speed.y;
            leafParticles[i].rotation += 6;
            leafParticles[i].alpha -= 0.03f;
            leafParticles[i].size -= 0.004;

            if (leafParticles[i].size <= 0) leafParticles[i].size = 0.0f;

            if (leafParticles[i].alpha <= 0)
            {
                leafParticles[i].alpha = 0.0f;
                leafParticles[i].active = false;
            }               
        }
    }

    // Buttons logic
#if (defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB))
    if ((IsGestureDetected(GESTURE_TAP)) && CheckCollisionPointRec(GetTouchPosition(0), playButton))
    {
        endingCounter = REPLAY;
        replaying = true;
    }
    
#elif (defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB))
    if (CheckCollisionPointRec(GetMousePosition(), playButton)) 
    {
        buttonPlayColor = GOLD;  
        if (IsMouseButtonPressed(0)) 
        {
            endingCounter = REPLAY;
            replaying = true;
        }
    }       
    else buttonPlayColor = WHITE;

    if (CheckCollisionPointRec(GetMousePosition(), shopButton)) buttonShopColor = GOLD;
    else buttonShopColor = WHITE;
    
    if (CheckCollisionPointRec(GetMousePosition(), trophyButton)) buttonTrophyColor = GOLD;
    else buttonTrophyColor = WHITE;
    
    if (CheckCollisionPointRec(GetMousePosition(), shareButton)) buttonShareColor = GOLD;
    else buttonShareColor = WHITE;
#endif
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    for (int x = 0; x < 15; x++)
    {
        DrawTextureRec(atlas02, ending_background, (Vector2){ending_background.width*(x%5), ending_background.height*(x/5)}, backgroundColor);
    }
    
    // Frames and backgrounds
    DrawTexturePro(atlas01, ending_plate_frame, (Rectangle){GetScreenWidth()*0.042, GetScreenHeight()*0.606, ending_plate_frame.width, ending_plate_frame.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_paint_back, (Rectangle){GetScreenWidth()*0.133, GetScreenHeight()*0.097, ending_paint_back.width, ending_paint_back.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    if (killer == 0) DrawTexturePro(atlas01, ending_paint_koalafire, (Rectangle){GetScreenWidth()*0.145, GetScreenHeight()*0.171, ending_paint_koalafire.width, ending_paint_koalafire.height}, (Vector2){ 0, 0}, 0, WHITE);
    else if (killer == 1) DrawTexturePro(atlas01, ending_paint_koalasnake, (Rectangle){GetScreenWidth()*0.145, GetScreenHeight()*0.171, ending_paint_koalasnake.width, ending_paint_koalasnake.height}, (Vector2){ 0, 0}, 0, WHITE);
    else if (killer == 2) DrawTexturePro(atlas01, ending_paint_koaladingo, (Rectangle){GetScreenWidth()*0.145, GetScreenHeight()*0.171, ending_paint_koaladingo.width, ending_paint_koaladingo.height}, (Vector2){ 0, 0}, 0, WHITE);
    else if (killer == 3) DrawTexturePro(atlas01, ending_paint_koalaowl, (Rectangle){GetScreenWidth()*0.2, GetScreenHeight()*0.3, ending_paint_koalaowl.width, ending_paint_koalaowl.height}, (Vector2){ 0, 0}, 0, WHITE);
    else if (killer == 4) DrawTexturePro(atlas01, ending_paint_koalageneric, (Rectangle){GetScreenWidth()*0.133, GetScreenHeight()*0.171, ending_paint_koalageneric.width, ending_paint_koalageneric.height}, (Vector2){ 0, 0}, 0, WHITE);
    else if (killer == 5) DrawTexturePro(atlas01, ending_paint_koalabee, (Rectangle){GetScreenWidth()*0.145, GetScreenHeight()*0.171, ending_paint_koalabee.width, ending_paint_koalabee.height}, (Vector2){ 0, 0}, 0, WHITE);
    else if (killer == 6) DrawTexturePro(atlas01, ending_paint_koalaeagle, (Rectangle){GetScreenWidth()*0.145, GetScreenHeight()*0.171, ending_paint_koalaeagle.width, ending_paint_koalaeagle.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    DrawTexturePro(atlas01, ending_paint_frame, (Rectangle){GetScreenWidth()*0.102, GetScreenHeight()*0.035, ending_paint_frame.width, ending_paint_frame.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    // UI Score planks
    DrawTexturePro(atlas01, ending_score_planksmall, (Rectangle){GetScreenWidth()*0.521, GetScreenHeight()*0.163, ending_score_planksmall.width, ending_score_planksmall.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_score_planklarge, (Rectangle){GetScreenWidth()*0.415, GetScreenHeight()*0.303, ending_score_planklarge.width, ending_score_planklarge.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_score_planksmall, (Rectangle){GetScreenWidth()*0.521, GetScreenHeight()*0.440, ending_score_planksmall.width, ending_score_planksmall.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    // UI Score icons and frames
    DrawTexturePro(atlas01, ending_score_seasonicon, (Rectangle){GetScreenWidth()*0.529, GetScreenHeight()*0.096, ending_score_seasonicon.width, ending_score_seasonicon.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_score_seasonneedle, (Rectangle){GetScreenWidth()*0.579, GetScreenHeight()*0.189, ending_score_seasonneedle.width, ending_score_seasonneedle.height}, (Vector2){ending_score_seasonneedle.width/2, ending_score_seasonneedle.height*0.9}, clockRotation, WHITE);
    DrawTexturePro(atlas01, ending_score_frame, (Rectangle){GetScreenWidth()*0.535, GetScreenHeight()*0.11, ending_score_frame.width, ending_score_frame.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    DrawTexturePro(atlas01, ending_score_frameback, (Rectangle){GetScreenWidth()*0.430, GetScreenHeight()*0.246, ending_score_frameback.width, ending_score_frameback.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_score_frame, (Rectangle){GetScreenWidth()*0.429, GetScreenHeight()*0.244, ending_score_frame.width, ending_score_frame.height}, (Vector2){ 0, 0}, 0, WHITE);
   
    for (int i = 0; i < 20; i++)
    {
        if (leafParticles[i].active)
        {                
            DrawTexturePro(atlas01, particle_ecualyptusleaf,
                          (Rectangle){ leafParticles[i].position.x, leafParticles[i].position.y, particle_ecualyptusleaf.width*leafParticles[i].size, particle_ecualyptusleaf.height*leafParticles[i].size },
                          (Vector2){ particle_ecualyptusleaf.width/2*leafParticles[i].size, particle_ecualyptusleaf.height/2*leafParticles[i].size }, leafParticles[i].rotation, Fade(WHITE,leafParticles[i].alpha));        
        }
    }
    
    DrawTexturePro(atlas01, ending_score_leavesicon, (Rectangle){GetScreenWidth()*0.421, GetScreenHeight()*0.228, ending_score_leavesicon.width, ending_score_leavesicon.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    DrawTexturePro(atlas01, ending_score_frameback, (Rectangle){GetScreenWidth()*0.536, GetScreenHeight()*0.383, ending_score_frameback.width, ending_score_frameback.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_score_frame, (Rectangle){GetScreenWidth()*0.535, GetScreenHeight()*0.383, ending_score_frame.width, ending_score_frame.height}, (Vector2){ 0, 0}, 0, WHITE);
    DrawTexturePro(atlas01, ending_score_enemyicon, (Rectangle){GetScreenWidth()*0.538, GetScreenHeight()*0.414, ending_score_enemyicon.width, ending_score_enemyicon.height}, (Vector2){ 0, 0}, 0, WHITE);
    
    // UI Buttons
    DrawTexturePro(atlas01, ending_button_replay, (Rectangle){GetScreenWidth()*0.871, GetScreenHeight()*0.096, ending_button_replay.width, ending_button_replay.height}, (Vector2){ 0, 0}, 0, buttonPlayColor);
    DrawTexturePro(atlas01, ending_button_shop, (Rectangle){GetScreenWidth()*0.871, GetScreenHeight()*0.303, ending_button_shop.width, ending_button_shop.height}, (Vector2){ 0, 0}, 0, buttonShopColor);
    DrawTexturePro(atlas01, ending_button_trophy, (Rectangle){GetScreenWidth()*0.871, GetScreenHeight()*0.513, ending_button_trophy.width, ending_button_trophy.height}, (Vector2){ 0, 0}, 0, buttonTrophyColor);
    DrawTexturePro(atlas01, ending_button_share, (Rectangle){GetScreenWidth()*0.871, GetScreenHeight()*0.719, ending_button_share.width, ending_button_share.height}, (Vector2){ 0, 0}, 0, buttonShareColor);
    
    DrawTextEx(font, FormatText("%03i", seasonsCounter), (Vector2){ GetScreenWidth()*0.73f, GetScreenHeight()*0.14f }, font.baseSize, 1, WHITE);
    DrawTextEx(font, FormatText("%03i", currentLeavesEnding), (Vector2){ GetScreenWidth()*0.73f, GetScreenHeight()*0.29f }, font.baseSize, 1, WHITE);
    DrawTextEx(font, FormatText("%04i", currentScore), (Vector2){ GetScreenWidth()*0.715f, GetScreenHeight()*0.426f }, font.baseSize, 1, WHITE);
    
    DrawTextEx(font, FormatText("%s %i - %s %i", initMonthText, initYears, finalMonthText, finalYears), (Vector2){ GetScreenWidth()*0.1f, GetScreenHeight()*0.7f }, font.baseSize/2.0f, 1, WHITE);
    
    for (int i = 0; i < MAX_KILLS; i++)
    {
        if (active[i])
        {
            switch (killHistory[i])
            {
                case 1: DrawTextureRec(atlas01, ending_plate_headsnake, (Vector2){GetScreenWidth()*0.448 + ending_plate_headsnake.width*(i%10), GetScreenHeight()*0.682  + (GetScreenHeight()*0.055)*(i/10)}, WHITE); break;
                case 2: DrawTextureRec(atlas01, ending_plate_headdingo, (Vector2){GetScreenWidth()*0.448 + ending_plate_headdingo.width*(i%10), GetScreenHeight()*0.682  + (GetScreenHeight()*0.055)*(i/10)}, WHITE); break;
                case 3: DrawTextureRec(atlas01, ending_plate_headowl, (Vector2){GetScreenWidth()*0.448 + ending_plate_headowl.width*(i%10), GetScreenHeight()*0.682  + (GetScreenHeight()*0.055)*(i/10)}, WHITE); break;
                case 4: DrawTextureRec(atlas01, ending_plate_headbee, (Vector2){GetScreenWidth()*0.448 + ending_plate_headbee.width*(i%10), GetScreenHeight()*0.682  + (GetScreenHeight()*0.055)*(i/10)}, WHITE); break;
                case 5: DrawTextureRec(atlas01, ending_plate_headeagle, (Vector2){GetScreenWidth()*0.448 + ending_plate_headeagle.width*(i%10), GetScreenHeight()*0.682  + (GetScreenHeight()*0.055)*(i/10)}, WHITE); break;
                default: break;
            }
        }
    }
    
/*
    DrawText(FormatText("KOALA IS DEAD :("), GetScreenWidth()/2 -  MeasureText("YOU'RE DEAD   ", 60)/2, GetScreenHeight()/3, 60, RED);
    DrawText(FormatText("Score: %02i - HiScore: %02i", score, hiscore),GetScreenWidth()/2 -  MeasureText("Score: 00 - HiScore: 00", 60)/2, GetScreenHeight()/3 +100, 60, RED);
    DrawText(FormatText("You lived: %02i years", years),GetScreenWidth()/2 -  MeasureText("You lived: 00", 60)/2 + 60, GetScreenHeight()/3 +200, 30, RED);
    DrawText(FormatText("%02s killed you", killer),GetScreenWidth()/2 -  MeasureText("killer killed you", 60)/2 + 90, GetScreenHeight()/3 +270, 30, RED);
    //DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5));
*/
    
    //DrawTextEx(font, FormatText("%02s", killer), (Vector2){ GetScreenWidth()*0.08, GetScreenHeight()*0.78 }, font.baseSize/2, 1, WHITE);
    if (killer == 0) DrawTextEx(font, textFire01, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
    else if (killer == 2) DrawTextEx(font, textDingo01, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
    else if (killer == 1) 
    {
        DrawTextEx(font, textSnake01, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
        DrawTextEx(font, textSnake02, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.83f }, font.baseSize/2.0f, 1, WHITE);
    }
    else if (killer == 3) 
    {
        DrawTextEx(font, textOwl01, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
        DrawTextEx(font, textOwl02, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.83f }, font.baseSize/2.0f, 1, WHITE);
    }
    else if (killer == 4) DrawTextEx(font, textNaturalDeath01, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
    else if (killer == 5) 
    {
        DrawTextEx(font, textBee01, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
        DrawTextEx(font, textBee02, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.83f }, font.baseSize/2.0f, 1, WHITE);
    }
    else if (killer == 6) DrawTextEx(font, textEagle, (Vector2){ GetScreenWidth()*0.13f, GetScreenHeight()*0.78f }, font.baseSize/2.0f, 1, WHITE);
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
    // ...
}

// Ending Screen should finish?
int FinishEndingScreen(void)
{
    return finishScreen;
}