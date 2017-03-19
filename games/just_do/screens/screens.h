/**********************************************************************************************
*
*   raylib - Standard Game template
*
*   Screens Functions Declarations (Init, Update, Draw, Unload)
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

#ifndef SCREENS_H
#define SCREENS_H

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen { LOGO, LEVEL00, LEVEL01, LEVEL02, LEVEL03, LEVEL04, LEVEL05, LEVEL06, LEVEL07, LEVEL08, LEVEL09 } GameScreen;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
GameScreen currentScreen;
Sound levelWin;

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
// Level00 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel00Screen(void);
void UpdateLevel00Screen(void);
void DrawLevel00Screen(void);
void UnloadLevel00Screen(void);
int FinishLevel00Screen(void);

//----------------------------------------------------------------------------------
// Level01 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel01Screen(void);
void UpdateLevel01Screen(void);
void DrawLevel01Screen(void);
void UnloadLevel01Screen(void);
int FinishLevel01Screen(void);

//----------------------------------------------------------------------------------
// Level02 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel02Screen(void);
void UpdateLevel02Screen(void);
void DrawLevel02Screen(void);
void UnloadLevel02Screen(void);
int FinishLevel02Screen(void);

//----------------------------------------------------------------------------------
// Level03 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel03Screen(void);
void UpdateLevel03Screen(void);
void DrawLevel03Screen(void);
void UnloadLevel03Screen(void);
int FinishLevel03Screen(void);

//----------------------------------------------------------------------------------
// Level04 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel04Screen(void);
void UpdateLevel04Screen(void);
void DrawLevel04Screen(void);
void UnloadLevel04Screen(void);
int FinishLevel04Screen(void);

//----------------------------------------------------------------------------------
// Level05 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel05Screen(void);
void UpdateLevel05Screen(void);
void DrawLevel05Screen(void);
void UnloadLevel05Screen(void);
int FinishLevel05Screen(void);

//----------------------------------------------------------------------------------
// Level06 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel06Screen(void);
void UpdateLevel06Screen(void);
void DrawLevel06Screen(void);
void UnloadLevel06Screen(void);
int FinishLevel06Screen(void);

//----------------------------------------------------------------------------------
// Level07 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel07Screen(void);
void UpdateLevel07Screen(void);
void DrawLevel07Screen(void);
void UnloadLevel07Screen(void);
int FinishLevel07Screen(void);

//----------------------------------------------------------------------------------
// Level08 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel08Screen(void);
void UpdateLevel08Screen(void);
void DrawLevel08Screen(void);
void UnloadLevel08Screen(void);
int FinishLevel08Screen(void);

//----------------------------------------------------------------------------------
// Level09 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLevel09Screen(void);
void UpdateLevel09Screen(void);
void DrawLevel09Screen(void);
void UnloadLevel09Screen(void);
int FinishLevel09Screen(void);


void DrawRectangleBordersRec(Rectangle rec, int offsetX, int offsetY, int borderSize, Color col);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H