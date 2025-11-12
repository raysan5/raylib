/*******************************************************************************************
*
*   raylib [audio] example - sound loading
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 1.1, last time updated with raylib 6.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2014-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <stdlib.h> // Required for: malloc(), free()
#include <string.h> // Required for: memcpy()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - sound loading");

    InitAudioDevice();      // Initialize audio device

    Sound fxWav = LoadSound("resources/sound.wav");         // Load WAV audio file
    Sound fxOgg = LoadSound("resources/target.ogg");        // Load OGG audio file

    bool soundReversed = false;

    float *soundData = malloc(sizeof(float)*fxWav.frameCount*fxWav.stream.channels);
    float *scratchSoundData = malloc(sizeof(float)*fxWav.frameCount*fxWav.stream.channels);

    Wave wave = LoadWave("resources/sound.wav");
    // Sounds always have 32bit sampleSize:
    WaveFormat(&wave, fxWav.stream.sampleRate, 32, fxWav.stream.channels);
    memcpy(soundData, wave.data, sizeof(float)*fxWav.frameCount*fxWav.stream.channels);
    UnloadWave(wave);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_R))
        {
            soundReversed = !soundReversed;
            for (unsigned int i = 0; i < fxWav.frameCount; i++)
            {
                unsigned int src = (soundReversed)? fxWav.frameCount - 1 - i : i;
                // Sounds always have STEREO channels:
                scratchSoundData[i * fxWav.stream.channels + 0] = soundData[src * fxWav.stream.channels + 0];
                scratchSoundData[i * fxWav.stream.channels + 1] = soundData[src * fxWav.stream.channels + 1];
            }
            UpdateSound(fxWav, scratchSoundData, fxWav.frameCount);
        }
        if (IsKeyPressed(KEY_SPACE)) PlaySound(fxWav);      // Play WAV sound
        if (IsKeyPressed(KEY_ENTER)) PlaySound(fxOgg);      // Play OGG sound
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Press SPACE to PLAY the WAV sound!", 200, 180, 20, LIGHTGRAY);
            DrawText(TextFormat("Press R to REVERSE the WAV sound : "), 120, 220, 20, LIGHTGRAY);
            DrawText((soundReversed)? "BACKWARDS" : "FORWARDS", 525, 220, 20, (soundReversed)? MAROON : DARKGREEN);
            DrawText("Press ENTER to PLAY the OGG sound!", 200, 260, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(soundData);
    free(scratchSoundData);
    UnloadSound(fxWav);     // Unload sound data
    UnloadSound(fxOgg);     // Unload sound data

    CloseAudioDevice();     // Close audio device

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}