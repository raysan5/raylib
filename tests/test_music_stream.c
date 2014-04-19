/*******************************************************************************************
*
*   raylib test - Music playing (streaming)
*
*   NOTE: This test requires OpenAL32 dll installed (or in the same folder)
*
*   This test has been created using raylib 1.1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "raylib test - music playing (streaming)");
    
    InitAudioDevice();      // Initialize audio device
    
    PlayMusicStream("resources/audio/deserve_to_be_loved.ogg");         // Load Music file
    
    int framesCounter = 0;
    float volume = 1.0;
    
    float timePlayed = 0;
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        framesCounter++;
        
        // Testing music fading
/*
        if (framesCounter > 600)
        {
            volume -= 0.01;
            
            if (volume <= 0)
            {
                volume = 1.0;
                framesCounter = -600;
                PlayMusicStream("resources/audio/destiny.ogg");
            }
            
            SetMusicVolume(volume);
        }
*/
        timePlayed = GetMusicTimePlayed() / GetMusicTimeLength() * 100 * 4; // We scale by 4 to fit 400 pixels
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawText("MUSIC SHOULD BE PLAYING!", 255, 200, 20, LIGHTGRAY);
            
            DrawRectangle(200, 250, 400, 12, LIGHTGRAY);
            DrawRectangle(200, 250, (int)timePlayed, 12, MAROON);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseAudioDevice();     // Close audio device (music streaming is automatically stopped)
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}