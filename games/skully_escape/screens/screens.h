/**********************************************************************************************
*
*   raylib - Advance Game template
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

#define PLAYER_ANIM_FRAMES     7
#define PLAYER_ANIM_SEQ        2

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen { LOGO = 0, LOGO_RL, TITLE, ATTIC, AISLE01, AISLE02, BATHROOM, LIVINGROOM, KITCHEN, ARMORY, ENDING } GameScreen;

typedef struct Door {
    Vector2 position;
    int facing;
    bool locked;
    bool selected;
    Rectangle frameRec;
    Rectangle bound;
} Door;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
GameScreen currentScreen;
Font font;

Texture2D doors;
Sound sndDoor;
Sound sndScream;


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
// raylib Logo Screen Functions Declaration
//----------------------------------------------------------------------------------
void rlInitLogoScreen(void);
void rlUpdateLogoScreen(void);
void rlDrawLogoScreen(void);
void rlUnloadLogoScreen(void);
int rlFinishLogoScreen(void);

//----------------------------------------------------------------------------------
// Title Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int FinishTitleScreen(void);

//----------------------------------------------------------------------------------
// Attic Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitAtticScreen(void);
void UpdateAtticScreen(void);
void DrawAtticScreen(void);
void UnloadAtticScreen(void);
int FinishAtticScreen(void);

//----------------------------------------------------------------------------------
// Aisle01 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitAisle01Screen(void);
void UpdateAisle01Screen(void);
void DrawAisle01Screen(void);
void UnloadAisle01Screen(void);
int FinishAisle01Screen(void);

//----------------------------------------------------------------------------------
// Aisle02 Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitAisle02Screen(void);
void UpdateAisle02Screen(void);
void DrawAisle02Screen(void);
void UnloadAisle02Screen(void);
int FinishAisle02Screen(void);

//----------------------------------------------------------------------------------
// Bathroom Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitBathroomScreen(void);
void UpdateBathroomScreen(void);
void DrawBathroomScreen(void);
void UnloadBathroomScreen(void);
int FinishBathroomScreen(void);

//----------------------------------------------------------------------------------
// Livingroom Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitLivingroomScreen(void);
void UpdateLivingroomScreen(void);
void DrawLivingroomScreen(void);
void UnloadLivingroomScreen(void);
int FinishLivingroomScreen(void);

//----------------------------------------------------------------------------------
// Kitchen Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitKitchenScreen(void);
void UpdateKitchenScreen(void);
void DrawKitchenScreen(void);
void UnloadKitchenScreen(void);
int FinishKitchenScreen(void);

//----------------------------------------------------------------------------------
// Armory Screen Functions Declaration
//----------------------------------------------------------------------------------
void InitArmoryScreen(void);
void UpdateArmoryScreen(void);
void DrawArmoryScreen(void);
void UnloadArmoryScreen(void);
int FinishArmoryScreen(void);

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