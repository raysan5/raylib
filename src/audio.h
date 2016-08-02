/**********************************************************************************************
*
*   raylib.audio
*
*   Basic functions to manage Audio:
*       Manage audio device (init/close)
*       Load and Unload audio files
*       Play/Stop/Pause/Resume loaded audio
*       Manage mixing channels
*       Manage raw audio context
*
*   Uses external lib:
*       OpenAL Soft - Audio device management lib (http://kcat.strangesoft.net/openal.html)
*       stb_vorbis - Ogg audio files loading (http://www.nothings.org/stb_vorbis/)
*       jar_xm - XM module file loading
*       jar_mod - MOD audio file loading
*
*   Many thanks to Joshua Reisenauer (github: @kd7tck) for the following additions:
*       XM audio module support (jar_xm)
*       MOD audio module support (jar_mod)
*       Mixing channels support
*       Raw audio context support
*
*   Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
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

#ifndef AUDIO_H
#define AUDIO_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for CAMERA_STANDALONE usage
//----------------------------------------------------------------------------------
#ifndef __cplusplus
// Boolean type
    #if !defined(_STDBOOL_H)
        typedef enum { false, true } bool;
        #define _STDBOOL_H
    #endif
#endif

// Sound source type
typedef struct Sound {
    unsigned int source;    // Sound audio source id
    unsigned int buffer;    // Sound audio buffer id
} Sound;

// Wave type, defines audio wave data
typedef struct Wave {
    void *data;                 // Buffer data pointer
    unsigned int dataSize;      // Data size in bytes
    unsigned int sampleRate;    // Samples per second to be played
    short bitsPerSample;        // Sample size in bits
    short channels;
} Wave;

// Music type (file streaming from memory)
// NOTE: Anything longer than ~10 seconds should be streamed
typedef struct Music *Music;

// Audio stream type
// NOTE: Useful to create custom audio streams not bound to a specific file
typedef struct AudioStream {
    unsigned int sampleRate;    // Frequency (samples per second)
    unsigned int sampleSize;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;      // Number of channels (1-mono, 2-stereo)

    int format;                 // OpenAL audio format specifier
    unsigned int source;        // OpenAL audio source id
    unsigned int buffers[2];    // OpenAL audio buffers (double buffering)
} AudioStream;

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
void InitAudioDevice(void);                                     // Initialize audio device and context
void CloseAudioDevice(void);                                    // Close the audio device and context (and music stream)
bool IsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully

Sound LoadSound(char *fileName);                                // Load sound to memory
Sound LoadSoundFromWave(Wave wave);                             // Load sound to memory from wave data
Sound LoadSoundFromRES(const char *rresName, int resId);        // Load sound to memory from rRES file (raylib Resource)
void UnloadSound(Sound sound);                                  // Unload sound
void PlaySound(Sound sound);                                    // Play a sound
void PauseSound(Sound sound);                                   // Pause a sound
void ResumeSound(Sound sound);                                  // Resume a paused sound
void StopSound(Sound sound);                                    // Stop playing a sound
bool IsSoundPlaying(Sound sound);                               // Check if a sound is currently playing
void SetSoundVolume(Sound sound, float volume);                 // Set volume for a sound (1.0 is max level)
void SetSoundPitch(Sound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)

Music LoadMusicStream(char *fileName);                          // Load music stream from file
void UnloadMusicStream(Music music);                            // Unload music stream
void PlayMusicStream(Music music);                              // Start music playing (open stream)
void UpdateMusicStream(Music music);                            // Updates buffers for music streaming
void StopMusicStream(Music music);                              // Stop music playing (close stream)
void PauseMusicStream(Music music);                             // Pause music playing
void ResumeMusicStream(Music music);                            // Resume playing paused music
bool IsMusicPlaying(Music music);                               // Check if music is playing
void SetMusicVolume(Music music, float volume);                 // Set volume for music (1.0 is max level)
void SetMusicPitch(Music music, float pitch);                   // Set pitch for a music (1.0 is base level)
float GetMusicTimeLength(Music music);                          // Get music time length (in seconds)
float GetMusicTimePlayed(Music music);                          // Get current music time played (in seconds)

AudioStream InitAudioStream(unsigned int sampleRate, 
                            unsigned int sampleSize, 
                            unsigned int channels);             // Init audio stream (to stream audio pcm data)
void UpdateAudioStream(AudioStream stream, void *data, int numSamples); // Update audio stream buffers with data
void CloseAudioStream(AudioStream stream);                      // Close audio stream and free memory
bool IsAudioBufferProcessed(AudioStream stream);                // Check if any audio stream buffers requires refill
void PlayAudioStream(AudioStream stream);                       // Play audio stream
void PauseAudioStream(AudioStream stream);                      // Pause audio stream
void ResumeAudioStream(AudioStream stream);                     // Resume audio stream
void StopAudioStream(AudioStream stream);                       // Stop audio stream

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H