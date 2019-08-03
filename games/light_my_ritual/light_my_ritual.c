/*******************************************************************************************
*
*   LIGHT MY RITUAL [GLOBAL GAME JAM 2016]
*
*   Preparing a ritual session is not that easy. 
*   You must light all the candles before the astral alignment finishes... 
*   but dark creatures move in the shadows to put out all your lights! 
*   Be fast! Be smart! Light my ritual!
*
*   This game has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
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

static Music music;
    
//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
void TransitionToScreen(int screen);
void ChangeToScreen(int screen);    // No transition effect
void UpdateTransition(void);
void DrawTransition(void);

void UpdateDrawFrame(void);         // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "LIGHT MY RITUAL! [GGJ16]");

    // Global data loading (assets that must be available in all screens, i.e. fonts)
    InitAudioDevice();

    Image image = LoadImage("resources/lights_map.png");  // Load image in CPU memory (RAM)
    
    lightsMap = GetImageData(image);            // Get image pixels data as an array of Color
    lightsMapWidth = image.width;
    lightsMapHeight = image.height;
    
    UnloadImage(image);                         // Unload image from CPU memory (RAM)
    
    font = LoadFont("resources/font_arcadian.png");
    //doors = LoadTexture("resources/textures/doors.png");
    //sndDoor = LoadSound("resources/audio/door.ogg");
    
    music = LoadMusicStream("resources/audio/ambient.ogg");
    PlayMusicStream(music);
    SetMusicVolume(music, 1.0f);

    // Setup and Init first screen
    currentScreen = LOGO_RL;
    //InitTitleScreen();
    //InitGameplayScreen();
    rlInitLogoScreen();

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
        case LOGO_RL: rlUnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        default: break;
    }
    
    // Unload all global loaded data (i.e. fonts) here!
    UnloadFont(font);
    //UnloadSound(sndDoor);
    
    UnloadMusicStream(music);
    
    free(lightsMap);
    
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
        case LOGO_RL: rlUnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        default: break;
    }
    
    switch (screen)
    {
        case LOGO_RL: rlInitLogoScreen(); break;
        case TITLE: InitTitleScreen(); break;
        case GAMEPLAY: InitGameplayScreen(); break;
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
                case LOGO_RL: rlUnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                default: break;
            }
            
            switch (transToScreen)
            {
                case LOGO_RL:
                {
                    rlInitLogoScreen();
                    currentScreen = LOGO_RL;
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
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
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
            case LOGO_RL: 
            {
                rlUpdateLogoScreen();
                
                if (rlFinishLogoScreen()) TransitionToScreen(TITLE);

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
                
                if (FinishGameplayScreen() == 1) ChangeToScreen(LOGO_RL);
                else if (FinishGameplayScreen() == 2) TransitionToScreen(TITLE);

            } break;
            default: break;
        }
    }
    else
    {
        // Update transition (fade-in, fade-out)
        UpdateTransition();
    }
    
    if (currentScreen != GAMEPLAY) UpdateMusicStream(music);
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
        ClearBackground(RAYWHITE);
        
        switch(currentScreen) 
        {
            case LOGO_RL: rlDrawLogoScreen(); break;
            case TITLE: DrawTitleScreen(); break;
            case GAMEPLAY: DrawGameplayScreen(); break;
            default: break;
        }
    
        if (onTransition) DrawTransition();
    
        //DrawFPS(10, 10);
    
    EndDrawing();
    //----------------------------------------------------------------------------------
}

