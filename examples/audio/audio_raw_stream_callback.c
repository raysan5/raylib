/*******************************************************************************************
*
*   raylib [audio] example - raw stream
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 1.6, last time updated with raylib 6.0
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

#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sinf()

enum Flags { FLAG_CHANNEL_MONO = 1u<<0, FLAG_SAMPLESIZE_SHORT = 1u<<1 };
static unsigned int gflags = FLAG_CHANNEL_MONO | FLAG_SAMPLESIZE_SHORT; // mono + 16-bit to match initial stream specs
#define CHANNEL_MONO() ((gflags & FLAG_CHANNEL_MONO) != 0)
#define SAMPLESIZE_SHORT() ((gflags & FLAG_SAMPLESIZE_SHORT) != 0)
#define TOGGLE(K, F) do { if (IsKeyPressed(K)) { gflags ^= (F); } } while (0)

#define MAX_SAMPLES               512
#define MAX_SAMPLES_PER_UPDATE   4096

// Cycles per second (hz)
float frequency = 440.0f;

// Audio frequency, for smoothing
float audioFrequency = 440.0f;

// Previous value, used to test if sine needs to be rewritten, and to smoothly modulate frequency
float oldFrequency = 1.0f;

// Index for audio rendering
float sineIdx = 0.0f;

// Audio input processing callback
void AudioInputCallbackMonoShort(void *buffer, unsigned int frames)
{
    audioFrequency = frequency + (audioFrequency - frequency)*0.95f;

    float incr = audioFrequency/44100.0f;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        d[i] = (short)(32000.0f*sinf(2*PI*sineIdx));
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

void AudioInputCallbackStereoShort(void *buffer, unsigned int frames)
{
    audioFrequency = frequency + (audioFrequency - frequency)*0.95f;

    float incr = audioFrequency/44100.0f;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        short s = (short)(32000.0f*sinf(2*PI*sineIdx));
        d[2*i + 0] = s;   // L
        d[2*i + 1] = s;   // R
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

void AudioInputCallbackMonoFloat(void *buffer, unsigned int frames)
{
    audioFrequency = frequency + (audioFrequency - frequency)*0.95f;

    float incr = audioFrequency/44100.0f;
    float *d = (float *)buffer;

    for (unsigned int i = 0; i < frames; i++)
    {
        d[i] = sinf(2*PI*sineIdx);
        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

void AudioInputCallbackStereoFloat(void *buffer, unsigned int frames)
{
    audioFrequency = frequency + (audioFrequency - frequency)*0.95f;
    float incr = audioFrequency/44100.0f;
    float *d = (float *)buffer;
    for (unsigned int i = 0; i < frames; i++)
    {
        float s = sinf(2*PI*sineIdx);
        d[2*i + 0] = s;
        d[2*i + 1] = s;

        sineIdx += incr;
        if (sineIdx > 1.0f) sineIdx -= 1.0f;
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw stream with callbacks");

    InitAudioDevice();              // Initialize audio device

    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

    // Init raw audio stream (sample rate: 44100, sample size: 16bit-short, channels: 1-mono)
    AudioStream stream = LoadAudioStream(44100, 16, 1);

    SetAudioStreamCallback(stream, AudioInputCallbackMonoShort);
    unsigned int previousSampleSize = stream.sampleSize;
    unsigned int previousChannels   = stream.channels;

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
        TOGGLE(KEY_M, FLAG_CHANNEL_MONO);
        TOGGLE(KEY_F, FLAG_SAMPLESIZE_SHORT);
        unsigned int nextSampleSize = (SAMPLESIZE_SHORT())? 16u : 32u;
        unsigned int nextChannels = (CHANNEL_MONO())? 1u  : 2u;
        if (nextSampleSize != previousSampleSize || nextChannels != previousChannels)
        {
            StopAudioStream(stream);
            UnloadAudioStream(stream);
            stream = LoadAudioStream(44100, nextSampleSize, nextChannels);
            // CORRECT ALIGNMENT
            if (nextChannels == 1 && nextSampleSize == 16) SetAudioStreamCallback(stream, AudioInputCallbackMonoShort);
            if (nextChannels == 2 && nextSampleSize == 16) SetAudioStreamCallback(stream, AudioInputCallbackStereoShort);
            if (nextChannels == 1 && nextSampleSize == 32) SetAudioStreamCallback(stream, AudioInputCallbackMonoFloat);
            if (nextChannels == 2 && nextSampleSize == 32) SetAudioStreamCallback(stream, AudioInputCallbackStereoFloat);

            // INCORRECT ALIGNMENT TESTS: comment and uncomment or add your own to observe common misconfigurations
            // if (nextChannels == 1 && nextSampleSize == 16) SetAudioStreamCallback(stream, AudioInputCallbackStereoShort);
            // if (nextChannels == 1 && nextSampleSize == 32) SetAudioStreamCallback(stream, AudioInputCallbackMonoShort);
            // if (nextChannels == 2 && nextSampleSize == 32) SetAudioStreamCallback(stream, AudioInputCallbackMonoShort);
            // if (nextChannels == 2 && nextSampleSize == 16) SetAudioStreamCallback(stream, AudioInputCallbackMonoFloat);
            // if (nextChannels == 2 && nextSampleSize == 16) SetAudioStreamCallback(stream, AudioInputCallbackStereoFloat);

            PlayAudioStream(stream);
            previousSampleSize = nextSampleSize;
            previousChannels = nextChannels;
        }
        // Update
        //----------------------------------------------------------------------------------
        mousePosition = GetMousePosition();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            float fp = (float)(mousePosition.y);
            frequency = 40.0f + (float)(fp);

            float pan = (float)(mousePosition.x)/(float)screenWidth;
            SetAudioStreamPan(stream, pan);
        }

        // Rewrite the sine wave
        // Compute two cycles to allow the buffer padding, simplifying any modulation, resampling, etc.
        if (frequency != oldFrequency)
        {
            // Compute wavelength. Limit size in both directions
            waveLength = (int)(22050/frequency);
            if (waveLength > MAX_SAMPLES/2) waveLength = MAX_SAMPLES/2;
            if (waveLength < 1) waveLength = 1;

            // Write sine wave
            for (int i = 0; i < waveLength*2; i++)
            {
                data[i] = (short)(sinf(((2*PI*(float)i/waveLength)))*32000);
            }
            // Make sure the rest of the line is flat
            for (int j = waveLength*2; j < MAX_SAMPLES; j++)
            {
                data[j] = (short)0;
            }

            oldFrequency = frequency;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText(TextFormat("sine frequency: %i",(int)frequency), GetScreenWidth() - 220, 10, 20, RED);
            DrawText("click mouse button to change frequency or pan", 10, 10, 20, DARKGRAY);
            DrawText("press M to SWAP channels [ M ]:", 250, 366, 20, BLUE);
            DrawText((CHANNEL_MONO())? "MONO" : "STEREO", 600, 366, 20, (CHANNEL_MONO())? GREEN : RED);
            DrawText("press F to SWAP Sample Size [ F ]:", 250, 400, 20, BLUE);
            DrawText((SAMPLESIZE_SHORT())? "16" : "32", 620, 400, 20, (SAMPLESIZE_SHORT())? GREEN : RED);

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