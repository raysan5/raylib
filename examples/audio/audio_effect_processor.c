/*******************************************************************************************
*
*   raylib [audio] example - Audio effects (streaming)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <stdlib.h>

#define AUDIO_THREAD_MUSIC_UPDATE true

// a simple lowpass filter applied to the music stream
static void processFilterEffect(float* buffer, unsigned int nframes)
{
    static float low[2] = { 0.0f, 0.0f };
    static const float cutoff = 70.0f / 44100.0f; // 70 Hz lowpass filter
    const float k = cutoff / (cutoff + 0.1591549431f); // RC filter formula

    for (unsigned int i = 0; i < nframes*2; i+=2)
    {
        float l = buffer[i], r = buffer[i + 1];
        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        buffer[i] = low[0];
        buffer[i + 1] = low[1];
    }
}


float* delayBuffer = NULL;
unsigned int delayBufferSize = 0;
unsigned int delayReadIndex = 2;
unsigned int delayWriteIndex = 0;
static void processDelayEffect(float* buffer, unsigned int nframes)
{
    for (unsigned int i = 0; i < nframes*2; i+=2)
    {
        float leftDelay = delayBuffer[delayReadIndex++];
        float rightDelay = delayBuffer[delayReadIndex++];
        if (delayReadIndex == delayBufferSize) delayReadIndex = 0;

        buffer[i] = 0.5f * buffer[i] + 0.5f * leftDelay;
        buffer[i+1] = 0.5f * buffer[i+1] + 0.5f * rightDelay;

        delayBuffer[delayWriteIndex++] = buffer[i];
        delayBuffer[delayWriteIndex++] = buffer[i+1];
        if (delayWriteIndex == delayBufferSize) delayWriteIndex = 0;
    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - audio effects");

    AudioDeviceInfo audioDevice = InitAudioDevice();              // Initialize audio device
    if (!audioDevice.channels) return -1;
    
    // allocate buffer for the delay effect
    delayBufferSize = audioDevice.channels * audioDevice.sampleRate; // 1s delay
    delayBuffer = (float*)RL_CALLOC(delayBufferSize, sizeof(float));

    Music music = LoadMusicStream("resources/country.mp3");
    music.background = AUDIO_THREAD_MUSIC_UPDATE;

    PlayMusicStream(music);

    float timePlayed = 0.0f;
    bool pause = false;
    bool hasFilter = false;
    bool hasDelay = false;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (!AUDIO_THREAD_MUSIC_UPDATE)
        {
            UpdateMusicStream(music);   // Update music buffer with new stream data
        }

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

        // Add/Remove effects
        if (IsKeyPressed(KEY_F))
        {
            hasFilter = !hasFilter;
            if (hasFilter)
            {
                AddAudioStreamProcessor(music.stream, &processFilterEffect);
            }
            else
            {
                RemoveAudioStreamProcessor(music.stream, &processFilterEffect);
            }
        }

        if (IsKeyPressed(KEY_D))
        {
            hasDelay = !hasDelay;
            if (hasDelay)
            {
                AddAudioStreamProcessor(music.stream, &processDelayEffect);
            }
            else
            {
                RemoveAudioStreamProcessor(music.stream, &processDelayEffect);
            }
        }

        // Get timePlayed scaled to bar dimensions (400 pixels)
        timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music)*400;

        if (timePlayed > 400) StopMusicStream(music);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("MUSIC SHOULD BE PLAYING!", 255, 150, 20, LIGHTGRAY);

            DrawRectangle(200, 200, 400, 12, LIGHTGRAY);
            DrawRectangle(200, 200, (int)timePlayed, 12, MAROON);
            DrawRectangleLines(200, 200, 400, 12, GRAY);
            DrawRectangleLines(200, 200, 400, 12, GRAY);

            DrawText("PRESS SPACE TO RESTART MUSIC", 215, 250, 20, LIGHTGRAY);
            DrawText("PRESS P TO PAUSE/RESUME MUSIC", 208, 280, 20, LIGHTGRAY);
            DrawText("PRESS F TO ADD/REMOVE FILTER EFFECT", 180, 310, 20, LIGHTGRAY);
            DrawText("PRESS D TO ADD/REMOVE DELAY EFFECT", 180, 340, 20, LIGHTGRAY);

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