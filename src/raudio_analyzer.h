/**********************************************************************************************
*
*   raudio_analyzer - Audio spectrum analysis module for raylib
*
*   DESCRIPTION:
*       FFT-based audio spectrum analysis with windowing functions, peak detection,
*       and smoothing for real-time audio visualization.
*
*   FEATURES:
*       - Radix-2 Cooley-Tukey FFT algorithm
*       - Multiple windowing functions (Hann, Hamming, Blackman, Rectangular)
*       - Configurable frequency bin count
*       - Peak detection with configurable decay rate
*       - Temporal smoothing for stable visualization
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2024 raylib contributors
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RAUDIO_ANALYZER_H
#define RAUDIO_ANALYZER_H

#include <stdbool.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef MAX_SPECTRUM_BINS
    #define MAX_SPECTRUM_BINS       1024
#endif
#ifndef DEFAULT_SMOOTHING_FACTOR
    #define DEFAULT_SMOOTHING_FACTOR  0.8f
#endif
#ifndef DEFAULT_PEAK_DECAY_RATE
    #define DEFAULT_PEAK_DECAY_RATE   0.95f
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Window function types for FFT preprocessing
typedef enum {
    WINDOW_RECTANGULAR = 0,     // No windowing (rectangular/boxcar)
    WINDOW_HANN,                // Hann (raised cosine) window
    WINDOW_HAMMING,             // Hamming window
    WINDOW_BLACKMAN             // Blackman window
} WindowFunction;

// Spectrum analyzer configuration
typedef struct {
    int binCount;               // Number of frequency bins (must be power of 2)
    WindowFunction windowFunc;  // Window function to apply before FFT
    float smoothingFactor;      // Temporal smoothing factor [0.0 - 1.0]
    float peakDecayRate;        // Peak value decay rate per frame [0.0 - 1.0]
    int sampleRate;             // Audio sample rate in Hz
} SpectrumConfig;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Lifecycle management
void InitSpectrumAnalyzer(SpectrumConfig config);   // Initialize spectrum analyzer with config
void CloseSpectrumAnalyzer(void);                   // Free all allocated resources

// Spectrum processing
void UpdateSpectrum(float *samples, int sampleCount);   // Process audio samples through FFT
void ApplySmoothing(void);                              // Apply temporal smoothing to spectrum data

// Data retrieval
float *GetSpectrumData(void);       // Get array of frequency bin magnitudes
float GetPeakFrequency(void);       // Get the dominant frequency in Hz
float GetPeakMagnitude(void);       // Get the magnitude of the peak frequency bin
int GetSpectrumBinCount(void);      // Get the current number of frequency bins
bool IsSpectrumReady(void);         // Check if analyzer is initialized and ready

#endif // RAUDIO_ANALYZER_H
