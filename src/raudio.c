/**********************************************************************************************
*
*   raudio v1.1 - A simple and easy-to-use audio library based on miniaudio
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
*       #define SUPPORT_MODULE_RAUDIO
*           raudio module is included in the build
*
*       #define RAUDIO_STANDALONE
*           Define to use the module as standalone library (independently of raylib).
*           Required types and functions are defined in the same module.
*
*       #define SUPPORT_FILEFORMAT_WAV
*       #define SUPPORT_FILEFORMAT_OGG
*       #define SUPPORT_FILEFORMAT_MP3
*       #define SUPPORT_FILEFORMAT_QOA
*       #define SUPPORT_FILEFORMAT_FLAC
*       #define SUPPORT_FILEFORMAT_XM
*       #define SUPPORT_FILEFORMAT_MOD
*           Selected desired fileformats to be supported for loading. Some of those formats are
*           supported by default, to remove support, just comment unrequired #define in this module
*
*   DEPENDENCIES:
*       miniaudio.h  - Audio device management lib (https://github.com/mackron/miniaudio)
*       stb_vorbis.h - Ogg audio files loading (http://www.nothings.org/stb_vorbis/)
*       dr_wav.h     - WAV audio files loading (http://github.com/mackron/dr_libs)
*       dr_mp3.h     - MP3 audio file loading (https://github.com/mackron/dr_libs)
*       dr_flac.h    - FLAC audio file loading (https://github.com/mackron/dr_libs)
*       jar_xm.h     - XM module file loading
*       jar_mod.h    - MOD audio file loading
*
*   CONTRIBUTORS:
*       David Reid (github: @mackron) (Nov. 2017):
*           - Complete port to miniaudio library
*
*       Joshua Reisenauer (github: @kd7tck) (2015):
*           - XM audio module support (jar_xm)
*           - MOD audio module support (jar_mod)
*           - Mixing channels support
*           - Raw audio context support
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
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
#else
    #include "raylib.h"         // Declares module functions

    // Check if config flags have been externally provided on compilation line
    #if !defined(EXTERNAL_CONFIG_FLAGS)
        #include "config.h"     // Defines module configuration flags
    #endif
    #include "utils.h"          // Required for: fopen() Android mapping
#endif

#if defined(SUPPORT_MODULE_RAUDIO)

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

#include <windows.h>        // Windows functionality (miniaudio)

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

#include <objbase.h>        // Component Object Model (COM) header
#include <mmreg.h>          // Windows Multimedia, defines some WAVE structs
#include <mmsystem.h>       // Windows Multimedia, used by Windows GDI, defines DIBINDEX macro

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
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MA_NO_GENERATION

// Threading model: Default: [0] COINIT_MULTITHREADED: COM calls objects on any thread (free threading)
#define MA_COINIT_VALUE  2              // [2] COINIT_APARTMENTTHREADED: Each object has its own thread (apartment model)

#define MINIAUDIO_IMPLEMENTATION
//#define MA_DEBUG_OUTPUT
#include "external/miniaudio.h"         // Audio device initialization and management
#undef PlaySound                        // Win32 API: windows.h > mmsystem.h defines PlaySound macro

#include <stdlib.h>                     // Required for: malloc(), free()
#include <stdio.h>                      // Required for: FILE, fopen(), fclose(), fread()
#include <string.h>                     // Required for: strcmp() [Used in IsFileExtension(), LoadWaveFromMemory(), LoadMusicStreamFromMemory()]

#if defined(RAUDIO_STANDALONE)
    #ifndef TRACELOG
        #define TRACELOG(level, ...)    printf(__VA_ARGS__)
    #endif

    // Allow custom memory allocators
    #ifndef RL_MALLOC
        #define RL_MALLOC(sz)           malloc(sz)
    #endif
    #ifndef RL_CALLOC
        #define RL_CALLOC(n,sz)         calloc(n,sz)
    #endif
    #ifndef RL_REALLOC
        #define RL_REALLOC(ptr,sz)      realloc(ptr,sz)
    #endif
    #ifndef RL_FREE
        #define RL_FREE(ptr)            free(ptr)
    #endif
#endif

#if defined(SUPPORT_FILEFORMAT_WAV)
    #define DRWAV_MALLOC RL_MALLOC
    #define DRWAV_REALLOC RL_REALLOC
    #define DRWAV_FREE RL_FREE

    #define DR_WAV_IMPLEMENTATION
    #include "external/dr_wav.h"        // WAV loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_OGG)
    // TODO: Remap stb_vorbis malloc()/free() calls to RL_MALLOC/RL_FREE
    #include "external/stb_vorbis.c"    // OGG loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_MP3)
    #define DRMP3_MALLOC RL_MALLOC
    #define DRMP3_REALLOC RL_REALLOC
    #define DRMP3_FREE RL_FREE

    #define DR_MP3_IMPLEMENTATION
    #include "external/dr_mp3.h"        // MP3 loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_QOA)
    #define QOA_MALLOC RL_MALLOC
    #define QOA_FREE RL_FREE

    #if defined(_MSC_VER)           // Disable some MSVC warning
        #pragma warning(push)
        #pragma warning(disable : 4018)
        #pragma warning(disable : 4267)
        #pragma warning(disable : 4244)
    #endif

    #define QOA_IMPLEMENTATION
    #include "external/qoa.h"           // QOA loading and saving functions
    #include "external/qoaplay.c"       // QOA stream playing helper functions

    #if defined(_MSC_VER)
        #pragma warning(pop)        // Disable MSVC warning suppression
    #endif
#endif

#if defined(SUPPORT_FILEFORMAT_FLAC)
    #define DRFLAC_MALLOC RL_MALLOC
    #define DRFLAC_REALLOC RL_REALLOC
    #define DRFLAC_FREE RL_FREE

    #define DR_FLAC_IMPLEMENTATION
    #define DR_FLAC_NO_WIN32_IO
    #include "external/dr_flac.h"       // FLAC loading functions
#endif

#if defined(SUPPORT_FILEFORMAT_XM)
    #define JARXM_MALLOC RL_MALLOC
    #define JARXM_FREE RL_FREE

    #if defined(_MSC_VER)           // Disable some MSVC warning
        #pragma warning(push)
        #pragma warning(disable : 4244)
    #endif

    #define JAR_XM_IMPLEMENTATION
    #include "external/jar_xm.h"        // XM loading functions

    #if defined(_MSC_VER)
        #pragma warning(pop)        // Disable MSVC warning suppression
    #endif
#endif

#if defined(SUPPORT_FILEFORMAT_MOD)
    #define JARMOD_MALLOC RL_MALLOC
    #define JARMOD_FREE RL_FREE

    #define JAR_MOD_IMPLEMENTATION
    #include "external/jar_mod.h"       // MOD loading functions
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
    #define AUDIO_DEVICE_SAMPLE_RATE           0    // Device output sample rate
#endif

#ifndef MAX_AUDIO_BUFFER_POOL_CHANNELS
    #define MAX_AUDIO_BUFFER_POOL_CHANNELS    16    // Audio pool channels
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if defined(RAUDIO_STANDALONE)
// Trace log level
// NOTE: Organized by priority level
typedef enum {
    LOG_ALL = 0,        // Display all logs
    LOG_TRACE,          // Trace logging, intended for internal use only
    LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    LOG_INFO,           // Info logging, used for program execution info
    LOG_WARNING,        // Warning logging, used on recoverable failures
    LOG_ERROR,          // Error logging, used on unrecoverable failures
    LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    LOG_NONE            // Disable logging
} TraceLogLevel;
#endif

// Music context type
// NOTE: Depends on data structure provided by the library
// in charge of reading the different file types
typedef enum {
    MUSIC_AUDIO_NONE = 0,   // No audio context loaded
    MUSIC_AUDIO_WAV,        // WAV audio context
    MUSIC_AUDIO_OGG,        // OGG audio context
    MUSIC_AUDIO_FLAC,       // FLAC audio context
    MUSIC_AUDIO_MP3,        // MP3 audio context
    MUSIC_AUDIO_QOA,        // QOA audio context
    MUSIC_MODULE_XM,        // XM module audio context
    MUSIC_MODULE_MOD        // MOD module audio context
} MusicContextType;

// NOTE: Different logic is used when feeding data to the playback device
// depending on whether data is streamed (Music vs Sound)
typedef enum {
    AUDIO_BUFFER_USAGE_STATIC = 0,
    AUDIO_BUFFER_USAGE_STREAM
} AudioBufferUsage;

// Audio buffer struct
struct rAudioBuffer {
    ma_data_converter converter;    // Audio data converter

    AudioCallback callback;         // Audio buffer callback for buffer filling on audio threads
    rAudioProcessor *processor;     // Audio processor

    float volume;                   // Audio buffer volume
    float pitch;                    // Audio buffer pitch
    float pan;                      // Audio buffer pan (0.0f to 1.0f)

    bool playing;                   // Audio buffer state: AUDIO_PLAYING
    bool paused;                    // Audio buffer state: AUDIO_PAUSED
    bool looping;                   // Audio buffer looping, default to true for AudioStreams
    int usage;                      // Audio buffer usage mode: STATIC or STREAM

    bool isSubBufferProcessed[2];   // SubBuffer processed (virtual double buffer)
    unsigned int sizeInFrames;      // Total buffer size in frames
    unsigned int frameCursorPos;    // Frame cursor position
    unsigned int framesProcessed;   // Total frames processed in this buffer (required for play timing)

    unsigned char *data;            // Data buffer, on music stream keeps filling

    rAudioBuffer *next;             // Next audio buffer on the list
    rAudioBuffer *prev;             // Previous audio buffer on the list
};

// Audio processor struct
// NOTE: Useful to apply effects to an AudioBuffer
struct rAudioProcessor {
    AudioCallback process;          // Processor callback function
    rAudioProcessor *next;          // Next audio processor on the list
    rAudioProcessor *prev;          // Previous audio processor on the list
};

#define AudioBuffer rAudioBuffer    // HACK: To avoid CoreAudio (macOS) symbol collision

// Audio data context
typedef struct AudioData {
    struct {
        ma_context context;         // miniaudio context data
        ma_device device;           // miniaudio device
        ma_mutex lock;              // miniaudio mutex lock
        bool isReady;               // Check if audio device is ready
        size_t pcmBufferSize;       // Pre-allocated buffer size
        void *pcmBuffer;            // Pre-allocated buffer to read audio data from file/memory
    } System;
    struct {
        AudioBuffer *first;         // Pointer to first AudioBuffer in the list
        AudioBuffer *last;          // Pointer to last AudioBuffer in the list
        int defaultSize;            // Default audio buffer size for audio streams
    } Buffer;
    rAudioProcessor *mixedProcessor;
} AudioData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static AudioData AUDIO = {          // Global AUDIO context

    // NOTE: Music buffer size is defined by number of samples, independent of sample size and channels number
    // After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds and a
    // standard double-buffering system, a 4096 samples buffer has been chosen, it should be enough
    // In case of music-stalls, just increase this number
    .Buffer.defaultSize = 0,
    .mixedProcessor = NULL
};

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static void OnLog(void *pUserData, ma_uint32 level, const char *pMessage);

// Reads audio data from an AudioBuffer object in internal/device formats
static ma_uint32 ReadAudioBufferFramesInInternalFormat(AudioBuffer *audioBuffer, void *framesOut, ma_uint32 frameCount);
static ma_uint32 ReadAudioBufferFramesInMixingFormat(AudioBuffer *audioBuffer, float *framesOut, ma_uint32 frameCount);

static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount);
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, AudioBuffer *buffer);

static bool IsAudioBufferPlayingInLockedState(AudioBuffer *buffer);
static void StopAudioBufferInLockedState(AudioBuffer *buffer);
static void UpdateAudioStreamInLockedState(AudioStream stream, const void *data, int frameCount);

#if defined(RAUDIO_STANDALONE)
static bool IsFileExtension(const char *fileName, const char *ext); // Check file extension
static const char *GetFileExtension(const char *fileName);          // Get pointer to extension for a filename string (includes the dot: .png)
static const char *GetFileName(const char *filePath);               // Get pointer to filename for a path string
static const char *GetFileNameWithoutExt(const char *filePath);     // Get filename string without extension (uses static string)

static unsigned char *LoadFileData(const char *fileName, int *dataSize);    // Load file data as byte array (read)
static bool SaveFileData(const char *fileName, void *data, int dataSize);   // Save data to file from byte array (write)
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
void SetAudioBufferPan(AudioBuffer *buffer, float pan);
void TrackAudioBuffer(AudioBuffer *buffer);
void UntrackAudioBuffer(AudioBuffer *buffer);


//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------
// Initialize audio device
void InitAudioDevice(void)
{
    // Init audio context
    ma_context_config ctxConfig = ma_context_config_init();
    ma_log_callback_init(OnLog, NULL);

    ma_result result = ma_context_init(NULL, 0, &ctxConfig, &AUDIO.System.context);
    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to initialize context");
        return;
    }

    // Init audio device
    // NOTE: Using the default device. Format is floating point because it simplifies mixing
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;  // NULL for the default playback AUDIO.System.device
    config.playback.format = AUDIO_DEVICE_FORMAT;
    config.playback.channels = AUDIO_DEVICE_CHANNELS;
    config.capture.pDeviceID = NULL;  // NULL for the default capture AUDIO.System.device
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

    // Mixing happens on a separate thread which means we need to synchronize. I'm using a mutex here to make things simple, but may
    // want to look at something a bit smarter later on to keep everything real-time, if that's necessary
    if (ma_mutex_init(&AUDIO.System.lock) != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to create mutex for mixing");
        ma_device_uninit(&AUDIO.System.device);
        ma_context_uninit(&AUDIO.System.context);
        return;
    }

    // Keep the device running the whole time. May want to consider doing something a bit smarter and only have the device running
    // while there's at least one sound being played
    result = ma_device_start(&AUDIO.System.device);
    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to start playback device");
        ma_device_uninit(&AUDIO.System.device);
        ma_context_uninit(&AUDIO.System.context);
        return;
    }

    TRACELOG(LOG_INFO, "AUDIO: Device initialized successfully");
    TRACELOG(LOG_INFO, "    > Backend:       miniaudio | %s", ma_get_backend_name(AUDIO.System.context.backend));
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
        ma_mutex_uninit(&AUDIO.System.lock);
        ma_device_uninit(&AUDIO.System.device);
        ma_context_uninit(&AUDIO.System.context);

        AUDIO.System.isReady = false;
        RL_FREE(AUDIO.System.pcmBuffer);
        AUDIO.System.pcmBuffer = NULL;
        AUDIO.System.pcmBufferSize = 0;

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

// Get master volume (listener)
float GetMasterVolume(void)
{
    float volume = 0.0f;
    ma_device_get_master_volume(&AUDIO.System.device, &volume);
    return volume;
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
    converterConfig.allowDynamicSampleRate = true;

    ma_result result = ma_data_converter_init(&converterConfig, NULL, &audioBuffer->converter);

    if (result != MA_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "AUDIO: Failed to create data conversion pipeline");
        RL_FREE(audioBuffer);
        return NULL;
    }

    // Init audio buffer values
    audioBuffer->volume = 1.0f;
    audioBuffer->pitch = 1.0f;
    audioBuffer->pan = 0.5f;

    audioBuffer->callback = NULL;
    audioBuffer->processor = NULL;

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
        UntrackAudioBuffer(buffer);
        ma_data_converter_uninit(&buffer->converter, NULL);
        RL_FREE(buffer->data);
        RL_FREE(buffer);
    }
}

// Check if an audio buffer is playing from a program state without lock
bool IsAudioBufferPlaying(AudioBuffer *buffer)
{
    bool result = false;
    ma_mutex_lock(&AUDIO.System.lock);
    result = IsAudioBufferPlayingInLockedState(buffer);
    ma_mutex_unlock(&AUDIO.System.lock);
    return result;
}

// Play an audio buffer
// NOTE: Buffer is restarted to the start
// Use PauseAudioBuffer() and ResumeAudioBuffer() if the playback position should be maintained
void PlayAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        ma_mutex_lock(&AUDIO.System.lock);
        buffer->playing = true;
        buffer->paused = false;
        buffer->frameCursorPos = 0;
        ma_mutex_unlock(&AUDIO.System.lock);
    }
}

// Stop an audio buffer from a program state without lock
void StopAudioBuffer(AudioBuffer *buffer)
{
    ma_mutex_lock(&AUDIO.System.lock);
    StopAudioBufferInLockedState(buffer);
    ma_mutex_unlock(&AUDIO.System.lock);
}

// Pause an audio buffer
void PauseAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        ma_mutex_lock(&AUDIO.System.lock);
        buffer->paused = true;
        ma_mutex_unlock(&AUDIO.System.lock);
    }
}

// Resume an audio buffer
void ResumeAudioBuffer(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        ma_mutex_lock(&AUDIO.System.lock);
        buffer->paused = false;
        ma_mutex_unlock(&AUDIO.System.lock);
    }
}

// Set volume for an audio buffer
void SetAudioBufferVolume(AudioBuffer *buffer, float volume)
{
    if (buffer != NULL)
    {
        ma_mutex_lock(&AUDIO.System.lock);
        buffer->volume = volume;
        ma_mutex_unlock(&AUDIO.System.lock);
    }
}

// Set pitch for an audio buffer
void SetAudioBufferPitch(AudioBuffer *buffer, float pitch)
{
    if ((buffer != NULL) && (pitch > 0.0f))
    {
        ma_mutex_lock(&AUDIO.System.lock);
        // Pitching is just an adjustment of the sample rate
        // Note that this changes the duration of the sound:
        //  - higher pitches will make the sound faster
        //  - lower pitches make it slower
        ma_uint32 outputSampleRate = (ma_uint32)((float)buffer->converter.sampleRateOut/pitch);
        ma_data_converter_set_rate(&buffer->converter, buffer->converter.sampleRateIn, outputSampleRate);

        buffer->pitch = pitch;
        ma_mutex_unlock(&AUDIO.System.lock);
    }
}

// Set pan for an audio buffer
void SetAudioBufferPan(AudioBuffer *buffer, float pan)
{
    if (pan < 0.0f) pan = 0.0f;
    else if (pan > 1.0f) pan = 1.0f;

    if (buffer != NULL)
    {
        ma_mutex_lock(&AUDIO.System.lock);
        buffer->pan = pan;
        ma_mutex_unlock(&AUDIO.System.lock);
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
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);

    // Loading wave from memory data
    if (fileData != NULL) wave = LoadWaveFromMemory(GetFileExtension(fileName), fileData, dataSize);

    UnloadFileData(fileData);

    return wave;
}

// Load wave from memory buffer, fileType refers to extension: i.e. ".wav"
// WARNING: File extension must be provided in lower-case
Wave LoadWaveFromMemory(const char *fileType, const unsigned char *fileData, int dataSize)
{
    Wave wave = { 0 };

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if ((strcmp(fileType, ".wav") == 0) || (strcmp(fileType, ".WAV") == 0))
    {
        drwav wav = { 0 };
        bool success = drwav_init_memory(&wav, fileData, dataSize, NULL);

        if (success)
        {
            wave.frameCount = (unsigned int)wav.totalPCMFrameCount;
            wave.sampleRate = wav.sampleRate;
            wave.sampleSize = 16;
            wave.channels = wav.channels;
            wave.data = (short *)RL_MALLOC(wave.frameCount*wave.channels*sizeof(short));

            // NOTE: We are forcing conversion to 16bit sample size on reading
            drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, wave.data);
        }
        else TRACELOG(LOG_WARNING, "WAVE: Failed to load WAV data");

        drwav_uninit(&wav);
    }
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if ((strcmp(fileType, ".ogg") == 0) || (strcmp(fileType, ".OGG") == 0))
    {
        stb_vorbis *oggData = stb_vorbis_open_memory((unsigned char *)fileData, dataSize, NULL, NULL);

        if (oggData != NULL)
        {
            stb_vorbis_info info = stb_vorbis_get_info(oggData);

            wave.sampleRate = info.sample_rate;
            wave.sampleSize = 16;       // By default, ogg data is 16 bit per sample (short)
            wave.channels = info.channels;
            wave.frameCount = (unsigned int)stb_vorbis_stream_length_in_samples(oggData);  // NOTE: It returns frames!
            wave.data = (short *)RL_MALLOC(wave.frameCount*wave.channels*sizeof(short));

            // NOTE: Get the number of samples to process (be careful! we ask for number of shorts, not bytes!)
            stb_vorbis_get_samples_short_interleaved(oggData, info.channels, (short *)wave.data, wave.frameCount*wave.channels);
            stb_vorbis_close(oggData);
        }
        else TRACELOG(LOG_WARNING, "WAVE: Failed to load OGG data");
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if ((strcmp(fileType, ".mp3") == 0) || (strcmp(fileType, ".MP3") == 0))
    {
        drmp3_config config = { 0 };
        unsigned long long int totalFrameCount = 0;

        // NOTE: We are forcing conversion to 32bit float sample size on reading
        wave.data = drmp3_open_memory_and_read_pcm_frames_f32(fileData, dataSize, &config, &totalFrameCount, NULL);
        wave.sampleSize = 32;

        if (wave.data != NULL)
        {
            wave.channels = config.channels;
            wave.sampleRate = config.sampleRate;
            wave.frameCount = (int)totalFrameCount;
        }
        else TRACELOG(LOG_WARNING, "WAVE: Failed to load MP3 data");

    }
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
    else if ((strcmp(fileType, ".qoa") == 0) || (strcmp(fileType, ".QOA") == 0))
    {
        qoa_desc qoa = { 0 };

        // NOTE: Returned sample data is always 16 bit?
        wave.data = qoa_decode(fileData, dataSize, &qoa);
        wave.sampleSize = 16;

        if (wave.data != NULL)
        {
            wave.channels = qoa.channels;
            wave.sampleRate = qoa.samplerate;
            wave.frameCount = qoa.samples;
        }
        else TRACELOG(LOG_WARNING, "WAVE: Failed to load QOA data");

    }
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if ((strcmp(fileType, ".flac") == 0) || (strcmp(fileType, ".FLAC") == 0))
    {
        unsigned long long int totalFrameCount = 0;

        // NOTE: We are forcing conversion to 16bit sample size on reading
        wave.data = drflac_open_memory_and_read_pcm_frames_s16(fileData, dataSize, &wave.channels, &wave.sampleRate, &totalFrameCount, NULL);
        wave.sampleSize = 16;

        if (wave.data != NULL) wave.frameCount = (unsigned int)totalFrameCount;
        else TRACELOG(LOG_WARNING, "WAVE: Failed to load FLAC data");
    }
#endif
    else TRACELOG(LOG_WARNING, "WAVE: Data format not supported");

    TRACELOG(LOG_INFO, "WAVE: Data loaded successfully (%i Hz, %i bit, %i channels)", wave.sampleRate, wave.sampleSize, wave.channels);

    return wave;
}

// Checks if wave data is ready
bool IsWaveReady(Wave wave)
{
    bool result = false;

    if ((wave.data != NULL) &&      // Validate wave data available
        (wave.frameCount > 0) &&    // Validate frame count
        (wave.sampleRate > 0) &&    // Validate sample rate is supported
        (wave.sampleSize > 0) &&    // Validate sample size is supported
        (wave.channels > 0)) result = true; // Validate number of channels supported

    return result;
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
        // When using miniaudio we need to do our own mixing
        // To simplify this we need convert the format of each sound to be consistent with
        // the format used to open the playback AUDIO.System.device. We can do this two ways:
        //
        //   1) Convert the whole sound in one go at load time (here)
        //   2) Convert the audio data in chunks at mixing time
        //
        // First option has been selected, format conversion is done on the loading stage
        // The downside is that it uses more memory if the original sound is u8 or s16
        ma_format formatIn = ((wave.sampleSize == 8)? ma_format_u8 : ((wave.sampleSize == 16)? ma_format_s16 : ma_format_f32));
        ma_uint32 frameCountIn = wave.frameCount;

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

        sound.frameCount = frameCount;
        sound.stream.sampleRate = AUDIO.System.device.sampleRate;
        sound.stream.sampleSize = 32;
        sound.stream.channels = AUDIO_DEVICE_CHANNELS;
        sound.stream.buffer = audioBuffer;
    }

    return sound;
}

// Clone sound from existing sound data, clone does not own wave data
// NOTE: Wave data must be unallocated manually and will be shared across all clones
Sound LoadSoundAlias(Sound source)
{
    Sound sound = { 0 };

    if (source.stream.buffer->data != NULL)
    {
        AudioBuffer *audioBuffer = LoadAudioBuffer(AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, AUDIO.System.device.sampleRate, 0, AUDIO_BUFFER_USAGE_STATIC);

        if (audioBuffer == NULL)
        {
            TRACELOG(LOG_WARNING, "SOUND: Failed to create buffer");
            return sound; // Early return to avoid dereferencing the audioBuffer null pointer
        }

        audioBuffer->sizeInFrames = source.stream.buffer->sizeInFrames;
        audioBuffer->volume = source.stream.buffer->volume;
        audioBuffer->data = source.stream.buffer->data;

        sound.frameCount = source.frameCount;
        sound.stream.sampleRate = AUDIO.System.device.sampleRate;
        sound.stream.sampleSize = 32;
        sound.stream.channels = AUDIO_DEVICE_CHANNELS;
        sound.stream.buffer = audioBuffer;
    }

    return sound;
}


// Checks if a sound is ready
bool IsSoundReady(Sound sound)
{
    bool result = false;

    if ((sound.frameCount > 0) &&           // Validate frame count
        (sound.stream.buffer != NULL) &&    // Validate stream buffer
        (sound.stream.sampleRate > 0) &&    // Validate sample rate is supported
        (sound.stream.sampleSize > 0) &&    // Validate sample size is supported
        (sound.stream.channels > 0)) result = true; // Validate number of channels supported

    return result;
}

// Unload wave data
void UnloadWave(Wave wave)
{
    RL_FREE(wave.data);
    //TRACELOG(LOG_INFO, "WAVE: Unloaded wave data from RAM");
}

// Unload sound
void UnloadSound(Sound sound)
{
    UnloadAudioBuffer(sound.stream.buffer);
    //TRACELOG(LOG_INFO, "SOUND: Unloaded sound data from RAM");
}

void UnloadSoundAlias(Sound alias)
{
    // Untrack and unload just the sound buffer, not the sample data, it is shared with the source for the alias
    if (alias.stream.buffer != NULL)
    {
        UntrackAudioBuffer(alias.stream.buffer);
        ma_data_converter_uninit(&alias.stream.buffer->converter, NULL);
        RL_FREE(alias.stream.buffer);
    }
}

// Update sound buffer with new data
void UpdateSound(Sound sound, const void *data, int frameCount)
{
    if (sound.stream.buffer != NULL)
    {
        StopAudioBuffer(sound.stream.buffer);

        memcpy(sound.stream.buffer->data, data, frameCount*ma_get_bytes_per_frame(sound.stream.buffer->converter.formatIn, sound.stream.buffer->converter.channelsIn));
    }
}

// Export wave data to file
bool ExportWave(Wave wave, const char *fileName)
{
    bool success = false;

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if (IsFileExtension(fileName, ".wav"))
    {
        drwav wav = { 0 };
        drwav_data_format format = { 0 };
        format.container = drwav_container_riff;
        if (wave.sampleSize == 32) format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
        else format.format = DR_WAVE_FORMAT_PCM;
        format.channels = wave.channels;
        format.sampleRate = wave.sampleRate;
        format.bitsPerSample = wave.sampleSize;

        void *fileData = NULL;
        size_t fileDataSize = 0;
        success = drwav_init_memory_write(&wav, &fileData, &fileDataSize, &format, NULL);
        if (success) success = (int)drwav_write_pcm_frames(&wav, wave.frameCount, wave.data);
        drwav_result result = drwav_uninit(&wav);

        if (result == DRWAV_SUCCESS) success = SaveFileData(fileName, (unsigned char *)fileData, (unsigned int)fileDataSize);

        drwav_free(fileData, NULL);
    }
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
    else if (IsFileExtension(fileName, ".qoa"))
    {
        if (wave.sampleSize == 16)
        {
            qoa_desc qoa = { 0 };
            qoa.channels = wave.channels;
            qoa.samplerate = wave.sampleRate;
            qoa.samples = wave.frameCount;

            int bytesWritten = qoa_write(fileName, wave.data, &qoa);
            if (bytesWritten > 0) success = true;
        }
        else TRACELOG(LOG_WARNING, "AUDIO: Wave data must be 16 bit per sample for QOA format export");
    }
#endif
    else if (IsFileExtension(fileName, ".raw"))
    {
        // Export raw sample data (without header)
        // NOTE: It's up to the user to track wave parameters
        success = SaveFileData(fileName, wave.data, wave.frameCount*wave.channels*wave.sampleSize/8);
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

    int waveDataSize = wave.frameCount*wave.channels*wave.sampleSize/8;

    // NOTE: Text data buffer size is estimated considering wave data size in bytes
    // and requiring 12 char bytes for every byte; the actual size varies, but
    // the longest possible char being appended is "%.4ff,\n    ", which is 12 bytes.
    char *txtData = (char *)RL_CALLOC(waveDataSize*12 + 2000, sizeof(char));

    int byteCount = 0;
    byteCount += sprintf(txtData + byteCount, "\n//////////////////////////////////////////////////////////////////////////////////\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                              //\n");
    byteCount += sprintf(txtData + byteCount, "// WaveAsCode exporter v1.1 - Wave data exported as an array of bytes           //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                              //\n");
    byteCount += sprintf(txtData + byteCount, "// more info and bugs-report:  github.com/raysan5/raylib                        //\n");
    byteCount += sprintf(txtData + byteCount, "// feedback and support:       ray[at]raylib.com                                //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                              //\n");
    byteCount += sprintf(txtData + byteCount, "// Copyright (c) 2018-2024 Ramon Santamaria (@raysan5)                          //\n");
    byteCount += sprintf(txtData + byteCount, "//                                                                              //\n");
    byteCount += sprintf(txtData + byteCount, "//////////////////////////////////////////////////////////////////////////////////\n\n");

    // Get file name from path and convert variable name to uppercase
    char varFileName[256] = { 0 };
    strcpy(varFileName, GetFileNameWithoutExt(fileName));
    for (int i = 0; varFileName[i] != '\0'; i++) if (varFileName[i] >= 'a' && varFileName[i] <= 'z') { varFileName[i] = varFileName[i] - 32; }

    // Add wave information
    byteCount += sprintf(txtData + byteCount, "// Wave data information\n");
    byteCount += sprintf(txtData + byteCount, "#define %s_FRAME_COUNT      %u\n", varFileName, wave.frameCount);
    byteCount += sprintf(txtData + byteCount, "#define %s_SAMPLE_RATE      %u\n", varFileName, wave.sampleRate);
    byteCount += sprintf(txtData + byteCount, "#define %s_SAMPLE_SIZE      %u\n", varFileName, wave.sampleSize);
    byteCount += sprintf(txtData + byteCount, "#define %s_CHANNELS         %u\n\n", varFileName, wave.channels);

    // Write wave data as an array of values
    // Wave data is exported as byte array for 8/16bit and float array for 32bit float data
    // NOTE: Frame data exported is channel-interlaced: frame01[sampleChannel1, sampleChannel2, ...], frame02[], frame03[]
    if (wave.sampleSize == 32)
    {
        byteCount += sprintf(txtData + byteCount, "static float %s_DATA[%i] = {\n", varFileName, waveDataSize/4);
        for (int i = 1; i < waveDataSize/4; i++) byteCount += sprintf(txtData + byteCount, ((i%TEXT_BYTES_PER_LINE == 0)? "%.4ff,\n    " : "%.4ff, "), ((float *)wave.data)[i - 1]);
        byteCount += sprintf(txtData + byteCount, "%.4ff };\n", ((float *)wave.data)[waveDataSize/4 - 1]);
    }
    else
    {
        byteCount += sprintf(txtData + byteCount, "static unsigned char %s_DATA[%i] = { ", varFileName, waveDataSize);
        for (int i = 1; i < waveDataSize; i++) byteCount += sprintf(txtData + byteCount, ((i%TEXT_BYTES_PER_LINE == 0)? "0x%x,\n    " : "0x%x, "), ((unsigned char *)wave.data)[i - 1]);
        byteCount += sprintf(txtData + byteCount, "0x%x };\n", ((unsigned char *)wave.data)[waveDataSize - 1]);
    }

    // NOTE: Text data length exported is determined by '\0' (NULL) character
    success = SaveFileText(fileName, txtData);

    RL_FREE(txtData);

    if (success != 0) TRACELOG(LOG_INFO, "FILEIO: [%s] Wave as code exported successfully", fileName);
    else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to export wave as code", fileName);

    return success;
}

// Play a sound
void PlaySound(Sound sound)
{
    PlayAudioBuffer(sound.stream.buffer);
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
    bool result = false;

    if (IsAudioBufferPlaying(sound.stream.buffer)) result = true;

    return result;
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

// Set pan for a sound
void SetSoundPan(Sound sound, float pan)
{
    SetAudioBufferPan(sound.stream.buffer, pan);
}

// Convert wave data to desired format
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
{
    ma_format formatIn = ((wave->sampleSize == 8)? ma_format_u8 : ((wave->sampleSize == 16)? ma_format_s16 : ma_format_f32));
    ma_format formatOut = ((sampleSize == 8)? ma_format_u8 : ((sampleSize == 16)? ma_format_s16 : ma_format_f32));

    ma_uint32 frameCountIn = wave->frameCount;
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

    wave->frameCount = frameCount;
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

    newWave.data = RL_MALLOC(wave.frameCount*wave.channels*wave.sampleSize/8);

    if (newWave.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newWave.data, wave.data, wave.frameCount*wave.channels*wave.sampleSize/8);

        newWave.frameCount = wave.frameCount;
        newWave.sampleRate = wave.sampleRate;
        newWave.sampleSize = wave.sampleSize;
        newWave.channels = wave.channels;
    }

    return newWave;
}

// Crop a wave to defined frames range
// NOTE: Security check in case of out-of-range
void WaveCrop(Wave *wave, int initFrame, int finalFrame)
{
    if ((initFrame >= 0) && (initFrame < finalFrame) && ((unsigned int)finalFrame <= wave->frameCount))
    {
        int frameCount = finalFrame - initFrame;

        void *data = RL_MALLOC(frameCount*wave->channels*wave->sampleSize/8);

        memcpy(data, (unsigned char *)wave->data + (initFrame*wave->channels*wave->sampleSize/8), frameCount*wave->channels*wave->sampleSize/8);

        RL_FREE(wave->data);
        wave->data = data;
        wave->frameCount = (unsigned int)frameCount;
    }
    else TRACELOG(LOG_WARNING, "WAVE: Crop range out of bounds");
}

// Load samples data from wave as a floats array
// NOTE 1: Returned sample values are normalized to range [-1..1]
// NOTE 2: Sample data allocated should be freed with UnloadWaveSamples()
float *LoadWaveSamples(Wave wave)
{
    float *samples = (float *)RL_MALLOC(wave.frameCount*wave.channels*sizeof(float));

    // NOTE: sampleCount is the total number of interlaced samples (including channels)

    for (unsigned int i = 0; i < wave.frameCount*wave.channels; i++)
    {
        if (wave.sampleSize == 8) samples[i] = (float)(((unsigned char *)wave.data)[i] - 128)/128.0f;
        else if (wave.sampleSize == 16) samples[i] = (float)(((short *)wave.data)[i])/32768.0f;
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
// Module Functions Definition - Music loading and stream playing
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

        if (success)
        {
            music.ctxType = MUSIC_AUDIO_WAV;
            music.ctxData = ctxWav;
            int sampleSize = ctxWav->bitsPerSample;
            if (ctxWav->bitsPerSample == 24) sampleSize = 16;   // Forcing conversion to s16 on UpdateMusicStream()

            music.stream = LoadAudioStream(ctxWav->sampleRate, sampleSize, ctxWav->channels);
            music.frameCount = (unsigned int)ctxWav->totalPCMFrameCount;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            RL_FREE(ctxWav);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if (IsFileExtension(fileName, ".ogg"))
    {
        // Open ogg audio stream
        stb_vorbis *ctxOgg = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (ctxOgg != NULL)
        {
            music.ctxType = MUSIC_AUDIO_OGG;
            music.ctxData = ctxOgg;
            stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis *)music.ctxData);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music.stream = LoadAudioStream(info.sample_rate, 16, info.channels);

            // WARNING: It seems this function returns length in frames, not samples, so we multiply by channels
            music.frameCount = (unsigned int)stb_vorbis_stream_length_in_samples((stb_vorbis *)music.ctxData);
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            stb_vorbis_close(ctxOgg);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if (IsFileExtension(fileName, ".mp3"))
    {
        drmp3 *ctxMp3 = RL_CALLOC(1, sizeof(drmp3));
        int result = drmp3_init_file(ctxMp3, fileName, NULL);

        if (result > 0)
        {
            music.ctxType = MUSIC_AUDIO_MP3;
            music.ctxData = ctxMp3;
            music.stream = LoadAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
            music.frameCount = (unsigned int)drmp3_get_pcm_frame_count(ctxMp3);
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            RL_FREE(ctxMp3);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
    else if (IsFileExtension(fileName, ".qoa"))
    {
        qoaplay_desc *ctxQoa = qoaplay_open(fileName);

        if (ctxQoa != NULL)
        {
            music.ctxType = MUSIC_AUDIO_QOA;
            music.ctxData = ctxQoa;
            // NOTE: We are loading samples are 32bit float normalized data, so,
            // we configure the output audio stream to also use float 32bit
            music.stream = LoadAudioStream(ctxQoa->info.samplerate, 32, ctxQoa->info.channels);
            music.frameCount = ctxQoa->info.samples;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else{} //No uninit required
    }
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (IsFileExtension(fileName, ".flac"))
    {
        drflac *ctxFlac = drflac_open_file(fileName, NULL);

        if (ctxFlac != NULL)
        {
            music.ctxType = MUSIC_AUDIO_FLAC;
            music.ctxData = ctxFlac;
            int sampleSize = ctxFlac->bitsPerSample;
            if (ctxFlac->bitsPerSample == 24) sampleSize = 16;   // Forcing conversion to s16 on UpdateMusicStream()
            music.stream = LoadAudioStream(ctxFlac->sampleRate, sampleSize, ctxFlac->channels);
            music.frameCount = (unsigned int)ctxFlac->totalPCMFrameCount;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            drflac_free(ctxFlac, NULL);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if (IsFileExtension(fileName, ".xm"))
    {
        jar_xm_context_t *ctxXm = NULL;
        int result = jar_xm_create_context_from_file(&ctxXm, AUDIO.System.device.sampleRate, fileName);

        if (result == 0)    // XM AUDIO.System.context created successfully
        {
            music.ctxType = MUSIC_MODULE_XM;
            music.ctxData = ctxXm;
            jar_xm_set_max_loop_count(ctxXm, 0);    // Set infinite number of loops

            unsigned int bits = 32;
            if (AUDIO_DEVICE_FORMAT == ma_format_s16) bits = 16;
            else if (AUDIO_DEVICE_FORMAT == ma_format_u8) bits = 8;

            // NOTE: Only stereo is supported for XM
            music.stream = LoadAudioStream(AUDIO.System.device.sampleRate, bits, AUDIO_DEVICE_CHANNELS);
            music.frameCount = (unsigned int)jar_xm_get_remaining_samples(ctxXm);    // NOTE: Always 2 channels (stereo)
            music.looping = true;   // Looping enabled by default
            jar_xm_reset(ctxXm);    // Make sure we start at the beginning of the song
            musicLoaded = true;
        }
        else
        {
            jar_xm_free_context(ctxXm);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if (IsFileExtension(fileName, ".mod"))
    {
        jar_mod_context_t *ctxMod = RL_CALLOC(1, sizeof(jar_mod_context_t));
        jar_mod_init(ctxMod);
        int result = jar_mod_load_file(ctxMod, fileName);

        if (result > 0)
        {
            music.ctxType = MUSIC_MODULE_MOD;
            music.ctxData = ctxMod;
            // NOTE: Only stereo is supported for MOD
            music.stream = LoadAudioStream(AUDIO.System.device.sampleRate, 16, AUDIO_DEVICE_CHANNELS);
            music.frameCount = (unsigned int)jar_mod_max_samples(ctxMod);    // NOTE: Always 2 channels (stereo)
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            jar_mod_unload(ctxMod);
            RL_FREE(ctxMod);
        }
    }
#endif
    else TRACELOG(LOG_WARNING, "STREAM: [%s] File format not supported", fileName);

    if (!musicLoaded)
    {
        TRACELOG(LOG_WARNING, "FILEIO: [%s] Music file could not be opened", fileName);
    }
    else
    {
        // Show some music stream info
        TRACELOG(LOG_INFO, "FILEIO: [%s] Music file loaded successfully", fileName);
        TRACELOG(LOG_INFO, "    > Sample rate:   %i Hz", music.stream.sampleRate);
        TRACELOG(LOG_INFO, "    > Sample size:   %i bits", music.stream.sampleSize);
        TRACELOG(LOG_INFO, "    > Channels:      %i (%s)", music.stream.channels, (music.stream.channels == 1)? "Mono" : (music.stream.channels == 2)? "Stereo" : "Multi");
        TRACELOG(LOG_INFO, "    > Total frames:  %i", music.frameCount);
    }

    return music;
}

// Load music stream from memory buffer, fileType refers to extension: i.e. ".wav"
// WARNING: File extension must be provided in lower-case
Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize)
{
    Music music = { 0 };
    bool musicLoaded = false;

    if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
    else if ((strcmp(fileType, ".wav") == 0) || (strcmp(fileType, ".WAV") == 0))
    {
        drwav *ctxWav = RL_CALLOC(1, sizeof(drwav));

        bool success = drwav_init_memory(ctxWav, (const void *)data, dataSize, NULL);

        if (success)
        {
            music.ctxType = MUSIC_AUDIO_WAV;
            music.ctxData = ctxWav;
            int sampleSize = ctxWav->bitsPerSample;
            if (ctxWav->bitsPerSample == 24) sampleSize = 16;   // Forcing conversion to s16 on UpdateMusicStream()

            music.stream = LoadAudioStream(ctxWav->sampleRate, sampleSize, ctxWav->channels);
            music.frameCount = (unsigned int)ctxWav->totalPCMFrameCount;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else {
            drwav_uninit(ctxWav);
            RL_FREE(ctxWav);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
    else if ((strcmp(fileType, ".ogg") == 0) || (strcmp(fileType, ".OGG") == 0))
    {
        // Open ogg audio stream
        stb_vorbis* ctxOgg = stb_vorbis_open_memory((const unsigned char *)data, dataSize, NULL, NULL);

        if (ctxOgg != NULL)
        {
            music.ctxType = MUSIC_AUDIO_OGG;
            music.ctxData = ctxOgg;
            stb_vorbis_info info = stb_vorbis_get_info((stb_vorbis *)music.ctxData);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music.stream = LoadAudioStream(info.sample_rate, 16, info.channels);

            // WARNING: It seems this function returns length in frames, not samples, so we multiply by channels
            music.frameCount = (unsigned int)stb_vorbis_stream_length_in_samples((stb_vorbis *)music.ctxData);
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            stb_vorbis_close(ctxOgg);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
    else if ((strcmp(fileType, ".mp3") == 0) || (strcmp(fileType, ".MP3") == 0))
    {
        drmp3 *ctxMp3 = RL_CALLOC(1, sizeof(drmp3));
        int success = drmp3_init_memory(ctxMp3, (const void*)data, dataSize, NULL);

        if (success)
        {
            music.ctxType = MUSIC_AUDIO_MP3;
            music.ctxData = ctxMp3;
            music.stream = LoadAudioStream(ctxMp3->sampleRate, 32, ctxMp3->channels);
            music.frameCount = (unsigned int)drmp3_get_pcm_frame_count(ctxMp3);
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            drmp3_uninit(ctxMp3);
            RL_FREE(ctxMp3);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
    else if ((strcmp(fileType, ".qoa") == 0) || (strcmp(fileType, ".QOA") == 0))
    {
        qoaplay_desc *ctxQoa = NULL;
        if ((data != NULL) && (dataSize > 0))
        {
            ctxQoa = qoaplay_open_memory(data, dataSize);
        }

        if (ctxQoa != NULL)
        {
            music.ctxType = MUSIC_AUDIO_QOA;
            music.ctxData = ctxQoa;
            // NOTE: We are loading samples are 32bit float normalized data, so,
            // we configure the output audio stream to also use float 32bit
            music.stream = LoadAudioStream(ctxQoa->info.samplerate, 32, ctxQoa->info.channels);
            music.frameCount = ctxQoa->info.samples;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else{} //No uninit required
    }
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if ((strcmp(fileType, ".flac") == 0) || (strcmp(fileType, ".FLAC") == 0))
    {
        drflac *ctxFlac = drflac_open_memory((const void*)data, dataSize, NULL);

        if (ctxFlac != NULL)
        {
            music.ctxType = MUSIC_AUDIO_FLAC;
            music.ctxData = ctxFlac;
            music.stream = LoadAudioStream(ctxFlac->sampleRate, ctxFlac->bitsPerSample, ctxFlac->channels);
            music.frameCount = (unsigned int)ctxFlac->totalPCMFrameCount;
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            drflac_free(ctxFlac, NULL);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
    else if ((strcmp(fileType, ".xm") == 0) || (strcmp(fileType, ".XM") == 0))
    {
        jar_xm_context_t *ctxXm = NULL;
        int result = jar_xm_create_context_safe(&ctxXm, (const char *)data, dataSize, AUDIO.System.device.sampleRate);
        if (result == 0)    // XM AUDIO.System.context created successfully
        {
            music.ctxType = MUSIC_MODULE_XM;
            music.ctxData = ctxXm;
            jar_xm_set_max_loop_count(ctxXm, 0);    // Set infinite number of loops

            unsigned int bits = 32;
            if (AUDIO_DEVICE_FORMAT == ma_format_s16) bits = 16;
            else if (AUDIO_DEVICE_FORMAT == ma_format_u8) bits = 8;

            // NOTE: Only stereo is supported for XM
            music.stream = LoadAudioStream(AUDIO.System.device.sampleRate, bits, 2);
            music.frameCount = (unsigned int)jar_xm_get_remaining_samples(ctxXm);    // NOTE: Always 2 channels (stereo)
            music.looping = true;   // Looping enabled by default
            jar_xm_reset(ctxXm);    // Make sure we start at the beginning of the song

            musicLoaded = true;
        }
        else
        {
            jar_xm_free_context(ctxXm);
        }
    }
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
    else if ((strcmp(fileType, ".mod") == 0) || (strcmp(fileType, ".MOD") == 0))
    {
        jar_mod_context_t *ctxMod = (jar_mod_context_t *)RL_MALLOC(sizeof(jar_mod_context_t));
        int result = 0;

        jar_mod_init(ctxMod);

        // Copy data to allocated memory for default UnloadMusicStream
        unsigned char *newData = (unsigned char *)RL_MALLOC(dataSize);
        int it = dataSize/sizeof(unsigned char);
        for (int i = 0; i < it; i++) newData[i] = data[i];

        // Memory loaded version for jar_mod_load_file()
        if (dataSize && (dataSize < 32*1024*1024))
        {
            ctxMod->modfilesize = dataSize;
            ctxMod->modfile = newData;
            if (jar_mod_load(ctxMod, (void *)ctxMod->modfile, dataSize)) result = dataSize;
        }

        if (result > 0)
        {
            music.ctxType = MUSIC_MODULE_MOD;
            music.ctxData = ctxMod;

            // NOTE: Only stereo is supported for MOD
            music.stream = LoadAudioStream(AUDIO.System.device.sampleRate, 16, 2);
            music.frameCount = (unsigned int)jar_mod_max_samples(ctxMod);    // NOTE: Always 2 channels (stereo)
            music.looping = true;   // Looping enabled by default
            musicLoaded = true;
        }
        else
        {
            jar_mod_unload(ctxMod);
            RL_FREE(ctxMod);
        }
    }
#endif
    else TRACELOG(LOG_WARNING, "STREAM: Data format not supported");

    if (!musicLoaded)
    {
        TRACELOG(LOG_WARNING, "FILEIO: Music data could not be loaded");
    }
    else
    {
        // Show some music stream info
        TRACELOG(LOG_INFO, "FILEIO: Music data loaded successfully");
        TRACELOG(LOG_INFO, "    > Sample rate:   %i Hz", music.stream.sampleRate);
        TRACELOG(LOG_INFO, "    > Sample size:   %i bits", music.stream.sampleSize);
        TRACELOG(LOG_INFO, "    > Channels:      %i (%s)", music.stream.channels, (music.stream.channels == 1)? "Mono" : (music.stream.channels == 2)? "Stereo" : "Multi");
        TRACELOG(LOG_INFO, "    > Total frames:  %i", music.frameCount);
    }

    return music;
}

// Checks if a music stream is ready
bool IsMusicReady(Music music)
{
    return ((music.ctxData != NULL) &&          // Validate context loaded
            (music.frameCount > 0) &&           // Validate audio frame count
            (music.stream.sampleRate > 0) &&    // Validate sample rate is supported
            (music.stream.sampleSize > 0) &&    // Validate sample size is supported
            (music.stream.channels > 0));       // Validate number of channels supported
}

// Unload music stream
void UnloadMusicStream(Music music)
{
    UnloadAudioStream(music.stream);

    if (music.ctxData != NULL)
    {
        if (false) { }
#if defined(SUPPORT_FILEFORMAT_WAV)
        else if (music.ctxType == MUSIC_AUDIO_WAV) drwav_uninit((drwav *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
        else if (music.ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close((stb_vorbis *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
        else if (music.ctxType == MUSIC_AUDIO_MP3) { drmp3_uninit((drmp3 *)music.ctxData); RL_FREE(music.ctxData); }
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
        else if (music.ctxType == MUSIC_AUDIO_QOA) qoaplay_close((qoaplay_desc *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
        else if (music.ctxType == MUSIC_AUDIO_FLAC) drflac_free((drflac *)music.ctxData, NULL);
#endif
#if defined(SUPPORT_FILEFORMAT_XM)
        else if (music.ctxType == MUSIC_MODULE_XM) jar_xm_free_context((jar_xm_context_t *)music.ctxData);
#endif
#if defined(SUPPORT_FILEFORMAT_MOD)
        else if (music.ctxType == MUSIC_MODULE_MOD) { jar_mod_unload((jar_mod_context_t *)music.ctxData); RL_FREE(music.ctxData); }
#endif
    }
}

// Start music playing (open stream) from beginning
void PlayMusicStream(Music music)
{
    PlayAudioStream(music.stream);
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
        case MUSIC_AUDIO_WAV: drwav_seek_to_first_pcm_frame((drwav *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_start((stb_vorbis *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
        case MUSIC_AUDIO_MP3: drmp3_seek_to_start_of_stream((drmp3 *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
        case MUSIC_AUDIO_QOA: qoaplay_rewind((qoaplay_desc *)music.ctxData); break;
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
        case MUSIC_AUDIO_FLAC: drflac__seek_to_first_frame((drflac *)music.ctxData); break;
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

// Seek music to a certain position (in seconds)
void SeekMusicStream(Music music, float position)
{
    // Seeking is not supported in module formats
    if ((music.ctxType == MUSIC_MODULE_XM) || (music.ctxType == MUSIC_MODULE_MOD)) return;

    unsigned int positionInFrames = (unsigned int)(position*music.stream.sampleRate);

    switch (music.ctxType)
    {
#if defined(SUPPORT_FILEFORMAT_WAV)
        case MUSIC_AUDIO_WAV: drwav_seek_to_pcm_frame((drwav *)music.ctxData, positionInFrames); break;
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_frame((stb_vorbis *)music.ctxData, positionInFrames); break;
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
        case MUSIC_AUDIO_MP3: drmp3_seek_to_pcm_frame((drmp3 *)music.ctxData, positionInFrames); break;
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
        case MUSIC_AUDIO_QOA:
        {
            int qoaFrame = positionInFrames/QOA_FRAME_LEN;
            qoaplay_seek_frame((qoaplay_desc *)music.ctxData, qoaFrame); // Seeks to QOA frame, not PCM frame

            // We need to compute QOA frame number and update positionInFrames
            positionInFrames = ((qoaplay_desc *)music.ctxData)->sample_position;
        } break;
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
        case MUSIC_AUDIO_FLAC: drflac_seek_to_pcm_frame((drflac *)music.ctxData, positionInFrames); break;
#endif
        default: break;
    }

    ma_mutex_lock(&AUDIO.System.lock);
    music.stream.buffer->framesProcessed = positionInFrames;
    ma_mutex_unlock(&AUDIO.System.lock);
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(Music music)
{
    if (music.stream.buffer == NULL) return;

    ma_mutex_lock(&AUDIO.System.lock);

    unsigned int subBufferSizeInFrames = music.stream.buffer->sizeInFrames/2;

    // On first call of this function we lazily pre-allocated a temp buffer to read audio files/memory data in
    int frameSize = music.stream.channels*music.stream.sampleSize/8;
    unsigned int pcmSize = subBufferSizeInFrames*frameSize;

    if (AUDIO.System.pcmBufferSize < pcmSize)
    {
        RL_FREE(AUDIO.System.pcmBuffer);
        AUDIO.System.pcmBuffer = RL_CALLOC(1, pcmSize);
        AUDIO.System.pcmBufferSize = pcmSize;
    }

    // Check both sub-buffers to check if they require refilling
    for (int i = 0; i < 2; i++)
    {
        if (!music.stream.buffer->isSubBufferProcessed[i]) continue; // No refilling required, move to next sub-buffer

        unsigned int framesLeft = music.frameCount - music.stream.buffer->framesProcessed;  // Frames left to be processed
        unsigned int framesToStream = 0;                 // Total frames to be streamed

        if ((framesLeft >= subBufferSizeInFrames) || music.looping) framesToStream = subBufferSizeInFrames;
        else framesToStream = framesLeft;

        int frameCountStillNeeded = framesToStream;
        int frameCountReadTotal = 0;

        switch (music.ctxType)
        {
        #if defined(SUPPORT_FILEFORMAT_WAV)
            case MUSIC_AUDIO_WAV:
            {
                if (music.stream.sampleSize == 16)
                {
                    while (true)
                    {
                        int frameCountRead = (int)drwav_read_pcm_frames_s16((drwav *)music.ctxData, frameCountStillNeeded, (short *)((char *)AUDIO.System.pcmBuffer + frameCountReadTotal*frameSize));
                        frameCountReadTotal += frameCountRead;
                        frameCountStillNeeded -= frameCountRead;
                        if (frameCountStillNeeded == 0) break;
                        else drwav_seek_to_first_pcm_frame((drwav *)music.ctxData);
                    }
                }
                else if (music.stream.sampleSize == 32)
                {
                    while (true)
                    {
                        int frameCountRead = (int)drwav_read_pcm_frames_f32((drwav *)music.ctxData, frameCountStillNeeded, (float *)((char *)AUDIO.System.pcmBuffer + frameCountReadTotal*frameSize));
                        frameCountReadTotal += frameCountRead;
                        frameCountStillNeeded -= frameCountRead;
                        if (frameCountStillNeeded == 0) break;
                        else drwav_seek_to_first_pcm_frame((drwav *)music.ctxData);
                    }
                }
            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_OGG)
            case MUSIC_AUDIO_OGG:
            {
                while (true)
                {
                    int frameCountRead = stb_vorbis_get_samples_short_interleaved((stb_vorbis *)music.ctxData, music.stream.channels, (short *)((char *)AUDIO.System.pcmBuffer + frameCountReadTotal*frameSize), frameCountStillNeeded*music.stream.channels);
                    frameCountReadTotal += frameCountRead;
                    frameCountStillNeeded -= frameCountRead;
                    if (frameCountStillNeeded == 0) break;
                    else stb_vorbis_seek_start((stb_vorbis *)music.ctxData);
                }
            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MP3)
            case MUSIC_AUDIO_MP3:
            {
                while (true)
                {
                    int frameCountRead = (int)drmp3_read_pcm_frames_f32((drmp3 *)music.ctxData, frameCountStillNeeded, (float *)((char *)AUDIO.System.pcmBuffer + frameCountReadTotal*frameSize));
                    frameCountReadTotal += frameCountRead;
                    frameCountStillNeeded -= frameCountRead;
                    if (frameCountStillNeeded == 0) break;
                    else drmp3_seek_to_start_of_stream((drmp3 *)music.ctxData);
                }
            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_QOA)
            case MUSIC_AUDIO_QOA:
            {
                unsigned int frameCountRead = qoaplay_decode((qoaplay_desc *)music.ctxData, (float *)AUDIO.System.pcmBuffer, framesToStream);
                frameCountReadTotal += frameCountRead;
                /*
                while (true)
                {
                    int frameCountRead = (int)qoaplay_decode((qoaplay_desc *)music.ctxData, (float *)((char *)AUDIO.System.pcmBuffer + frameCountReadTotal*frameSize),  frameCountStillNeeded);
                    frameCountReadTotal += frameCountRead;
                    frameCountStillNeeded -= frameCountRead;
                    if (frameCountStillNeeded == 0) break;
                    else qoaplay_rewind((qoaplay_desc *)music.ctxData);
                }
                */
            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_FLAC)
            case MUSIC_AUDIO_FLAC:
            {
                while (true)
                {
                    int frameCountRead = (int)drflac_read_pcm_frames_s16((drflac *)music.ctxData, frameCountStillNeeded, (short *)((char *)AUDIO.System.pcmBuffer + frameCountReadTotal*frameSize));
                    frameCountReadTotal += frameCountRead;
                    frameCountStillNeeded -= frameCountRead;
                    if (frameCountStillNeeded == 0) break;
                    else drflac__seek_to_first_frame((drflac *)music.ctxData);
                }
            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_XM)
            case MUSIC_MODULE_XM:
            {
                // NOTE: Internally we consider 2 channels generation, so sampleCount/2
                if (AUDIO_DEVICE_FORMAT == ma_format_f32) jar_xm_generate_samples((jar_xm_context_t *)music.ctxData, (float *)AUDIO.System.pcmBuffer, framesToStream);
                else if (AUDIO_DEVICE_FORMAT == ma_format_s16) jar_xm_generate_samples_16bit((jar_xm_context_t *)music.ctxData, (short *)AUDIO.System.pcmBuffer, framesToStream);
                else if (AUDIO_DEVICE_FORMAT == ma_format_u8) jar_xm_generate_samples_8bit((jar_xm_context_t *)music.ctxData, (char *)AUDIO.System.pcmBuffer, framesToStream);
                //jar_xm_reset((jar_xm_context_t *)music.ctxData);

            } break;
        #endif
        #if defined(SUPPORT_FILEFORMAT_MOD)
            case MUSIC_MODULE_MOD:
            {
                // NOTE: 3rd parameter (nbsample) specify the number of stereo 16bits samples you want, so sampleCount/2
                jar_mod_fillbuffer((jar_mod_context_t *)music.ctxData, (short *)AUDIO.System.pcmBuffer, framesToStream, 0);
                //jar_mod_seek_start((jar_mod_context_t *)music.ctxData);

            } break;
        #endif
            default: break;
        }

        UpdateAudioStreamInLockedState(music.stream, AUDIO.System.pcmBuffer, framesToStream);

        music.stream.buffer->framesProcessed = music.stream.buffer->framesProcessed%music.frameCount;

        if (framesLeft <= subBufferSizeInFrames)
        {
            if (!music.looping)
            {
                ma_mutex_unlock(&AUDIO.System.lock);
                // Streaming is ending, we filled latest frames from input
                StopMusicStream(music);
                return;
            }
        }
    }

    ma_mutex_unlock(&AUDIO.System.lock);
}

// Check if any music is playing
bool IsMusicStreamPlaying(Music music)
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

// Set pan for a music
void SetMusicPan(Music music, float pan)
{
    SetAudioBufferPan(music.stream.buffer, pan);
}

// Get music time length (in seconds)
float GetMusicTimeLength(Music music)
{
    float totalSeconds = 0.0f;

    totalSeconds = (float)music.frameCount/music.stream.sampleRate;

    return totalSeconds;
}

// Get current music time played (in seconds)
float GetMusicTimePlayed(Music music)
{
    float secondsPlayed = 0.0f;
    if (music.stream.buffer != NULL)
    {
#if defined(SUPPORT_FILEFORMAT_XM)
        if (music.ctxType == MUSIC_MODULE_XM)
        {
            uint64_t framesPlayed = 0;

            jar_xm_get_position(music.ctxData, NULL, NULL, NULL, &framesPlayed);
            secondsPlayed = (float)framesPlayed/music.stream.sampleRate;
        }
        else
#endif
        {
            ma_mutex_lock(&AUDIO.System.lock);
            //ma_uint32 frameSizeInBytes = ma_get_bytes_per_sample(music.stream.buffer->dsp.formatConverterIn.config.formatIn)*music.stream.buffer->dsp.formatConverterIn.config.channels;
            int framesProcessed = (int)music.stream.buffer->framesProcessed;
            int subBufferSize = (int)music.stream.buffer->sizeInFrames/2;
            int framesInFirstBuffer = music.stream.buffer->isSubBufferProcessed[0]? 0 : subBufferSize;
            int framesInSecondBuffer = music.stream.buffer->isSubBufferProcessed[1]? 0 : subBufferSize;
            int framesSentToMix = music.stream.buffer->frameCursorPos%subBufferSize;
            int framesPlayed = (framesProcessed - framesInFirstBuffer - framesInSecondBuffer + framesSentToMix)%(int)music.frameCount;
            if (framesPlayed < 0) framesPlayed += music.frameCount;
            secondsPlayed = (float)framesPlayed/music.stream.sampleRate;
            ma_mutex_unlock(&AUDIO.System.lock);
        }
    }

    return secondsPlayed;
}

// Load audio stream (to stream audio pcm data)
AudioStream LoadAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
{
    AudioStream stream = { 0 };

    stream.sampleRate = sampleRate;
    stream.sampleSize = sampleSize;
    stream.channels = channels;

    ma_format formatIn = ((stream.sampleSize == 8)? ma_format_u8 : ((stream.sampleSize == 16)? ma_format_s16 : ma_format_f32));

    // The size of a streaming buffer must be at least double the size of a period
    unsigned int periodSize = AUDIO.System.device.playback.internalPeriodSizeInFrames;

    // If the buffer is not set, compute one that would give us a buffer good enough for a decent frame rate
    unsigned int subBufferSize = (AUDIO.Buffer.defaultSize == 0)? AUDIO.System.device.sampleRate/30 : AUDIO.Buffer.defaultSize;

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

// Checks if an audio stream is ready
bool IsAudioStreamReady(AudioStream stream)
{
    return ((stream.buffer != NULL) &&    // Validate stream buffer
            (stream.sampleRate > 0) &&    // Validate sample rate is supported
            (stream.sampleSize > 0) &&    // Validate sample size is supported
            (stream.channels > 0));       // Validate number of channels supported
}

// Unload audio stream and free memory
void UnloadAudioStream(AudioStream stream)
{
    UnloadAudioBuffer(stream.buffer);

    TRACELOG(LOG_INFO, "STREAM: Unloaded audio stream data from RAM");
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: dequeue -> update -> queue
// NOTE 2: To dequeue a buffer it needs to be processed: IsAudioStreamProcessed()
void UpdateAudioStream(AudioStream stream, const void *data, int frameCount)
{
    ma_mutex_lock(&AUDIO.System.lock);
    UpdateAudioStreamInLockedState(stream, data, frameCount);
    ma_mutex_unlock(&AUDIO.System.lock);
}

// Check if any audio stream buffers requires refill
bool IsAudioStreamProcessed(AudioStream stream)
{
    if (stream.buffer == NULL) return false;

    bool result = false;
    ma_mutex_lock(&AUDIO.System.lock);
    result = stream.buffer->isSubBufferProcessed[0] || stream.buffer->isSubBufferProcessed[1];
    ma_mutex_unlock(&AUDIO.System.lock);
    return result;
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

// Check if audio stream is playing
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

// Set pan for audio stream
void SetAudioStreamPan(AudioStream stream, float pan)
{
    SetAudioBufferPan(stream.buffer, pan);
}

// Default size for new audio streams
void SetAudioStreamBufferSizeDefault(int size)
{
    AUDIO.Buffer.defaultSize = size;
}

// Audio thread callback to request new data
void SetAudioStreamCallback(AudioStream stream, AudioCallback callback)
{
    if (stream.buffer != NULL)
    {
        ma_mutex_lock(&AUDIO.System.lock);
        stream.buffer->callback = callback;
        ma_mutex_unlock(&AUDIO.System.lock);
    }
}

// Add processor to audio stream. Contrary to buffers, the order of processors is important
// The new processor must be added at the end. As there aren't supposed to be a lot of processors attached to
// a given stream, we iterate through the list to find the end. That way we don't need a pointer to the last element
void AttachAudioStreamProcessor(AudioStream stream, AudioCallback process)
{
    ma_mutex_lock(&AUDIO.System.lock);

    rAudioProcessor *processor = (rAudioProcessor *)RL_CALLOC(1, sizeof(rAudioProcessor));
    processor->process = process;

    rAudioProcessor *last = stream.buffer->processor;

    while (last && last->next)
    {
        last = last->next;
    }
    if (last)
    {
        processor->prev = last;
        last->next = processor;
    }
    else stream.buffer->processor = processor;

    ma_mutex_unlock(&AUDIO.System.lock);
}

// Remove processor from audio stream
void DetachAudioStreamProcessor(AudioStream stream, AudioCallback process)
{
    ma_mutex_lock(&AUDIO.System.lock);

    rAudioProcessor *processor = stream.buffer->processor;

    while (processor)
    {
        rAudioProcessor *next = processor->next;
        rAudioProcessor *prev = processor->prev;

        if (processor->process == process)
        {
            if (stream.buffer->processor == processor) stream.buffer->processor = next;
            if (prev) prev->next = next;
            if (next) next->prev = prev;

            RL_FREE(processor);
        }

        processor = next;
    }

    ma_mutex_unlock(&AUDIO.System.lock);
}

// Add processor to audio pipeline. Order of processors is important
// Works the same way as {Attach,Detach}AudioStreamProcessor() functions, except
// these two work on the already mixed output just before sending it to the sound hardware
void AttachAudioMixedProcessor(AudioCallback process)
{
    ma_mutex_lock(&AUDIO.System.lock);

    rAudioProcessor *processor = (rAudioProcessor *)RL_CALLOC(1, sizeof(rAudioProcessor));
    processor->process = process;

    rAudioProcessor *last = AUDIO.mixedProcessor;

    while (last && last->next)
    {
        last = last->next;
    }
    if (last)
    {
        processor->prev = last;
        last->next = processor;
    }
    else AUDIO.mixedProcessor = processor;

    ma_mutex_unlock(&AUDIO.System.lock);
}

// Remove processor from audio pipeline
void DetachAudioMixedProcessor(AudioCallback process)
{
    ma_mutex_lock(&AUDIO.System.lock);

    rAudioProcessor *processor = AUDIO.mixedProcessor;

    while (processor)
    {
        rAudioProcessor *next = processor->next;
        rAudioProcessor *prev = processor->prev;

        if (processor->process == process)
        {
            if (AUDIO.mixedProcessor == processor) AUDIO.mixedProcessor = next;
            if (prev) prev->next = next;
            if (next) next->prev = prev;

            RL_FREE(processor);
        }

        processor = next;
    }

    ma_mutex_unlock(&AUDIO.System.lock);
}


//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Log callback function
static void OnLog(void *pUserData, ma_uint32 level, const char *pMessage)
{
    TRACELOG(LOG_WARNING, "miniaudio: %s", pMessage);   // All log messages from miniaudio are errors
}

// Reads audio data from an AudioBuffer object in internal format
static ma_uint32 ReadAudioBufferFramesInInternalFormat(AudioBuffer *audioBuffer, void *framesOut, ma_uint32 frameCount)
{
    // Using audio buffer callback
    if (audioBuffer->callback)
    {
        audioBuffer->callback(framesOut, frameCount);
        audioBuffer->framesProcessed += frameCount;

        return frameCount;
    }

    ma_uint32 subBufferSizeInFrames = (audioBuffer->sizeInFrames > 1)? audioBuffer->sizeInFrames/2 : audioBuffer->sizeInFrames;
    ma_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos/subBufferSizeInFrames;

    if (currentSubBufferIndex > 1) return 0;

    // Another thread can update the processed state of buffers, so
    // we just take a copy here to try and avoid potential synchronization problems
    bool isSubBufferProcessed[2] = { 0 };
    isSubBufferProcessed[0] = audioBuffer->isSubBufferProcessed[0];
    isSubBufferProcessed[1] = audioBuffer->isSubBufferProcessed[1];

    ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(audioBuffer->converter.formatIn, audioBuffer->converter.channelsIn);

    // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0
    ma_uint32 framesRead = 0;
    while (1)
    {
        // We break from this loop differently depending on the buffer's usage
        //  - For static buffers, we simply fill as much data as we can
        //  - For streaming buffers we only fill half of the buffer that are processed
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
                StopAudioBufferInLockedState(audioBuffer);
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
        // to know whether a non-looping sound has finished playback
        if (audioBuffer->usage != AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
    }

    return framesRead;
}

// Reads audio data from an AudioBuffer object in device format, returned data will be in a format appropriate for mixing
static ma_uint32 ReadAudioBufferFramesInMixingFormat(AudioBuffer *audioBuffer, float *framesOut, ma_uint32 frameCount)
{
    // What's going on here is that we're continuously converting data from the AudioBuffer's internal format to the mixing format, which
    // should be defined by the output format of the data converter. We do this until frameCount frames have been output. The important
    // detail to remember here is that we never, ever attempt to read more input data than is required for the specified number of output
    // frames. This can be achieved with ma_data_converter_get_required_input_frame_count()
    ma_uint8 inputBuffer[4096] = { 0 };
    ma_uint32 inputBufferFrameCap = sizeof(inputBuffer)/ma_get_bytes_per_frame(audioBuffer->converter.formatIn, audioBuffer->converter.channelsIn);

    ma_uint32 totalOutputFramesProcessed = 0;
    while (totalOutputFramesProcessed < frameCount)
    {
        ma_uint64 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;
        ma_uint64 inputFramesToProcessThisIteration = 0;

        (void)ma_data_converter_get_required_input_frame_count(&audioBuffer->converter, outputFramesToProcessThisIteration, &inputFramesToProcessThisIteration);
        if (inputFramesToProcessThisIteration > inputBufferFrameCap)
        {
            inputFramesToProcessThisIteration = inputBufferFrameCap;
        }

        float *runningFramesOut = framesOut + (totalOutputFramesProcessed*audioBuffer->converter.channelsOut);

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
// This function will be called when miniaudio needs more data
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
                    float tempBuffer[1024] = { 0 }; // Frames for stereo

                    ma_uint32 framesToReadRightNow = framesToRead;
                    if (framesToReadRightNow > sizeof(tempBuffer)/sizeof(tempBuffer[0])/AUDIO_DEVICE_CHANNELS)
                    {
                        framesToReadRightNow = sizeof(tempBuffer)/sizeof(tempBuffer[0])/AUDIO_DEVICE_CHANNELS;
                    }

                    ma_uint32 framesJustRead = ReadAudioBufferFramesInMixingFormat(audioBuffer, tempBuffer, framesToReadRightNow);
                    if (framesJustRead > 0)
                    {
                        float *framesOut = (float *)pFramesOut + (framesRead*AUDIO.System.device.playback.channels);
                        float *framesIn = tempBuffer;

                        // Apply processors chain if defined
                        rAudioProcessor *processor = audioBuffer->processor;
                        while (processor)
                        {
                            processor->process(framesIn, framesJustRead);
                            processor = processor->next;
                        }

                        MixAudioFrames(framesOut, framesIn, framesJustRead, audioBuffer);

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
                            StopAudioBufferInLockedState(audioBuffer);
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

    rAudioProcessor *processor = AUDIO.mixedProcessor;
    while (processor)
    {
        processor->process(pFramesOut, frameCount);
        processor = processor->next;
    }

    ma_mutex_unlock(&AUDIO.System.lock);
}

// Main mixing function, pretty simple in this project, just an accumulation
// NOTE: framesOut is both an input and an output, it is initially filled with zeros outside of this function
static void MixAudioFrames(float *framesOut, const float *framesIn, ma_uint32 frameCount, AudioBuffer *buffer)
{
    const float localVolume = buffer->volume;
    const ma_uint32 channels = AUDIO.System.device.playback.channels;

    if (channels == 2)  // We consider panning
    {
        const float left = buffer->pan;
        const float right = 1.0f - left;

        // Fast sine approximation in [0..1] for pan law: y = 0.5f*x*(3 - x*x);
        const float levels[2] = { localVolume*0.5f*left*(3.0f - left*left), localVolume*0.5f*right*(3.0f - right*right) };

        float *frameOut = framesOut;
        const float *frameIn = framesIn;

        for (ma_uint32 frame = 0; frame < frameCount; frame++)
        {
            frameOut[0] += (frameIn[0]*levels[0]);
            frameOut[1] += (frameIn[1]*levels[1]);

            frameOut += 2;
            frameIn += 2;
        }
    }
    else  // We do not consider panning
    {
        for (ma_uint32 frame = 0; frame < frameCount; frame++)
        {
            for (ma_uint32 c = 0; c < channels; c++)
            {
                float *frameOut = framesOut + (frame*channels);
                const float *frameIn = framesIn + (frame*channels);

                // Output accumulates input multiplied by volume to provided output (usually 0)
                frameOut[c] += (frameIn[c]*localVolume);
            }
        }
    }
}

// Check if an audio buffer is playing, assuming the audio system mutex has been locked
static bool IsAudioBufferPlayingInLockedState(AudioBuffer *buffer)
{
    bool result = false;

    if (buffer != NULL) result = (buffer->playing && !buffer->paused);

    return result;
}

// Stop an audio buffer, assuming the audio system mutex has been locked
static void StopAudioBufferInLockedState(AudioBuffer *buffer)
{
    if (buffer != NULL)
    {
        if (IsAudioBufferPlayingInLockedState(buffer))
        {
            buffer->playing = false;
            buffer->paused = false;
            buffer->frameCursorPos = 0;
            buffer->framesProcessed = 0;
            buffer->isSubBufferProcessed[0] = true;
            buffer->isSubBufferProcessed[1] = true;
        }
    }
}

// Update audio stream, assuming the audio system mutex has been locked
static void UpdateAudioStreamInLockedState(AudioStream stream, const void *data, int frameCount)
{
    if (stream.buffer != NULL)
    {
        if (stream.buffer->isSubBufferProcessed[0] || stream.buffer->isSubBufferProcessed[1])
        {
            ma_uint32 subBufferToUpdate = 0;

            if (stream.buffer->isSubBufferProcessed[0] && stream.buffer->isSubBufferProcessed[1])
            {
                // Both buffers are available for updating
                // Update the first one and make sure the cursor is moved back to the front
                subBufferToUpdate = 0;
                stream.buffer->frameCursorPos = 0;
            }
            else
            {
                // Just update whichever sub-buffer is processed
                subBufferToUpdate = (stream.buffer->isSubBufferProcessed[0])? 0 : 1;
            }

            ma_uint32 subBufferSizeInFrames = stream.buffer->sizeInFrames/2;
            unsigned char *subBuffer = stream.buffer->data + ((subBufferSizeInFrames*stream.channels*(stream.sampleSize/8))*subBufferToUpdate);

            // Total frames processed in buffer is always the complete size, filled with 0 if required
            stream.buffer->framesProcessed += subBufferSizeInFrames;

            // Does this API expect a whole buffer to be updated in one go?
            // Assuming so, but if not will need to change this logic
            if (subBufferSizeInFrames >= (ma_uint32)frameCount)
            {
                ma_uint32 framesToWrite = (ma_uint32)frameCount;

                ma_uint32 bytesToWrite = framesToWrite*stream.channels*(stream.sampleSize/8);
                memcpy(subBuffer, data, bytesToWrite);

                // Any leftover frames should be filled with zeros
                ma_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;

                if (leftoverFrameCount > 0) memset(subBuffer + bytesToWrite, 0, leftoverFrameCount*stream.channels*(stream.sampleSize/8));

                stream.buffer->isSubBufferProcessed[subBufferToUpdate] = false;
            }
            else TRACELOG(LOG_WARNING, "STREAM: Attempting to write too many frames to buffer");
        }
        else TRACELOG(LOG_WARNING, "STREAM: Buffer not available for updating");
    }
}

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

// Get pointer to extension for a filename string (includes the dot: .png)
static const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return dot;
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

// Get pointer to filename for a path string
static const char *GetFileName(const char *filePath)
{
    const char *fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

    if (!fileName) return filePath;

    return fileName + 1;
}

// Get filename string without extension (uses static string)
static const char *GetFileNameWithoutExt(const char *filePath)
{
    #define MAX_FILENAMEWITHOUTEXT_LENGTH   256

    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH] = { 0 };
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);

    if (filePath != NULL) strcpy(fileName, GetFileName(filePath));   // Get filename with extension

    int size = (int)strlen(fileName);   // Get size in bytes

    for (int i = 0; (i < size) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
        }
    }

    return fileName;
}

// Load data from file into a buffer
static unsigned char *LoadFileData(const char *fileName, int *dataSize)
{
    unsigned char *data = NULL;
    *dataSize = 0;

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
                *dataSize = count;

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
static bool SaveFileData(const char *fileName, void *data, int dataSize)
{
    if (fileName != NULL)
    {
        FILE *file = fopen(fileName, "wb");

        if (file != NULL)
        {
            unsigned int count = (unsigned int)fwrite(data, sizeof(unsigned char), dataSize, file);

            if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write file", fileName);
            else if (count != dataSize) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially written", fileName);
            else TRACELOG(LOG_INFO, "FILEIO: [%s] File saved successfully", fileName);

            fclose(file);
        }
        else
        {
            TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
            return false;
        }
    }
    else
    {
        TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
        return false;
    }

    return true;
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
        else
        {
            TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
            return false;
        }
    }
    else
    {
        TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
        return false;
    }

    return true;
}
#endif

#undef AudioBuffer

#endif      // SUPPORT_MODULE_RAUDIO
