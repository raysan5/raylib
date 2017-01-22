/*******************************************************************************************
*
*   GLOBAL GAME JAM 2017 - WAVE COLLECTOR
*
*   The ultimate wave particles collector is here!
*   You must follow the wave and collect all the particles 
*   The level is actually the wave and the wave is the level!
*   Be fast! Be smart! Be the best wave collector!
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens/screens.h"    // NOTE: Defines global variable: currentScreen

#include <stdlib.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
const int screenWidth = 1280;
const int screenHeight = 720;

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
void ChangeToScreen(int screen);    // No transition effect
void UpdateTransition(void);
void DrawTransition(void);

void UpdateDrawFrame(void);         // Update and Draw one frame

//static const char *GetExtension(const char *fileName);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	// Initialization
	//---------------------------------------------------------
    /*
#if !defined(PLATFORM_WEB)
    // TODO: Add support for dropped files on the exe
    sampleFilename = (char *)malloc(256);
    if (argc > 1)
    {
        if ((strcmp(GetExtension(argv[1]), "ogg") == 0) ||
            (strcmp(GetExtension(argv[1]), "wav") == 0))
        {
            strcpy(sampleFilename, argv[1]);
        }
    }
#endif
    */
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "GGJ17 - WAVE COLLECTOR");

    // Global data loading (assets that must be available in all screens, i.e. fonts)
    InitAudioDevice();

    font = LoadSpriteFont("resources/font.fnt");
    music = LoadMusicStream("resources/audio/wave.ogg");
    
    SetMusicVolume(music, 1.0f);

    // Setup and Init first screen
    currentScreen = LOGO;
    InitLogoScreen();
    //InitTitleScreen();
    //InitGameplayScreen();
    //InitEndingScreen();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    // Unload all global loaded data (i.e. fonts) here!
    UnloadSpriteFont(font);
    UnloadMusicStream(music);

    CloseAudioDevice();
    
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

void ChangeToScreen(int screen)
{
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    switch (screen)
    {
        case LOGO: InitLogoScreen(); break;
        case TITLE: InitTitleScreen(); break;
        case GAMEPLAY: InitGameplayScreen(); break;
        case ENDING: InitEndingScreen(); break;
        default: break;
    }
    
    currentScreen = screen;
}

void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;

        if (transAlpha >= 1.0)
        {
            transAlpha = 1.0;
        
            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
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
        transAlpha -= 0.05f;
        
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
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RAYWHITE, transAlpha));
}

// Update and draw game frame
void UpdateDrawFrame(void)
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
                
                if (FinishTitleScreen() == 1)
                {
                    StopMusicStream(music);
                    TransitionToScreen(GAMEPLAY);
                }

            } break;
            case GAMEPLAY:
            { 
                UpdateGameplayScreen();
                
                if (FinishGameplayScreen() == 1) TransitionToScreen(ENDING);
                //else if (FinishGameplayScreen() == 2) TransitionToScreen(TITLE);

            } break;
            case ENDING:
            { 
                UpdateEndingScreen();
                
                if (FinishEndingScreen() == 1) TransitionToScreen(TITLE);

            } break;
            default: break;
        }
    }
    else
    {
        // Update transition (fade-in, fade-out)
        UpdateTransition();
    }
    
    if (currentScreen != ENDING) UpdateMusicStream(music);
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
        ClearBackground(RAYWHITE);
        
        switch(currentScreen) 
        {
            case LOGO: DrawLogoScreen(); break;
            case TITLE: DrawTitleScreen(); break;
            case GAMEPLAY: DrawGameplayScreen(); break;
            case ENDING: DrawEndingScreen(); break;
            default: break;
        }
	
        if (onTransition) DrawTransition();
    
        //DrawFPS(10, 10);
    
    EndDrawing();
    //----------------------------------------------------------------------------------
}

/*
#if !defined(PLATFORM_WEB)
// Get the extension for a filename
static const char *GetExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');
    if (!dot || dot == fileName) return "";
    return (dot + 1);
}
#endif
*/