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
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static int transToScreen = -1;

// NOTE: Some global variables that require to be visible for all screens,
// are defined in screens.h (i.e. currentScreen)
    
//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void TransitionToScreen(int screen);
static void ChangeToScreen(int screen);    // No transition effect
static void UpdateTransition(void);
static void DrawTransition(void);

static void UpdateDrawFrame(void);         // Update and Draw one frame

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

    CloseAudioDevice();     // Close audio context
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
	
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Define transition to next screen
static void TransitionToScreen(int screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Change to next screen, no transition
void ChangeToScreen(int screen)
{
    // Unload current screen
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    // Init next screen
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

// Update transition effect
void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;
        
        printf("transAlpha: %f\n", transAlpha);

        // TODO: Investigate this! SO WEIRD! Comparing with 1.0f does not work! Compiler optimization???
        if (transAlpha > 1.00001f)   // Make sure alpha is greater than 1.0, to avoid last frame loading stop
        {
            printf("alpha on change: %e\n", transAlpha);
            
            transAlpha = 1.0f;
        
            // Unload current screen
            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                case ENDING: UnloadEndingScreen(); break;
                default: break;
            }
            
            // Load next screen
            switch (transToScreen)
            {
                case LOGO: InitLogoScreen(); break;
                case TITLE: InitTitleScreen(); break;
                case GAMEPLAY: InitGameplayScreen(); break;
                case ENDING: InitEndingScreen(); break;
                default: break;
            }
            
            currentScreen = transToScreen;
            
            // Activate fade out effect to next loaded screen
            transFadeOut = true;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.05f;
        
        if (transAlpha <= 0.0f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = -1;
        }
    }
}

// Draw transition effect (full-screen rectangle)
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
                
                if (FinishTitleScreen() == 1) TransitionToScreen(GAMEPLAY);

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
    else UpdateTransition();    // Update transition (fade-in, fade-out)
    
    // TODO: Review! It breaks the game sometimes!!!
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
	
        // Draw full screen rectangle in front of everything
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