/*******************************************************************************************
*
*   TRANSMISSION MISSION [GLOBAL GAME JAM 2018]
*
*   Code the different filtration messages to be send to newspaper 
*   to avoid being understood in case of interception. 
*
*   This game has been created using raylib 1.8 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2018 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens/screens.h"    // NOTE: Defines global variable: currentScreen

#include <stdlib.h>
#include <stdio.h>

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
static void ChangeToScreen(int screen);     // No transition effect

static void TransitionToScreen(int screen);
static void UpdateTransition(void);
static void DrawTransition(void);

static void UpdateDrawFrame(void);          // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "transmission mission [GGJ18]");

    // Global data loading (assets that must be available in all screens, i.e. fonts)
    InitAudioDevice();

    music = LoadMusicStream("resources/audio/music_title.ogg");
    fxButton = LoadSound("resources/audio/fx_newspaper.ogg");
    
    SetMusicVolume(music, 1.0f);
    PlayMusicStream(music);
    
    fontMission = LoadFontEx("resources/fonts/traveling_typewriter.ttf", 64, 0, 250);
    texButton = LoadTexture("resources/textures/title_ribbon.png");
    
    // UI BUTTON
    recButton.width = texButton.width;
    recButton.height = texButton.height;
    recButton.x = screenWidth - recButton.width;
    recButton.y = screenHeight - recButton.height - 50;
    fadeButton = 0.8f;
    colorButton = RED;
    textPositionButton = (Vector2){recButton.x + recButton.width/2, recButton.y + recButton.height/2};
    fontSizeButton = 30;
    textColorButton = WHITE;

    currentMission = 0;
    totalMissions = 4;
    
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
    
    // Unload current screen data before closing
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case MISSION: UnloadMissionScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    // Unload all global loaded data (i.e. fonts) here!
    UnloadMusicStream(music);
    UnloadSound(fxButton);
    
    UnloadFont(fontMission);
    UnloadTexture(texButton);
    
    CloseAudioDevice();     // Close audio context
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Change to next screen, no transition
static void ChangeToScreen(int screen)
{
    // Unload current screen
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case MISSION: UnloadMissionScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
    
    // Init next screen
    switch (screen)
    {
        case LOGO: InitLogoScreen(); break;
        case TITLE: InitTitleScreen(); break;
        case MISSION: InitMissionScreen(); break;
        case GAMEPLAY: InitGameplayScreen(); break;
        case ENDING: InitEndingScreen(); break;
        default: break;
    }
    
    currentScreen = screen;
}

// Define transition to next screen
static void TransitionToScreen(int screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Update transition effect
static void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.02f;
        
        // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
        // For that reason we compare against 1.01f, to avoid last frame loading stop
        if (transAlpha > 1.01f)
        {
            transAlpha = 1.0f;
        
            // Unload current screen
            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case MISSION: UnloadMissionScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                case ENDING: UnloadEndingScreen(); break;
                default: break;
            }
            
            // Load next screen
            switch (transToScreen)
            {
                case LOGO: InitLogoScreen(); break;
                case TITLE: InitTitleScreen(); break;
                case MISSION: InitMissionScreen(); break;
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
        transAlpha -= 0.02f;
        
        if (transAlpha < -0.01f)
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
static void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateMusicStream(music);       // NOTE: Music keeps playing between screens
    
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

                if (FinishTitleScreen()) 
                {
                    StopMusicStream(music);
                    TransitionToScreen(MISSION);
                }

            } break;
            case MISSION:
            {
                UpdateMissionScreen();
                    
                if (FinishMissionScreen())
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
                
                if (FinishEndingScreen() == 1)          // Continue to next mission
                {
                    TransitionToScreen(MISSION);
                }
                else if (FinishEndingScreen() == 2)     // Replay current mission
                {
                    PlayMusicStream(music);
                    TransitionToScreen(TITLE);
                }

            } break;
            default: break;
        }
    }
    else UpdateTransition();    // Update transition (fade-in, fade-out)
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        
        ClearBackground(RAYWHITE);
            
        switch(currentScreen) 
        {
            case LOGO: DrawLogoScreen(); break;
            case TITLE: DrawTitleScreen(); break;
            case MISSION: DrawMissionScreen(); break;
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

// Load missions from text file
Mission *LoadMissions(const char *fileName)
{
    Mission *missions = NULL;
    char buffer[512];
    
    int missionsCount = 0;
    
    FILE *misFile = fopen(fileName, "rt");
    
    if (misFile == NULL) printf("[%s] Missions file could not be opened\n", fileName);
    else
    {
        // First pass to get total missions count
        while (!feof(misFile))
        {
            fgets(buffer, 512, misFile);
            
            switch (buffer[0])
            {
                case 't': sscanf(buffer, "t %i", &missionsCount); break;
                default: break;
            }
        }
        
        if (missionsCount > 0) missions = (Mission *)malloc(missionsCount*sizeof(Mission));
        else return NULL;
        
        rewind(misFile);    // Return to the beginning of the file, to read again
        
        int missionNum = 0;
        
        while (!feof(misFile))
        {
            fgets(buffer, 512, misFile);

            if (missionNum < missionsCount)
            {
                switch (buffer[0])
                {
                    case 'b': 
                    {
                        // New mission brief starts!
                        missions[missionNum].id = missionNum;
                        sscanf(buffer, "b %[^\n]s", missions[missionNum].brief); 
                    } break;
                    case 'k': sscanf(buffer, "k %[^\n]s", missions[missionNum].key); break;
                    case 'm':
                    {
                        // NOTE: Message is loaded as is, needs to be processed!
                        sscanf(buffer, "m %[^\n]s", missions[missionNum].msg);
                    } break;
                    case 's': 
                    {
                        sscanf(buffer, "s %i %i %i %i %i %i %i %i", 
                               &missions[missionNum].sols[0], 
                               &missions[missionNum].sols[1],
                               &missions[missionNum].sols[2],
                               &missions[missionNum].sols[3],
                               &missions[missionNum].sols[4],
                               &missions[missionNum].sols[5],
                               &missions[missionNum].sols[6],
                               &missions[missionNum].sols[7]);
                               
                        missions[missionNum].wordsCount = 0;
                        
                        for (int i = 0; i < 8; i++) 
                        {
                            if (missions[missionNum].sols[i] > -1)
                            {
                                missions[missionNum].wordsCount++;
                            }
                        }
                        
                        TraceLog(LOG_WARNING, "Mission %i - Words count %i", missionNum, missions[missionNum].wordsCount);
                        
                        missionNum++;
                    } break;
                    default: break;
                }
            }
        }
        
        if (missionsCount != missionNum) TraceLog(LOG_WARNING, "Missions count and loaded missions don't match!");
    }

    fclose(misFile);
    
    if (missions != NULL) 
    {
        TraceLog(LOG_INFO, "Missions loaded: %i", missionsCount);
        TraceLog(LOG_INFO, "Missions loaded successfully!");
    }
    
    return missions;
}

bool IsButtonPressed()
{
    if (CheckCollisionPointRec(GetMousePosition(), recButton))
    {
        fadeButton = 1.0f;
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsGestureDetected(GESTURE_TAP))
        {
            PlaySound(fxButton);
            return true;
        }
    }
    else fadeButton = 0.80f;
    
    return false;
}

void DrawButton(const char *text)
{
    //DrawRectangleRec(recButton, Fade(colorButton, fadeButton));
    DrawTexturePro(texButton, (Rectangle){0,0,texButton.width, texButton.height}, recButton, (Vector2){0,0},0, Fade(WHITE, fadeButton));
    Vector2 measure = MeasureTextEx(fontMission, text, fontSizeButton, 0);
    Vector2 textPos = {textPositionButton.x - measure.x/2 + 10, textPositionButton.y - measure.y/2 - 10};
    DrawTextEx(fontMission, text, textPos , fontSizeButton, 0, textColorButton);
}
