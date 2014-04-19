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

//#include "stb_vorbis.h"      // OGG loading functions

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Sound source type (all file loaded in memory)
/*
struct Sound {
    unsigned int source;
    unsigned int buffer;
};

// Music type (file streamming from memory)
// NOTE: Anything longer than ~10 seconds should be Music...
struct Music {
    stb_vorbis* stream;
	stb_vorbis_info info;
    
    ALuint id; 
	ALuint buffers[2];
	ALuint source;
	ALenum format;
 
	int bufferSize;
	int totalSamplesLeft;
	bool loop;
};
*/

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
static bool musicIsPlaying;
static Music *currentMusic;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static Wave LoadWAV(char *fileName);
static void UnloadWAV(Wave wave);
//static Ogg LoadOGG(char *fileName);
static bool MusicStream(Music music, ALuint buffer);

extern bool MusicStreamUpdate();
extern void PlayCurrentMusic();

//----------------------------------------------------------------------------------
// Module Functions Definition - Window and OpenGL Context Functions
//----------------------------------------------------------------------------------

// Initialize audio device and context
void InitAudioDevice()
{
    // Open and initialize a device with default settings
    ALCdevice *device = alcOpenDevice(NULL);
    
    if(!device) TraceLog(ERROR, "Could not open audio device");

    ALCcontext *context = alcCreateContext(device, NULL);
    
    if(context == NULL || alcMakeContextCurrent(context) == ALC_FALSE)
    {
        if(context != NULL) alcDestroyContext(context);
        
        alcCloseDevice(device);
        
        TraceLog(ERROR, "Could not setup audio context");
    }

    TraceLog(INFO, "Audio device and context initialized: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
    
    // Listener definition (just for 2D)
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListener3f(AL_ORIENTATION, 0, 0, -1);
    
    musicIsPlaying = false;
}

// Close the audio device for the current context, and destroys the context
void CloseAudioDevice()
{
    ALCdevice *device;
    ALCcontext *context = alcGetCurrentContext();
    
    if (context == NULL) TraceLog(WARNING, "Could not get current audio context for closing");

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
    
    // Unallocate WAV data
    UnloadWAV(wave);
    
    TraceLog(INFO, "[%s] Sound file loaded successfully", fileName);  
    TraceLog(INFO, "[%s] Sample rate: %i - Channels: %i", fileName, wave.sampleRate, wave.channels);
    
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

    if (!rresFile) TraceLog(WARNING, "[%s] Could not open raylib resource file", rresName);
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
                        
                        // Unallocate WAV data
                        UnloadWAV(wave);

                        TraceLog(INFO, "[%s] Sound loaded successfully from resource, sample rate: %i", rresName, (int)sampleRate);
                        
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
    
    TraceLog(INFO, "Playing sound");

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
bool SoundIsPlaying(Sound sound)
{
    bool playing = false;
    ALint state;
    
    alGetSourcei(sound.source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) playing = true;
    
    return playing;
}

// Check if music is playing
bool MusicIsPlaying(Music music)
{
    ALenum state;
    
    alGetSourcei(music.source, AL_SOURCE_STATE, &state);
    
    return (state == AL_PLAYING);
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
        TraceLog(WARNING, "[%s] Could not open WAV file", fileName);
    }
    else
    {
        // Read in the first chunk into the struct
        fread(&riffHeader, sizeof(RiffHeader), 1, wavFile);
     
        // Check for RIFF and WAVE tags
        if (((riffHeader.chunkID[0] != 'R') || (riffHeader.chunkID[1] != 'I') || (riffHeader.chunkID[2] != 'F') || (riffHeader.chunkID[3] != 'F')) ||
            ((riffHeader.format[0] != 'W') || (riffHeader.format[1] != 'A') || (riffHeader.format[2] != 'V') || (riffHeader.format[3] != 'E')))
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
                    
                    TraceLog(INFO, "[%s] Wave file loaded successfully", fileName);
                }
            }
        }

        fclose(wavFile);
    }
    
    return wave;
}

// Unload WAV file data
static void UnloadWAV(Wave wave)
{
    free(wave.data);
}

// TODO: Ogg data loading
Music LoadMusic(char *fileName)
{
    Music music;
    
    // Open audio stream
    music.stream = stb_vorbis_open_filename(fileName, NULL, NULL);
    
	if (music.stream == NULL) TraceLog(WARNING, "Could not open ogg audio file");
    else
    {
        // Get file info
        music.info = stb_vorbis_get_info(music.stream);
        
        printf("Ogg sample rate: %i\n", music.info.sample_rate);
        printf("Ogg channels: %i\n", music.info.channels);
        printf("Temp memory required: %i\n", music.info.temp_memory_required);
        
        if (music.info.channels == 2) music.format = AL_FORMAT_STEREO16;
        else music.format = AL_FORMAT_MONO16;
        
        music.bufferSize = 4096*8;
        music.loop = true;          // We loop by default
        
        // Create an audio source
        alGenSources(1, &music.source);             // Generate pointer to audio source

        alSourcef(music.source, AL_PITCH, 1);    
        alSourcef(music.source, AL_GAIN, 1);
        alSource3f(music.source, AL_POSITION, 0, 0, 0);
        alSource3f(music.source, AL_VELOCITY, 0, 0, 0);
        alSourcei(music.source, AL_LOOPING, AL_TRUE);     // We loop by default
        
        // Convert loaded data to OpenAL buffers
        alGenBuffers(2, music.buffers);
    /*
        if (!MusicStream(music, music.buffers[0])) exit(1);
        if (!MusicStream(music, music.buffers[1])) exit(1);
        
        alSourceQueueBuffers(music.source, 2, music.buffers);
     
        PlayMusic(music);
    */ 
        music.totalSamplesLeft = stb_vorbis_stream_length_in_samples(music.stream) * music.info.channels;
     
        currentMusic = &music;
    }
    
    return music;
}

void UnloadMusic(Music music)
{
    StopMusic(music);

    alDeleteSources(1, &music.source);
	alDeleteBuffers(2, music.buffers);
    
	stb_vorbis_close(music.stream);
}

void PlayMusic(Music music)
{
    //if (MusicIsPlaying(music)) return true;

    if (!MusicStream(music, music.buffers[0])) TraceLog(WARNING, "MusicStream returned 0");
    if (!MusicStream(music, music.buffers[1])) TraceLog(WARNING, "MusicStream returned 0");
    
    alSourceQueueBuffers(music.source, 2, music.buffers);
    alSourcePlay(music.source);

    TraceLog(INFO, "Playing music");
}

extern void PlayCurrentMusic()
{
    if (!MusicStream(*currentMusic, currentMusic->buffers[0])) TraceLog(WARNING, "MusicStream returned 0");
    if (!MusicStream(*currentMusic, currentMusic->buffers[1])) TraceLog(WARNING, "MusicStream returned 0");
    
    alSourceQueueBuffers(currentMusic->source, 2, currentMusic->buffers);
    alSourcePlay(currentMusic->source);
}

// Stop reproducing music
void StopMusic(Music music)
{
    alSourceStop(music.source);
    
    musicIsPlaying = false;
}

static bool MusicStream(Music music, ALuint buffer)
{
	//Uncomment this to avoid VLAs
	//#define BUFFER_SIZE 4096*32
	#ifndef BUFFER_SIZE//VLAs ftw
	#define BUFFER_SIZE (music.bufferSize)
	#endif
	ALshort pcm[BUFFER_SIZE];
    
	int  size = 0;
	int  result = 0;
 
	while (size < BUFFER_SIZE)
    {
		result = stb_vorbis_get_samples_short_interleaved(music.stream, music.info.channels, pcm+size, BUFFER_SIZE-size);
        
		if (result > 0) size += (result*music.info.channels);
		else break;
	}
 
	if (size == 0) return false;
 
	alBufferData(buffer, music.format, pcm, size*sizeof(ALshort), music.info.sample_rate);
    
	music.totalSamplesLeft -= size;
	
    #undef BUFFER_SIZE
 
	return true;
}
/*
extern bool MusicStreamUpdate()
{
	ALint processed = 0;
 
    alGetSourcei(currentMusic->source, AL_BUFFERS_PROCESSED, &processed);
 
    while (processed--)
    {
        ALuint buffer = 0;
        
        alSourceUnqueueBuffers(currentMusic->source, 1, &buffer);
 
		if (!MusicStream(*currentMusic, buffer))
        {
			bool shouldExit = true;
 
			if (currentMusic->loop)
            {
				stb_vorbis_seek_start(currentMusic->stream);
				currentMusic->totalSamplesLeft = stb_vorbis_stream_length_in_samples(currentMusic->stream) * currentMusic->info.channels;
				
                shouldExit = !MusicStream(*currentMusic, buffer);
			}
 
			if (shouldExit) return false;
		}
        
		alSourceQueueBuffers(currentMusic->source, 1, &buffer);
	}
 
	return true;
}
*/
extern bool MusicStreamUpdate()
{
    int processed;
    bool active = true;
 
    alGetSourcei(currentMusic->source, AL_BUFFERS_PROCESSED, &processed);
    
    printf("Data processed: %i\n", processed);
 
    while (processed--)
    {
        ALuint buffer = 0;
        
        alSourceUnqueueBuffers(currentMusic->source, 1, &buffer);

        active = MusicStream(*currentMusic, buffer);
 
        alSourceQueueBuffers(currentMusic->source, 1, &buffer);
    }
 
    return active;
}

void MusicStreamEmpty()
{
    int queued;
    
    alGetSourcei(currentMusic->source, AL_BUFFERS_QUEUED, &queued);
    
    while(queued--)
    {
        ALuint buffer;  
        alSourceUnqueueBuffers(currentMusic->source, 1, &buffer);
    }
}