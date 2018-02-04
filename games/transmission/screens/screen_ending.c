/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Ending Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
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
#include <stdlib.h>

#define MAX_TITLE_CHAR 128
#define MAX_SUBTITLE_CHAR 256

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static char *codingWords[MAX_CODING_WORDS] = { 
    "pollo\0", 
    "conejo\0", 
    "huevo\0",
    "nido\0", 
    "aire\0", 
    "armario\0", 
    "agujero\0",
    "platano\0",
    "pastel\0",
    "mercado\0",
    "raton\0",
    "melon\0",
};

// Ending screen global variables
static int framesCounter;
static int finishScreen;

static Texture2D texBackground;
static Texture2D texNewspaper;
static Texture2D texVignette;

static Sound fxNews;

static float rotation = 0.1f;
static float scale = 0.05f;
static int state = 0;

static Mission *missions = NULL;

static bool showResults = false;

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;
    
    rotation = 0.1f;
    scale = 0.05f;
    state = 0;
    
    texBackground = LoadTexture("resources/textures/ending_background.png");
    texVignette = LoadTexture("resources/textures/message_vignette.png");
    
    fxNews = LoadSound("resources/audio/fx_batman.ogg");
    
    // TODO: Check game results!
    missions = LoadMissions("resources/missions.txt");
    int wordsCount = missions[currentMission].wordsCount;
    TraceLog(LOG_WARNING, "Words count %i", wordsCount);
    
    char title[MAX_TITLE_CHAR] = "\0";
    //char subtitle[MAX_SUBTITLE_CHAR] = "\0";
    
    char *ptrTitle = title;
    int len = 0;
    
    for (int i = 0; i < wordsCount; i++)
    {
        if (messageWords[i].id == missions[currentMission].sols[i])
        {
            len = strlen(messageWords[i].text);
            strncpy(ptrTitle, messageWords[i].text, len);
            ptrTitle += len;
            
            // title[len] = ' ';
            // len++;
            // ptrTitle++;
        }
        else
        {
            TraceLog(LOG_WARNING, "Coding word: %s", codingWords[messageWords[i].id]);
            len = strlen(codingWords[messageWords[i].id]);
            TraceLog(LOG_WARNING, "Lenght: %i", len);
            strncpy(ptrTitle, codingWords[messageWords[i].id], len);
            ptrTitle += len;
            
            // title[len] = ' ';
            // len++;
            // ptrTitle++;
        }
    }
    
    ptrTitle = '\0';
    
    //TraceLog(LOG_WARNING, "Titular: %s", title);
    
    // Generate newspaper with title and subtitle
    Image imNewspaper = LoadImage("resources/textures/ending_newspaper.png");
    SpriteFont fontNews = LoadSpriteFontEx("resources/fonts/Lora-Bold.ttf", 82, 250, 0);
    ImageDrawTextEx(&imNewspaper, (Vector2){ 50, 220 }, fontNews, "FRACASO EN LA GGJ18!", fontNews.baseSize, 0, DARKGRAY);
    
    // TODO: Draw subtitle message
    //ImageDrawTextEx(&imNewspaper, (Vector2){ 50, 210 }, fontNews, "SUBE LA ESCALERA!", fontNews.baseSize, 0, DARKGRAY);
  
    texNewspaper = LoadTextureFromImage(imNewspaper);
    UnloadSpriteFont(fontNews);
    UnloadImage(imNewspaper);
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    framesCounter++;
    
    if (framesCounter == 10) PlaySound(fxNews);
    
    if (state == 0)
    {
        rotation += 18.0f;
        scale += 0.0096f;
        
        if (scale >= 1.0f) 
        {
            scale = 1.0f;
            state = 1;
        }
    }
    
    if ((state == 1) && (IsKeyPressed(KEY_ENTER) || IsButtonPressed())) 
    {
        currentMission++;
        
        if (currentMission >= totalMissions) finishScreen = 2;
        else finishScreen = 1;
    }
    
    if (IsKeyPressed(KEY_SPACE)) showResults = !showResults;
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    DrawTexture(texBackground, 0, 0, WHITE);
    
    DrawTexturePro(texNewspaper, (Rectangle){ 0, 0, texNewspaper.width, texNewspaper.height },
                   (Rectangle){ GetScreenWidth()/2, GetScreenHeight()/2, texNewspaper.width*scale, texNewspaper.height*scale }, 
                   (Vector2){ (float)texNewspaper.width*scale/2, (float)texNewspaper.height*scale/2 }, rotation, WHITE);

    DrawTextureEx(texVignette, (Vector2){ 0, 0 }, 0.0f, 2.0f, WHITE);
    
    if (showResults)
    {
        for (int i = 0; i < missions[currentMission].wordsCount; i++)
        {
            if (messageWords[i].id == missions[currentMission].sols[i]) DrawText(messageWords[i].text, 10, 10 + 30*i, 20, GREEN);
            else DrawText(codingWords[messageWords[i].id], 10, 10 + 30*i, 20, RED);
        }
    }
    
    if (state == 1) DrawButton("continuar");
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
    UnloadTexture(texBackground);
    UnloadTexture(texNewspaper);
    UnloadTexture(texVignette);
    
    UnloadSound(fxNews);
	free(missions);
}

// Ending Screen should finish?
int FinishEndingScreen(void)
{
    return finishScreen;
}