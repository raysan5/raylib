/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#define MAX_CODING_WORDS    12
//#define MAX_MISSION_WORDS    8
#define MAX_LINE_CHAR  30

/*
// NOTE: Coding words are generic and the same words
// are used for all missions,
typedef enum CodingWords {
    POLLO = 0,
    CONEJO,
    HUEVO,
    NIDO,
    AIRE,
    ARMARIO,
    AGUJERO,
    COSA,
    WORD,
} CodingWords;
*/

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
    "melon\0"
};

// Words to be coded or coding words
/*typedef struct Word {
    int id;
    Rectangle rec;
    Rectangle iniRec;
    bool hover;
    bool picked;
    char text[32];          // text
} Word;*/

/*
// Mission information
typedef struct Mission {
    int id;
    char brief[512];        // Mission briefing
    char key[32];           // Mission keyword
    char msg[256];          // Message to be coded
    int wordsCount;         // Number of words to coded
    int sols[8];            // Solution code, depends on wordsCount
} Mission;
*/
//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Gameplay screen global variables
static int framesCounter;
static int finishScreen;

static Texture2D texBackground;
static Font fontMessage;
static Texture2D texWordsAtlas;
static Texture2D texVignette;

static Sound fxGrab;
static Sound fxPlace;
static Sound fxLeave;

static Music musSpy;

static Word words[MAX_CODING_WORDS] = { 0 };

// Hay que hacerlo global, para poder consultar el resultado desde la endingscreen
//static Word messageWords[MAX_MISSION_WORDS] = { 0 };

static Mission *missions = NULL;

static bool canSend = false;

Vector2 msgOffset = { 430, 300 };

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    framesCounter = 0;
    finishScreen = 0;

    fontMessage = LoadFontEx("resources/fonts/traveling_typewriter.ttf", 30, 0, 250);

    texBackground = LoadTexture("resources/textures/message_background.png");
    texVignette = LoadTexture("resources/textures/message_vignette.png");

    fxGrab = LoadSound("resources/audio/fx_grab.ogg");
    fxPlace = LoadSound("resources/audio/fx_place.ogg");
    fxLeave = LoadSound("resources/audio/fx_leave.ogg");

    musSpy = LoadMusicStream("resources/audio/s_p_y.xm");
    PlayMusicStream(musSpy);

#if defined(PLATFORM_WEB)
    #define WORD_ATLAS_FROM_FILE
#endif
#if defined(WORD_ATLAS_FROM_FILE)
    texWordsAtlas = LoadTexture("resources/textures/mission_words.png");
#else
    // Generate coding words atlas directly from text
    Image imWordsBase = LoadImage("resources/textures/words_base.png");
    Image imWords = GenImageColor(imWordsBase.width, imWordsBase.height*MAX_CODING_WORDS, WHITE);

    for (int i = 0; i < MAX_CODING_WORDS; i++)
    {
        ImageDraw(&imWords, imWordsBase,
                  (Rectangle){ 0, 0, imWordsBase.width, imWordsBase.height },
                  (Rectangle){ 0, imWordsBase.height*i, imWordsBase.width, imWordsBase.height }, WHITE);

        ImageDrawTextEx(&imWords,(Vector2){ imWordsBase.width/2 - MeasureTextEx(fontMessage, codingWords[i],
                        fontMessage.baseSize, 0).x/2, imWordsBase.height*i }, fontMessage, codingWords[i],
                        fontMessage.baseSize, 0, BLACK);
    }

    texWordsAtlas = LoadTextureFromImage(imWords);

    UnloadImage(imWordsBase);
    UnloadImage(imWords);
#endif

    // Initialize missions
    // WARNING: Some problem with imWords image generation (memory leak?) could cause
    // that loading missions before/after generation breaks game, on web is the other way round... :(
    missions = LoadMissions("resources/missions.txt");
    TraceLog(LOG_WARNING, "Words count %i", missions[currentMission].wordsCount);

    // Initialize coding words
    for (int i = 0; i < MAX_CODING_WORDS; i++)
    {
        words[i].id = -1;          // Not placed anywhere

        words[i].rec.x = 110 + 940*(i/(MAX_CODING_WORDS/2));
        words[i].rec.y = 200 + 60*(i%(MAX_CODING_WORDS/2));
        words[i].rec.width = 140;  // texWordsAtlas.width/MAX_MISSIONS
        words[i].rec.height = 35;  // texWordsAtlas.height/MAX_MISSION_WORDS
        words[i].iniRec = words[i].rec;
        words[i].hover = false;    // Mouse hover detected
        words[i].picked = false;   // Mouse picked

        //words[i].text = ''; //codingWords[i];     // Fill text if required...
    }

    // Analize missions[currentMission].msg string for words!
    int msgLen = strlen(missions[currentMission].msg);

    // Add '/' each MAX_LINE_CHAR chars
    int currentLine = 1;
    int i = currentLine * MAX_LINE_CHAR;

    while (i < msgLen - 1)
    {
        if (missions[currentMission].msg[i] == ' ')
        {
            missions[currentMission].msg[i] = '/';
            currentLine++;
            i = currentLine*MAX_LINE_CHAR;
        }
        else i++;
    }

    int currentWord = 0;
    int offsetX = 0;
    int offsetY = 0;
    bool foundWord = false;
    int wordInitPosX = 0;
    int wordInitPosY = 0;

    for (int i = 0; i < msgLen; i++)
    {
        char c = missions[currentMission].msg[i];
        if (foundWord && (c == ' ' || c == '.'))
        {
            foundWord = false;

            messageWords[currentWord - 1].rec.width = (int)MeasureTextEx(fontMessage, TextSubtext(missions[currentMission].msg, wordInitPosX, (i - wordInitPosX)), 30, 0).x;
            messageWords[currentWord - 1].rec.height = fontMessage.baseSize;

            strncpy(messageWords[currentWord - 1].text, TextSubtext(missions[currentMission].msg, wordInitPosX, (i - wordInitPosX)), i - wordInitPosX);
        }

        if (c == '@') // One word to change
        {
            foundWord = true;
            missions[currentMission].msg[i] = ' ';

            offsetX = (int)MeasureTextEx(fontMessage, TextSubtext(missions[currentMission].msg, wordInitPosY, (i + 1) - wordInitPosY), 30, 0).x;

            messageWords[currentWord].rec.x = offsetX;
            messageWords[currentWord].rec.y = offsetY;

            wordInitPosX = i + 1;

            currentWord++;
        }
        else if (c == '/')
        {
            missions[currentMission].msg[i] = '\n';
            wordInitPosY = i;
            offsetY += (fontMessage.baseSize + fontMessage.baseSize/2);     // raylib internal increment on line break...
        }
    }

    for (int i = 0; i < missions[currentMission].wordsCount; i++)
    {
        messageWords[i].id = -1;          // Not required for message words, id is the array position

        // Recalculate words rectangles considering text offset on screen
        messageWords[i].rec.x += msgOffset.x;
        messageWords[i].rec.y += msgOffset.y;

        // Recalculate words rectangle considering new width height
        messageWords[i].rec.x -= (texWordsAtlas.width - messageWords[i].rec.width)/2;
        messageWords[i].rec.y -= ((texWordsAtlas.height / MAX_CODING_WORDS) - messageWords[i].rec.height)/2;

        //Recalculate width height
        messageWords[i].rec.width =  texWordsAtlas.width;
        messageWords[i].rec.height = texWordsAtlas.height / MAX_CODING_WORDS;

        messageWords[i].hover = false;    // Mouse hover detected
        messageWords[i].picked = false;   // Mouse picked
    }
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    UpdateMusicStream(musSpy);

    for (int i = 0; i < MAX_CODING_WORDS; i++)
    {
        if (CheckCollisionPointRec(GetMousePosition(), words[i].rec))
        {
            words[i].hover = true;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                words[i].picked = true;
                PlaySound(fxGrab);
            }
        }
        else words[i].hover = false;


        if (words[i].picked)
        {
            for (int j = 0; j < missions[currentMission].wordsCount; j++)
            {
                if (CheckCollisionPointRec(GetMousePosition(), messageWords[j].rec)) messageWords[j].hover = true;
                else messageWords[j].hover = false;
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                words[i].picked = false;

                for (int j = 0; j < missions[currentMission].wordsCount; j++)
                {
                    messageWords[j].hover = false;

                    if (CheckCollisionPointRec(GetMousePosition(), messageWords[j].rec))
                    {
                        PlaySound(fxPlace);

                        words[i].rec.x = messageWords[j].rec.x;
                        words[i].rec.y = messageWords[j].rec.y;

                        if (messageWords[j].id != -1)
                        {
                            int id = messageWords[j].id;
                            words[id].rec = words[id].iniRec;
                        }

                        messageWords[j].id = i;
                        for (int k = 0; k < missions[currentMission].wordsCount; k++)
                        {
                            if (j != k && messageWords[j].id == messageWords[k].id)
                            {
                                messageWords[k].id = -1;
                                break;
                            }
                        }
                        break;
                    }
                    else
                    {
                        PlaySound(fxLeave);

                        words[i].rec = words[i].iniRec;
                        if (i == messageWords[j].id) messageWords[j].id = -1;
                    }
                }
            }
        }

        // Move word picked with mouse
        if (words[i].picked)
        {
            words[i].rec.x = GetMouseX() - words[i].rec.width/2;
            words[i].rec.y = GetMouseY() - words[i].rec.height/2;
        }
    }

    canSend = true;
    for (int j = 0; j < missions[currentMission].wordsCount; j++)
    {
        if (messageWords[j].id == -1)
        {
            canSend = false;
            break;
        }
    }

    if (canSend && (IsKeyPressed(KEY_ENTER) || IsButtonPressed()))
    {
        finishScreen = true;
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    DrawTexture(texBackground, 0, 0, WHITE);

    DrawTextEx(fontMessage, missions[currentMission].msg, msgOffset, fontMessage.baseSize, 0, BLACK);

    for (int i = 0; i < missions[currentMission].wordsCount; i++)
    {
        Rectangle recLines = messageWords[i].rec;
        DrawRectangleLines(recLines.x, recLines.y, recLines.width, recLines.height, Fade(RED, 0.35f));
        if (messageWords[i].hover) DrawRectangleRec(messageWords[i].rec, Fade(RED, 0.30f));
        DrawText(FormatText("%i", messageWords[i].id), i*25, 0, 30, RED);
    }
    for (int i = 0; i < MAX_CODING_WORDS; i++)
    {
        if (words[i].picked) DrawTextureRec(texWordsAtlas, (Rectangle){ 0, i*35, 140, 35 }, (Vector2){ words[i].rec.x, words[i].rec.y }, MAROON);
        else if (words[i].hover) DrawTextureRec(texWordsAtlas, (Rectangle){ 0, i*35, 140, 35 }, (Vector2){ words[i].rec.x, words[i].rec.y }, RED);
        else DrawTextureRec(texWordsAtlas, (Rectangle){ 0, i*35, 140, 35 }, (Vector2){ words[i].rec.x, words[i].rec.y }, WHITE);
    }

    DrawTexturePro(texVignette, (Rectangle){0,0,texVignette.width, texVignette.height}, (Rectangle){0,0,GetScreenWidth(), GetScreenHeight()}, (Vector2){0,0}, 0, WHITE);

    if (canSend) DrawButton("enviar");
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    UnloadTexture(texBackground);
    UnloadTexture(texVignette);
    UnloadTexture(texWordsAtlas);

    UnloadSound(fxGrab);
    UnloadSound(fxLeave);
    UnloadSound(fxPlace);

    UnloadMusicStream(musSpy);

    free(missions);
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}
