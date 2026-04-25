/**********************************************************************************************
*
*   raudio_analyzer - Audio spectrum analysis module for raylib
*
*   IMPLEMENTATION:
*       Radix-2 Cooley-Tukey FFT with windowing, peak detection, and temporal smoothing.
*       Handles non-power-of-2 input by padding/truncating to nearest power of 2.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2024 raylib contributors
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
**********************************************************************************************/

#include "raudio_analyzer.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef PI
    #define PI 3.14159265358979323846
#endif

//----------------------------------------------------------------------------------
// Internal state
//----------------------------------------------------------------------------------
static float *spectrumMagnitudes = NULL;     // Current magnitude per frequency bin
static float *spectrumSmoothed = NULL;       // Smoothed magnitude per frequency bin
static float *peakValues = NULL;             // Peak-hold values per bin
static float *fftReal = NULL;                // FFT real component buffer
static float *fftImag = NULL;                // FFT imaginary component buffer
static float *windowCoeffs = NULL;           // Precomputed window coefficients
static SpectrumConfig analyzerConfig = { 0 };
static bool analyzerReady = false;

//----------------------------------------------------------------------------------
// Internal helpers: forward declarations
//----------------------------------------------------------------------------------
static int NextPowerOfTwo(int n);
static void BitReversalPermutation(float *real, float *imag, int n);
static void ComputeFFT(float *real, float *imag, int n);
static void ComputeWindowCoefficients(float *coeffs, int n, WindowFunction func);

//----------------------------------------------------------------------------------
// Lifecycle management
//----------------------------------------------------------------------------------

// Initialize the spectrum analyzer, allocating all internal buffers
void InitSpectrumAnalyzer(SpectrumConfig config)
{
    // Ensure bin count is a power of 2
    config.binCount = NextPowerOfTwo(config.binCount);
    if (config.binCount < 4) config.binCount = 4;
    if (config.binCount > MAX_SPECTRUM_BINS) config.binCount = MAX_SPECTRUM_BINS;

    // Clamp parameters to valid ranges
    if (config.smoothingFactor < 0.0f) config.smoothingFactor = 0.0f;
    if (config.smoothingFactor > 1.0f) config.smoothingFactor = 1.0f;
    if (config.peakDecayRate < 0.0f) config.peakDecayRate = 0.0f;
    if (config.peakDecayRate > 1.0f) config.peakDecayRate = 1.0f;
    if (config.sampleRate <= 0) config.sampleRate = 44100;

    analyzerConfig = config;

    // Allocate buffers
    spectrumMagnitudes = (float *)calloc(config.binCount, sizeof(float));
    spectrumSmoothed   = (float *)calloc(config.binCount, sizeof(float));
    peakValues         = (float *)calloc(config.binCount, sizeof(float));
    fftReal            = (float *)calloc(config.binCount, sizeof(float));
    fftImag            = (float *)calloc(config.binCount, sizeof(float));
    windowCoeffs       = (float *)calloc(config.binCount, sizeof(float));

    // Precompute window coefficients
    ComputeWindowCoefficients(windowCoeffs, config.binCount, config.windowFunc);

    analyzerReady = true;
}

// Free all resources used by the spectrum analyzer
void CloseSpectrumAnalyzer(void)
{
    free(spectrumMagnitudes); spectrumMagnitudes = NULL;
    free(spectrumSmoothed);   spectrumSmoothed = NULL;
    free(peakValues);         peakValues = NULL;
    free(fftReal);            fftReal = NULL;
    free(fftImag);            fftImag = NULL;
    free(windowCoeffs);       windowCoeffs = NULL;

    analyzerReady = false;
}

//----------------------------------------------------------------------------------
// Spectrum processing
//----------------------------------------------------------------------------------

// Process audio samples: apply window, run FFT, compute magnitudes.
// Bug-fix: handles non-power-of-2 sampleCount by padding with zeros or truncating.
void UpdateSpectrum(float *samples, int sampleCount)
{
    if (!analyzerReady || samples == NULL || sampleCount <= 0) return;

    int n = analyzerConfig.binCount;

    // Bounds checking: pad or truncate input to match FFT size (power of 2)
    if (sampleCount >= n)
    {
        // Truncate: only use the first n samples
        for (int i = 0; i < n; i++)
        {
            fftReal[i] = samples[i] * windowCoeffs[i];
            fftImag[i] = 0.0f;
        }
    }
    else
    {
        // Pad: copy available samples, zero-pad the rest
        for (int i = 0; i < sampleCount; i++)
        {
            fftReal[i] = samples[i] * windowCoeffs[i];
            fftImag[i] = 0.0f;
        }
        for (int i = sampleCount; i < n; i++)
        {
            fftReal[i] = 0.0f;
            fftImag[i] = 0.0f;
        }
    }

    // Perform in-place FFT
    BitReversalPermutation(fftReal, fftImag, n);
    ComputeFFT(fftReal, fftImag, n);

    // Compute magnitudes for each bin (only first half is unique due to symmetry)
    int halfN = n / 2;
    for (int i = 0; i < halfN; i++)
    {
        spectrumMagnitudes[i] = sqrtf(fftReal[i] * fftReal[i] + fftImag[i] * fftImag[i]) / (float)n;
    }
    // Mirror: zero out upper half (redundant for real input)
    for (int i = halfN; i < n; i++)
    {
        spectrumMagnitudes[i] = 0.0f;
    }

    // Update peak-hold values with decay
    for (int i = 0; i < halfN; i++)
    {
        if (spectrumMagnitudes[i] > peakValues[i])
        {
            peakValues[i] = spectrumMagnitudes[i];
        }
        else
        {
            peakValues[i] *= analyzerConfig.peakDecayRate;
        }
    }
}

// Apply temporal smoothing between current and previous spectrum data
void ApplySmoothing(void)
{
    if (!analyzerReady) return;

    float alpha = analyzerConfig.smoothingFactor;
    int halfN = analyzerConfig.binCount / 2;

    for (int i = 0; i < halfN; i++)
    {
        spectrumSmoothed[i] = alpha * spectrumSmoothed[i] + (1.0f - alpha) * spectrumMagnitudes[i];
    }
}

//----------------------------------------------------------------------------------
// Data retrieval
//----------------------------------------------------------------------------------

// Get pointer to smoothed spectrum magnitude array
float *GetSpectrumData(void)
{
    if (!analyzerReady) return NULL;
    return spectrumSmoothed;
}

// Find the dominant frequency in Hz from the current spectrum
float GetPeakFrequency(void)
{
    if (!analyzerReady) return 0.0f;

    int halfN = analyzerConfig.binCount / 2;
    int peakBin = 0;
    float peakMag = 0.0f;

    for (int i = 1; i < halfN; i++)
    {
        if (spectrumMagnitudes[i] > peakMag)
        {
            peakMag = spectrumMagnitudes[i];
            peakBin = i;
        }
    }

    // Convert bin index to frequency: freq = bin * sampleRate / binCount
    return (float)peakBin * (float)analyzerConfig.sampleRate / (float)analyzerConfig.binCount;
}

// Get the magnitude of the largest frequency bin
float GetPeakMagnitude(void)
{
    if (!analyzerReady) return 0.0f;

    int halfN = analyzerConfig.binCount / 2;
    float peakMag = 0.0f;

    for (int i = 0; i < halfN; i++)
    {
        if (spectrumMagnitudes[i] > peakMag) peakMag = spectrumMagnitudes[i];
    }

    return peakMag;
}

// Get the configured number of frequency bins
int GetSpectrumBinCount(void)
{
    return analyzerConfig.binCount;
}

// Check whether the analyzer has been initialized
bool IsSpectrumReady(void)
{
    return analyzerReady;
}

//----------------------------------------------------------------------------------
// Internal helpers
//----------------------------------------------------------------------------------

// Return the smallest power of 2 >= n
static int NextPowerOfTwo(int n)
{
    int power = 1;
    while (power < n) power <<= 1;
    return power;
}

// In-place bit-reversal permutation for radix-2 FFT
static void BitReversalPermutation(float *real, float *imag, int n)
{
    int j = 0;
    for (int i = 0; i < n - 1; i++)
    {
        if (i < j)
        {
            // Swap real parts
            float tmpR = real[i];
            real[i] = real[j];
            real[j] = tmpR;
            // Swap imaginary parts
            float tmpI = imag[i];
            imag[i] = imag[j];
            imag[j] = tmpI;
        }
        int k = n >> 1;
        while (k <= j)
        {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
}

// Radix-2 Cooley-Tukey decimation-in-time FFT (in-place)
static void ComputeFFT(float *real, float *imag, int n)
{
    for (int step = 2; step <= n; step <<= 1)
    {
        int halfStep = step / 2;
        float angle = -2.0f * (float)PI / (float)step;

        for (int group = 0; group < n; group += step)
        {
            for (int pair = 0; pair < halfStep; pair++)
            {
                float twiddleReal = cosf(angle * (float)pair);
                float twiddleImag = sinf(angle * (float)pair);

                int even = group + pair;
                int odd  = group + pair + halfStep;

                // Butterfly operation
                float tR = twiddleReal * real[odd] - twiddleImag * imag[odd];
                float tI = twiddleReal * imag[odd] + twiddleImag * real[odd];

                real[odd] = real[even] - tR;
                imag[odd] = imag[even] - tI;
                real[even] += tR;
                imag[even] += tI;
            }
        }
    }
}

// Precompute window function coefficients for a given size and type
static void ComputeWindowCoefficients(float *coeffs, int n, WindowFunction func)
{
    for (int i = 0; i < n; i++)
    {
        switch (func)
        {
            case WINDOW_HANN:
                coeffs[i] = 0.5f * (1.0f - cosf(2.0f * (float)PI * (float)i / (float)(n - 1)));
                break;
            case WINDOW_HAMMING:
                coeffs[i] = 0.54f - 0.46f * cosf(2.0f * (float)PI * (float)i / (float)(n - 1));
                break;
            case WINDOW_BLACKMAN:
                coeffs[i] = 0.42f
                           - 0.5f  * cosf(2.0f * (float)PI * (float)i / (float)(n - 1))
                           + 0.08f * cosf(4.0f * (float)PI * (float)i / (float)(n - 1));
                break;
            case WINDOW_RECTANGULAR:
            default:
                coeffs[i] = 1.0f;
                break;
        }
    }
}
