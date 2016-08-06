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

#define MAX_SAMPLES      20000

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw audio streaming");

    InitAudioDevice();              // Initialize audio device

    // Init raw audio stream (sample rate: 22050, sample size: 32bit-float, channels: 1-mono)
    AudioStream stream = InitAudioStream(22050, 32, 1);
    
    // Fill audio stream with some samples (sine wave)
    float *data = (float *)malloc(sizeof(float)*MAX_SAMPLES);
    
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        data[i] = sinf(((2*PI*(float)i)/2)*DEG2RAD);
    }
    
    // NOTE: The generated MAX_SAMPLES do not fit to close a perfect loop
    // for that reason, there is a clip everytime audio stream is looped
    
    PlayAudioStream(stream);
    
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
        if (IsAudioBufferProcessed(stream)) 
        {
            int numSamples = 0;
            if (samplesLeft >= 4096) numSamples = 4096;
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
            
            // NOTE: Draw a part of the sine wave (only screen width)
            for (int i = 0; i < GetScreenWidth(); i++)
            {
                position.x = i;
                position.y = 250 + 50*data[i];
                
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