/*******************************************************************************************
*
*   raylib [audio] example - Music stream processing effects
*
*   Example originally created with raylib 4.2, last time updated with raylib 5.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h> // Required for: NULL

// Required delay effect variables
static float *delayBuffer = NULL;
static unsigned int delayBufferSize = 0;
static unsigned int delayReadIndex = 2;
static unsigned int delayWriteIndex = 0;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void AudioProcessEffectLPF(void *buffer, unsigned int frames);   // Audio effect: lowpass filter
static void AudioProcessEffectDelay(void *buffer, unsigned int frames); // Audio effect: delay

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - stream effects");

    InitAudioDevice();              // Initialize audio device

    Music music = LoadMusicStream("resources/country.mp3");

    // Allocate buffer for the delay effect
    delayBufferSize = 48000*2;      // 1 second delay (device sampleRate*channels)
    delayBuffer = (float *)RL_CALLOC(delayBufferSize, sizeof(float));

    PlayMusicStream(music);

    float timePlayed = 0.0f;        // Time played normalized [0.0f..1.0f]
    bool pause = false;             // Music playing paused
    
    bool enableEffectLPF = false;   // Enable effect low-pass-filter
    bool enableEffectDelay = false; // Enable effect delay (1 second)

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMusicStream(music);   // Update music buffer with new stream data

        // Restart music playing (stop and play)
        if (IsKeyPressed(KEY_SPACE))
        {
            StopMusicStream(music);
            PlayMusicStream(music);
        }

        // Pause/Resume music playing
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;

            if (pause) PauseMusicStream(music);
            else ResumeMusicStream(music);
        }

        // Add/Remove effect: lowpass filter
        if (IsKeyPressed(KEY_F))
        {
            enableEffectLPF = !enableEffectLPF;
            if (enableEffectLPF) AttachAudioStreamProcessor(music.stream, AudioProcessEffectLPF);
            else DetachAudioStreamProcessor(music.stream, AudioProcessEffectLPF);
        }

        // Add/Remove effect: delay
        if (IsKeyPressed(KEY_D))
        {
            enableEffectDelay = !enableEffectDelay;
            if (enableEffectDelay) AttachAudioStreamProcessor(music.stream, AudioProcessEffectDelay);
            else DetachAudioStreamProcessor(music.stream, AudioProcessEffectDelay);
        }
        
        // Get normalized time played for current music stream
        timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music);

        if (timePlayed > 1.0f) timePlayed = 1.0f;   // Make sure time played is no longer than music
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("MUSIC SHOULD BE PLAYING!", 245, 150, 20, LIGHTGRAY);

            DrawRectangle(200, 180, 400, 12, LIGHTGRAY);
            DrawRectangle(200, 180, (int)(timePlayed*400.0f), 12, MAROON);
            DrawRectangleLines(200, 180, 400, 12, GRAY);

            DrawText("PRESS SPACE TO RESTART MUSIC", 215, 230, 20, LIGHTGRAY);
            DrawText("PRESS P TO PAUSE/RESUME MUSIC", 208, 260, 20, LIGHTGRAY);
            
            DrawText(TextFormat("PRESS F TO TOGGLE LPF EFFECT: %s", enableEffectLPF? "ON" : "OFF"), 200, 320, 20, GRAY);
            DrawText(TextFormat("PRESS D TO TOGGLE DELAY EFFECT: %s", enableEffectDelay? "ON" : "OFF"), 180, 350, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadMusicStream(music);   // Unload music stream buffers from RAM

    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    RL_FREE(delayBuffer);       // Free delay buffer

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
// Audio effect: lowpass filter
static void AudioProcessEffectLPF(void *buffer, unsigned int frames)
{
    static float low[2] = { 0.0f, 0.0f };
    static const float cutoff = 70.0f / 44100.0f; // 70 Hz lowpass filter
    const float k = cutoff / (cutoff + 0.1591549431f); // RC filter formula

    // Converts the buffer data before using it
    float *bufferData = (float *)buffer;
    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        const float l = bufferData[i];
        const float r = bufferData[i + 1];

        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        bufferData[i] = low[0];
        bufferData[i + 1] = low[1];
    }
}

// Audio effect: delay
static void AudioProcessEffectDelay(void *buffer, unsigned int frames)
{
    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        float leftDelay = delayBuffer[delayReadIndex++];    // ERROR: Reading buffer -> WHY??? Maybe thread related???
        float rightDelay = delayBuffer[delayReadIndex++];

        if (delayReadIndex == delayBufferSize) delayReadIndex = 0;

        ((float *)buffer)[i] = 0.5f*((float *)buffer)[i] + 0.5f*leftDelay;
        ((float *)buffer)[i + 1] = 0.5f*((float *)buffer)[i + 1] + 0.5f*rightDelay;

        delayBuffer[delayWriteIndex++] = ((float *)buffer)[i];
        delayBuffer[delayWriteIndex++] = ((float *)buffer)[i + 1];
        if (delayWriteIndex == delayBufferSize) delayWriteIndex = 0;
    }
}
