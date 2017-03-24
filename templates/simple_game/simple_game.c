/*******************************************************************************************
*
*   raylib - Simple Game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   raylib - Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "screens.h"

//----------------------------------------------------------------------------------
// Global Variables Defined in other modules
//----------------------------------------------------------------------------------
extern GameScreen currentScreen;    // Defined in screens.c

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

    // Initialize all screens
    InitLogoScreen();
    InitTitleScreen();
    InitGameplayScreen();
    InitEndingScreen();

    // Define first screen
    currentScreen = LOGO;
    
	SetTargetFPS(60);
	//----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        switch(currentScreen) 
        {
            case LOGO:      UpdateLogoScreen(); break;      // Update LOGO currentScreen
            case TITLE:     UpdateTitleScreen(); break;     // Update TITLE currentScreen
            case GAMEPLAY:  UpdateGameplayScreen(); break;  // Update GAMEPLAY currentScreen
            case ENDING:    UpdateEndingScreen(); break;       // Update END currentScreen
            default: break;
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            switch(currentScreen) 
            {
                case LOGO:      DrawLogoScreen(); break;        // Draw LOGO currentScreen
                case TITLE:     DrawTitleScreen(); break;       // Draw TITLE currentScreen
                case GAMEPLAY:  DrawGameplayScreen(); break;    // Draw GAMEPLAY currentScreen
                case ENDING:    DrawEndingScreen(); break;         // Draw END currentScreen
                default: break;
            }
        
            DrawFPS(screenWidth - 100, 20);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // Unload all loaded data (textures, fonts, audio)
    UnloadLogoScreen();
    UnloadTitleScreen();
    UnloadGameplayScreen();
    UnloadEndingScreen();
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
	
    return 0;
}