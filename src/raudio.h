/**********************************************************************************************
*
*   raudio v1.0 - A simple and easy-to-use audio library based on miniaudio
*
*   FEATURES:
*       - Manage audio device (init/close)
*       - Load and unload audio files
*       - Format wave data (sample rate, size, channels)
*       - Play/Stop/Pause/Resume loaded audio
*       - Manage mixing channels
*       - Manage raw audio context
*
*   DEPENDENCIES:
*       miniaudio.h  - Audio device management lib (https://github.com/dr-soft/miniaudio)
*       stb_vorbis.h - Ogg audio files loading (http://www.nothings.org/stb_vorbis/)
*       dr_mp3.h     - MP3 audio file loading (https://github.com/mackron/dr_libs)
*       dr_flac.h    - FLAC audio file loading (https://github.com/mackron/dr_libs)
*       jar_xm.h     - XM module file loading
*       jar_mod.h    - MOD audio file loading
*
*   CONTRIBUTORS:
*       David Reid (github: @mackron) (Nov. 2017):
*           - Complete port to miniaudio library
*
*       Joshua Reisenauer (github: @kd7tck) (2015)
*           - XM audio module support (jar_xm)
*           - MOD audio module support (jar_mod)
*           - Mixing channels support
*           - Raw audio context support
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2020 Ramon Santamaria (@raysan5)
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

#ifndef RAUDIO_H
#define RAUDIO_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Allow custom memory allocators
#ifndef RL_MALLOC
    #define RL_MALLOC(sz)       malloc(sz)
#endif
#ifndef RL_CALLOC
    #define RL_CALLOC(n,sz)     calloc(n,sz)
#endif
#ifndef RL_FREE
    #define RL_FREE(p)          free(p)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#ifndef __cplusplus
// Boolean type
    #if !defined(_STDBOOL_H)
        typedef enum { false, true } bool;
        #define _STDBOOL_H
    #endif
#endif

// Wave type, defines audio wave data
typedef struct Wave {
    unsigned int sampleCount;       // Total number of samples
    unsigned int sampleRate;        // Frequency (samples per second)
    unsigned int sampleSize;        // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;          // Number of channels (1-mono, 2-stereo)
    void *data;                     // Buffer data pointer
} Wave;

typedef struct rAudioBuffer rAudioBuffer;

// Audio stream type
// NOTE: Useful to create custom audio streams not bound to a specific file
typedef struct AudioStream {
    unsigned int sampleRate;        // Frequency (samples per second)
    unsigned int sampleSize;        // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;          // Number of channels (1-mono, 2-stereo)

    rAudioBuffer *buffer;           // Pointer to internal data used by the audio system
} AudioStream;

// Sound source type
typedef struct Sound {
    unsigned int sampleCount;       // Total number of samples
    AudioStream stream;             // Audio stream
} Sound;

// Music stream type (audio file streaming from memory)
// NOTE: Anything longer than ~10 seconds should be streamed
typedef struct Music {
    int ctxType;                    // Type of music context (audio filetype)
    void *ctxData;                  // Audio context data, depends on type

    bool looping;                   // Music looping enable
    unsigned int sampleCount;       // Total number of samples

    AudioStream stream;             // Audio stream
} Music;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Audio device management functions
void InitAudioDevice(void);                                     // Initialize audio device and context
void CloseAudioDevice(void);                                    // Close the audio device and context
bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
void SetMasterVolume(float volume);                             // Set master volume (listener)

// Wave/Sound loading/unloading functions
Wave LoadWave(const char *fileName);                            // Load wave data from file
Sound LoadSound(const char *fileName);                          // Load sound from file
Sound LoadSoundFromWave(Wave wave);                             // Load sound from wave data
void UpdateSound(Sound sound, const void *data, int samplesCount);// Update sound buffer with new data
void UnloadWave(Wave wave);                                     // Unload wave data
void UnloadSound(Sound sound);                                  // Unload sound
void ExportWave(Wave wave, const char *fileName);               // Export wave data to file
void ExportWaveAsCode(Wave wave, const char *fileName);         // Export wave sample data to code (.h)

// Wave/Sound management functions
void PlaySound(Sound sound);                                    // Play a sound
void StopSound(Sound sound);                                    // Stop playing a sound
void PauseSound(Sound sound);                                   // Pause a sound
void ResumeSound(Sound sound);                                  // Resume a paused sound
void PlaySoundMulti(Sound sound);                               // Play a sound (using multichannel buffer pool)
void StopSoundMulti(void);                                      // Stop any sound playing (using multichannel buffer pool)
int GetSoundsPlaying(void);                                     // Get number of sounds playing in the multichannel
bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)
void SetSoundPitch(Sound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels);  // Convert wave data to desired format
Wave WaveCopy(Wave wave);                                       // Copy a wave to a new wave
void WaveCrop(Wave *wave, int initSample, int finalSample);     // Crop a wave to defined samples range
float *GetWaveData(Wave wave);                                  // Get samples data from wave as a floats array

// Music management functions
Music LoadMusicStream(const char *fileName);                    // Load music stream from file
void UnloadMusicStream(Music music);                            // Unload music stream
void PlayMusicStream(Music music);                              // Start music playing
void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
void StopMusicStream(Music music);                              // Stop music playing
void PauseMusicStream(Music music);                             // Pause music playing
void ResumeMusicStream(Music music);                            // Resume playing paused music
bool IsMusicPlaying(Music music);                               // Check if music is playing
void SetMusicVolume(Music music, float volume);                 // Set volume for music (1.0 is max level)
void SetMusicPitch(Music music, float pitch);                   // Set pitch for a music (1.0 is base level)
float GetMusicTimeLength(Music music);                          // Get music time length (in seconds)
float GetMusicTimePlayed(Music music);                          // Get current music time played (in seconds)

// AudioStream management functions
AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels); // Init audio stream (to stream raw audio pcm data)
void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount); // Update audio stream buffers with data
void CloseAudioStream(AudioStream stream);                      // Close audio stream and free memory
bool IsAudioStreamProcessed(AudioStream stream);                // Check if any audio stream buffers requires refill
void PlayAudioStream(AudioStream stream);                       // Play audio stream
void PauseAudioStream(AudioStream stream);                      // Pause audio stream
void ResumeAudioStream(AudioStream stream);                     // Resume audio stream
bool IsAudioStreamPlaying(AudioStream stream);                  // Check if audio stream is playing
void StopAudioStream(AudioStream stream);                       // Stop audio stream
void SetAudioStreamVolume(AudioStream stream, float volume);    // Set volume for audio stream (1.0 is max level)
void SetAudioStreamPitch(AudioStream stream, float pitch);      // Set pitch for audio stream (1.0 is base level)
void SetAudioStreamBufferSizeDefault(int size);                 // Default size for new audio streams

#ifdef __cplusplus
}
#endif

#endif // RAUDIO_H
