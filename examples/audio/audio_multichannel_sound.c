/*******************************************************************************************
*
*   raylib [audio] example - Multichannel sound playing
*
*   This example has been created using raylib 2.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Chris Camacho (@codifies) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2019 Chris Camacho (@codifies) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - Multichannel sound playing");

    InitAudioDevice();      // Initialize audio device

    Sound fxWav = LoadSound("resources/sound.wav");         // Load WAV audio file
    Sound fxOgg = LoadSound("resources/tanatana.ogg");      // Load OGG audio file
    
    int frame = 0;

    SetSoundVolume(fxWav, 0.2);
    PlaySound(fxOgg);

    bool inhibitWav = false;
    bool inhibitOgg = false;
    int maxFrame = 60;
    
    int soundsCounter = 0;

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frame++;

        if (IsKeyDown(KEY_ENTER)) inhibitWav = !inhibitWav;
        if (IsKeyDown(KEY_SPACE)) inhibitOgg = !inhibitOgg;

        // Deliberatly hammer the play pool to see what dropping old pool entries sounds like....
        if ((frame%5) == 0) 
        {
           if (!inhibitWav) PlaySoundMulti(fxWav);
        }
        
        if (frame == maxFrame) 
        {
            if (!inhibitOgg) PlaySoundMulti(fxOgg);
            
            frame = 0;
            maxFrame = GetRandomValue(6,12);
        }

        soundsCounter = GetSoundsPlaying();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawText("Multichannel sound abuse!", 200, 180, 20, LIGHTGRAY);
            DrawText("Space to inhibit new ogg triggering", 200, 200, 20, LIGHTGRAY);
            DrawText("Enter to inhibit new wav triggering", 200, 220, 20, LIGHTGRAY);

            DrawText(FormatText("Number of concurrentsounds: %i", soundsCounter), 200, 280, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    StopSoundMulti();       // We must stop the buffer pool before unloading

    UnloadSound(fxWav);     // Unload sound data
    UnloadSound(fxOgg);     // Unload sound data

    CloseAudioDevice();     // Close audio device

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
