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

//#define AUDIO_STANDALONE     // NOTE: To use the audio module as standalone lib, just uncomment this line

#if defined(AUDIO_STANDALONE)
    #include "audio.h"
#else
    #include "raylib.h"
#endif

#include "AL/al.h"              // OpenAL basic header
#include "AL/alc.h"             // OpenAL context header (like OpenGL, OpenAL requires a context to work)

#include <stdlib.h>             // Required for: malloc(), free()
#include <string.h>             // Required for: strcmp(), strncmp()
#include <stdio.h>              // Required for: FILE, fopen(), fclose(), fread()

// Tokens defined by OpenAL extension: AL_EXT_float32
#ifndef AL_FORMAT_MONO_FLOAT32
    #define AL_FORMAT_MONO_FLOAT32 0x10010
#endif
#ifndef AL_FORMAT_STEREO_FLOAT32
    #define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif

#if defined(AUDIO_STANDALONE)
    #include <stdarg.h>         // Required for: va_list, va_start(), vfprintf(), va_end()
#else
    #include "utils.h"          // Required for: DecompressData()
                                // NOTE: Includes Android fopen() function map
#endif

//#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.h"    // OGG loading functions

#define JAR_XM_IMPLEMENTATION
#include "external/jar_xm.h"        // XM loading functions

#define JAR_MOD_IMPLEMENTATION
#include "external/jar_mod.h"       // MOD loading functions

#ifdef _MSC_VER
    #undef bool
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_STREAM_BUFFERS          2             // Number of buffers for each audio stream

// NOTE: Music buffer size is defined by number of samples, independent of sample size
// After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds
// and double-buffering system, I concluded that a 4096 samples buffer should be enough
// In case of music-stalls, just inclease this number
#define AUDIO_BUFFER_SIZE        4096             // PCM data samples (i.e. short: 32Kb)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef enum { MUSIC_AUDIO_OGG = 0, MUSIC_MODULE_XM, MUSIC_MODULE_MOD } MusicContextType;

// Used to create custom audio streams that are not bound to a specific file.
typedef struct AudioStream {
    unsigned int sampleRate;             // Frequency (samples per second): default is 48000
    unsigned int sampleSize;             // BitDepth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;               // Number of channels

    ALenum format;                       // OpenAL format specifier
    ALuint source;                       // OpenAL source
    ALuint buffers[MAX_STREAM_BUFFERS];  // OpenAL buffers (double buffering)
} AudioStream;

// Music type (file streaming from memory)
typedef struct Music {
    MusicContextType ctxType;           // Type of music context (OGG, XM, MOD)
    stb_vorbis *ctxOgg;                 // OGG audio context
    jar_xm_context_t *ctxXm;            // XM chiptune context
    jar_mod_context_t ctxMod;           // MOD chiptune context

    AudioStream stream;                 // Audio stream

    bool loop;                          // Repeat music after finish (loop)
    unsigned int totalSamples;          // Total number of samples
    unsigned int samplesLeft;           // Number of samples left to end
} MusicData, *Music;

// Audio errors to register
/*
typedef enum {
    ERROR_RAW_CONTEXT_CREATION      = 1,
    ERROR_XM_CONTEXT_CREATION       = 2,
    ERROR_MOD_CONTEXT_CREATION      = 4,
    ERROR_MIX_CHANNEL_CREATION      = 8,
    ERROR_MUSIC_CHANNEL_CREATION    = 16,
    ERROR_LOADING_XM                = 32,
    ERROR_LOADING_MOD               = 64,
    ERROR_LOADING_WAV               = 128,
    ERROR_LOADING_OGG               = 256,
    ERROR_OUT_OF_MIX_CHANNELS       = 512,
    ERROR_EXTENSION_NOT_RECOGNIZED  = 1024,
    ERROR_UNABLE_TO_OPEN_RRES_FILE  = 2048,
    ERROR_INVALID_RRES_FILE         = 4096,
    ERROR_INVALID_RRES_RESOURCE     = 8192,
    ERROR_UNINITIALIZED_CHANNELS    = 16384,
    ERROR_UNINTIALIZED_MUSIC_BUFFER = 32768
} AudioError;
*/

#if defined(AUDIO_STANDALONE)
typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int lastAudioError = 0;                     // Registers last audio error

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(const char *fileName);         // Load WAV file
static Wave LoadOGG(char *fileName);               // Load OGG file
static void UnloadWave(Wave wave);                 // Unload wave data

static AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels);
static void CloseAudioStream(AudioStream stream); // Frees mix channel
static int BufferAudioStream(AudioStream stream, void *data, int numberElements); // Pushes more audio data into mix channel

static bool BufferMusicStream(Music music, int numBuffersToProcess);  // Fill music buffers with data

#if defined(AUDIO_STANDALONE)
const char *GetExtension(const char *fileName);     // Get the extension for a filename
void TraceLog(int msgType, const char *text, ...);  // Outputs a trace log message (INFO, ERROR, WARNING)
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------

// Initialize audio device
void InitAudioDevice(void)
{
    // Open and initialize a device with default settings
    ALCdevice *device = alcOpenDevice(NULL);

    if (!device) TraceLog(ERROR, "Audio device could not be opened");
    else
    {
        ALCcontext *context = alcCreateContext(device, NULL);

        if ((context == NULL) || (alcMakeContextCurrent(context) == ALC_FALSE))
        {
            if (context != NULL) alcDestroyContext(context);

            alcCloseDevice(device);

            TraceLog(ERROR, "Could not initialize audio context");
        }
        else
        {
            TraceLog(INFO, "Audio device and context initialized successfully: %s", alcGetString(device, ALC_DEVICE_SPECIFIER));

            // Listener definition (just for 2D)
            alListener3f(AL_POSITION, 0, 0, 0);
            alListener3f(AL_VELOCITY, 0, 0, 0);
            alListener3f(AL_ORIENTATION, 0, 0, -1);
        }
    }
}

// Close the audio device for all contexts
void CloseAudioDevice(void)
{
    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) TraceLog(WARNING, "Could not get current audio context for closing");

    device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
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

//----------------------------------------------------------------------------------
// Module Functions Definition - Sounds loading and playing (.WAV)
//----------------------------------------------------------------------------------

// Load sound to memory
// NOTE: The entire file is loaded to memory to be played (no-streaming)
Sound LoadSound(char *fileName)
{
    Wave wave = { 0 };

    if (strcmp(GetExtension(fileName), "wav") == 0) wave = LoadWAV(fileName);
    else if (strcmp(GetExtension(fileName), "ogg") == 0) wave = LoadOGG(fileName);
    else TraceLog(WARNING, "[%s] Sound extension not recognized, it can't be loaded", fileName);

    Sound sound = LoadSoundFromWave(wave);
    
    // Sound is loaded, we can unload wave
    UnloadWave(wave);

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
        // The OpenAL format is worked out by looking at the number of channels and the bits per sample
        if (wave.channels == 1)
        {
            if (wave.bitsPerSample == 8 ) format = AL_FORMAT_MONO8;
            else if (wave.bitsPerSample == 16) format = AL_FORMAT_MONO16;
        }
        else if (wave.channels == 2)
        {
            if (wave.bitsPerSample == 8 ) format = AL_FORMAT_STEREO8;
            else if (wave.bitsPerSample == 16) format = AL_FORMAT_STEREO16;
        }

        // Create an audio source
        ALuint source;
        alGenSources(1, &source);            // Generate pointer to audio source

        alSourcef(source, AL_PITCH, 1);
        alSourcef(source, AL_GAIN, 1);
        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSource3f(source, AL_VELOCITY, 0, 0, 0);
        alSourcei(source, AL_LOOPING, AL_FALSE);

        // Convert loaded data to OpenAL buffer
        //----------------------------------------
        ALuint buffer;
        alGenBuffers(1, &buffer);            // Generate pointer to buffer

        // Upload sound data to buffer
        alBufferData(buffer, format, wave.data, wave.dataSize, wave.sampleRate);

        // Attach sound buffer to source
        alSourcei(source, AL_BUFFER, buffer);

        TraceLog(INFO, "[SND ID %i][BUFR ID %i] Sound data loaded successfully (SampleRate: %i, BitRate: %i, Channels: %i)", source, buffer, wave.sampleRate, wave.bitsPerSample, wave.channels);

        sound.source = source;
        sound.buffer = buffer;
    }

    return sound;
}

// Load sound to memory from rRES file (raylib Resource)
// TODO: Maybe rresName could be directly a char array with all the data?
Sound LoadSoundFromRES(const char *rresName, int resId)
{
    Sound sound = { 0 };

#if defined(AUDIO_STANDALONE)
    TraceLog(WARNING, "Sound loading from rRES resource file not supported on standalone mode");
#else

    bool found = false;

    char id[4];             // rRES file identifier
    unsigned char version;  // rRES file version and subversion
    char useless;           // rRES header reserved data
    short numRes;

    ResInfoHeader infoHeader;

    FILE *rresFile = fopen(rresName, "rb");

    if (rresFile == NULL) TraceLog(WARNING, "[%s] rRES raylib resource file could not be opened", rresName);
    else
    {
        // Read rres file (basic file check - id)
        fread(&id[0], sizeof(char), 1, rresFile);
        fread(&id[1], sizeof(char), 1, rresFile);
        fread(&id[2], sizeof(char), 1, rresFile);
        fread(&id[3], sizeof(char), 1, rresFile);
        fread(&version, sizeof(char), 1, rresFile);
        fread(&useless, sizeof(char), 1, rresFile);

        if ((id[0] != 'r') && (id[1] != 'R') && (id[2] != 'E') &&(id[3] != 'S'))
        {
            TraceLog(WARNING, "[%s] This is not a valid raylib resource file", rresName);
        }
        else
        {
            // Read number of resources embedded
            fread(&numRes, sizeof(short), 1, rresFile);

            for (int i = 0; i < numRes; i++)
            {
                fread(&infoHeader, sizeof(ResInfoHeader), 1, rresFile);

                if (infoHeader.id == resId)
                {
                    found = true;

                    // Check data is of valid SOUND type
                    if (infoHeader.type == 1)   // SOUND data type
                    {
                        // TODO: Check data compression type
                        // NOTE: We suppose compression type 2 (DEFLATE - default)

                        // Reading SOUND parameters
                        Wave wave;
                        short sampleRate, bps;
                        char channels, reserved;

                        fread(&sampleRate, sizeof(short), 1, rresFile); // Sample rate (frequency)
                        fread(&bps, sizeof(short), 1, rresFile);        // Bits per sample
                        fread(&channels, 1, 1, rresFile);               // Channels (1 - mono, 2 - stereo)
                        fread(&reserved, 1, 1, rresFile);               // <reserved>

                        wave.sampleRate = sampleRate;
                        wave.dataSize = infoHeader.srcSize;
                        wave.bitsPerSample = bps;
                        wave.channels = (short)channels;

                        unsigned char *data = malloc(infoHeader.size);

                        fread(data, infoHeader.size, 1, rresFile);

                        wave.data = DecompressData(data, infoHeader.size, infoHeader.srcSize);

                        free(data);

                        sound = LoadSoundFromWave(wave);
    
                        // Sound is loaded, we can unload wave data
                        UnloadWave(wave);
                    }
                    else TraceLog(WARNING, "[%s] Required resource do not seem to be a valid SOUND resource", rresName);
                }
                else
                {
                    // Depending on type, skip the right amount of parameters
                    switch (infoHeader.type)
                    {
                        case 0: fseek(rresFile, 6, SEEK_CUR); break;   // IMAGE: Jump 6 bytes of parameters
                        case 1: fseek(rresFile, 6, SEEK_CUR); break;   // SOUND: Jump 6 bytes of parameters
                        case 2: fseek(rresFile, 5, SEEK_CUR); break;   // MODEL: Jump 5 bytes of parameters (TODO: Review)
                        case 3: break;   // TEXT: No parameters
                        case 4: break;   // RAW: No parameters
                        default: break;
                    }

                    // Jump DATA to read next infoHeader
                    fseek(rresFile, infoHeader.size, SEEK_CUR);
                }
            }
        }

        fclose(rresFile);
    }

    if (!found) TraceLog(WARNING, "[%s] Required resource id [%i] could not be found in the raylib resource file", rresName, resId);
#endif
    return sound;
}

// Unload sound
void UnloadSound(Sound sound)
{
    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);

    TraceLog(INFO, "[SND ID %i][BUFR ID %i] Unloaded sound data from RAM", sound.source, sound.buffer);
}

// Play a sound
void PlaySound(Sound sound)
{
    alSourcePlay(sound.source);        // Play the sound

    //TraceLog(INFO, "Playing sound");

    // Find the current position of the sound being played
    // NOTE: Only work when the entire file is in a single buffer
    //int byteOffset;
    //alGetSourcei(sound.source, AL_BYTE_OFFSET, &byteOffset);
    //
    //int sampleRate;
    //alGetBufferi(sound.buffer, AL_FREQUENCY, &sampleRate);    // AL_CHANNELS, AL_BITS (bps)

    //float seconds = (float)byteOffset / sampleRate;      // Number of seconds since the beginning of the sound
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

//----------------------------------------------------------------------------------
// Module Functions Definition - Music loading and stream playing (.OGG)
//----------------------------------------------------------------------------------

// Load music stream from file
Music LoadMusicStream(char *fileName)
{
    Music music = (MusicData *)malloc(sizeof(MusicData));

    if (strcmp(GetExtension(fileName), "ogg") == 0)
    {
        // Open ogg audio stream
        music->ctxOgg = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (music->ctxOgg == NULL)
        {
            TraceLog(WARNING, "[%s] OGG audio file could not be opened", fileName);
        }
        else
        {
            stb_vorbis_info info = stb_vorbis_get_info(music->ctxOgg);  // Get Ogg file info

            TraceLog(DEBUG, "[%s] Ogg sample rate: %i", fileName, info.sample_rate);
            TraceLog(DEBUG, "[%s] Ogg channels: %i", fileName, info.channels);
            TraceLog(DEBUG, "[%s] Temp memory required: %i", fileName, info.temp_memory_required);

            // TODO: Support 32-bit sampleSize OGGs
            music->stream = InitAudioStream(info.sample_rate, 16, info.channels);
            
            music->totalSamples = (unsigned int)stb_vorbis_stream_length_in_samples(music->ctxOgg)*info.channels;
            music->samplesLeft = music->totalSamples;
            //float totalLengthSeconds = stb_vorbis_stream_length_in_seconds(music->ctxOgg);

            music->ctxType = MUSIC_AUDIO_OGG;
            music->loop = true;                  // We loop by default
        }
    }
    else if (strcmp(GetExtension(fileName), "xm") == 0)
    {
        int result = jar_xm_create_context_from_file(&music->ctxXm, 48000, fileName);
        
        if (!result)    // XM context created successfully
        {
            jar_xm_set_max_loop_count(music->ctxXm, 0);     // Set infinite number of loops

            music->totalSamples = (unsigned int)jar_xm_get_remaining_samples(music->ctxXm);
            music->samplesLeft = music->totalSamples;

            TraceLog(INFO, "[%s] XM number of samples: %i", fileName, music->totalSamples);
            TraceLog(INFO, "[%s] XM track length: %11.6f sec", fileName, (float)music->totalSamples/48000.0f);
            
            // NOTE: Only stereo is supported for XM
            music->stream = InitAudioStream(48000, 32, 2);
            
            music->ctxType = MUSIC_MODULE_XM;
            music->loop = true;
        }
        else TraceLog(WARNING, "[%s] XM file could not be opened", fileName);
    }
    else if (strcmp(GetExtension(fileName), "mod") == 0)
    {
        jar_mod_init(&music->ctxMod);

        if (jar_mod_load_file(&music->ctxMod, fileName))
        {
            music->totalSamples = (unsigned int)jar_mod_max_samples(&music->ctxMod);
            music->samplesLeft = music->totalSamples;

            TraceLog(INFO, "[%s] MOD number of samples: %i", fileName, music->samplesLeft);
            TraceLog(INFO, "[%s] MOD track length: %11.6f sec", fileName, (float)music->totalSamples/48000.0f);

            music->stream = InitAudioStream(48000, 16, 2);

            music->ctxType = MUSIC_MODULE_MOD;
            music->loop = true;
        }
        else TraceLog(WARNING, "[%s] MOD file could not be opened", fileName);
    }
    else TraceLog(WARNING, "[%s] Music extension not recognized, it can't be loaded", fileName);

    return music;
}

// Unload music stream
void UnloadMusicStream(Music music)
{
    CloseAudioStream(music->stream);
    
    if (music->ctxType == MUSIC_AUDIO_OGG) stb_vorbis_close(music->ctxOgg);
    else if (music->ctxType == MUSIC_MODULE_XM) jar_xm_free_context(music->ctxXm);
    else if (music->ctxType == MUSIC_MODULE_MOD) jar_mod_unload(&music->ctxMod);
    
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

    if (state == AL_PAUSED) alSourcePlay(music->stream.source);
}

// Stop music playing (close stream)
void StopMusicStream(Music music)
{
    alSourceStop(music->stream.source);
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(Music music)
{
    ALenum state;
    bool active = true;
    ALint processed = 0;

    // Determine if music stream is ready to be written
    alGetSourcei(music->stream.source, AL_BUFFERS_PROCESSED, &processed);

    if (processed > 0)
    {
        active = BufferMusicStream(music, processed);

        if (!active && music->loop)
        {
            // Restart music context (if required)
            if (music->ctxType == MUSIC_MODULE_MOD) jar_mod_seek_start(&music->ctxMod);
            else if (music->ctxType == MUSIC_AUDIO_OGG) stb_vorbis_seek_start(music->ctxOgg);
            
            music->samplesLeft = music->totalSamples;

            // Determine if music stream is ready to be written
            alGetSourcei(music->stream.source, AL_BUFFERS_PROCESSED, &processed);

            active = BufferMusicStream(music, processed);
        }

        if (alGetError() != AL_NO_ERROR) TraceLog(WARNING, "Error buffering data...");

        alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);

        if (state != AL_PLAYING && active) alSourcePlay(music->stream.source);

        if (!active) StopMusicStream(music);
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

    if (music->ctxType == MUSIC_MODULE_XM)
    {
        uint64_t samplesPlayed;
        jar_xm_get_position(music->ctxXm, NULL, NULL, NULL, &samplesPlayed);
        
        // TODO: Not sure if this is the correct value
        secondsPlayed = (float)samplesPlayed/(music->stream.sampleRate*music->stream.channels);
    }
    else if (music->ctxType == MUSIC_MODULE_MOD)
    {
        long samplesPlayed = jar_mod_current_samples(&music->ctxMod);
        
        secondsPlayed = (float)samplesPlayed/music->stream.sampleRate;
    }
    else if (music->ctxType == MUSIC_AUDIO_OGG)
    {
        unsigned int samplesPlayed = music->totalSamples - music->samplesLeft;
        
        secondsPlayed = (float)samplesPlayed/(music->stream.sampleRate*music->stream.channels);
    }

    return secondsPlayed;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Init audio stream (to stream audio pcm data)
static AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
{
    AudioStream stream = { 0 };
    
    stream.sampleRate = sampleRate;
    stream.sampleSize = sampleSize;
    stream.channels = channels;

    // Setup OpenAL format
    if (channels == 1)
    {
        switch (sampleSize)
        {
            case 8: stream.format = AL_FORMAT_MONO8; break;
            case 16: stream.format = AL_FORMAT_MONO16; break;
            case 32: stream.format = AL_FORMAT_MONO_FLOAT32; break;
            default: TraceLog(WARNING, "Init audio stream: Sample size not supported: %i", sampleSize); break;
        }
    }
    else if (channels == 2)
    {
        switch (sampleSize)
        {
            case 8: stream.format = AL_FORMAT_STEREO8; break;
            case 16: stream.format = AL_FORMAT_STEREO16; break;
            case 32: stream.format = AL_FORMAT_STEREO_FLOAT32; break;
            default: TraceLog(WARNING, "Init audio stream: Sample size not supported: %i", sampleSize); break;
        }
    }
    else TraceLog(WARNING, "Init audio stream: Number of channels not supported: %i", channels);

    // Create an audio source
    alGenSources(1, &stream.source);
    alSourcef(stream.source, AL_PITCH, 1);
    alSourcef(stream.source, AL_GAIN, 1);
    alSource3f(stream.source, AL_POSITION, 0, 0, 0);
    alSource3f(stream.source, AL_VELOCITY, 0, 0, 0);

    // Create Buffers
    alGenBuffers(MAX_STREAM_BUFFERS, stream.buffers);

    // Initialize buffer with zeros by default
    for (int i = 0; i < MAX_STREAM_BUFFERS; i++)
    {
        if (stream.sampleSize == 8)
        {
            unsigned char pcm[AUDIO_BUFFER_SIZE] = { 0 };
            alBufferData(stream.buffers[i], stream.format, pcm, AUDIO_BUFFER_SIZE*sizeof(unsigned char), stream.sampleRate);
        }
        else if (stream.sampleSize == 16)
        {
            short pcm[AUDIO_BUFFER_SIZE] = { 0 };
            alBufferData(stream.buffers[i], stream.format, pcm, AUDIO_BUFFER_SIZE*sizeof(short), stream.sampleRate);
        }
        else if (stream.sampleSize == 32)
        {
            float pcm[AUDIO_BUFFER_SIZE] = { 0.0f };
            alBufferData(stream.buffers[i], stream.format, pcm, AUDIO_BUFFER_SIZE*sizeof(float), stream.sampleRate);
        }
    }

    alSourceQueueBuffers(stream.source, MAX_STREAM_BUFFERS, stream.buffers);
    
    TraceLog(INFO, "[AUD ID %i] Audio stream loaded successfully", stream.source);

    return stream;
}

// Close audio stream and free memory
static void CloseAudioStream(AudioStream stream)
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
    
    TraceLog(INFO, "[AUD ID %i] Unloaded audio stream data", stream.source);
}

// Push more audio data into audio stream, only one buffer per call
// NOTE: Returns number of samples that were processed
static int BufferAudioStream(AudioStream stream, void *data, int numberElements)
{
    if (!data || !numberElements)   
    { 
        // Pauses audio until data is given
        alSourcePause(stream.source);
        return 0;
    }
    
    ALuint buffer = 0;
    alSourceUnqueueBuffers(stream.source, 1, &buffer);
    
    if (!buffer) return 0;
    
    // Reference
    //void alBufferData(ALuint bufferName, ALenum format, const ALvoid *data, ALsizei size, ALsizei frequency);
    
    // ALuint bufferName: buffer id
    // ALenum format: Valid formats are 
    //      AL_FORMAT_MONO8,        // unsigned char
    //      AL_FORMAT_MONO16,       // short
    //      AL_FORMAT_STEREO8,
    //      AL_FORMAT_STEREO16      // stereo data is interleaved: left+right channels sample
    //      AL_FORMAT_MONO_FLOAT32 (extension)
    //      AL_FORMAT_STEREO_FLOAT32 (extension)
    // ALsizei size: Number of bytes, must be coherent with format
    // ALsizei frequency: sample rate
    
    if (stream.sampleSize == 8) alBufferData(buffer, stream.format, (unsigned char *)data, numberElements*sizeof(unsigned char), stream.sampleRate);
    else if (stream.sampleSize == 16) alBufferData(buffer, stream.format, (short *)data, numberElements*sizeof(short), stream.sampleRate);
    else if (stream.sampleSize == 32) alBufferData(buffer, stream.format, (float *)data, numberElements*sizeof(float), stream.sampleRate);

    alSourceQueueBuffers(stream.source, 1, &buffer);

    return numberElements;
}

// Fill music buffers with new data from music stream
static bool BufferMusicStream(Music music, int numBuffersToProcess)
{
    short pcm[AUDIO_BUFFER_SIZE];
    float pcmf[AUDIO_BUFFER_SIZE];

    int size = 0;              // Total size of data steamed in L+R samples for xm floats, individual L or R for ogg shorts
    bool active = true;        // We can get more data from stream (not finished)

    if (music->ctxType == MUSIC_MODULE_XM) // There is no end of stream for xmfiles, once the end is reached zeros are generated for non looped chiptunes.
    {
        for (int i = 0; i < numBuffersToProcess; i++)
        {
            if (music->samplesLeft >= AUDIO_BUFFER_SIZE) size = AUDIO_BUFFER_SIZE/2;
            else size = music->samplesLeft/2;

            // Read 2*shorts and moves them to buffer+size memory location
            jar_xm_generate_samples(music->ctxXm, pcmf, size); 
            
            BufferAudioStream(music->stream, pcmf, size*2);

            music->samplesLeft -= size;

            if (music->samplesLeft <= 0)
            {
                active = false;
                break;
            }
        }
    }
    else if (music->ctxType == MUSIC_MODULE_MOD)
    {
        for (int i = 0; i < numBuffersToProcess; i++)
        {
            if (music->samplesLeft >= AUDIO_BUFFER_SIZE) size = AUDIO_BUFFER_SIZE/2;
            else size = music->samplesLeft/2;

            jar_mod_fillbuffer(&music->ctxMod, pcm, size, 0);
            
            BufferAudioStream(music->stream, pcm, size*2);

            music->samplesLeft -= size;

            if (music->samplesLeft <= 0)
            {
                active = false;
                break;
            }
        }
    }
    else if (music->ctxType == MUSIC_AUDIO_OGG)
    {
        if (music->samplesLeft >= AUDIO_BUFFER_SIZE) size = AUDIO_BUFFER_SIZE;
        else size = music->samplesLeft;

        for (int i = 0; i < numBuffersToProcess; i++)
        {
            // NOTE: Returns the number of samples stored per channel
            int numSamples = stb_vorbis_get_samples_short_interleaved(music->ctxOgg, music->stream.channels, pcm, size);
            
            BufferAudioStream(music->stream, pcm, numSamples*music->stream.channels);
            
            music->samplesLeft -= (numSamples*music->stream.channels);

            if (music->samplesLeft <= 0)
            {
                active = false;
                break;
            }
        }
    }

    return active;
}

// Load WAV file into Wave structure
static Wave LoadWAV(const char *fileName)
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

    Wave wave = { 0 };
    FILE *wavFile;

    wavFile = fopen(fileName, "rb");

    if (wavFile == NULL)
    {
        TraceLog(WARNING, "[%s] WAV file could not be opened", fileName);
        wave.data = NULL;
    }
    else
    {
        // Read in the first chunk into the struct
        fread(&riffHeader, sizeof(RiffHeader), 1, wavFile);

        // Check for RIFF and WAVE tags
        if (strncmp(riffHeader.chunkID, "RIFF", 4) ||
            strncmp(riffHeader.format, "WAVE", 4))
        {
                TraceLog(WARNING, "[%s] Invalid RIFF or WAVE Header", fileName);
        }
        else
        {
            // Read in the 2nd chunk for the wave info
            fread(&waveFormat, sizeof(WaveFormat), 1, wavFile);

            // Check for fmt tag
            if ((waveFormat.subChunkID[0] != 'f') || (waveFormat.subChunkID[1] != 'm') ||
                (waveFormat.subChunkID[2] != 't') || (waveFormat.subChunkID[3] != ' '))
            {
                TraceLog(WARNING, "[%s] Invalid Wave format", fileName);
            }
            else
            {
                // Check for extra parameters;
                if (waveFormat.subChunkSize > 16) fseek(wavFile, sizeof(short), SEEK_CUR);

                // Read in the the last byte of data before the sound file
                fread(&waveData, sizeof(WaveData), 1, wavFile);

                // Check for data tag
                if ((waveData.subChunkID[0] != 'd') || (waveData.subChunkID[1] != 'a') ||
                    (waveData.subChunkID[2] != 't') || (waveData.subChunkID[3] != 'a'))
                {
                    TraceLog(WARNING, "[%s] Invalid data header", fileName);
                }
                else
                {
                    // Allocate memory for data
                    wave.data = (unsigned char *)malloc(sizeof(unsigned char) * waveData.subChunkSize);

                    // Read in the sound data into the soundData variable
                    fread(wave.data, waveData.subChunkSize, 1, wavFile);

                    // Now we set the variables that we need later
                    wave.dataSize = waveData.subChunkSize;
                    wave.sampleRate = waveFormat.sampleRate;
                    wave.channels = waveFormat.numChannels;
                    wave.bitsPerSample = waveFormat.bitsPerSample;

                    TraceLog(INFO, "[%s] WAV file loaded successfully (SampleRate: %i, BitRate: %i, Channels: %i)", fileName, wave.sampleRate, wave.bitsPerSample, wave.channels);
                }
            }
        }

        fclose(wavFile);
    }

    return wave;
}

// Load OGG file into Wave structure
// NOTE: Using stb_vorbis library
static Wave LoadOGG(char *fileName)
{
    Wave wave;

    stb_vorbis *oggFile = stb_vorbis_open_filename(fileName, NULL, NULL);

    if (oggFile == NULL)
    {
        TraceLog(WARNING, "[%s] OGG file could not be opened", fileName);
        wave.data = NULL;
    }
    else
    {
        stb_vorbis_info info = stb_vorbis_get_info(oggFile);

        wave.sampleRate = info.sample_rate;
        wave.bitsPerSample = 16;
        wave.channels = info.channels;

        TraceLog(DEBUG, "[%s] Ogg sample rate: %i", fileName, info.sample_rate);
        TraceLog(DEBUG, "[%s] Ogg channels: %i", fileName, info.channels);

        int totalSamplesLength = (stb_vorbis_stream_length_in_samples(oggFile)*info.channels);

        wave.dataSize = totalSamplesLength*sizeof(short);   // Size must be in bytes

        TraceLog(DEBUG, "[%s] Samples length: %i", fileName, totalSamplesLength);

        float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);

        TraceLog(DEBUG, "[%s] Total seconds: %f", fileName, totalSeconds);

        if (totalSeconds > 10) TraceLog(WARNING, "[%s] Ogg audio lenght is larger than 10 seconds (%f), that's a big file in memory, consider music streaming", fileName, totalSeconds);

        int totalSamples = totalSeconds*info.sample_rate*info.channels;

        TraceLog(DEBUG, "[%s] Total samples calculated: %i", fileName, totalSamples);

        wave.data = malloc(sizeof(short)*totalSamplesLength);

        int samplesObtained = stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, wave.data, totalSamplesLength);

        TraceLog(DEBUG, "[%s] Samples obtained: %i", fileName, samplesObtained);

        TraceLog(INFO, "[%s] OGG file loaded successfully (SampleRate: %i, BitRate: %i, Channels: %i)", fileName, wave.sampleRate, wave.bitsPerSample, wave.channels);

        stb_vorbis_close(oggFile);
    }

    return wave;
}

// Unload Wave data
static void UnloadWave(Wave wave)
{
    free(wave.data);

    TraceLog(INFO, "Unloaded wave data from RAM");
}

// Some required functions for audio standalone module version
#if defined(AUDIO_STANDALONE)
// Get the extension for a filename
const char *GetExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');
    if(!dot || dot == fileName) return "";
    return (dot + 1);
}

// Outputs a trace log message (INFO, ERROR, WARNING)
// NOTE: If a file has been init, output log is written there
void TraceLog(int msgType, const char *text, ...)
{
    va_list args;
    int traceDebugMsgs = 0;

#ifdef DO_NOT_TRACE_DEBUG_MSGS
    traceDebugMsgs = 0;
#endif

    switch(msgType)
    {
        case INFO: fprintf(stdout, "INFO: "); break;
        case ERROR: fprintf(stdout, "ERROR: "); break;
        case WARNING: fprintf(stdout, "WARNING: "); break;
        case DEBUG: if (traceDebugMsgs) fprintf(stdout, "DEBUG: "); break;
        default: break;
    }

    if ((msgType != DEBUG) || ((msgType == DEBUG) && (traceDebugMsgs)))
    {
        va_start(args, text);
        vfprintf(stdout, text, args);
        va_end(args);

        fprintf(stdout, "\n");
    }

    if (msgType == ERROR) exit(1);      // If ERROR message, exit program
}
#endif