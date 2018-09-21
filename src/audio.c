/**********************************************************************************************
*
*   raylib.audio - Basic funtionality to work with audio
*
*   FEATURES:
*       - Manage audio device (init/close)
*       - Load and unload audio files
*       - Format wave data (sample rate, size, channels)
*       - Play/Stop/Pause/Resume loaded audio
*       - Manage mixing channels
*       - Manage raw audio context
*
*   CONFIGURATION:
*
*   #define AUDIO_STANDALONE
*       Define to use the module as standalone library (independently of raylib).
*       Required types and functions are defined in the same module.
*
*   #define USE_OPENAL_BACKEND
*       Use OpenAL Soft audio backend
*
*   #define SUPPORT_FILEFORMAT_WAV
*   #define SUPPORT_FILEFORMAT_OGG
*   #define SUPPORT_FILEFORMAT_XM
*   #define SUPPORT_FILEFORMAT_MOD
*   #define SUPPORT_FILEFORMAT_FLAC
*   #define SUPPORT_FILEFORMAT_MP3
*       Selected desired fileformats to be supported for loading. Some of those formats are
*       supported by default, to remove support, just comment unrequired #define in this module
*
*   LIMITATIONS (only OpenAL Soft):
*       Only up to two channels supported: MONO and STEREO (for additional channels, use AL_EXT_MCFORMATS)
*       Only the following sample sizes supported: 8bit PCM, 16bit PCM, 32-bit float PCM (using AL_EXT_FLOAT32)
*
*   DEPENDENCIES:
*       mini_al     - Audio device/context management (https://github.com/dr-soft/mini_al)
*       stb_vorbis  - OGG audio files loading (http://www.nothings.org/stb_vorbis/)
*       jar_xm      - XM module file loading
*       jar_mod     - MOD audio file loading
*       dr_flac     - FLAC audio file loading
*
*      *OpenAL Soft - Audio device management, still used on HTML5 and OSX platforms
*
*   CONTRIBUTORS:
*       David Reid (github: @mackron) (Nov. 2017):
*           - Complete port to mini_al library
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
*   Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
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

#if defined(AUDIO_STANDALONE)
    #include "audio.h"
    #include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#else
    #include "config.h"         // Defines module configuration flags
    #include "raylib.h"         // Declares module functions
    #include "utils.h"          // Required for: fopen() Android mapping
#endif

#if !defined(USE_OPENAL_BACKEND)
    #define USE_MINI_AL 1       // Set to 1 to use mini_al; 0 to use OpenAL.
#endif

#include "external/mini_al.h"   // Implemented in mini_al.c. Cannot implement this here because it conflicts with Win32 APIs such as CloseWindow(), etc.

#if !defined(USE_MINI_AL) || (USE_MINI_AL == 0)
    #if defined(__APPLE__)
        #include "OpenAL/al.h"          // OpenAL basic header
        #include "OpenAL/alc.h"         // OpenAL context header (like OpenGL, OpenAL requires a context to work)
    #else
        #include "AL/al.h"              // OpenAL basic header
        #include "AL/alc.h"             // OpenAL context header (like OpenGL, OpenAL requires a context to work)
        //#include "AL/alext.h"         // OpenAL extensions header, required for AL_EXT_FLOAT32 and AL_EXT_MCFORMATS
    #endif

    // OpenAL extension: AL_EXT_FLOAT32 - Support for 32bit float samples
    // OpenAL extension: AL_EXT_MCFORMATS - Support for multi-channel formats (Quad, 5.1, 6.1, 7.1)
#endif

#include <stdlib.h>             // Required for: malloc(), free()
#include <string.h>             // Required for: strcmp(), strncmp()
#include <stdio.h>              // Required for: FILE, fopen(), fclose(), fread()

#if defined(SUPPORT_FILEFORMAT_OGG)
    //#define STB_VORBIS_HEADER_ONLY
    #include "external/stb_vorbis.h"    // OGG loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_XM)
    #define JAR_XM_IMPLEMENTATION
    #include "external/jar_xm.h"        // XM loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_MOD)
    #define JAR_MOD_IMPLEMENTATION
    #include "external/jar_mod.h"       // MOD loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)
    #define DR_FLAC_IMPLEMENTATION
    #define DR_FLAC_NO_WIN32_IO
    #include "external/dr_flac.h"       // FLAC loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
    #define DR_MP3_IMPLEMENTATION
    #include "external/dr_mp3.h"       // MP3 loading functions
#endif

#ifdef _MSC_VER
    #undef bool
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_STREAM_BUFFERS          2    // Number of buffers for each audio stream

// NOTE: Music buffer size is defined by number of samples, independent of sample size and channels number
// After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds
// and double-buffering system, I concluded that a 4096 samples buffer should be enough
// In case of music-stalls, just increase this number
#define AUDIO_BUFFER_SIZE        4096    // PCM data samples (i.e. 16bit, Mono: 8Kb)

// Support uncompressed PCM data in 32-bit float IEEE format
// NOTE: This definition is included in "AL/alext.h", but some OpenAL implementations
// could not provide the extensions header (Android), so its defined here
#if !defined(AL_EXT_float32)
    #define AL_EXT_float32              1
    #define AL_FORMAT_MONO_FLOAT32      0x10010
    #define AL_FORMAT_STEREO_FLOAT32    0x10011
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef enum { 
    MUSIC_AUDIO_OGG = 0, 
    MUSIC_AUDIO_FLAC, 
    MUSIC_AUDIO_MP3, 
    MUSIC_MODULE_XM, 
    MUSIC_MODULE_MOD 
} MusicContextType;

// Music type (file streaming from memory)
typedef struct MusicData {
    MusicContextType ctxType;           // Type of music context (OGG, XM, MOD)
#if defined(SUPPORT_FILEFORMAT_OGG)
    stb_vorbis *ctxOgg;                 // OGG audio context
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    drflac *ctxFlac;                    // FLAC audio context
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    drmp3 ctxMp3;                       // MP3 audio context
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    jar_xm_context_t *ctxXm;            // XM chiptune context
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    jar_mod_context_t ctxMod;           // MOD chiptune context
#endif

    AudioStream stream;                 // Audio stream (double buffering)

    int loopCount;                      // Loops count (times music repeats), -1 means infinite loop
    unsigned int totalSamples;          // Total number of samples
    unsigned int samplesLeft;           // Number of samples left to end
} MusicData;

#if defined(AUDIO_STANDALONE)
typedef enum { 
    LOG_INFO = 0, 
    LOG_ERROR, 
    LOG_WARNING, 
    LOG_DEBUG, 
    LOG_OTHER 
} TraceLogType;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_WAV)
static Wave LoadWAV(const char *fileName);          // Load WAV file
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
static Wave LoadOGG(const char *fileName);          // Load OGG file
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
static Wave LoadFLAC(const char *fileName);         // Load FLAC file
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
static Wave LoadMP3(const char *fileName);          // Load MP3 file
#endif

#if defined(AUDIO_STANDALONE)
bool IsFileExtension(const char *fileName, const char *ext);    // Check file extension
void TraceLog(int msgType, const char *text, ...);              // Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
#endif

//----------------------------------------------------------------------------------
// mini_al AudioBuffer Functionality
//----------------------------------------------------------------------------------
#if USE_MINI_AL

#define DEVICE_FORMAT       mal_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  44100

typedef enum { AUDIO_BUFFER_USAGE_STATIC = 0, AUDIO_BUFFER_USAGE_STREAM } AudioBufferUsage;

// Audio buffer structure
// NOTE: Slightly different logic is used when feeding data to the playback device depending on whether or not data is streamed
typedef struct AudioBuffer AudioBuffer;
struct AudioBuffer {
    mal_dsp dsp;                    // Required for format conversion
    float volume;
    float pitch;
    bool playing;
    bool paused;
    bool looping;                   // Always true for AudioStreams
    int usage;                      // AudioBufferUsage type
    bool isSubBufferProcessed[2];
    unsigned int frameCursorPos;
    unsigned int bufferSizeInFrames;
    AudioBuffer *next;
    AudioBuffer *prev;
    unsigned char buffer[1];
};

// mini_al global variables
static mal_context context;
static mal_device device;
static mal_mutex audioLock;
static bool isAudioInitialized = MAL_FALSE;
static float masterVolume = 1.0f;

// Audio buffers are tracked in a linked list
static AudioBuffer *firstAudioBuffer = NULL;   
static AudioBuffer *lastAudioBuffer = NULL;

// mini_al functions declaration
static void OnLog(mal_context *pContext, mal_device *pDevice, const char *message);
static mal_uint32 OnSendAudioDataToDevice(mal_device *pDevice, mal_uint32 frameCount, void *pFramesOut);
static mal_uint32 OnAudioBufferDSPRead(mal_dsp *pDSP, mal_uint32 frameCount, void *pFramesOut, void *pUserData);
static void MixAudioFrames(float *framesOut, const float *framesIn, mal_uint32 frameCount, float localVolume);

// AudioBuffer management functions declaration
// NOTE: Those functions are not exposed by raylib... for the moment
AudioBuffer *CreateAudioBuffer(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_uint32 bufferSizeInFrames, AudioBufferUsage usage);
void DeleteAudioBuffer(AudioBuffer *audioBuffer);
bool IsAudioBufferPlaying(AudioBuffer *audioBuffer);
void PlayAudioBuffer(AudioBuffer *audioBuffer);
void StopAudioBuffer(AudioBuffer *audioBuffer);
void PauseAudioBuffer(AudioBuffer *audioBuffer);
void ResumeAudioBuffer(AudioBuffer *audioBuffer);
void SetAudioBufferVolume(AudioBuffer *audioBuffer, float volume);
void SetAudioBufferPitch(AudioBuffer *audioBuffer, float pitch);
void TrackAudioBuffer(AudioBuffer *audioBuffer);
void UntrackAudioBuffer(AudioBuffer *audioBuffer);


// Log callback function
static void OnLog(mal_context *pContext, mal_device *pDevice, const char *message)
{
    (void)pContext;
    (void)pDevice;
    
    TraceLog(LOG_ERROR, message);   // All log messages from mini_al are errors
}

// Sending audio data to device callback function
static mal_uint32 OnSendAudioDataToDevice(mal_device *pDevice, mal_uint32 frameCount, void *pFramesOut)
{
    // This is where all of the mixing takes place.
    (void)pDevice;

    // Mixing is basically just an accumulation. We need to initialize the output buffer to 0.
    memset(pFramesOut, 0, frameCount*pDevice->channels*mal_get_bytes_per_sample(pDevice->format));

    // Using a mutex here for thread-safety which makes things not real-time. This is unlikely to be necessary for this project, but may
    // want to consider how you might want to avoid this.
    mal_mutex_lock(&audioLock);
    {
        for (AudioBuffer *audioBuffer = firstAudioBuffer; audioBuffer != NULL; audioBuffer = audioBuffer->next)
        {
            // Ignore stopped or paused sounds.
            if (!audioBuffer->playing || audioBuffer->paused) continue;

            mal_uint32 framesRead = 0;
            for (;;) 
            {
                if (framesRead > frameCount) 
                {
                    TraceLog(LOG_DEBUG, "Mixed too many frames from audio buffer");
                    break;
                }
                
                if (framesRead == frameCount) break;

                // Just read as much data as we can from the stream.
                mal_uint32 framesToRead = (frameCount - framesRead);
                while (framesToRead > 0) 
                {
                    float tempBuffer[1024]; // 512 frames for stereo.

                    mal_uint32 framesToReadRightNow = framesToRead;
                    if (framesToReadRightNow > sizeof(tempBuffer)/sizeof(tempBuffer[0])/DEVICE_CHANNELS) 
                    {
                        framesToReadRightNow = sizeof(tempBuffer)/sizeof(tempBuffer[0])/DEVICE_CHANNELS;
                    }

                    mal_uint32 framesJustRead = (mal_uint32)mal_dsp_read(&audioBuffer->dsp, framesToReadRightNow, tempBuffer, audioBuffer->dsp.pUserData);
                    if (framesJustRead > 0) 
                    {
                        float *framesOut = (float *)pFramesOut + (framesRead*device.channels);
                        float *framesIn  = tempBuffer;
                        MixAudioFrames(framesOut, framesIn, framesJustRead, audioBuffer->volume);

                        framesToRead -= framesJustRead;
                        framesRead += framesJustRead;
                    }

                    // If we weren't able to read all the frames we requested, break.
                    if (framesJustRead < framesToReadRightNow) 
                    {
                        if (!audioBuffer->looping) 
                        {
                            StopAudioBuffer(audioBuffer);
                            break;
                        } 
                        else 
                        {
                            // Should never get here, but just for safety, 
                            // move the cursor position back to the start and continue the loop.
                            audioBuffer->frameCursorPos = 0;
                            continue;
                        }
                    }
                }

                // If for some reason we weren't able to read every frame we'll need to break from the loop. 
                // Not doing this could theoretically put us into an infinite loop.
                if (framesToRead > 0) break;
            }
        }
    }
    
    mal_mutex_unlock(&audioLock);

    return frameCount;  // We always output the same number of frames that were originally requested.
}

// DSP read from audio buffer callback function
static mal_uint32 OnAudioBufferDSPRead(mal_dsp *pDSP, mal_uint32 frameCount, void *pFramesOut, void *pUserData)
{
    AudioBuffer *audioBuffer = (AudioBuffer *)pUserData;

    mal_uint32 subBufferSizeInFrames = audioBuffer->bufferSizeInFrames/2;
    mal_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos/subBufferSizeInFrames;
    
    if (currentSubBufferIndex > 1) 
    {
        TraceLog(LOG_DEBUG, "Frame cursor position moved too far forward in audio stream");
        return 0;
    }

    // Another thread can update the processed state of buffers so we just take a copy here to try and avoid potential synchronization problems.
    bool isSubBufferProcessed[2];
    isSubBufferProcessed[0] = audioBuffer->isSubBufferProcessed[0];
    isSubBufferProcessed[1] = audioBuffer->isSubBufferProcessed[1];

    mal_uint32 frameSizeInBytes = mal_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn)*audioBuffer->dsp.formatConverterIn.config.channels;

    // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0.
    mal_uint32 framesRead = 0;
    for (;;)
    {
        // We break from this loop differently depending on the buffer's usage. For static buffers, we simply fill as much data as we can. For
        // streaming buffers we only fill the halves of the buffer that are processed. Unprocessed halves must keep their audio data in-tact.
        if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC) 
        {
            if (framesRead >= frameCount) break;
        } 
        else 
        {
            if (isSubBufferProcessed[currentSubBufferIndex]) break;
        }

        mal_uint32 totalFramesRemaining = (frameCount - framesRead);
        if (totalFramesRemaining == 0) break;

        mal_uint32 framesRemainingInOutputBuffer;
        if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC) 
        {
            framesRemainingInOutputBuffer = audioBuffer->bufferSizeInFrames - audioBuffer->frameCursorPos;
        } 
        else 
        {
            mal_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames * currentSubBufferIndex;
            framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos - firstFrameIndexOfThisSubBuffer);
        }

        mal_uint32 framesToRead = totalFramesRemaining;
        if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

        memcpy((unsigned char *)pFramesOut + (framesRead*frameSizeInBytes), audioBuffer->buffer + (audioBuffer->frameCursorPos*frameSizeInBytes), framesToRead*frameSizeInBytes);
        audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead) % audioBuffer->bufferSizeInFrames;
        framesRead += framesToRead;

        // If we've read to the end of the buffer, mark it as processed.
        if (framesToRead == framesRemainingInOutputBuffer) 
        {
            audioBuffer->isSubBufferProcessed[currentSubBufferIndex] = true;
            isSubBufferProcessed[currentSubBufferIndex] = true;

            currentSubBufferIndex = (currentSubBufferIndex + 1)%2;

            // We need to break from this loop if we're not looping.
            if (!audioBuffer->looping) 
            {
                StopAudioBuffer(audioBuffer);
                break;
            }
        }
    }

    // Zero-fill excess.
    mal_uint32 totalFramesRemaining = (frameCount - framesRead);
    if (totalFramesRemaining > 0) 
    {
        memset((unsigned char*)pFramesOut + (framesRead*frameSizeInBytes), 0, totalFramesRemaining*frameSizeInBytes);

        // For static buffers we can fill the remaining frames with silence for safety, but we don't want
        // to report those frames as "read". The reason for this is that the caller uses the return value
        // to know whether or not a non-looping sound has finished playback.
        if (audioBuffer->usage != AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
    }

    return framesRead;
}

// This is the main mixing function. Mixing is pretty simple in this project - it's just an accumulation.
// NOTE: framesOut is both an input and an output. It will be initially filled with zeros outside of this function.
static void MixAudioFrames(float *framesOut, const float *framesIn, mal_uint32 frameCount, float localVolume)
{
    for (mal_uint32 iFrame = 0; iFrame < frameCount; ++iFrame) 
    {
        for (mal_uint32 iChannel = 0; iChannel < device.channels; ++iChannel) 
        {
                  float *frameOut = framesOut + (iFrame*device.channels);
            const float *frameIn  = framesIn  + (iFrame*device.channels);

            frameOut[iChannel] += frameIn[iChannel]*masterVolume*localVolume;
        }
    }
}
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------
// Initialize audio device
void InitAudioDevice(void)
{
#if USE_MINI_AL
    // Context.
    mal_context_config contextConfig = mal_context_config_init(OnLog);
    mal_result result = mal_context_init(NULL, 0, &contextConfig, &context);
    if (result != MAL_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to initialize audio context");
        return;
    }

    // Device. Using the default device. Format is floating point because it simplifies mixing.
    mal_device_config deviceConfig = mal_device_config_init(DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, NULL, OnSendAudioDataToDevice);

    result = mal_device_init(&context, mal_device_type_playback, NULL, &deviceConfig, NULL, &device);
    if (result != MAL_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to initialize audio playback device");
        mal_context_uninit(&context);
        return;
    }

    // Keep the device running the whole time. May want to consider doing something a bit smarter and only have the device running
    // while there's at least one sound being played.
    result = mal_device_start(&device);
    if (result != MAL_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to start audio playback device");
        mal_device_uninit(&device);
        mal_context_uninit(&context);
        return;
    }

    // Mixing happens on a seperate thread which means we need to synchronize. I'm using a mutex here to make things simple, but may
    // want to look at something a bit smarter later on to keep everything real-time, if that's necessary.
    if (mal_mutex_init(&context, &audioLock) != MAL_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to create mutex for audio mixing");
        mal_device_uninit(&device);
        mal_context_uninit(&context);
        return;
    }

    TraceLog(LOG_INFO, "Audio device initialized successfully: %s", device.name);
    TraceLog(LOG_INFO, "Audio backend: mini_al / %s", mal_get_backend_name(context.backend));
    TraceLog(LOG_INFO, "Audio format: %s -> %s", mal_get_format_name(device.format), mal_get_format_name(device.internalFormat));
    TraceLog(LOG_INFO, "Audio channels: %d -> %d", device.channels, device.internalChannels);
    TraceLog(LOG_INFO, "Audio sample rate: %d -> %d", device.sampleRate, device.internalSampleRate);
    TraceLog(LOG_INFO, "Audio buffer size: %d", device.bufferSizeInFrames);

    isAudioInitialized = MAL_TRUE;
#else
    // Open and initialize a device with default settings
    ALCdevice *device = alcOpenDevice(NULL);

    if (!device) TraceLog(LOG_ERROR, "Audio device could not be opened");
    else
    {
        ALCcontext *context = alcCreateContext(device, NULL);

        if ((context == NULL) || (alcMakeContextCurrent(context) == ALC_FALSE))
        {
            if (context != NULL) alcDestroyContext(context);

            alcCloseDevice(device);

            TraceLog(LOG_ERROR, "Could not initialize audio context");
        }
        else
        {
            TraceLog(LOG_INFO, "Audio device and context initialized successfully: %s", alcGetString(device, ALC_DEVICE_SPECIFIER));

            // Listener definition (just for 2D)
            alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
            alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            alListener3f(AL_ORIENTATION, 0.0f, 0.0f, -1.0f);
            
            alListenerf(AL_GAIN, 1.0f);

            if (alIsExtensionPresent("AL_EXT_float32")) TraceLog(LOG_INFO, "[EXTENSION] AL_EXT_float32 supported");
            else TraceLog(LOG_INFO, "[EXTENSION] AL_EXT_float32 not supported");
        }
    }
#endif
}

// Close the audio device for all contexts
void CloseAudioDevice(void)
{
#if USE_MINI_AL
    if (!isAudioInitialized) 
    {
        TraceLog(LOG_WARNING, "Could not close audio device because it is not currently initialized");
        return;
    }

    mal_mutex_uninit(&audioLock);
    mal_device_uninit(&device);
    mal_context_uninit(&context);
#else
    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) TraceLog(LOG_WARNING, "Could not get current audio context for closing");

    device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
#endif

    TraceLog(LOG_INFO, "Audio device closed successfully");
}

// Check if device has been initialized successfully
bool IsAudioDeviceReady(void)
{
#if USE_MINI_AL
    return isAudioInitialized;
#else
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) return false;
    else
    {
        ALCdevice *device = alcGetContextsDevice(context);

        if (device == NULL) return false;
        else return true;
    }
#endif
}

// Set master volume (listener)
void SetMasterVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    else if (volume > 1.0f) volume = 1.0f;
 
#if USE_MINI_AL
    masterVolume = volume;
#else
    alListenerf(AL_GAIN, volume);
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Buffer management
//----------------------------------------------------------------------------------
#if USE_MINI_AL
// Create a new audio buffer. Initially filled with silence
AudioBuffer *CreateAudioBuffer(mal_format format, mal_uint32 channels, mal_uint32 sampleRate, mal_uint32 bufferSizeInFrames, AudioBufferUsage usage)
{
    AudioBuffer *audioBuffer = (AudioBuffer *)calloc(sizeof(*audioBuffer) + (bufferSizeInFrames*channels*mal_get_bytes_per_sample(format)), 1);
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "CreateAudioBuffer() : Failed to allocate memory for audio buffer");
        return NULL;
    }

    // We run audio data through a format converter.
    mal_dsp_config dspConfig;
    memset(&dspConfig, 0, sizeof(dspConfig));
    dspConfig.formatIn = format;
    dspConfig.formatOut = DEVICE_FORMAT;
    dspConfig.channelsIn = channels;
    dspConfig.channelsOut = DEVICE_CHANNELS;
    dspConfig.sampleRateIn = sampleRate;
    dspConfig.sampleRateOut = DEVICE_SAMPLE_RATE;
    dspConfig.onRead = OnAudioBufferDSPRead;
    dspConfig.pUserData = audioBuffer;
    dspConfig.allowDynamicSampleRate = MAL_TRUE;    // <-- Required for pitch shifting.
    mal_result resultMAL = mal_dsp_init(&dspConfig, &audioBuffer->dsp);
    if (resultMAL != MAL_SUCCESS) 
    {
        TraceLog(LOG_ERROR, "CreateAudioBuffer() : Failed to create data conversion pipeline");
        free(audioBuffer);
        return NULL;
    }

    audioBuffer->volume = 1;
    audioBuffer->pitch = 1;
    audioBuffer->playing = 0;
    audioBuffer->paused = 0;
    audioBuffer->looping = 0;
    audioBuffer->usage = usage;
    audioBuffer->bufferSizeInFrames = bufferSizeInFrames;
    audioBuffer->frameCursorPos = 0;

    // Buffers should be marked as processed by default so that a call to UpdateAudioStream() immediately after initialization works correctly.
    audioBuffer->isSubBufferProcessed[0] = true;
    audioBuffer->isSubBufferProcessed[1] = true;

    TrackAudioBuffer(audioBuffer);

    return audioBuffer;
}

// Delete an audio buffer
void DeleteAudioBuffer(AudioBuffer *audioBuffer)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "DeleteAudioBuffer() : No audio buffer");
        return;
    }

    UntrackAudioBuffer(audioBuffer);
    free(audioBuffer);
}

// Check if an audio buffer is playing
bool IsAudioBufferPlaying(AudioBuffer *audioBuffer)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "IsAudioBufferPlaying() : No audio buffer");
        return false;
    }

    return audioBuffer->playing && !audioBuffer->paused;
}

// Play an audio buffer
// NOTE: Buffer is restarted to the start.
// Use PauseAudioBuffer() and ResumeAudioBuffer() if the playback position should be maintained.
void PlayAudioBuffer(AudioBuffer *audioBuffer)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "PlayAudioBuffer() : No audio buffer");
        return;
    }

    audioBuffer->playing = true;
    audioBuffer->paused = false;
    audioBuffer->frameCursorPos = 0;
}

// Stop an audio buffer
void StopAudioBuffer(AudioBuffer *audioBuffer)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "StopAudioBuffer() : No audio buffer");
        return;
    }

    // Don't do anything if the audio buffer is already stopped.
    if (!IsAudioBufferPlaying(audioBuffer)) return;

    audioBuffer->playing = false;
    audioBuffer->paused = false;
    audioBuffer->frameCursorPos = 0;
    audioBuffer->isSubBufferProcessed[0] = true;
    audioBuffer->isSubBufferProcessed[1] = true;
}

// Pause an audio buffer
void PauseAudioBuffer(AudioBuffer *audioBuffer)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "PauseAudioBuffer() : No audio buffer");
        return;
    }

    audioBuffer->paused = true;
}

// Resume an audio buffer
void ResumeAudioBuffer(AudioBuffer *audioBuffer)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "ResumeAudioBuffer() : No audio buffer");
        return;
    }

    audioBuffer->paused = false;
}

// Set volume for an audio buffer
void SetAudioBufferVolume(AudioBuffer *audioBuffer, float volume)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "SetAudioBufferVolume() : No audio buffer");
        return;
    }

    audioBuffer->volume = volume;
}

// Set pitch for an audio buffer
void SetAudioBufferPitch(AudioBuffer *audioBuffer, float pitch)
{
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "SetAudioBufferPitch() : No audio buffer");
        return;
    }

    audioBuffer->pitch = pitch;

    // Pitching is just an adjustment of the sample rate. Note that this changes the duration of the sound - higher pitches
    // will make the sound faster; lower pitches make it slower.
    mal_uint32 newOutputSampleRate = (mal_uint32)((((float)audioBuffer->dsp.src.config.sampleRateOut / (float)audioBuffer->dsp.src.config.sampleRateIn) / pitch) * audioBuffer->dsp.src.config.sampleRateIn);
    mal_dsp_set_output_sample_rate(&audioBuffer->dsp, newOutputSampleRate);
}

// Track audio buffer to linked list next position
void TrackAudioBuffer(AudioBuffer *audioBuffer)
{
    mal_mutex_lock(&audioLock);
    
    {
        if (firstAudioBuffer == NULL) firstAudioBuffer = audioBuffer;
        else 
        {
            lastAudioBuffer->next = audioBuffer;
            audioBuffer->prev = lastAudioBuffer;
        }

        lastAudioBuffer = audioBuffer;
    }
    
    mal_mutex_unlock(&audioLock);
}

// Untrack audio buffer from linked list
void UntrackAudioBuffer(AudioBuffer *audioBuffer)
{
    mal_mutex_lock(&audioLock);
    
    {
        if (audioBuffer->prev == NULL) firstAudioBuffer = audioBuffer->next;
        else audioBuffer->prev->next = audioBuffer->next;

        if (audioBuffer->next == NULL) lastAudioBuffer = audioBuffer->prev;
        else audioBuffer->next->prev = audioBuffer->prev;

        audioBuffer->prev = NULL;
        audioBuffer->next = NULL;
    }
    
    mal_mutex_unlock(&audioLock);
}
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Sounds loading and playing (.WAV)
//----------------------------------------------------------------------------------

// Load wave data from file
Wave LoadWave(const char *fileName)
{
    Wave wave = { 0 };

    if (IsFileExtension(fileName, ".wav")) wave = LoadWAV(fileName);
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (IsFileExtension(fileName, ".ogg")) wave = LoadOGG(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (IsFileExtension(fileName, ".flac")) wave = LoadFLAC(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (IsFileExtension(fileName, ".mp3")) wave = LoadMP3(fileName);
#endif
    else TraceLog(LOG_WARNING, "[%s] Audio fileformat not supported, it can't be loaded", fileName);

    return wave;
}

// Load wave data from raw array data
Wave LoadWaveEx(void *data, int sampleCount, int sampleRate, int sampleSize, int channels)
{
    Wave wave;

    wave.data = data;
    wave.sampleCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = sampleSize;
    wave.channels = channels;

    // NOTE: Copy wave data to work with, user is responsible of input data to free
    Wave cwave = WaveCopy(wave);

    WaveFormat(&cwave, sampleRate, sampleSize, channels);

    return cwave;
}

// Load sound from file
// NOTE: The entire file is loaded to memory to be played (no-streaming)
Sound LoadSound(const char *fileName)
{
    Wave wave = LoadWave(fileName);

    Sound sound = LoadSoundFromWave(wave);

    UnloadWave(wave);       // Sound is loaded, we can unload wave

    return sound;
}

// Load sound from wave data
// NOTE: Wave data must be unallocated manually
Sound LoadSoundFromWave(Wave wave)
{
    Sound sound = { 0 };

    if (wave.data != NULL)
    {
#if USE_MINI_AL
        // When using mini_al we need to do our own mixing. To simplify this we need convert the format of each sound to be consistent with
        // the format used to open the playback device. We can do this two ways:
        // 
        //   1) Convert the whole sound in one go at load time (here).
        //   2) Convert the audio data in chunks at mixing time.
        //
        // I have decided on the first option because it offloads work required for the format conversion to the to the loading stage. The
        // downside to this is that it uses more memory if the original sound is u8 or s16.
        mal_format formatIn  = ((wave.sampleSize == 8) ? mal_format_u8 : ((wave.sampleSize == 16) ? mal_format_s16 : mal_format_f32));
        mal_uint32 frameCountIn = wave.sampleCount;  // Is wave->sampleCount actually the frame count? That terminology needs to change, if so.

        mal_uint32 frameCount = (mal_uint32)mal_convert_frames(NULL, DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, NULL, formatIn, wave.channels, wave.sampleRate, frameCountIn);
        if (frameCount == 0) TraceLog(LOG_WARNING, "LoadSoundFromWave() : Failed to get frame count for format conversion");

        AudioBuffer* audioBuffer = CreateAudioBuffer(DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, frameCount, AUDIO_BUFFER_USAGE_STATIC);
        if (audioBuffer == NULL) TraceLog(LOG_WARNING, "LoadSoundFromWave() : Failed to create audio buffer");

        frameCount = (mal_uint32)mal_convert_frames(audioBuffer->buffer, audioBuffer->dsp.formatConverterIn.config.formatIn, audioBuffer->dsp.formatConverterIn.config.channels, audioBuffer->dsp.src.config.sampleRateIn, wave.data, formatIn, wave.channels, wave.sampleRate, frameCountIn);
        if (frameCount == 0) TraceLog(LOG_WARNING, "LoadSoundFromWave() : Format conversion failed");

        sound.audioBuffer = audioBuffer;
#else
        ALenum format = 0;

        // The OpenAL format is worked out by looking at the number of channels and the sample size (bits per sample)
        if (wave.channels == 1)
        {
            switch (wave.sampleSize)
            {
                case 8: format = AL_FORMAT_MONO8; break;
                case 16: format = AL_FORMAT_MONO16; break;
                case 32: format = AL_FORMAT_MONO_FLOAT32; break;  // Requires OpenAL extension: AL_EXT_FLOAT32
                default: TraceLog(LOG_WARNING, "Wave sample size not supported: %i", wave.sampleSize); break;
            }
        }
        else if (wave.channels == 2)
        {
            switch (wave.sampleSize)
            {
                case 8: format = AL_FORMAT_STEREO8; break;
                case 16: format = AL_FORMAT_STEREO16; break;
                case 32: format = AL_FORMAT_STEREO_FLOAT32; break;  // Requires OpenAL extension: AL_EXT_FLOAT32
                default: TraceLog(LOG_WARNING, "Wave sample size not supported: %i", wave.sampleSize); break;
            }
        }
        else TraceLog(LOG_WARNING, "Wave number of channels not supported: %i", wave.channels);

        // Create an audio source
        ALuint source;
        alGenSources(1, &source);            // Generate pointer to audio source

        alSourcef(source, AL_PITCH, 1.0f);
        alSourcef(source, AL_GAIN, 1.0f);
        alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alSourcei(source, AL_LOOPING, AL_FALSE);

        // Convert loaded data to OpenAL buffer
        //----------------------------------------
        ALuint buffer;
        alGenBuffers(1, &buffer);            // Generate pointer to buffer

        unsigned int dataSize = wave.sampleCount*wave.channels*wave.sampleSize/8;    // Size in bytes

        // Upload sound data to buffer
        alBufferData(buffer, format, wave.data, dataSize, wave.sampleRate);

        // Attach sound buffer to source
        alSourcei(source, AL_BUFFER, buffer);

        TraceLog(LOG_INFO, "[SND ID %i][BUFR ID %i] Sound data loaded successfully (%i Hz, %i bit, %s)", source, buffer, wave.sampleRate, wave.sampleSize, (wave.channels == 1) ? "Mono" : "Stereo");

        sound.source = source;
        sound.buffer = buffer;
        sound.format = format;
#endif
    }

    return sound;
}

// Unload wave data
void UnloadWave(Wave wave)
{
    if (wave.data != NULL) free(wave.data);

    TraceLog(LOG_INFO, "Unloaded wave data from RAM");
}

// Unload sound
void UnloadSound(Sound sound)
{
#if USE_MINI_AL
    DeleteAudioBuffer((AudioBuffer *)sound.audioBuffer);
#else
    alSourceStop(sound.source);

    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);
#endif

    TraceLog(LOG_INFO, "[SND ID %i][BUFR ID %i] Unloaded sound data from RAM", sound.source, sound.buffer);
}

// Update sound buffer with new data
// NOTE: data must match sound.format
void UpdateSound(Sound sound, const void *data, int samplesCount)
{
#if USE_MINI_AL
    AudioBuffer *audioBuffer = (AudioBuffer *)sound.audioBuffer;
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "UpdateSound() : Invalid sound - no audio buffer");
        return;
    }

    StopAudioBuffer(audioBuffer);

    // TODO: May want to lock/unlock this since this data buffer is read at mixing time.
    memcpy(audioBuffer->buffer, data, samplesCount*audioBuffer->dsp.formatConverterIn.config.channels*mal_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn));
#else
    ALint sampleRate, sampleSize, channels;
    alGetBufferi(sound.buffer, AL_FREQUENCY, &sampleRate);
    alGetBufferi(sound.buffer, AL_BITS, &sampleSize);           // It could also be retrieved from sound.format
    alGetBufferi(sound.buffer, AL_CHANNELS, &channels);         // It could also be retrieved from sound.format

    TraceLog(LOG_DEBUG, "UpdateSound() : AL_FREQUENCY: %i", sampleRate);
    TraceLog(LOG_DEBUG, "UpdateSound() : AL_BITS: %i", sampleSize);
    TraceLog(LOG_DEBUG, "UpdateSound() : AL_CHANNELS: %i", channels);

    unsigned int dataSize = samplesCount*channels*sampleSize/8;   // Size of data in bytes

    alSourceStop(sound.source);                 // Stop sound
    alSourcei(sound.source, AL_BUFFER, 0);      // Unbind buffer from sound to update
    //alDeleteBuffers(1, &sound.buffer);          // Delete current buffer data
    //alGenBuffers(1, &sound.buffer);             // Generate new buffer

    // Upload new data to sound buffer
    alBufferData(sound.buffer, sound.format, data, dataSize, sampleRate);

    // Attach sound buffer to source again
    alSourcei(sound.source, AL_BUFFER, sound.buffer);
#endif
}

// Export wave data to file
void ExportWave(Wave wave, const char *fileName)
{
    bool success = false;
    
    if (IsFileExtension(fileName, ".wav"))
    {
        // Basic WAV headers structs
        typedef struct {
            char chunkID[4];
            int chunkSize;
            char format[4];
        } RiffHeader;

        typedef struct {
            char subChunkID[4];
            int subChunkSize;
            short audioFormat;
            short numChannels;
            int sampleRate;
            int byteRate;
            short blockAlign;
            short bitsPerSample;
        } WaveFormat;

        typedef struct {
            char subChunkID[4];
            int subChunkSize;
        } WaveData;
    
        RiffHeader riffHeader;
        WaveFormat waveFormat;
        WaveData waveData;

        // Fill structs with data
        riffHeader.chunkID[0] = 'R';
        riffHeader.chunkID[1] = 'I';
        riffHeader.chunkID[2] = 'F';
        riffHeader.chunkID[3] = 'F';
        riffHeader.chunkSize = 44 - 4 + wave.sampleCount*wave.sampleSize/8;
        riffHeader.format[0] = 'W';
        riffHeader.format[1] = 'A';
        riffHeader.format[2] = 'V';
        riffHeader.format[3] = 'E';

        waveFormat.subChunkID[0] = 'f';
        waveFormat.subChunkID[1] = 'm';
        waveFormat.subChunkID[2] = 't';
        waveFormat.subChunkID[3] = ' ';
        waveFormat.subChunkSize = 16;
        waveFormat.audioFormat = 1;
        waveFormat.numChannels = wave.channels;
        waveFormat.sampleRate = wave.sampleRate;
        waveFormat.byteRate = wave.sampleRate*wave.sampleSize/8;
        waveFormat.blockAlign = wave.sampleSize/8;
        waveFormat.bitsPerSample = wave.sampleSize;

        waveData.subChunkID[0] = 'd';
        waveData.subChunkID[1] = 'a';
        waveData.subChunkID[2] = 't';
        waveData.subChunkID[3] = 'a';
        waveData.subChunkSize = wave.sampleCount*wave.channels*wave.sampleSize/8;

        FILE *wavFile = fopen(fileName, "wb");
        
        if (wavFile == NULL) return;

        fwrite(&riffHeader, 1, sizeof(RiffHeader), wavFile);
        fwrite(&waveFormat, 1, sizeof(WaveFormat), wavFile);
        fwrite(&waveData, 1, sizeof(WaveData), wavFile);

        fwrite(wave.data, 1, wave.sampleCount*wave.channels*wave.sampleSize/8, wavFile);

        fclose(wavFile);
        
        success = true;
    }
    else if (IsFileExtension(fileName, ".raw")) { }   // TODO: Support additional file formats to export wave sample data

    if (success) TraceLog(LOG_INFO, "Wave exported successfully: %s", fileName);
    else TraceLog(LOG_WARNING, "Wave could not be exported.");
}

// Play a sound
void PlaySound(Sound sound)
{
#if USE_MINI_AL
    PlayAudioBuffer((AudioBuffer *)sound.audioBuffer);
#else
    alSourcePlay(sound.source);        // Play the sound
#endif

    //TraceLog(LOG_INFO, "Playing sound");

    // Find the current position of the sound being played
    // NOTE: Only work when the entire file is in a single buffer
    //int byteOffset;
    //alGetSourcei(sound.source, AL_BYTE_OFFSET, &byteOffset);
    //
    //int sampleRate;
    //alGetBufferi(sound.buffer, AL_FREQUENCY, &sampleRate);    // AL_CHANNELS, AL_BITS (bps)

    //float seconds = (float)byteOffset/sampleRate;      // Number of seconds since the beginning of the sound
    //or
    //float result;
    //alGetSourcef(sound.source, AL_SEC_OFFSET, &result);   // AL_SAMPLE_OFFSET
}

// Pause a sound
void PauseSound(Sound sound)
{
#if USE_MINI_AL
    PauseAudioBuffer((AudioBuffer *)sound.audioBuffer);
#else
    alSourcePause(sound.source);
#endif
}

// Resume a paused sound
void ResumeSound(Sound sound)
{
#if USE_MINI_AL
    ResumeAudioBuffer((AudioBuffer *)sound.audioBuffer);
#else
    ALenum state;

    alGetSourcei(sound.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED) alSourcePlay(sound.source);
#endif
}

// Stop reproducing a sound
void StopSound(Sound sound)
{
#if USE_MINI_AL
    StopAudioBuffer((AudioBuffer *)sound.audioBuffer);
#else
    alSourceStop(sound.source);
#endif
}

// Check if a sound is playing
bool IsSoundPlaying(Sound sound)
{
#if USE_MINI_AL
    return IsAudioBufferPlaying((AudioBuffer *)sound.audioBuffer);
#else
    bool playing = false;
    ALint state;

    alGetSourcei(sound.source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) playing = true;

    return playing;
#endif
}

// Set volume for a sound
void SetSoundVolume(Sound sound, float volume)
{
#if USE_MINI_AL
    SetAudioBufferVolume((AudioBuffer *)sound.audioBuffer, volume);
#else
    alSourcef(sound.source, AL_GAIN, volume);
#endif
}

// Set pitch for a sound
void SetSoundPitch(Sound sound, float pitch)
{
#if USE_MINI_AL
    SetAudioBufferPitch((AudioBuffer *)sound.audioBuffer, pitch);
#else
    alSourcef(sound.source, AL_PITCH, pitch);
#endif
}

// Convert wave data to desired format
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
{
#if USE_MINI_AL
    mal_format formatIn  = ((wave->sampleSize == 8) ? mal_format_u8 : ((wave->sampleSize == 16) ? mal_format_s16 : mal_format_f32));
    mal_format formatOut = ((      sampleSize == 8) ? mal_format_u8 : ((      sampleSize == 16) ? mal_format_s16 : mal_format_f32));

    mal_uint32 frameCountIn = wave->sampleCount;  // Is wave->sampleCount actually the frame count? That terminology needs to change, if so.

    mal_uint32 frameCount = (mal_uint32)mal_convert_frames(NULL, formatOut, channels, sampleRate, NULL, formatIn, wave->channels, wave->sampleRate, frameCountIn);
    if (frameCount == 0) 
    {
        TraceLog(LOG_ERROR, "WaveFormat() : Failed to get frame count for format conversion.");
        return;
    }

    void *data = malloc(frameCount*channels*(sampleSize/8));

    frameCount = (mal_uint32)mal_convert_frames(data, formatOut, channels, sampleRate, wave->data, formatIn, wave->channels, wave->sampleRate, frameCountIn);
    if (frameCount == 0) 
    {
        TraceLog(LOG_ERROR, "WaveFormat() : Format conversion failed.");
        return;
    }

    wave->sampleCount = frameCount;
    wave->sampleSize = sampleSize;
    wave->sampleRate = sampleRate;
    wave->channels = channels;
    free(wave->data);
    wave->data = data;

#else
    // Format sample rate
    // NOTE: Only supported 22050 <--> 44100
    if (wave->sampleRate != sampleRate)
    {
        // TODO: Resample wave data (upsampling or downsampling)
        // NOTE 1: To downsample, you have to drop samples or average them.
        // NOTE 2: To upsample, you have to interpolate new samples.

        wave->sampleRate = sampleRate;
    }

    // Format sample size
    // NOTE: Only supported 8 bit <--> 16 bit <--> 32 bit
    if (wave->sampleSize != sampleSize)
    {
        void *data = malloc(wave->sampleCount*wave->channels*sampleSize/8);

        for (int i = 0; i < wave->sampleCount; i++)
        {
            for (int j = 0; j < wave->channels; j++)
            {
                if (sampleSize == 8)
                {
                    if (wave->sampleSize == 16) ((unsigned char *)data)[wave->channels*i + j] = (unsigned char)(((float)(((short *)wave->data)[wave->channels*i + j])/32767.0f)*256);
                    else if (wave->sampleSize == 32) ((unsigned char *)data)[wave->channels*i + j] = (unsigned char)(((float *)wave->data)[wave->channels*i + j]*127.0f + 127);
                }
                else if (sampleSize == 16)
                {
                    if (wave->sampleSize == 8) ((short *)data)[wave->channels*i + j] = (short)(((float)(((unsigned char *)wave->data)[wave->channels*i + j] - 127)/256.0f)*32767);
                    else if (wave->sampleSize == 32) ((short *)data)[wave->channels*i + j] = (short)((((float *)wave->data)[wave->channels*i + j])*32767);
                }
                else if (sampleSize == 32)
                {
                    if (wave->sampleSize == 8) ((float *)data)[wave->channels*i + j] = (float)(((unsigned char *)wave->data)[wave->channels*i + j] - 127)/256.0f;
                    else if (wave->sampleSize == 16) ((float *)data)[wave->channels*i + j] = (float)(((short *)wave->data)[wave->channels*i + j])/32767.0f;
                }
            }
        }

        wave->sampleSize = sampleSize;
        free(wave->data);
        wave->data = data;
    }

    // Format channels (interlaced mode)
    // NOTE: Only supported mono <--> stereo
    if (wave->channels != channels)
    {
        void *data = malloc(wave->sampleCount*wave->sampleSize/8*channels);

        if ((wave->channels == 1) && (channels == 2))       // mono ---> stereo (duplicate mono information)
        {
            for (int i = 0; i < wave->sampleCount; i++)
            {
                for (int j = 0; j < channels; j++)
                {
                    if (wave->sampleSize == 8) ((unsigned char *)data)[channels*i + j] = ((unsigned char *)wave->data)[i];
                    else if (wave->sampleSize == 16) ((short *)data)[channels*i + j] = ((short *)wave->data)[i];
                    else if (wave->sampleSize == 32) ((float *)data)[channels*i + j] = ((float *)wave->data)[i];
                }
            }
        }
        else if ((wave->channels == 2) && (channels == 1))  // stereo ---> mono (mix stereo channels)
        {
            for (int i = 0, j = 0; i < wave->sampleCount; i++, j += 2)
            {
                if (wave->sampleSize == 8) ((unsigned char *)data)[i] = (((unsigned char *)wave->data)[j] + ((unsigned char *)wave->data)[j + 1])/2;
                else if (wave->sampleSize == 16) ((short *)data)[i] = (((short *)wave->data)[j] + ((short *)wave->data)[j + 1])/2;
                else if (wave->sampleSize == 32) ((float *)data)[i] = (((float *)wave->data)[j] + ((float *)wave->data)[j + 1])/2.0f;
            }
        }

        // TODO: Add/remove additional interlaced channels

        wave->channels = channels;
        free(wave->data);
        wave->data = data;
    }
#endif
}

// Copy a wave to a new wave
Wave WaveCopy(Wave wave)
{
    Wave newWave = { 0 };

    newWave.data = malloc(wave.sampleCount*wave.sampleSize/8*wave.channels);

    if (newWave.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newWave.data, wave.data, wave.sampleCount*wave.channels*wave.sampleSize/8);

        newWave.sampleCount = wave.sampleCount;
        newWave.sampleRate = wave.sampleRate;
        newWave.sampleSize = wave.sampleSize;
        newWave.channels = wave.channels;
    }

    return newWave;
}

// Crop a wave to defined samples range
// NOTE: Security check in case of out-of-range
void WaveCrop(Wave *wave, int initSample, int finalSample)
{
    if ((initSample >= 0) && (initSample < finalSample) &&
        (finalSample > 0) && ((unsigned int)finalSample < wave->sampleCount))
    {
        int sampleCount = finalSample - initSample;

        void *data = malloc(sampleCount*wave->sampleSize/8*wave->channels);

        memcpy(data, (unsigned char*)wave->data + (initSample*wave->channels*wave->sampleSize/8), sampleCount*wave->channels*wave->sampleSize/8);

        free(wave->data);
        wave->data = data;
    }
    else TraceLog(LOG_WARNING, "Wave crop range out of bounds");
}

// Get samples data from wave as a floats array
// NOTE: Returned sample values are normalized to range [-1..1]
float *GetWaveData(Wave wave)
{
    float *samples = (float *)malloc(wave.sampleCount*wave.channels*sizeof(float));

    for (unsigned int i = 0; i < wave.sampleCount; i++)
    {
        for (unsigned int j = 0; j < wave.channels; j++)
        {
            if (wave.sampleSize == 8) samples[wave.channels*i + j] = (float)(((unsigned char *)wave.data)[wave.channels*i + j] - 127)/256.0f;
            else if (wave.sampleSize == 16) samples[wave.channels*i + j] = (float)((short *)wave.data)[wave.channels*i + j]/32767.0f;
            else if (wave.sampleSize == 32) samples[wave.channels*i + j] = ((float *)wave.data)[wave.channels*i + j];
        }
    }

    return samples;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Music loading and stream playing (.OGG)
//----------------------------------------------------------------------------------

// Load music stream from file
Music LoadMusicStream(const char *fileName)
{
    Music music = (MusicData *)malloc(sizeof(MusicData));
    bool musicLoaded = true;

    if (IsFileExtension(fileName, ".ogg"))
    {
        // Open ogg audio stream
        music->ctxOgg = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (music->ctxOgg == NULL) musicLoaded = false;
        else
        {
            stb_vorbis_info info = stb_vorbis_get_info(music->ctxOgg);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music->stream = InitAudioStream(info.sample_rate, 16, info.channels);
            music->totalSamples = (unsigned int)stb_vorbis_stream_length_in_samples(music->ctxOgg); // Independent by channel
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_AUDIO_OGG;
            music->loopCount = -1;                       // Infinite loop by default

            TraceLog(LOG_DEBUG, "[%s] OGG total samples: %i", fileName, music->totalSamples);
            TraceLog(LOG_DEBUG, "[%s] OGG sample rate: %i", fileName, info.sample_rate);
            TraceLog(LOG_DEBUG, "[%s] OGG channels: %i", fileName, info.channels);
            TraceLog(LOG_DEBUG, "[%s] OGG memory required: %i", fileName, info.temp_memory_required);
        }
    }
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (IsFileExtension(fileName, ".flac"))
    {
        music->ctxFlac = drflac_open_file(fileName);

        if (music->ctxFlac == NULL) musicLoaded = false;
        else
        {
            music->stream = InitAudioStream(music->ctxFlac->sampleRate, music->ctxFlac->bitsPerSample, music->ctxFlac->channels);
            music->totalSamples = (unsigned int)music->ctxFlac->totalSampleCount/music->ctxFlac->channels;
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_AUDIO_FLAC;
            music->loopCount = -1;                       // Infinite loop by default

            TraceLog(LOG_DEBUG, "[%s] FLAC total samples: %i", fileName, music->totalSamples);
            TraceLog(LOG_DEBUG, "[%s] FLAC sample rate: %i", fileName, music->ctxFlac->sampleRate);
            TraceLog(LOG_DEBUG, "[%s] FLAC bits per sample: %i", fileName, music->ctxFlac->bitsPerSample);
            TraceLog(LOG_DEBUG, "[%s] FLAC channels: %i", fileName, music->ctxFlac->channels);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (IsFileExtension(fileName, ".mp3"))
    {
        drmp3_init_file(&music->ctxMp3, fileName, NULL);

        if (music->ctxMp3.framesRemaining <= 0) musicLoaded = false;
        else
        {
            music->stream = InitAudioStream(music->ctxMp3.sampleRate, 16, music->ctxMp3.channels);
            music->totalSamples = (unsigned int)music->ctxMp3.framesRemaining*music->ctxMp3.channels;
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_AUDIO_MP3;
            music->loopCount = -1;                       // Infinite loop by default

            TraceLog(LOG_DEBUG, "[%s] MP3 total samples: %i", fileName, music->totalSamples);
            TraceLog(LOG_DEBUG, "[%s] MP3 sample rate: %i", fileName, music->ctxMp3.sampleRate);
            //TraceLog(LOG_DEBUG, "[%s] MP3 bits per sample: %i", fileName, music->ctxMp3.bitsPerSample);
            TraceLog(LOG_DEBUG, "[%s] MP3 channels: %i", fileName, music->ctxMp3.channels);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (IsFileExtension(fileName, ".xm"))
    {
        int result = jar_xm_create_context_from_file(&music->ctxXm, 48000, fileName);

        if (!result)    // XM context created successfully
        {
            jar_xm_set_max_loop_count(music->ctxXm, 0); // Set infinite number of loops

            // NOTE: Only stereo is supported for XM
            music->stream = InitAudioStream(48000, 16, 2);
            music->totalSamples = (unsigned int)jar_xm_get_remaining_samples(music->ctxXm);
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_MODULE_XM;
            music->loopCount = -1;                       // Infinite loop by default

            TraceLog(LOG_DEBUG, "[%s] XM number of samples: %i", fileName, music->totalSamples);
            TraceLog(LOG_DEBUG, "[%s] XM track length: %11.6f sec", fileName, (float)music->totalSamples/48000.0f);
        }
        else musicLoaded = false;
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (IsFileExtension(fileName, ".mod"))
    {
        jar_mod_init(&music->ctxMod);

        if (jar_mod_load_file(&music->ctxMod, fileName))
        {
            music->stream = InitAudioStream(48000, 16, 2);
            music->totalSamples = (unsigned int)jar_mod_max_samples(&music->ctxMod);
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_MODULE_MOD;
            music->loopCount = -1;                       // Infinite loop by default

            TraceLog(LOG_DEBUG, "[%s] MOD number of samples: %i", fileName, music->samplesLeft);
            TraceLog(LOG_DEBUG, "[%s] MOD track length: %11.6f sec", fileName, (float)music->totalSamples/48000.0f);
        }
        else musicLoaded = false;
    }
#endif
    else musicLoaded = false;
    
    if (!musicLoaded)
    {
        if (music->ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close(music->ctxOgg);
    #if defined(SUPPORT_FILEFORMAT_FLAC)
        else if (music->ctxType == MUSIC_AUDIO_FLAC) drflac_free(music->ctxFlac);
    #endif
    #if defined(SUPPORT_FILEFORMAT_MP3)
        else if (music->ctxType == MUSIC_AUDIO_MP3) drmp3_uninit(&music->ctxMp3);
    #endif
    #if defined(SUPPORT_FILEFORMAT_XM)
        else if (music->ctxType == MUSIC_MODULE_XM) jar_xm_free_context(music->ctxXm);
    #endif
    #if defined(SUPPORT_FILEFORMAT_MOD)
        else if (music->ctxType == MUSIC_MODULE_MOD) jar_mod_unload(&music->ctxMod);
    #endif

        free(music);
        music = NULL;

        TraceLog(LOG_WARNING, "[%s] Music file could not be opened", fileName);
    }

    return music;
}

// Unload music stream
void UnloadMusicStream(Music music)
{
    CloseAudioStream(music->stream);

    if (music->ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close(music->ctxOgg);
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (music->ctxType == MUSIC_AUDIO_FLAC) drflac_free(music->ctxFlac);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (music->ctxType == MUSIC_AUDIO_MP3) drmp3_uninit(&music->ctxMp3);
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (music->ctxType == MUSIC_MODULE_XM) jar_xm_free_context(music->ctxXm);
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (music->ctxType == MUSIC_MODULE_MOD) jar_mod_unload(&music->ctxMod);
#endif

    free(music);
}

// Start music playing (open stream)
void PlayMusicStream(Music music)
{
#if USE_MINI_AL
    AudioBuffer *audioBuffer = (AudioBuffer *)music->stream.audioBuffer;
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "PlayMusicStream() : No audio buffer");
        return;
    }

    // For music streams, we need to make sure we maintain the frame cursor position. This is hack for this section of code in UpdateMusicStream()
    //     // NOTE: In case window is minimized, music stream is stopped,
    //     // just make sure to play again on window restore
    //     if (IsMusicPlaying(music)) PlayMusicStream(music);
    mal_uint32 frameCursorPos = audioBuffer->frameCursorPos;
    
    PlayAudioStream(music->stream); // <-- This resets the cursor position.

    audioBuffer->frameCursorPos = frameCursorPos;
#else
    alSourcePlay(music->stream.source);
#endif
}

// Pause music playing
void PauseMusicStream(Music music)
{
#if USE_MINI_AL
    PauseAudioStream(music->stream);
#else
    alSourcePause(music->stream.source);
#endif
}

// Resume music playing
void ResumeMusicStream(Music music)
{
#if USE_MINI_AL
    ResumeAudioStream(music->stream);
#else
    ALenum state;
    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED) 
    {
        TraceLog(LOG_INFO, "[AUD ID %i] Resume music stream playing", music->stream.source);
        alSourcePlay(music->stream.source);
    }
#endif
}

// Stop music playing (close stream)
// TODO: To clear a buffer, make sure they have been already processed!
void StopMusicStream(Music music)
{
#if USE_MINI_AL
    StopAudioStream(music->stream);
#else
    alSourceStop(music->stream.source);
    
    /*
    // Clear stream buffers
    // WARNING: Queued buffers must have been processed before unqueueing and reloaded with data!!!
    void *pcm = calloc(AUDIO_BUFFER_SIZE*music->stream.sampleSize/8*music->stream.channels, 1);
    
    for (int i = 0; i < MAX_STREAM_BUFFERS; i++)
    {
        //UpdateAudioStream(music->stream, pcm, AUDIO_BUFFER_SIZE);       // Update one buffer at a time
        alBufferData(music->stream.buffers[i], music->stream.format, pcm, AUDIO_BUFFER_SIZE*music->stream.sampleSize/8*music->stream.channels, music->stream.sampleRate);
    }

    free(pcm);
    */
#endif
    
    // Restart music context
    switch (music->ctxType)
    {
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_start(music->ctxOgg); break;
#if defined(SUPPORT_FILEFORMAT_FLAC)
        case MUSIC_AUDIO_FLAC: /* TODO: Restart FLAC context */ break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
        case MUSIC_AUDIO_MP3: /* TODO: Restart MP3 context */ break;
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
        case MUSIC_MODULE_XM: /* TODO: Restart XM context */ break;
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
        case MUSIC_MODULE_MOD: jar_mod_seek_start(&music->ctxMod); break;
#endif
        default: break;
    }

    music->samplesLeft = music->totalSamples;
}

// Update (re-fill) music buffers if data already processed
// TODO: Make sure buffers are ready for update... check music state
void UpdateMusicStream(Music music)
{
#if USE_MINI_AL
    bool streamEnding = false;

    unsigned int subBufferSizeInFrames = ((AudioBuffer *)music->stream.audioBuffer)->bufferSizeInFrames/2;

    // NOTE: Using dynamic allocation because it could require more than 16KB
    void *pcm = calloc(subBufferSizeInFrames*music->stream.sampleSize/8*music->stream.channels, 1);

    int samplesCount = 0;    // Total size of data steamed in L+R samples for xm floats, individual L or R for ogg shorts

    while (IsAudioBufferProcessed(music->stream))
    {
        if (music->samplesLeft >= subBufferSizeInFrames) samplesCount = subBufferSizeInFrames;
        else samplesCount = music->samplesLeft;

        // TODO: Really don't like ctxType thingy...
        switch (music->ctxType)
        {
            case MUSIC_AUDIO_OGG:
            {
                // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
                stb_vorbis_get_samples_short_interleaved(music->ctxOgg, music->stream.channels, (short *)pcm, samplesCount*music->stream.channels);

            } break;
        #if defined(SUPPORT_FILEFORMAT_FLAC)
            case MUSIC_AUDIO_FLAC:
            {
                // NOTE: Returns the number of samples to process
                unsigned int numSamplesFlac = (unsigned int)drflac_read_s16(music->ctxFlac, samplesCount*music->stream.channels, (short *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MP3)
            case MUSIC_AUDIO_MP3: 
            {
                // NOTE: Returns the number of samples to process
                unsigned int numSamplesMp3 = (unsigned int)drmp3_read_f32(&music->ctxMp3, samplesCount*music->stream.channels, (float *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_XM)
            case MUSIC_MODULE_XM: jar_xm_generate_samples_16bit(music->ctxXm, pcm, samplesCount); break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MOD)
            case MUSIC_MODULE_MOD: jar_mod_fillbuffer(&music->ctxMod, pcm, samplesCount, 0); break;
        #endif
            default: break;
        }

        UpdateAudioStream(music->stream, pcm, samplesCount);
        music->samplesLeft -= samplesCount;

        if (music->samplesLeft <= 0)
        {
            streamEnding = true;
            break;
        }
    }

    // Free allocated pcm data
    free(pcm);

    // Reset audio stream for looping
    if (streamEnding)
    {
        StopMusicStream(music);        // Stop music (and reset)
            
        // Decrease loopCount to stop when required
        if (music->loopCount > 0)
        {
            music->loopCount--;        // Decrease loop count
            PlayMusicStream(music);    // Play again
        }
        else
        {
            if (music->loopCount == -1) PlayMusicStream(music);
        }
    }
    else
    {
        // NOTE: In case window is minimized, music stream is stopped,
        // just make sure to play again on window restore
        if (IsMusicPlaying(music)) PlayMusicStream(music);
    }
#else
    ALenum state;
    ALint processed = 0;

    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);          // Get music stream state
    alGetSourcei(music->stream.source, AL_BUFFERS_PROCESSED, &processed); // Get processed buffers

    if (processed > 0)
    {
        bool streamEnding = false;

        // NOTE: Using dynamic allocation because it could require more than 16KB
        void *pcm = calloc(AUDIO_BUFFER_SIZE*music->stream.sampleSize/8*music->stream.channels, 1);

        int numBuffersToProcess = processed;
        int samplesCount = 0;    // Total size of data steamed in L+R samples for xm floats, 
                                 // individual L or R for ogg shorts

        for (int i = 0; i < numBuffersToProcess; i++)
        {
            if (music->samplesLeft >= AUDIO_BUFFER_SIZE) samplesCount = AUDIO_BUFFER_SIZE;
            else samplesCount = music->samplesLeft;

            // TODO: Really don't like ctxType thingy...
            switch (music->ctxType)
            {
                case MUSIC_AUDIO_OGG:
                {
                    // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
                    int numSamplesOgg = stb_vorbis_get_samples_short_interleaved(music->ctxOgg, music->stream.channels, (short *)pcm, samplesCount*music->stream.channels);

                } break;
            #if defined(SUPPORT_FILEFORMAT_FLAC)
                case MUSIC_AUDIO_FLAC:
                {
                    // NOTE: Returns the number of samples to process
                    unsigned int numSamplesFlac = (unsigned int)drflac_read_s16(music->ctxFlac, samplesCount*music->stream.channels, (short *)pcm);

                } break;
            #endif
            #if defined(SUPPORT_FILEFORMAT_MP3)
                case MUSIC_AUDIO_MP3:
                {
                    // NOTE: Returns the number of samples to process
                    unsigned int numSamplesMp3 = (unsigned int)drmp3_read_f32(&music->ctxMp3, samplesCount*music->stream.channels, (float *)pcm);
                } break;
            #endif
            #if defined(SUPPORT_FILEFORMAT_XM)
                case MUSIC_MODULE_XM: jar_xm_generate_samples_16bit(music->ctxXm, pcm, samplesCount); break;
            #endif
            #if defined(SUPPORT_FILEFORMAT_MOD)
                case MUSIC_MODULE_MOD: jar_mod_fillbuffer(&music->ctxMod, pcm, samplesCount, 0); break;
            #endif
                default: break;
            }

            UpdateAudioStream(music->stream, pcm, samplesCount);
            music->samplesLeft -= samplesCount;

            if (music->samplesLeft <= 0)
            {
                streamEnding = true;
                break;
            }
        }
        
        // Free allocated pcm data
        free(pcm);

        // Reset audio stream for looping
        if (streamEnding)
        {
            StopMusicStream(music);        // Stop music (and reset)
            
            // Decrease loopCount to stop when required
            if (music->loopCount > 0)
            {
                music->loopCount--;        // Decrease loop count
                PlayMusicStream(music);    // Play again
            }
            else
            {
                if (music->loopCount == -1)
                {
                    PlayMusicStream(music);
                }
            }
        }
        else
        {
            // NOTE: In case window is minimized, music stream is stopped,
            // just make sure to play again on window restore
            if (state != AL_PLAYING) PlayMusicStream(music);
        }
    }
#endif
}

// Check if any music is playing
bool IsMusicPlaying(Music music)
{
#if USE_MINI_AL
    return IsAudioStreamPlaying(music->stream);
#else
    bool playing = false;
    ALint state;

    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING) playing = true;

    return playing;
#endif
}

// Set volume for music
void SetMusicVolume(Music music, float volume)
{
#if USE_MINI_AL
    SetAudioStreamVolume(music->stream, volume);
#else
    alSourcef(music->stream.source, AL_GAIN, volume);
#endif
}

// Set pitch for music
void SetMusicPitch(Music music, float pitch)
{
#if USE_MINI_AL
    SetAudioStreamPitch(music->stream, pitch);
#else
    alSourcef(music->stream.source, AL_PITCH, pitch);
#endif
}

// Set music loop count (loop repeats)
// NOTE: If set to -1, means infinite loop
void SetMusicLoopCount(Music music, int count)
{
    music->loopCount = count;
}

// Get music time length (in seconds)
float GetMusicTimeLength(Music music)
{
    float totalSeconds = (float)music->totalSamples/music->stream.sampleRate;

    return totalSeconds;
}

// Get current music time played (in seconds)
float GetMusicTimePlayed(Music music)
{
    float secondsPlayed = 0.0f;

    unsigned int samplesPlayed = music->totalSamples - music->samplesLeft;
    secondsPlayed = (float)samplesPlayed/music->stream.sampleRate;

    return secondsPlayed;
}


// Init audio stream (to stream audio pcm data)
AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
{
    AudioStream stream = { 0 };

    stream.sampleRate = sampleRate;
    stream.sampleSize = sampleSize;

    // Only mono and stereo channels are supported, more channels require AL_EXT_MCFORMATS extension
    if ((channels > 0) && (channels < 3)) stream.channels = channels;
    else
    {
        TraceLog(LOG_WARNING, "Init audio stream: Number of channels not supported: %i", channels);
        stream.channels = 1;  // Fallback to mono channel
    }


#if USE_MINI_AL
    mal_format formatIn = ((stream.sampleSize == 8) ? mal_format_u8 : ((stream.sampleSize == 16) ? mal_format_s16 : mal_format_f32));

    // The size of a streaming buffer must be at least double the size of a period.
    unsigned int periodSize = device.bufferSizeInFrames / device.periods;
    unsigned int subBufferSize = AUDIO_BUFFER_SIZE;
    if (subBufferSize < periodSize) subBufferSize = periodSize;

    AudioBuffer *audioBuffer = CreateAudioBuffer(formatIn, stream.channels, stream.sampleRate, subBufferSize*2, AUDIO_BUFFER_USAGE_STREAM);
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "InitAudioStream() : Failed to create audio buffer");
        return stream;
    }

    audioBuffer->looping = true;    // Always loop for streaming buffers.
    stream.audioBuffer = audioBuffer;
#else
    // Setup OpenAL format
    if (stream.channels == 1)
    {
        switch (sampleSize)
        {
            case 8: stream.format = AL_FORMAT_MONO8; break;
            case 16: stream.format = AL_FORMAT_MONO16; break;
            case 32: stream.format = AL_FORMAT_MONO_FLOAT32; break;     // Requires OpenAL extension: AL_EXT_FLOAT32
            default: TraceLog(LOG_WARNING, "Init audio stream: Sample size not supported: %i", sampleSize); break;
        }
    }
    else if (stream.channels == 2)
    {
        switch (sampleSize)
        {
            case 8: stream.format = AL_FORMAT_STEREO8; break;
            case 16: stream.format = AL_FORMAT_STEREO16; break;
            case 32: stream.format = AL_FORMAT_STEREO_FLOAT32; break;   // Requires OpenAL extension: AL_EXT_FLOAT32
            default: TraceLog(LOG_WARNING, "Init audio stream: Sample size not supported: %i", sampleSize); break;
        }
    }

    // Create an audio source
    alGenSources(1, &stream.source);
    alSourcef(stream.source, AL_PITCH, 1.0f);
    alSourcef(stream.source, AL_GAIN, 1.0f);
    alSource3f(stream.source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(stream.source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);

    // Create Buffers (double buffering)
    alGenBuffers(MAX_STREAM_BUFFERS, stream.buffers);

    // Initialize buffer with zeros by default
    // NOTE: Using dynamic allocation because it requires more than 16KB
    void *pcm = calloc(AUDIO_BUFFER_SIZE*stream.sampleSize/8*stream.channels, 1);

    for (int i = 0; i < MAX_STREAM_BUFFERS; i++)
    {
        alBufferData(stream.buffers[i], stream.format, pcm, AUDIO_BUFFER_SIZE*stream.sampleSize/8*stream.channels, stream.sampleRate);
    }

    free(pcm);

    alSourceQueueBuffers(stream.source, MAX_STREAM_BUFFERS, stream.buffers);
#endif

    TraceLog(LOG_INFO, "[AUD ID %i] Audio stream loaded successfully (%i Hz, %i bit, %s)", stream.source, stream.sampleRate, stream.sampleSize, (stream.channels == 1) ? "Mono" : "Stereo");

    return stream;
}

// Close audio stream and free memory
void CloseAudioStream(AudioStream stream)
{
#if USE_MINI_AL
    DeleteAudioBuffer((AudioBuffer *)stream.audioBuffer);
#else
    // Stop playing channel
    alSourceStop(stream.source);

    // Flush out all queued buffers
    int queued = 0;
    alGetSourcei(stream.source, AL_BUFFERS_QUEUED, &queued);

    ALuint buffer = 0;

    while (queued > 0)
    {
        alSourceUnqueueBuffers(stream.source, 1, &buffer);
        queued--;
    }

    // Delete source and buffers
    alDeleteSources(1, &stream.source);
    alDeleteBuffers(MAX_STREAM_BUFFERS, stream.buffers);
#endif
    
    TraceLog(LOG_INFO, "[AUD ID %i] Unloaded audio stream data", stream.source);
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: unqueue -> update -> queue
// NOTE 2: To unqueue a buffer it needs to be processed: IsAudioBufferProcessed()
void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount)
{
#if USE_MINI_AL
    AudioBuffer *audioBuffer = (AudioBuffer *)stream.audioBuffer;
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "UpdateAudioStream() : No audio buffer");
        return;
    }

    if (audioBuffer->isSubBufferProcessed[0] || audioBuffer->isSubBufferProcessed[1])
    {
        mal_uint32 subBufferToUpdate;
        if (audioBuffer->isSubBufferProcessed[0] && audioBuffer->isSubBufferProcessed[1])
        {
            // Both buffers are available for updating. Update the first one and make sure the cursor is moved back to the front.
            subBufferToUpdate = 0;
            audioBuffer->frameCursorPos = 0;
        }
        else
        {
            // Just update whichever sub-buffer is processed.
            subBufferToUpdate = (audioBuffer->isSubBufferProcessed[0]) ? 0 : 1;
        }

        mal_uint32 subBufferSizeInFrames = audioBuffer->bufferSizeInFrames/2;
        unsigned char *subBuffer = audioBuffer->buffer + ((subBufferSizeInFrames*stream.channels*(stream.sampleSize/8))*subBufferToUpdate);

        // Does this API expect a whole buffer to be updated in one go? Assuming so, but if not will need to change this logic.
        if (subBufferSizeInFrames >= (mal_uint32)samplesCount)
        {
            mal_uint32 framesToWrite = subBufferSizeInFrames;
            if (framesToWrite > (mal_uint32)samplesCount) framesToWrite = (mal_uint32)samplesCount;

            mal_uint32 bytesToWrite = framesToWrite*stream.channels*(stream.sampleSize/8);
            memcpy(subBuffer, data, bytesToWrite);

            // Any leftover frames should be filled with zeros.
            mal_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;
            if (leftoverFrameCount > 0) 
            {
                memset(subBuffer + bytesToWrite, 0, leftoverFrameCount*stream.channels*(stream.sampleSize/8));
            }

            audioBuffer->isSubBufferProcessed[subBufferToUpdate] = false;
        }
        else
        {
            TraceLog(LOG_ERROR, "UpdateAudioStream() : Attempting to write too many frames to buffer");
            return;
        }
    }
    else
    {
        TraceLog(LOG_ERROR, "Audio buffer not available for updating");
        return;
    }
#else
    ALuint buffer = 0;
    alSourceUnqueueBuffers(stream.source, 1, &buffer);

    // Check if any buffer was available for unqueue
    if (alGetError() != AL_INVALID_VALUE)
    {
        alBufferData(buffer, stream.format, data, samplesCount*stream.sampleSize/8*stream.channels, stream.sampleRate);
        alSourceQueueBuffers(stream.source, 1, &buffer);
    }
    else TraceLog(LOG_WARNING, "[AUD ID %i] Audio buffer not available for unqueuing", stream.source);
#endif
}

// Check if any audio stream buffers requires refill
bool IsAudioBufferProcessed(AudioStream stream)
{
#if USE_MINI_AL
    AudioBuffer *audioBuffer = (AudioBuffer *)stream.audioBuffer;
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "IsAudioBufferProcessed() : No audio buffer");
        return false;
    }

    return audioBuffer->isSubBufferProcessed[0] || audioBuffer->isSubBufferProcessed[1];
#else
    ALint processed = 0;

    // Determine if music stream is ready to be written
    alGetSourcei(stream.source, AL_BUFFERS_PROCESSED, &processed);

    return (processed > 0);
#endif
}

// Play audio stream
void PlayAudioStream(AudioStream stream)
{
#if USE_MINI_AL
    PlayAudioBuffer((AudioBuffer *)stream.audioBuffer);
#else
    alSourcePlay(stream.source);
#endif
}

// Play audio stream
void PauseAudioStream(AudioStream stream)
{
#if USE_MINI_AL
    PauseAudioBuffer((AudioBuffer *)stream.audioBuffer);
#else
    alSourcePause(stream.source);
#endif
}

// Resume audio stream playing
void ResumeAudioStream(AudioStream stream)
{
#if USE_MINI_AL
    ResumeAudioBuffer((AudioBuffer *)stream.audioBuffer);
#else
    ALenum state;
    alGetSourcei(stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED) alSourcePlay(stream.source);
#endif
}

// Check if audio stream is playing.
bool IsAudioStreamPlaying(AudioStream stream)
{
#if USE_MINI_AL
    return IsAudioBufferPlaying((AudioBuffer *)stream.audioBuffer);
#else
    bool playing = false;
    ALint state;

    alGetSourcei(stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING) playing = true;

    return playing;
#endif
}

// Stop audio stream
void StopAudioStream(AudioStream stream)
{
#if USE_MINI_AL
    StopAudioBuffer((AudioBuffer *)stream.audioBuffer);
#else
    alSourceStop(stream.source);
#endif
}

void SetAudioStreamVolume(AudioStream stream, float volume)
{
#if USE_MINI_AL
    SetAudioBufferVolume((AudioBuffer *)stream.audioBuffer, volume);
#else
    alSourcef(stream.source, AL_GAIN, volume);
#endif
}

void SetAudioStreamPitch(AudioStream stream, float pitch)
{
#if USE_MINI_AL
    SetAudioBufferPitch((AudioBuffer *)stream.audioBuffer, pitch);
#else
    alSourcef(stream.source, AL_PITCH, pitch);
#endif
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

#if defined(SUPPORT_FILEFORMAT_WAV)
// Load WAV file into Wave structure
static Wave LoadWAV(const char *fileName)
{
    // Basic WAV headers structs
    typedef struct {
        char chunkID[4];
        int chunkSize;
        char format[4];
    } WAVRiffHeader;

    typedef struct {
        char subChunkID[4];
        int subChunkSize;
        short audioFormat;
        short numChannels;
        int sampleRate;
        int byteRate;
        short blockAlign;
        short bitsPerSample;
    } WAVFormat;

    typedef struct {
        char subChunkID[4];
        int subChunkSize;
    } WAVData;

    WAVRiffHeader wavRiffHeader;
    WAVFormat wavFormat;
    WAVData wavData;

    Wave wave = { 0 };
    FILE *wavFile;

    wavFile = fopen(fileName, "rb");

    if (wavFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] WAV file could not be opened", fileName);
        wave.data = NULL;
    }
    else
    {
        // Read in the first chunk into the struct
        fread(&wavRiffHeader, sizeof(WAVRiffHeader), 1, wavFile);

        // Check for RIFF and WAVE tags
        if (strncmp(wavRiffHeader.chunkID, "RIFF", 4) ||
            strncmp(wavRiffHeader.format, "WAVE", 4))
        {
                TraceLog(LOG_WARNING, "[%s] Invalid RIFF or WAVE Header", fileName);
        }
        else
        {
            // Read in the 2nd chunk for the wave info
            fread(&wavFormat, sizeof(WAVFormat), 1, wavFile);

            // Check for fmt tag
            if ((wavFormat.subChunkID[0] != 'f') || (wavFormat.subChunkID[1] != 'm') ||
                (wavFormat.subChunkID[2] != 't') || (wavFormat.subChunkID[3] != ' '))
            {
                TraceLog(LOG_WARNING, "[%s] Invalid Wave format", fileName);
            }
            else
            {
                // Check for extra parameters;
                if (wavFormat.subChunkSize > 16) fseek(wavFile, sizeof(short), SEEK_CUR);

                // Read in the the last byte of data before the sound file
                fread(&wavData, sizeof(WAVData), 1, wavFile);

                // Check for data tag
                if ((wavData.subChunkID[0] != 'd') || (wavData.subChunkID[1] != 'a') ||
                    (wavData.subChunkID[2] != 't') || (wavData.subChunkID[3] != 'a'))
                {
                    TraceLog(LOG_WARNING, "[%s] Invalid data header", fileName);
                }
                else
                {
                    // Allocate memory for data
                    wave.data = malloc(wavData.subChunkSize);

                    // Read in the sound data into the soundData variable
                    fread(wave.data, wavData.subChunkSize, 1, wavFile);

                    // Store wave parameters
                    wave.sampleRate = wavFormat.sampleRate;
                    wave.sampleSize = wavFormat.bitsPerSample;
                    wave.channels = wavFormat.numChannels;

                    // NOTE: Only support 8 bit, 16 bit and 32 bit sample sizes
                    if ((wave.sampleSize != 8) && (wave.sampleSize != 16) && (wave.sampleSize != 32))
                    {
                        TraceLog(LOG_WARNING, "[%s] WAV sample size (%ibit) not supported, converted to 16bit", fileName, wave.sampleSize);
                        WaveFormat(&wave, wave.sampleRate, 16, wave.channels);
                    }

                    // NOTE: Only support up to 2 channels (mono, stereo)
                    if (wave.channels > 2)
                    {
                        WaveFormat(&wave, wave.sampleRate, wave.sampleSize, 2);
                        TraceLog(LOG_WARNING, "[%s] WAV channels number (%i) not supported, converted to 2 channels", fileName, wave.channels);
                    }

                    // NOTE: subChunkSize comes in bytes, we need to translate it to number of samples
                    wave.sampleCount = (wavData.subChunkSize/(wave.sampleSize/8))/wave.channels;

                    TraceLog(LOG_INFO, "[%s] WAV file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1) ? "Mono" : "Stereo");
                }
            }
        }

        fclose(wavFile);
    }

    return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_OGG)
// Load OGG file into Wave structure
// NOTE: Using stb_vorbis library
static Wave LoadOGG(const char *fileName)
{
    Wave wave = { 0 };

    stb_vorbis *oggFile = stb_vorbis_open_filename(fileName, NULL, NULL);

    if (oggFile == NULL) TraceLog(LOG_WARNING, "[%s] OGG file could not be opened", fileName);
    else
    {
        stb_vorbis_info info = stb_vorbis_get_info(oggFile);
        
        wave.sampleRate = info.sample_rate;
        wave.sampleSize = 16;                   // 16 bit per sample (short)
        wave.channels = info.channels;
        wave.sampleCount = (int)stb_vorbis_stream_length_in_samples(oggFile);  // Independent by channel

        float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);
        if (totalSeconds > 10) TraceLog(LOG_WARNING, "[%s] Ogg audio length is larger than 10 seconds (%f), that's a big file in memory, consider music streaming", fileName, totalSeconds);

        wave.data = (short *)malloc(wave.sampleCount*wave.channels*sizeof(short));

        // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
        int numSamplesOgg = stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, (short *)wave.data, wave.sampleCount*wave.channels);

        TraceLog(LOG_DEBUG, "[%s] Samples obtained: %i", fileName, numSamplesOgg);

        TraceLog(LOG_INFO, "[%s] OGG file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1) ? "Mono" : "Stereo");

        stb_vorbis_close(oggFile);
    }

    return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)
// Load FLAC file into Wave structure
// NOTE: Using dr_flac library
static Wave LoadFLAC(const char *fileName)
{
    Wave wave;

    // Decode an entire FLAC file in one go
    uint64_t totalSampleCount;
    wave.data = drflac_open_and_decode_file_s16(fileName, &wave.channels, &wave.sampleRate, &totalSampleCount);

    wave.sampleCount = (int)totalSampleCount/wave.channels;
    wave.sampleSize = 16;

    // NOTE: Only support up to 2 channels (mono, stereo)
    if (wave.channels > 2) TraceLog(LOG_WARNING, "[%s] FLAC channels number (%i) not supported", fileName, wave.channels);

    if (wave.data == NULL) TraceLog(LOG_WARNING, "[%s] FLAC data could not be loaded", fileName);
    else TraceLog(LOG_INFO, "[%s] FLAC file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1) ? "Mono" : "Stereo");

    return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
// Load MP3 file into Wave structure
// NOTE: Using dr_mp3 library
static Wave LoadMP3(const char *fileName)
{
    Wave wave;

    // Decode an entire MP3 file in one go
    uint64_t totalSampleCount;
    drmp3_config *config;
    wave.data = drmp3_open_and_decode_file_f32(fileName, config, &totalSampleCount);
    
    wave.channels = config->outputChannels;
    wave.sampleRate = config->outputSampleRate;
    wave.sampleCount = (int)totalSampleCount/wave.channels;
    wave.sampleSize = 16;

    // NOTE: Only support up to 2 channels (mono, stereo)
    if (wave.channels > 2) TraceLog(LOG_WARNING, "[%s] MP3 channels number (%i) not supported", fileName, wave.channels);

    if (wave.data == NULL) TraceLog(LOG_WARNING, "[%s] MP3 data could not be loaded", fileName);
    else TraceLog(LOG_INFO, "[%s] MP3 file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1) ? "Mono" : "Stereo");

    return wave;
}
#endif

// Some required functions for audio standalone module version
#if defined(AUDIO_STANDALONE)
// Check file extension
bool IsFileExtension(const char *fileName, const char *ext)
{
    bool result = false;
    const char *fileExt;
    
    if ((fileExt = strrchr(fileName, '.')) != NULL)
    {
        if (strcmp(fileExt, ext) == 0) result = true;
    }

    return result;
}

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int msgType, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    switch (msgType)
    {
        case LOG_INFO: fprintf(stdout, "INFO: "); break;
        case LOG_ERROR: fprintf(stdout, "ERROR: "); break;
        case LOG_WARNING: fprintf(stdout, "WARNING: "); break;
        case LOG_DEBUG: fprintf(stdout, "DEBUG: "); break;
        default: break;
    }

    vfprintf(stdout, text, args);
    fprintf(stdout, "\n");

    va_end(args);

    if (msgType == LOG_ERROR) exit(1);
}
#endif
