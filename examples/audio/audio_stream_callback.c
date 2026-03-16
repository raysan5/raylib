/*******************************************************************************************
*
*   raylib [audio] example - stream callback
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example created by Dan Hoang (@dan-hoang) and reviewed by Ramon Santamaria (@raysan5)
*
*   NOTE: Example sends a wave to the audio device,
*     user gets the choice of four waves: sine, square, triangle, and sawtooth
*     A stream is set up to play to the audio device; stream is hooked to a callback that
*     generates a wave, that is determined by user choice
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Dan Hoang (@dan-hoang)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>
#include <math.h>

#define BUFFER_SIZE 4096
#define SAMPLE_RATE 44100

// Wave type
typedef enum {
    SINE,
    SQUARE,
    TRIANGLE,
    SAWTOOTH
} WaveType;

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void SineCallback(void *framesOut, unsigned int frameCount);
static void SquareCallback(void *framesOut, unsigned int frameCount);
static void TriangleCallback(void *framesOut, unsigned int frameCount);
static void SawtoothCallback(void *framesOut, unsigned int frameCount);

static int waveFrequency = 440;
static int newWaveFrequency = 440;
static int waveIndex = 0;

// Buffer to keep the last second of uploaded audio,
// part of which will be drawn on the screen
static float buffer[SAMPLE_RATE] = { 0 };
static AudioCallback waveCallbacks[] = { SineCallback, SquareCallback, TriangleCallback, SawtoothCallback };
static char *waveTypesAsString[] = { "sine", "square", "triangle", "sawtooth" };

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - stream callback");

    InitAudioDevice();

    // Set the number of samples the stream will keep in memory at a time to BUFFER_SIZE
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);

    // Init raw audio stream (sample rate: 44100, sample size: 32bit-float, channels: 1-mono)
    AudioStream stream = LoadAudioStream(SAMPLE_RATE, 32, 1);
    PlayAudioStream(stream);

    // Configure it so that waveCallbacks[waveType] is called whenever stream is out of samples
    WaveType waveType = SINE;
    SetAudioStreamCallback(stream, waveCallbacks[waveType]);

    SetTargetFPS(30);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_UP))
        {
            newWaveFrequency += 10;
            if (newWaveFrequency > 12500) newWaveFrequency = 12500;
        }

        if (IsKeyDown(KEY_DOWN))
        {
            newWaveFrequency -= 10;
            if (newWaveFrequency < 20) newWaveFrequency = 20;
        }

        if (IsKeyPressed(KEY_LEFT))
        {
            if (waveType == SINE) waveType = SAWTOOTH;
            else if (waveType == SQUARE) waveType = SINE;
            else if (waveType == TRIANGLE) waveType = SQUARE;
            else waveType = TRIANGLE;

            SetAudioStreamCallback(stream, waveCallbacks[waveType]);
        }

        if (IsKeyPressed(KEY_RIGHT))
        {
            if (waveType == SINE) waveType = SQUARE;
            else if (waveType == SQUARE) waveType = TRIANGLE;
            else if (waveType == TRIANGLE) waveType = SAWTOOTH;
            else waveType = SINE;

            SetAudioStreamCallback(stream, waveCallbacks[waveType]);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawText(TextFormat("frequency: %i", newWaveFrequency), screenWidth - 220, 10, 20, RED);
            DrawText(TextFormat("wave type: %s", waveTypesAsString[waveType]), screenWidth - 220, 30, 20, RED);
            DrawText("Up/down to change frequency", 10, 10, 20, DARKGRAY);
            DrawText("Left/right to change wave type", 10, 30, 20, DARKGRAY);

            // Draw the last 10 ms of uploaded audio
            for (int i = 0; i < screenWidth; i++)
            {
                Vector2 startPos = { i, 250 - 50*buffer[SAMPLE_RATE - SAMPLE_RATE/100 + i*SAMPLE_RATE/100/screenWidth] };
                Vector2 endPos = { i + 1, 250 - 50*buffer[SAMPLE_RATE - SAMPLE_RATE/100 + (i + 1)*SAMPLE_RATE/100/screenWidth] };
                DrawLineV(startPos, endPos, RED);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadAudioStream(stream);  // Close raw audio stream and delete buffers from RAM
    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
static void SineCallback(void *framesOut, unsigned int frameCount)
{
    int wavelength = SAMPLE_RATE/waveFrequency;

    // Synthesize the sine wave
    for (int i = 0; i < frameCount; i++)
    {
        ((float *)framesOut)[i] = sin(2*PI*waveIndex/wavelength);

        waveIndex++;

        if (waveIndex >= wavelength)
        {
            waveFrequency = newWaveFrequency;
            waveIndex = 0;
        }
    }

    // Save the synthesized samples for later drawing
    for (int i = 0; i < SAMPLE_RATE - frameCount; i++) buffer[i] = buffer[i + frameCount];
    for (int i = 0; i < frameCount; i++) buffer[SAMPLE_RATE - frameCount + i] = ((float *)framesOut)[i];
}

static void SquareCallback(void *framesOut, unsigned int frameCount)
{
    int wavelength = SAMPLE_RATE/waveFrequency;

    // Synthesize the square wave
    for (int i = 0; i < frameCount; i++)
    {
        ((float *)framesOut)[i] = (waveIndex < wavelength/2)? 1 : -1;
        waveIndex++;

        if (waveIndex >= wavelength)
        {
            waveFrequency = newWaveFrequency;
            waveIndex = 0;
        }
    }

    // Save the synthesized samples for later drawing
    for (int i = 0; i < SAMPLE_RATE - frameCount; i++) buffer[i] = buffer[i + frameCount];
    for (int i = 0; i < frameCount; i++) buffer[SAMPLE_RATE - frameCount + i] = ((float *)framesOut)[i];
}

static void TriangleCallback(void *framesOut, unsigned int frameCount)
{
    int wavelength = SAMPLE_RATE/waveFrequency;

    // Synthesize the triangle wave
    for (int i = 0; i < frameCount; i++)
    {
        ((float *)framesOut)[i] = (waveIndex < wavelength/2)? (-1 + 2.0f*waveIndex/(wavelength/2)) : (1 - 2.0f*(waveIndex - wavelength/2)/(wavelength/2));
        waveIndex++;

        if (waveIndex >= wavelength)
        {
            waveFrequency = newWaveFrequency;
            waveIndex = 0;
        }
    }

    // Save the synthesized samples for later drawing
    for (int i = 0; i < SAMPLE_RATE - frameCount; i++) buffer[i] = buffer[i + frameCount];
    for (int i = 0; i < frameCount; i++) buffer[SAMPLE_RATE - frameCount + i] = ((float *)framesOut)[i];
}

static void SawtoothCallback(void *framesOut, unsigned int frameCount)
{
    int wavelength = SAMPLE_RATE/waveFrequency;

    // Synthesize the sawtooth wave
    for (int i = 0; i < frameCount; i++)
    {
        ((float *)framesOut)[i] = -1 + 2.0f*waveIndex/wavelength;
        waveIndex++;

        if (waveIndex >= wavelength)
        {
            waveFrequency = newWaveFrequency;
            waveIndex = 0;
        }
    }

    // Save the synthesized samples for later drawing
    for (int i = 0; i < SAMPLE_RATE - frameCount; i++) buffer[i] = buffer[i + frameCount];
    for (int i = 0; i < frameCount; i++) buffer[SAMPLE_RATE - frameCount + i] = ((float *)framesOut)[i];
}
