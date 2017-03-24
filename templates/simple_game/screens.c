/**********************************************************************************************
*
*   raylib - Simple Game template
*
*   Screens Functions Definitions (Init, Update, Draw, Unload)
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

//----------------------------------------------------------------------------------
// Global Variables Definition (visible to other modules)
//----------------------------------------------------------------------------------
GameScreen currentScreen = LOGO;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Logo screen global variables
static int framesCounter;

// Title screen global variables

// Gameplay screen global variables

// Ending screen global variables

//----------------------------------------------------------------------------------
// Logo Screen Functions Definition
//----------------------------------------------------------------------------------

// Logo Screen Initialization logic
void InitLogoScreen(void)
{
    // TODO: Initialize LOGO screen variables here!
    framesCounter = 0;
}

// Logo Screen Update logic
void UpdateLogoScreen(void)
{
    // TODO: Update LOGO screen variables here!

    framesCounter++;    // Count frames

    // Wait for 2 seconds (120 frames) before jumping to TITLE screen
    if (framesCounter > 120)
    {
        currentScreen = TITLE;
    }
}

// Logo Screen Draw logic
void DrawLogoScreen(void)
{
    // TODO: Draw LOGO screen here!
    DrawText("LOGO SCREEN", 20, 20, 40, LIGHTGRAY);
    DrawText("WAIT for 2 SECONDS...", 290, 220, 20, GRAY);
}

// Logo Screen Unload logic
void UnloadLogoScreen(void)
{
    // TODO: Unload LOGO screen variables here!
}

//----------------------------------------------------------------------------------
// Title Screen Functions Definition
//----------------------------------------------------------------------------------

// Title Screen Initialization logic
void InitTitleScreen(void)
{
    // TODO: Initialize TITLE screen variables here!
}

// Title Screen Update logic
void UpdateTitleScreen(void)
{
    // TODO: Update TITLE screen variables here!

    // Press enter to change to GAMEPLAY screen
    if (IsKeyPressed(KEY_ENTER))
    {
        currentScreen = GAMEPLAY;
    }
}

// Title Screen Draw logic
void DrawTitleScreen(void)
{
    // TODO: Draw TITLE screen here!
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GREEN);
    DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
    DrawText("PRESS ENTER to JUMP to GAMEPLAY SCREEN", 160, 220, 20, DARKGREEN);
}

// Title Screen Unload logic
void UnloadTitleScreen(void)
{
    // TODO: Unload TITLE screen variables here!
}

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    // TODO: Initialize GAMEPLAY screen variables here!
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    // TODO: Update GAMEPLAY screen variables here!

    // Press enter to change to ENDING screen
    if (IsKeyPressed(KEY_ENTER))
    {
        currentScreen = ENDING;
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    // TODO: Draw GAMEPLAY screen here!
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), PURPLE);
    DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);
    DrawText("PRESS ENTER to JUMP to ENDING SCREEN", 170, 220, 20, MAROON);
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
}

//----------------------------------------------------------------------------------
// Ending Screen Functions Definition
//----------------------------------------------------------------------------------

// Ending Screen Initialization logic
void InitEndingScreen(void)
{
    // TODO: Initialize ENDING screen variables here!
}

// Ending Screen Update logic
void UpdateEndingScreen(void)
{
    // TODO: Update ENDING screen variables here!

    // Press enter to return to TITLE screen
    if (IsKeyPressed(KEY_ENTER))
    {
        currentScreen = TITLE;
    }
}

// Ending Screen Draw logic
void DrawEndingScreen(void)
{
    // TODO: Draw ENDING screen here!
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLUE);
    DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
    DrawText("PRESS ENTER to RETURN to TITLE SCREEN", 160, 220, 20, DARKBLUE);
}

// Ending Screen Unload logic
void UnloadEndingScreen(void)
{
    // TODO: Unload ENDING screen variables here!
}