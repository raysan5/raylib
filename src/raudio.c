/**********************************************************************************************
*
*   raudio - A simple and easy-to-use audio library based on miniaudio
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
*   #define RAUDIO_STANDALONE
*       Define to use the module as standalone library (independently of raylib).
*       Required types and functions are defined in the same module.
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
*   Copyright (c) 2013-2019 Ramon Santamaria (@raysan5)
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

#if defined(RAUDIO_STANDALONE)
    #include "raudio.h"
    #include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#else
    #include "raylib.h"         // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"         // Defines module configuration flags
#endif
    #include "utils.h"          // Required for: fopen() Android mapping
#endif

#define MA_NO_JACK
#define MINIAUDIO_IMPLEMENTATION
#include "external/miniaudio.h" // miniaudio library
#undef PlaySound                // Win32 API: windows.h > mmsystem.h defines PlaySound macro

#include <stdlib.h>             // Required for: malloc(), free()
#include <string.h>             // Required for: strcmp(), strncmp()
#include <stdio.h>              // Required for: FILE, fopen(), fclose(), fread()

#if defined(SUPPORT_FILEFORMAT_OGG)
    #define STB_VORBIS_IMPLEMENTATION
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
    #include "external/dr_mp3.h"        // MP3 loading functions
#endif

#if defined(_MSC_VER)
    #undef bool
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// NOTE: Music buffer size is defined by number of samples, independent of sample size and channels number
// After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds and a
// standard double-buffering system, a 4096 samples buffer has been chosen, it should be enough
// In case of music-stalls, just increase this number
#define AUDIO_BUFFER_SIZE        4096   // PCM data samples (i.e. 16bit, Mono: 8Kb)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Music context type
// NOTE: Depends on data structure provided by the library
// in charge of reading the different file types
typedef enum {
    MUSIC_AUDIO_WAV = 0,
    MUSIC_AUDIO_OGG,
    MUSIC_AUDIO_FLAC,
    MUSIC_AUDIO_MP3,
    MUSIC_MODULE_XM,
    MUSIC_MODULE_MOD
} MusicContextType;

#if defined(RAUDIO_STANDALONE)
typedef enum {
    LOG_ALL,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_NONE
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
static Wave LoadWAV(const char *fileName);              // Load WAV file
static int SaveWAV(Wave wave, const char *fileName);    // Save wave data as WAV file
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
static Wave LoadOGG(const char *fileName);              // Load OGG file
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
static Wave LoadFLAC(const char *fileName);             // Load FLAC file
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
static Wave LoadMP3(const char *fileName);              // Load MP3 file
#endif

#if defined(RAUDIO_STANDALONE)
bool IsFileExtension(const char *fileName, const char *ext);    // Check file extension
void TraceLog(int msgType, const char *text, ...);              // Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
#endif

//----------------------------------------------------------------------------------
// AudioBuffer Functionality
//----------------------------------------------------------------------------------
#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  44100

typedef enum { AUDIO_BUFFER_USAGE_STATIC = 0, AUDIO_BUFFER_USAGE_STREAM } AudioBufferUsage;

// Audio buffer structure
// NOTE: Slightly different logic is used when feeding data to the
// playback device depending on whether or not data is streamed
struct rAudioBuffer {
    ma_pcm_converter dsp;   // PCM data converter
    
    float volume;           // Audio buffer volume
    float pitch;            // Audio buffer pitch
    
    bool playing;           // Audio buffer state: AUDIO_PLAYING
    bool paused;            // Audio buffer state: AUDIO_PAUSED
    bool looping;           // Audio buffer looping, always true for AudioStreams
    int usage;              // Audio buffer usage mode: STATIC or STREAM

    bool isSubBufferProcessed[2];
    unsigned int frameCursorPos;        // Samples processed?
    unsigned int bufferSizeInFrames;
    
    rAudioBuffer *next;
    rAudioBuffer *prev;
    unsigned char *buffer;
};

#define AudioBuffer rAudioBuffer    // HACK: To avoid CoreAudio (macOS) symbol collision

// miniaudio global variables
static ma_context context;
static ma_device device;
static ma_mutex audioLock;
static bool isAudioInitialized = false;
static float masterVolume = 1.0f;

// Audio buffers are tracked in a linked list
static AudioBuffer *firstAudioBuffer = NULL;
static AudioBuffer *lastAudioBuffer = NULL;

// miniaudio functions declaration
static void OnLog(ma_context *pContext, ma_device *pDevice, ma_uint32 logLevel, const char *message);
static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount);
static ma_uint32 OnAudioBufferDSPRead(ma_pcm_converter *pDSP, void *pFramesOut, ma_uint32 frameCount, void *pUserData);
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume);

// AudioBuffer management functions declaration
// NOTE: Those functions are not exposed by raylib... for the moment
AudioBuffer *InitAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 bufferSizeInFrames, int usage);
void CloseAudioBuffer(AudioBuffer *buffer);
bool IsAudioBufferPlaying(AudioBuffer *buffer);
void PlayAudioBuffer(AudioBuffer *buffer);
void StopAudioBuffer(AudioBuffer *buffer);
void PauseAudioBuffer(AudioBuffer *buffer);
void ResumeAudioBuffer(AudioBuffer *buffer);
void SetAudioBufferVolume(AudioBuffer *buffer, float volume);
void SetAudioBufferPitch(AudioBuffer *buffer, float pitch);
void TrackAudioBuffer(AudioBuffer *buffer);
void UntrackAudioBuffer(AudioBuffer *buffer);

//----------------------------------------------------------------------------------
// Multi channel playback globals
//----------------------------------------------------------------------------------

// Number of channels in the audio pool
#define MAX_AUDIO_BUFFER_POOL_CHANNELS 16

// Audio buffer pool
AudioBuffer *audioBufferPool[MAX_AUDIO_BUFFER_POOL_CHANNELS] = { 0 };

// These are used to determine the oldest playing channel
unsigned long audioBufferPoolCounter = 0;
unsigned long audioBufferPoolChannels[MAX_AUDIO_BUFFER_POOL_CHANNELS] = { 0 };
//----------------------------------------------------------------------------------

// Log callback function
static void OnLog(ma_context *pContext, ma_device *pDevice, ma_uint32 logLevel, const char *message)
{
    (void)pContext;
    (void)pDevice;

    TraceLog(LOG_ERROR, message);   // All log messages from miniaudio are errors
}

// Sending audio data to device callback function
// NOTE: All the mixing takes place here
static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount)
{
    (void)pDevice;

    // Mixing is basically just an accumulation, we need to initialize the output buffer to 0
    memset(pFramesOut, 0, frameCount*pDevice->playback.channels*ma_get_bytes_per_sample(pDevice->playback.format));

    // Using a mutex here for thread-safety which makes things not real-time
    // This is unlikely to be necessary for this project, but may want to consider how you might want to avoid this
    ma_mutex_lock(&audioLock);
    {
        for (AudioBuffer *audioBuffer = firstAudioBuffer; audioBuffer != NULL; audioBuffer = audioBuffer->next)
        {
            // Ignore stopped or paused sounds
            if (!audioBuffer->playing || audioBuffer->paused) continue;

            ma_uint32 framesRead = 0;
            
            while (1)
            {
                if (framesRead > frameCount)
                {
                    TraceLog(LOG_DEBUG, "Mixed too many frames from audio buffer");
                    break;
                }

                if (framesRead == frameCount) break;

                // Just read as much data as we can from the stream
                ma_uint32 framesToRead = (frameCount - framesRead);
                
                while (framesToRead > 0)
                {
                    float tempBuffer[1024]; // 512 frames for stereo

                    ma_uint32 framesToReadRightNow = framesToRead;
                    if (framesToReadRightNow > sizeof(tempBuffer)/sizeof(tempBuffer[0])/DEVICE_CHANNELS)
                    {
                        framesToReadRightNow = sizeof(tempBuffer)/sizeof(tempBuffer[0])/DEVICE_CHANNELS;
                    }

                    ma_uint32 framesJustRead = (ma_uint32)ma_pcm_converter_read(&audioBuffer->dsp, tempBuffer, framesToReadRightNow);
                    if (framesJustRead > 0)
                    {
                        float *framesOut = (float *)pFramesOut + (framesRead*device.playback.channels);
                        float *framesIn  = tempBuffer;
                        MixAudioFrames(framesOut, framesIn, framesJustRead, audioBuffer->volume);

                        framesToRead -= framesJustRead;
                        framesRead += framesJustRead;
                    }

                    // If we weren't able to read all the frames we requested, break
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
                            // move the cursor position back to the start and continue the loop
                            audioBuffer->frameCursorPos = 0;
                            continue;
                        }
                    }
                }

                // If for some reason we weren't able to read every frame we'll need to break from the loop
                // Not doing this could theoretically put us into an infinite loop
                if (framesToRead > 0) break;
            }
        }
    }

    ma_mutex_unlock(&audioLock);
}

// DSP read from audio buffer callback function
static ma_uint32 OnAudioBufferDSPRead(ma_pcm_converter *pDSP, void *pFramesOut, ma_uint32 frameCount, void *pUserData)
{
    AudioBuffer *audioBuffer = (AudioBuffer *)pUserData;

    ma_uint32 subBufferSizeInFrames = (audioBuffer->bufferSizeInFrames > 1)? audioBuffer->bufferSizeInFrames/2 : audioBuffer->bufferSizeInFrames;
    ma_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos/subBufferSizeInFrames;

    if (currentSubBufferIndex > 1)
    {
        TraceLog(LOG_DEBUG, "Frame cursor position moved too far forward in audio stream");
        return 0;
    }

    // Another thread can update the processed state of buffers so
    // we just take a copy here to try and avoid potential synchronization problems
    bool isSubBufferProcessed[2];
    isSubBufferProcessed[0] = audioBuffer->isSubBufferProcessed[0];
    isSubBufferProcessed[1] = audioBuffer->isSubBufferProcessed[1];

    ma_uint32 frameSizeInBytes = ma_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn)*audioBuffer->dsp.formatConverterIn.config.channels;

    // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0
    ma_uint32 framesRead = 0;
    while (1)
    {
        // We break from this loop differently depending on the buffer's usage
        //  - For static buffers, we simply fill as much data as we can
        //  - For streaming buffers we only fill the halves of the buffer that are processed
        //    Unprocessed halves must keep their audio data in-tact
        if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC)
        {
            if (framesRead >= frameCount) break;
        }
        else 
        {
            if (isSubBufferProcessed[currentSubBufferIndex]) break;
        }

        ma_uint32 totalFramesRemaining = (frameCount - framesRead);
        if (totalFramesRemaining == 0) break;

        ma_uint32 framesRemainingInOutputBuffer;
        if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC)
        {
            framesRemainingInOutputBuffer = audioBuffer->bufferSizeInFrames - audioBuffer->frameCursorPos;
        }
        else
        {
            ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames*currentSubBufferIndex;
            framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos - firstFrameIndexOfThisSubBuffer);
        }

        ma_uint32 framesToRead = totalFramesRemaining;
        if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

        memcpy((unsigned char *)pFramesOut + (framesRead*frameSizeInBytes), audioBuffer->buffer + (audioBuffer->frameCursorPos*frameSizeInBytes), framesToRead*frameSizeInBytes);
        audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead)%audioBuffer->bufferSizeInFrames;
        framesRead += framesToRead;

        // If we've read to the end of the buffer, mark it as processed
        if (framesToRead == framesRemainingInOutputBuffer)
        {
            audioBuffer->isSubBufferProcessed[currentSubBufferIndex] = true;
            isSubBufferProcessed[currentSubBufferIndex] = true;

            currentSubBufferIndex = (currentSubBufferIndex + 1)%2;

            // We need to break from this loop if we're not looping
            if (!audioBuffer->looping)
            {
                StopAudioBuffer(audioBuffer);
                break;
            }
        }
    }

    // Zero-fill excess
    ma_uint32 totalFramesRemaining = (frameCount - framesRead);
    if (totalFramesRemaining > 0)
    {
        memset((unsigned char *)pFramesOut + (framesRead*frameSizeInBytes), 0, totalFramesRemaining*frameSizeInBytes);

        // For static buffers we can fill the remaining frames with silence for safety, but we don't want
        // to report those frames as "read". The reason for this is that the caller uses the return value
        // to know whether or not a non-looping sound has finished playback.
        if (audioBuffer->usage != AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
    }

    return framesRead;
}

// This is the main mixing function. Mixing is pretty simple in this project - it's just an accumulation.
// NOTE: framesOut is both an input and an output. It will be initially filled with zeros outside of this function.
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume)
{
    for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame)
    {
        for (ma_uint32 iChannel = 0; iChannel < device.playback.channels; ++iChannel)
        {
            float *frameOut = framesOut + (iFrame*device.playback.channels);
            const float *frameIn  = framesIn  + (iFrame*device.playback.channels);

            frameOut[iChannel] += (frameIn[iChannel]*masterVolume*localVolume);
        }
    }
}

// Initialise the multichannel buffer pool
static void InitAudioBufferPool()
{
    // Dummy buffers
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++) 
    {
        audioBufferPool[i] = InitAudioBuffer(DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, 0, AUDIO_BUFFER_USAGE_STATIC);
    }
}

// Close the audio buffers pool
static void CloseAudioBufferPool()
{
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++) RL_FREE(audioBufferPool[i]);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------
// Initialize audio device
void InitAudioDevice(void)
{
    // Init audio context
    ma_context_config contextConfig = ma_context_config_init();
    
    contextConfig.logCallback = OnLog;
    
    ma_result result = ma_context_init(NULL, 0, &contextConfig, &context);
    if (result != MA_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to initialize audio context");
        return;
    }

    // Init audio device
    // NOTE: Using the default device. Format is floating point because it simplifies mixing.
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;  // NULL for the default playback device.
    config.playback.format    = DEVICE_FORMAT;
    config.playback.channels  = DEVICE_CHANNELS;
    config.capture.pDeviceID  = NULL;  // NULL for the default capture device.
    config.capture.format     = ma_format_s16;
    config.capture.channels   = 1;
    config.sampleRate         = DEVICE_SAMPLE_RATE;
    config.dataCallback       = OnSendAudioDataToDevice;
    config.pUserData          = NULL;

    result = ma_device_init(&context, &config, &device);
    if (result != MA_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to initialize audio playback device");
        ma_context_uninit(&context);
        return;
    }

    // Keep the device running the whole time. May want to consider doing something a bit smarter and only have the device running
    // while there's at least one sound being played.
    result = ma_device_start(&device);
    if (result != MA_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to start audio playback device");
        ma_device_uninit(&device);
        ma_context_uninit(&context);
        return;
    }

    // Mixing happens on a seperate thread which means we need to synchronize. I'm using a mutex here to make things simple, but may
    // want to look at something a bit smarter later on to keep everything real-time, if that's necessary.
    if (ma_mutex_init(&context, &audioLock) != MA_SUCCESS)
    {
        TraceLog(LOG_ERROR, "Failed to create mutex for audio mixing");
        ma_device_uninit(&device);
        ma_context_uninit(&context);
        return;
    }

    TraceLog(LOG_INFO, "Audio device initialized successfully");
    TraceLog(LOG_INFO, "Audio backend: miniaudio / %s", ma_get_backend_name(context.backend));
    TraceLog(LOG_INFO, "Audio format: %s -> %s", ma_get_format_name(device.playback.format), ma_get_format_name(device.playback.internalFormat));
    TraceLog(LOG_INFO, "Audio channels: %d -> %d", device.playback.channels, device.playback.internalChannels);
    TraceLog(LOG_INFO, "Audio sample rate: %d -> %d", device.sampleRate, device.playback.internalSampleRate);
    TraceLog(LOG_INFO, "Audio buffer size: %d", device.playback.internalBufferSizeInFrames);

    InitAudioBufferPool();
    TraceLog(LOG_INFO, "Audio multichannel pool size: %i", MAX_AUDIO_BUFFER_POOL_CHANNELS);

    isAudioInitialized = true;
}

// Close the audio device for all contexts
void CloseAudioDevice(void)
{
    if (!isAudioInitialized)
    {
        TraceLog(LOG_WARNING, "Could not close audio device because it is not currently initialized");
    }
    else
    {
        ma_mutex_uninit(&audioLock);
        ma_device_uninit(&device);
        ma_context_uninit(&context);

        CloseAudioBufferPool();

        TraceLog(LOG_INFO, "Audio device closed successfully");
    }
}

// Check if device has been initialized successfully
bool IsAudioDeviceReady(void)
{
    return isAudioInitialized;
}

// Set master volume (listener)
void SetMasterVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    else if (volume > 1.0f) volume = 1.0f;

    masterVolume = volume;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Buffer management
//----------------------------------------------------------------------------------

// Create a new audio buffer. Initially filled with silence
AudioBuffer *InitAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 bufferSizeInFrames, int usage)
{
    AudioBuffer *audioBuffer = (AudioBuffer *)RL_CALLOC(sizeof(*audioBuffer), 1);
    audioBuffer->buffer = RL_CALLOC((bufferSizeInFrames*channels*ma_get_bytes_per_sample(format)), 1);
    
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "InitAudioBuffer() : Failed to allocate memory for audio buffer");
        return NULL;
    }

    // Audio data runs through a format converter
    ma_pcm_converter_config dspConfig;
    memset(&dspConfig, 0, sizeof(dspConfig));
    dspConfig.formatIn = format;
    dspConfig.formatOut = DEVICE_FORMAT;
    dspConfig.channelsIn = channels;
    dspConfig.channelsOut = DEVICE_CHANNELS;
    dspConfig.sampleRateIn = sampleRate;
    dspConfig.sampleRateOut = DEVICE_SAMPLE_RATE;
    dspConfig.onRead = OnAudioBufferDSPRead;        // Callback on data reading
    dspConfig.pUserData = audioBuffer;              // Audio data pointer
    dspConfig.allowDynamicSampleRate = true;        // Required for pitch shifting
    
    ma_result result = ma_pcm_converter_init(&dspConfig, &audioBuffer->dsp);

    if (result != MA_SUCCESS)
    {
        TraceLog(LOG_ERROR, "InitAudioBuffer() : Failed to create data conversion pipeline");
        RL_FREE(audioBuffer);
        return NULL;
    }

    // Init audio buffer values
    audioBuffer->volume = 1.0f;
    audioBuffer->pitch = 1.0f;
    audioBuffer->playing = false;
    audioBuffer->paused = false;
    audioBuffer->looping = false;
    audioBuffer->usage = usage;
    audioBuffer->frameCursorPos = 0;
    audioBuffer->bufferSizeInFrames = bufferSizeInFrames;

    // Buffers should be marked as processed by default so that a call to
    // UpdateAudioStream() immediately after initialization works correctly
    audioBuffer->isSubBufferProcessed[0] = true;
    audioBuffer->isSubBufferProcessed[1] = true;

    // Track audio buffer to linked list next position
    TrackAudioBuffer(audioBuffer);

    return audioBuffer;
}

// Delete an audio buffer
void CloseAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        UntrackAudioBuffer(buffer);
        RL_FREE(buffer->buffer);
        RL_FREE(buffer);
    }
    else TraceLog(LOG_ERROR, "CloseAudioBuffer() : No audio buffer");
}

// Check if an audio buffer is playing
bool IsAudioBufferPlaying(AudioBuffer *buffer)
{
    bool result = false;
    
    if (buffer != NULL) result = (buffer->playing && !buffer->paused);
    else TraceLog(LOG_ERROR, "IsAudioBufferPlaying() : No audio buffer");

    return result;
}

// Play an audio buffer
// NOTE: Buffer is restarted to the start.
// Use PauseAudioBuffer() and ResumeAudioBuffer() if the playback position should be maintained.
void PlayAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        buffer->playing = true;
        buffer->paused = false;
        buffer->frameCursorPos = 0;
    }
    else TraceLog(LOG_ERROR, "PlayAudioBuffer() : No audio buffer");
}

// Stop an audio buffer
void StopAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        if (IsAudioBufferPlaying(buffer))
        {
            buffer->playing = false;
            buffer->paused = false;
            buffer->frameCursorPos = 0;
            buffer->isSubBufferProcessed[0] = true;
            buffer->isSubBufferProcessed[1] = true;
        }
    }
    else TraceLog(LOG_ERROR, "StopAudioBuffer() : No audio buffer");
}

// Pause an audio buffer
void PauseAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL) buffer->paused = true;
    else TraceLog(LOG_ERROR, "PauseAudioBuffer() : No audio buffer");    
}

// Resume an audio buffer
void ResumeAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL) buffer->paused = false;
    else TraceLog(LOG_ERROR, "ResumeAudioBuffer() : No audio buffer");
}

// Set volume for an audio buffer
void SetAudioBufferVolume(AudioBuffer *buffer, float volume)
{
    if (buffer != NULL) buffer->volume = volume;
    else TraceLog(LOG_WARNING, "SetAudioBufferVolume() : No audio buffer");
}

// Set pitch for an audio buffer
void SetAudioBufferPitch(AudioBuffer *buffer, float pitch)
{
    if (buffer != NULL)
    {
        float pitchMul = pitch/buffer->pitch;

        // Pitching is just an adjustment of the sample rate. Note that this changes the duration of the sound - higher pitches
        // will make the sound faster; lower pitches make it slower.
        ma_uint32 newOutputSampleRate = (ma_uint32)((float)buffer->dsp.src.config.sampleRateOut/pitchMul);
        buffer->pitch *= (float)buffer->dsp.src.config.sampleRateOut/newOutputSampleRate;

        ma_pcm_converter_set_output_sample_rate(&buffer->dsp, newOutputSampleRate);
    }
    else TraceLog(LOG_WARNING, "SetAudioBufferPitch() : No audio buffer");
}

// Track audio buffer to linked list next position
void TrackAudioBuffer(AudioBuffer *buffer)
{
    ma_mutex_lock(&audioLock);
    {
        if (firstAudioBuffer == NULL) firstAudioBuffer = buffer;
        else
        {
            lastAudioBuffer->next = buffer;
            buffer->prev = lastAudioBuffer;
        }

        lastAudioBuffer = buffer;
    }
    ma_mutex_unlock(&audioLock);
}

// Untrack audio buffer from linked list
void UntrackAudioBuffer(AudioBuffer *buffer)
{
    ma_mutex_lock(&audioLock);
    {
        if (buffer->prev == NULL) firstAudioBuffer = buffer->next;
        else buffer->prev->next = buffer->next;

        if (buffer->next == NULL) lastAudioBuffer = buffer->prev;
        else buffer->next->prev = buffer->prev;

        buffer->prev = NULL;
        buffer->next = NULL;
    }
    ma_mutex_unlock(&audioLock);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Sounds loading and playing (.WAV)
//----------------------------------------------------------------------------------

// Load wave data from file
Wave LoadWave(const char *fileName)
{
    Wave wave = { 0 };

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if (IsFileExtension(fileName, ".wav")) wave = LoadWAV(fileName);
#endif
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
        // When using miniaudio we need to do our own mixing. 
        // To simplify this we need convert the format of each sound to be consistent with
        // the format used to open the playback device. We can do this two ways:
        //
        //   1) Convert the whole sound in one go at load time (here).
        //   2) Convert the audio data in chunks at mixing time.
        //
        // First option has been selected, format conversion is done on the loading stage.
        // The downside is that it uses more memory if the original sound is u8 or s16.
        ma_format formatIn  = ((wave.sampleSize == 8)? ma_format_u8 : ((wave.sampleSize == 16)? ma_format_s16 : ma_format_f32));
        ma_uint32 frameCountIn = wave.sampleCount/wave.channels;

        ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, NULL, formatIn, wave.channels, wave.sampleRate, frameCountIn);
        if (frameCount == 0) TraceLog(LOG_WARNING, "LoadSoundFromWave() : Failed to get frame count for format conversion");

        AudioBuffer *audioBuffer = InitAudioBuffer(DEVICE_FORMAT, DEVICE_CHANNELS, DEVICE_SAMPLE_RATE, frameCount, AUDIO_BUFFER_USAGE_STATIC);
        if (audioBuffer == NULL) TraceLog(LOG_WARNING, "LoadSoundFromWave() : Failed to create audio buffer");

        frameCount = (ma_uint32)ma_convert_frames(audioBuffer->buffer, audioBuffer->dsp.formatConverterIn.config.formatIn, audioBuffer->dsp.formatConverterIn.config.channels, audioBuffer->dsp.src.config.sampleRateIn, wave.data, formatIn, wave.channels, wave.sampleRate, frameCountIn);
        if (frameCount == 0) TraceLog(LOG_WARNING, "LoadSoundFromWave() : Format conversion failed");

        sound.sampleCount = frameCount*DEVICE_CHANNELS;
        sound.stream.sampleRate = DEVICE_SAMPLE_RATE;
        sound.stream.sampleSize = 32;
        sound.stream.channels = DEVICE_CHANNELS;
        sound.stream.buffer = audioBuffer;
    }

    return sound;
}

// Unload wave data
void UnloadWave(Wave wave)
{
    if (wave.data != NULL) RL_FREE(wave.data);

    TraceLog(LOG_INFO, "Unloaded wave data from RAM");
}

// Unload sound
void UnloadSound(Sound sound)
{
    CloseAudioBuffer(sound.stream.buffer);

    TraceLog(LOG_INFO, "Unloaded sound data from RAM");
}

// Update sound buffer with new data
void UpdateSound(Sound sound, const void *data, int samplesCount)
{
    AudioBuffer *audioBuffer = sound.stream.buffer;

    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "UpdateSound() : Invalid sound - no audio buffer");
        return;
    }

    StopAudioBuffer(audioBuffer);

    // TODO: May want to lock/unlock this since this data buffer is read at mixing time
    memcpy(audioBuffer->buffer, data, samplesCount*audioBuffer->dsp.formatConverterIn.config.channels*ma_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn));
}

// Export wave data to file
void ExportWave(Wave wave, const char *fileName)
{
    bool success = false;

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if (IsFileExtension(fileName, ".wav")) success = SaveWAV(wave, fileName);
#endif
    else if (IsFileExtension(fileName, ".raw"))
    {
        // Export raw sample data (without header)
        // NOTE: It's up to the user to track wave parameters
        FILE *rawFile = fopen(fileName, "wb");
        success = fwrite(wave.data, wave.sampleCount*wave.channels*wave.sampleSize/8, 1, rawFile);
        fclose(rawFile);
    }

    if (success) TraceLog(LOG_INFO, "Wave exported successfully: %s", fileName);
    else TraceLog(LOG_WARNING, "Wave could not be exported.");
}

// Export wave sample data to code (.h)
void ExportWaveAsCode(Wave wave, const char *fileName)
{
    #define BYTES_TEXT_PER_LINE     20

    char varFileName[256] = { 0 };
    int dataSize = wave.sampleCount*wave.channels*wave.sampleSize/8;

    FILE *txtFile = fopen(fileName, "wt");

    fprintf(txtFile, "\n//////////////////////////////////////////////////////////////////////////////////\n");
    fprintf(txtFile, "//                                                                              //\n");
    fprintf(txtFile, "// WaveAsCode exporter v1.0 - Wave data exported as an array of bytes           //\n");
    fprintf(txtFile, "//                                                                              //\n");
    fprintf(txtFile, "// more info and bugs-report:  github.com/raysan5/raylib                        //\n");
    fprintf(txtFile, "// feedback and support:       ray[at]raylib.com                                //\n");
    fprintf(txtFile, "//                                                                              //\n");
    fprintf(txtFile, "// Copyright (c) 2018 Ramon Santamaria (@raysan5)                               //\n");
    fprintf(txtFile, "//                                                                              //\n");
    fprintf(txtFile, "//////////////////////////////////////////////////////////////////////////////////\n\n");

#if !defined(RAUDIO_STANDALONE)
    // Get file name from path and convert variable name to uppercase
    strcpy(varFileName, GetFileNameWithoutExt(fileName));
    for (int i = 0; varFileName[i] != '\0'; i++) if (varFileName[i] >= 'a' && varFileName[i] <= 'z') { varFileName[i] = varFileName[i] - 32; }
#else
    strcpy(varFileName, fileName);
#endif

    fprintf(txtFile, "// Wave data information\n");
    fprintf(txtFile, "#define %s_SAMPLE_COUNT     %i\n", varFileName, wave.sampleCount);
    fprintf(txtFile, "#define %s_SAMPLE_RATE      %i\n", varFileName, wave.sampleRate);
    fprintf(txtFile, "#define %s_SAMPLE_SIZE      %i\n", varFileName, wave.sampleSize);
    fprintf(txtFile, "#define %s_CHANNELS         %i\n\n", varFileName, wave.channels);

    // Write byte data as hexadecimal text
    fprintf(txtFile, "static unsigned char %s_DATA[%i] = { ", varFileName, dataSize);
    for (int i = 0; i < dataSize - 1; i++) fprintf(txtFile, ((i%BYTES_TEXT_PER_LINE == 0)? "0x%x,\n" : "0x%x, "), ((unsigned char *)wave.data)[i]);
    fprintf(txtFile, "0x%x };\n", ((unsigned char *)wave.data)[dataSize - 1]);

    fclose(txtFile);
}

// Play a sound
void PlaySound(Sound sound)
{
    PlayAudioBuffer(sound.stream.buffer);
}

// Play a sound in the multichannel buffer pool
void PlaySoundMulti(Sound sound)
{
    int index = -1;
    unsigned long oldAge = 0;
    int oldIndex = -1;

    // find the first non playing pool entry
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
    {
        if (audioBufferPoolChannels[i] > oldAge)
        {
            oldAge = audioBufferPoolChannels[i];
            oldIndex = i;
        }
        
        if (!IsAudioBufferPlaying(audioBufferPool[i]))
        {
            index = i;
            break;
        }
    }

    // If no none playing pool members can be index choose the oldest
    if (index == -1)
    {
        TraceLog(LOG_WARNING,"pool age %i ended a sound early no room in buffer pool", audioBufferPoolCounter);
        
        if (oldIndex == -1)
        {
            // Shouldn't be able to get here... but just in case something odd happens!
            TraceLog(LOG_ERROR,"sound buffer pool couldn't determine oldest buffer not playing sound");
            
            return;
        }
        
        index = oldIndex;
        
        // Just in case...
        StopAudioBuffer(audioBufferPool[index]);
    }

    // Experimentally mutex lock doesn't seem to be needed this makes sense
    // as audioBufferPool[index] isn't playing and the only stuff we're copying
    // shouldn't be changing...

    audioBufferPoolChannels[index] = audioBufferPoolCounter;
    audioBufferPoolCounter++;
    
    audioBufferPool[index]->volume = sound.stream.buffer->volume;
    audioBufferPool[index]->pitch = sound.stream.buffer->pitch;
    audioBufferPool[index]->looping = sound.stream.buffer->looping;
    audioBufferPool[index]->usage = sound.stream.buffer->usage;
    audioBufferPool[index]->isSubBufferProcessed[0] = false;
    audioBufferPool[index]->isSubBufferProcessed[1] = false;
    audioBufferPool[index]->bufferSizeInFrames = sound.stream.buffer->bufferSizeInFrames;
    audioBufferPool[index]->buffer = sound.stream.buffer->buffer;

    PlayAudioBuffer(audioBufferPool[index]);
}

// Stop any sound played with PlaySoundMulti()
void StopSoundMulti(void)
{
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++) StopAudioBuffer(audioBufferPool[i]);
}

// Get number of sounds playing in the multichannel buffer pool
int GetSoundsPlaying(void)
{
    int counter = 0;
    
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
    {
        if (IsAudioBufferPlaying(audioBufferPool[i])) counter++;
    }
    
    return counter;
}

// Pause a sound
void PauseSound(Sound sound)
{
    PauseAudioBuffer(sound.stream.buffer);
}

// Resume a paused sound
void ResumeSound(Sound sound)
{
    ResumeAudioBuffer(sound.stream.buffer);
}

// Stop reproducing a sound
void StopSound(Sound sound)
{
    StopAudioBuffer(sound.stream.buffer);
}

// Check if a sound is playing
bool IsSoundPlaying(Sound sound)
{
    return IsAudioBufferPlaying(sound.stream.buffer);
}

// Set volume for a sound
void SetSoundVolume(Sound sound, float volume)
{
    SetAudioBufferVolume(sound.stream.buffer, volume);
}

// Set pitch for a sound
void SetSoundPitch(Sound sound, float pitch)
{
    SetAudioBufferPitch(sound.stream.buffer, pitch);
}

// Convert wave data to desired format
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
{
    ma_format formatIn  = ((wave->sampleSize == 8)? ma_format_u8 : ((wave->sampleSize == 16)? ma_format_s16 : ma_format_f32));
    ma_format formatOut = ((      sampleSize == 8)? ma_format_u8 : ((      sampleSize == 16)? ma_format_s16 : ma_format_f32));

    ma_uint32 frameCountIn = wave->sampleCount;  // Is wave->sampleCount actually the frame count? That terminology needs to change, if so.

    ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, formatOut, channels, sampleRate, NULL, formatIn, wave->channels, wave->sampleRate, frameCountIn);
    if (frameCount == 0)
    {
        TraceLog(LOG_ERROR, "WaveFormat() : Failed to get frame count for format conversion.");
        return;
    }

    void *data = RL_MALLOC(frameCount*channels*(sampleSize/8));

    frameCount = (ma_uint32)ma_convert_frames(data, formatOut, channels, sampleRate, wave->data, formatIn, wave->channels, wave->sampleRate, frameCountIn);
    if (frameCount == 0)
    {
        TraceLog(LOG_ERROR, "WaveFormat() : Format conversion failed.");
        return;
    }

    wave->sampleCount = frameCount;
    wave->sampleSize = sampleSize;
    wave->sampleRate = sampleRate;
    wave->channels = channels;
    RL_FREE(wave->data);
    wave->data = data;
}

// Copy a wave to a new wave
Wave WaveCopy(Wave wave)
{
    Wave newWave = { 0 };

    newWave.data = RL_MALLOC(wave.sampleCount*wave.sampleSize/8*wave.channels);

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

        void *data = RL_MALLOC(sampleCount*wave->sampleSize/8*wave->channels);

        memcpy(data, (unsigned char *)wave->data + (initSample*wave->channels*wave->sampleSize/8), sampleCount*wave->channels*wave->sampleSize/8);

        RL_FREE(wave->data);
        wave->data = data;
    }
    else TraceLog(LOG_WARNING, "Wave crop range out of bounds");
}

// Get samples data from wave as a floats array
// NOTE: Returned sample values are normalized to range [-1..1]
float *GetWaveData(Wave wave)
{
    float *samples = (float *)RL_MALLOC(wave.sampleCount*wave.channels*sizeof(float));

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
    Music music = { 0 };
    bool musicLoaded = false;

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (IsFileExtension(fileName, ".ogg"))
    {
        // Open ogg audio stream
        music.ctxData = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (music.ctxData != NULL)
        {
            music.ctxType = MUSIC_AUDIO_OGG;
            stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis *)music.ctxData);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music.stream = InitAudioStream(info.sample_rate, 16, info.channels);
            music.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples((stb_vorbis *)music.ctxData)*info.channels;
            music.sampleLeft = music.sampleCount;
            music.loopCount = 0;   // Infinite loop by default
            musicLoaded = true;

            TraceLog(LOG_INFO, "[%s] OGG total samples: %i", fileName, music.sampleCount);
            TraceLog(LOG_INFO, "[%s] OGG sample rate: %i", fileName, info.sample_rate);
            TraceLog(LOG_INFO, "[%s] OGG channels: %i", fileName, info.channels);
            TraceLog(LOG_INFO, "[%s] OGG memory required: %i", fileName, info.temp_memory_required);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (IsFileExtension(fileName, ".flac"))
    {
        music.ctxData = drflac_open_file(fileName);

        if (music.ctxData != NULL)
        {
            music.ctxType = MUSIC_AUDIO_FLAC;
            drflac *ctxFlac = (drflac *)music.ctxData;

            music.stream = InitAudioStream(ctxFlac->sampleRate, ctxFlac->bitsPerSample, ctxFlac->channels);
            music.sampleCount = (unsigned int)ctxFlac->totalSampleCount;
            music.sampleLeft = music.sampleCount;
            music.loopCount = 0;   // Infinite loop by default
            musicLoaded = true;

            TraceLog(LOG_DEBUG, "[%s] FLAC total samples: %i", fileName, music.sampleCount);
            TraceLog(LOG_DEBUG, "[%s] FLAC sample rate: %i", fileName, ctxFlac->sampleRate);
            TraceLog(LOG_DEBUG, "[%s] FLAC bits per sample: %i", fileName, ctxFlac->bitsPerSample);
            TraceLog(LOG_DEBUG, "[%s] FLAC channels: %i", fileName, ctxFlac->channels);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (IsFileExtension(fileName, ".mp3"))
    {
        drmp3 *ctxMp3 = RL_MALLOC(sizeof(drmp3));
        music.ctxData = ctxMp3;
        
        int result = drmp3_init_file(ctxMp3, fileName, NULL);

        if (result > 0)
        {
            music.ctxType = MUSIC_AUDIO_MP3;

            music.stream = InitAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
            music.sampleCount = drmp3_get_pcm_frame_count(ctxMp3)*ctxMp3->channels;
            music.sampleLeft = music.sampleCount;
            music.loopCount = 0;   // Infinite loop by default
            musicLoaded = true;

            TraceLog(LOG_INFO, "[%s] MP3 sample rate: %i", fileName, ctxMp3->sampleRate);
            TraceLog(LOG_INFO, "[%s] MP3 bits per sample: %i", fileName, 32);
            TraceLog(LOG_INFO, "[%s] MP3 channels: %i", fileName, ctxMp3->channels);
            TraceLog(LOG_INFO, "[%s] MP3 total samples: %i", fileName, music.sampleCount);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (IsFileExtension(fileName, ".xm"))
    {
        jar_xm_context_t *ctxXm = NULL;

        int result = jar_xm_create_context_from_file(&ctxXm, 48000, fileName);

        if (result == 0)    // XM context created successfully
        {
            music.ctxType = MUSIC_MODULE_XM;
            jar_xm_set_max_loop_count(ctxXm, 0);    // Set infinite number of loops

            // NOTE: Only stereo is supported for XM
            music.stream = InitAudioStream(48000, 16, 2);
            music.sampleCount = (unsigned int)jar_xm_get_remaining_samples(ctxXm);
            music.sampleLeft = music.sampleCount;
            music.loopCount = 0;   // Infinite loop by default
            musicLoaded = true;
            
            music.ctxData = ctxXm;

            TraceLog(LOG_INFO, "[%s] XM number of samples: %i", fileName, music.sampleCount);
            TraceLog(LOG_INFO, "[%s] XM track length: %11.6f sec", fileName, (float)music.sampleCount/48000.0f);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (IsFileExtension(fileName, ".mod"))
    {
        jar_mod_context_t *ctxMod = RL_MALLOC(sizeof(jar_mod_context_t));
        music.ctxData = ctxMod;
        
        jar_mod_init(ctxMod);
        int result = jar_mod_load_file(ctxMod, fileName);

        if (result > 0)
        {
            music.ctxType = MUSIC_MODULE_MOD;

            // NOTE: Only stereo is supported for MOD
            music.stream = InitAudioStream(48000, 16, 2);
            music.sampleCount = (unsigned int)jar_mod_max_samples(ctxMod);
            music.sampleLeft = music.sampleCount;
            music.loopCount = 0;   // Infinite loop by default
            musicLoaded = true;
            
            TraceLog(LOG_INFO, "[%s] MOD number of samples: %i", fileName, music.sampleCount);
            TraceLog(LOG_INFO, "[%s] MOD track length: %11.6f sec", fileName, (float)music.sampleCount/48000.0f);
        }
    }
#endif

    if (!musicLoaded)
    {
        if (false) { }
    #if defined(SUPPORT_FILEFORMAT_OGG)
        else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_FLAC)
        else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_MP3)
        else if (music.ctxType == MUSIC_AUDIO_MP3) { drmp3_uninit((drmp3 *)music.ctxData); RL_FREE(music.ctxData); }
    #endif
    #if defined(SUPPORT_FILEFORMAT_XM)
        else if (music.ctxType == MUSIC_MODULE_XM) jar_xm_free_context((jar_xm_context_t *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_MOD)
        else if (music.ctxType == MUSIC_MODULE_MOD) { jar_mod_unload((jar_mod_context_t *)music.ctxData); RL_FREE(music.ctxData); }
    #endif

        TraceLog(LOG_WARNING, "[%s] Music file could not be opened", fileName);
    }

    return music;
}

// Unload music stream
void UnloadMusicStream(Music music)
{
    CloseAudioStream(music.stream);

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (music.ctxType == MUSIC_AUDIO_MP3) { drmp3_uninit((drmp3 *)music.ctxData); RL_FREE(music.ctxData); }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (music.ctxType == MUSIC_MODULE_XM) jar_xm_free_context((jar_xm_context_t *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (music.ctxType == MUSIC_MODULE_MOD) { jar_mod_unload((jar_mod_context_t *)music.ctxData); RL_FREE(music.ctxData); }
#endif
}

// Start music playing (open stream)
void PlayMusicStream(Music music)
{
    AudioBuffer *audioBuffer = music.stream.buffer;

    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "PlayMusicStream() : No audio buffer");
        return;
    }

    // For music streams, we need to make sure we maintain the frame cursor position
    // This is a hack for this section of code in UpdateMusicStream()
    // NOTE: In case window is minimized, music stream is stopped, just make sure to 
    // play again on window restore: if (IsMusicPlaying(music)) PlayMusicStream(music);
    ma_uint32 frameCursorPos = audioBuffer->frameCursorPos;

    PlayAudioStream(music.stream); // <-- This resets the cursor position.

    audioBuffer->frameCursorPos = frameCursorPos;
}

// Pause music playing
void PauseMusicStream(Music music)
{
    PauseAudioStream(music.stream);
}

// Resume music playing
void ResumeMusicStream(Music music)
{
    ResumeAudioStream(music.stream);
}

// Stop music playing (close stream)
void StopMusicStream(Music* music)
{
    StopAudioStream(music->stream);

    // Restart music context
    switch (music->ctxType)
    {
#if defined(SUPPORT_FILEFORMAT_OGG)
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_start((stb_vorbis *)music->ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
        case MUSIC_AUDIO_FLAC: /* TODO: Restart FLAC context */ break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
        case MUSIC_AUDIO_MP3: drmp3_seek_to_pcm_frame((drmp3 *)music->ctxData, 0); break;
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
        case MUSIC_MODULE_XM: jar_xm_reset((jar_xm_context_t *)music->ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
        case MUSIC_MODULE_MOD: jar_mod_seek_start((jar_mod_context_t *)music->ctxData); break;
#endif
        default: break;
    }

    music->sampleLeft = music->sampleCount;
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(Music* music)
{
    bool streamEnding = false;

    unsigned int subBufferSizeInFrames = music->stream.buffer->bufferSizeInFrames/2;

    // NOTE: Using dynamic allocation because it could require more than 16KB
    void *pcm = RL_CALLOC(subBufferSizeInFrames*music->stream.channels*music->stream.sampleSize/8, 1);

    int samplesCount = 0;    // Total size of data steamed in L+R samples for xm floats, individual L or R for ogg shorts

    while (IsAudioStreamProcessed(music->stream))
    {
        if ((music->sampleLeft/music->stream.channels) >= subBufferSizeInFrames) samplesCount = subBufferSizeInFrames*music->stream.channels;
        else samplesCount = music->sampleLeft;

        switch (music->ctxType)
        {
        #if defined(SUPPORT_FILEFORMAT_OGG)
            case MUSIC_AUDIO_OGG:
            {
                // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
                stb_vorbis_get_samples_short_interleaved((stb_vorbis *)music->ctxData, music->stream.channels, (short *)pcm, samplesCount);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_FLAC)
            case MUSIC_AUDIO_FLAC:
            {
                // NOTE: Returns the number of samples to process (not required)
                drflac_read_s16((drflac *)music->ctxData, samplesCount, (short *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MP3)
            case MUSIC_AUDIO_MP3:
            {
                // NOTE: samplesCount, actually refers to framesCount and returns the number of frames processed
                drmp3_read_pcm_frames_f32((drmp3 *)music->ctxData, samplesCount/music->stream.channels, (float *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_XM)
            case MUSIC_MODULE_XM:
            {
                // NOTE: Internally this function considers 2 channels generation, so samplesCount/2
                jar_xm_generate_samples_16bit((jar_xm_context_t *)music->ctxData, (short *)pcm, samplesCount/2);
            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MOD)
            case MUSIC_MODULE_MOD:
            {
                // NOTE: 3rd parameter (nbsample) specify the number of stereo 16bits samples you want, so sampleCount/2
                jar_mod_fillbuffer((jar_mod_context_t *)music->ctxData, (short *)pcm, samplesCount/2, 0);
            } break;
        #endif
            default: break;
        }

        UpdateAudioStream(music->stream, pcm, samplesCount);
        
        if ((music->ctxType == MUSIC_MODULE_XM) || (music->ctxType == MUSIC_MODULE_MOD))
        {
            if (samplesCount > 1) music->sampleLeft -= samplesCount/2;
            else music->sampleLeft -= samplesCount;
        }
        else music->sampleLeft -= samplesCount;

        if (music->sampleLeft <= 0)
        {
            streamEnding = true;
            break;
        }
    }

    // Free allocated pcm data
    RL_FREE(pcm);

    // Reset audio stream for looping
    if (streamEnding)
    {
        StopMusicStream(music);        // Stop music (and reset)

        // Decrease loopCount to stop when required
        if (music->loopCount > 1)
        {
            music->loopCount--;        // Decrease loop count
            PlayMusicStream(*music);    // Play again
        }
    }
    else
    {
        // NOTE: In case window is minimized, music stream is stopped,
        // just make sure to play again on window restore
        if (IsMusicPlaying(*music)) PlayMusicStream(*music);
    }
}

// Check if any music is playing
bool IsMusicPlaying(Music music)
{
    return IsAudioStreamPlaying(music.stream);
}

// Set volume for music
void SetMusicVolume(Music music, float volume)
{
    SetAudioStreamVolume(music.stream, volume);
}

// Set pitch for music
void SetMusicPitch(Music music, float pitch)
{
    SetAudioStreamPitch(music.stream, pitch);
}

// Set music loop count (loop repeats)
// NOTE: If set to 0, means infinite loop
void SetMusicLoopCount(Music music, int count)
{
    music.loopCount = count;
}

// Get music time length (in seconds)
float GetMusicTimeLength(Music music)
{
    float totalSeconds = 0.0f;

    totalSeconds = (float)music.sampleCount/(music.stream.sampleRate*music.stream.channels);

    return totalSeconds;
}

// Get current music time played (in seconds)
float GetMusicTimePlayed(Music music)
{
    float secondsPlayed = 0.0f;

    unsigned int samplesPlayed = music.sampleCount - music.sampleLeft;
    secondsPlayed = (float)samplesPlayed/(music.stream.sampleRate*music.stream.channels);

    return secondsPlayed;
}

// Init audio stream (to stream audio pcm data)
AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
{
    AudioStream stream = { 0 };

    stream.sampleRate = sampleRate;
    stream.sampleSize = sampleSize;

    // Only mono and stereo channels are supported
    if ((channels > 0) && (channels < 3)) stream.channels = channels;
    else
    {
        TraceLog(LOG_WARNING, "Init audio stream: Number of channels not supported: %i", channels);
        stream.channels = 1;  // Fallback to mono channel
    }

    ma_format formatIn = ((stream.sampleSize == 8)? ma_format_u8 : ((stream.sampleSize == 16)? ma_format_s16 : ma_format_f32));

    // The size of a streaming buffer must be at least double the size of a period
    unsigned int periodSize = device.playback.internalBufferSizeInFrames/device.playback.internalPeriods;
    unsigned int subBufferSize = AUDIO_BUFFER_SIZE;
    
    if (subBufferSize < periodSize) subBufferSize = periodSize;

    AudioBuffer *audioBuffer = InitAudioBuffer(formatIn, stream.channels, stream.sampleRate, subBufferSize*2, AUDIO_BUFFER_USAGE_STREAM);
    
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "InitAudioStream() : Failed to create audio buffer");
        return stream;
    }

    audioBuffer->looping = true;        // Always loop for streaming buffers
    stream.buffer = audioBuffer;

    TraceLog(LOG_INFO, "Audio stream loaded successfully (%i Hz, %i bit, %s)", stream.sampleRate, stream.sampleSize, (stream.channels == 1)? "Mono" : "Stereo");

    return stream;
}

// Close audio stream and free memory
void CloseAudioStream(AudioStream stream)
{
    CloseAudioBuffer(stream.buffer);

    TraceLog(LOG_INFO, "Unloaded audio stream data");
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: unqueue -> update -> queue
// NOTE 2: To unqueue a buffer it needs to be processed: IsAudioStreamProcessed()
void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount)
{
    AudioBuffer *audioBuffer = stream.buffer;
    
    if (audioBuffer == NULL)
    {
        TraceLog(LOG_ERROR, "UpdateAudioStream() : No audio buffer");
        return;
    }

    if (audioBuffer->isSubBufferProcessed[0] || audioBuffer->isSubBufferProcessed[1])
    {
        ma_uint32 subBufferToUpdate = 0;

        if (audioBuffer->isSubBufferProcessed[0] && audioBuffer->isSubBufferProcessed[1])
        {
            // Both buffers are available for updating. 
            // Update the first one and make sure the cursor is moved back to the front.
            subBufferToUpdate = 0;
            audioBuffer->frameCursorPos = 0;
        }
        else
        {
            // Just update whichever sub-buffer is processed.
            subBufferToUpdate = (audioBuffer->isSubBufferProcessed[0])? 0 : 1;
        }

        ma_uint32 subBufferSizeInFrames = audioBuffer->bufferSizeInFrames/2;
        unsigned char *subBuffer = audioBuffer->buffer + ((subBufferSizeInFrames*stream.channels*(stream.sampleSize/8))*subBufferToUpdate);

        // Does this API expect a whole buffer to be updated in one go? 
        // Assuming so, but if not will need to change this logic.
        if (subBufferSizeInFrames >= (ma_uint32)samplesCount/stream.channels)
        {
            ma_uint32 framesToWrite = subBufferSizeInFrames;

            if (framesToWrite > ((ma_uint32)samplesCount/stream.channels)) framesToWrite = (ma_uint32)samplesCount/stream.channels;

            ma_uint32 bytesToWrite = framesToWrite*stream.channels*(stream.sampleSize/8);
            memcpy(subBuffer, data, bytesToWrite);

            // Any leftover frames should be filled with zeros.
            ma_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;

            if (leftoverFrameCount > 0)
            {
                memset(subBuffer + bytesToWrite, 0, leftoverFrameCount*stream.channels*(stream.sampleSize/8));
            }

            audioBuffer->isSubBufferProcessed[subBufferToUpdate] = false;
        }
        else TraceLog(LOG_ERROR, "UpdateAudioStream() : Attempting to write too many frames to buffer");
    }
    else TraceLog(LOG_ERROR, "Audio buffer not available for updating");
}

// Check if any audio stream buffers requires refill
bool IsAudioStreamProcessed(AudioStream stream)
{
    if (stream.buffer == NULL)
    {
        TraceLog(LOG_ERROR, "IsAudioStreamProcessed() : No audio buffer");
        return false;
    }

    return (stream.buffer->isSubBufferProcessed[0] || stream.buffer->isSubBufferProcessed[1]);
}

// Play audio stream
void PlayAudioStream(AudioStream stream)
{
    PlayAudioBuffer(stream.buffer);
}

// Play audio stream
void PauseAudioStream(AudioStream stream)
{
    PauseAudioBuffer(stream.buffer);
}

// Resume audio stream playing
void ResumeAudioStream(AudioStream stream)
{
    ResumeAudioBuffer(stream.buffer);
}

// Check if audio stream is playing.
bool IsAudioStreamPlaying(AudioStream stream)
{
    return IsAudioBufferPlaying(stream.buffer);
}

// Stop audio stream
void StopAudioStream(AudioStream stream)
{
    StopAudioBuffer(stream.buffer);
}

void SetAudioStreamVolume(AudioStream stream, float volume)
{
    SetAudioBufferVolume(stream.buffer, volume);
}

void SetAudioStreamPitch(AudioStream stream, float pitch)
{
    SetAudioBufferPitch(stream.buffer, pitch);
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
                    wave.data = RL_MALLOC(wavData.subChunkSize);

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

                    TraceLog(LOG_INFO, "[%s] WAV file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");
                }
            }
        }

        fclose(wavFile);
    }

    return wave;
}

// Save wave data as WAV file
static int SaveWAV(Wave wave, const char *fileName)
{
    int success = 0;
    int dataSize = wave.sampleCount*wave.channels*wave.sampleSize/8;

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

    FILE *wavFile = fopen(fileName, "wb");

    if (wavFile == NULL) TraceLog(LOG_WARNING, "[%s] WAV audio file could not be created", fileName);
    else
    {
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
        waveData.subChunkSize = dataSize;

        success = fwrite(&riffHeader, sizeof(RiffHeader), 1, wavFile);
        success = fwrite(&waveFormat, sizeof(WaveFormat), 1, wavFile);
        success = fwrite(&waveData, sizeof(WaveData), 1, wavFile);

        success = fwrite(wave.data, dataSize, 1, wavFile);

        fclose(wavFile);
    }

    // If all data has been written correctly to file, success = 1
    return success;
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
        wave.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples(oggFile)*info.channels;  // Independent by channel

        float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);
        if (totalSeconds > 10) TraceLog(LOG_WARNING, "[%s] Ogg audio length is larger than 10 seconds (%f), that's a big file in memory, consider music streaming", fileName, totalSeconds);

        wave.data = (short *)RL_MALLOC(wave.sampleCount*wave.channels*sizeof(short));

        // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
        int numSamplesOgg = stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, (short *)wave.data, wave.sampleCount*wave.channels);

        TraceLog(LOG_DEBUG, "[%s] Samples obtained: %i", fileName, numSamplesOgg);

        TraceLog(LOG_INFO, "[%s] OGG file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");

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

    wave.sampleCount = (unsigned int)totalSampleCount;
    wave.sampleSize = 16;

    // NOTE: Only support up to 2 channels (mono, stereo)
    if (wave.channels > 2) TraceLog(LOG_WARNING, "[%s] FLAC channels number (%i) not supported", fileName, wave.channels);

    if (wave.data == NULL) TraceLog(LOG_WARNING, "[%s] FLAC data could not be loaded", fileName);
    else TraceLog(LOG_INFO, "[%s] FLAC file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");

    return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
// Load MP3 file into Wave structure
// NOTE: Using dr_mp3 library
static Wave LoadMP3(const char *fileName)
{
    Wave wave = { 0 };

    // Decode an entire MP3 file in one go
    uint64_t totalFrameCount = 0;
    drmp3_config config = { 0 };
    wave.data = drmp3_open_file_and_read_f32(fileName, &config, &totalFrameCount);

    wave.channels = config.outputChannels;
    wave.sampleRate = config.outputSampleRate;
    wave.sampleCount = (int)totalFrameCount*wave.channels;
    wave.sampleSize = 32;

    // NOTE: Only support up to 2 channels (mono, stereo)
    if (wave.channels > 2) TraceLog(LOG_WARNING, "[%s] MP3 channels number (%i) not supported", fileName, wave.channels);

    if (wave.data == NULL) TraceLog(LOG_WARNING, "[%s] MP3 data could not be loaded", fileName);
    else TraceLog(LOG_INFO, "[%s] MP3 file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");

    return wave;
}
#endif

// Some required functions for audio standalone module version
#if defined(RAUDIO_STANDALONE)
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

#undef AudioBuffer
