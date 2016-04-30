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
#define MUSIC_STREAM_BUFFERS        2

#if defined(PLATFORM_RPI) || defined(PLATFORM_ANDROID)
    // NOTE: On RPI and Android should be lower to avoid frame-stalls
    #define MUSIC_BUFFER_SIZE      4096*2   // PCM data buffer (short) - 16Kb (RPI)
#else
    // NOTE: On HTML5 (emscripten) this is allocated on heap, by default it's only 16MB!...just take care...
    #define MUSIC_BUFFER_SIZE      4096*8   // PCM data buffer (short) - 64Kb
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Music type (file streaming from memory)
// NOTE: Anything longer than ~10 seconds should be streamed...
typedef struct Music {
    stb_vorbis *stream;
    jar_xm_context_t *chipctx; // Stores jar_xm context

    ALuint buffers[MUSIC_STREAM_BUFFERS];
    ALuint source;
    ALenum format;

    int channels;
    int sampleRate;
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
static bool mixChannelsActive_g[4]; // What mix channels are currently active
static bool musicEnabled = false;
static Music currentMusic;        // Current music loaded
                                  // NOTE: Only one music file playing at a time

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(const char *fileName);          // Load WAV file
static Wave LoadOGG(char *fileName);                // Load OGG file
static void UnloadWave(Wave wave);                  // Unload wave data

static bool BufferMusicStream(ALuint buffer);       // Fill music buffers with data
static void EmptyMusicStream(void);                 // Empty music buffers

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

// Close the audio device for the current context, and destroys the context
void CloseAudioDevice(void)
{
    StopMusicStream();      // Stop music streaming and close current stream

    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();

    if (context == NULL) TraceLog(WARNING, "Could not get current audio context for closing");

    device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

// True if call to InitAudioDevice() was successful and CloseAudioDevice() has not been called yet
bool AudioDeviceReady(void)
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
// The mix_t is what mix channel you want to operate on, mixA->mixD are the ones available. Each mix channel can only be used one at a time.
// exmple usage is InitAudioContext(48000, 16, mixA, stereo);
AudioContext* InitAudioContext(unsigned short sampleRate, unsigned char bitsPerSample, mix_t mixChannel, channel_t channels)
{
    if(!AudioDeviceReady()) InitAudioDevice();
    else StopMusicStream();
    
    if(!mixChannelsActive_g[mixChannel]){
        AudioContext *ac = malloc(sizeof(AudioContext));
        ac->sampleRate = sampleRate;
        ac->bitsPerSample = bitsPerSample;
        ac->mixChannel = mixChannel;
        ac->channels = channels;
        mixChannelsActive_g[mixChannel] = true;
        return ac;
    }
    return NULL;
}

// Frees buffer in audio context
void CloseAudioContext(AudioContext *ctx)
{
    if(ctx){
        mixChannelsActive_g[ctx->mixChannel] = false;
        free(ctx);
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
bool SoundIsPlaying(Sound sound)
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
void PlayMusicStream(char *fileName)
{
    if (strcmp(GetExtension(fileName),"ogg") == 0)
    {
        // Stop current music, clean buffers, unload current stream
        StopMusicStream();

        // Open audio stream
        currentMusic.stream = stb_vorbis_open_filename(fileName, NULL, NULL);

        if (currentMusic.stream == NULL)
        {
            TraceLog(WARNING, "[%s] OGG audio file could not be opened", fileName);
        }
        else
        {
            // Get file info
            stb_vorbis_info info = stb_vorbis_get_info(currentMusic.stream);

            currentMusic.channels = info.channels;
            currentMusic.sampleRate = info.sample_rate;

            TraceLog(INFO, "[%s] Ogg sample rate: %i", fileName, info.sample_rate);
            TraceLog(INFO, "[%s] Ogg channels: %i", fileName, info.channels);
            TraceLog(DEBUG, "[%s] Temp memory required: %i", fileName, info.temp_memory_required);

            if (info.channels == 2) currentMusic.format = AL_FORMAT_STEREO16;
            else currentMusic.format = AL_FORMAT_MONO16;

            currentMusic.loop = true;                  // We loop by default
            musicEnabled = true;

            // Create an audio source
            alGenSources(1, &currentMusic.source);     // Generate pointer to audio source

            alSourcef(currentMusic.source, AL_PITCH, 1);
            alSourcef(currentMusic.source, AL_GAIN, 1);
            alSource3f(currentMusic.source, AL_POSITION, 0, 0, 0);
            alSource3f(currentMusic.source, AL_VELOCITY, 0, 0, 0);
            //alSourcei(currentMusic.source, AL_LOOPING, AL_TRUE);     // ERROR: Buffers do not queue!

            // Generate two OpenAL buffers
            alGenBuffers(2, currentMusic.buffers);

            // Fill buffers with music...
            BufferMusicStream(currentMusic.buffers[0]);
            BufferMusicStream(currentMusic.buffers[1]);

            // Queue buffers and start playing
            alSourceQueueBuffers(currentMusic.source, 2, currentMusic.buffers);
            alSourcePlay(currentMusic.source);

            // NOTE: Regularly, we must check if a buffer has been processed and refill it: UpdateMusicStream()

            currentMusic.totalSamplesLeft = stb_vorbis_stream_length_in_samples(currentMusic.stream) * currentMusic.channels;
            currentMusic.totalLengthSeconds = stb_vorbis_stream_length_in_seconds(currentMusic.stream);
        }
    }
    else if (strcmp(GetExtension(fileName),"xm") == 0)
    {
        // Stop current music, clean buffers, unload current stream
        StopMusicStream();
        
        // new song settings for xm chiptune
        currentMusic.chipTune = true;
        currentMusic.channels = 2;
        currentMusic.sampleRate = 48000;
        currentMusic.loop = true;
        
        // only stereo is supported for xm
        if(!jar_xm_create_context_from_file(&currentMusic.chipctx, currentMusic.sampleRate, fileName))
        {
            currentMusic.format = AL_FORMAT_STEREO16;
            jar_xm_set_max_loop_count(currentMusic.chipctx, 0); // infinite number of loops
            currentMusic.totalSamplesLeft =  jar_xm_get_remaining_samples(currentMusic.chipctx);
            currentMusic.totalLengthSeconds = ((float)currentMusic.totalSamplesLeft) / ((float)currentMusic.sampleRate);
            musicEnabled = true;
            
            TraceLog(INFO, "[%s] XM number of samples: %i", fileName, currentMusic.totalSamplesLeft);
            TraceLog(INFO, "[%s] XM track length: %11.6f sec", fileName, currentMusic.totalLengthSeconds);
            
            // Set up OpenAL
            alGenSources(1, &currentMusic.source);
            alSourcef(currentMusic.source, AL_PITCH, 1);
            alSourcef(currentMusic.source, AL_GAIN, 1);
            alSource3f(currentMusic.source, AL_POSITION, 0, 0, 0);
            alSource3f(currentMusic.source, AL_VELOCITY, 0, 0, 0);
            alGenBuffers(2, currentMusic.buffers);
            BufferMusicStream(currentMusic.buffers[0]);
            BufferMusicStream(currentMusic.buffers[1]);
            alSourceQueueBuffers(currentMusic.source, 2, currentMusic.buffers);
            alSourcePlay(currentMusic.source);
            
            // NOTE: Regularly, we must check if a buffer has been processed and refill it: UpdateMusicStream()
        }
        else TraceLog(WARNING, "[%s] XM file could not be opened", fileName);
    }
    else TraceLog(WARNING, "[%s] Music extension not recognized, it can't be loaded", fileName);
}

// Stop music playing (close stream)
void StopMusicStream(void)
{
    if (musicEnabled)
    {
        alSourceStop(currentMusic.source);
        EmptyMusicStream(); // Empty music buffers
        alDeleteSources(1, &currentMusic.source);
        alDeleteBuffers(2, currentMusic.buffers);
        
        if (currentMusic.chipTune)
        {
            jar_xm_free_context(currentMusic.chipctx);
        }
        else
        {
            stb_vorbis_close(currentMusic.stream);
        }
    }

    musicEnabled = false;
}

// Pause music playing
void PauseMusicStream(void)
{
    // Pause music stream if music available!
    if (musicEnabled)
    {
        TraceLog(INFO, "Pausing music stream");
        alSourcePause(currentMusic.source);
        musicEnabled = false;
    }
}

// Resume music playing
void ResumeMusicStream(void)
{
    // Resume music playing... if music available!
    ALenum state;
    alGetSourcei(currentMusic.source, AL_SOURCE_STATE, &state);

    if (state == AL_PAUSED)
    {
        TraceLog(INFO, "Resuming music stream");
        alSourcePlay(currentMusic.source);
        musicEnabled = true;
    }
}

// Check if music is playing
bool MusicIsPlaying(void)
{
    bool playing = false;
    ALint state;

    alGetSourcei(currentMusic.source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) playing = true;

    return playing;
}

// Set volume for music
void SetMusicVolume(float volume)
{
    alSourcef(currentMusic.source, AL_GAIN, volume);
}

// Get current music time length (in seconds)
float GetMusicTimeLength(void)
{
    float totalSeconds;
    if (currentMusic.chipTune)
    {
        totalSeconds = currentMusic.totalLengthSeconds;
    }
    else
    {
        totalSeconds = stb_vorbis_stream_length_in_seconds(currentMusic.stream);
    }

    return totalSeconds;
}

// Get current music time played (in seconds)
float GetMusicTimePlayed(void)
{
    float secondsPlayed;
    if (currentMusic.chipTune)
    {
        uint64_t samples;
        jar_xm_get_position(currentMusic.chipctx, NULL, NULL, NULL, &samples);
        secondsPlayed = (float)samples / (currentMusic.sampleRate * currentMusic.channels); // Not sure if this is the correct value
    }
    else
    {
        int totalSamples = stb_vorbis_stream_length_in_samples(currentMusic.stream) * currentMusic.channels;
        int samplesPlayed = totalSamples - currentMusic.totalSamplesLeft;
        secondsPlayed = (float)samplesPlayed / (currentMusic.sampleRate * currentMusic.channels);
    }
    

    return secondsPlayed;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Fill music buffers with new data from music stream
static bool BufferMusicStream(ALuint buffer)
{
    short pcm[MUSIC_BUFFER_SIZE];
    
    int  size = 0;              // Total size of data steamed (in bytes)
    int  streamedBytes = 0;     // samples of data obtained, channels are not included in calculation
    bool active = true;         // We can get more data from stream (not finished)

    if (musicEnabled)
    {
        if (currentMusic.chipTune) // There is no end of stream for xmfiles, once the end is reached zeros are generated for non looped chiptunes.
        {
            int readlen = MUSIC_BUFFER_SIZE / 2;
            jar_xm_generate_samples_16bit(currentMusic.chipctx, pcm, readlen); // reads 2*readlen shorts and moves them to buffer+size memory location
            size += readlen * currentMusic.channels; // Not sure if this is what it needs
        }
        else
        {
            while (size < MUSIC_BUFFER_SIZE)
            {
                streamedBytes = stb_vorbis_get_samples_short_interleaved(currentMusic.stream, currentMusic.channels, pcm + size, MUSIC_BUFFER_SIZE - size);
                if (streamedBytes > 0) size += (streamedBytes*currentMusic.channels);
                else break;
            }
        }
        TraceLog(DEBUG, "Streaming music data to buffer. Bytes streamed: %i", size);
    }

    if (size > 0)
    {
        alBufferData(buffer, currentMusic.format, pcm, size*sizeof(short), currentMusic.sampleRate);
        currentMusic.totalSamplesLeft -= size;
        
        if(currentMusic.totalSamplesLeft <= 0) active = false; // end if no more samples left
    }
    else
    {
        active = false;
        TraceLog(WARNING, "No more data obtained from stream");
    }

    return active;
}

// Empty music buffers
static void EmptyMusicStream(void)
{
    ALuint buffer = 0;
    int queued = 0;

    alGetSourcei(currentMusic.source, AL_BUFFERS_QUEUED, &queued);

    while (queued > 0)
    {
        alSourceUnqueueBuffers(currentMusic.source, 1, &buffer);

        queued--;
    }
}

// Update (re-fill) music buffers if data already processed
void UpdateMusicStream(void)
{
    ALuint buffer = 0;
    ALint processed = 0;
    bool active = true;

    if (musicEnabled)
    {
        // Get the number of already processed buffers (if any)
        alGetSourcei(currentMusic.source, AL_BUFFERS_PROCESSED, &processed);

        while (processed > 0)
        {
            // Recover processed buffer for refill
            alSourceUnqueueBuffers(currentMusic.source, 1, &buffer);

            // Refill buffer
            active = BufferMusicStream(buffer);

            // If no more data to stream, restart music (if loop)
            if ((!active) && (currentMusic.loop))
            {
                if(currentMusic.chipTune)
                {
                    currentMusic.totalSamplesLeft = currentMusic.totalLengthSeconds * currentMusic.sampleRate;
                }
                else
                {
                    stb_vorbis_seek_start(currentMusic.stream);
                    currentMusic.totalSamplesLeft = stb_vorbis_stream_length_in_samples(currentMusic.stream)*currentMusic.channels;
                }
                active = BufferMusicStream(buffer);
            }

            // Add refilled buffer to queue again... don't let the music stop!
            alSourceQueueBuffers(currentMusic.source, 1, &buffer);

            if (alGetError() != AL_NO_ERROR) TraceLog(WARNING, "Error buffering data...");

            processed--;
        }

        ALenum state;
        alGetSourcei(currentMusic.source, AL_SOURCE_STATE, &state);

        if ((state != AL_PLAYING) && active) alSourcePlay(currentMusic.source);

        if (!active) StopMusicStream();
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