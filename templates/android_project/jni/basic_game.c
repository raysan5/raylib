/*******************************************************************************************
*
*   raylib - Android Basic Game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib v1.2 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "android_native_app_glue.h"

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum GameScreen { LOGO, TITLE, GAMEPLAY, ENDING } GameScreen;

//----------------------------------------------------------------------------------
// Android Main entry point
//----------------------------------------------------------------------------------
void android_main(struct android_app *app) 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    GameScreen currentScreen = LOGO;

    InitWindow(screenWidth, screenHeight, app);
    
    // TODO: Initialize all required variables and load all required data here!

    InitAudioDevice();                  // Initialize audio device
    
    Texture2D texture = LoadTexture("raylib_logo.png");     // Load texture (placed on assets folder)

    Sound fx = LoadSound("coin.wav");   // Load WAV audio file (placed on assets folder)
    Music ambient = LoadMusicStream("ambient.ogg");
    PlayMusicStream(ambient);

    int framesCounter = 0;          // Used to count frames
    
    SetTargetFPS(60);               // Not required on Android, already locked to 60 fps
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMusicStream(ambient);
        
        switch(currentScreen) 
        {
            case LOGO: 
            {
                // TODO: Update LOGO screen variables here!

                framesCounter++;    // Count frames

                // Wait for 4 seconds (240 frames) before jumping to TITLE screen
                if (framesCounter > 240)
                {
                    currentScreen = TITLE;
                }
            } break;
            case TITLE: 
            {
                // TODO: Update TITLE screen variables here!

                // Press enter to change to GAMEPLAY screen
                if (IsGestureDetected(GESTURE_TAP))
                {
                    PlaySound(fx);
                    currentScreen = GAMEPLAY;
                }
            } break;
            case GAMEPLAY:
            { 
                // TODO: Update GAMEPLAY screen variables here!

                // Press enter to change to ENDING screen
                if (IsGestureDetected(GESTURE_TAP))
                {  
                    PlaySound(fx);
                    currentScreen = ENDING;
                } 
            } break;
            case ENDING: 
            {
                // TODO: Update ENDING screen variables here!

                // Press enter to return to TITLE screen
                if (IsGestureDetected(GESTURE_TAP))
                {
                    PlaySound(fx);
                    currentScreen = TITLE;
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
                case LOGO: 
                {
                    // TODO: Draw LOGO screen here!
                    DrawText("LOGO SCREEN", 20, 20, 40, LIGHTGRAY);
                    DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);
                    DrawText("WAIT for 4 SECONDS...", 290, 400, 20, GRAY);
                    
                } break;
                case TITLE: 
                {
                    // TODO: Draw TITLE screen here!
                    DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
                    DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
                    DrawText("TAP SCREEN to JUMP to GAMEPLAY SCREEN", 160, 220, 20, DARKGREEN);
                    
                } break;
                case GAMEPLAY:
                { 
                    // TODO: Draw GAMEPLAY screen here!
                    DrawRectangle(0, 0, screenWidth, screenHeight, PURPLE);
                    DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);
                    DrawText("TAP SCREEN to JUMP to ENDING SCREEN", 170, 220, 20, MAROON);

                } break;
                case ENDING: 
                {
                    // TODO: Draw ENDING screen here!
                    DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
                    DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
                    DrawText("TAP SCREEN to RETURN to TITLE SCREEN", 160, 220, 20, DARKBLUE);
                    
                } break;
                default: break;
            }
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // TODO: Unload all loaded data (textures, fonts, audio) here!

    UnloadSound(fx);            // Unload sound data
    UnloadMusicStream(ambient); // Unload music stream data
    
    CloseAudioDevice();     // Close audio device (music streaming is automatically stopped)
    
    UnloadTexture(texture); // Unload texture data
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}