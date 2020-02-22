/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Screens Functions Declarations (Init, Update, Draw, Unload)
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

#ifndef SCREENS_H
#define SCREENS_H

#define BASE_HEAD_WIDTH     400
#define BASE_HAIR_WIDTH     500
#define BASE_NOSE_WIDTH      80
#define BASE_MOUTH_WIDTH    170
#define BASE_EYES_WIDTH     240

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen { LOGO = 0, TITLE, OPTIONS, GAMEPLAY, ENDING } GameScreen;

typedef struct {
    int head;
    int colHead;
    int eyes;           // Config
    int nose;           // Config
    int mouth;          // Config
    int hair;           // Config
    int colHair;        // Config
    int glasses;        // Config
    //int piercing;
    //int freckles;
} Character;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern const unsigned int headColors[6];
extern const unsigned int hairColors[10];

extern GameScreen currentScreen;
extern Font font;
extern Music music;
extern Sound fxCoin;
extern Texture2D background;
extern Texture2D texNPatch;
extern NPatchInfo npInfo;
extern Texture2D texHead, texHair, texNose, texMouth, texEyes, texComp;
extern Texture2D texMakeup;

extern Character player;
extern Character playerBase;
extern Character dating;
extern Character datingBase;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

// Gui Button
bool GuiButton(Rectangle rec, const char *text, int forcedState);

Character GenerateCharacter(void);
void CustomizeCharacter(Character *character);
void DrawCharacter(Character character, Vector2 position);

//----------------------------------------------------------------------------------
// Logo Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLogoScreen(void);
void UpdateLogoScreen(void);
void DrawLogoScreen(void);
void UnloadLogoScreen(void);
int FinishLogoScreen(void);

//----------------------------------------------------------------------------------
// Title Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int FinishTitleScreen(void);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitGameplayScreen(void);
void UpdateGameplayScreen(void);
void DrawGameplayScreen(void);
void UnloadGameplayScreen(void);
int FinishGameplayScreen(void);

//----------------------------------------------------------------------------------
// Ending Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitEndingScreen(void);
void UpdateEndingScreen(void);
void DrawEndingScreen(void);
void UnloadEndingScreen(void);
int FinishEndingScreen(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H