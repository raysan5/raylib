/**********************************************************************************************
*
*   raudio v1.0 - A simple and easy-to-use audio library based on miniaudio
*
*   FEATURES:
*       - Manage audio device (init/close)
*       - Manage raw audio context
*       - Manage mixing channels
*       - Load and unload audio files
*       - Format wave data (sample rate, size, channels)
*       - Play/Stop/Pause/Resume loaded audio
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
*   Copyright (c) 2013-2021 Ramon Santamaria (@raysan5)
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

#if defined(_WIN32)
// To avoid conflicting windows.h symbols with raylib, some flags are defined
// WARNING: Those flags avoid inclusion of some Win32 headers that could be required
// by user at some point and won't be included...
//-------------------------------------------------------------------------------------

// If defined, the following flags inhibit definition of the indicated items.
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
//#define NONLS             // All NLS defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions

// Type required before windows.h inclusion
typedef struct tagMSG *LPMSG;

#include <windows.h>

// Type required by some unused function...
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#include <objbase.h>
#include <mmreg.h>
#include <mmsystem.h>

// Some required types defined for MSVC/TinyC compiler
#if defined(_MSC_VER) || defined(__TINYC__)
    #include "propidl.h"
#endif
#endif

#define MA_MALLOC RL_MALLOC
#define MA_FREE RL_FREE

#define MA_NO_JACK
#define MA_NO_WAV
#define MA_NO_FLAC
#define MA_NO_MP3
#define MINIAUDIO_IMPLEMENTATION
//#define MA_DEBUG_OUTPUT
#include "external/miniaudio.h"         // miniaudio library
#undef PlaySound                        // Win32 API: windows.h > mmsystem.h defines PlaySound macro

#include <stdlib.h>                     // Required for: malloc(), free()
#include <stdio.h>                      // Required for: FILE, fopen(), fclose(), fread()

#if defined(RAUDIO_STANDALONE)
    #include <string.h>                 // Required for: strcmp() [Used in IsFileExtension()]

    #if !defined(TRACELOG)
        #define TRACELOG(level, ...) (void)0
    #endif

    // Allow custom memory allocators
    #ifndef RL_MALLOC
        #define RL_MALLOC(sz)       malloc(sz)
    #endif
    #ifndef RL_CALLOC
        #define RL_CALLOC(n,sz)     calloc(n,sz)
    #endif
    #ifndef RL_REALLOC
        #define RL_REALLOC(ptr,sz)  realloc(ptr,sz)
    #endif
    #ifndef RL_FREE
        #define RL_FREE(ptr)        free(ptr)
    #endif
#endif

#if defined(SUPPORT_FILEFORMAT_OGG)
    // TODO: Remap malloc()/free() calls to RL_MALLOC/RL_FREE

    #define STB_VORBIS_IMPLEMENTATION
    #include "external/stb_vorbis.h"    // OGG loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_XM)
    #define JARXM_MALLOC RL_MALLOC
    #define JARXM_FREE RL_FREE

    #define JAR_XM_IMPLEMENTATION
    #include "external/jar_xm.h"        // XM loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_MOD)
    #define JARMOD_MALLOC RL_MALLOC
    #define JARMOD_FREE RL_FREE

    #define JAR_MOD_IMPLEMENTATION
    #include "external/jar_mod.h"       // MOD loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_WAV)
    #define DRWAV_MALLOC RL_MALLOC
    #define DRWAV_REALLOC RL_REALLOC
    #define DRWAV_FREE RL_FREE

    #define DR_WAV_IMPLEMENTATION
    #include "external/dr_wav.h"        // WAV loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
    #define DRMP3_MALLOC RL_MALLOC
    #define DRMP3_REALLOC RL_REALLOC
    #define DRMP3_FREE RL_FREE

    #define DR_MP3_IMPLEMENTATION
    #include "external/dr_mp3.h"        // MP3 loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)
    #define DRFLAC_MALLOC RL_MALLOC
    #define DRFLAC_REALLOC RL_REALLOC
    #define DRFLAC_FREE RL_FREE

    #define DR_FLAC_IMPLEMENTATION
    #define DR_FLAC_NO_WIN32_IO
    #include "external/dr_flac.h"       // FLAC loading functions
#endif

#if defined(_MSC_VER)
    #undef bool
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef AUDIO_DEVICE_FORMAT
    #define AUDIO_DEVICE_FORMAT    ma_format_f32    // Device output format (float-32bit)
#endif
#ifndef AUDIO_DEVICE_CHANNELS
    #define AUDIO_DEVICE_CHANNELS              2    // Device output channels: stereo
#endif

#ifndef AUDIO_DEVICE_SAMPLE_RATE
    #define AUDIO_DEVICE_SAMPLE_RATE              0    // Device output channels: stereo
#endif
#ifndef MAX_AUDIO_BUFFER_POOL_CHANNELS
    #define MAX_AUDIO_BUFFER_POOL_CHANNELS    16    // Audio pool channels
#endif
#ifndef DEFAULT_AUDIO_BUFFER_SIZE
    #define DEFAULT_AUDIO_BUFFER_SIZE       4096    // Default audio buffer size
#endif


//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Music context type
// NOTE: Depends on data structure provided by the library
// in charge of reading the different file types
typedef enum {
    MUSIC_AUDIO_NONE = 0,
    MUSIC_AUDIO_WAV,
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
} TraceLogLevel;
#endif

// NOTE: Different logic is used when feeding data to the playback device
// depending on whether or not data is streamed (Music vs Sound)
typedef enum {
    AUDIO_BUFFER_USAGE_STATIC = 0,
    AUDIO_BUFFER_USAGE_STREAM
} AudioBufferUsage;

// Audio buffer structure
struct rAudioBuffer {
    ma_data_converter converter;    // Audio data converter

    float volume;                   // Audio buffer volume
    float pitch;                    // Audio buffer pitch

    bool playing;                   // Audio buffer state: AUDIO_PLAYING
    bool paused;                    // Audio buffer state: AUDIO_PAUSED
    bool looping;                   // Audio buffer looping, always true for AudioStreams
    int usage;                      // Audio buffer usage mode: STATIC or STREAM

    bool isSubBufferProcessed[2];   // SubBuffer processed (virtual double buffer)
    unsigned int sizeInFrames;      // Total buffer size in frames
    unsigned int frameCursorPos;    // Frame cursor position
    unsigned int totalFramesProcessed;  // Total frames processed in this buffer (required for play timing)

    unsigned char *data;            // Data buffer, on music stream keeps filling

    rAudioBuffer *next;             // Next audio buffer on the list
    rAudioBuffer *prev;             // Previous audio buffer on the list
};

#define AudioBuffer rAudioBuffer    // HACK: To avoid CoreAudio (macOS) symbol collision

// Audio data context
typedef struct AudioData {
    struct {
        ma_context context;         // miniaudio context data
        ma_device device;           // miniaudio device
        ma_mutex lock;              // miniaudio mutex lock
        bool isReady;               // Check if audio device is ready
    } System;
    struct {
        AudioBuffer *first;         // Pointer to first AudioBuffer in the list
        AudioBuffer *last;          // Pointer to last AudioBuffer in the list
        int defaultSize;            // Default audio buffer size for audio streams
    } Buffer;
    struct {
        unsigned int poolCounter;                               // AudioBuffer pointers pool counter
        AudioBuffer *pool[MAX_AUDIO_BUFFER_POOL_CHANNELS];      // Multichannel AudioBuffer pointers pool
        unsigned int channels[MAX_AUDIO_BUFFER_POOL_CHANNELS];  // AudioBuffer pool channels
    } MultiChannel;
} AudioData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static AudioData AUDIO = {          // Global AUDIO context

    // NOTE: Music buffer size is defined by number of samples, independent of sample size and channels number
    // After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds and a
    // standard double-buffering system, a 4096 samples buffer has been chosen, it should be enough
    // In case of music-stalls, just increase this number
    .Buffer.defaultSize = 0
};

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void OnLog(ma_context *pContext, ma_device *pDevice, ma_uint32 logLevel, const char *message);
static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount);
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume);

#if defined(SUPPORT_FILEFORMAT_WAV)
static Wave LoadWAV(const unsigned char *fileData, unsigned int fileSize);   // Load WAV file
static int SaveWAV(Wave wave, const char *fileName);    // Save wave data as WAV file
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
static Wave LoadOGG(const unsigned char *fileData, unsigned int fileSize);   // Load OGG file
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
static Wave LoadFLAC(const unsigned char *fileData, unsigned int fileSize);  // Load FLAC file
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
static Wave LoadMP3(const unsigned char *fileData, unsigned int fileSize);   // Load MP3 file
#endif

#if defined(RAUDIO_STANDALONE)
static bool IsFileExtension(const char *fileName, const char *ext); // Check file extension
static unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead);     // Load file data as byte array (read)
static bool SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite); // Save data to file from byte array (write)
static bool SaveFileText(const char *fileName, char *text);         // Save text data to file (write), string must be '\0' terminated
#endif

//----------------------------------------------------------------------------------
// AudioBuffer management functions declaration
// NOTE: Those functions are not exposed by raylib... for the moment
//----------------------------------------------------------------------------------
AudioBuffer *LoadAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, int usage);
void UnloadAudioBuffer(AudioBuffer *buffer);

bool IsAudioBufferPlaying(AudioBuffer *buffer);
void PlayAudioBuffer(AudioBuffer *buffer);
void StopAudioBuffer(AudioBuffer *buffer);
void PauseAudioBuffer(AudioBuffer *buffer);
void ResumeAudioBuffer(AudioBuffer *buffer);
void SetAudioBufferVolume(AudioBuffer *buffer, float volume);
void SetAudioBufferPitch(AudioBuffer *buffer, float pitch);
void TrackAudioBuffer(AudioBuffer *buffer);
void UntrackAudioBuffer(AudioBuffer *buffer);
int GetAudioStreamBufferSizeDefault();

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------
// Initialize audio device
void InitAudioDevice(void)
{
    // TODO: Load AUDIO context memory dynamically?

    // Init audio context
    ma_context_config ctxConfig = ma_context_config_init();
    ctxConfig.logCallback = OnLog;

    ma_result result = ma_context_init(NULL, 0, &ctxConfig, &AUDIO.System.context);
    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to initialize context");
        return;
    }

    // Init audio device
    // NOTE: Using the default device. Format is floating point because it simplifies mixing.
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;  // NULL for the default playback AUDIO.System.device.
    config.playback.format = AUDIO_DEVICE_FORMAT;
    config.playback.channels = AUDIO_DEVICE_CHANNELS;
    config.capture.pDeviceID = NULL;  // NULL for the default capture AUDIO.System.device.
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = AUDIO_DEVICE_SAMPLE_RATE;
    config.dataCallback = OnSendAudioDataToDevice;
    config.pUserData = NULL;

    result = ma_device_init(&AUDIO.System.context, &config, &AUDIO.System.device);
    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to initialize playback device");
        ma_context_uninit(&AUDIO.System.context);
        return;
    }

    // Keep the device running the whole time. May want to consider doing something a bit smarter and only have the device running
    // while there's at least one sound being played.
    result = ma_device_start(&AUDIO.System.device);
    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to start playback device");
        ma_device_uninit(&AUDIO.System.device);
        ma_context_uninit(&AUDIO.System.context);
        return;
    }

    // Mixing happens on a seperate thread which means we need to synchronize. I'm using a mutex here to make things simple, but may
    // want to look at something a bit smarter later on to keep everything real-time, if that's necessary.
    if (ma_mutex_init(&AUDIO.System.lock) != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to create mutex for mixing");
        ma_device_uninit(&AUDIO.System.device);
        ma_context_uninit(&AUDIO.System.context);
        return;
    }

    // Init dummy audio buffers pool for multichannel sound playing
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
    {
        // WARNING: An empty audioBuffer is created (data = 0)
        // AudioBuffer data just points to loaded sound data
        AUDIO.MultiChannel.pool[i] = LoadAudioBuffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO.System.device.sampleRate, 0, AUDIO_BUFFER_USAGE_STATIC);
    }

    TRACELOG(LOG_INFO, "AUDIO: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Backend:       miniaudio / %s", ma_get_backend_name(AUDIO.System.context.backend));
    TRACELOG(LOG_INFO, "    > Format:        %s -> %s", ma_get_format_name(AUDIO.System.device.playback.format), ma_get_format_name(AUDIO.System.device.playback.internalFormat));
    TRACELOG(LOG_INFO, "    > Channels:      %d -> %d", AUDIO.System.device.playback.channels, AUDIO.System.device.playback.internalChannels);
    TRACELOG(LOG_INFO, "    > Sample rate:   %d -> %d", AUDIO.System.device.sampleRate, AUDIO.System.device.playback.internalSampleRate);
    TRACELOG(LOG_INFO, "    > Periods size:  %d", AUDIO.System.device.playback.internalPeriodSizeInFrames*AUDIO.System.device.playback.internalPeriods);

    AUDIO.System.isReady = true;
}

// Close the audio device for all contexts
void CloseAudioDevice(void)
{
    if (AUDIO.System.isReady)
    {
        // Unload dummy audio buffers pool
        // WARNING: They can be pointing to already unloaded data
        for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
        {
            //UnloadAudioBuffer(AUDIO.MultiChannel.pool[i]);
            if (AUDIO.MultiChannel.pool[i] != NULL)
            {
                ma_data_converter_uninit(&AUDIO.MultiChannel.pool[i]->converter);
                UntrackAudioBuffer(AUDIO.MultiChannel.pool[i]);
                //RL_FREE(buffer->data);    // Already unloaded by UnloadSound()
                RL_FREE(AUDIO.MultiChannel.pool[i]);
            }
        }

        ma_mutex_uninit(&AUDIO.System.lock);
        ma_device_uninit(&AUDIO.System.device);
        ma_context_uninit(&AUDIO.System.context);

        AUDIO.System.isReady = false;

        TRACELOG(LOG_INFO, "AUDIO: Device closed successfully");
    }
    else TRACELOG(LOG_WARNING, "AUDIO: Device could not be closed, not currently initialized");
}

// Check if device has been initialized successfully
bool IsAudioDeviceReady(void)
{
    return AUDIO.System.isReady;
}

// Set master volume (listener)
void SetMasterVolume(float volume)
{
    ma_device_set_master_volume(&AUDIO.System.device, volume);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Buffer management
//----------------------------------------------------------------------------------

// Initialize a new audio buffer (filled with silence)
AudioBuffer *LoadAudioBuffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, int usage)
{
    AudioBuffer *audioBuffer = (AudioBuffer *)RL_CALLOC(1, sizeof(AudioBuffer));

    if (audioBuffer == NULL)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to allocate memory for buffer");
        return NULL;
    }

    if (sizeInFrames > 0) audioBuffer->data = RL_CALLOC(sizeInFrames*channels*ma_get_bytes_per_sample(format), 1);

    // Audio data runs through a format converter
    ma_data_converter_config converterConfig = ma_data_converter_config_init(format, AUDIO_DEVICE_FORMAT, channels, AUDIO_DEVICE_CHANNELS, sampleRate, AUDIO.System.device.sampleRate);
    converterConfig.resampling.allowDynamicSampleRate = true;        // Required for pitch shifting

    ma_result result = ma_data_converter_init(&converterConfig, &audioBuffer->converter);

    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to create data conversion pipeline");
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
    audioBuffer->sizeInFrames = sizeInFrames;

    // Buffers should be marked as processed by default so that a call to
    // UpdateAudioStream() immediately after initialization works correctly
    audioBuffer->isSubBufferProcessed[0] = true;
    audioBuffer->isSubBufferProcessed[1] = true;

    // Track audio buffer to linked list next position
    TrackAudioBuffer(audioBuffer);

    return audioBuffer;
}

// Delete an audio buffer
void UnloadAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        ma_data_converter_uninit(&buffer->converter);
        UntrackAudioBuffer(buffer);
        RL_FREE(buffer->data);
        RL_FREE(buffer);
    }
}

// Check if an audio buffer is playing
bool IsAudioBufferPlaying(AudioBuffer *buffer)
{
    bool result = false;

    if (buffer != NULL) result = (buffer->playing && !buffer->paused);

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
            buffer->totalFramesProcessed = 0;
            buffer->isSubBufferProcessed[0] = true;
            buffer->isSubBufferProcessed[1] = true;
        }
    }
}

// Pause an audio buffer
void PauseAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL) buffer->paused = true;
}

// Resume an audio buffer
void ResumeAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL) buffer->paused = false;
}

// Set volume for an audio buffer
void SetAudioBufferVolume(AudioBuffer *buffer, float volume)
{
    if (buffer != NULL) buffer->volume = volume;
}

// Set pitch for an audio buffer
void SetAudioBufferPitch(AudioBuffer *buffer, float pitch)
{
    if ((buffer != NULL) && (pitch > 0.0f))
    {
        // Pitching is just an adjustment of the sample rate.
        // Note that this changes the duration of the sound:
        //  - higher pitches will make the sound faster
        //  - lower pitches make it slower
        ma_uint32 outputSampleRate = (ma_uint32)((float)buffer->converter.config.sampleRateOut/pitch);
        ma_data_converter_set_rate(&buffer->converter, buffer->converter.config.sampleRateIn, outputSampleRate);

        buffer->pitch = pitch;
    }
}

// Track audio buffer to linked list next position
void TrackAudioBuffer(AudioBuffer *buffer)
{
    ma_mutex_lock(&AUDIO.System.lock);
    {
        if (AUDIO.Buffer.first == NULL) AUDIO.Buffer.first = buffer;
        else
        {
            AUDIO.Buffer.last->next = buffer;
            buffer->prev = AUDIO.Buffer.last;
        }

        AUDIO.Buffer.last = buffer;
    }
    ma_mutex_unlock(&AUDIO.System.lock);
}

// Untrack audio buffer from linked list
void UntrackAudioBuffer(AudioBuffer *buffer)
{
    ma_mutex_lock(&AUDIO.System.lock);
    {
        if (buffer->prev == NULL) AUDIO.Buffer.first = buffer->next;
        else buffer->prev->next = buffer->next;

        if (buffer->next == NULL) AUDIO.Buffer.last = buffer->prev;
        else buffer->next->prev = buffer->prev;

        buffer->prev = NULL;
        buffer->next = NULL;
    }
    ma_mutex_unlock(&AUDIO.System.lock);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Sounds loading and playing (.WAV)
//----------------------------------------------------------------------------------

// Load wave data from file
Wave LoadWave(const char *fileName)
{
    Wave wave = { 0 };

    // Loading file to memory
    unsigned int fileSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &fileSize);

    if (fileData != NULL)
    {
        // Loading wave from memory data
        wave = LoadWaveFromMemory(GetFileExtension(fileName), fileData, fileSize);

        RL_FREE(fileData);
    }

    return wave;
}

// Load wave from memory buffer, fileType refers to extension: i.e. ".wav"
Wave LoadWaveFromMemory(const char *fileType, const unsigned char *fileData, int dataSize)
{
    Wave wave = { 0 };

    char fileExtLower[16] = { 0 };
    strcpy(fileExtLower, TextToLower(fileType));

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if (TextIsEqual(fileExtLower, ".wav")) wave = LoadWAV(fileData, dataSize);
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (TextIsEqual(fileExtLower, ".ogg")) wave = LoadOGG(fileData, dataSize);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (TextIsEqual(fileExtLower, ".flac")) wave = LoadFLAC(fileData, dataSize);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (TextIsEqual(fileExtLower, ".mp3")) wave = LoadMP3(fileData, dataSize);
#endif
    else TRACELOG(LOG_WARNING, "WAVE: File format not supported");

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
        // the format used to open the playback AUDIO.System.device. We can do this two ways:
        //
        //   1) Convert the whole sound in one go at load time (here).
        //   2) Convert the audio data in chunks at mixing time.
        //
        // First option has been selected, format conversion is done on the loading stage.
        // The downside is that it uses more memory if the original sound is u8 or s16.
        ma_format formatIn  = ((wave.sampleSize == 8)? ma_format_u8 : ((wave.sampleSize == 16)? ma_format_s16 : ma_format_f32));
        ma_uint32 frameCountIn = wave.sampleCount/wave.channels;

        ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, 0, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO.System.device.sampleRate, NULL, frameCountIn, formatIn, wave.channels, wave.sampleRate);
        if (frameCount == 0) TRACELOG(LOG_WARNING, "SOUND: Failed to get frame count for format conversion");

        AudioBuffer *audioBuffer = LoadAudioBuffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO.System.device.sampleRate, frameCount, AUDIO_BUFFER_USAGE_STATIC);
        if (audioBuffer == NULL)
        {
            TRACELOG(LOG_WARNING, "SOUND: Failed to create buffer");
            return sound; // early return to avoid dereferencing the audioBuffer null pointer
        }

        frameCount = (ma_uint32)ma_convert_frames(audioBuffer->data, frameCount, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO.System.device.sampleRate, wave.data, frameCountIn, formatIn, wave.channels, wave.sampleRate);
        if (frameCount == 0) TRACELOG(LOG_WARNING, "SOUND: Failed format conversion");

        sound.sampleCount = frameCount*AUDIO_DEVICE_CHANNELS;
        sound.stream.sampleRate = AUDIO.System.device.sampleRate;
        sound.stream.sampleSize = 32;
        sound.stream.channels = AUDIO_DEVICE_CHANNELS;
        sound.stream.buffer = audioBuffer;
    }

    return sound;
}

// Unload wave data
void UnloadWave(Wave wave)
{
    if (wave.data != NULL) RL_FREE(wave.data);

    TRACELOG(LOG_INFO, "WAVE: Unloaded wave data from RAM");
}

// Unload sound
void UnloadSound(Sound sound)
{
    UnloadAudioBuffer(sound.stream.buffer);

    TRACELOG(LOG_INFO, "WAVE: Unloaded sound data from RAM");
}

// Update sound buffer with new data
void UpdateSound(Sound sound, const void *data, int samplesCount)
{
    if (sound.stream.buffer != NULL)
    {
        StopAudioBuffer(sound.stream.buffer);

        // TODO: May want to lock/unlock this since this data buffer is read at mixing time
        memcpy(sound.stream.buffer->data, data, samplesCount*ma_get_bytes_per_frame(sound.stream.buffer->converter.config.formatIn, sound.stream.buffer->converter.config.channelsIn));
    }
}

// Export wave data to file
bool ExportWave(Wave wave, const char *fileName)
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
        success = SaveFileData(fileName, wave.data, wave.sampleCount*wave.sampleSize/8);
    }

    if (success) TRACELOG(LOG_INFO, "FILEIO: [%s] Wave data exported successfully", fileName);
    else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export wave data", fileName);

    return success;
}

// Export wave sample data to code (.h)
bool ExportWaveAsCode(Wave wave, const char *fileName)
{
    bool success = false;

#ifndef TEXT_BYTES_PER_LINE
    #define TEXT_BYTES_PER_LINE     20
#endif

    int waveDataSize = wave.sampleCount*wave.channels*wave.sampleSize/8;

    // NOTE: Text data buffer size is estimated considering wave data size in bytes
    // and requiring 6 char bytes for every byte: "0x00, "
    char *txtData = (char *)RL_CALLOC(6*waveDataSize + 2000, sizeof(char));

    int bytesCount = 0;
    bytesCount += sprintf(txtData + bytesCount, "\n//////////////////////////////////////////////////////////////////////////////////\n");
    bytesCount += sprintf(txtData + bytesCount, "//                                                                              //\n");
    bytesCount += sprintf(txtData + bytesCount, "// WaveAsCode exporter v1.0 - Wave data exported as an array of bytes           //\n");
    bytesCount += sprintf(txtData + bytesCount, "//                                                                              //\n");
    bytesCount += sprintf(txtData + bytesCount, "// more info and bugs-report:  github.com/raysan5/raylib                        //\n");
    bytesCount += sprintf(txtData + bytesCount, "// feedback and support:       ray[at]raylib.com                                //\n");
    bytesCount += sprintf(txtData + bytesCount, "//                                                                              //\n");
    bytesCount += sprintf(txtData + bytesCount, "// Copyright (c) 2018 Ramon Santamaria (@raysan5)                               //\n");
    bytesCount += sprintf(txtData + bytesCount, "//                                                                              //\n");
    bytesCount += sprintf(txtData + bytesCount, "//////////////////////////////////////////////////////////////////////////////////\n\n");

    char varFileName[256] = { 0 };
#if !defined(RAUDIO_STANDALONE)
    // Get file name from path and convert variable name to uppercase
    strcpy(varFileName, GetFileNameWithoutExt(fileName));
    for (int i = 0; varFileName[i] != '\0'; i++) if (varFileName[i] >= 'a' && varFileName[i] <= 'z') { varFileName[i] = varFileName[i] - 32; }
#else
    strcpy(varFileName, fileName);
#endif

    bytesCount += sprintf(txtData + bytesCount, "// Wave data information\n");
    bytesCount += sprintf(txtData + bytesCount, "#define %s_SAMPLE_COUNT     %u\n", varFileName, wave.sampleCount);
    bytesCount += sprintf(txtData + bytesCount, "#define %s_SAMPLE_RATE      %u\n", varFileName, wave.sampleRate);
    bytesCount += sprintf(txtData + bytesCount, "#define %s_SAMPLE_SIZE      %u\n", varFileName, wave.sampleSize);
    bytesCount += sprintf(txtData + bytesCount, "#define %s_CHANNELS         %u\n\n", varFileName, wave.channels);

    // Write byte data as hexadecimal text
    bytesCount += sprintf(txtData + bytesCount, "static unsigned char %s_DATA[%i] = { ", varFileName, waveDataSize);
    for (int i = 0; i < waveDataSize - 1; i++) bytesCount += sprintf(txtData + bytesCount, ((i%TEXT_BYTES_PER_LINE == 0)? "0x%x,\n" : "0x%x, "), ((unsigned char *)wave.data)[i]);
    bytesCount += sprintf(txtData + bytesCount, "0x%x };\n", ((unsigned char *)wave.data)[waveDataSize - 1]);

    // NOTE: Text data length exported is determined by '\0' (NULL) character
    success = SaveFileText(fileName, txtData);

    RL_FREE(txtData);

    return success;
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
    unsigned int oldAge = 0;
    int oldIndex = -1;

    // find the first non playing pool entry
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
    {
        if (AUDIO.MultiChannel.channels[i] > oldAge)
        {
            oldAge = AUDIO.MultiChannel.channels[i];
            oldIndex = i;
        }

        if (!IsAudioBufferPlaying(AUDIO.MultiChannel.pool[i]))
        {
            index = i;
            break;
        }
    }

    // If no none playing pool members can be index choose the oldest
    if (index == -1)
    {
        TRACELOG(LOG_WARNING, "SOUND: Buffer pool is already full, count: %i", AUDIO.MultiChannel.poolCounter);

        if (oldIndex == -1)
        {
            // Shouldn't be able to get here... but just in case something odd happens!
            TRACELOG(LOG_WARNING, "SOUND: Buffer pool could not determine oldest buffer not playing sound");
            return;
        }

        index = oldIndex;

        // Just in case...
        StopAudioBuffer(AUDIO.MultiChannel.pool[index]);
    }

    // Experimentally mutex lock doesn't seem to be needed this makes sense
    // as pool[index] isn't playing and the only stuff we're copying
    // shouldn't be changing...

    AUDIO.MultiChannel.channels[index] = AUDIO.MultiChannel.poolCounter;
    AUDIO.MultiChannel.poolCounter++;

    AUDIO.MultiChannel.pool[index]->volume = sound.stream.buffer->volume;
    AUDIO.MultiChannel.pool[index]->pitch = sound.stream.buffer->pitch;
    AUDIO.MultiChannel.pool[index]->looping = sound.stream.buffer->looping;
    AUDIO.MultiChannel.pool[index]->usage = sound.stream.buffer->usage;
    AUDIO.MultiChannel.pool[index]->isSubBufferProcessed[0] = false;
    AUDIO.MultiChannel.pool[index]->isSubBufferProcessed[1] = false;
    AUDIO.MultiChannel.pool[index]->sizeInFrames = sound.stream.buffer->sizeInFrames;
    AUDIO.MultiChannel.pool[index]->data = sound.stream.buffer->data;

    PlayAudioBuffer(AUDIO.MultiChannel.pool[index]);
}

// Stop any sound played with PlaySoundMulti()
void StopSoundMulti(void)
{
    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++) StopAudioBuffer(AUDIO.MultiChannel.pool[i]);
}

// Get number of sounds playing in the multichannel buffer pool
int GetSoundsPlaying(void)
{
    int counter = 0;

    for (int i = 0; i < MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
    {
        if (IsAudioBufferPlaying(AUDIO.MultiChannel.pool[i])) counter++;
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
    ma_format formatIn = ((wave->sampleSize == 8)? ma_format_u8 : ((wave->sampleSize == 16)? ma_format_s16 : ma_format_f32));
    ma_format formatOut = ((sampleSize == 8)? ma_format_u8 : ((sampleSize == 16)? ma_format_s16 : ma_format_f32));

    ma_uint32 frameCountIn = wave->sampleCount/wave->channels;

    ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, 0, formatOut, channels, sampleRate, NULL, frameCountIn, formatIn, wave->channels, wave->sampleRate);
    if (frameCount == 0)
    {
        TRACELOG(LOG_WARNING, "WAVE: Failed to get frame count for format conversion");
        return;
    }

    void *data = RL_MALLOC(frameCount*channels*(sampleSize/8));

    frameCount = (ma_uint32)ma_convert_frames(data, frameCount, formatOut, channels, sampleRate, wave->data, frameCountIn, formatIn, wave->channels, wave->sampleRate);
    if (frameCount == 0)
    {
        TRACELOG(LOG_WARNING, "WAVE: Failed format conversion");
        return;
    }

    wave->sampleCount = frameCount*channels;
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

    newWave.data = RL_MALLOC(wave.sampleCount*wave.sampleSize/8);

    if (newWave.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newWave.data, wave.data, wave.sampleCount*wave.sampleSize/8);

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

        void *data = RL_MALLOC(sampleCount*wave->sampleSize/8);

        memcpy(data, (unsigned char *)wave->data + (initSample*wave->channels*wave->sampleSize/8), sampleCount*wave->sampleSize/8);

        RL_FREE(wave->data);
        wave->data = data;
    }
    else TRACELOG(LOG_WARNING, "WAVE: Crop range out of bounds");
}

// Load samples data from wave as a floats array
// NOTE 1: Returned sample values are normalized to range [-1..1]
// NOTE 2: Sample data allocated should be freed with UnloadWaveSamples()
float *LoadWaveSamples(Wave wave)
{
    float *samples = (float *)RL_MALLOC(wave.sampleCount*sizeof(float));

    // NOTE: sampleCount is the total number of interlaced samples (including channels)

    for (unsigned int i = 0; i < wave.sampleCount; i++)
    {
        if (wave.sampleSize == 8) samples[i] = (float)(((unsigned char *)wave.data)[i] - 127)/256.0f;
        else if (wave.sampleSize == 16) samples[i] = (float)(((short *)wave.data)[i])/32767.0f;
        else if (wave.sampleSize == 32) samples[i] = ((float *)wave.data)[i];
    }

    return samples;
}

// Unload samples data loaded with LoadWaveSamples()
void UnloadWaveSamples(float *samples)
{
    RL_FREE(samples);
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
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if (IsFileExtension(fileName, ".wav"))
    {
        drwav *ctxWav = RL_CALLOC(1, sizeof(drwav));
        bool success = drwav_init_file(ctxWav, fileName, NULL);

        music.ctxType = MUSIC_AUDIO_WAV;
        music.ctxData = ctxWav;

        if (success)
        {
            int sampleSize = ctxWav->bitsPerSample;
            if (ctxWav->bitsPerSample == 24) sampleSize = 16;   // Forcing conversion to s16 on UpdateMusicStream()

            music.stream = InitAudioStream(ctxWav->sampleRate, sampleSize, ctxWav->channels);
            music.sampleCount = (unsigned int)ctxWav->totalPCMFrameCount*ctxWav->channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (IsFileExtension(fileName, ".ogg"))
    {
        // Open ogg audio stream
        music.ctxType = MUSIC_AUDIO_OGG;
        music.ctxData = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (music.ctxData != NULL)
        {
            stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis *)music.ctxData);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music.stream = InitAudioStream(info.sample_rate, 16, info.channels);

            // WARNING: It seems this function returns length in frames, not samples, so we multiply by channels
            music.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples((stb_vorbis *)music.ctxData)*info.channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (IsFileExtension(fileName, ".flac"))
    {
        music.ctxType = MUSIC_AUDIO_FLAC;
        music.ctxData = drflac_open_file(fileName, NULL);

        if (music.ctxData != NULL)
        {
            drflac *ctxFlac = (drflac *)music.ctxData;

            music.stream = InitAudioStream(ctxFlac->sampleRate, ctxFlac->bitsPerSample, ctxFlac->channels);
            music.sampleCount = (unsigned int)ctxFlac->totalPCMFrameCount*ctxFlac->channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (IsFileExtension(fileName, ".mp3"))
    {
        drmp3 *ctxMp3 = RL_CALLOC(1, sizeof(drmp3));
        int result = drmp3_init_file(ctxMp3, fileName, NULL);

        music.ctxType = MUSIC_AUDIO_MP3;
        music.ctxData = ctxMp3;

        if (result > 0)
        {
            music.stream = InitAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
            music.sampleCount = (unsigned int)drmp3_get_pcm_frame_count(ctxMp3)*ctxMp3->channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (IsFileExtension(fileName, ".xm"))
    {
        jar_xm_context_t *ctxXm = NULL;
        int result = jar_xm_create_context_from_file(&ctxXm, AUDIO.System.device.sampleRate, fileName);

        music.ctxType = MUSIC_MODULE_XM;
        music.ctxData = ctxXm;

        if (result == 0)    // XM AUDIO.System.context created successfully
        {
            jar_xm_set_max_loop_count(ctxXm, 0);    // Set infinite number of loops

            unsigned int bits = 32;
            if (AUDIO_DEVICE_FORMAT == ma_format_s16)
                bits = 16;
            else if (AUDIO_DEVICE_FORMAT == ma_format_u8)
                bits = 8;

            // NOTE: Only stereo is supported for XM
            music.stream = InitAudioStream(AUDIO.System.device.sampleRate, bits, AUDIO_DEVICE_CHANNELS);
            music.sampleCount = (unsigned int)jar_xm_get_remaining_samples(ctxXm)*2;    // 2 channels
            music.looping = true;   // Looping enabled by default
            jar_xm_reset(ctxXm);   // make sure we start at the beginning of the song
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (IsFileExtension(fileName, ".mod"))
    {
        jar_mod_context_t *ctxMod = RL_CALLOC(1, sizeof(jar_mod_context_t));
        jar_mod_init(ctxMod);
        int result = jar_mod_load_file(ctxMod, fileName);

        music.ctxType = MUSIC_MODULE_MOD;
        music.ctxData = ctxMod;

        if (result > 0)
        {
            // NOTE: Only stereo is supported for MOD
            music.stream = InitAudioStream(AUDIO.System.device.sampleRate, 16, AUDIO_DEVICE_CHANNELS);
            music.sampleCount = (unsigned int)jar_mod_max_samples(ctxMod)*2;    // 2 channels
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
    else TRACELOG(LOG_WARNING, "STREAM: [%s] Fileformat not supported", fileName);

    if (!musicLoaded)
    {
        if (false) { }
    #if defined(SUPPORT_FILEFORMAT_WAV)
        else if (music.ctxType == MUSIC_AUDIO_WAV) drwav_uninit((drwav *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_OGG)
        else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_FLAC)
        else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData, NULL);
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

        music.ctxData = NULL;
        TRACELOG(LOG_WARNING, "FILEIO: [%s] Music file could not be opened", fileName);
    }
    else
    {
        // Show some music stream info
        TRACELOG(LOG_INFO, "FILEIO: [%s] Music file successfully loaded:", fileName);
        TRACELOG(LOG_INFO, "    > Total samples: %i", music.sampleCount);
        TRACELOG(LOG_INFO, "    > Sample rate:   %i Hz", music.stream.sampleRate);
        TRACELOG(LOG_INFO, "    > Sample size:   %i bits", music.stream.sampleSize);
        TRACELOG(LOG_INFO, "    > Channels:      %i (%s)", music.stream.channels, (music.stream.channels == 1)? "Mono" : (music.stream.channels == 2)? "Stereo" : "Multi");
    }

    return music;
}

// extension including period ".mod"
Music LoadMusicStreamFromMemory(const char *fileType, unsigned char* data, int dataSize)
{
    Music music = { 0 };
    bool musicLoaded = false;

    char fileExtLower[16] = { 0 };
    strcpy(fileExtLower, TextToLower(fileType));

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if (TextIsEqual(fileExtLower, ".wav"))
    {
        drwav *ctxWav = RL_CALLOC(1, sizeof(drwav));

        bool success = drwav_init_memory(ctxWav, (const void*)data, dataSize, NULL);

        music.ctxType = MUSIC_AUDIO_WAV;
        music.ctxData = ctxWav;

        if (success)
        {
            int sampleSize = ctxWav->bitsPerSample;
            if (ctxWav->bitsPerSample == 24) sampleSize = 16;   // Forcing conversion to s16 on UpdateMusicStream()

            music.stream = InitAudioStream(ctxWav->sampleRate, sampleSize, ctxWav->channels);
            music.sampleCount = (unsigned int)ctxWav->totalPCMFrameCount*ctxWav->channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (TextIsEqual(fileExtLower, ".flac"))
    {
        music.ctxType = MUSIC_AUDIO_FLAC;
        music.ctxData = drflac_open_memory((const void*)data, dataSize, NULL);

        if (music.ctxData != NULL)
        {
            drflac *ctxFlac = (drflac *)music.ctxData;

            music.stream = InitAudioStream(ctxFlac->sampleRate, ctxFlac->bitsPerSample, ctxFlac->channels);
            music.sampleCount = (unsigned int)ctxFlac->totalPCMFrameCount*ctxFlac->channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (TextIsEqual(fileExtLower, ".mp3"))
    {
        drmp3 *ctxMp3 = RL_CALLOC(1, sizeof(drmp3));
        int success = drmp3_init_memory(ctxMp3, (const void*)data, dataSize, NULL);

        music.ctxType = MUSIC_AUDIO_MP3;
        music.ctxData = ctxMp3;

        if (success)
        {
            music.stream = InitAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
            music.sampleCount = (unsigned int)drmp3_get_pcm_frame_count(ctxMp3)*ctxMp3->channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (TextIsEqual(fileExtLower, ".ogg"))
    {
        // Open ogg audio stream
        music.ctxType = MUSIC_AUDIO_OGG;
        //music.ctxData = stb_vorbis_open_filename(fileName, NULL, NULL);
        music.ctxData = stb_vorbis_open_memory((const unsigned char*)data, dataSize, NULL, NULL);

        if (music.ctxData != NULL)
        {
            stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis *)music.ctxData);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music.stream = InitAudioStream(info.sample_rate, 16, info.channels);

            // WARNING: It seems this function returns length in frames, not samples, so we multiply by channels
            music.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples((stb_vorbis *)music.ctxData)*info.channels;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (TextIsEqual(fileExtLower, ".xm"))
    {
        jar_xm_context_t *ctxXm = NULL;
        int result = jar_xm_create_context_safe(&ctxXm, (const char*)data, dataSize, AUDIO.System.device.sampleRate);
        if (result == 0)    // XM AUDIO.System.context created successfully
        {
            music.ctxType = MUSIC_MODULE_XM;
            jar_xm_set_max_loop_count(ctxXm, 0);    // Set infinite number of loops

            unsigned int bits = 32;
            if (AUDIO_DEVICE_FORMAT == ma_format_s16)
                bits = 16;
            else if (AUDIO_DEVICE_FORMAT == ma_format_u8)
                bits = 8;

            // NOTE: Only stereo is supported for XM
            music.stream = InitAudioStream(AUDIO.System.device.sampleRate, bits, 2);
            music.sampleCount = (unsigned int)jar_xm_get_remaining_samples(ctxXm)*2;    // 2 channels
            music.looping = true;   // Looping enabled by default
            jar_xm_reset(ctxXm);   // make sure we start at the beginning of the song

            music.ctxData = ctxXm;
            musicLoaded = true;
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (TextIsEqual(fileExtLower, ".mod"))
    {
        jar_mod_context_t *ctxMod = RL_MALLOC(sizeof(jar_mod_context_t));
        int result = 0;

        jar_mod_init(ctxMod);

        // copy data to allocated memory for default UnloadMusicStream
        unsigned char *newData = RL_MALLOC(dataSize);
        int it = dataSize/sizeof(unsigned char);
        for (int i = 0; i < it; i++){
            newData[i] = data[i];
        }

        // Memory loaded version for jar_mod_load_file()
        if (dataSize && dataSize < 32*1024*1024)
        {
            ctxMod->modfilesize = dataSize;
            ctxMod->modfile = newData;
            if (jar_mod_load(ctxMod, (void *)ctxMod->modfile, dataSize)) result = dataSize;
        }

        if (result > 0)
        {
            music.ctxType = MUSIC_MODULE_MOD;

            // NOTE: Only stereo is supported for MOD
            music.stream = InitAudioStream(AUDIO.System.device.sampleRate, 16, 2);
            music.sampleCount = (unsigned int)jar_mod_max_samples(ctxMod)*2;    // 2 channels
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;

            music.ctxData = ctxMod;
            musicLoaded = true;
        }
    }
#endif
    else TRACELOG(LOG_WARNING, "STREAM: [%s] Fileformat not supported", fileType);

    if (!musicLoaded)
    {
        if (false) { }
    #if defined(SUPPORT_FILEFORMAT_WAV)
        else if (music.ctxType == MUSIC_AUDIO_WAV) drwav_uninit((drwav *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_FLAC)
        else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData, NULL);
    #endif
    #if defined(SUPPORT_FILEFORMAT_MP3)
        else if (music.ctxType == MUSIC_AUDIO_MP3) { drmp3_uninit((drmp3 *)music.ctxData); RL_FREE(music.ctxData); }
    #endif
    #if defined(SUPPORT_FILEFORMAT_OGG)
        else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_XM)
        else if (music.ctxType == MUSIC_MODULE_XM) jar_xm_free_context((jar_xm_context_t *)music.ctxData);
    #endif
    #if defined(SUPPORT_FILEFORMAT_MOD)
        else if (music.ctxType == MUSIC_MODULE_MOD) { jar_mod_unload((jar_mod_context_t *)music.ctxData); RL_FREE(music.ctxData); }
    #endif

        music.ctxData = NULL;
        TRACELOG(LOG_WARNING, "FILEIO: [%s] Music memory could not be opened", fileType);
    }
    else
    {
        // Show some music stream info
        TRACELOG(LOG_INFO, "FILEIO: [%s] Music memory successfully loaded:", fileType);
        TRACELOG(LOG_INFO, "    > Total samples: %i", music.sampleCount);
        TRACELOG(LOG_INFO, "    > Sample rate:   %i Hz", music.stream.sampleRate);
        TRACELOG(LOG_INFO, "    > Sample size:   %i bits", music.stream.sampleSize);
        TRACELOG(LOG_INFO, "    > Channels:      %i (%s)", music.stream.channels, (music.stream.channels == 1)? "Mono" : (music.stream.channels == 2)? "Stereo" : "Multi");
    }

    return music;
}

// Unload music stream
void UnloadMusicStream(Music music)
{
    CloseAudioStream(music.stream);

    if (music.ctxData != NULL)
    {
        if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
        else if (music.ctxType == MUSIC_AUDIO_WAV) drwav_uninit((drwav *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
        else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
        else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData, NULL);
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
}

// Start music playing (open stream)
void PlayMusicStream(Music music)
{
    if (music.stream.buffer != NULL)
    {
        // For music streams, we need to make sure we maintain the frame cursor position
        // This is a hack for this section of code in UpdateMusicStream()
        // NOTE: In case window is minimized, music stream is stopped, just make sure to
        // play again on window restore: if (IsMusicPlaying(music)) PlayMusicStream(music);
        ma_uint32 frameCursorPos = music.stream.buffer->frameCursorPos;
        PlayAudioStream(music.stream);  // WARNING: This resets the cursor position.
        music.stream.buffer->frameCursorPos = frameCursorPos;
    }
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
void StopMusicStream(Music music)
{
    StopAudioStream(music.stream);

    switch (music.ctxType)
    {
#if defined(SUPPORT_FILEFORMAT_WAV)
        case MUSIC_AUDIO_WAV: drwav_seek_to_pcm_frame((drwav *)music.ctxData, 0); break;
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_start((stb_vorbis *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
        case MUSIC_AUDIO_FLAC: drflac_seek_to_pcm_frame((drflac *)music.ctxData, 0); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
        case MUSIC_AUDIO_MP3: drmp3_seek_to_pcm_frame((drmp3 *)music.ctxData, 0); break;
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
        case MUSIC_MODULE_XM: jar_xm_reset((jar_xm_context_t *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
        case MUSIC_MODULE_MOD: jar_mod_seek_start((jar_mod_context_t *)music.ctxData); break;
#endif
        default: break;
    }
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(Music music)
{
    if (music.stream.buffer == NULL)
        return;

    if (music.ctxType == MUSIC_MODULE_XM)
        jar_xm_set_max_loop_count(music.ctxData, music.looping ? 0 : 1);

    bool streamEnding = false;

    unsigned int subBufferSizeInFrames = music.stream.buffer->sizeInFrames/2;

    // NOTE: Using dynamic allocation because it could require more than 16KB
    void *pcm = RL_CALLOC(subBufferSizeInFrames*music.stream.channels*music.stream.sampleSize/8, 1);

    int samplesCount = 0;    // Total size of data streamed in L+R samples for xm floats, individual L or R for ogg shorts

    // TODO: Get the sampleLeft using totalFramesProcessed... but first, get total frames processed correctly...
    //ma_uint32 frameSizeInBytes = ma_get_bytes_per_sample(music.stream.buffer->dsp.formatConverterIn.config.formatIn)*music.stream.buffer->dsp.formatConverterIn.config.channels;
    int sampleLeft = music.sampleCount - (music.stream.buffer->totalFramesProcessed*music.stream.channels);

    if (music.ctxType == MUSIC_MODULE_XM && music.looping) sampleLeft = subBufferSizeInFrames*4;

    while (IsAudioStreamProcessed(music.stream))
    {
        if ((sampleLeft/music.stream.channels) >= subBufferSizeInFrames) samplesCount = subBufferSizeInFrames*music.stream.channels;
        else samplesCount = sampleLeft;

        switch (music.ctxType)
        {
        #if defined(SUPPORT_FILEFORMAT_WAV)
            case MUSIC_AUDIO_WAV:
            {
                // NOTE: Returns the number of samples to process (not required)
                if (music.stream.sampleSize == 16) drwav_read_pcm_frames_s16((drwav *)music.ctxData, samplesCount/music.stream.channels, (short *)pcm);
                else if (music.stream.sampleSize == 32) drwav_read_pcm_frames_f32((drwav *)music.ctxData, samplesCount/music.stream.channels, (float *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_OGG)
            case MUSIC_AUDIO_OGG:
            {
                // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
                stb_vorbis_get_samples_short_interleaved((stb_vorbis *)music.ctxData, music.stream.channels, (short *)pcm, samplesCount);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_FLAC)
            case MUSIC_AUDIO_FLAC:
            {
                // NOTE: Returns the number of samples to process (not required)
                drflac_read_pcm_frames_s16((drflac *)music.ctxData, samplesCount, (short *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MP3)
            case MUSIC_AUDIO_MP3:
            {
                // NOTE: samplesCount, actually refers to framesCount and returns the number of frames processed
                drmp3_read_pcm_frames_f32((drmp3 *)music.ctxData, samplesCount/music.stream.channels, (float *)pcm);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_XM)
            case MUSIC_MODULE_XM:
            {
                switch (AUDIO_DEVICE_FORMAT)
                {
                case ma_format_f32:
                    // NOTE: Internally this function considers 2 channels generation, so samplesCount/2
                    jar_xm_generate_samples((jar_xm_context_t*)music.ctxData, (float*)pcm, samplesCount / 2);
                    break;

                case ma_format_s16:
                    // NOTE: Internally this function considers 2 channels generation, so samplesCount/2
                    jar_xm_generate_samples_16bit((jar_xm_context_t*)music.ctxData, (short*)pcm, samplesCount / 2);
                    break;

                case ma_format_u8:
                    // NOTE: Internally this function considers 2 channels generation, so samplesCount/2
                    jar_xm_generate_samples_8bit((jar_xm_context_t*)music.ctxData, (char*)pcm, samplesCount / 2);
                    break;
                }

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MOD)
            case MUSIC_MODULE_MOD:
            {
                // NOTE: 3rd parameter (nbsample) specify the number of stereo 16bits samples you want, so sampleCount/2
                jar_mod_fillbuffer((jar_mod_context_t *)music.ctxData, (short *)pcm, samplesCount/2, 0);
            } break;
        #endif
            default: break;
        }

        UpdateAudioStream(music.stream, pcm, samplesCount);

        if ((music.ctxType == MUSIC_MODULE_XM) || music.ctxType == MUSIC_MODULE_MOD)
        {
            if (samplesCount > 1) sampleLeft -= samplesCount/2;
            else sampleLeft -= samplesCount;
        }
        else sampleLeft -= samplesCount;

        if (sampleLeft <= 0)
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
        StopMusicStream(music);                     // Stop music (and reset)
        if (music.looping) PlayMusicStream(music);  // Play again
    }
    else
    {
        // NOTE: In case window is minimized, music stream is stopped,
        // just make sure to play again on window restore
        if (IsMusicPlaying(music)) PlayMusicStream(music);
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
    SetAudioBufferPitch(music.stream.buffer, pitch);
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
#if defined(SUPPORT_FILEFORMAT_XM)
    if (music.ctxType == MUSIC_MODULE_XM)
    {
        uint64_t samples = 0;
        jar_xm_get_position(music.ctxData, NULL, NULL, NULL, &samples);
        samples = samples % (music.sampleCount);

        return (float)(samples)/(music.stream.sampleRate*music.stream.channels);
    }
#endif
    float secondsPlayed = 0.0f;
    if (music.stream.buffer != NULL)
    {
        //ma_uint32 frameSizeInBytes = ma_get_bytes_per_sample(music.stream.buffer->dsp.formatConverterIn.config.formatIn)*music.stream.buffer->dsp.formatConverterIn.config.channels;
        unsigned int samplesPlayed = music.stream.buffer->totalFramesProcessed*music.stream.channels;
        secondsPlayed = (float)samplesPlayed/(music.stream.sampleRate*music.stream.channels);
    }

    return secondsPlayed;
}

// Init audio stream (to stream audio pcm data)
AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
{
    AudioStream stream = { 0 };

    stream.sampleRate = sampleRate;
    stream.sampleSize = sampleSize;
    stream.channels = channels;

    ma_format formatIn = ((stream.sampleSize == 8)? ma_format_u8 : ((stream.sampleSize == 16)? ma_format_s16 : ma_format_f32));

    // The size of a streaming buffer must be at least double the size of a period
    unsigned int periodSize = AUDIO.System.device.playback.internalPeriodSizeInFrames;
    unsigned int subBufferSize = GetAudioStreamBufferSizeDefault();

    if (subBufferSize < periodSize) subBufferSize = periodSize;

    // Create a double audio buffer of defined size
    stream.buffer = LoadAudioBuffer(formatIn, stream.channels, stream.sampleRate, subBufferSize*2, AUDIO_BUFFER_USAGE_STREAM);

    if (stream.buffer != NULL)
    {
        stream.buffer->looping = true;    // Always loop for streaming buffers
        TRACELOG(LOG_INFO, "STREAM: Initialized successfully (%i Hz, %i bit, %s)", stream.sampleRate, stream.sampleSize, (stream.channels == 1)? "Mono" : "Stereo");
    }
    else TRACELOG(LOG_WARNING, "STREAM: Failed to load audio buffer, stream could not be created");

    return stream;
}

// Close audio stream and free memory
void CloseAudioStream(AudioStream stream)
{
    UnloadAudioBuffer(stream.buffer);

    TRACELOG(LOG_INFO, "STREAM: Unloaded audio stream data from RAM");
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: unqueue -> update -> queue
// NOTE 2: To unqueue a buffer it needs to be processed: IsAudioStreamProcessed()
void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount)
{
    if (stream.buffer != NULL)
    {
        if (stream.buffer->isSubBufferProcessed[0] || stream.buffer->isSubBufferProcessed[1])
        {
            ma_uint32 subBufferToUpdate = 0;

            if (stream.buffer->isSubBufferProcessed[0] && stream.buffer->isSubBufferProcessed[1])
            {
                // Both buffers are available for updating.
                // Update the first one and make sure the cursor is moved back to the front.
                subBufferToUpdate = 0;
                stream.buffer->frameCursorPos = 0;
            }
            else
            {
                // Just update whichever sub-buffer is processed.
                subBufferToUpdate = (stream.buffer->isSubBufferProcessed[0])? 0 : 1;
            }

            ma_uint32 subBufferSizeInFrames = stream.buffer->sizeInFrames/2;
            unsigned char *subBuffer = stream.buffer->data + ((subBufferSizeInFrames*stream.channels*(stream.sampleSize/8))*subBufferToUpdate);

            // TODO: Get total frames processed on this buffer... DOES NOT WORK.
            stream.buffer->totalFramesProcessed += subBufferSizeInFrames;

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

                if (leftoverFrameCount > 0) memset(subBuffer + bytesToWrite, 0, leftoverFrameCount*stream.channels*(stream.sampleSize/8));

                stream.buffer->isSubBufferProcessed[subBufferToUpdate] = false;
            }
            else TRACELOG(LOG_WARNING, "STREAM: Attempting to write too many frames to buffer");
        }
        else TRACELOG(LOG_WARNING, "STREAM: Buffer not available for updating");
    }
}

// Check if any audio stream buffers requires refill
bool IsAudioStreamProcessed(AudioStream stream)
{
    if (stream.buffer == NULL) return false;

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

// Set volume for audio stream (1.0 is max level)
void SetAudioStreamVolume(AudioStream stream, float volume)
{
    SetAudioBufferVolume(stream.buffer, volume);
}

// Set pitch for audio stream (1.0 is base level)
void SetAudioStreamPitch(AudioStream stream, float pitch)
{
    SetAudioBufferPitch(stream.buffer, pitch);
}

// Default size for new audio streams
void SetAudioStreamBufferSizeDefault(int size)
{
    AUDIO.Buffer.defaultSize = size;
}

int GetAudioStreamBufferSizeDefault()
{
    // if the buffer is not set, compute one that would give us a buffer good enough for a decent frame rate
    if (AUDIO.Buffer.defaultSize == 0)
        AUDIO.Buffer.defaultSize = AUDIO.System.device.sampleRate/30;

    return AUDIO.Buffer.defaultSize;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Log callback function
static void OnLog(ma_context *pContext, ma_device *pDevice, ma_uint32 logLevel, const char *message)
{
    (void)pContext;
    (void)pDevice;

    TRACELOG(LOG_WARNING, "miniaudio: %s", message);   // All log messages from miniaudio are errors
}

// Reads audio data from an AudioBuffer object in internal format.
static ma_uint32 ReadAudioBufferFramesInInternalFormat(AudioBuffer *audioBuffer, void *framesOut, ma_uint32 frameCount)
{
    ma_uint32 subBufferSizeInFrames = (audioBuffer->sizeInFrames > 1)? audioBuffer->sizeInFrames/2 : audioBuffer->sizeInFrames;
    ma_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos/subBufferSizeInFrames;

    if (currentSubBufferIndex > 1) return 0;

    // Another thread can update the processed state of buffers so
    // we just take a copy here to try and avoid potential synchronization problems
    bool isSubBufferProcessed[2];
    isSubBufferProcessed[0] = audioBuffer->isSubBufferProcessed[0];
    isSubBufferProcessed[1] = audioBuffer->isSubBufferProcessed[1];

    ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(audioBuffer->converter.config.formatIn, audioBuffer->converter.config.channelsIn);

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
            framesRemainingInOutputBuffer = audioBuffer->sizeInFrames - audioBuffer->frameCursorPos;
        }
        else
        {
            ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames*currentSubBufferIndex;
            framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos - firstFrameIndexOfThisSubBuffer);
        }

        ma_uint32 framesToRead = totalFramesRemaining;
        if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

        memcpy((unsigned char *)framesOut + (framesRead*frameSizeInBytes), audioBuffer->data + (audioBuffer->frameCursorPos*frameSizeInBytes), framesToRead*frameSizeInBytes);
        audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead)%audioBuffer->sizeInFrames;
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
        memset((unsigned char *)framesOut + (framesRead*frameSizeInBytes), 0, totalFramesRemaining*frameSizeInBytes);

        // For static buffers we can fill the remaining frames with silence for safety, but we don't want
        // to report those frames as "read". The reason for this is that the caller uses the return value
        // to know whether or not a non-looping sound has finished playback.
        if (audioBuffer->usage != AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
    }

    return framesRead;
}

// Reads audio data from an AudioBuffer object in device format. Returned data will be in a format appropriate for mixing.
static ma_uint32 ReadAudioBufferFramesInMixingFormat(AudioBuffer *audioBuffer, float *framesOut, ma_uint32 frameCount)
{
    // What's going on here is that we're continuously converting data from the AudioBuffer's internal format to the mixing format, which
    // should be defined by the output format of the data converter. We do this until frameCount frames have been output. The important
    // detail to remember here is that we never, ever attempt to read more input data than is required for the specified number of output
    // frames. This can be achieved with ma_data_converter_get_required_input_frame_count().
    ma_uint8 inputBuffer[4096];
    ma_uint32 inputBufferFrameCap = sizeof(inputBuffer)/ma_get_bytes_per_frame(audioBuffer->converter.config.formatIn, audioBuffer->converter.config.channelsIn);

    ma_uint32 totalOutputFramesProcessed = 0;
    while (totalOutputFramesProcessed < frameCount)
    {
        ma_uint64 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;

        ma_uint64 inputFramesToProcessThisIteration = ma_data_converter_get_required_input_frame_count(&audioBuffer->converter, outputFramesToProcessThisIteration);
        if (inputFramesToProcessThisIteration > inputBufferFrameCap)
        {
            inputFramesToProcessThisIteration = inputBufferFrameCap;
        }

        float *runningFramesOut = framesOut + (totalOutputFramesProcessed*audioBuffer->converter.config.channelsOut);

        /* At this point we can convert the data to our mixing format. */
        ma_uint64 inputFramesProcessedThisIteration = ReadAudioBufferFramesInInternalFormat(audioBuffer, inputBuffer, (ma_uint32)inputFramesToProcessThisIteration);    /* Safe cast. */
        ma_uint64 outputFramesProcessedThisIteration = outputFramesToProcessThisIteration;
        ma_data_converter_process_pcm_frames(&audioBuffer->converter, inputBuffer, &inputFramesProcessedThisIteration, runningFramesOut, &outputFramesProcessedThisIteration);

        totalOutputFramesProcessed += (ma_uint32)outputFramesProcessedThisIteration; /* Safe cast. */

        if (inputFramesProcessedThisIteration < inputFramesToProcessThisIteration)
        {
            break;  /* Ran out of input data. */
        }

        /* This should never be hit, but will add it here for safety. Ensures we get out of the loop when no input nor output frames are processed. */
        if (inputFramesProcessedThisIteration == 0 && outputFramesProcessedThisIteration == 0)
        {
            break;
        }
    }

    return totalOutputFramesProcessed;
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
    ma_mutex_lock(&AUDIO.System.lock);
    {
        for (AudioBuffer *audioBuffer = AUDIO.Buffer.first; audioBuffer != NULL; audioBuffer = audioBuffer->next)
        {
            // Ignore stopped or paused sounds
            if (!audioBuffer->playing || audioBuffer->paused) continue;

            ma_uint32 framesRead = 0;

            while (1)
            {
                if (framesRead >= frameCount) break;

                // Just read as much data as we can from the stream
                ma_uint32 framesToRead = (frameCount - framesRead);

                while (framesToRead > 0)
                {
                    float tempBuffer[1024]; // 512 frames for stereo

                    ma_uint32 framesToReadRightNow = framesToRead;
                    if (framesToReadRightNow > sizeof(tempBuffer)/sizeof(tempBuffer[0])/AUDIO_DEVICE_CHANNELS)
                    {
                        framesToReadRightNow = sizeof(tempBuffer)/sizeof(tempBuffer[0])/AUDIO_DEVICE_CHANNELS;
                    }

                    ma_uint32 framesJustRead = ReadAudioBufferFramesInMixingFormat(audioBuffer, tempBuffer, framesToReadRightNow);
                    if (framesJustRead > 0)
                    {
                        float *framesOut = (float *)pFramesOut + (framesRead*AUDIO.System.device.playback.channels);
                        float *framesIn  = tempBuffer;

                        MixAudioFrames(framesOut, framesIn, framesJustRead, audioBuffer->volume);

                        framesToRead -= framesJustRead;
                        framesRead += framesJustRead;
                    }

                    if (!audioBuffer->playing)
                    {
                        framesRead = frameCount;
                        break;
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

    ma_mutex_unlock(&AUDIO.System.lock);
}

// This is the main mixing function. Mixing is pretty simple in this project - it's just an accumulation.
// NOTE: framesOut is both an input and an output. It will be initially filled with zeros outside of this function.
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume)
{
    for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame)
    {
        for (ma_uint32 iChannel = 0; iChannel < AUDIO.System.device.playback.channels; ++iChannel)
        {
            float *frameOut = framesOut + (iFrame*AUDIO.System.device.playback.channels);
            const float *frameIn  = framesIn  + (iFrame*AUDIO.System.device.playback.channels);

            frameOut[iChannel] += (frameIn[iChannel]*localVolume);
        }
    }
}

#if defined(SUPPORT_FILEFORMAT_WAV)
// Load WAV file data into Wave structure
// NOTE: Using dr_wav library
static Wave LoadWAV(const unsigned char *fileData, unsigned int fileSize)
{
    Wave wave = { 0 };
    drwav wav = { 0 };

    bool success = drwav_init_memory(&wav, fileData, fileSize, NULL);

    if (success)
    {
        wave.sampleCount = (unsigned int)wav.totalPCMFrameCount*wav.channels;
        wave.sampleRate = wav.sampleRate;
        wave.sampleSize = 16;   // NOTE: We are forcing conversion to 16bit
        wave.channels = wav.channels;
        wave.data = (short *)RL_MALLOC(wave.sampleCount*sizeof(short));
        drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, wave.data);
    }
    else TRACELOG(LOG_WARNING, "WAVE: Failed to load WAV data");

    drwav_uninit(&wav);

    return wave;
}

// Save wave data as WAV file
// NOTE: Using dr_wav library
static int SaveWAV(Wave wave, const char *fileName)
{
    int success = false;

    drwav wav = { 0 };
    drwav_data_format format = { 0 };
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_PCM;
    format.channels = wave.channels;
    format.sampleRate = wave.sampleRate;
    format.bitsPerSample = wave.sampleSize;

    void *fileData = NULL;
    size_t fileDataSize = 0;
    success = drwav_init_memory_write(&wav, &fileData, &fileDataSize, &format, NULL);
    if (success) success = (int)drwav_write_pcm_frames(&wav, wave.sampleCount/wave.channels, wave.data);
    drwav_result result = drwav_uninit(&wav);

    if (result == DRWAV_SUCCESS) success = SaveFileData(fileName, (unsigned char *)fileData, (unsigned int)fileDataSize);

    drwav_free(fileData, NULL);

    return success;
}
#endif

#if defined(SUPPORT_FILEFORMAT_OGG)
// Load OGG file data into Wave structure
// NOTE: Using stb_vorbis library
static Wave LoadOGG(const unsigned char *fileData, unsigned int fileSize)
{
    Wave wave = { 0 };

    stb_vorbis *oggData = stb_vorbis_open_memory((unsigned char *)fileData, fileSize, NULL, NULL);

    if (oggData != NULL)
    {
        stb_vorbis_info info = stb_vorbis_get_info(oggData);

        wave.sampleRate = info.sample_rate;
        wave.sampleSize = 16;                   // 16 bit per sample (short)
        wave.channels = info.channels;
        wave.sampleCount = (unsigned int)stb_vorbis_stream_length_in_samples(oggData)*info.channels;  // Independent by channel

        float totalSeconds = stb_vorbis_stream_length_in_seconds(oggData);
        if (totalSeconds > 10) TRACELOG(LOG_WARNING, "WAVE: OGG audio length larger than 10 seconds (%f sec.), that's a big file in memory, consider music streaming", totalSeconds);

        wave.data = (short *)RL_MALLOC(wave.sampleCount*sizeof(short));

        // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
        stb_vorbis_get_samples_short_interleaved(oggData, info.channels, (short *)wave.data, wave.sampleCount);
        TRACELOG(LOG_INFO, "WAVE: OGG data loaded successfully (%i Hz, %i bit, %s)", wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");

        stb_vorbis_close(oggData);
    }
    else TRACELOG(LOG_WARNING, "WAVE: Failed to load OGG data");

    return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)
// Load FLAC file data into Wave structure
// NOTE: Using dr_flac library
static Wave LoadFLAC(const unsigned char *fileData, unsigned int fileSize)
{
    Wave wave = { 0 };

    // Decode the entire FLAC file in one go
    unsigned long long int totalFrameCount = 0;
    wave.data = drflac_open_memory_and_read_pcm_frames_s16(fileData, fileSize, &wave.channels, &wave.sampleRate, &totalFrameCount, NULL);

    if (wave.data != NULL)
    {
        wave.sampleCount = (unsigned int)totalFrameCount*wave.channels;
        wave.sampleSize = 16;

        TRACELOG(LOG_INFO, "WAVE: FLAC data loaded successfully (%i Hz, %i bit, %s)", wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");
    }
    else TRACELOG(LOG_WARNING, "WAVE: Failed to load FLAC data");

    return wave;
}
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
// Load MP3 file data into Wave structure
// NOTE: Using dr_mp3 library
static Wave LoadMP3(const unsigned char *fileData, unsigned int fileSize)
{
    Wave wave = { 0 };
    drmp3_config config = { 0 };

    // Decode the entire MP3 file in one go
    unsigned long long int totalFrameCount = 0;
    wave.data = drmp3_open_memory_and_read_pcm_frames_f32(fileData, fileSize, &config, &totalFrameCount, NULL);

    if (wave.data != NULL)
    {
        wave.channels = config.channels;
        wave.sampleRate = config.sampleRate;
        wave.sampleCount = (int)totalFrameCount*wave.channels;
        wave.sampleSize = 32;

        // NOTE: Only support up to 2 channels (mono, stereo)
        // TODO: Really?
        if (wave.channels > 2) TRACELOG(LOG_WARNING, "WAVE: MP3 channels number (%i) not supported", wave.channels);

        TRACELOG(LOG_INFO, "WAVE: MP3 file loaded successfully (%i Hz, %i bit, %s)", wave.sampleRate, wave.sampleSize, (wave.channels == 1)? "Mono" : "Stereo");
    }
    else TRACELOG(LOG_WARNING, "WAVE: Failed to load MP3 data");

    return wave;
}
#endif

// Some required functions for audio standalone module version
#if defined(RAUDIO_STANDALONE)
// Check file extension
static bool IsFileExtension(const char *fileName, const char *ext)
{
    bool result = false;
    const char *fileExt;

    if ((fileExt = strrchr(fileName, '.')) != NULL)
    {
        if (strcmp(fileExt, ext) == 0) result = true;
    }

    return result;
}

// Load data from file into a buffer
static unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead)
{
    unsigned char *data = NULL;
    *bytesRead = 0;

    if (fileName != NULL)
    {
        FILE *file = fopen(fileName, "rb");

        if (file != NULL)
        {
            // WARNING: On binary streams SEEK_END could not be found,
            // using fseek() and ftell() could not work in some (rare) cases
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                data = (unsigned char *)RL_MALLOC(size*sizeof(unsigned char));

                // NOTE: fread() returns number of read elements instead of bytes, so we read [1 byte, size elements]
                unsigned int count = (unsigned int)fread(data, sizeof(unsigned char), size, file);
                *bytesRead = count;

                if (count != size) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded", fileName);
                else TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return data;
}

// Save data to file from buffer
static bool SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite)
{
    if (fileName != NULL)
    {
        FILE *file = fopen(fileName, "wb");

        if (file != NULL)
        {
            unsigned int count = (unsigned int)fwrite(data, sizeof(unsigned char), bytesToWrite, file);

            if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write file", fileName);
            else if (count != bytesToWrite) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially written", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] File saved successfully", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
}

// Save text data to file (write), string must be '\0' terminated
static bool SaveFileText(const char *fileName, char *text)
{
    if (fileName != NULL)
    {
        FILE *file = fopen(fileName, "wt");

        if (file != NULL)
        {
            int count = fprintf(file, "%s", text);

            if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write text file", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] Text file saved successfully", fileName);

            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
}
#endif

#undef AudioBuffer
