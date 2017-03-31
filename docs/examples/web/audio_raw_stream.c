/*******************************************************************************************
*
*   raylib [audio] example - Raw audio streaming
*
*   NOTE: This example requires OpenAL Soft library installed
*
*   This example has been created using raylib 1.6 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>         // Required for: malloc(), free()
#include <math.h>           // Required for: sinf()

#define MAX_SAMPLES             22050
#define MAX_SAMPLES_PER_UPDATE   4096

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw audio streaming");

    InitAudioDevice();              // Initialize audio device

    // Init raw audio stream (sample rate: 22050, sample size: 16bit-short, channels: 1-mono)
    AudioStream stream = InitAudioStream(22050, 16, 1);
    
    // Generate samples data from sine wave
    short *data = (short *)malloc(sizeof(short)*MAX_SAMPLES);
    
    // TODO: Review data generation, it seems data is discontinued for loop,
    // for that reason, there is a clip everytime audio stream is looped...
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        data[i] = (short)(sinf(((2*PI*(float)i)/2)*DEG2RAD)*32000);
    }
    
    PlayAudioStream(stream);        // Start processing stream buffer (no data loaded currently)
    
    int totalSamples = MAX_SAMPLES;
    int samplesLeft = totalSamples;
    
    Vector2 position = { 0, 0 };

    SetTargetFPS(30);               // Set our game to run at 30 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Refill audio stream if required
        // NOTE: Every update we check if stream data has been already consumed and we update
        // buffer with new data from the generated samples, we upload data at a rate (MAX_SAMPLES_PER_UPDATE),
        // but notice that at some point we update < MAX_SAMPLES_PER_UPDATE data...
        if (IsAudioBufferProcessed(stream)) 
        {
            int numSamples = 0;
            if (samplesLeft >= MAX_SAMPLES_PER_UPDATE) numSamples = MAX_SAMPLES_PER_UPDATE;
            else numSamples = samplesLeft;

            UpdateAudioStream(stream, data + (totalSamples - samplesLeft), numSamples);
            
            samplesLeft -= numSamples;
            
            // Reset samples feeding (loop audio)
            if (samplesLeft <= 0) samplesLeft = totalSamples;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("SINE WAVE SHOULD BE PLAYING!", 240, 140, 20, LIGHTGRAY);
            
            // NOTE: Draw a part of the sine wave (only screen width, proportional values)
            for (int i = 0; i < GetScreenWidth(); i++)
            {
                position.x = i;
                position.y = 250 + 50*data[i]/32000;
                
                DrawPixelV(position, RED);
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(data);                 // Unload sine wave data
    
    CloseAudioStream(stream);   // Close raw audio stream and delete buffers from RAM

    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}