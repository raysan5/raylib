/*******************************************************************************************
*
*   raylib - Advance Game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens/screens.h"    // NOTE: Defines global variable: currentScreen

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------

// Required variables to manage screen transitions (fade-in, fade-out)
float transAlpha = 0;
bool onTransition = false;
bool transFadeOut = false;
int transFromScreen = -1;
int transToScreen = -1;
    
//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
void TransitionToScreen(int screen);
void UpdateTransition(void);
void DrawTransition(void);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
	// Initialization
	//---------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
	const char windowTitle[30] = "<game name goes here>";
    
    InitWindow(screenWidth, screenHeight, windowTitle);

    // TODO: Load global data here (assets that must be available in all screens, i.e. fonts)
    
    // Setup and Init first screen
    currentScreen = LOGO;
    InitLogoScreen();
    
	SetTargetFPS(60);
	//----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (!onTransition)
        {
            switch(currentScreen) 
            {
                case LOGO: 
                {
                    UpdateLogoScreen();
                    
                    if (FinishLogoScreen()) TransitionToScreen(TITLE);
                    
                } break;
                case TITLE: 
                {
                    UpdateTitleScreen();
                    
                    if (FinishTitleScreen() == 1) TransitionToScreen(OPTIONS);
                    else if (FinishTitleScreen() == 2) TransitionToScreen(GAMEPLAY);

                } break;
                case OPTIONS:
                {
                    UpdateOptionsScreen();
                    
                    if (FinishOptionsScreen()) TransitionToScreen(TITLE);

                } break;
                case GAMEPLAY:
                { 
                    UpdateGameplayScreen();
                    
                    if (FinishGameplayScreen()) TransitionToScreen(ENDING);
  
                } break;
                case ENDING: 
                {
                    UpdateEndingScreen();
                    
                    if (FinishEndingScreen()) TransitionToScreen(TITLE);

                } break;
                default: break;
            }
        }
        else
        {
            // Update transition (fade-in, fade-out)
            UpdateTransition();
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            switch(currentScreen) 
            {
                case LOGO: DrawLogoScreen(); break;
                case TITLE: DrawTitleScreen(); break;
                case OPTIONS: DrawOptionsScreen(); break;
                case GAMEPLAY: DrawGameplayScreen(); break;
                case ENDING: DrawEndingScreen(); break;
                default: break;
            }
            
            if (onTransition) DrawTransition();
        
            //DrawFPS(10, 10);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // TODO: Unload all global loaded data (i.e. fonts) here!
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
	
    return 0;
}

void TransitionToScreen(int screen)
{
    onTransition = true;
    transFromScreen = currentScreen;
    transToScreen = screen;
}

void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.01;

        if (transAlpha >= 1.0)
        {
            transAlpha = 1.0;
        
            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case OPTIONS: UnloadOptionsScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                case ENDING: UnloadEndingScreen(); break;
                default: break;
            }
            
            switch (transToScreen)
            {
                case LOGO:
                {
                    InitLogoScreen(); 
                    currentScreen = LOGO; 
                } break;
                case TITLE: 
                {
                    InitTitleScreen();
                    currentScreen = TITLE;                  
                } break;
                case OPTIONS:
                {
                    InitOptionsScreen(); 
                    currentScreen = OPTIONS;
                } break;
                case GAMEPLAY:
                {
                    InitGameplayScreen(); 
                    currentScreen = GAMEPLAY;
                } break;
                case ENDING:
                {
                    InitEndingScreen(); 
                    currentScreen = ENDING;
                } break;
                default: break;
            }
            
            transFadeOut = true;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.01f;
        
        if (transAlpha <= 0)
        {
            transAlpha = 0;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = -1;
        }
    }
}

void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}