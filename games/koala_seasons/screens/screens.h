/**********************************************************************************************
*
*   raylib - Koala Seasons game
*
*   Screens Functions Declarations (Init, Update, Draw, Unload)
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

#ifndef SCREENS_H
#define SCREENS_H

#define GAME_FPS           60.0
#define TIME_FACTOR        60.0/GAME_FPS

#define MAX_KILLS          128

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen { LOGO, TITLE, OPTIONS, GAMEPLAY, ENDING } GameScreen;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
GameScreen currentScreen;

// NOTE: This is all the data used in the game
Font font;
Shader colorBlend;
Texture2D atlas01;
Texture2D atlas02;

Sound fxJump; 
Sound fxDash; 
Sound fxEatLeaves;
Sound fxDieSnake;
Sound fxDieDingo;
Sound fxDieOwl;
Sound fxHitResin;
Sound fxWind;

// Global Variables (required by ending screen and gameplay screen)
int score;
int hiscore;
int killHistory[MAX_KILLS];
int killer;
int seasons;
int years;
int currentLeaves;
int currentSeason;
int initSeason;
int initYears;
int rainChance;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

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
// Options Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitOptionsScreen(void);
void UpdateOptionsScreen(void);
void DrawOptionsScreen(void);
void UnloadOptionsScreen(void);
int FinishOptionsScreen(void);

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