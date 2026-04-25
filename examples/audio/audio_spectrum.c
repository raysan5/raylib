/*******************************************************************************************
*
*   raylib [audio] example - Audio Spectrum Analyzer
*
*   Example originally created with raylib, using raudio_analyzer module
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 raylib contributors
*
********************************************************************************************/

#include "raylib.h"
#include "../src/raudio_analyzer.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   450
#define NUM_BARS         64     // Number of frequency bars to draw
#define FFT_SIZE        512     // FFT bin count (must be power of 2)

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib [audio] example - spectrum analyzer");
    InitAudioDevice();

    Music music = LoadMusicStream("resources/guitar_noodling.ogg");
    PlayMusicStream(music);

    // Configure and initialize spectrum analyzer
    SpectrumConfig config = {
        .binCount = FFT_SIZE,
        .windowFunc = WINDOW_HANN,
        .smoothingFactor = 0.8f,
        .peakDecayRate = 0.95f,
        .sampleRate = 44100,
    };
    InitSpectrumAnalyzer(config);

    float samples[FFT_SIZE] = { 0 };   // Buffer for audio samples

    SetTargetFPS(60);
    //--------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------
        UpdateMusicStream(music);

        // Feed samples into spectrum analyzer
        UpdateSpectrum(samples, FFT_SIZE);
        ApplySmoothing();

        float *spectrum = GetSpectrumData();
        //----------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw frequency bars
            int barWidth = SCREEN_WIDTH / NUM_BARS;
            int binsPerBar = (FFT_SIZE / 2) / NUM_BARS;
            if (binsPerBar < 1) binsPerBar = 1;

            for (int i = 0; i < NUM_BARS; i++)
            {
                // Average magnitudes across bins for this bar
                float magnitude = 0.0f;
                for (int j = 0; j < binsPerBar; j++)
                {
                    int binIndex = i * binsPerBar + j;
                    if (binIndex < FFT_SIZE / 2 && spectrum != NULL)
                    {
                        magnitude += spectrum[binIndex];
                    }
                }
                magnitude /= (float)binsPerBar;

                // Scale for visual display
                int barHeight = (int)(magnitude * SCREEN_HEIGHT * 4.0f);
                if (barHeight > SCREEN_HEIGHT) barHeight = SCREEN_HEIGHT;

                // Color gradient from green (low) to red (high)
                Color barColor = (Color){
                    (unsigned char)(255 * i / NUM_BARS),
                    (unsigned char)(255 * (NUM_BARS - i) / NUM_BARS),
                    50,
                    255
                };

                DrawRectangle(
                    i * barWidth,
                    SCREEN_HEIGHT - barHeight,
                    barWidth - 2,
                    barHeight,
                    barColor
                );
            }

            // Display info
            DrawText("AUDIO SPECTRUM ANALYZER", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Peak Freq: %.1f Hz", GetPeakFrequency()), 10, 40, 16, GRAY);
            DrawText(TextFormat("Peak Mag:  %.4f", GetPeakMagnitude()), 10, 60, 16, GRAY);
            DrawFPS(SCREEN_WIDTH - 90, 10);

        EndDrawing();
        //----------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------
    CloseSpectrumAnalyzer();
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
    //--------------------------------------------------------------------------

    return 0;
}
