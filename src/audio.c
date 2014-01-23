/*********************************************************************************************
*
*   raylib.audio
*
*   Basic functions to manage Audio: InitAudioDevice, LoadAudioFiles, PlayAudioFiles
*    
*   Uses external lib:    
*       OpenAL - Audio device management lib
*       TODO: stb_vorbis - Ogg audio files loading
*       
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

#include "raylib.h"

#include <AL/al.h>           // OpenAL basic header
#include <AL/alc.h>          // OpenAL context header (like OpenGL, OpenAL requires a context to work)

#include <stdlib.h>          // To use exit() function
#include <stdio.h>           // Used for .WAV loading

#include "utils.h"           // rRES data decompression utility function

//#include "stb_vorbis.h"    // TODO: OGG loading functions

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Wave file data
typedef struct Wave {
    unsigned char *data;      // Buffer data pointer
    unsigned int sampleRate;
    unsigned int dataSize;
    short bitsPerSample;
    short channels;  
} Wave;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(char *fileName);
static void UnloadWAV(Wave wave);
//static Ogg LoadOGG(char *fileName);

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------

// Initialize audio device and context
void InitAudioDevice()
{
    // Open and initialize a device with default settings
    ALCdevice *device = alcOpenDevice(NULL);
    
    if(!device)
    {
        fprintf(stderr, "Could not open a device!\n");
        exit(1);
    }

    ALCcontext *context = alcCreateContext(device, NULL);
    
    if(context == NULL || alcMakeContextCurrent(context) == ALC_FALSE)
    {
        if(context != NULL)    alcDestroyContext(context);
        
        alcCloseDevice(device);
        
        fprintf(stderr, "Could not set a context!\n");
        exit(1);
    }

    printf("Opened \"%s\"\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
    
    // Listener definition (just for 2D)
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListener3f(AL_ORIENTATION, 0, 0, -1);
}

// Close the audio device for the current context, and destroys the context
void CloseAudioDevice()
{
    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();
    
    if (context == NULL) return;

    device = alcGetContextsDevice(context);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

// Load sound to memory
Sound LoadSound(char *fileName)
{
    Sound sound;
    
    // NOTE: The entire file is loaded to memory to play it all at once (no-streaming)
    
    // WAV file loading
    // NOTE: Buffer space is allocated inside LoadWAV, Wave must be freed
    Wave wave = LoadWAV(fileName);
    
    ALenum format;
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
    
    // Unallocate WAV data
    UnloadWAV(wave);
    
    printf("Sample rate: %i\n", wave.sampleRate);
    printf("Channels: %i\n", wave.channels);
    
    printf("Audio file loaded...!\n");
    
    sound.source = source;
    sound.buffer = buffer;
    
    return sound;
}

// Load sound to memory from rRES file (raylib Resource)
Sound LoadSoundFromRES(const char *rresName, int resId)
{
    // NOTE: rresName could be directly a char array with all the data!!! --> TODO
    Sound sound;
    bool found = false;

    char id[4];             // rRES file identifier
    unsigned char version;  // rRES file version and subversion
    char useless;           // rRES header reserved data
    short numRes;
    
    ResInfoHeader infoHeader;
    
    FILE *rresFile = fopen(rresName, "rb");

    if (!rresFile) printf("Error opening raylib Resource file\n");
    
    // Read rres file (basic file check - id)
    fread(&id[0], sizeof(char), 1, rresFile);
    fread(&id[1], sizeof(char), 1, rresFile);
    fread(&id[2], sizeof(char), 1, rresFile);
    fread(&id[3], sizeof(char), 1, rresFile);
    fread(&version, sizeof(char), 1, rresFile);
    fread(&useless, sizeof(char), 1, rresFile);
    
    if ((id[0] != 'r') && (id[1] != 'R') && (id[2] != 'E') &&(id[3] != 'S'))
    {
        printf("This is not a valid raylib Resource file!\n");
        exit(1);
    }
    
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
        
                printf("Sample rate: %i\n", (int)sampleRate);
                printf("Bits per sample: %i\n", (int)bps);
                printf("Channels: %i\n", (int)channels);
                
                wave.sampleRate = sampleRate;
                wave.dataSize = infoHeader.srcSize;
                wave.bitsPerSample = bps;
                wave.channels = (short)channels;
                
                unsigned char *data = malloc(infoHeader.size);

                fread(data, infoHeader.size, 1, rresFile);
                
                wave.data = DecompressData(data, infoHeader.size, infoHeader.srcSize);
                
                free(data);
                
                // Convert wave to Sound (OpenAL)
                ALenum format;
                
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
                
                // Unallocate WAV data
                UnloadWAV(wave);

                printf("Audio file loaded...!\n");
                
                sound.source = source;
                sound.buffer = buffer;
            }
            else
            {

                printf("Required resource do not seem to be a valid IMAGE resource\n");
                exit(2);
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
    
    fclose(rresFile);
    
    if (!found) printf("Required resource id could not be found in the raylib Resource file!\n");
    
    return sound;
}

// Unload sound
void UnloadSound(Sound sound)
{
    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);
}

// Play a sound
void PlaySound(Sound sound)
{
    alSourcePlay(sound.source);        // Play the sound
    
    printf("Playing sound!\n");

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

// Play a sound with extended options
// TODO: This function should be reviewed...
void PlaySoundEx(Sound sound, float timePosition, bool loop)
{
    // TODO: Review
    
    // Change the current position (e.g. skip some part of the sound)
    // NOTE: Only work when the entire file is in a single buffer
    //alSourcei(sound.source, AL_BYTE_OFFSET, int(position * sampleRate));

    alSourcePlay(sound.source);        // Play the sound
    
    if (loop) alSourcei(sound.source, AL_LOOPING, AL_TRUE);
    else alSourcei(sound.source, AL_LOOPING, AL_FALSE);
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
bool IsPlaying(Sound sound)
{
    bool playing = false;
    ALint state;
    
    alGetSourcei(sound.source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) playing = true;
    
    return playing;
}

// Set volume for a sound
void SetVolume(Sound sound, float volume)
{
    alSourcef(sound.source, AL_GAIN, volume);
}

// Set pitch for a sound
void SetPitch(Sound sound, float pitch)
{
    alSourcef(sound.source, AL_PITCH, pitch);
}

// Load WAV file into Wave structure
static Wave LoadWAV(char *fileName) 
{
    // Basic WAV headers structs
    typedef struct {
        char chunkID[4];
        long chunkSize;
        char format[4];
    } RiffHeader;

    typedef struct {
        char subChunkID[4];
        long subChunkSize;
        short audioFormat;
        short numChannels;
        long sampleRate;
        long byteRate;
        short blockAlign;
        short bitsPerSample;
    } WaveFormat;

    typedef struct {
        char subChunkID[4];
        long subChunkSize;
    } WaveData;
    
    RiffHeader riffHeader;
    WaveFormat waveFormat;
    WaveData waveData;
    
    Wave wave;
    FILE *wavFile;
    
    wavFile = fopen(fileName, "rb");
    
    if (!wavFile)
    {
        printf("Could not open WAV file.\n");
        exit(1);
    }
   
    // Read in the first chunk into the struct
    fread(&riffHeader, sizeof(RiffHeader), 1, wavFile);
 
    // Check for RIFF and WAVE tags
    if ((riffHeader.chunkID[0] != 'R' ||
         riffHeader.chunkID[1] != 'I' ||
         riffHeader.chunkID[2] != 'F' ||
         riffHeader.chunkID[3] != 'F') ||
        (riffHeader.format[0] != 'W' ||
         riffHeader.format[1] != 'A' ||
         riffHeader.format[2] != 'V' ||
         riffHeader.format[3] != 'E'))
            printf("Invalid RIFF or WAVE Header");
 
    // Read in the 2nd chunk for the wave info
    fread(&waveFormat, sizeof(WaveFormat), 1, wavFile);
    
    // Check for fmt tag
    if (waveFormat.subChunkID[0] != 'f' ||
        waveFormat.subChunkID[1] != 'm' ||
        waveFormat.subChunkID[2] != 't' ||
        waveFormat.subChunkID[3] != ' ')
            printf("Invalid Wave Format");
 
    // Check for extra parameters;
    if (waveFormat.subChunkSize > 16)
        fseek(wavFile, sizeof(short), SEEK_CUR);
 
    // Read in the the last byte of data before the sound file
    fread(&waveData, sizeof(WaveData), 1, wavFile);
    
    // Check for data tag
    if (waveData.subChunkID[0] != 'd' ||
        waveData.subChunkID[1] != 'a' ||
        waveData.subChunkID[2] != 't' ||
        waveData.subChunkID[3] != 'a')
            printf("Invalid data header");
 
    // Allocate memory for data
    wave.data = (unsigned char *)malloc(sizeof(unsigned char) * waveData.subChunkSize); 
 
    // Read in the sound data into the soundData variable
    fread(wave.data, waveData.subChunkSize, 1, wavFile);
    
    // Now we set the variables that we need later
    wave.dataSize = waveData.subChunkSize;
    wave.sampleRate = waveFormat.sampleRate;
    wave.channels = waveFormat.numChannels;
    wave.bitsPerSample = waveFormat.bitsPerSample;  

    return wave;
}

// Unload WAV file data
static void UnloadWAV(Wave wave)
{
    free(wave.data);
}

// TODO: Ogg data loading
//static Ogg LoadOGG(char *fileName) { }

