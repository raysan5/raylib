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

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_WIN32_IO
#include "external/dr_flac.h"       // FLAC loading functions

#ifdef _MSC_VER
    #undef bool
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_STREAM_BUFFERS          2    // Number of buffers for each audio stream

// NOTE: Music buffer size is defined by number of samples, independent of sample size
// After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds
// and double-buffering system, I concluded that a 4096 samples buffer should be enough
// In case of music-stalls, just increase this number
#define AUDIO_BUFFER_SIZE        4096    // PCM data samples (i.e. short: 32Kb)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

typedef enum { MUSIC_AUDIO_OGG = 0, MUSIC_AUDIO_FLAC, MUSIC_MODULE_XM, MUSIC_MODULE_MOD } MusicContextType;

// Music type (file streaming from memory)
typedef struct MusicData {
    MusicContextType ctxType;           // Type of music context (OGG, XM, MOD)
    stb_vorbis *ctxOgg;                 // OGG audio context
    drflac *ctxFlac;                    // FLAC audio context
    jar_xm_context_t *ctxXm;            // XM chiptune context
    jar_mod_context_t ctxMod;           // MOD chiptune context

    AudioStream stream;                 // Audio stream (double buffering)

    bool loop;                          // Repeat music after finish (loop)
    unsigned int totalSamples;          // Total number of samples
    unsigned int samplesLeft;           // Number of samples left to end
} MusicData, *Music;

#if defined(AUDIO_STANDALONE)
typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(const char *fileName);          // Load WAV file
static Wave LoadOGG(const char *fileName);          // Load OGG file
static Wave LoadFLAC(const char *fileName);         // Load FLAC file

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

    TraceLog(INFO, "Audio device closed successfully");
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

// Load wave data from file into RAM
Wave LoadWave(const char *fileName)
{
    Wave wave = { 0 };

    if (strcmp(GetExtension(fileName), "wav") == 0) wave = LoadWAV(fileName);
    else if (strcmp(GetExtension(fileName), "ogg") == 0) wave = LoadOGG(fileName);
    else if (strcmp(GetExtension(fileName), "flac") == 0) wave = LoadFLAC(fileName);
    else TraceLog(WARNING, "[%s] File extension not recognized, it can't be loaded", fileName);

    return wave;
}

// Load wave data from float array data (32bit)
Wave LoadWaveEx(float *data, int sampleCount, int sampleRate, int sampleSize, int channels)
{
    Wave wave;
    
    wave.data = data;
    wave.sampleCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32;
    wave.channels = channels;
    
    // NOTE: Copy wave data to work with, 
    // user is responsible of input data to free
    Wave cwave = WaveCopy(wave);
    
    WaveFormat(&cwave, sampleRate, sampleSize, channels);
    
    return cwave;
}

// Load sound to memory
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
                case 32: format = AL_FORMAT_MONO_FLOAT32; break;
                default: TraceLog(WARNING, "Wave sample size not supported: %i", wave.sampleSize); break;
            }
        }
        else if (wave.channels == 2)
        {
            switch (wave.sampleSize)
            {
                case 8: format = AL_FORMAT_STEREO8; break;
                case 16: format = AL_FORMAT_STEREO16; break;
                case 32: format = AL_FORMAT_STEREO_FLOAT32; break;
                default: TraceLog(WARNING, "Wave sample size not supported: %i", wave.sampleSize); break;
            }
        }
        else TraceLog(WARNING, "Wave number of channels not supported: %i", wave.channels);

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

        unsigned int dataSize = wave.sampleCount*wave.sampleSize/8;    // Size in bytes

        // Upload sound data to buffer
        alBufferData(buffer, format, wave.data, dataSize, wave.sampleRate);

        // Attach sound buffer to source
        alSourcei(source, AL_BUFFER, buffer);

        TraceLog(INFO, "[SND ID %i][BUFR ID %i] Sound data loaded successfully (SampleRate: %i, SampleSize: %i, Channels: %i)", source, buffer, wave.sampleRate, wave.sampleSize, wave.channels);

        sound.source = source;
        sound.buffer = buffer;
        sound.format = format;
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
                        wave.sampleSize = bps;
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

// Unload Wave data
void UnloadWave(Wave wave)
{
    free(wave.data);

    TraceLog(INFO, "Unloaded wave data from RAM");
}

// Unload sound
void UnloadSound(Sound sound)
{
    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);

    TraceLog(INFO, "[SND ID %i][BUFR ID %i] Unloaded sound data from RAM", sound.source, sound.buffer);
}

// Update sound buffer with new data
// NOTE: data must match sound.format
void UpdateSound(Sound sound, void *data, int numSamples)
{
    ALint sampleRate, sampleSize, channels;
    alGetBufferi(sound.buffer, AL_FREQUENCY, &sampleRate);
    alGetBufferi(sound.buffer, AL_BITS, &sampleSize);       // It could also be retrieved from sound.format
    alGetBufferi(sound.buffer, AL_CHANNELS, &channels);     // It could also be retrieved from sound.format
    
    TraceLog(DEBUG, "UpdateSound() : AL_FREQUENCY: %i", sampleRate);
    TraceLog(DEBUG, "UpdateSound() : AL_BITS: %i", sampleSize);
    TraceLog(DEBUG, "UpdateSound() : AL_CHANNELS: %i", channels);

    unsigned int dataSize = numSamples*sampleSize/8;        // Size of data in bytes
    
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

    //TraceLog(INFO, "Playing sound");

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
// TODO: Consider channels (mono - stereo)
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels)
{
    if (wave->sampleSize != sampleSize)
    {
        float *samples = GetWaveData(*wave);   //Color *pixels = GetImageData(*image);

        free(wave->data);
        
        wave->sampleSize = sampleSize;

        //sample *= 4.0f;    // Arbitrary gain to get reasonable output volume...
        //if (sample > 1.0f) sample = 1.0f;
        //if (sample < -1.0f) sample = -1.0f;

        if (sampleSize == 8)
        {
            wave->data = (unsigned char *)malloc(wave->sampleCount*sizeof(unsigned char));

            for (int i = 0; i < wave->sampleCount; i++)
            {
                ((unsigned char *)wave->data)[i] = (unsigned char)((float)samples[i]*127 + 128);
            }
        }
        else if (sampleSize == 16)
        {
            wave->data = (short *)malloc(wave->sampleCount*sizeof(short));
            
            for (int i = 0; i < wave->sampleCount; i++)
            {
                ((short *)wave->data)[i] = (short)((float)samples[i]*32000);  // SHRT_MAX = 32767
            }
        }
        else if (sampleSize == 32)
        {
            wave->data = (float *)malloc(wave->sampleCount*sizeof(float));
            
            for (int i = 0; i < wave->sampleCount; i++)
            {
                ((float *)wave->data)[i] = (float)samples[i];
            }
        }
        else TraceLog(WARNING, "Wave formatting: Sample size not supported");
        
        free(samples);
    }
    
    // NOTE: Only supported 1 or 2 channels (mono or stereo)
    if ((channels > 0) && (channels < 3) && (wave->channels != channels))
    {
        // TODO: Add/remove channels interlaced data if required...
    }
}

// Copy a wave to a new wave
Wave WaveCopy(Wave wave)
{
    Wave newWave;

    if (wave.sampleSize == 8) newWave.data = (unsigned char *)malloc(wave.sampleCount*wave.channels*sizeof(unsigned char));
    else if (wave.sampleSize == 16) newWave.data = (short *)malloc(wave.sampleCount*wave.channels*sizeof(short));
    else if (wave.sampleSize == 32) newWave.data = (float *)malloc(wave.sampleCount*wave.channels*sizeof(float));
    else TraceLog(WARNING, "Wave sample size not supported for copy");

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
        // TODO: Review cropping (it could be simplified...)
        
        float *samples = GetWaveData(*wave);
        float *cropSamples = (float *)malloc((finalSample - initSample)*sizeof(float));
        
        for (int i = initSample; i < finalSample; i++) cropSamples[i] = samples[i];

        free(wave->data);
        wave->data = cropSamples;
        int sampleSize = wave->sampleSize;
        wave->sampleSize = 32;

        WaveFormat(wave, wave->sampleRate, sampleSize, wave->channels);
    }
    else TraceLog(WARNING, "Wave crop range out of bounds");
}

// Get samples data from wave as a floats array
// NOTE: Returned sample values are normalized to range [-1..1]
// TODO: Consider multiple channels (mono - stereo)
float *GetWaveData(Wave wave)
{
    float *samples = (float *)malloc(wave.sampleCount*sizeof(float));
    
    for (int i = 0; i < wave.sampleCount; i++)
    {
        if (wave.sampleSize == 8) samples[i] = (float)(((unsigned char *)wave.data)[i] - 127)/256.0f;
        else if (wave.sampleSize == 16) samples[i] = (float)((short *)wave.data)[i]/32767.0f;
        else if (wave.sampleSize == 32) samples[i] = ((float *)wave.data)[i];
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

    if (strcmp(GetExtension(fileName), "ogg") == 0)
    {
        // Open ogg audio stream
        music->ctxOgg = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (music->ctxOgg == NULL) TraceLog(WARNING, "[%s] OGG audio file could not be opened", fileName);
        else
        {
            stb_vorbis_info info = stb_vorbis_get_info(music->ctxOgg);  // Get Ogg file info
            //float totalLengthSeconds = stb_vorbis_stream_length_in_seconds(music->ctxOgg);

            // TODO: Support 32-bit sampleSize OGGs
            music->stream = InitAudioStream(info.sample_rate, 16, info.channels);
            music->totalSamples = (unsigned int)stb_vorbis_stream_length_in_samples(music->ctxOgg)*info.channels;
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_AUDIO_OGG;
            music->loop = true;                  // We loop by default

            TraceLog(DEBUG, "[%s] OGG sample rate: %i", fileName, info.sample_rate);
            TraceLog(DEBUG, "[%s] OGG channels: %i", fileName, info.channels);
            TraceLog(DEBUG, "[%s] OGG memory required: %i", fileName, info.temp_memory_required);

        }
    }
    else if (strcmp(GetExtension(fileName), "flac") == 0)
    {
        music->ctxFlac = drflac_open_file(fileName);
        
        if (music->ctxFlac == NULL) TraceLog(WARNING, "[%s] FLAC audio file could not be opened", fileName);
        else
        {
            music->stream = InitAudioStream(music->ctxFlac->sampleRate, music->ctxFlac->bitsPerSample, music->ctxFlac->channels);
            music->totalSamples = music->ctxFlac->totalSampleCount;
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_AUDIO_FLAC;
            music->loop = true;                  // We loop by default
            
            TraceLog(DEBUG, "[%s] FLAC sample rate: %i", fileName, music->ctxFlac->sampleRate);
            TraceLog(DEBUG, "[%s] FLAC bits per sample: %i", fileName, music->ctxFlac->bitsPerSample);
            TraceLog(DEBUG, "[%s] FLAC channels: %i", fileName, music->ctxFlac->channels);
        }
    }
    else if (strcmp(GetExtension(fileName), "xm") == 0)
    {
        int result = jar_xm_create_context_from_file(&music->ctxXm, 48000, fileName);

        if (!result)    // XM context created successfully
        {
            jar_xm_set_max_loop_count(music->ctxXm, 0);     // Set infinite number of loops

            // NOTE: Only stereo is supported for XM
            music->stream = InitAudioStream(48000, 32, 2);
            music->totalSamples = (unsigned int)jar_xm_get_remaining_samples(music->ctxXm);
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_MODULE_XM;
            music->loop = true;

            TraceLog(DEBUG, "[%s] XM number of samples: %i", fileName, music->totalSamples);
            TraceLog(DEBUG, "[%s] XM track length: %11.6f sec", fileName, (float)music->totalSamples/48000.0f);
        }
        else TraceLog(WARNING, "[%s] XM file could not be opened", fileName);
    }
    else if (strcmp(GetExtension(fileName), "mod") == 0)
    {
        jar_mod_init(&music->ctxMod);

        if (jar_mod_load_file(&music->ctxMod, fileName))
        {
            music->stream = InitAudioStream(48000, 16, 2);
            music->totalSamples = (unsigned int)jar_mod_max_samples(&music->ctxMod);
            music->samplesLeft = music->totalSamples;
            music->ctxType = MUSIC_MODULE_MOD;
            music->loop = true;

            TraceLog(INFO, "[%s] MOD number of samples: %i", fileName, music->samplesLeft);
            TraceLog(INFO, "[%s] MOD track length: %11.6f sec", fileName, (float)music->totalSamples/48000.0f);
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
    else if (music->ctxType == MUSIC_AUDIO_FLAC) drflac_free(music->ctxFlac);
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
// TODO: Restart XM context
void StopMusicStream(Music music)
{
    alSourceStop(music->stream.source);
    
    switch (music->ctxType)
    {
        case MUSIC_AUDIO_OGG: stb_vorbis_seek_start(music->ctxOgg); break;
        case MUSIC_MODULE_XM: break;
        case MUSIC_MODULE_MOD: jar_mod_seek_start(&music->ctxMod); break;
        default: break;
    }
    
    music->samplesLeft = music->totalSamples;
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(Music music)
{
    ALenum state;
    ALint processed = 0;
    
    alGetSourcei(music->stream.source, AL_SOURCE_STATE, &state);          // Get music stream state
    alGetSourcei(music->stream.source, AL_BUFFERS_PROCESSED, &processed); // Get processed buffers

    if (processed > 0)
    {
        bool active = true;
        short pcm[AUDIO_BUFFER_SIZE];
        float pcmf[AUDIO_BUFFER_SIZE];
        int numBuffersToProcess = processed;

        int numSamples = 0;     // Total size of data steamed in L+R samples for xm floats,
                                // individual L or R for ogg shorts

        for (int i = 0; i < numBuffersToProcess; i++)
        {
            switch (music->ctxType)
            {
                case MUSIC_AUDIO_OGG:
                {
                    if (music->samplesLeft >= AUDIO_BUFFER_SIZE) numSamples = AUDIO_BUFFER_SIZE;
                    else numSamples = music->samplesLeft;

                    // NOTE: Returns the number of samples to process (should be the same as numSamples -> it is)
                    int numSamplesOgg = stb_vorbis_get_samples_short_interleaved(music->ctxOgg, music->stream.channels, pcm, numSamples);

                    // TODO: Review stereo channels Ogg, not enough samples served!
                    UpdateAudioStream(music->stream, pcm, numSamplesOgg*music->stream.channels);
                    music->samplesLeft -= (numSamplesOgg*music->stream.channels);

                } break;
                case MUSIC_AUDIO_FLAC:
                {
                    if (music->samplesLeft >= AUDIO_BUFFER_SIZE) numSamples = AUDIO_BUFFER_SIZE;
                    else numSamples = music->samplesLeft;
                    
                    int pcmi[AUDIO_BUFFER_SIZE];
                    
                    // NOTE: Returns the number of samples to process (should be the same as numSamples)
                    int numSamplesFlac = drflac_read_s32(music->ctxFlac, numSamples, pcmi);

                    UpdateAudioStream(music->stream, pcmi, numSamples*music->stream.channels);
                    music->samplesLeft -= (numSamples*music->stream.channels);

                } break;
                case MUSIC_MODULE_XM:
                {
                    if (music->samplesLeft >= AUDIO_BUFFER_SIZE/2) numSamples = AUDIO_BUFFER_SIZE/2;
                    else numSamples = music->samplesLeft;

                    // NOTE: Output buffer is 2*numsamples elements (left and right value for each sample)
                    jar_xm_generate_samples(music->ctxXm, pcmf, numSamples);
                    UpdateAudioStream(music->stream, pcmf, numSamples*2);           // Using 32bit PCM data
                    music->samplesLeft -= numSamples;

                    //TraceLog(INFO, "Samples left: %i", music->samplesLeft);

                } break;
                case MUSIC_MODULE_MOD:
                {
                    if (music->samplesLeft >= AUDIO_BUFFER_SIZE/2) numSamples = AUDIO_BUFFER_SIZE/2;
                    else numSamples = music->samplesLeft;

                    // NOTE: Output buffer size is nbsample*channels (default: 48000Hz, 16bit, Stereo)
                    jar_mod_fillbuffer(&music->ctxMod, pcm, numSamples, 0);
                    UpdateAudioStream(music->stream, pcm, numSamples*2);
                    music->samplesLeft -= numSamples;

                } break;
                default: break;
            }

            if (music->samplesLeft <= 0)
            {
                active = false;
                break;
            }
        }
        
        // This error is registered when UpdateAudioStream() fails
        if (alGetError() == AL_INVALID_VALUE) TraceLog(WARNING, "OpenAL: Error buffering data...");

        // Reset audio stream for looping
        if (!active) 
        {
            StopMusicStream(music);        // Stop music (and reset)
            
            if (music->loop) PlayMusicStream(music);    // Play again
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
    secondsPlayed = (float)samplesPlayed/(music->stream.sampleRate*music->stream.channels);

    return secondsPlayed;
}

// Init audio stream (to stream audio pcm data)
AudioStream InitAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels)
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

    // Create Buffers (double buffering)
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

    TraceLog(INFO, "[AUD ID %i] Unloaded audio stream data", stream.source);
}

// Update audio stream buffers with data
// NOTE: Only one buffer per call
void UpdateAudioStream(AudioStream stream, void *data, int numSamples)
{
    ALuint buffer = 0;
    alSourceUnqueueBuffers(stream.source, 1, &buffer);

    // Check if any buffer was available for unqueue
    if (alGetError() != AL_INVALID_VALUE)
    {
        if (stream.sampleSize == 8) alBufferData(buffer, stream.format, (unsigned char *)data, numSamples*sizeof(unsigned char), stream.sampleRate);
        else if (stream.sampleSize == 16) alBufferData(buffer, stream.format, (short *)data, numSamples*sizeof(short), stream.sampleRate);
        else if (stream.sampleSize == 32) alBufferData(buffer, stream.format, (float *)data, numSamples*sizeof(float), stream.sampleRate);

        alSourceQueueBuffers(stream.source, 1, &buffer);
    }
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
                    wave.data = (unsigned char *)malloc(sizeof(unsigned char)*waveData.subChunkSize);

                    // Read in the sound data into the soundData variable
                    fread(wave.data, waveData.subChunkSize, 1, wavFile);

                    // Now we set the variables that we need later
                    wave.sampleCount = waveData.subChunkSize;
                    wave.sampleRate = waveFormat.sampleRate;
                    wave.sampleSize = waveFormat.bitsPerSample;
                    wave.channels = waveFormat.numChannels;

                    TraceLog(INFO, "[%s] WAV file loaded successfully (SampleRate: %i, SampleSize: %i, Channels: %i)", fileName, wave.sampleRate, wave.sampleSize, wave.channels);
                }
            }
        }

        fclose(wavFile);
    }

    return wave;
}

// Load OGG file into Wave structure
// NOTE: Using stb_vorbis library
static Wave LoadOGG(const char *fileName)
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
        wave.sampleSize = 16;                   // 16 bit per sample (short)
        wave.channels = info.channels;

        int totalSamplesLength = (stb_vorbis_stream_length_in_samples(oggFile)*info.channels);
        float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);

        if (totalSeconds > 10) TraceLog(WARNING, "[%s] Ogg audio lenght is larger than 10 seconds (%f), that's a big file in memory, consider music streaming", fileName, totalSeconds);

        int totalSamples = totalSeconds*info.sample_rate*info.channels;
        wave.sampleCount = totalSamples;

        wave.data = (short *)malloc(totalSamplesLength*sizeof(short));

        int samplesObtained = stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, (short *)wave.data, totalSamplesLength);

        TraceLog(DEBUG, "[%s] Samples obtained: %i", fileName, samplesObtained);

        TraceLog(INFO, "[%s] OGG file loaded successfully (SampleRate: %i, SampleSize: %i, Channels: %i)", fileName, wave.sampleRate, wave.sampleSize, wave.channels);

        stb_vorbis_close(oggFile);
    }

    return wave;
}

// Load FLAC file into Wave structure
// NOTE: Using dr_flac library
static Wave LoadFLAC(const char *fileName)
{
    Wave wave;

    // Decode an entire FLAC file in one go
    uint64_t totalSampleCount;
    wave.data = drflac_open_and_decode_file_s32(fileName, &wave.channels, &wave.sampleRate, &totalSampleCount);
    
    wave.sampleCount = (int)totalSampleCount;
    wave.sampleSize = 32;
    
    if (wave.data == NULL) TraceLog(WARNING, "[%s] FLAC data could not be loaded", fileName);
    
    return wave;
}

// Some required functions for audio standalone module version
#if defined(AUDIO_STANDALONE)
// Get the extension for a filename
const char *GetExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');
    if (!dot || dot == fileName) return "";
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

    switch (msgType)
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