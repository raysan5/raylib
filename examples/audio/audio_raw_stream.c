/*******************************************************************************************
*
*   raylib [audio] example - Raw audio streaming
*
*   This example has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example created by Ramon Santamaria (@raysan5) and reviewed by James Hofmann (@triplefox)
*
*   Copyright (c) 2015-2019 Ramon Santamaria (@raysan5) and James Hofmann (@triplefox)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sinf()
#include <string.h>         // Required for: memcpy()

#define MAX_SAMPLES               512
#define MAX_SAMPLES_PER_UPDATE   4096

// Cycles per second (hz)
float frequency = 440.0f;

// Audio frequency, for smoothing
float audioFrequency = 440.0f;

// Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frequency
float oldFrequency = 1.0f;

// index for audio rendering
float sineIdx = 0.0f;
  
void audioCallback(void* buffer, unsigned int nFrames, void* callbackData)
{
    audioFrequency = frequency + (audioFrequency - frequency) * 0.95f;
    audioFrequency += 1.0f;
    audioFrequency -= 1.0f;
    float incr = audioFrequency / 44100.0f;
    short* d = (short*)buffer;
    for (int i = 0; i < nFrames; i++)
    {
        d[i] = (short)(32000.0f * sinf(2 * PI * sineIdx));
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw audio streaming");

    InitAudioDevice();              // Initialize audio device

    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

    // Init raw audio stream (sample rate: 44100, sample size: 16bit-short, channels: 1-mono)
    AudioStream stream = LoadAudioStream(44100, 16, 1);

    SetAudioStreamCallback(stream, &audioCallback, NULL);

    // Buffer for the single cycle waveform we are synthesizing
    short *data = (short *)malloc(sizeof(short)*MAX_SAMPLES);

    // Frame buffer, describing the waveform when repeated over the course of a frame
    short *writeBuf = (short *)malloc(sizeof(short)*MAX_SAMPLES_PER_UPDATE);

    PlayAudioStream(stream);        // Start processing stream buffer (no data loaded currently)

    // Position read in to determine next frequency
    Vector2 mousePosition = { -100.0f, -100.0f };

    // Computed size in samples of the sine wave
    int waveLength = 1;

    Vector2 position = { 0, 0 };

    SetTargetFPS(30);               // Set our game to run at 30 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Sample mouse input.
        mousePosition = GetMousePosition();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            float fp = (float)(mousePosition.y);
            frequency = 40.0f + (float)(fp);
        }
        // Rewrite the sine wave.
        if (frequency != oldFrequency)
        {
            // Compute wavelength. Limit size in both directions.
            int oldWavelength = waveLength;
            waveLength = (int)(22050 / frequency);
            if (waveLength > MAX_SAMPLES / 2) waveLength = MAX_SAMPLES / 2;
            if (waveLength < 1) waveLength = 1;

            // Write sine wave.
            for (int i = 0; i < waveLength * 2; i++)
            {
                data[i] = (short)(sinf(((2 * PI * (float)i / waveLength))) * 32000);
            }
            oldFrequency = frequency;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText(TextFormat("sine frequency: %i",(int)frequency), GetScreenWidth() - 220, 10, 20, RED);
            DrawText("click mouse button to change frequency", 10, 10, 20, DARKGRAY);

            // Draw the current buffer state proportionate to the screen
            for (int i = 0; i < screenWidth; i++)
            {
                position.x = (float)i;
                position.y = 250 + 50*data[i*MAX_SAMPLES/screenWidth]/32000.0f;

                DrawPixelV(position, RED);
            }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(data);                 // Unload sine wave data
    free(writeBuf);             // Unload write buffer

    UnloadAudioStream(stream);   // Close raw audio stream and delete buffers from RAM
    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
