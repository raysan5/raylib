/*******************************************************************************************
*
*   raylib [audio] example - raw stream
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 1.6, last time updated with raylib 4.2
*
*   Example created by Ramon Santamaria (@raysan5) and reviewed by James Hofmann (@triplefox)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2025 Ramon Santamaria (@raysan5) and James Hofmann (@triplefox)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>

#define BUFFER_SIZE 4096
#define SAMPLE_RATE 44100

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw stream");

    InitAudioDevice();

    // Set the number of samples the stream will keep in memory at a time to BUFFER_SIZE
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    float buffer[BUFFER_SIZE] = {};

    // Init raw audio stream (sample rate: 44100, sample size: 32bit-float, channels: 1-mono)
    AudioStream stream = LoadAudioStream(SAMPLE_RATE, 32, 1);
    float pan = 0.0f;
    SetAudioStreamPan(stream, pan);
    PlayAudioStream(stream);

    int sineFrequency = 440;
    int newSineFrequency = 440;
    int sineIndex = 0;
    double sineStartTime = 0.0;

    SetTargetFPS(30);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        if (IsKeyDown(KEY_UP))
        {
            newSineFrequency += 10;
            if (newSineFrequency > 12500) newSineFrequency = 12500;
        }

        if (IsKeyDown(KEY_DOWN))
        {
            newSineFrequency -= 10;
            if (newSineFrequency < 20) newSineFrequency = 20;
        }

        if (IsKeyDown(KEY_LEFT))
        {
            pan -= 0.01f;
            if (pan < -1.0f) pan = -1.0f;
            SetAudioStreamPan(stream, pan);
        }

        if (IsKeyDown(KEY_RIGHT))
        {
            pan += 0.01f;
            if (pan > 1.0f) pan = 1.0f;
            SetAudioStreamPan(stream, pan);
        }

        if (IsAudioStreamProcessed(stream))
        {
            for (int i = 0; i < BUFFER_SIZE; i++)
            {
                int wavelength = SAMPLE_RATE/sineFrequency;
                buffer[i] = sin(2*PI*sineIndex/wavelength);
                sineIndex++;

                if (sineIndex >= wavelength)
                {
                    sineFrequency = newSineFrequency;
                    sineIndex = 0;
                    sineStartTime = GetTime();
                }
            }

            UpdateAudioStream(stream, buffer, BUFFER_SIZE);
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText(TextFormat("sine frequency: %i", sineFrequency), screenWidth - 220, 10, 20, RED);
        DrawText(TextFormat("pan: %.2f", pan), screenWidth - 220, 30, 20, RED);
        DrawText("Up/down to change frequency", 10, 10, 20, DARKGRAY);
        DrawText("Left/right to pan", 10, 30, 20, DARKGRAY);

        int windowStart = (GetTime() - sineStartTime)*SAMPLE_RATE;
        int windowSize = 0.1f*SAMPLE_RATE;
        int wavelength = SAMPLE_RATE/sineFrequency;

        // Draw a sine wave with the same frequency as the one being sent to the audio stream
        for (int i = 0; i < screenWidth; i++)
        {
            int t0 = windowStart + i*windowSize/screenWidth;
            int t1 = windowStart + (i + 1)*windowSize/screenWidth;
            Vector2 startPos = { i, 250 + 50*sin(2*PI*t0/wavelength) };
            Vector2 endPos = { i + 1, 250 + 50*sin(2*PI*t1/wavelength) };
            DrawLineV(startPos, endPos, RED);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadAudioStream(stream);   // Close raw audio stream and delete buffers from RAM
    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
