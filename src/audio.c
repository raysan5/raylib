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
*   #define SUPPORT_FILEFORMAT_WAV
*   #define SUPPORT_FILEFORMAT_OGG
*   #define SUPPORT_FILEFORMAT_XM
*   #define SUPPORT_FILEFORMAT_MOD
*   #define SUPPORT_FILEFORMAT_FLAC
*       Selected desired fileformats to be supported for loading. Some of those formats are 
*       supported by default, to remove support, just comment unrequired #define in this module
*
*   LIMITATIONS:
*       Only up to two channels supported: MONO and STEREO (for additional channels, use AL_EXT_MCFORMATS)
*       Only the following sample sizes supported: 8bit PCM, 16bit PCM, 32-bit float PCM (using AL_EXT_FLOAT32)
*
*   DEPENDENCIES:
*       OpenAL Soft - Audio device management (http://kcat.strangesoft.net/openal.html)
*       stb_vorbis  - OGG audio files loading (http://www.nothings.org/stb_vorbis/)
*       jar_xm      - XM module file loading
*       jar_mod     - MOD audio file loading
*       dr_flac     - FLAC audio file loading
*
*   CONTRIBUTORS:
*       Joshua Reisenauer (github: @kd7tck):
*           - XM audio module support (jar_xm)
*           - MOD audio module support (jar_mod)
*           - Mixing channels support
*           - Raw audio context support
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2017 Ramon Santamaria (@raysan5)
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

// Default configuration flags (supported features)
//-------------------------------------------------
#define SUPPORT_FILEFORMAT_WAV
#define SUPPORT_FILEFORMAT_OGG
#define SUPPORT_FILEFORMAT_XM
#define SUPPORT_FILEFORMAT_MOD
//-------------------------------------------------

#if defined(AUDIO_STANDALONE)
    #include "audio.h"
    #include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#else
    #include "raylib.h"
    #include "utils.h"          // Required for: fopen() Android mapping
#endif

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

typedef enum { MUSIC_AUDIO_OGG = 0, MUSIC_AUDIO_FLAC, MUSIC_MODULE_XM, MUSIC_MODULE_MOD } MusicContextType;

// Music type (file streaming from memory)
typedef struct MusicData {
    MusicContextType ctxType;           // Type of music context (OGG, XM, MOD)
#if defined(SUPPORT_FILEFORMAT_OGG)
    stb_vorbis *ctxOgg;                 // OGG audio context
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
    drflac *ctxFlac;                    // FLAC audio context
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
typedef enum { LOG_INFO = 0, LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_OTHER } TraceLogType;
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

#if defined(AUDIO_STANDALONE)
bool IsFileExtension(const char *fileName, const char *ext);    // Check file extension
void TraceLog(int msgType, const char *text, ...);              // Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------

// Initialize audio device
void InitAudioDevice(void)
{
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
        }
    }
}

// Close the audio device for all contexts
void CloseAudioDevice(void)
{
    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) TraceLog(LOG_WARNING, "Could not get current audio context for closing");

    device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    TraceLog(LOG_INFO, "Audio device closed successfully");
}

// Check if device has been initialized successfully
bool IsAudioDeviceReady(void)
{
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) return false;
    else
    {
        ALCdevice *device = alcGetContextsDevice(context);

        if (device == NULL) return false;
        else return true;
    }
}

// Set master volume (listener)
void SetMasterVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    else if (volume > 1.0f) volume = 1.0f;
    
    alListenerf(AL_GAIN, volume);
}

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
#if !defined(AUDIO_STANDALONE)
    else if (IsFileExtension(fileName, ".rres"))
    {
        RRES rres = LoadResource(fileName, 0);

        // NOTE: Parameters for RRES_TYPE_WAVE are: sampleCount, sampleRate, sampleSize, channels

        if (rres[0].type == RRES_TYPE_WAVE) wave = LoadWaveEx(rres[0].data, rres[0].param1, rres[0].param2, rres[0].param3, rres[0].param4);
        else TraceLog(LOG_WARNING, "[%s] Resource file does not contain wave data", fileName);

        UnloadResource(rres);
    }
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
    alSourceStop(sound.source);

    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);

    TraceLog(LOG_INFO, "[SND ID %i][BUFR ID %i] Unloaded sound data from RAM", sound.source, sound.buffer);
}

// Update sound buffer with new data
// NOTE: data must match sound.format
void UpdateSound(Sound sound, const void *data, int samplesCount)
{
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
}

// Play a sound
void PlaySound(Sound sound)
{
    alSourcePlay(sound.source);        // Play the sound

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
    alSourcePause(sound.source);
}

// Resume a paused sound
void ResumeSound(Sound sound)
{
    ALenum state;

    alGetSourcei(sound.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED) alSourcePlay(sound.source);
}

// Stop reproducing a sound
void StopSound(Sound sound)
{
    alSourceStop(sound.source);
}

// Check if a sound is playing
bool IsSoundPlaying(Sound sound)
{
    bool playing = false;
    ALint state;

    alGetSourcei(sound.source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) playing = true;

    return playing;
}

// Set volume for a sound
void SetSoundVolume(Sound sound, float volume)
{
    alSourcef(sound.source, AL_GAIN, volume);
}

// Set pitch for a sound
void SetSoundPitch(Sound sound, float pitch)
{
    alSourcef(sound.source, AL_PITCH, pitch);
}

// Convert wave data to desired format
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
{
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
        (finalSample > 0) && (finalSample < wave->sampleCount))
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

    for (int i = 0; i < wave.sampleCount; i++)
    {
        for (int j = 0; j < wave.channels; j++)
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

    if (IsFileExtension(fileName, ".ogg"))
    {
        // Open ogg audio stream
        music->ctxOgg = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (music->ctxOgg == NULL) TraceLog(LOG_WARNING, "[%s] OGG audio file could not be opened", fileName);
        else
        {
            stb_vorbis_info info = stb_vorbis_get_info(music->ctxOgg);  // Get Ogg file info

            // OGG bit rate defaults to 16 bit, it's enough for compressed format
            music->stream = InitAudioStream(info.sample_rate, 16, info.channels);
            music->totalSamples = (unsigned int)stb_vorbis_stream_length_in_samples(music->ctxOgg); // Independent by channel
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_AUDIO_OGG;
            music->loopCount = -1;                       // Infinite loop by default

            TraceLog(LOG_DEBUG, "[%s] FLAC total samples: %i", fileName, music->totalSamples);
            TraceLog(LOG_DEBUG, "[%s] OGG sample rate: %i", fileName, info.sample_rate);
            TraceLog(LOG_DEBUG, "[%s] OGG channels: %i", fileName, info.channels);
            TraceLog(LOG_DEBUG, "[%s] OGG memory required: %i", fileName, info.temp_memory_required);
        }
    }
#if defined(SUPPORT_FILEFORMAT_FLAC)
    else if (IsFileExtension(fileName, ".flac"))
    {
        music->ctxFlac = drflac_open_file(fileName);

        if (music->ctxFlac == NULL) TraceLog(LOG_WARNING, "[%s] FLAC audio file could not be opened", fileName);
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
        else TraceLog(LOG_WARNING, "[%s] XM file could not be opened", fileName);
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
        else TraceLog(LOG_WARNING, "[%s] MOD file could not be opened", fileName);
    }
#endif
    else TraceLog(LOG_WARNING, "[%s] Audio fileformat not supported, it can't be loaded", fileName);

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
    alSourcePlay(music->stream.source);
}

// Pause music playing
void PauseMusicStream(Music music)
{
    alSourcePause(music->stream.source);
}

// Resume music playing
void ResumeMusicStream(Music music)
{
    ALenum state;
    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED) 
    {
        TraceLog(LOG_INFO, "[AUD ID %i] Resume music stream playing", music->stream.source);
        alSourcePlay(music->stream.source);
    }
}

// Stop music playing (close stream)
// TODO: To clear a buffer, make sure they have been already processed!
void StopMusicStream(Music music)
{
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
    
    // Restart music context
    switch (music->ctxType)
    {
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_start(music->ctxOgg); break;
#if defined(SUPPORT_FILEFORMAT_FLAC)
        case MUSIC_MODULE_FLAC: /* TODO: Restart FLAC context */ break;
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
        }
        else
        {
            // NOTE: In case window is minimized, music stream is stopped,
            // just make sure to play again on window restore
            if (state != AL_PLAYING) PlayMusicStream(music);
        }
    }
}

// Check if any music is playing
bool IsMusicPlaying(Music music)
{
    bool playing = false;
    ALint state;

    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING) playing = true;

    return playing;
}

// Set volume for music
void SetMusicVolume(Music music, float volume)
{
    alSourcef(music->stream.source, AL_GAIN, volume);
}

// Set pitch for music
void SetMusicPitch(Music music, float pitch)
{
    alSourcef(music->stream.source, AL_PITCH, pitch);
}

// Set music loop count (loop repeats)
// NOTE: If set to -1, means infinite loop
void SetMusicLoopCount(Music music, float count)
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

    TraceLog(LOG_INFO, "[AUD ID %i] Audio stream loaded successfully (%i Hz, %i bit, %s)", stream.source, stream.sampleRate, stream.sampleSize, (stream.channels == 1) ? "Mono" : "Stereo");

    return stream;
}

// Close audio stream and free memory
void CloseAudioStream(AudioStream stream)
{
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

    TraceLog(LOG_INFO, "[AUD ID %i] Unloaded audio stream data", stream.source);
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: unqueue -> update -> queue
// NOTE 2: To unqueue a buffer it needs to be processed: IsAudioBufferProcessed()
void UpdateAudioStream(AudioStream stream, const void *data, int samplesCount)
{
    ALuint buffer = 0;
    alSourceUnqueueBuffers(stream.source, 1, &buffer);

    // Check if any buffer was available for unqueue
    if (alGetError() != AL_INVALID_VALUE)
    {
        alBufferData(buffer, stream.format, data, samplesCount*stream.sampleSize/8*stream.channels, stream.sampleRate);
        alSourceQueueBuffers(stream.source, 1, &buffer);
    }
    else TraceLog(LOG_WARNING, "[AUD ID %i] Audio buffer not available for unqueuing", stream.source);
}

// Check if any audio stream buffers requires refill
bool IsAudioBufferProcessed(AudioStream stream)
{
    ALint processed = 0;

    // Determine if music stream is ready to be written
    alGetSourcei(stream.source, AL_BUFFERS_PROCESSED, &processed);

    return (processed > 0);
}

// Play audio stream
void PlayAudioStream(AudioStream stream)
{
    alSourcePlay(stream.source);
}

// Play audio stream
void PauseAudioStream(AudioStream stream)
{
    alSourcePause(stream.source);
}

// Resume audio stream playing
void ResumeAudioStream(AudioStream stream)
{
    ALenum state;
    alGetSourcei(stream.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED) alSourcePlay(stream.source);
}

// Stop audio stream
void StopAudioStream(AudioStream stream)
{
    alSourceStop(stream.source);
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
