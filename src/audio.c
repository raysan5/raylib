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

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_STREAM_BUFFERS          2             // Number of buffers for each source
#define MAX_MUSIC_STREAMS           2             // Number of simultanious music sources
#define MAX_MIX_CHANNELS            4             // Number of mix channels (OpenAL sources)

#if defined(PLATFORM_RPI) || defined(PLATFORM_ANDROID)
    // NOTE: On RPI and Android should be lower to avoid frame-stalls
    #define MUSIC_BUFFER_SIZE_SHORT      4096*2   // PCM data buffer (short) - 16Kb (RPI)
    #define MUSIC_BUFFER_SIZE_FLOAT      4096     // PCM data buffer (float) - 16Kb (RPI)
#else
    // NOTE: On HTML5 (emscripten) this is allocated on heap, by default it's only 16MB!...just take care...
    #define MUSIC_BUFFER_SIZE_SHORT      4096*8   // PCM data buffer (short) - 64Kb
    #define MUSIC_BUFFER_SIZE_FLOAT      4096*4   // PCM data buffer (float) - 64Kb
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Used to create custom audio streams that are not bound to a specific file. 
// There can be no more than 4 concurrent mixchannels in use. 
// This is due to each active mixc being tied to a dedicated mix channel.
typedef struct MixChannel {
    unsigned short sampleRate;           // default is 48000
    unsigned char channels;              // 1=mono,2=stereo
    unsigned char mixChannel;            // 0-3 or mixA-mixD, each mix channel can receive up to one dedicated audio stream
    bool floatingPoint;                  // if false then the short datatype is used instead
    bool playing;                        // false if paused
    
    ALenum alFormat;                     // OpenAL format specifier
    ALuint alSource;                     // OpenAL source
    ALuint alBuffer[MAX_STREAM_BUFFERS]; // OpenAL sample buffer
} MixChannel;

// Music type (file streaming from memory)
// NOTE: Anything longer than ~10 seconds should be streamed into a mix channel...
typedef struct Music {
    stb_vorbis *stream;
    jar_xm_context_t *xmctx;    // XM chiptune context
    jar_mod_context_t modctx;   // MOD chiptune context
    MixChannel *mixc;           // Mix channel
    
    unsigned int totalSamplesLeft;
    float totalLengthSeconds;
    bool loop;
    bool chipTune;              // chiptune is loaded?
    bool enabled;
} Music;

// Audio errors registered
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
    ERROR_UNINITIALIZED_CHANNELS    = 16384
} AudioError;

#if defined(AUDIO_STANDALONE)
typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Music musicStreams[MAX_MUSIC_STREAMS];            // Current music loaded, up to two can play at the same time
static MixChannel *mixChannels[MAX_MIX_CHANNELS];        // Mix channels currently active (from music streams)

static int lastAudioError = 0;                           // Registers last audio error

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(const char *fileName);         // Load WAV file
static Wave LoadOGG(char *fileName);               // Load OGG file
static void UnloadWave(Wave wave);                 // Unload wave data

static bool BufferMusicStream(int index, int numBuffers); // Fill music buffers with data
static void EmptyMusicStream(int index);                  // Empty music buffers

static MixChannel *InitMixChannel(unsigned short sampleRate, unsigned char mixChannel, unsigned char channels, bool floatingPoint);
static void CloseMixChannel(MixChannel *mixc); // Frees mix channel
static int BufferMixChannel(MixChannel *mixc, void *data, int numberElements); // Pushes more audio data into mix channel
//static void ResampleShortToFloat(short *shorts, float *floats, unsigned short len); // Pass two arrays of the same legnth in
//static void ResampleByteToFloat(char *chars, float *floats, unsigned short len); // Pass two arrays of same length in

#if defined(AUDIO_STANDALONE)
const char *GetExtension(const char *fileName);     // Get the extension for a filename
void TraceLog(int msgType, const char *text, ...);  // Outputs a trace log message (INFO, ERROR, WARNING)
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------

// Initialize audio device and mixc
void InitAudioDevice(void)
{
    // Open and initialize a device with default settings
    ALCdevice *device = alcOpenDevice(NULL);

    if (!device) TraceLog(ERROR, "Audio device could not be opened");

    ALCcontext *context = alcCreateContext(device, NULL);

    if ((context == NULL) || (alcMakeContextCurrent(context) == ALC_FALSE))
    {
        if (context != NULL) alcDestroyContext(context);

        alcCloseDevice(device);

        TraceLog(ERROR, "Could not setup mix channel");
    }

    TraceLog(INFO, "Audio device and context initialized successfully: %s", alcGetString(device, ALC_DEVICE_SPECIFIER));

    // Listener definition (just for 2D)
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListener3f(AL_ORIENTATION, 0, 0, -1);
}

// Close the audio device for all contexts
void CloseAudioDevice(void)
{
    for (int index = 0; index < MAX_MUSIC_STREAMS; index++)
    {
        if (musicStreams[index].mixc) StopMusicStream(index);      // Stop music streaming and close current stream
    }

    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) TraceLog(WARNING, "Could not get current mix channel for closing");

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
// Module Functions Definition - Custom audio output
//----------------------------------------------------------------------------------

// Init mix channel for streaming
// The mixChannel is what audio muxing channel you want to operate on, 0-3 are the ones available. 
// Each mix channel can only be used one at a time.
static MixChannel *InitMixChannel(unsigned short sampleRate, unsigned char mixChannel, unsigned char channels, bool floatingPoint)
{
    if (mixChannel >= MAX_MIX_CHANNELS) return NULL;
    if (!IsAudioDeviceReady()) InitAudioDevice();
    
    if (!mixChannels[mixChannel])
    {
        MixChannel *mixc = (MixChannel *)malloc(sizeof(MixChannel));
        mixc->sampleRate = sampleRate;
        mixc->channels = channels;
        mixc->mixChannel = mixChannel;
        mixc->floatingPoint = floatingPoint;
        mixChannels[mixChannel] = mixc;
        
        // Setup OpenAL format
        if (channels == 1)
        {
            if (floatingPoint) mixc->alFormat = AL_FORMAT_MONO_FLOAT32;
            else mixc->alFormat = AL_FORMAT_MONO16;
        }
        else if (channels == 2)
        {
            if (floatingPoint) mixc->alFormat = AL_FORMAT_STEREO_FLOAT32;
            else mixc->alFormat = AL_FORMAT_STEREO16;
        }
        
        // Create an audio source
        alGenSources(1, &mixc->alSource);
        alSourcef(mixc->alSource, AL_PITCH, 1);
        alSourcef(mixc->alSource, AL_GAIN, 1);
        alSource3f(mixc->alSource, AL_POSITION, 0, 0, 0);
        alSource3f(mixc->alSource, AL_VELOCITY, 0, 0, 0);
        
        // Create Buffer
        alGenBuffers(MAX_STREAM_BUFFERS, mixc->alBuffer);
        
        // Fill buffers
        for (int i = 0; i < MAX_STREAM_BUFFERS; i++)
        {
            // Initialize buffer with zeros by default
            if (mixc->floatingPoint)
            {
                float pcm[MUSIC_BUFFER_SIZE_FLOAT] = { 0.0f };
                alBufferData(mixc->alBuffer[i], mixc->alFormat, pcm, MUSIC_BUFFER_SIZE_FLOAT*sizeof(float), mixc->sampleRate);
            }
            else
            {
                short pcm[MUSIC_BUFFER_SIZE_SHORT] = { 0 };
                alBufferData(mixc->alBuffer[i], mixc->alFormat, pcm, MUSIC_BUFFER_SIZE_SHORT*sizeof(short), mixc->sampleRate);
            }
        }
        
        alSourceQueueBuffers(mixc->alSource, MAX_STREAM_BUFFERS, mixc->alBuffer);
        mixc->playing = true;
        alSourcePlay(mixc->alSource);
        
        return mixc;
    }
    
    return NULL;
}

// Frees buffer in mix channel
static void CloseMixChannel(MixChannel *mixc)
{
    if (mixc)
    {
        alSourceStop(mixc->alSource);
        mixc->playing = false;
        
        // Flush out all queued buffers
        ALuint buffer = 0;
        int queued = 0;
        alGetSourcei(mixc->alSource, AL_BUFFERS_QUEUED, &queued);
        
        while (queued > 0)
        {
            alSourceUnqueueBuffers(mixc->alSource, 1, &buffer);
            queued--;
        }
        
        // Delete source and buffers
        alDeleteSources(1, &mixc->alSource);
        alDeleteBuffers(MAX_STREAM_BUFFERS, mixc->alBuffer);
        mixChannels[mixc->mixChannel] = NULL;
        free(mixc);
        mixc = NULL;
    }
}

// Pushes more audio data into mix channel, only one buffer per call
// Call "BufferMixChannel(mixc, NULL, 0)" if you want to pause the audio.
// Returns number of samples that where processed.
static int BufferMixChannel(MixChannel *mixc, void *data, int numberElements)
{
    if (!mixc || (mixChannels[mixc->mixChannel] != mixc)) return 0; // When there is two channels there must be an even number of samples
    
    if (!data || !numberElements)   
    { 
        // Pauses audio until data is given
        if (mixc->playing)
        {
            alSourcePause(mixc->alSource);
            mixc->playing = false;
        }
        
        return 0;
    }
    else if (!mixc->playing)
    { 
        // Restart audio otherwise
        alSourcePlay(mixc->alSource);
        mixc->playing = true;
    }

    ALuint buffer = 0;
    
    alSourceUnqueueBuffers(mixc->alSource, 1, &buffer);
    if (!buffer) return 0;
    
    if (mixc->floatingPoint)
    {
        // Process float buffers
        float *ptr = (float *)data;
        alBufferData(buffer, mixc->alFormat, ptr, numberElements*sizeof(float), mixc->sampleRate);
    }
    else
    {
        // Process short buffers
        short *ptr = (short *)data;
        alBufferData(buffer, mixc->alFormat, ptr, numberElements*sizeof(short), mixc->sampleRate);
    }
    
    alSourceQueueBuffers(mixc->alSource, 1, &buffer);
    
    return numberElements;
}

/*
// Convert data from short to float
// example usage:
//      short sh[3] = {1,2,3};float fl[3];
//      ResampleShortToFloat(sh,fl,3);
static void ResampleShortToFloat(short *shorts, float *floats, unsigned short len)
{
    for (int i = 0; i < len; i++)
    {
        if (shorts[i] < 0) floats[i] = (float)shorts[i]/32766.0f;
        else floats[i] = (float)shorts[i]/32767.0f;
    }
}

// Convert data from float to short
// example usage:
//      char ch[3] = {1,2,3};float fl[3];
//      ResampleByteToFloat(ch,fl,3);
static void ResampleByteToFloat(char *chars, float *floats, unsigned short len)
{
    for (int i = 0; i < len; i++)
    {
        if (chars[i] < 0) floats[i] = (float)chars[i]/127.0f;
        else floats[i] = (float)chars[i]/128.0f;
    }
}
*/

// Initialize raw audio mix channel for audio buffering
// NOTE: Returns mix channel index or -1 if it fails (errors are registered on lastAudioError)
int InitRawMixChannel(int sampleRate, int channels, bool floatingPoint)
{
    int mixIndex;

    for (mixIndex = 0; mixIndex < MAX_MIX_CHANNELS; mixIndex++) // find empty mix channel slot
    {
        if (mixChannels[mixIndex] == NULL) break;
        else if (mixIndex == (MAX_MIX_CHANNELS - 1))
        {
            lastAudioError = ERROR_OUT_OF_MIX_CHANNELS;
            return -1;
        }
    }
    
    if (InitMixChannel(sampleRate, mixIndex, channels, floatingPoint)) return mixIndex;
    else
    {
        lastAudioError = ERROR_RAW_CONTEXT_CREATION;
        return -1;
    }
}

// Buffers data directly to raw mix channel
// if 0 is returned, buffers are still full and you need to keep trying with the same data
// otherwise it will return number of samples buffered.
// NOTE: Data could be either be an array of floats or shorts, depending on the created context
int BufferRawAudioContext(int ctx, void *data, unsigned short numberElements)
{
    int numBuffered = 0;
    
    if (ctx >= 0)
    {
        MixChannel *mixc = mixChannels[ctx];
        numBuffered = BufferMixChannel(mixc, data, numberElements);
    }
    
    return numBuffered;
}

// Closes and frees raw mix channel
void CloseRawAudioContext(int ctx)
{
    if (mixChannels[ctx]) CloseMixChannel(mixChannels[ctx]);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Sounds loading and playing (.WAV)
//----------------------------------------------------------------------------------

// Load sound to memory
Sound LoadSound(char *fileName)
{
    Sound sound = { 0 };
    Wave wave = { 0 };

    // NOTE: The entire file is loaded to memory to play it all at once (no-streaming)

    // Audio file loading
    // NOTE: Buffer space is allocated inside function, Wave must be freed

    if (strcmp(GetExtension(fileName),"wav") == 0) wave = LoadWAV(fileName);
    else if (strcmp(GetExtension(fileName),"ogg") == 0) wave = LoadOGG(fileName);
    else
    {
        TraceLog(WARNING, "[%s] Sound extension not recognized, it can't be loaded", fileName);
        
        // TODO: Find a better way to register errors (similar to glGetError())
        lastAudioError = ERROR_EXTENSION_NOT_RECOGNIZED;
    }

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

        TraceLog(INFO, "[%s] Sound file loaded successfully (SampleRate: %i, BitRate: %i, Channels: %i)", fileName, wave.sampleRate, wave.bitsPerSample, wave.channels);

        // Unallocate WAV data
        UnloadWave(wave);

        sound.source = source;
        sound.buffer = buffer;
    }

    return sound;
}

// Load sound from wave data
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

        // Unallocate WAV data
        UnloadWave(wave);

        TraceLog(INFO, "[Wave] Sound file loaded successfully (SampleRate: %i, BitRate: %i, Channels: %i)", wave.sampleRate, wave.bitsPerSample, wave.channels);

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

    if (rresFile == NULL)
    {
        TraceLog(WARNING, "[%s] rRES raylib resource file could not be opened", rresName);
        lastAudioError = ERROR_UNABLE_TO_OPEN_RRES_FILE;
    }
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
            lastAudioError = ERROR_INVALID_RRES_FILE;
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

                        // Convert wave to Sound (OpenAL)
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
                        alBufferData(buffer, format, (void*)wave.data, wave.dataSize, wave.sampleRate);

                        // Attach sound buffer to source
                        alSourcei(source, AL_BUFFER, buffer);

                        TraceLog(INFO, "[%s] Sound loaded successfully from resource (SampleRate: %i, BitRate: %i, Channels: %i)", rresName, wave.sampleRate, wave.bitsPerSample, wave.channels);

                        // Unallocate WAV data
                        UnloadWave(wave);

                        sound.source = source;
                        sound.buffer = buffer;
                    }
                    else
                    {
                        TraceLog(WARNING, "[%s] Required resource do not seem to be a valid SOUND resource", rresName);
                        lastAudioError = ERROR_INVALID_RRES_RESOURCE;
                    }
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
    
    TraceLog(INFO, "Unloaded sound data");
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

// Start music playing (open stream)
// returns 0 on success or error code
int PlayMusicStream(int index, char *fileName)
{
    int mixIndex;
    
    if (musicStreams[index].stream || musicStreams[index].xmctx) return ERROR_UNINITIALIZED_CHANNELS; // error
    
    for (mixIndex = 0; mixIndex < MAX_MIX_CHANNELS; mixIndex++) // find empty mix channel slot
    {
        if (mixChannels[mixIndex] == NULL) break;
        else if (mixIndex == (MAX_MIX_CHANNELS - 1)) return ERROR_OUT_OF_MIX_CHANNELS; // error
    }
    
    if (strcmp(GetExtension(fileName),"ogg") == 0)
    {
        // Open audio stream
        musicStreams[index].stream = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (musicStreams[index].stream == NULL)
        {
            TraceLog(WARNING, "[%s] OGG audio file could not be opened", fileName);
            return ERROR_LOADING_OGG; // error
        }
        else
        {
            // Get file info
            stb_vorbis_info info = stb_vorbis_get_info(musicStreams[index].stream);

            TraceLog(INFO, "[%s] Ogg sample rate: %i", fileName, info.sample_rate);
            TraceLog(INFO, "[%s] Ogg channels: %i", fileName, info.channels);
            TraceLog(DEBUG, "[%s] Temp memory required: %i", fileName, info.temp_memory_required);

            musicStreams[index].loop = true;                  // We loop by default
            musicStreams[index].enabled = true;
            

            musicStreams[index].totalSamplesLeft = (unsigned int)stb_vorbis_stream_length_in_samples(musicStreams[index].stream) * info.channels;
            musicStreams[index].totalLengthSeconds = stb_vorbis_stream_length_in_seconds(musicStreams[index].stream);
            
            if (info.channels == 2)
            {
                musicStreams[index].mixc = InitMixChannel(info.sample_rate, mixIndex, 2, false);
                musicStreams[index].mixc->playing = true;
            }
            else
            {
                musicStreams[index].mixc = InitMixChannel(info.sample_rate, mixIndex, 1, false);
                musicStreams[index].mixc->playing = true;
            }
            
            if (!musicStreams[index].mixc) return ERROR_LOADING_OGG; // error
        }
    }
    else if (strcmp(GetExtension(fileName),"xm") == 0)
    {
        // only stereo is supported for xm
        if (!jar_xm_create_context_from_file(&musicStreams[index].xmctx, 48000, fileName))
        {
            musicStreams[index].chipTune = true;
            musicStreams[index].loop = true;
            jar_xm_set_max_loop_count(musicStreams[index].xmctx, 0); // infinite number of loops
            musicStreams[index].totalSamplesLeft =  (unsigned int)jar_xm_get_remaining_samples(musicStreams[index].xmctx);
            musicStreams[index].totalLengthSeconds = ((float)musicStreams[index].totalSamplesLeft)/48000.0f;
            musicStreams[index].enabled = true;
            
            TraceLog(INFO, "[%s] XM number of samples: %i", fileName, musicStreams[index].totalSamplesLeft);
            TraceLog(INFO, "[%s] XM track length: %11.6f sec", fileName, musicStreams[index].totalLengthSeconds);
            
            musicStreams[index].mixc = InitMixChannel(48000, mixIndex, 2, true);
            
            if (!musicStreams[index].mixc) return ERROR_XM_CONTEXT_CREATION; // error
            
            musicStreams[index].mixc->playing = true;
        }
        else
        {
            TraceLog(WARNING, "[%s] XM file could not be opened", fileName);
            return ERROR_LOADING_XM; // error
        }
    }
    else if (strcmp(GetExtension(fileName),"mod") == 0)
    {
        jar_mod_init(&musicStreams[index].modctx);
        
        if (jar_mod_load_file(&musicStreams[index].modctx, fileName))
        {
            musicStreams[index].chipTune = true;
            musicStreams[index].loop = true;
            musicStreams[index].totalSamplesLeft = (unsigned int)jar_mod_max_samples(&musicStreams[index].modctx);
            musicStreams[index].totalLengthSeconds = ((float)musicStreams[index].totalSamplesLeft)/48000.0f;
            musicStreams[index].enabled = true;
            
            TraceLog(INFO, "[%s] MOD number of samples: %i", fileName, musicStreams[index].totalSamplesLeft);
            TraceLog(INFO, "[%s] MOD track length: %11.6f sec", fileName, musicStreams[index].totalLengthSeconds);
            
            musicStreams[index].mixc = InitMixChannel(48000, mixIndex, 2, false);
            
            if (!musicStreams[index].mixc) return ERROR_MOD_CONTEXT_CREATION; // error
            
            musicStreams[index].mixc->playing = true;
        }
        else
        {
            TraceLog(WARNING, "[%s] MOD file could not be opened", fileName);
            return ERROR_LOADING_MOD; // error
        }
    }
    else
    {
        TraceLog(WARNING, "[%s] Music extension not recognized, it can't be loaded", fileName);
        return ERROR_EXTENSION_NOT_RECOGNIZED; // error
    }
    
    return 0; // normal return
}

// Stop music playing for individual music index of musicStreams array (close stream)
void StopMusicStream(int index)
{
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc)
    {
        CloseMixChannel(musicStreams[index].mixc);
        
        if (musicStreams[index].xmctx)
            jar_xm_free_context(musicStreams[index].xmctx);
        else if (musicStreams[index].modctx.mod_loaded)
            jar_mod_unload(&musicStreams[index].modctx);
        else
            stb_vorbis_close(musicStreams[index].stream);
        
        musicStreams[index].enabled = false;
        
        if (musicStreams[index].stream || musicStreams[index].xmctx)
        {
            musicStreams[index].stream = NULL;
            musicStreams[index].xmctx = NULL;
        }
    }
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(int index)
{
    ALenum state;
    bool active = true;
    ALint processed = 0;
    
    // Determine if music stream is ready to be written
    alGetSourcei(musicStreams[index].mixc->alSource, AL_BUFFERS_PROCESSED, &processed);
    
    if (musicStreams[index].mixc->playing && (index < MAX_MUSIC_STREAMS) && musicStreams[index].enabled && musicStreams[index].mixc && (processed > 0))
    {
        active = BufferMusicStream(index, processed);
        
        if (!active && musicStreams[index].loop)
        {
            if (musicStreams[index].chipTune)
            {
                if(musicStreams[index].modctx.mod_loaded) jar_mod_seek_start(&musicStreams[index].modctx);
                
                musicStreams[index].totalSamplesLeft = musicStreams[index].totalLengthSeconds*48000.0f;
            }
            else
            {
                stb_vorbis_seek_start(musicStreams[index].stream);
                musicStreams[index].totalSamplesLeft = stb_vorbis_stream_length_in_samples(musicStreams[index].stream)*musicStreams[index].mixc->channels;
            }
            
            // Determine if music stream is ready to be written
            alGetSourcei(musicStreams[index].mixc->alSource, AL_BUFFERS_PROCESSED, &processed);
            
            active = BufferMusicStream(index, processed);
        }

        if (alGetError() != AL_NO_ERROR) TraceLog(WARNING, "Error buffering data...");
        
        alGetSourcei(musicStreams[index].mixc->alSource, AL_SOURCE_STATE, &state);

        if (state != AL_PLAYING && active) alSourcePlay(musicStreams[index].mixc->alSource);

        if (!active) StopMusicStream(index);
        
    }
}

//get number of music channels active at this time, this does not mean they are playing
int GetMusicStreamCount(void)
{
    int musicCount = 0;
    
    // Find empty music slot
    for (int musicIndex = 0; musicIndex < MAX_MUSIC_STREAMS; musicIndex++)
    {
        if(musicStreams[musicIndex].stream != NULL || musicStreams[musicIndex].chipTune) musicCount++;
    }
    
    return musicCount;
}

// Pause music playing
void PauseMusicStream(int index)
{
    // Pause music stream if music available!
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc && musicStreams[index].enabled)
    {
        TraceLog(INFO, "Pausing music stream");
        alSourcePause(musicStreams[index].mixc->alSource);
        musicStreams[index].mixc->playing = false;
    }
}

// Resume music playing
void ResumeMusicStream(int index)
{
    // Resume music playing... if music available!
    ALenum state;
    
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc)
    {
        alGetSourcei(musicStreams[index].mixc->alSource, AL_SOURCE_STATE, &state);
        
        if (state == AL_PAUSED)
        {
            TraceLog(INFO, "Resuming music stream");
            alSourcePlay(musicStreams[index].mixc->alSource);
            musicStreams[index].mixc->playing = true;
        }
    }
}

// Check if any music is playing
bool IsMusicPlaying(int index)
{
    bool playing = false;
    ALint state;
    
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc)
    {
        alGetSourcei(musicStreams[index].mixc->alSource, AL_SOURCE_STATE, &state);
        
        if (state == AL_PLAYING) playing = true;
    }

    return playing;
}

// Set volume for music
void SetMusicVolume(int index, float volume)
{
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc)
    {
        alSourcef(musicStreams[index].mixc->alSource, AL_GAIN, volume);
    }
}

// Set pitch for music
void SetMusicPitch(int index, float pitch)
{
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc)
    {
        alSourcef(musicStreams[index].mixc->alSource, AL_PITCH, pitch);
    }
}

// Get music time length (in seconds)
float GetMusicTimeLength(int index)
{
    float totalSeconds;
    
    if (musicStreams[index].chipTune) totalSeconds = (float)musicStreams[index].totalLengthSeconds;
    else totalSeconds = stb_vorbis_stream_length_in_seconds(musicStreams[index].stream);

    return totalSeconds;
}

// Get current music time played (in seconds)
float GetMusicTimePlayed(int index)
{
    float secondsPlayed = 0.0f;
    
    if (index < MAX_MUSIC_STREAMS && musicStreams[index].mixc)
    {
        if (musicStreams[index].chipTune && musicStreams[index].xmctx)
        {
            uint64_t samples;
            jar_xm_get_position(musicStreams[index].xmctx, NULL, NULL, NULL, &samples);
            secondsPlayed = (float)samples/(48000.0f*musicStreams[index].mixc->channels); // Not sure if this is the correct value
        }
        else if(musicStreams[index].chipTune && musicStreams[index].modctx.mod_loaded)
        {
            long numsamp = jar_mod_current_samples(&musicStreams[index].modctx);
            secondsPlayed = (float)numsamp/(48000.0f);
        }
        else
        {
            int totalSamples = stb_vorbis_stream_length_in_samples(musicStreams[index].stream)*musicStreams[index].mixc->channels;
            int samplesPlayed = totalSamples - musicStreams[index].totalSamplesLeft;
            secondsPlayed = (float)samplesPlayed/(musicStreams[index].mixc->sampleRate*musicStreams[index].mixc->channels);
        }
    }

    return secondsPlayed;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Fill music buffers with new data from music stream
static bool BufferMusicStream(int index, int numBuffers)
{
    short pcm[MUSIC_BUFFER_SIZE_SHORT];
    float pcmf[MUSIC_BUFFER_SIZE_FLOAT];
    
    int size = 0;              // Total size of data steamed in L+R samples for xm floats, individual L or R for ogg shorts
    bool active = true;        // We can get more data from stream (not finished)
    
    if (musicStreams[index].chipTune) // There is no end of stream for xmfiles, once the end is reached zeros are generated for non looped chiptunes.
    {
        for (int i = 0; i < numBuffers; i++)
        {
            if (musicStreams[index].modctx.mod_loaded)
            {
                if (musicStreams[index].totalSamplesLeft >= MUSIC_BUFFER_SIZE_SHORT) size = MUSIC_BUFFER_SIZE_SHORT/2;
                else size = musicStreams[index].totalSamplesLeft/2;
                
                jar_mod_fillbuffer(&musicStreams[index].modctx, pcm, size, 0 );
                BufferMixChannel(musicStreams[index].mixc, pcm, size*2);
            }
            else if (musicStreams[index].xmctx)
            {
                if (musicStreams[index].totalSamplesLeft >= MUSIC_BUFFER_SIZE_FLOAT) size = MUSIC_BUFFER_SIZE_FLOAT/2;
                else size = musicStreams[index].totalSamplesLeft/2;
                
                jar_xm_generate_samples(musicStreams[index].xmctx, pcmf, size); // reads 2*readlen shorts and moves them to buffer+size memory location
                BufferMixChannel(musicStreams[index].mixc, pcmf, size*2);
            }

            musicStreams[index].totalSamplesLeft -= size;
            
            if (musicStreams[index].totalSamplesLeft <= 0)
            {
                active = false;
                break;
            }
        }
    }
    else
    {
        if (musicStreams[index].totalSamplesLeft >= MUSIC_BUFFER_SIZE_SHORT) size = MUSIC_BUFFER_SIZE_SHORT;
        else size = musicStreams[index].totalSamplesLeft;
        
        for (int i = 0; i < numBuffers; i++)
        {
            int streamedBytes = stb_vorbis_get_samples_short_interleaved(musicStreams[index].stream, musicStreams[index].mixc->channels, pcm, size);
            BufferMixChannel(musicStreams[index].mixc, pcm, streamedBytes * musicStreams[index].mixc->channels);
            musicStreams[index].totalSamplesLeft -= streamedBytes * musicStreams[index].mixc->channels;
            
            if (musicStreams[index].totalSamplesLeft <= 0)
            {
                active = false;
                break;
            }
        }
    }

    return active;
}

// Empty music buffers
static void EmptyMusicStream(int index)
{
    ALuint buffer = 0;
    int queued = 0;

    alGetSourcei(musicStreams[index].mixc->alSource, AL_BUFFERS_QUEUED, &queued);

    while (queued > 0)
    {
        alSourceUnqueueBuffers(musicStreams[index].mixc->alSource, 1, &buffer);

        queued--;
    }
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

        int totalSamplesLength = (stb_vorbis_stream_length_in_samples(oggFile) * info.channels);

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
    
    TraceLog(INFO, "Unloaded wave data");
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