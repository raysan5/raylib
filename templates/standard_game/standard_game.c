/*******************************************************************************************
*
*   raylib - Standard Game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens/screens.h"    // NOTE: Defines global variable: currentScreen

#if defined(PLATFORM_ANDROID)
    #include "android_native_app_glue.h"
#endif

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
void android_main(struct android_app *app) 
#else
int main(void)
#endif
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

#if defined(PLATFORM_ANDROID)
    InitWindow(screenWidth, screenHeight, app);
#else
    InitWindow(screenWidth, screenHeight, "raylib template - standard game");
#endif

    // TODO: Load global data here (assets that must be available in all screens, i.e. fonts)
    
    // Define and init first screen
    currentScreen = LOGO;   // NOTE: currentScreen is defined in screens.h as a global variable
    InitLogoScreen();
    
    SetTargetFPS(60);
    //----------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        switch(currentScreen) 
        {
            case LOGO: 
            {
                UpdateLogoScreen();
                
                if (FinishLogoScreen())
                {
                    UnloadLogoScreen();
                    currentScreen = TITLE;
                    InitTitleScreen();
                }
            } break;
            case TITLE: 
            {
                UpdateTitleScreen();
                
                // NOTE: FinishTitleScreen() return an int defining the screen to jump to
                if (FinishTitleScreen() == 1)
                {
                    UnloadTitleScreen();
                    currentScreen = OPTIONS;
                    InitOptionsScreen();
                }
                else if (FinishTitleScreen() == 2)
                {
                    UnloadTitleScreen();
                    currentScreen = GAMEPLAY;
                    InitGameplayScreen();
                }
            } break;
            case OPTIONS:
            {
                UpdateOptionsScreen();
                
                if (FinishOptionsScreen())
                {
                    UnloadOptionsScreen();
                    currentScreen = TITLE;
                    InitTitleScreen();
                } 
            } break;
            case GAMEPLAY:
            { 
                UpdateGameplayScreen();
                
                if (FinishGameplayScreen())
                {
                    UnloadGameplayScreen();
                    currentScreen = ENDING;
                    InitEndingScreen();
                }   
            } break;
            case ENDING: 
            {
                UpdateEndingScreen();
                
                if (FinishEndingScreen())
                {
                    UnloadEndingScreen();
                    currentScreen = TITLE;
                    InitTitleScreen();
                } 
            } break;
            default: break;
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
        
            //DrawFPS(10, 10);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // TODO: Unload all global loaded data (i.e. fonts) here!
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
#if !defined(PLATFORM_ANDROID)
    return 0;
#endif
}