/*******************************************************************************************
*
*   raylib [audio] example - Music stream processing effects
*
*   Example originally created with raylib 4.2, last time updated with raylib 4.2
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>         // Required for: NULL
#include <switch.h>

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
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Initialize resource directory
    romfsInit();
    InitWindow(screenWidth, screenHeight, "raylib [audio] example - stream effects");

    InitAudioDevice();              // Initialize audio device

    Music music = LoadMusicStream("romfs:/resources/country.mp3");

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
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT))
        {
            StopMusicStream(music);
            PlayMusicStream(music);
        }

        // Pause/Resume music playing
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
        {
            pause = !pause;

            if (pause) PauseMusicStream(music);
            else ResumeMusicStream(music);
        }

        // Add/Remove effect: lowpass filter
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP))
        {
            enableEffectLPF = !enableEffectLPF;
            if (enableEffectLPF) AttachAudioStreamProcessor(music.stream, AudioProcessEffectLPF);
            else DetachAudioStreamProcessor(music.stream, AudioProcessEffectLPF);
        }

        // Add/Remove effect: delay
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
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

            DrawText("MUSIC SHOULD BE PLAYING!", screenWidth/2 - 200, 230, 20, LIGHTGRAY);

            DrawRectangle(screenWidth/2 - 200, 260, 400, 12, LIGHTGRAY);
            DrawRectangle(screenWidth/2 - 200, 260, (int)(timePlayed*400.0f), 12, MAROON);
            DrawRectangleLines(screenWidth/2 - 200, 260, 400, 12, GRAY);

            DrawText("PRESS Y button TO RESTART MUSIC", screenWidth/2 - 200, 310, 20, LIGHTGRAY);
            DrawText("PRESS A button TO PAUSE/RESUME MUSIC", screenWidth/2 - 200, 340, 20, LIGHTGRAY);
            
            DrawText(TextFormat("PRESS X button TO TOGGLE LPF EFFECT: %s", enableEffectLPF? "ON" : "OFF"), screenWidth/2 - 200, 400, 20, GRAY);
            DrawText(TextFormat("PRESS B button TO TOGGLE DELAY EFFECT: %s", enableEffectDelay? "ON" : "OFF"), screenWidth/2 - 200, 430, 20, GRAY);

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

    romfsExit();
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

    for (unsigned int i = 0; i < frames*2; i += 2)
    {
        float l = ((float *)buffer)[i], r = ((float *)buffer)[i + 1];
        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        ((float *)buffer)[i] = low[0];
        ((float *)buffer)[i + 1] = low[1];
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