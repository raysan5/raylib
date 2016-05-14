/**********************************************************************************************
*
*   raylib.audio
*
*   Basic functions to manage Audio: InitAudioDevice, LoadAudioFiles, PlayAudioFiles
*
*   Uses external lib:
*       OpenAL Soft - Audio device management lib (http://kcat.strangesoft.net/openal.html)
*       stb_vorbis - Ogg audio files loading (http://www.nothings.org/stb_vorbis/)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
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

#include "AL/al.h"          // OpenAL basic header
#include "AL/alc.h"         // OpenAL context header (like OpenGL, OpenAL requires a context to work)
#include "AL/alext.h"       // extensions for other format types

#include <stdlib.h>         // Declares malloc() and free() for memory management
#include <string.h>         // Required for strcmp()
#include <stdio.h>          // Used for .WAV loading

#if defined(AUDIO_STANDALONE)
    #include <stdarg.h>     // Used for functions with variable number of parameters (TraceLog())
#else
    #include "utils.h"      // rRES data decompression utility function
                            // NOTE: Includes Android fopen function map
#endif

//#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.h"     // OGG loading functions

#define JAR_XM_IMPLEMENTATION
#include "jar_xm.h"         // For playing .xm files

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_STREAM_BUFFERS          2
#define MAX_AUDIO_CONTEXTS          4             // Number of open AL sources
#define MAX_MUSIC_STREAMS           2

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

// Audio Context, used to create custom audio streams that are not bound to a sound file. There can be
// no more than 4 concurrent audio contexts in use. This is due to each active context being tied to
// a dedicated mix channel. All audio is 32bit floating point in stereo.
typedef struct AudioContext_t {
    unsigned short sampleRate;           // default is 48000
    unsigned char channels;              // 1=mono,2=stereo
    unsigned char mixChannel;            // 0-3 or mixA-mixD, each mix channel can receive up to one dedicated audio stream
    bool floatingPoint;                  // if false then the short datatype is used instead
    bool playing;                        // false if paused
    ALenum alFormat;                     // openAL format specifier
    ALuint alSource;                     // openAL source
    ALuint alBuffer[MAX_STREAM_BUFFERS]; // openAL sample buffer
} AudioContext_t;

// Music type (file streaming from memory)
// NOTE: Anything longer than ~10 seconds should be streamed...
typedef struct Music {
    stb_vorbis *stream;
    jar_xm_context_t *chipctx; // Stores jar_xm context
    AudioContext_t *ctx; // audio context
    
    int totalSamplesLeft;
    float totalLengthSeconds;
    bool loop;
    bool chipTune; // True if chiptune is loaded
} Music;

#if defined(AUDIO_STANDALONE)
typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static AudioContext_t* mixChannelsActive_g[MAX_AUDIO_CONTEXTS];      // What mix channels are currently active
static bool musicEnabled = false;
static Music currentMusic[MAX_MUSIC_STREAMS];                        // Current music loaded, up to two can play at the same time

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(const char *fileName);         // Load WAV file
static Wave LoadOGG(char *fileName);               // Load OGG file
static void UnloadWave(Wave wave);                 // Unload wave data

static bool BufferMusicStream(int index);          // Fill music buffers with data
static void EmptyMusicStream(int index);           // Empty music buffers

static unsigned short FillAlBufferWithSilence(AudioContext_t *context, ALuint buffer);// fill buffer with zeros, returns number processed
static void ResampleShortToFloat(short *shorts, float *floats, unsigned short len); // pass two arrays of the same legnth in
static void ResampleByteToFloat(char *chars, float *floats, unsigned short len); // pass two arrays of same length in

#if defined(AUDIO_STANDALONE)
const char *GetExtension(const char *fileName);     // Get the extension for a filename
void TraceLog(int msgType, const char *text, ...);  // Outputs a trace log message (INFO, ERROR, WARNING)
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Audio Device initialization and Closing
//----------------------------------------------------------------------------------

// Initialize audio device and context
void InitAudioDevice(void)
{
    // Open and initialize a device with default settings
    ALCdevice *device = alcOpenDevice(NULL);

    if(!device) TraceLog(ERROR, "Audio device could not be opened");

    ALCcontext *context = alcCreateContext(device, NULL);

    if(context == NULL || alcMakeContextCurrent(context) == ALC_FALSE)
    {
        if(context != NULL) alcDestroyContext(context);

        alcCloseDevice(device);

        TraceLog(ERROR, "Could not setup audio context");
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
    for(int index=0; index<MAX_MUSIC_STREAMS; index++)
    {
        if(currentMusic[index].ctx) StopMusicStream(index);      // Stop music streaming and close current stream
    }
    

    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) TraceLog(WARNING, "Could not get current audio context for closing");

    device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

// True if call to InitAudioDevice() was successful and CloseAudioDevice() has not been called yet
bool IsAudioDeviceReady(void)
{
    ALCcontext *context = alcGetCurrentContext();
    if (context == NULL) return false;
    else{
        ALCdevice *device = alcGetContextsDevice(context);
        if (device == NULL) return false;
        else return true;
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Custom audio output
//----------------------------------------------------------------------------------

// Audio contexts are for outputing custom audio waveforms, This will shut down any other sound sources currently playing
// The mixChannel is what mix channel you want to operate on, 0-3 are the ones available. Each mix channel can only be used one at a time.
// exmple usage is InitAudioContext(48000, 0, 2, true); // mixchannel 1, 48khz, stereo, floating point
AudioContext InitAudioContext(unsigned short sampleRate, unsigned char mixChannel, unsigned char channels, bool floatingPoint)
{
    if(mixChannel >= MAX_AUDIO_CONTEXTS) return NULL;
    if(!IsAudioDeviceReady()) InitAudioDevice();
    
    if(!mixChannelsActive_g[mixChannel]){
        AudioContext_t *ac = (AudioContext_t*)malloc(sizeof(AudioContext_t));
        ac->sampleRate = sampleRate;
        ac->channels = channels;
        ac->mixChannel = mixChannel;
        ac->floatingPoint = floatingPoint;
        mixChannelsActive_g[mixChannel] = ac;
        
        // setup openAL format
        if(channels == 1)
        {
            if(floatingPoint)
                ac->alFormat = AL_FORMAT_MONO_FLOAT32;
            else
                ac->alFormat = AL_FORMAT_MONO16;
        }
        else if(channels == 2)
        {
            if(floatingPoint)
                ac->alFormat = AL_FORMAT_STEREO_FLOAT32;
            else
                ac->alFormat = AL_FORMAT_STEREO16;
        }
        
        // Create an audio source
        alGenSources(1, &ac->alSource);
        alSourcef(ac->alSource, AL_PITCH, 1);
        alSourcef(ac->alSource, AL_GAIN, 1);
        alSource3f(ac->alSource, AL_POSITION, 0, 0, 0);
        alSource3f(ac->alSource, AL_VELOCITY, 0, 0, 0);
        
        // Create Buffer
        alGenBuffers(MAX_STREAM_BUFFERS, ac->alBuffer);
        
        //fill buffers
        int x;
        for(x=0;x<MAX_STREAM_BUFFERS;x++)
            FillAlBufferWithSilence(ac, ac->alBuffer[x]);
        
        alSourceQueueBuffers(ac->alSource, MAX_STREAM_BUFFERS, ac->alBuffer);
        alSourcePlay(ac->alSource);
        ac->playing = true;
        
        return ac;
    }
    return NULL;
}

// Frees buffer in audio context
void CloseAudioContext(AudioContext ctx)
{
    AudioContext_t *context = (AudioContext_t*)ctx;
    if(context){
        alSourceStop(context->alSource);
        context->playing = false;
        
        //flush out all queued buffers
        ALuint buffer = 0;
        int queued = 0;
        alGetSourcei(context->alSource, AL_BUFFERS_QUEUED, &queued);
        while (queued > 0)
        {
            alSourceUnqueueBuffers(context->alSource, 1, &buffer);
            queued--;
        }
        
        //delete source and buffers
        alDeleteSources(1, &context->alSource);
        alDeleteBuffers(MAX_STREAM_BUFFERS, context->alBuffer);
        mixChannelsActive_g[context->mixChannel] = NULL;
        free(context);
        ctx = NULL;
    }
}

// Pushes more audio data into context mix channel, if none are ever pushed then zeros are fed in.
// Call "UpdateAudioContext(ctx, NULL, 0)" if you want to pause the audio.
// @Returns number of samples that where processed.
unsigned short UpdateAudioContext(AudioContext ctx, void *data, unsigned short numberElements)
{
    AudioContext_t *context = (AudioContext_t*)ctx;
    
    if(!context || (context->channels == 2 && numberElements % 2 != 0)) return 0; // when there is two channels there must be an even number of samples
    
    if (!data || !numberElements)
    { // pauses audio until data is given
        alSourcePause(context->alSource);
        context->playing = false;
        return 0;
    }
    else
    { // restart audio otherwise
        ALint state;
        alGetSourcei(context->alSource, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING){
            alSourcePlay(context->alSource);
            context->playing = true;
        }
    }
    
    if (context && context->playing && mixChannelsActive_g[context->mixChannel] == context)
    {
        ALint processed = 0;
        ALuint buffer = 0;
        unsigned short numberProcessed = 0;
        unsigned short numberRemaining = numberElements;
        
        
        alGetSourcei(context->alSource, AL_BUFFERS_PROCESSED, &processed); // Get the number of already processed buffers (if any)
        if(!processed) return 0; // nothing to process, queue is still full
        
        
        while (processed > 0)
        {
            if(context->floatingPoint) // process float buffers
            {
                float *ptr = (float*)data;
                alSourceUnqueueBuffers(context->alSource, 1, &buffer);
                if(numberRemaining >= MUSIC_BUFFER_SIZE_FLOAT)
                {
                    alBufferData(buffer, context->alFormat, &ptr[numberProcessed], MUSIC_BUFFER_SIZE_FLOAT*sizeof(float), context->sampleRate);
                    numberProcessed+=MUSIC_BUFFER_SIZE_FLOAT;
                    numberRemaining-=MUSIC_BUFFER_SIZE_FLOAT;
                }
                else
                {
                    alBufferData(buffer, context->alFormat, &ptr[numberProcessed], numberRemaining*sizeof(float), context->sampleRate);
                    numberProcessed+=numberRemaining;
                    numberRemaining=0;
                }
                alSourceQueueBuffers(context->alSource, 1, &buffer);
                processed--;
            }
            else if(!context->floatingPoint) // process short buffers
            {
                short *ptr = (short*)data;
                alSourceUnqueueBuffers(context->alSource, 1, &buffer);
                if(numberRemaining >= MUSIC_BUFFER_SIZE_SHORT)
                {
                    alBufferData(buffer, context->alFormat, &ptr[numberProcessed], MUSIC_BUFFER_SIZE_FLOAT*sizeof(short), context->sampleRate);
                    numberProcessed+=MUSIC_BUFFER_SIZE_SHORT;
                    numberRemaining-=MUSIC_BUFFER_SIZE_SHORT;
                }
                else
                {
                    alBufferData(buffer, context->alFormat, &ptr[numberProcessed], numberRemaining*sizeof(short), context->sampleRate);
                    numberProcessed+=numberRemaining;
                    numberRemaining=0;
                }
                alSourceQueueBuffers(context->alSource, 1, &buffer);
                processed--;
            }
            else
                break;
        }
        return numberProcessed;
    }
    return 0;
}

// fill buffer with zeros, returns number processed
static unsigned short FillAlBufferWithSilence(AudioContext_t *context, ALuint buffer)
{
    if(context->floatingPoint){
        float pcm[MUSIC_BUFFER_SIZE_FLOAT] = {0.f};
        alBufferData(buffer, context->alFormat, pcm, MUSIC_BUFFER_SIZE_FLOAT*sizeof(float), context->sampleRate);
        return MUSIC_BUFFER_SIZE_FLOAT;
    }
    else
    {
        short pcm[MUSIC_BUFFER_SIZE_SHORT] = {0};
        alBufferData(buffer, context->alFormat, pcm, MUSIC_BUFFER_SIZE_SHORT*sizeof(short), context->sampleRate);
        return MUSIC_BUFFER_SIZE_SHORT;
    }
}

// example usage:
// short sh[3] = {1,2,3};float fl[3];
// ResampleShortToFloat(sh,fl,3);
static void ResampleShortToFloat(short *shorts, float *floats, unsigned short len)
{
    int x;
    for(x=0;x<len;x++)
    {
        if(shorts[x] < 0)
            floats[x] = (float)shorts[x] / 32766.f;
        else
            floats[x] = (float)shorts[x] / 32767.f;
    }
}

// example usage:
// char ch[3] = {1,2,3};float fl[3];
// ResampleByteToFloat(ch,fl,3);
static void ResampleByteToFloat(char *chars, float *floats, unsigned short len)
{
    int x;
    for(x=0;x<len;x++)
    {
        if(chars[x] < 0)
            floats[x] = (float)chars[x] / 127.f;
        else
            floats[x] = (float)chars[x] / 128.f;
    }
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
    else TraceLog(WARNING, "[%s] Sound extension not recognized, it can't be loaded", fileName);

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
// returns 0 on success
int PlayMusicStream(int musicIndex, char *fileName)
{
    int mixIndex;
    
    if(currentMusic[musicIndex].stream || currentMusic[musicIndex].chipctx) return 1; // error
    
    for(mixIndex = 0; mixIndex < MAX_AUDIO_CONTEXTS; mixIndex++) // find empty mix channel slot
    {
        if(mixChannelsActive_g[mixIndex] == NULL) break;
        else if(mixIndex = MAX_AUDIO_CONTEXTS - 1) return 2; // error
    }
    
    if (strcmp(GetExtension(fileName),"ogg") == 0)
    {
        // Open audio stream
        currentMusic[musicIndex].stream = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (currentMusic[musicIndex].stream == NULL)
        {
            TraceLog(WARNING, "[%s] OGG audio file could not be opened", fileName);
            return 3; // error
        }
        else
        {
            // Get file info
            stb_vorbis_info info = stb_vorbis_get_info(currentMusic[musicIndex].stream);

            TraceLog(INFO, "[%s] Ogg sample rate: %i", fileName, info.sample_rate);
            TraceLog(INFO, "[%s] Ogg channels: %i", fileName, info.channels);
            TraceLog(DEBUG, "[%s] Temp memory required: %i", fileName, info.temp_memory_required);

            currentMusic[musicIndex].loop = true;                  // We loop by default
            musicEnabled = true;
            currentMusic[musicIndex].ctx->playing = true;

            currentMusic[musicIndex].totalSamplesLeft = stb_vorbis_stream_length_in_samples(currentMusic[musicIndex].stream) * info.channels;
            currentMusic[musicIndex].totalLengthSeconds = stb_vorbis_stream_length_in_seconds(currentMusic[musicIndex].stream);
            
            if (info.channels == 2){
                currentMusic[musicIndex].ctx = InitAudioContext(info.sample_rate, mixIndex, 2, false);
            }
            else{
                currentMusic[musicIndex].ctx = InitAudioContext(info.sample_rate, mixIndex, 1, false);
            }
        }
    }
    else if (strcmp(GetExtension(fileName),"xm") == 0)
    {
        // only stereo is supported for xm
        if(!jar_xm_create_context_from_file(&currentMusic[musicIndex].chipctx, 48000, fileName))
        {
            currentMusic[musicIndex].chipTune = true;
            currentMusic[musicIndex].loop = true;
            jar_xm_set_max_loop_count(currentMusic[musicIndex].chipctx, 0); // infinite number of loops
            currentMusic[musicIndex].totalSamplesLeft =  jar_xm_get_remaining_samples(currentMusic[musicIndex].chipctx);
            currentMusic[musicIndex].totalLengthSeconds = ((float)currentMusic[musicIndex].totalSamplesLeft) / 48000.f;
            musicEnabled = true;
            currentMusic[musicIndex].ctx->playing = true;
            
            TraceLog(INFO, "[%s] XM number of samples: %i", fileName, currentMusic[musicIndex].totalSamplesLeft);
            TraceLog(INFO, "[%s] XM track length: %11.6f sec", fileName, currentMusic[musicIndex].totalLengthSeconds);
            
            currentMusic[musicIndex].ctx = InitAudioContext(48000, mixIndex, 2, true);
        }
        else
        {
            TraceLog(WARNING, "[%s] XM file could not be opened", fileName);
            return 4; // error
        }
    }
    else
    {
        TraceLog(WARNING, "[%s] Music extension not recognized, it can't be loaded", fileName);
        return 5; // error
    }
    return 0; // normal return
}

// Stop music playing for individual music index of currentMusic array (close stream)
void StopMusicStream(int index)
{
    if (index < MAX_MUSIC_STREAMS && currentMusic[index].ctx)
    {
        CloseAudioContext(currentMusic[index].ctx);
        
        if (currentMusic[index].chipTune)
        {
            jar_xm_free_context(currentMusic[index].chipctx);
        }
        else
        {
            stb_vorbis_close(currentMusic[index].stream);
        }
        
        if(!getMusicStreamCount()) musicEnabled = false;
        if(currentMusic[index].stream || currentMusic[index].chipctx)
        {
            currentMusic[index].stream = NULL;
            currentMusic[index].chipctx = NULL;
        }
    }
}

//get number of music channels active at this time, this does not mean they are playing
int getMusicStreamCount(void)
{
    int musicCount = 0;
    for(int musicIndex = 0; musicIndex < MAX_MUSIC_STREAMS; musicIndex++) // find empty music slot
        if(currentMusic[musicIndex].stream != NULL || currentMusic[musicIndex].chipTune) musicCount++;
    
    return musicCount;
}

// Pause music playing
void PauseMusicStream(int index)
{
    // Pause music stream if music available!
    if (currentMusic[index].ctx && musicEnabled)
    {
        TraceLog(INFO, "Pausing music stream");
        alSourcePause(currentMusic[index].ctx->alSource);
        currentMusic[index].ctx->playing = false;
    }
}

// Resume music playing
void ResumeMusicStream(int index)
{
    // Resume music playing... if music available!
    ALenum state;
    if(currentMusic[index].ctx){
        alGetSourcei(currentMusic[index].ctx->alSource, AL_SOURCE_STATE, &state);
        if (state == AL_PAUSED)
        {
            TraceLog(INFO, "Resuming music stream");
            alSourcePlay(currentMusic[index].ctx->alSource);
            currentMusic[index].ctx->playing = true;
        }
    }
}

// Check if any music is playing
bool IsMusicPlaying(int index)
{
    bool playing = false;
    ALint state;
    
    if(index < MAX_MUSIC_STREAMS && currentMusic[index].ctx){
        alGetSourcei(currentMusic[index].ctx->alSource, AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING) playing = true;
    }

    return playing;
}

// Set volume for music
void SetMusicVolume(int index, float volume)
{
    if(index < MAX_MUSIC_STREAMS && currentMusic[index].ctx){
        alSourcef(currentMusic[index].ctx->alSource, AL_GAIN, volume);
    }
}

void SetMusicPitch(int index, float pitch)
{
    if(index < MAX_MUSIC_STREAMS && currentMusic[index].ctx){
        alSourcef(currentMusic[index].ctx->alSource, AL_PITCH, pitch);
    }
}

// Get current music time length (in seconds)
float GetMusicTimeLength(int index)
{
    float totalSeconds;
    if (currentMusic[index].chipTune)
    {
        totalSeconds = currentMusic[index].totalLengthSeconds;
    }
    else
    {
        totalSeconds = stb_vorbis_stream_length_in_seconds(currentMusic[index].stream);
    }

    return totalSeconds;
}

// Get current music time played (in seconds)
float GetMusicTimePlayed(int index)
{
    float secondsPlayed;
    if (currentMusic[index].chipTune)
    {
        uint64_t samples;
        jar_xm_get_position(currentMusic[index].chipctx, NULL, NULL, NULL, &samples);
        secondsPlayed = (float)samples / (48000 * currentMusic[index].ctx->channels); // Not sure if this is the correct value
    }
    else
    {
        int totalSamples = stb_vorbis_stream_length_in_samples(currentMusic[index].stream) * currentMusic[index].ctx->channels;
        int samplesPlayed = totalSamples - currentMusic[index].totalSamplesLeft;
        secondsPlayed = (float)samplesPlayed / (currentMusic[index].ctx->sampleRate * currentMusic[index].ctx->channels);
    }
    

    return secondsPlayed;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Fill music buffers with new data from music stream
static bool BufferMusicStream(int index)
{
    short pcm[MUSIC_BUFFER_SIZE_SHORT];
    float pcmf[MUSIC_BUFFER_SIZE_FLOAT];
    
    int  size = 0;              // Total size of data steamed (in bytes)
    bool active = true;         // We can get more data from stream (not finished)
        
    if (musicEnabled)
    {
        if(!currentMusic[index].ctx->playing)
        {
            UpdateAudioContext(currentMusic[index].ctx, NULL, 0);
            return false;
        }
        
        if (currentMusic[index].chipTune) // There is no end of stream for xmfiles, once the end is reached zeros are generated for non looped chiptunes.
        {
            jar_xm_generate_samples(currentMusic[index].chipctx, pcmf, MUSIC_BUFFER_SIZE_FLOAT / 2); // reads 2*readlen shorts and moves them to buffer+size memory location
            UpdateAudioContext(currentMusic[index].ctx, pcmf, MUSIC_BUFFER_SIZE_FLOAT);
            currentMusic[index].totalSamplesLeft -= MUSIC_BUFFER_SIZE_FLOAT;
            if(currentMusic[index].totalSamplesLeft <= 0) active = false;
        }
        else
        {
            int streamedBytes = stb_vorbis_get_samples_short_interleaved(currentMusic[index].stream, currentMusic[index].ctx->channels, pcm, MUSIC_BUFFER_SIZE_SHORT);
            UpdateAudioContext(currentMusic[index].ctx, pcm, streamedBytes);
            currentMusic[index].totalSamplesLeft -= streamedBytes*currentMusic[index].ctx->channels;
            if(currentMusic[index].totalSamplesLeft <= 0) active = false;
        }
        TraceLog(DEBUG, "Buffering index:%i, chiptune:%i", index, (int)currentMusic[index].chipTune);
    }

    return active;
}

// Empty music buffers
static void EmptyMusicStream(int index)
{
    ALuint buffer = 0;
    int queued = 0;

    alGetSourcei(currentMusic[index].ctx->alSource, AL_BUFFERS_QUEUED, &queued);

    while (queued > 0)
    {
        alSourceUnqueueBuffers(currentMusic[index].ctx->alSource, 1, &buffer);

        queued--;
    }
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(int index)
{
    ALenum state;
    bool active = true;

    if (index < MAX_MUSIC_STREAMS && musicEnabled)
    {
        active = BufferMusicStream(index);
        
        if ((!active) && (currentMusic[index].loop))
        {
            if(currentMusic[index].chipTune)
            {
                currentMusic[index].totalSamplesLeft = currentMusic[index].totalLengthSeconds * currentMusic[index].ctx->sampleRate;
            }
            else
            {
                stb_vorbis_seek_start(currentMusic[index].stream);
                currentMusic[index].totalSamplesLeft = stb_vorbis_stream_length_in_samples(currentMusic[index].stream)*currentMusic[index].ctx->channels;
            }
            active = BufferMusicStream(index);
        }
        

        if (alGetError() != AL_NO_ERROR) TraceLog(WARNING, "Error buffering data...");
        
        alGetSourcei(currentMusic[index].ctx->alSource, AL_SOURCE_STATE, &state);

        if ((state != AL_PLAYING) && active) alSourcePlay(currentMusic[index].ctx->alSource);

        if (!active) StopMusicStream(index);
        
    }
    else
        return;

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