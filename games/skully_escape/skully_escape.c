/*******************************************************************************************
*
*   SKULLY ESCAPE [KING GAME JAM 2015]
*
*   A scary graphic adventure in an old mansion
*
*   This game has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens/screens.h"    // NOTE: Defines global variable: currentScreen

#include "player.h"

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

static int framesCounter = 0;

Music music;
    
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
    InitWindow(screenWidth, screenHeight, "SKULLY ESCAPE [KING GAMEJAM 2015]");

    // Global data loading (assets that must be available in all screens, i.e. fonts)
    InitAudioDevice();
    
    music = LoadMusicStream("resources/audio/come_play_with_me.ogg");
    PlayMusicStream(music);
    
    font = LoadFont("resources/textures/alagard.png");
    doors = LoadTexture("resources/textures/doors.png");
    sndDoor = LoadSound("resources/audio/door.ogg");
    sndScream = LoadSound("resources/audio/scream.ogg");
    
    InitPlayer();
    
    // Setup and Init first screen
    currentScreen = LOGO;
    InitLogoScreen();

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
    
    // Unload all global loaded data (i.e. fonts) here!
    UnloadPlayer();
    UnloadFont(font);
    UnloadTexture(doors);
    UnloadSound(sndDoor);
    UnloadSound(sndScream);
    
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
        case LOGO_RL: rlUnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case ATTIC: UnloadAtticScreen(); break;
        case AISLE01: UnloadAisle01Screen();break;
        case AISLE02: UnloadAisle02Screen();break;
        case ARMORY: UnloadArmoryScreen();break;
        case LIVINGROOM: UnloadLivingroomScreen();break;
        case KITCHEN: UnloadKitchenScreen(); break;
        case BATHROOM: UnloadBathroomScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    switch (screen)
    {
        case LOGO: InitLogoScreen(); break;
        case LOGO_RL: rlInitLogoScreen(); break;
        case TITLE: InitTitleScreen(); break;
        case ATTIC: InitAtticScreen(); break;
        case AISLE01: InitAisle01Screen();break;
        case AISLE02: InitAisle02Screen();break;
        case ARMORY: InitArmoryScreen();break;
        case LIVINGROOM: InitLivingroomScreen();break;
        case KITCHEN: InitKitchenScreen(); break;
        case BATHROOM: InitBathroomScreen(); break;
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
                case LOGO_RL: rlUnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case ATTIC: UnloadAtticScreen(); break;
                case AISLE01: UnloadAisle01Screen();break;
                case AISLE02: UnloadAisle02Screen();break;
                case ARMORY: UnloadArmoryScreen();break;
                case LIVINGROOM: UnloadLivingroomScreen();break;
                case KITCHEN: UnloadKitchenScreen(); break;
                case BATHROOM: UnloadBathroomScreen(); break;
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
                case ATTIC:
                {
                    InitAtticScreen(); 
                    currentScreen = ATTIC;
                } break;
                case AISLE01:
                {
                    InitAisle01Screen(); 
                    currentScreen = AISLE01;
                } break;
                case AISLE02:
                {
                    InitAisle02Screen(); 
                    currentScreen = AISLE02;
                } break;
                case BATHROOM:
                {
                    InitBathroomScreen(); 
                    currentScreen = BATHROOM;
                } break;
                case LIVINGROOM:
                {
                    InitLivingroomScreen(); 
                    currentScreen = LIVINGROOM;
                } break;
                case KITCHEN:
                {
                    InitKitchenScreen(); 
                    currentScreen = KITCHEN;
                } break;
                case ARMORY:
                {
                    InitArmoryScreen(); 
                    currentScreen = ARMORY;
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
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    if (!onTransition)
    {
        if (player.dead)
        {
            framesCounter++;
            
            if (framesCounter > 80)
            {
                framesCounter = 0;
                player.dead = false;
                player.numLifes = 4;
                
                TransitionToScreen(TITLE);
            }
        }
        
        switch(currentScreen) 
        {
            case LOGO: 
            {
                UpdateLogoScreen();
                
                if (FinishLogoScreen()) ChangeToScreen(LOGO_RL);
                
            } break;
            case LOGO_RL: 
            {
                rlUpdateLogoScreen();
                
                if (rlFinishLogoScreen()) TransitionToScreen(TITLE);
                
            } break;
            case TITLE: 
            {
                UpdateTitleScreen();
                
                if (FinishTitleScreen() == 1) TransitionToScreen(ATTIC);

            } break;
            case ATTIC:
            { 
                UpdateAtticScreen();
                
                if (FinishAtticScreen() == 1) TransitionToScreen(AISLE01);

            } break;
            case AISLE01:
            { 
                UpdateAisle01Screen();
                
                if (FinishAisle01Screen() == 1) TransitionToScreen(BATHROOM);
                else if(FinishAisle01Screen() == 2) TransitionToScreen(KITCHEN);
                else if(FinishAisle01Screen() == 3) TransitionToScreen(LIVINGROOM);

            } break;
            case BATHROOM:
            { 
                UpdateBathroomScreen();
                
                if (FinishBathroomScreen() == 1) TransitionToScreen(AISLE01);

            } break;
            case LIVINGROOM:
            { 
                UpdateLivingroomScreen();
                
                if (FinishLivingroomScreen() == 1) TransitionToScreen(AISLE01);
                else if(FinishLivingroomScreen() == 2)TransitionToScreen(AISLE02);

            } break;
            case AISLE02:
            { 
                UpdateAisle02Screen();
                
                if (FinishAisle02Screen() == 1) TransitionToScreen(KITCHEN);

            } break;
            case KITCHEN:
            { 
                UpdateKitchenScreen();
                
                if (FinishKitchenScreen() == 1) TransitionToScreen(ARMORY);
                else if(FinishKitchenScreen() == 2)TransitionToScreen(AISLE02);

            } break;
            case ARMORY:
            { 
                UpdateArmoryScreen();
                
                if(FinishArmoryScreen() == 1) TransitionToScreen(ENDING);
                else if(FinishArmoryScreen() == 2) TransitionToScreen(KITCHEN);

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
    
    UpdateMusicStream(music);
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
        ClearBackground(RAYWHITE);
        
        switch(currentScreen) 
        {
            case LOGO: DrawLogoScreen(); break;
            case LOGO_RL: rlDrawLogoScreen(); break;
            case TITLE: DrawTitleScreen(); break;
            case ATTIC: DrawAtticScreen(); break;
            case AISLE01: DrawAisle01Screen();break;
            case AISLE02: DrawAisle02Screen();break;
            case BATHROOM: DrawBathroomScreen();break;
            case LIVINGROOM: DrawLivingroomScreen();break;
            case KITCHEN: DrawKitchenScreen();break;
            case ARMORY: DrawArmoryScreen();break;
            case ENDING: DrawEndingScreen(); break;
            default: break;
        }
    
        if (onTransition) DrawTransition();
    
        //DrawFPS(10, 10);
    
    EndDrawing();
    //----------------------------------------------------------------------------------
}

