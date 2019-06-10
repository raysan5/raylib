/*
WAV audio loader and writer. Choice of public domain or MIT-0. See license statements at the end of this file.
dr_wav - v0.9.1 - 2019-05-05

David Reid - mackron@gmail.com
*/

/*
DEPRECATED APIS
===============
Version 0.9.0 deprecated the per-sample reading and seeking APIs and replaced them with versions that work on the resolution
of a PCM frame instead. For example, given a stereo WAV file, previously you would pass 2 to drwav_read_f32() to read one
PCM frame, whereas now you would pass in 1 to drwav_read_pcm_frames_f32(). The old APIs would return the number of samples
read, whereas now it will return the number of PCM frames. Below is a list of APIs that have been deprecated and their
replacements.

    drwav_read()                     -> drwav_read_pcm_frames()
    drwav_read_s16()                 -> drwav_read_pcm_frames_s16()
    drwav_read_f32()                 -> drwav_read_pcm_frames_f32()
    drwav_read_s32()                 -> drwav_read_pcm_frames_s32()
    drwav_seek_to_sample()           -> drwav_seek_to_pcm_frame()
    drwav_write()                    -> drwav_write_pcm_frames()
    drwav_open_and_read_s16()        -> drwav_open_and_read_pcm_frames_s16()
    drwav_open_and_read_f32()        -> drwav_open_and_read_pcm_frames_f32()
    drwav_open_and_read_s32()        -> drwav_open_and_read_pcm_frames_s32()
    drwav_open_file_and_read_s16()   -> drwav_open_file_and_read_pcm_frames_s16()
    drwav_open_file_and_read_f32()   -> drwav_open_file_and_read_pcm_frames_f32()
    drwav_open_file_and_read_s32()   -> drwav_open_file_and_read_pcm_frames_s32()
    drwav_open_memory_and_read_s16() -> drwav_open_memory_and_read_pcm_frames_s16()
    drwav_open_memory_and_read_f32() -> drwav_open_memory_and_read_pcm_frames_f32()
    drwav_open_memory_and_read_s32() -> drwav_open_memory_and_read_pcm_frames_s32()
    drwav::totalSampleCount          -> drwav::totalPCMFrameCount

Rationale:
    1) Most programs will want to read in multiples of the channel count which demands a per-frame reading API. Per-sample
       reading just adds complexity and maintenance costs for no practical benefit.
    2) This is consistent with my other decoders - dr_flac and dr_mp3.

These APIs will be removed completely in version 0.10.0. You can continue to use drwav_read_raw() if you need per-sample
reading.
*/

/*
USAGE
=====
This is a single-file library. To use it, do something like the following in one .c file.
    #define DR_WAV_IMPLEMENTATION
    #include "dr_wav.h"

You can then #include this file in other parts of the program as you would with any other header file. Do something
like the following to read audio data:

    drwav wav;
    if (!drwav_init_file(&wav, "my_song.wav")) {
        // Error opening WAV file.
    }

    drwav_int32* pDecodedInterleavedSamples = malloc(wav.totalPCMFrameCount * wav.channels * sizeof(drwav_int32));
    size_t numberOfSamplesActuallyDecoded = drwav_read_pcm_frames_s32(&wav, wav.totalPCMFrameCount, pDecodedInterleavedSamples);

    ...

    drwav_uninit(&wav);

You can also use drwav_open() to allocate and initialize the loader for you:

    drwav* pWav = drwav_open_file("my_song.wav");
    if (pWav == NULL) {
        // Error opening WAV file.
    }

    ...

    drwav_close(pWav);

If you just want to quickly open and read the audio data in a single operation you can do something like this:

    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalPCMFrameCount;
    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32("my_song.wav", &channels, &sampleRate, &totalPCMFrameCount);
    if (pSampleData == NULL) {
        // Error opening and reading WAV file.
    }

    ...

    drwav_free(pSampleData);

The examples above use versions of the API that convert the audio data to a consistent format (32-bit signed PCM, in
this case), but you can still output the audio data in its internal format (see notes below for supported formats):

    size_t samplesRead = drwav_read_pcm_frames(&wav, wav.totalPCMFrameCount, pDecodedInterleavedSamples);

You can also read the raw bytes of audio data, which could be useful if dr_wav does not have native support for
a particular data format:

    size_t bytesRead = drwav_read_raw(&wav, bytesToRead, pRawDataBuffer);


dr_wav can also be used to output WAV files. This does not currently support compressed formats. To use this, look at
drwav_open_write(), drwav_open_file_write(), etc. Use drwav_write_pcm_frames() to write samples, or drwav_write_raw()
to write raw data in the "data" chunk.

    drwav_data_format format;
    format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
    format.format = DR_WAVE_FORMAT_PCM;          // <-- Any of the DR_WAVE_FORMAT_* codes.
    format.channels = 2;
    format.sampleRate = 44100;
    format.bitsPerSample = 16;
    drwav* pWav = drwav_open_file_write("data/recording.wav", &format);

    ...

    drwav_uint64 samplesWritten = drwav_write_pcm_frames(pWav, frameCount, pSamples);


dr_wav has seamless support the Sony Wave64 format. The decoder will automatically detect it and it should Just Work
without any manual intervention.


OPTIONS
=======
#define these options before including this file.

#define DR_WAV_NO_CONVERSION_API
  Disables conversion APIs such as drwav_read_pcm_frames_f32() and drwav_s16_to_f32().

#define DR_WAV_NO_STDIO
  Disables drwav_open_file(), drwav_open_file_write(), etc.



QUICK NOTES
===========
- Samples are always interleaved.
- The default read function does not do any data conversion. Use drwav_read_pcm_frames_f32(), drwav_read_pcm_frames_s32()
  and drwav_read_pcm_frames_s16() to read and convert audio data to 32-bit floating point, signed 32-bit integer and
  signed 16-bit integer samples respectively. Tested and supported internal formats include the following:
  - Unsigned 8-bit PCM
  - Signed 12-bit PCM
  - Signed 16-bit PCM
  - Signed 24-bit PCM
  - Signed 32-bit PCM
  - IEEE 32-bit floating point
  - IEEE 64-bit floating point
  - A-law and u-law
  - Microsoft ADPCM
  - IMA ADPCM (DVI, format code 0x11)
- dr_wav will try to read the WAV file as best it can, even if it's not strictly conformant to the WAV format.
*/

#ifndef dr_wav_h
#define dr_wav_h

#include <stddef.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    drwav_int8;
typedef unsigned char    drwav_uint8;
typedef   signed short   drwav_int16;
typedef unsigned short   drwav_uint16;
typedef   signed int     drwav_int32;
typedef unsigned int     drwav_uint32;
typedef   signed __int64 drwav_int64;
typedef unsigned __int64 drwav_uint64;
#else
#include <stdint.h>
typedef int8_t           drwav_int8;
typedef uint8_t          drwav_uint8;
typedef int16_t          drwav_int16;
typedef uint16_t         drwav_uint16;
typedef int32_t          drwav_int32;
typedef uint32_t         drwav_uint32;
typedef int64_t          drwav_int64;
typedef uint64_t         drwav_uint64;
#endif
typedef drwav_uint8      drwav_bool8;
typedef drwav_uint32     drwav_bool32;
#define DRWAV_TRUE       1
#define DRWAV_FALSE      0

#ifdef __cplusplus
extern "C" {
#endif

/* Common data formats. */
#define DR_WAVE_FORMAT_PCM          0x1
#define DR_WAVE_FORMAT_ADPCM        0x2
#define DR_WAVE_FORMAT_IEEE_FLOAT   0x3
#define DR_WAVE_FORMAT_ALAW         0x6
#define DR_WAVE_FORMAT_MULAW        0x7
#define DR_WAVE_FORMAT_DVI_ADPCM    0x11
#define DR_WAVE_FORMAT_EXTENSIBLE   0xFFFE

/* Constants. */
#ifndef DRWAV_MAX_SMPL_LOOPS
#define DRWAV_MAX_SMPL_LOOPS        1
#endif

/* Flags to pass into drwav_init_ex(), etc. */
#define DRWAV_SEQUENTIAL            0x00000001

typedef enum
{
    drwav_seek_origin_start,
    drwav_seek_origin_current
} drwav_seek_origin;

typedef enum
{
    drwav_container_riff,
    drwav_container_w64
} drwav_container;

typedef struct
{
    union
    {
        drwav_uint8 fourcc[4];
        drwav_uint8 guid[16];
    } id;

    /* The size in bytes of the chunk. */
    drwav_uint64 sizeInBytes;

    /*
    RIFF = 2 byte alignment.
    W64  = 8 byte alignment.
    */
    unsigned int paddingSize;
} drwav_chunk_header;

/*
Callback for when data is read. Return value is the number of bytes actually read.

pUserData   [in]  The user data that was passed to drwav_init(), drwav_open() and family.
pBufferOut  [out] The output buffer.
bytesToRead [in]  The number of bytes to read.

Returns the number of bytes actually read.

A return value of less than bytesToRead indicates the end of the stream. Do _not_ return from this callback until
either the entire bytesToRead is filled or you have reached the end of the stream.
*/
typedef size_t (* drwav_read_proc)(void* pUserData, void* pBufferOut, size_t bytesToRead);

/*
Callback for when data is written. Returns value is the number of bytes actually written.

pUserData    [in]  The user data that was passed to drwav_init_write(), drwav_open_write() and family.
pData        [out] A pointer to the data to write.
bytesToWrite [in]  The number of bytes to write.

Returns the number of bytes actually written.

If the return value differs from bytesToWrite, it indicates an error.
*/
typedef size_t (* drwav_write_proc)(void* pUserData, const void* pData, size_t bytesToWrite);

/*
Callback for when data needs to be seeked.

pUserData [in] The user data that was passed to drwav_init(), drwav_open() and family.
offset    [in] The number of bytes to move, relative to the origin. Will never be negative.
origin    [in] The origin of the seek - the current position or the start of the stream.

Returns whether or not the seek was successful.

Whether or not it is relative to the beginning or current position is determined by the "origin" parameter which
will be either drwav_seek_origin_start or drwav_seek_origin_current.
*/
typedef drwav_bool32 (* drwav_seek_proc)(void* pUserData, int offset, drwav_seek_origin origin);

/*
Callback for when drwav_init_ex/drwav_open_ex finds a chunk.

pChunkUserData    [in] The user data that was passed to the pChunkUserData parameter of drwav_init_ex(), drwav_open_ex() and family.
onRead            [in] A pointer to the function to call when reading.
onSeek            [in] A pointer to the function to call when seeking.
pReadSeekUserData [in] The user data that was passed to the pReadSeekUserData parameter of drwav_init_ex(), drwav_open_ex() and family.
pChunkHeader      [in] A pointer to an object containing basic header information about the chunk. Use this to identify the chunk.

Returns the number of bytes read + seeked.

To read data from the chunk, call onRead(), passing in pReadSeekUserData as the first parameter. Do the same
for seeking with onSeek(). The return value must be the total number of bytes you have read _plus_ seeked.

You must not attempt to read beyond the boundary of the chunk.
*/
typedef drwav_uint64 (* drwav_chunk_proc)(void* pChunkUserData, drwav_read_proc onRead, drwav_seek_proc onSeek, void* pReadSeekUserData, const drwav_chunk_header* pChunkHeader);

/* Structure for internal use. Only used for loaders opened with drwav_open_memory(). */
typedef struct
{
    const drwav_uint8* data;
    size_t dataSize;
    size_t currentReadPos;
} drwav__memory_stream;

/* Structure for internal use. Only used for writers opened with drwav_open_memory_write(). */
typedef struct
{
    void** ppData;
    size_t* pDataSize;
    size_t dataSize;
    size_t dataCapacity;
    size_t currentWritePos;
} drwav__memory_stream_write;

typedef struct
{
    drwav_container container;  /* RIFF, W64. */
    drwav_uint32 format;        /* DR_WAVE_FORMAT_* */
    drwav_uint32 channels;
    drwav_uint32 sampleRate;
    drwav_uint32 bitsPerSample;
} drwav_data_format;

typedef struct
{
    /*
    The format tag exactly as specified in the wave file's "fmt" chunk. This can be used by applications
    that require support for data formats not natively supported by dr_wav.
    */
    drwav_uint16 formatTag;

    /* The number of channels making up the audio data. When this is set to 1 it is mono, 2 is stereo, etc. */
    drwav_uint16 channels;

    /* The sample rate. Usually set to something like 44100. */
    drwav_uint32 sampleRate;

    /* Average bytes per second. You probably don't need this, but it's left here for informational purposes. */
    drwav_uint32 avgBytesPerSec;

    /* Block align. This is equal to the number of channels * bytes per sample. */
    drwav_uint16 blockAlign;

    /* Bits per sample. */
    drwav_uint16 bitsPerSample;

    /* The size of the extended data. Only used internally for validation, but left here for informational purposes. */
    drwav_uint16 extendedSize;

    /*
    The number of valid bits per sample. When <formatTag> is equal to WAVE_FORMAT_EXTENSIBLE, <bitsPerSample>
    is always rounded up to the nearest multiple of 8. This variable contains information about exactly how
    many bits a valid per sample. Mainly used for informational purposes.
    */
    drwav_uint16 validBitsPerSample;

    /* The channel mask. Not used at the moment. */
    drwav_uint32 channelMask;

    /* The sub-format, exactly as specified by the wave file. */
    drwav_uint8 subFormat[16];
} drwav_fmt;

typedef struct
{
    drwav_uint32 cuePointId;
    drwav_uint32 type;
    drwav_uint32 start;
    drwav_uint32 end;
    drwav_uint32 fraction;
    drwav_uint32 playCount;
} drwav_smpl_loop;

 typedef struct
{
    drwav_uint32 manufacturer;
    drwav_uint32 product;
    drwav_uint32 samplePeriod;
    drwav_uint32 midiUnityNotes;
    drwav_uint32 midiPitchFraction;
    drwav_uint32 smpteFormat;
    drwav_uint32 smpteOffset;
    drwav_uint32 numSampleLoops;
    drwav_uint32 samplerData;
    drwav_smpl_loop loops[DRWAV_MAX_SMPL_LOOPS];
} drwav_smpl;

typedef struct
{
    /* A pointer to the function to call when more data is needed. */
    drwav_read_proc onRead;

    /* A pointer to the function to call when data needs to be written. Only used when the drwav object is opened in write mode. */
    drwav_write_proc onWrite;

    /* A pointer to the function to call when the wav file needs to be seeked. */
    drwav_seek_proc onSeek;

    /* The user data to pass to callbacks. */
    void* pUserData;


    /* Whether or not the WAV file is formatted as a standard RIFF file or W64. */
    drwav_container container;


    /* Structure containing format information exactly as specified by the wav file. */
    drwav_fmt fmt;

    /* The sample rate. Will be set to something like 44100. */
    drwav_uint32 sampleRate;

    /* The number of channels. This will be set to 1 for monaural streams, 2 for stereo, etc. */
    drwav_uint16 channels;

    /* The bits per sample. Will be set to something like 16, 24, etc. */
    drwav_uint16 bitsPerSample;

    /* Equal to fmt.formatTag, or the value specified by fmt.subFormat if fmt.formatTag is equal to 65534 (WAVE_FORMAT_EXTENSIBLE). */
    drwav_uint16 translatedFormatTag;

    /* The total number of PCM frames making up the audio data. */
    drwav_uint64 totalPCMFrameCount;


    /* The size in bytes of the data chunk. */
    drwav_uint64 dataChunkDataSize;
    
    /* The position in the stream of the first byte of the data chunk. This is used for seeking. */
    drwav_uint64 dataChunkDataPos;

    /* The number of bytes remaining in the data chunk. */
    drwav_uint64 bytesRemaining;


    /*
    Only used in sequential write mode. Keeps track of the desired size of the "data" chunk at the point of initialization time. Always
    set to 0 for non-sequential writes and when the drwav object is opened in read mode. Used for validation.
    */
    drwav_uint64 dataChunkDataSizeTargetWrite;

    /* Keeps track of whether or not the wav writer was initialized in sequential mode. */
    drwav_bool32 isSequentialWrite;


    /* smpl chunk. */
    drwav_smpl smpl;


    /* A hack to avoid a DRWAV_MALLOC() when opening a decoder with drwav_open_memory(). */
    drwav__memory_stream memoryStream;
    drwav__memory_stream_write memoryStreamWrite;

    /* Generic data for compressed formats. This data is shared across all block-compressed formats. */
    struct
    {
        drwav_uint64 iCurrentSample;    /* The index of the next sample that will be read by drwav_read_*(). This is used with "totalSampleCount" to ensure we don't read excess samples at the end of the last block. */
    } compressed;
    
    /* Microsoft ADPCM specific data. */
    struct
    {
        drwav_uint32 bytesRemainingInBlock;
        drwav_uint16 predictor[2];
        drwav_int32  delta[2];
        drwav_int32  cachedSamples[4];  /* Samples are stored in this cache during decoding. */
        drwav_uint32 cachedSampleCount;
        drwav_int32  prevSamples[2][2]; /* The previous 2 samples for each channel (2 channels at most). */
    } msadpcm;

    /* IMA ADPCM specific data. */
    struct
    {
        drwav_uint32 bytesRemainingInBlock;
        drwav_int32  predictor[2];
        drwav_int32  stepIndex[2];
        drwav_int32  cachedSamples[16]; /* Samples are stored in this cache during decoding. */
        drwav_uint32 cachedSampleCount;
    } ima;


    drwav_uint64 totalSampleCount;  /* <-- DEPRECATED. Will be removed in a future version. */
} drwav;


/*
Initializes a pre-allocated drwav object.

pWav                         [out]          A pointer to the drwav object being initialized.
onRead                       [in]           The function to call when data needs to be read from the client.
onSeek                       [in]           The function to call when the read position of the client data needs to move.
onChunk                      [in, optional] The function to call when a chunk is enumerated at initialized time.
pUserData, pReadSeekUserData [in, optional] A pointer to application defined data that will be passed to onRead and onSeek.
pChunkUserData               [in, optional] A pointer to application defined data that will be passed to onChunk.
flags                        [in, optional] A set of flags for controlling how things are loaded.

Returns true if successful; false otherwise.

Close the loader with drwav_uninit().

This is the lowest level function for initializing a WAV file. You can also use drwav_init_file() and drwav_init_memory()
to open the stream from a file or from a block of memory respectively.

If you want dr_wav to manage the memory allocation for you, consider using drwav_open() instead. This will allocate
a drwav object on the heap and return a pointer to it.

Possible values for flags:
  DRWAV_SEQUENTIAL: Never perform a backwards seek while loading. This disables the chunk callback and will cause this function
                    to return as soon as the data chunk is found. Any chunks after the data chunk will be ignored.

drwav_init() is equivalent to "drwav_init_ex(pWav, onRead, onSeek, NULL, pUserData, NULL, 0);".

The onChunk callback is not called for the WAVE or FMT chunks. The contents of the FMT chunk can be read from pWav->fmt
after the function returns.

See also: drwav_init_file(), drwav_init_memory(), drwav_uninit()
*/
drwav_bool32 drwav_init(drwav* pWav, drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData);
drwav_bool32 drwav_init_ex(drwav* pWav, drwav_read_proc onRead, drwav_seek_proc onSeek, drwav_chunk_proc onChunk, void* pReadSeekUserData, void* pChunkUserData, drwav_uint32 flags);

/*
Initializes a pre-allocated drwav object for writing.

onWrite   [in]           The function to call when data needs to be written.
onSeek    [in]           The function to call when the write position needs to move.
pUserData [in, optional] A pointer to application defined data that will be passed to onWrite and onSeek.

Returns true if successful; false otherwise.

Close the writer with drwav_uninit().

This is the lowest level function for initializing a WAV file. You can also use drwav_init_file() and drwav_init_memory()
to open the stream from a file or from a block of memory respectively.

If the total sample count is known, you can use drwav_init_write_sequential(). This avoids the need for dr_wav to perform
a post-processing step for storing the total sample count and the size of the data chunk which requires a backwards seek.

If you want dr_wav to manage the memory allocation for you, consider using drwav_open() instead. This will allocate
a drwav object on the heap and return a pointer to it.

See also: drwav_init_file_write(), drwav_init_memory_write(), drwav_uninit()
*/
drwav_bool32 drwav_init_write(drwav* pWav, const drwav_data_format* pFormat, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData);
drwav_bool32 drwav_init_write_sequential(drwav* pWav, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_write_proc onWrite, void* pUserData);

/*
Uninitializes the given drwav object.

Use this only for objects initialized with drwav_init().
*/
void drwav_uninit(drwav* pWav);


/*
Opens a wav file using the given callbacks.

onRead    [in]           The function to call when data needs to be read from the client.
onSeek    [in]           The function to call when the read position of the client data needs to move.
pUserData [in, optional] A pointer to application defined data that will be passed to onRead and onSeek.

Returns null on error.

Close the loader with drwav_close().

You can also use drwav_open_file() and drwav_open_memory() to open the stream from a file or from a block of
memory respectively.

This is different from drwav_init() in that it will allocate the drwav object for you via DRWAV_MALLOC() before
initializing it.

See also: drwav_init(), drwav_open_file(), drwav_open_memory(), drwav_close()
*/
drwav* drwav_open(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData);
drwav* drwav_open_ex(drwav_read_proc onRead, drwav_seek_proc onSeek, drwav_chunk_proc onChunk, void* pReadSeekUserData, void* pChunkUserData, drwav_uint32 flags);

/*
Opens a wav file for writing using the given callbacks.

onWrite   [in]           The function to call when data needs to be written.
onSeek    [in]           The function to call when the write position needs to move.
pUserData [in, optional] A pointer to application defined data that will be passed to onWrite and onSeek.

Returns null on error.

Close the loader with drwav_close().

You can also use drwav_open_file_write() and drwav_open_memory_write() to open the stream from a file or from a block
of memory respectively.

This is different from drwav_init_write() in that it will allocate the drwav object for you via DRWAV_MALLOC() before
initializing it.

See also: drwav_open_file_write(), drwav_open_memory_write(), drwav_close()
*/
drwav* drwav_open_write(const drwav_data_format* pFormat, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData);
drwav* drwav_open_write_sequential(const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_write_proc onWrite, void* pUserData);

/*
Uninitializes and deletes the the given drwav object.

Use this only for objects created with drwav_open().
*/
void drwav_close(drwav* pWav);


/*
Reads raw audio data.

This is the lowest level function for reading audio data. It simply reads the given number of
bytes of the raw internal sample data.

Consider using drwav_read_pcm_frames_s16(), drwav_read_pcm_frames_s32() or drwav_read_pcm_frames_f32() for
reading sample data in a consistent format.

Returns the number of bytes actually read.
*/
size_t drwav_read_raw(drwav* pWav, size_t bytesToRead, void* pBufferOut);

/*
Reads a chunk of audio data in the native internal format.

This is typically the most efficient way to retrieve audio data, but it does not do any format
conversions which means you'll need to convert the data manually if required.

If the return value is less than <framesToRead> it means the end of the file has been reached or
you have requested more samples than can possibly fit in the output buffer.

This function will only work when sample data is of a fixed size and uncompressed. If you are
using a compressed format consider using drwav_read_raw() or drwav_read_pcm_frames_s16/s32/f32/etc().
*/
drwav_uint64 drwav_read_pcm_frames(drwav* pWav, drwav_uint64 framesToRead, void* pBufferOut);

/*
Seeks to the given PCM frame.

Returns true if successful; false otherwise.
*/
drwav_bool32 drwav_seek_to_pcm_frame(drwav* pWav, drwav_uint64 targetFrameIndex);


/*
Writes raw audio data.

Returns the number of bytes actually written. If this differs from bytesToWrite, it indicates an error.
*/
size_t drwav_write_raw(drwav* pWav, size_t bytesToWrite, const void* pData);

/*
Writes PCM frames.

Returns the number of PCM frames written.
*/
drwav_uint64 drwav_write_pcm_frames(drwav* pWav, drwav_uint64 framesToWrite, const void* pData);


/* Conversion Utilities */
#ifndef DR_WAV_NO_CONVERSION_API

/*
Reads a chunk of audio data and converts it to signed 16-bit PCM samples.

Returns the number of PCM frames actually read.

If the return value is less than <framesToRead> it means the end of the file has been reached.
*/
drwav_uint64 drwav_read_pcm_frames_s16(drwav* pWav, drwav_uint64 framesToRead, drwav_int16* pBufferOut);

/* Low-level function for converting unsigned 8-bit PCM samples to signed 16-bit PCM samples. */
void drwav_u8_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting signed 24-bit PCM samples to signed 16-bit PCM samples. */
void drwav_s24_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting signed 32-bit PCM samples to signed 16-bit PCM samples. */
void drwav_s32_to_s16(drwav_int16* pOut, const drwav_int32* pIn, size_t sampleCount);

/* Low-level function for converting IEEE 32-bit floating point samples to signed 16-bit PCM samples. */
void drwav_f32_to_s16(drwav_int16* pOut, const float* pIn, size_t sampleCount);

/* Low-level function for converting IEEE 64-bit floating point samples to signed 16-bit PCM samples. */
void drwav_f64_to_s16(drwav_int16* pOut, const double* pIn, size_t sampleCount);

/* Low-level function for converting A-law samples to signed 16-bit PCM samples. */
void drwav_alaw_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting u-law samples to signed 16-bit PCM samples. */
void drwav_mulaw_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount);


/*
Reads a chunk of audio data and converts it to IEEE 32-bit floating point samples.

Returns the number of PCM frames actually read.

If the return value is less than <framesToRead> it means the end of the file has been reached.
*/
drwav_uint64 drwav_read_pcm_frames_f32(drwav* pWav, drwav_uint64 framesToRead, float* pBufferOut);

/* Low-level function for converting unsigned 8-bit PCM samples to IEEE 32-bit floating point samples. */
void drwav_u8_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting signed 16-bit PCM samples to IEEE 32-bit floating point samples. */
void drwav_s16_to_f32(float* pOut, const drwav_int16* pIn, size_t sampleCount);

/* Low-level function for converting signed 24-bit PCM samples to IEEE 32-bit floating point samples. */
void drwav_s24_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting signed 32-bit PCM samples to IEEE 32-bit floating point samples. */
void drwav_s32_to_f32(float* pOut, const drwav_int32* pIn, size_t sampleCount);

/* Low-level function for converting IEEE 64-bit floating point samples to IEEE 32-bit floating point samples. */
void drwav_f64_to_f32(float* pOut, const double* pIn, size_t sampleCount);

/* Low-level function for converting A-law samples to IEEE 32-bit floating point samples. */
void drwav_alaw_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting u-law samples to IEEE 32-bit floating point samples. */
void drwav_mulaw_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount);


/*
Reads a chunk of audio data and converts it to signed 32-bit PCM samples.

Returns the number of PCM frames actually read.

If the return value is less than <framesToRead> it means the end of the file has been reached.
*/
drwav_uint64 drwav_read_pcm_frames_s32(drwav* pWav, drwav_uint64 framesToRead, drwav_int32* pBufferOut);

/* Low-level function for converting unsigned 8-bit PCM samples to signed 32-bit PCM samples. */
void drwav_u8_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting signed 16-bit PCM samples to signed 32-bit PCM samples. */
void drwav_s16_to_s32(drwav_int32* pOut, const drwav_int16* pIn, size_t sampleCount);

/* Low-level function for converting signed 24-bit PCM samples to signed 32-bit PCM samples. */
void drwav_s24_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting IEEE 32-bit floating point samples to signed 32-bit PCM samples. */
void drwav_f32_to_s32(drwav_int32* pOut, const float* pIn, size_t sampleCount);

/* Low-level function for converting IEEE 64-bit floating point samples to signed 32-bit PCM samples. */
void drwav_f64_to_s32(drwav_int32* pOut, const double* pIn, size_t sampleCount);

/* Low-level function for converting A-law samples to signed 32-bit PCM samples. */
void drwav_alaw_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount);

/* Low-level function for converting u-law samples to signed 32-bit PCM samples. */
void drwav_mulaw_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount);

#endif  /* DR_WAV_NO_CONVERSION_API */


/* High-Level Convenience Helpers */

#ifndef DR_WAV_NO_STDIO
/*
Helper for initializing a wave file using stdio.

This holds the internal FILE object until drwav_uninit() is called. Keep this in mind if you're caching drwav
objects because the operating system may restrict the number of file handles an application can have open at
any given time.
*/
drwav_bool32 drwav_init_file(drwav* pWav, const char* filename);
drwav_bool32 drwav_init_file_ex(drwav* pWav, const char* filename, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags);

/*
Helper for initializing a wave file for writing using stdio.

This holds the internal FILE object until drwav_uninit() is called. Keep this in mind if you're caching drwav
objects because the operating system may restrict the number of file handles an application can have open at
any given time.
*/
drwav_bool32 drwav_init_file_write(drwav* pWav, const char* filename, const drwav_data_format* pFormat);
drwav_bool32 drwav_init_file_write_sequential(drwav* pWav, const char* filename, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount);

/*
Helper for opening a wave file using stdio.

This holds the internal FILE object until drwav_close() is called. Keep this in mind if you're caching drwav
objects because the operating system may restrict the number of file handles an application can have open at
any given time.
*/
drwav* drwav_open_file(const char* filename);
drwav* drwav_open_file_ex(const char* filename, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags);

/*
Helper for opening a wave file for writing using stdio.

This holds the internal FILE object until drwav_close() is called. Keep this in mind if you're caching drwav
objects because the operating system may restrict the number of file handles an application can have open at
any given time.
*/
drwav* drwav_open_file_write(const char* filename, const drwav_data_format* pFormat);
drwav* drwav_open_file_write_sequential(const char* filename, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount);

#endif  /* DR_WAV_NO_STDIO */

/*
Helper for initializing a loader from a pre-allocated memory buffer.

This does not create a copy of the data. It is up to the application to ensure the buffer remains valid for
the lifetime of the drwav object.

The buffer should contain the contents of the entire wave file, not just the sample data.
*/
drwav_bool32 drwav_init_memory(drwav* pWav, const void* data, size_t dataSize);
drwav_bool32 drwav_init_memory_ex(drwav* pWav, const void* data, size_t dataSize, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags);

/*
Helper for initializing a writer which outputs data to a memory buffer.

dr_wav will manage the memory allocations, however it is up to the caller to free the data with drwav_free().

The buffer will remain allocated even after drwav_uninit() is called. Indeed, the buffer should not be
considered valid until after drwav_uninit() has been called anyway.
*/
drwav_bool32 drwav_init_memory_write(drwav* pWav, void** ppData, size_t* pDataSize, const drwav_data_format* pFormat);
drwav_bool32 drwav_init_memory_write_sequential(drwav* pWav, void** ppData, size_t* pDataSize, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount);

/*
Helper for opening a loader from a pre-allocated memory buffer.

This does not create a copy of the data. It is up to the application to ensure the buffer remains valid for
the lifetime of the drwav object.

The buffer should contain the contents of the entire wave file, not just the sample data.
*/
drwav* drwav_open_memory(const void* data, size_t dataSize);
drwav* drwav_open_memory_ex(const void* data, size_t dataSize, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags);

/*
Helper for opening a writer which outputs data to a memory buffer.

dr_wav will manage the memory allocations, however it is up to the caller to free the data with drwav_free().

The buffer will remain allocated even after drwav_close() is called. Indeed, the buffer should not be
considered valid until after drwav_close() has been called anyway.
*/
drwav* drwav_open_memory_write(void** ppData, size_t* pDataSize, const drwav_data_format* pFormat);
drwav* drwav_open_memory_write_sequential(void** ppData, size_t* pDataSize, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount);


#ifndef DR_WAV_NO_CONVERSION_API
/* Opens and reads a wav file in a single operation. */
drwav_int16* drwav_open_and_read_pcm_frames_s16(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
float* drwav_open_and_read_pcm_frames_f32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
drwav_int32* drwav_open_and_read_pcm_frames_s32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
#ifndef DR_WAV_NO_STDIO
/* Opens and decodes a wav file in a single operation. */
drwav_int16* drwav_open_file_and_read_pcm_frames_s16(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
float* drwav_open_file_and_read_pcm_frames_f32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
drwav_int32* drwav_open_file_and_read_pcm_frames_s32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
#endif

/* Opens and decodes a wav file from a block of memory in a single operation. */
drwav_int16* drwav_open_memory_and_read_pcm_frames_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
float* drwav_open_memory_and_read_pcm_frames_f32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
drwav_int32* drwav_open_memory_and_read_pcm_frames_s32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalFrameCount);
#endif

/* Frees data that was allocated internally by dr_wav. */
void drwav_free(void* pDataReturnedByOpenAndRead);


/* DEPRECATED APIS */
drwav_uint64 drwav_read(drwav* pWav, drwav_uint64 samplesToRead, void* pBufferOut);
drwav_uint64 drwav_read_s16(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut);
drwav_uint64 drwav_read_f32(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut);
drwav_uint64 drwav_read_s32(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut);
drwav_bool32 drwav_seek_to_sample(drwav* pWav, drwav_uint64 sample);
drwav_uint64 drwav_write(drwav* pWav, drwav_uint64 samplesToWrite, const void* pData);
#ifndef DR_WAV_NO_CONVERSION_API
drwav_int16* drwav_open_and_read_s16(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
float* drwav_open_and_read_f32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
drwav_int32* drwav_open_and_read_s32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
#ifndef DR_WAV_NO_STDIO
drwav_int16* drwav_open_memory_and_read_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
float* drwav_open_file_and_read_f32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
drwav_int32* drwav_open_file_and_read_s32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
#endif
drwav_int16* drwav_open_memory_and_read_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
float* drwav_open_memory_and_read_f32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
drwav_int32* drwav_open_memory_and_read_s32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount);
#endif


#ifdef __cplusplus
}
#endif
#endif  /* dr_wav_h */


/************************************************************************************************************************************************************
 ************************************************************************************************************************************************************

 IMPLEMENTATION

 ************************************************************************************************************************************************************
 ************************************************************************************************************************************************************/
#ifdef DR_WAV_IMPLEMENTATION
#include <stdlib.h>
#include <string.h> /* For memcpy(), memset() */
#include <limits.h> /* For INT_MAX */

#ifndef DR_WAV_NO_STDIO
#include <stdio.h>
#endif

/* Standard library stuff. */
#ifndef DRWAV_ASSERT
#include <assert.h>
#define DRWAV_ASSERT(expression)           assert(expression)
#endif
#ifndef DRWAV_MALLOC
#define DRWAV_MALLOC(sz)                   malloc((sz))
#endif
#ifndef DRWAV_REALLOC
#define DRWAV_REALLOC(p, sz)               realloc((p), (sz))
#endif
#ifndef DRWAV_FREE
#define DRWAV_FREE(p)                      free((p))
#endif
#ifndef DRWAV_COPY_MEMORY
#define DRWAV_COPY_MEMORY(dst, src, sz)    memcpy((dst), (src), (sz))
#endif
#ifndef DRWAV_ZERO_MEMORY
#define DRWAV_ZERO_MEMORY(p, sz)           memset((p), 0, (sz))
#endif

#define drwav_countof(x)                   (sizeof(x) / sizeof(x[0]))
#define drwav_align(x, a)                  ((((x) + (a) - 1) / (a)) * (a))
#define drwav_min(a, b)                    (((a) < (b)) ? (a) : (b))
#define drwav_max(a, b)                    (((a) > (b)) ? (a) : (b))
#define drwav_clamp(x, lo, hi)             (drwav_max((lo), drwav_min((hi), (x))))

#define drwav_assert                       DRWAV_ASSERT
#define drwav_copy_memory                  DRWAV_COPY_MEMORY
#define drwav_zero_memory                  DRWAV_ZERO_MEMORY

typedef drwav_int32 drwav_result;
#define DRWAV_SUCCESS            0
#define DRWAV_ERROR             -1
#define DRWAV_INVALID_ARGS      -2
#define DRWAV_INVALID_OPERATION -3
#define DRWAV_INVALID_FILE      -100
#define DRWAV_EOF               -101

#define DRWAV_MAX_SIMD_VECTOR_SIZE         64  /* 64 for AVX-512 in the future. */

#ifdef _MSC_VER
#define DRWAV_INLINE __forceinline
#else
#ifdef __GNUC__
#define DRWAV_INLINE __inline__ __attribute__((always_inline))
#else
#define DRWAV_INLINE
#endif
#endif

#if defined(SIZE_MAX)
    #define DRWAV_SIZE_MAX  SIZE_MAX
#else
    #if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
        #define DRWAV_SIZE_MAX  ((drwav_uint64)0xFFFFFFFFFFFFFFFF)
    #else
        #define DRWAV_SIZE_MAX  0xFFFFFFFF
    #endif
#endif

static const drwav_uint8 drwavGUID_W64_RIFF[16] = {0x72,0x69,0x66,0x66, 0x2E,0x91, 0xCF,0x11, 0xA5,0xD6, 0x28,0xDB,0x04,0xC1,0x00,0x00};    /* 66666972-912E-11CF-A5D6-28DB04C10000 */
static const drwav_uint8 drwavGUID_W64_WAVE[16] = {0x77,0x61,0x76,0x65, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    /* 65766177-ACF3-11D3-8CD1-00C04F8EDB8A */
static const drwav_uint8 drwavGUID_W64_JUNK[16] = {0x6A,0x75,0x6E,0x6B, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    /* 6B6E756A-ACF3-11D3-8CD1-00C04F8EDB8A */
static const drwav_uint8 drwavGUID_W64_FMT [16] = {0x66,0x6D,0x74,0x20, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    /* 20746D66-ACF3-11D3-8CD1-00C04F8EDB8A */
static const drwav_uint8 drwavGUID_W64_FACT[16] = {0x66,0x61,0x63,0x74, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    /* 74636166-ACF3-11D3-8CD1-00C04F8EDB8A */
static const drwav_uint8 drwavGUID_W64_DATA[16] = {0x64,0x61,0x74,0x61, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    /* 61746164-ACF3-11D3-8CD1-00C04F8EDB8A */
static const drwav_uint8 drwavGUID_W64_SMPL[16] = {0x73,0x6D,0x70,0x6C, 0xF3,0xAC, 0xD3,0x11, 0x8C,0xD1, 0x00,0xC0,0x4F,0x8E,0xDB,0x8A};    /* 6C706D73-ACF3-11D3-8CD1-00C04F8EDB8A */

static DRWAV_INLINE drwav_bool32 drwav__guid_equal(const drwav_uint8 a[16], const drwav_uint8 b[16])
{
    const drwav_uint32* a32 = (const drwav_uint32*)a;
    const drwav_uint32* b32 = (const drwav_uint32*)b;

    return
        a32[0] == b32[0] &&
        a32[1] == b32[1] &&
        a32[2] == b32[2] &&
        a32[3] == b32[3];
}

static DRWAV_INLINE drwav_bool32 drwav__fourcc_equal(const unsigned char* a, const char* b)
{
    return
        a[0] == b[0] &&
        a[1] == b[1] &&
        a[2] == b[2] &&
        a[3] == b[3];
}



static DRWAV_INLINE int drwav__is_little_endian()
{
    int n = 1;
    return (*(char*)&n) == 1;
}

static DRWAV_INLINE unsigned short drwav__bytes_to_u16(const unsigned char* data)
{
    return (data[0] << 0) | (data[1] << 8);
}

static DRWAV_INLINE short drwav__bytes_to_s16(const unsigned char* data)
{
    return (short)drwav__bytes_to_u16(data);
}

static DRWAV_INLINE unsigned int drwav__bytes_to_u32(const unsigned char* data)
{
    return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static DRWAV_INLINE drwav_uint64 drwav__bytes_to_u64(const unsigned char* data)
{
    return
        ((drwav_uint64)data[0] <<  0) | ((drwav_uint64)data[1] <<  8) | ((drwav_uint64)data[2] << 16) | ((drwav_uint64)data[3] << 24) |
        ((drwav_uint64)data[4] << 32) | ((drwav_uint64)data[5] << 40) | ((drwav_uint64)data[6] << 48) | ((drwav_uint64)data[7] << 56);
}

static DRWAV_INLINE void drwav__bytes_to_guid(const unsigned char* data, drwav_uint8* guid)
{
    int i;
    for (i = 0; i < 16; ++i) {
        guid[i] = data[i];
    }
}


static DRWAV_INLINE drwav_bool32 drwav__is_compressed_format_tag(drwav_uint16 formatTag)
{
    return
        formatTag == DR_WAVE_FORMAT_ADPCM ||
        formatTag == DR_WAVE_FORMAT_DVI_ADPCM;
}

drwav_uint64 drwav_read_s16__msadpcm(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut);
drwav_uint64 drwav_read_s16__ima(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut);
drwav_bool32 drwav_init_write__internal(drwav* pWav, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData);
drwav* drwav_open_write__internal(const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData);

static drwav_result drwav__read_chunk_header(drwav_read_proc onRead, void* pUserData, drwav_container container, drwav_uint64* pRunningBytesReadOut, drwav_chunk_header* pHeaderOut)
{
    if (container == drwav_container_riff) {
        unsigned char sizeInBytes[4];

        if (onRead(pUserData, pHeaderOut->id.fourcc, 4) != 4) {
            return DRWAV_EOF;
        }

        if (onRead(pUserData, sizeInBytes, 4) != 4) {
            return DRWAV_INVALID_FILE;
        }

        pHeaderOut->sizeInBytes = drwav__bytes_to_u32(sizeInBytes);
        pHeaderOut->paddingSize = (unsigned int)(pHeaderOut->sizeInBytes % 2);
        *pRunningBytesReadOut += 8;
    } else {
        unsigned char sizeInBytes[8];

        if (onRead(pUserData, pHeaderOut->id.guid, 16) != 16) {
            return DRWAV_EOF;
        }

        if (onRead(pUserData, sizeInBytes, 8) != 8) {
            return DRWAV_INVALID_FILE;
        }

        pHeaderOut->sizeInBytes = drwav__bytes_to_u64(sizeInBytes) - 24;    /* <-- Subtract 24 because w64 includes the size of the header. */
        pHeaderOut->paddingSize = (unsigned int)(pHeaderOut->sizeInBytes % 8);
        *pRunningBytesReadOut += 24;
    }

    return DRWAV_SUCCESS;
}

static drwav_bool32 drwav__seek_forward(drwav_seek_proc onSeek, drwav_uint64 offset, void* pUserData)
{
    drwav_uint64 bytesRemainingToSeek = offset;
    while (bytesRemainingToSeek > 0) {
        if (bytesRemainingToSeek > 0x7FFFFFFF) {
            if (!onSeek(pUserData, 0x7FFFFFFF, drwav_seek_origin_current)) {
                return DRWAV_FALSE;
            }
            bytesRemainingToSeek -= 0x7FFFFFFF;
        } else {
            if (!onSeek(pUserData, (int)bytesRemainingToSeek, drwav_seek_origin_current)) {
                return DRWAV_FALSE;
            }
            bytesRemainingToSeek = 0;
        }
    }

    return DRWAV_TRUE;
}

static drwav_bool32 drwav__seek_from_start(drwav_seek_proc onSeek, drwav_uint64 offset, void* pUserData)
{
    if (offset <= 0x7FFFFFFF) {
        return onSeek(pUserData, (int)offset, drwav_seek_origin_start);
    }

    /* Larger than 32-bit seek. */
    if (!onSeek(pUserData, 0x7FFFFFFF, drwav_seek_origin_start)) {
        return DRWAV_FALSE;
    }
    offset -= 0x7FFFFFFF;

    for (;;) {
        if (offset <= 0x7FFFFFFF) {
            return onSeek(pUserData, (int)offset, drwav_seek_origin_current);
        }

        if (!onSeek(pUserData, 0x7FFFFFFF, drwav_seek_origin_current)) {
            return DRWAV_FALSE;
        }
        offset -= 0x7FFFFFFF;
    }

    /* Should never get here. */
    /*return DRWAV_TRUE; */
}


static drwav_bool32 drwav__read_fmt(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, drwav_container container, drwav_uint64* pRunningBytesReadOut, drwav_fmt* fmtOut)
{
    drwav_chunk_header header;
    unsigned char fmt[16];

    if (drwav__read_chunk_header(onRead, pUserData, container, pRunningBytesReadOut, &header) != DRWAV_SUCCESS) {
        return DRWAV_FALSE;
    }


    /* Skip non-fmt chunks. */
    while ((container == drwav_container_riff && !drwav__fourcc_equal(header.id.fourcc, "fmt ")) || (container == drwav_container_w64 && !drwav__guid_equal(header.id.guid, drwavGUID_W64_FMT))) {
        if (!drwav__seek_forward(onSeek, header.sizeInBytes + header.paddingSize, pUserData)) {
            return DRWAV_FALSE;
        }
        *pRunningBytesReadOut += header.sizeInBytes + header.paddingSize;

        /* Try the next header. */
        if (drwav__read_chunk_header(onRead, pUserData, container, pRunningBytesReadOut, &header) != DRWAV_SUCCESS) {
            return DRWAV_FALSE;
        }
    }


    /* Validation. */
    if (container == drwav_container_riff) {
        if (!drwav__fourcc_equal(header.id.fourcc, "fmt ")) {
            return DRWAV_FALSE;
        }
    } else {
        if (!drwav__guid_equal(header.id.guid, drwavGUID_W64_FMT)) {
            return DRWAV_FALSE;
        }
    }


    if (onRead(pUserData, fmt, sizeof(fmt)) != sizeof(fmt)) {
        return DRWAV_FALSE;
    }
    *pRunningBytesReadOut += sizeof(fmt);

    fmtOut->formatTag      = drwav__bytes_to_u16(fmt + 0);
    fmtOut->channels       = drwav__bytes_to_u16(fmt + 2);
    fmtOut->sampleRate     = drwav__bytes_to_u32(fmt + 4);
    fmtOut->avgBytesPerSec = drwav__bytes_to_u32(fmt + 8);
    fmtOut->blockAlign     = drwav__bytes_to_u16(fmt + 12);
    fmtOut->bitsPerSample  = drwav__bytes_to_u16(fmt + 14);

    fmtOut->extendedSize       = 0;
    fmtOut->validBitsPerSample = 0;
    fmtOut->channelMask        = 0;
    memset(fmtOut->subFormat, 0, sizeof(fmtOut->subFormat));

    if (header.sizeInBytes > 16) {
        unsigned char fmt_cbSize[2];
        int bytesReadSoFar = 0;

        if (onRead(pUserData, fmt_cbSize, sizeof(fmt_cbSize)) != sizeof(fmt_cbSize)) {
            return DRWAV_FALSE;    /* Expecting more data. */
        }
        *pRunningBytesReadOut += sizeof(fmt_cbSize);

        bytesReadSoFar = 18;

        fmtOut->extendedSize = drwav__bytes_to_u16(fmt_cbSize);
        if (fmtOut->extendedSize > 0) {
            /* Simple validation. */
            if (fmtOut->formatTag == DR_WAVE_FORMAT_EXTENSIBLE) {
                if (fmtOut->extendedSize != 22) {
                    return DRWAV_FALSE;
                }
            }

            if (fmtOut->formatTag == DR_WAVE_FORMAT_EXTENSIBLE) {
                unsigned char fmtext[22];
                if (onRead(pUserData, fmtext, fmtOut->extendedSize) != fmtOut->extendedSize) {
                    return DRWAV_FALSE;    /* Expecting more data. */
                }

                fmtOut->validBitsPerSample = drwav__bytes_to_u16(fmtext + 0);
                fmtOut->channelMask        = drwav__bytes_to_u32(fmtext + 2);
                drwav__bytes_to_guid(fmtext + 6, fmtOut->subFormat);
            } else {
                if (!onSeek(pUserData, fmtOut->extendedSize, drwav_seek_origin_current)) {
                    return DRWAV_FALSE;
                }
            }
            *pRunningBytesReadOut += fmtOut->extendedSize;

            bytesReadSoFar += fmtOut->extendedSize;
        }

        /* Seek past any leftover bytes. For w64 the leftover will be defined based on the chunk size. */
        if (!onSeek(pUserData, (int)(header.sizeInBytes - bytesReadSoFar), drwav_seek_origin_current)) {
            return DRWAV_FALSE;
        }
        *pRunningBytesReadOut += (header.sizeInBytes - bytesReadSoFar);
    }

    if (header.paddingSize > 0) {
        if (!onSeek(pUserData, header.paddingSize, drwav_seek_origin_current)) {
            return DRWAV_FALSE;
        }
        *pRunningBytesReadOut += header.paddingSize;
    }

    return DRWAV_TRUE;
}


#ifndef DR_WAV_NO_STDIO
FILE* drwav_fopen(const char* filePath, const char* openMode)
{
    FILE* pFile;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    if (fopen_s(&pFile, filePath, openMode) != 0) {
        return DRWAV_FALSE;
    }
#else
    pFile = fopen(filePath, openMode);
    if (pFile == NULL) {
        return DRWAV_FALSE;
    }
#endif

    return pFile;
}

static size_t drwav__on_read_stdio(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    return fread(pBufferOut, 1, bytesToRead, (FILE*)pUserData);
}

static size_t drwav__on_write_stdio(void* pUserData, const void* pData, size_t bytesToWrite)
{
    return fwrite(pData, 1, bytesToWrite, (FILE*)pUserData);
}

static drwav_bool32 drwav__on_seek_stdio(void* pUserData, int offset, drwav_seek_origin origin)
{
    return fseek((FILE*)pUserData, offset, (origin == drwav_seek_origin_current) ? SEEK_CUR : SEEK_SET) == 0;
}

drwav_bool32 drwav_init_file(drwav* pWav, const char* filename)
{
    return drwav_init_file_ex(pWav, filename, NULL, NULL, 0);
}

drwav_bool32 drwav_init_file_ex(drwav* pWav, const char* filename, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags)
{
    FILE* pFile = drwav_fopen(filename, "rb");
    if (pFile == NULL) {
        return DRWAV_FALSE;
    }

    return drwav_init_ex(pWav, drwav__on_read_stdio, drwav__on_seek_stdio, onChunk, (void*)pFile, pChunkUserData, flags);
}


drwav_bool32 drwav_init_file_write__internal(drwav* pWav, const char* filename, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential)
{
    FILE* pFile = drwav_fopen(filename, "wb");
    if (pFile == NULL) {
        return DRWAV_FALSE;
    }

    return drwav_init_write__internal(pWav, pFormat, totalSampleCount, isSequential, drwav__on_write_stdio, drwav__on_seek_stdio, (void*)pFile);
}

drwav_bool32 drwav_init_file_write(drwav* pWav, const char* filename, const drwav_data_format* pFormat)
{
    return drwav_init_file_write__internal(pWav, filename, pFormat, 0, DRWAV_FALSE);
}

drwav_bool32 drwav_init_file_write_sequential(drwav* pWav, const char* filename, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount)
{
    return drwav_init_file_write__internal(pWav, filename, pFormat, totalSampleCount, DRWAV_TRUE);
}

drwav* drwav_open_file(const char* filename)
{
    return drwav_open_file_ex(filename, NULL, NULL, 0);
}

drwav* drwav_open_file_ex(const char* filename, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags)
{
    FILE* pFile;
    drwav* pWav;

    pFile = drwav_fopen(filename, "rb");
    if (pFile == NULL) {
        return DRWAV_FALSE;
    }

    pWav = drwav_open_ex(drwav__on_read_stdio, drwav__on_seek_stdio, onChunk, (void*)pFile, pChunkUserData, flags);
    if (pWav == NULL) {
        fclose(pFile);
        return NULL;
    }

    return pWav;
}


drwav* drwav_open_file_write__internal(const char* filename, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential)
{
    FILE* pFile;
    drwav* pWav;

    pFile = drwav_fopen(filename, "wb");
    if (pFile == NULL) {
        return DRWAV_FALSE;
    }

    pWav = drwav_open_write__internal(pFormat, totalSampleCount, isSequential, drwav__on_write_stdio, drwav__on_seek_stdio, (void*)pFile);
    if (pWav == NULL) {
        fclose(pFile);
        return NULL;
    }

    return pWav;
}

drwav* drwav_open_file_write(const char* filename, const drwav_data_format* pFormat)
{
    return drwav_open_file_write__internal(filename, pFormat, 0, DRWAV_FALSE);
}

drwav* drwav_open_file_write_sequential(const char* filename, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount)
{
    return drwav_open_file_write__internal(filename, pFormat, totalSampleCount, DRWAV_TRUE);
}
#endif  /* DR_WAV_NO_STDIO */


static size_t drwav__on_read_memory(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    drwav__memory_stream* memory = (drwav__memory_stream*)pUserData;
    size_t bytesRemaining;

    drwav_assert(memory != NULL);
    drwav_assert(memory->dataSize >= memory->currentReadPos);

    bytesRemaining = memory->dataSize - memory->currentReadPos;
    if (bytesToRead > bytesRemaining) {
        bytesToRead = bytesRemaining;
    }

    if (bytesToRead > 0) {
        DRWAV_COPY_MEMORY(pBufferOut, memory->data + memory->currentReadPos, bytesToRead);
        memory->currentReadPos += bytesToRead;
    }

    return bytesToRead;
}

static drwav_bool32 drwav__on_seek_memory(void* pUserData, int offset, drwav_seek_origin origin)
{
    drwav__memory_stream* memory = (drwav__memory_stream*)pUserData;
    drwav_assert(memory != NULL);

    if (origin == drwav_seek_origin_current) {
        if (offset > 0) {
            if (memory->currentReadPos + offset > memory->dataSize) {
                return DRWAV_FALSE; /* Trying to seek too far forward. */
            }
        } else {
            if (memory->currentReadPos < (size_t)-offset) {
                return DRWAV_FALSE; /* Trying to seek too far backwards. */
            }
        }

        /* This will never underflow thanks to the clamps above. */
        memory->currentReadPos += offset;
    } else {
        if ((drwav_uint32)offset <= memory->dataSize) {
            memory->currentReadPos = offset;
        } else {
            return DRWAV_FALSE; /* Trying to seek too far forward. */
        }
    }
    
    return DRWAV_TRUE;
}

static size_t drwav__on_write_memory(void* pUserData, const void* pDataIn, size_t bytesToWrite)
{
    drwav__memory_stream_write* memory = (drwav__memory_stream_write*)pUserData;
    size_t bytesRemaining;

    drwav_assert(memory != NULL);
    drwav_assert(memory->dataCapacity >= memory->currentWritePos);

    bytesRemaining = memory->dataCapacity - memory->currentWritePos;
    if (bytesRemaining < bytesToWrite) {
        /* Need to reallocate. */
        void* pNewData;
        size_t newDataCapacity = (memory->dataCapacity == 0) ? 256 : memory->dataCapacity * 2;

        /* If doubling wasn't enough, just make it the minimum required size to write the data. */
        if ((newDataCapacity - memory->currentWritePos) < bytesToWrite) {
            newDataCapacity = memory->currentWritePos + bytesToWrite;
        }

        pNewData = DRWAV_REALLOC(*memory->ppData, newDataCapacity);
        if (pNewData == NULL) {
            return 0;
        }

        *memory->ppData = pNewData;
        memory->dataCapacity = newDataCapacity;
    }

    DRWAV_COPY_MEMORY(((drwav_uint8*)(*memory->ppData)) + memory->currentWritePos, pDataIn, bytesToWrite);

    memory->currentWritePos += bytesToWrite;
    if (memory->dataSize < memory->currentWritePos) {
        memory->dataSize = memory->currentWritePos;
    }

    *memory->pDataSize = memory->dataSize;

    return bytesToWrite;
}

static drwav_bool32 drwav__on_seek_memory_write(void* pUserData, int offset, drwav_seek_origin origin)
{
    drwav__memory_stream_write* memory = (drwav__memory_stream_write*)pUserData;
    drwav_assert(memory != NULL);

    if (origin == drwav_seek_origin_current) {
        if (offset > 0) {
            if (memory->currentWritePos + offset > memory->dataSize) {
                offset = (int)(memory->dataSize - memory->currentWritePos);  /* Trying to seek too far forward. */
            }
        } else {
            if (memory->currentWritePos < (size_t)-offset) {
                offset = -(int)memory->currentWritePos;  /* Trying to seek too far backwards. */
            }
        }

        /* This will never underflow thanks to the clamps above. */
        memory->currentWritePos += offset;
    } else {
        if ((drwav_uint32)offset <= memory->dataSize) {
            memory->currentWritePos = offset;
        } else {
            memory->currentWritePos = memory->dataSize;  /* Trying to seek too far forward. */
        }
    }
    
    return DRWAV_TRUE;
}

drwav_bool32 drwav_init_memory(drwav* pWav, const void* data, size_t dataSize)
{
    return drwav_init_memory_ex(pWav, data, dataSize, NULL, NULL, 0);
}

drwav_bool32 drwav_init_memory_ex(drwav* pWav, const void* data, size_t dataSize, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags)
{
    drwav__memory_stream memoryStream;

    if (data == NULL || dataSize == 0) {
        return DRWAV_FALSE;
    }

    drwav_zero_memory(&memoryStream, sizeof(memoryStream));
    memoryStream.data = (const unsigned char*)data;
    memoryStream.dataSize = dataSize;
    memoryStream.currentReadPos = 0;

    if (!drwav_init_ex(pWav, drwav__on_read_memory, drwav__on_seek_memory, onChunk, (void*)&memoryStream, pChunkUserData, flags)) {
        return DRWAV_FALSE;
    }

    pWav->memoryStream = memoryStream;
    pWav->pUserData = &pWav->memoryStream;
    return DRWAV_TRUE;
}


drwav_bool32 drwav_init_memory_write__internal(drwav* pWav, void** ppData, size_t* pDataSize, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential)
{
    drwav__memory_stream_write memoryStreamWrite;

    if (ppData == NULL) {
        return DRWAV_FALSE;
    }

    *ppData = NULL; /* Important because we're using realloc()! */
    *pDataSize = 0;

    drwav_zero_memory(&memoryStreamWrite, sizeof(memoryStreamWrite));
    memoryStreamWrite.ppData = ppData;
    memoryStreamWrite.pDataSize = pDataSize;
    memoryStreamWrite.dataSize = 0;
    memoryStreamWrite.dataCapacity = 0;
    memoryStreamWrite.currentWritePos = 0;

    if (!drwav_init_write__internal(pWav, pFormat, totalSampleCount, isSequential, drwav__on_write_memory, drwav__on_seek_memory_write, (void*)&memoryStreamWrite)) {
        return DRWAV_FALSE;
    }

    pWav->memoryStreamWrite = memoryStreamWrite;
    pWav->pUserData = &pWav->memoryStreamWrite;
    return DRWAV_TRUE;
}

drwav_bool32 drwav_init_memory_write(drwav* pWav, void** ppData, size_t* pDataSize, const drwav_data_format* pFormat)
{
    return drwav_init_memory_write__internal(pWav, ppData, pDataSize, pFormat, 0, DRWAV_FALSE);
}

drwav_bool32 drwav_init_memory_write_sequential(drwav* pWav, void** ppData, size_t* pDataSize, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount)
{
    return drwav_init_memory_write__internal(pWav, ppData, pDataSize, pFormat, totalSampleCount, DRWAV_TRUE);
}


drwav* drwav_open_memory(const void* data, size_t dataSize)
{
    return drwav_open_memory_ex(data, dataSize, NULL, NULL, 0);
}

drwav* drwav_open_memory_ex(const void* data, size_t dataSize, drwav_chunk_proc onChunk, void* pChunkUserData, drwav_uint32 flags)
{
    drwav__memory_stream memoryStream;
    drwav* pWav;

    if (data == NULL || dataSize == 0) {
        return NULL;
    }

    drwav_zero_memory(&memoryStream, sizeof(memoryStream));
    memoryStream.data = (const unsigned char*)data;
    memoryStream.dataSize = dataSize;
    memoryStream.currentReadPos = 0;

    pWav = drwav_open_ex(drwav__on_read_memory, drwav__on_seek_memory, onChunk, (void*)&memoryStream, pChunkUserData, flags);
    if (pWav == NULL) {
        return NULL;
    }

    pWav->memoryStream = memoryStream;
    pWav->pUserData = &pWav->memoryStream;
    return pWav;
}


drwav* drwav_open_memory_write__internal(void** ppData, size_t* pDataSize, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential)
{
    drwav__memory_stream_write memoryStreamWrite;
    drwav* pWav;

    if (ppData == NULL) {
        return NULL;
    }

    *ppData = NULL; /* Important because we're using realloc()! */
    *pDataSize = 0;

    drwav_zero_memory(&memoryStreamWrite, sizeof(memoryStreamWrite));
    memoryStreamWrite.ppData = ppData;
    memoryStreamWrite.pDataSize = pDataSize;
    memoryStreamWrite.dataSize = 0;
    memoryStreamWrite.dataCapacity = 0;
    memoryStreamWrite.currentWritePos = 0;

    pWav = drwav_open_write__internal(pFormat, totalSampleCount, isSequential, drwav__on_write_memory, drwav__on_seek_memory_write, (void*)&memoryStreamWrite);
    if (pWav == NULL) {
        return NULL;
    }

    pWav->memoryStreamWrite = memoryStreamWrite;
    pWav->pUserData = &pWav->memoryStreamWrite;
    return pWav;
}

drwav* drwav_open_memory_write(void** ppData, size_t* pDataSize, const drwav_data_format* pFormat)
{
    return drwav_open_memory_write__internal(ppData, pDataSize, pFormat, 0, DRWAV_FALSE);
}

drwav* drwav_open_memory_write_sequential(void** ppData, size_t* pDataSize, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount)
{
    return drwav_open_memory_write__internal(ppData, pDataSize, pFormat, totalSampleCount, DRWAV_TRUE);
}


size_t drwav__on_read(drwav_read_proc onRead, void* pUserData, void* pBufferOut, size_t bytesToRead, drwav_uint64* pCursor)
{
    size_t bytesRead;

    drwav_assert(onRead != NULL);
    drwav_assert(pCursor != NULL);

    bytesRead = onRead(pUserData, pBufferOut, bytesToRead);
    *pCursor += bytesRead;
    return bytesRead;
}

drwav_bool32 drwav__on_seek(drwav_seek_proc onSeek, void* pUserData, int offset, drwav_seek_origin origin, drwav_uint64* pCursor)
{
    drwav_assert(onSeek != NULL);
    drwav_assert(pCursor != NULL);

    if (!onSeek(pUserData, offset, origin)) {
        return DRWAV_FALSE;
    }

    if (origin == drwav_seek_origin_start) {
        *pCursor = offset;
    } else {
        *pCursor += offset;
    }

    return DRWAV_TRUE;
}


static drwav_uint32 drwav_get_bytes_per_sample(drwav* pWav)
{
    /*
    The number of bytes per sample is based on the bits per sample or the block align. We prioritize floor(bitsPerSample/8), but if
    this is zero or the bits per sample is not a multiple of 8 we need to fall back to the block align.
    */
    drwav_uint32 bytesPerSample = pWav->bitsPerSample >> 3;
    if (bytesPerSample == 0 || (pWav->bitsPerSample & 0x7) != 0) {
        bytesPerSample = pWav->fmt.blockAlign/pWav->fmt.channels;
    }

    return bytesPerSample;
}

static drwav_uint32 drwav_get_bytes_per_pcm_frame(drwav* pWav)
{
    /*
    The number of bytes per frame is based on the bits per sample or the block align. We prioritize floor(bitsPerSample*channels/8), but if
    this is zero or the bits per frame is not a multiple of 8 we need to fall back to the block align.
    */
    drwav_uint32 bitsPerFrame = pWav->bitsPerSample * pWav->fmt.channels;
    drwav_uint32 bytesPerFrame = bitsPerFrame >> 3;
    if (bytesPerFrame == 0 || (bitsPerFrame & 0x7) != 0) {
        bytesPerFrame = pWav->fmt.blockAlign;
    }

    return bytesPerFrame;
}


drwav_bool32 drwav_init(drwav* pWav, drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData)
{
    return drwav_init_ex(pWav, onRead, onSeek, NULL, pUserData, NULL, 0);
}

drwav_bool32 drwav_init_ex(drwav* pWav, drwav_read_proc onRead, drwav_seek_proc onSeek, drwav_chunk_proc onChunk, void* pReadSeekUserData, void* pChunkUserData, drwav_uint32 flags)
{
    drwav_uint64 cursor;    /* <-- Keeps track of the byte position so we can seek to specific locations. */
    drwav_bool32 sequential;
    unsigned char riff[4];
    drwav_fmt fmt;
    unsigned short translatedFormatTag;
    drwav_uint64 sampleCountFromFactChunk;
    drwav_bool32 foundDataChunk;
    drwav_uint64 dataChunkSize;
    drwav_uint64 chunkSize;

    if (onRead == NULL || onSeek == NULL) {
        return DRWAV_FALSE;
    }

    cursor = 0;
    sequential = (flags & DRWAV_SEQUENTIAL) != 0;

    drwav_zero_memory(pWav, sizeof(*pWav));
    pWav->onRead    = onRead;
    pWav->onSeek    = onSeek;
    pWav->pUserData = pReadSeekUserData;

    /* The first 4 bytes should be the RIFF identifier. */
    if (drwav__on_read(onRead, pReadSeekUserData, riff, sizeof(riff), &cursor) != sizeof(riff)) {
        return DRWAV_FALSE;
    }

    /*
    The first 4 bytes can be used to identify the container. For RIFF files it will start with "RIFF" and for
    w64 it will start with "riff".
    */
    if (drwav__fourcc_equal(riff, "RIFF")) {
        pWav->container = drwav_container_riff;
    } else if (drwav__fourcc_equal(riff, "riff")) {
        int i;
        drwav_uint8 riff2[12];

        pWav->container = drwav_container_w64;

        /* Check the rest of the GUID for validity. */
        if (drwav__on_read(onRead, pReadSeekUserData, riff2, sizeof(riff2), &cursor) != sizeof(riff2)) {
            return DRWAV_FALSE;
        }

        for (i = 0; i < 12; ++i) {
            if (riff2[i] != drwavGUID_W64_RIFF[i+4]) {
                return DRWAV_FALSE;
            }
        }
    } else {
        return DRWAV_FALSE;   /* Unknown or unsupported container. */
    }


    if (pWav->container == drwav_container_riff) {
        unsigned char chunkSizeBytes[4];
        unsigned char wave[4];

        /* RIFF/WAVE */
        if (drwav__on_read(onRead, pReadSeekUserData, chunkSizeBytes, sizeof(chunkSizeBytes), &cursor) != sizeof(chunkSizeBytes)) {
            return DRWAV_FALSE;
        }

        if (drwav__bytes_to_u32(chunkSizeBytes) < 36) {
            return DRWAV_FALSE;    /* Chunk size should always be at least 36 bytes. */
        }

        if (drwav__on_read(onRead, pReadSeekUserData, wave, sizeof(wave), &cursor) != sizeof(wave)) {
            return DRWAV_FALSE;
        }

        if (!drwav__fourcc_equal(wave, "WAVE")) {
            return DRWAV_FALSE;    /* Expecting "WAVE". */
        }
    } else {
        unsigned char chunkSizeBytes[8];
        drwav_uint8 wave[16];

        /* W64 */
        if (drwav__on_read(onRead, pReadSeekUserData, chunkSizeBytes, sizeof(chunkSizeBytes), &cursor) != sizeof(chunkSizeBytes)) {
            return DRWAV_FALSE;
        }

        if (drwav__bytes_to_u64(chunkSizeBytes) < 80) {
            return DRWAV_FALSE;
        }

        if (drwav__on_read(onRead, pReadSeekUserData, wave, sizeof(wave), &cursor) != sizeof(wave)) {
            return DRWAV_FALSE;
        }

        if (!drwav__guid_equal(wave, drwavGUID_W64_WAVE)) {
            return DRWAV_FALSE;
        }
    }


    /* The next bytes should be the "fmt " chunk. */
    if (!drwav__read_fmt(onRead, onSeek, pReadSeekUserData, pWav->container, &cursor, &fmt)) {
        return DRWAV_FALSE;    /* Failed to read the "fmt " chunk. */
    }

    /* Basic validation. */
    if (fmt.sampleRate == 0 || fmt.channels == 0 || fmt.bitsPerSample == 0 || fmt.blockAlign == 0) {
        return DRWAV_FALSE; /* Invalid channel count. Probably an invalid WAV file. */
    }


    /* Translate the internal format. */
    translatedFormatTag = fmt.formatTag;
    if (translatedFormatTag == DR_WAVE_FORMAT_EXTENSIBLE) {
        translatedFormatTag = drwav__bytes_to_u16(fmt.subFormat + 0);
    }



    sampleCountFromFactChunk = 0;

    /*
    We need to enumerate over each chunk for two reasons:
      1) The "data" chunk may not be the next one
      2) We may want to report each chunk back to the client
    
    In order to correctly report each chunk back to the client we will need to keep looping until the end of the file.
    */
    foundDataChunk = DRWAV_FALSE;
    dataChunkSize = 0;

    /* The next chunk we care about is the "data" chunk. This is not necessarily the next chunk so we'll need to loop. */
    chunkSize = 0;
    for (;;)
    {
        drwav_chunk_header header;
        drwav_result result = drwav__read_chunk_header(onRead, pReadSeekUserData, pWav->container, &cursor, &header);
        if (result != DRWAV_SUCCESS) {
            if (!foundDataChunk) {
                return DRWAV_FALSE;
            } else {
                break;  /* Probably at the end of the file. Get out of the loop. */
            }
        }

        /* Tell the client about this chunk. */
        if (!sequential && onChunk != NULL) {
            drwav_uint64 callbackBytesRead = onChunk(pChunkUserData, onRead, onSeek, pReadSeekUserData, &header);

            /*
            dr_wav may need to read the contents of the chunk, so we now need to seek back to the position before
            we called the callback.
            */
            if (callbackBytesRead > 0) {
                if (!drwav__seek_from_start(onSeek, cursor, pReadSeekUserData)) {
                    return DRWAV_FALSE;
                }
            }
        }
        

        if (!foundDataChunk) {
            pWav->dataChunkDataPos = cursor;
        }

        chunkSize = header.sizeInBytes;
        if (pWav->container == drwav_container_riff) {
            if (drwav__fourcc_equal(header.id.fourcc, "data")) {
                foundDataChunk = DRWAV_TRUE;
                dataChunkSize = chunkSize;
            }
        } else {
            if (drwav__guid_equal(header.id.guid, drwavGUID_W64_DATA)) {
                foundDataChunk = DRWAV_TRUE;
                dataChunkSize = chunkSize;
            }
        }

        /*
        If at this point we have found the data chunk and we're running in sequential mode, we need to break out of this loop. The reason for
        this is that we would otherwise require a backwards seek which sequential mode forbids.
        */
        if (foundDataChunk && sequential) {
            break;
        }

        /* Optional. Get the total sample count from the FACT chunk. This is useful for compressed formats. */
        if (pWav->container == drwav_container_riff) {
            if (drwav__fourcc_equal(header.id.fourcc, "fact")) {
                drwav_uint32 sampleCount;
                if (drwav__on_read(onRead, pReadSeekUserData, &sampleCount, 4, &cursor) != 4) {
                    return DRWAV_FALSE;
                }
                chunkSize -= 4;

                if (!foundDataChunk) {
                    pWav->dataChunkDataPos = cursor;
                }

                /*
                The sample count in the "fact" chunk is either unreliable, or I'm not understanding it properly. For now I am only enabling this
                for Microsoft ADPCM formats.
                */
                if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
                    sampleCountFromFactChunk = sampleCount;
                } else {
                    sampleCountFromFactChunk = 0;
                }
            }
        } else {
            if (drwav__guid_equal(header.id.guid, drwavGUID_W64_FACT)) {
                if (drwav__on_read(onRead, pReadSeekUserData, &sampleCountFromFactChunk, 8, &cursor) != 8) {
                    return DRWAV_FALSE;
                }
                chunkSize -= 8;

                if (!foundDataChunk) {
                    pWav->dataChunkDataPos = cursor;
                }
            }
        }

        /* "smpl" chunk. */
        if (pWav->container == drwav_container_riff) {
            if (drwav__fourcc_equal(header.id.fourcc, "smpl")) {
                unsigned char smplHeaderData[36];    /* 36 = size of the smpl header section, not including the loop data. */
                if (chunkSize >= sizeof(smplHeaderData)) {
                    drwav_uint64 bytesJustRead = drwav__on_read(onRead, pReadSeekUserData, smplHeaderData, sizeof(smplHeaderData), &cursor);
                    chunkSize -= bytesJustRead;

                    if (bytesJustRead == sizeof(smplHeaderData)) {
                        drwav_uint32 iLoop;

                        pWav->smpl.manufacturer      = drwav__bytes_to_u32(smplHeaderData+0);
                        pWav->smpl.product           = drwav__bytes_to_u32(smplHeaderData+4);
                        pWav->smpl.samplePeriod      = drwav__bytes_to_u32(smplHeaderData+8);
                        pWav->smpl.midiUnityNotes    = drwav__bytes_to_u32(smplHeaderData+12);
                        pWav->smpl.midiPitchFraction = drwav__bytes_to_u32(smplHeaderData+16);
                        pWav->smpl.smpteFormat       = drwav__bytes_to_u32(smplHeaderData+20);
                        pWav->smpl.smpteOffset       = drwav__bytes_to_u32(smplHeaderData+24);
                        pWav->smpl.numSampleLoops    = drwav__bytes_to_u32(smplHeaderData+28);
                        pWav->smpl.samplerData       = drwav__bytes_to_u32(smplHeaderData+32);

                        for (iLoop = 0; iLoop < pWav->smpl.numSampleLoops && iLoop < drwav_countof(pWav->smpl.loops); ++iLoop) {
                            unsigned char smplLoopData[24];  /* 24 = size of a loop section in the smpl chunk. */
                            bytesJustRead = drwav__on_read(onRead, pReadSeekUserData, smplLoopData, sizeof(smplLoopData), &cursor);
                            chunkSize -= bytesJustRead;

                            if (bytesJustRead == sizeof(smplLoopData)) {
                                pWav->smpl.loops[iLoop].cuePointId = drwav__bytes_to_u32(smplLoopData+0);
                                pWav->smpl.loops[iLoop].type       = drwav__bytes_to_u32(smplLoopData+4);
                                pWav->smpl.loops[iLoop].start      = drwav__bytes_to_u32(smplLoopData+8);
                                pWav->smpl.loops[iLoop].end        = drwav__bytes_to_u32(smplLoopData+12);
                                pWav->smpl.loops[iLoop].fraction   = drwav__bytes_to_u32(smplLoopData+16);
                                pWav->smpl.loops[iLoop].playCount  = drwav__bytes_to_u32(smplLoopData+20);
                            } else {
                                break;  /* Break from the smpl loop for loop. */
                            }
                        }
                    }
                } else {
                    /* Looks like invalid data. Ignore the chunk. */
                }
            }
        } else {
            if (drwav__guid_equal(header.id.guid, drwavGUID_W64_SMPL)) {
                /*
                This path will be hit when a W64 WAV file contains a smpl chunk. I don't have a sample file to test this path, so a contribution
                is welcome to add support for this.
                */
            }
        }

        /* Make sure we seek past the padding. */
        chunkSize += header.paddingSize;
        if (!drwav__seek_forward(onSeek, chunkSize, pReadSeekUserData)) {
            break;
        }
        cursor += chunkSize;

        if (!foundDataChunk) {
            pWav->dataChunkDataPos = cursor;
        }
    }

    /* If we haven't found a data chunk, return an error. */
    if (!foundDataChunk) {
        return DRWAV_FALSE;
    }

    /* We may have moved passed the data chunk. If so we need to move back. If running in sequential mode we can assume we are already sitting on the data chunk. */
    if (!sequential) {
        if (!drwav__seek_from_start(onSeek, pWav->dataChunkDataPos, pReadSeekUserData)) {
            return DRWAV_FALSE;
        }
        cursor = pWav->dataChunkDataPos;
    }
    

    /* At this point we should be sitting on the first byte of the raw audio data. */

    pWav->fmt                 = fmt;
    pWav->sampleRate          = fmt.sampleRate;
    pWav->channels            = fmt.channels;
    pWav->bitsPerSample       = fmt.bitsPerSample;
    pWav->bytesRemaining      = dataChunkSize;
    pWav->translatedFormatTag = translatedFormatTag;
    pWav->dataChunkDataSize   = dataChunkSize;

    if (sampleCountFromFactChunk != 0) {
        pWav->totalPCMFrameCount = sampleCountFromFactChunk;
    } else {
        pWav->totalPCMFrameCount = dataChunkSize / drwav_get_bytes_per_pcm_frame(pWav);

        if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
            drwav_uint64 blockCount = dataChunkSize / fmt.blockAlign;
            pWav->totalPCMFrameCount = (((blockCount * (fmt.blockAlign - (6*pWav->channels))) * 2)) / fmt.channels;  /* x2 because two samples per byte. */
        }
        if (pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
            drwav_uint64 blockCount = dataChunkSize / fmt.blockAlign;
            pWav->totalPCMFrameCount = (((blockCount * (fmt.blockAlign - (4*pWav->channels))) * 2) + (blockCount * pWav->channels)) / fmt.channels;
        }
    }

    /* Some formats only support a certain number of channels. */
    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM || pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
        if (pWav->channels > 2) {
            return DRWAV_FALSE;
        }
    }

#ifdef DR_WAV_LIBSNDFILE_COMPAT
    /*
    I use libsndfile as a benchmark for testing, however in the version I'm using (from the Windows installer on the libsndfile website),
    it appears the total sample count libsndfile uses for MS-ADPCM is incorrect. It would seem they are computing the total sample count
    from the number of blocks, however this results in the inclusion of extra silent samples at the end of the last block. The correct
    way to know the total sample count is to inspect the "fact" chunk, which should always be present for compressed formats, and should
    always include the sample count. This little block of code below is only used to emulate the libsndfile logic so I can properly run my
    correctness tests against libsndfile, and is disabled by default.
    */
    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
        drwav_uint64 blockCount = dataChunkSize / fmt.blockAlign;
        pWav->totalPCMFrameCount = (((blockCount * (fmt.blockAlign - (6*pWav->channels))) * 2)) / fmt.channels;  /* x2 because two samples per byte. */
    }
    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
        drwav_uint64 blockCount = dataChunkSize / fmt.blockAlign;
        pWav->totalPCMFrameCount = (((blockCount * (fmt.blockAlign - (4*pWav->channels))) * 2) + (blockCount * pWav->channels)) / fmt.channels;
    }
#endif

    pWav->totalSampleCount = pWav->totalPCMFrameCount * pWav->channels;

    return DRWAV_TRUE;
}


drwav_uint32 drwav_riff_chunk_size_riff(drwav_uint64 dataChunkSize)
{
    if (dataChunkSize <= (0xFFFFFFFF - 36)) {
        return 36 + (drwav_uint32)dataChunkSize;
    } else {
        return 0xFFFFFFFF;
    }
}

drwav_uint32 drwav_data_chunk_size_riff(drwav_uint64 dataChunkSize)
{
    if (dataChunkSize <= 0xFFFFFFFF) {
        return (drwav_uint32)dataChunkSize;
    } else {
        return 0xFFFFFFFF;
    }
}

drwav_uint64 drwav_riff_chunk_size_w64(drwav_uint64 dataChunkSize)
{
    return 80 + 24 + dataChunkSize;   /* +24 because W64 includes the size of the GUID and size fields. */
}

drwav_uint64 drwav_data_chunk_size_w64(drwav_uint64 dataChunkSize)
{
    return 24 + dataChunkSize;        /* +24 because W64 includes the size of the GUID and size fields. */
}


drwav_bool32 drwav_init_write__internal(drwav* pWav, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData)
{
    size_t runningPos = 0;
    drwav_uint64 initialDataChunkSize = 0;
    drwav_uint64 chunkSizeFMT;

    if (pWav == NULL) {
        return DRWAV_FALSE;
    }

    if (onWrite == NULL) {
        return DRWAV_FALSE;
    }

    if (!isSequential && onSeek == NULL) {
        return DRWAV_FALSE; /* <-- onSeek is required when in non-sequential mode. */
    }


    /* Not currently supporting compressed formats. Will need to add support for the "fact" chunk before we enable this. */
    if (pFormat->format == DR_WAVE_FORMAT_EXTENSIBLE) {
        return DRWAV_FALSE;
    }
    if (pFormat->format == DR_WAVE_FORMAT_ADPCM || pFormat->format == DR_WAVE_FORMAT_DVI_ADPCM) {
        return DRWAV_FALSE;
    }


    drwav_zero_memory(pWav, sizeof(*pWav));
    pWav->onWrite = onWrite;
    pWav->onSeek = onSeek;
    pWav->pUserData = pUserData;
    pWav->fmt.formatTag = (drwav_uint16)pFormat->format;
    pWav->fmt.channels = (drwav_uint16)pFormat->channels;
    pWav->fmt.sampleRate = pFormat->sampleRate;
    pWav->fmt.avgBytesPerSec = (drwav_uint32)((pFormat->bitsPerSample * pFormat->sampleRate * pFormat->channels) / 8);
    pWav->fmt.blockAlign = (drwav_uint16)((pFormat->channels * pFormat->bitsPerSample) / 8);
    pWav->fmt.bitsPerSample = (drwav_uint16)pFormat->bitsPerSample;
    pWav->fmt.extendedSize = 0;
    pWav->isSequentialWrite = isSequential;

    /*
    The initial values for the "RIFF" and "data" chunks depends on whether or not we are initializing in sequential mode or not. In
    sequential mode we set this to its final values straight away since they can be calculated from the total sample count. In non-
    sequential mode we initialize it all to zero and fill it out in drwav_uninit() using a backwards seek.
    */
    if (isSequential) {
        initialDataChunkSize = (totalSampleCount * pWav->fmt.bitsPerSample) / 8;

        /*
        The RIFF container has a limit on the number of samples. drwav is not allowing this. There's no practical limits for Wave64
        so for the sake of simplicity I'm not doing any validation for that.
        */
        if (pFormat->container == drwav_container_riff) {
            if (initialDataChunkSize > (0xFFFFFFFF - 36)) {
                return DRWAV_FALSE; /* Not enough room to store every sample. */
            }
        }
    }

    pWav->dataChunkDataSizeTargetWrite = initialDataChunkSize;


    /* "RIFF" chunk. */
    if (pFormat->container == drwav_container_riff) {
        drwav_uint32 chunkSizeRIFF = 36 + (drwav_uint32)initialDataChunkSize;   /* +36 = "RIFF"+[RIFF Chunk Size]+"WAVE" + [sizeof "fmt " chunk] */
        runningPos += pWav->onWrite(pUserData, "RIFF", 4);
        runningPos += pWav->onWrite(pUserData, &chunkSizeRIFF, 4);
        runningPos += pWav->onWrite(pUserData, "WAVE", 4);
    } else {
        drwav_uint64 chunkSizeRIFF = 80 + 24 + initialDataChunkSize;   /* +24 because W64 includes the size of the GUID and size fields. */
        runningPos += pWav->onWrite(pUserData, drwavGUID_W64_RIFF, 16);
        runningPos += pWav->onWrite(pUserData, &chunkSizeRIFF, 8);
        runningPos += pWav->onWrite(pUserData, drwavGUID_W64_WAVE, 16);
    }

    /* "fmt " chunk. */
    if (pFormat->container == drwav_container_riff) {
        chunkSizeFMT = 16;
        runningPos += pWav->onWrite(pUserData, "fmt ", 4);
        runningPos += pWav->onWrite(pUserData, &chunkSizeFMT, 4);
    } else {
        chunkSizeFMT = 40;
        runningPos += pWav->onWrite(pUserData, drwavGUID_W64_FMT, 16);
        runningPos += pWav->onWrite(pUserData, &chunkSizeFMT, 8);
    }

    runningPos += pWav->onWrite(pUserData, &pWav->fmt.formatTag,      2);
    runningPos += pWav->onWrite(pUserData, &pWav->fmt.channels,       2);
    runningPos += pWav->onWrite(pUserData, &pWav->fmt.sampleRate,     4);
    runningPos += pWav->onWrite(pUserData, &pWav->fmt.avgBytesPerSec, 4);
    runningPos += pWav->onWrite(pUserData, &pWav->fmt.blockAlign,     2);
    runningPos += pWav->onWrite(pUserData, &pWav->fmt.bitsPerSample,  2);

    pWav->dataChunkDataPos = runningPos;

    /* "data" chunk. */
    if (pFormat->container == drwav_container_riff) {
        drwav_uint32 chunkSizeDATA = (drwav_uint32)initialDataChunkSize;
        runningPos += pWav->onWrite(pUserData, "data", 4);
        runningPos += pWav->onWrite(pUserData, &chunkSizeDATA, 4);
    } else {
        drwav_uint64 chunkSizeDATA = 24 + initialDataChunkSize; /* +24 because W64 includes the size of the GUID and size fields. */
        runningPos += pWav->onWrite(pUserData, drwavGUID_W64_DATA, 16);
        runningPos += pWav->onWrite(pUserData, &chunkSizeDATA, 8);
    }


    /* Simple validation. */
    if (pFormat->container == drwav_container_riff) {
        if (runningPos != 20 + chunkSizeFMT + 8) {
            return DRWAV_FALSE;
        }
    } else {
        if (runningPos != 40 + chunkSizeFMT + 24) {
            return DRWAV_FALSE;
        }
    }
    


    /* Set some properties for the client's convenience. */
    pWav->container = pFormat->container;
    pWav->channels = (drwav_uint16)pFormat->channels;
    pWav->sampleRate = pFormat->sampleRate;
    pWav->bitsPerSample = (drwav_uint16)pFormat->bitsPerSample;
    pWav->translatedFormatTag = (drwav_uint16)pFormat->format;

    return DRWAV_TRUE;
}


drwav_bool32 drwav_init_write(drwav* pWav, const drwav_data_format* pFormat, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData)
{
    return drwav_init_write__internal(pWav, pFormat, 0, DRWAV_FALSE, onWrite, onSeek, pUserData);               /* DRWAV_FALSE = Not Sequential */
}

drwav_bool32 drwav_init_write_sequential(drwav* pWav, const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_write_proc onWrite, void* pUserData)
{
    return drwav_init_write__internal(pWav, pFormat, totalSampleCount, DRWAV_TRUE, onWrite, NULL, pUserData);   /* DRWAV_TRUE = Sequential */
}

void drwav_uninit(drwav* pWav)
{
    if (pWav == NULL) {
        return;
    }

    /*
    If the drwav object was opened in write mode we'll need to finalize a few things:
      - Make sure the "data" chunk is aligned to 16-bits for RIFF containers, or 64 bits for W64 containers.
      - Set the size of the "data" chunk.
    */
    if (pWav->onWrite != NULL) {
        drwav_uint32 paddingSize = 0;

        /* Validation for sequential mode. */
        if (pWav->isSequentialWrite) {
            drwav_assert(pWav->dataChunkDataSize == pWav->dataChunkDataSizeTargetWrite);
        }

        /* Padding. Do not adjust pWav->dataChunkDataSize - this should not include the padding. */
        if (pWav->container == drwav_container_riff) {
            paddingSize = (drwav_uint32)(pWav->dataChunkDataSize % 2);
        } else {
            paddingSize = (drwav_uint32)(pWav->dataChunkDataSize % 8);
        }
        
        if (paddingSize > 0) {
            drwav_uint64 paddingData = 0;
            pWav->onWrite(pWav->pUserData, &paddingData, paddingSize);
        }

        /*
        Chunk sizes. When using sequential mode, these will have been filled in at initialization time. We only need
        to do this when using non-sequential mode.
        */
        if (pWav->onSeek && !pWav->isSequentialWrite) {
            if (pWav->container == drwav_container_riff) {
                /* The "RIFF" chunk size. */
                if (pWav->onSeek(pWav->pUserData, 4, drwav_seek_origin_start)) {
                    drwav_uint32 riffChunkSize = drwav_riff_chunk_size_riff(pWav->dataChunkDataSize);
                    pWav->onWrite(pWav->pUserData, &riffChunkSize, 4);
                }

                /* the "data" chunk size. */
                if (pWav->onSeek(pWav->pUserData, (int)pWav->dataChunkDataPos + 4, drwav_seek_origin_start)) {
                    drwav_uint32 dataChunkSize = drwav_data_chunk_size_riff(pWav->dataChunkDataSize);
                    pWav->onWrite(pWav->pUserData, &dataChunkSize, 4);
                }
            } else {
                /* The "RIFF" chunk size. */
                if (pWav->onSeek(pWav->pUserData, 16, drwav_seek_origin_start)) {
                    drwav_uint64 riffChunkSize = drwav_riff_chunk_size_w64(pWav->dataChunkDataSize);
                    pWav->onWrite(pWav->pUserData, &riffChunkSize, 8);
                }

                /* The "data" chunk size. */
                if (pWav->onSeek(pWav->pUserData, (int)pWav->dataChunkDataPos + 16, drwav_seek_origin_start)) {
                    drwav_uint64 dataChunkSize = drwav_data_chunk_size_w64(pWav->dataChunkDataSize);
                    pWav->onWrite(pWav->pUserData, &dataChunkSize, 8);
                }
            }
        }
    }

#ifndef DR_WAV_NO_STDIO
    /*
    If we opened the file with drwav_open_file() we will want to close the file handle. We can know whether or not drwav_open_file()
    was used by looking at the onRead and onSeek callbacks.
    */
    if (pWav->onRead == drwav__on_read_stdio || pWav->onWrite == drwav__on_write_stdio) {
        fclose((FILE*)pWav->pUserData);
    }
#endif
}


drwav* drwav_open(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData)
{
    return drwav_open_ex(onRead, onSeek, NULL, pUserData, NULL, 0);
}

drwav* drwav_open_ex(drwav_read_proc onRead, drwav_seek_proc onSeek, drwav_chunk_proc onChunk, void* pReadSeekUserData, void* pChunkUserData, drwav_uint32 flags)
{
    drwav* pWav = (drwav*)DRWAV_MALLOC(sizeof(*pWav));
    if (pWav == NULL) {
        return NULL;
    }

    if (!drwav_init_ex(pWav, onRead, onSeek, onChunk, pReadSeekUserData, pChunkUserData, flags)) {
        DRWAV_FREE(pWav);
        return NULL;
    }

    return pWav;
}


drwav* drwav_open_write__internal(const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_bool32 isSequential, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData)
{
    drwav* pWav = (drwav*)DRWAV_MALLOC(sizeof(*pWav));
    if (pWav == NULL) {
        return NULL;
    }

    if (!drwav_init_write__internal(pWav, pFormat, totalSampleCount, isSequential, onWrite, onSeek, pUserData)) {
        DRWAV_FREE(pWav);
        return NULL;
    }

    return pWav;
}

drwav* drwav_open_write(const drwav_data_format* pFormat, drwav_write_proc onWrite, drwav_seek_proc onSeek, void* pUserData)
{
    return drwav_open_write__internal(pFormat, 0, DRWAV_FALSE, onWrite, onSeek, pUserData);
}

drwav* drwav_open_write_sequential(const drwav_data_format* pFormat, drwav_uint64 totalSampleCount, drwav_write_proc onWrite, void* pUserData)
{
    return drwav_open_write__internal(pFormat, totalSampleCount, DRWAV_TRUE, onWrite, NULL, pUserData);
}

void drwav_close(drwav* pWav)
{
    drwav_uninit(pWav);
    DRWAV_FREE(pWav);
}


size_t drwav_read_raw(drwav* pWav, size_t bytesToRead, void* pBufferOut)
{
    size_t bytesRead;

    if (pWav == NULL || bytesToRead == 0 || pBufferOut == NULL) {
        return 0;
    }

    if (bytesToRead > pWav->bytesRemaining) {
        bytesToRead = (size_t)pWav->bytesRemaining;
    }

    bytesRead = pWav->onRead(pWav->pUserData, pBufferOut, bytesToRead);

    pWav->bytesRemaining -= bytesRead;
    return bytesRead;
}

drwav_uint64 drwav_read(drwav* pWav, drwav_uint64 samplesToRead, void* pBufferOut)
{
    drwav_uint32 bytesPerSample;
    size_t bytesRead;

    if (pWav == NULL || samplesToRead == 0 || pBufferOut == NULL) {
        return 0;
    }

    /* Cannot use this function for compressed formats. */
    if (drwav__is_compressed_format_tag(pWav->translatedFormatTag)) {
        return 0;
    }

    bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    /* Don't try to read more samples than can potentially fit in the output buffer. */
    if (samplesToRead * bytesPerSample > DRWAV_SIZE_MAX) {
        samplesToRead = DRWAV_SIZE_MAX / bytesPerSample;
    }

    bytesRead = drwav_read_raw(pWav, (size_t)(samplesToRead * bytesPerSample), pBufferOut);
    return bytesRead / bytesPerSample;
}

drwav_uint64 drwav_read_pcm_frames(drwav* pWav, drwav_uint64 framesToRead, void* pBufferOut)
{
    drwav_uint32 bytesPerFrame;
    size_t bytesRead;

    if (pWav == NULL || framesToRead == 0 || pBufferOut == NULL) {
        return 0;
    }

    /* Cannot use this function for compressed formats. */
    if (drwav__is_compressed_format_tag(pWav->translatedFormatTag)) {
        return 0;
    }

    bytesPerFrame = drwav_get_bytes_per_pcm_frame(pWav);
    if (bytesPerFrame == 0) {
        return 0;
    }

    /* Don't try to read more samples than can potentially fit in the output buffer. */
    if (framesToRead * bytesPerFrame > DRWAV_SIZE_MAX) {
        framesToRead = DRWAV_SIZE_MAX / bytesPerFrame;
    }

    bytesRead = drwav_read_raw(pWav, (size_t)(framesToRead * bytesPerFrame), pBufferOut);
    return bytesRead / bytesPerFrame;
}

drwav_bool32 drwav_seek_to_first_pcm_frame(drwav* pWav)
{
    if (pWav->onWrite != NULL) {
        return DRWAV_FALSE; /* No seeking in write mode. */
    }

    if (!pWav->onSeek(pWav->pUserData, (int)pWav->dataChunkDataPos, drwav_seek_origin_start)) {
        return DRWAV_FALSE;
    }

    if (drwav__is_compressed_format_tag(pWav->translatedFormatTag)) {
        pWav->compressed.iCurrentSample = 0;
    }
    
    pWav->bytesRemaining = pWav->dataChunkDataSize;
    return DRWAV_TRUE;
}

drwav_bool32 drwav_seek_to_sample(drwav* pWav, drwav_uint64 sample)
{
    /* Seeking should be compatible with wave files > 2GB. */

    if (pWav->onWrite != NULL) {
        return DRWAV_FALSE; /* No seeking in write mode. */
    }

    if (pWav == NULL || pWav->onSeek == NULL) {
        return DRWAV_FALSE;
    }

    /* If there are no samples, just return DRWAV_TRUE without doing anything. */
    if (pWav->totalSampleCount == 0) {
        return DRWAV_TRUE;
    }

    /* Make sure the sample is clamped. */
    if (sample >= pWav->totalSampleCount) {
        sample  = pWav->totalSampleCount - 1;
    }

    /*
    For compressed formats we just use a slow generic seek. If we are seeking forward we just seek forward. If we are going backwards we need
    to seek back to the start.
    */
    if (drwav__is_compressed_format_tag(pWav->translatedFormatTag)) {
        /* TODO: This can be optimized. */
        
        /*
        If we're seeking forward it's simple - just keep reading samples until we hit the sample we're requesting. If we're seeking backwards,
        we first need to seek back to the start and then just do the same thing as a forward seek.
        */
        if (sample < pWav->compressed.iCurrentSample) {
            if (!drwav_seek_to_first_pcm_frame(pWav)) {
                return DRWAV_FALSE;
            }
        }

        if (sample > pWav->compressed.iCurrentSample) {
            drwav_uint64 offset = sample - pWav->compressed.iCurrentSample;

            drwav_int16 devnull[2048];
            while (offset > 0) {
                drwav_uint64 samplesRead = 0;
                drwav_uint64 samplesToRead = offset;
                if (samplesToRead > 2048) {
                    samplesToRead = 2048;
                }

                if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
                    samplesRead = drwav_read_s16__msadpcm(pWav, samplesToRead, devnull);
                } else if (pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
                    samplesRead = drwav_read_s16__ima(pWav, samplesToRead, devnull);
                } else {
                    assert(DRWAV_FALSE);    /* If this assertion is triggered it means I've implemented a new compressed format but forgot to add a branch for it here. */
                }

                if (samplesRead != samplesToRead) {
                    return DRWAV_FALSE;
                }

                offset -= samplesRead;
            }
        }
    } else {
        drwav_uint64 totalSizeInBytes;
        drwav_uint64 currentBytePos;
        drwav_uint64 targetBytePos;
        drwav_uint64 offset;

        totalSizeInBytes = pWav->totalPCMFrameCount * drwav_get_bytes_per_pcm_frame(pWav);
        drwav_assert(totalSizeInBytes >= pWav->bytesRemaining);

        currentBytePos = totalSizeInBytes - pWav->bytesRemaining;
        targetBytePos  = sample * drwav_get_bytes_per_sample(pWav);

        if (currentBytePos < targetBytePos) {
            /* Offset forwards. */
            offset = (targetBytePos - currentBytePos);
        } else {
            /* Offset backwards. */
            if (!drwav_seek_to_first_pcm_frame(pWav)) {
                return DRWAV_FALSE;
            }
            offset = targetBytePos;
        }

        while (offset > 0) {
            int offset32 = ((offset > INT_MAX) ? INT_MAX : (int)offset);
            if (!pWav->onSeek(pWav->pUserData, offset32, drwav_seek_origin_current)) {
                return DRWAV_FALSE;
            }

            pWav->bytesRemaining -= offset32;
            offset -= offset32;
        }
    }

    return DRWAV_TRUE;
}

drwav_bool32 drwav_seek_to_pcm_frame(drwav* pWav, drwav_uint64 targetFrameIndex)
{
    return drwav_seek_to_sample(pWav, targetFrameIndex * pWav->channels);
}


size_t drwav_write_raw(drwav* pWav, size_t bytesToWrite, const void* pData)
{
    size_t bytesWritten;

    if (pWav == NULL || bytesToWrite == 0 || pData == NULL) {
        return 0;
    }

    bytesWritten = pWav->onWrite(pWav->pUserData, pData, bytesToWrite);
    pWav->dataChunkDataSize += bytesWritten;

    return bytesWritten;
}

drwav_uint64 drwav_write(drwav* pWav, drwav_uint64 samplesToWrite, const void* pData)
{
    drwav_uint64 bytesToWrite;
    drwav_uint64 bytesWritten;
    const drwav_uint8* pRunningData;

    if (pWav == NULL || samplesToWrite == 0 || pData == NULL) {
        return 0;
    }

    bytesToWrite = ((samplesToWrite * pWav->bitsPerSample) / 8);
    if (bytesToWrite > DRWAV_SIZE_MAX) {
        return 0;
    }

    bytesWritten = 0;
    pRunningData = (const drwav_uint8*)pData;
    while (bytesToWrite > 0) {
        size_t bytesJustWritten;
        drwav_uint64 bytesToWriteThisIteration = bytesToWrite;
        if (bytesToWriteThisIteration > DRWAV_SIZE_MAX) {
            bytesToWriteThisIteration = DRWAV_SIZE_MAX;
        }

        bytesJustWritten = drwav_write_raw(pWav, (size_t)bytesToWriteThisIteration, pRunningData);
        if (bytesJustWritten == 0) {
            break;
        }

        bytesToWrite -= bytesJustWritten;
        bytesWritten += bytesJustWritten;
        pRunningData += bytesJustWritten;
    }

    return (bytesWritten * 8) / pWav->bitsPerSample;
}

drwav_uint64 drwav_write_pcm_frames(drwav* pWav, drwav_uint64 framesToWrite, const void* pData)
{
    return drwav_write(pWav, framesToWrite * pWav->channels, pData) / pWav->channels;
}



drwav_uint64 drwav_read_s16__msadpcm(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    drwav_uint64 totalSamplesRead = 0;

    drwav_assert(pWav != NULL);
    drwav_assert(samplesToRead > 0);
    drwav_assert(pBufferOut != NULL);

    /* TODO: Lots of room for optimization here. */

    while (samplesToRead > 0 && pWav->compressed.iCurrentSample < pWav->totalSampleCount) {
        /* If there are no cached samples we need to load a new block. */
        if (pWav->msadpcm.cachedSampleCount == 0 && pWav->msadpcm.bytesRemainingInBlock == 0) {
            if (pWav->channels == 1) {
                /* Mono. */
                drwav_uint8 header[7];
                if (pWav->onRead(pWav->pUserData, header, sizeof(header)) != sizeof(header)) {
                    return totalSamplesRead;
                }
                pWav->msadpcm.bytesRemainingInBlock = pWav->fmt.blockAlign - sizeof(header);

                pWav->msadpcm.predictor[0] = header[0];
                pWav->msadpcm.delta[0] = drwav__bytes_to_s16(header + 1);
                pWav->msadpcm.prevSamples[0][1] = (drwav_int32)drwav__bytes_to_s16(header + 3);
                pWav->msadpcm.prevSamples[0][0] = (drwav_int32)drwav__bytes_to_s16(header + 5);
                pWav->msadpcm.cachedSamples[2] = pWav->msadpcm.prevSamples[0][0];
                pWav->msadpcm.cachedSamples[3] = pWav->msadpcm.prevSamples[0][1];
                pWav->msadpcm.cachedSampleCount = 2;
            } else {
                /* Stereo. */
                drwav_uint8 header[14];
                if (pWav->onRead(pWav->pUserData, header, sizeof(header)) != sizeof(header)) {
                    return totalSamplesRead;
                }
                pWav->msadpcm.bytesRemainingInBlock = pWav->fmt.blockAlign - sizeof(header);

                pWav->msadpcm.predictor[0] = header[0];
                pWav->msadpcm.predictor[1] = header[1];
                pWav->msadpcm.delta[0] = drwav__bytes_to_s16(header + 2);
                pWav->msadpcm.delta[1] = drwav__bytes_to_s16(header + 4);
                pWav->msadpcm.prevSamples[0][1] = (drwav_int32)drwav__bytes_to_s16(header + 6);
                pWav->msadpcm.prevSamples[1][1] = (drwav_int32)drwav__bytes_to_s16(header + 8);
                pWav->msadpcm.prevSamples[0][0] = (drwav_int32)drwav__bytes_to_s16(header + 10);
                pWav->msadpcm.prevSamples[1][0] = (drwav_int32)drwav__bytes_to_s16(header + 12);

                pWav->msadpcm.cachedSamples[0] = pWav->msadpcm.prevSamples[0][0];
                pWav->msadpcm.cachedSamples[1] = pWav->msadpcm.prevSamples[1][0];
                pWav->msadpcm.cachedSamples[2] = pWav->msadpcm.prevSamples[0][1];
                pWav->msadpcm.cachedSamples[3] = pWav->msadpcm.prevSamples[1][1];
                pWav->msadpcm.cachedSampleCount = 4;
            }
        }

        /* Output anything that's cached. */
        while (samplesToRead > 0 && pWav->msadpcm.cachedSampleCount > 0 && pWav->compressed.iCurrentSample < pWav->totalSampleCount) {
            pBufferOut[0] = (drwav_int16)pWav->msadpcm.cachedSamples[drwav_countof(pWav->msadpcm.cachedSamples) - pWav->msadpcm.cachedSampleCount];
            pWav->msadpcm.cachedSampleCount -= 1;

            pBufferOut += 1;
            samplesToRead -= 1;
            totalSamplesRead += 1;
            pWav->compressed.iCurrentSample += 1;
        }

        if (samplesToRead == 0) {
            return totalSamplesRead;
        }


        /*
        If there's nothing left in the cache, just go ahead and load more. If there's nothing left to load in the current block we just continue to the next
        loop iteration which will trigger the loading of a new block.
        */
        if (pWav->msadpcm.cachedSampleCount == 0) {
            if (pWav->msadpcm.bytesRemainingInBlock == 0) {
                continue;
            } else {
                static drwav_int32 adaptationTable[] = { 
                    230, 230, 230, 230, 307, 409, 512, 614, 
                    768, 614, 512, 409, 307, 230, 230, 230 
                };
                static drwav_int32 coeff1Table[] = { 256, 512, 0, 192, 240, 460,  392 };
                static drwav_int32 coeff2Table[] = { 0,  -256, 0, 64,  0,  -208, -232 };

                drwav_uint8 nibbles;
                drwav_int32 nibble0;
                drwav_int32 nibble1;

                if (pWav->onRead(pWav->pUserData, &nibbles, 1) != 1) {
                    return totalSamplesRead;
                }
                pWav->msadpcm.bytesRemainingInBlock -= 1;

                /* TODO: Optimize away these if statements. */
                nibble0 = ((nibbles & 0xF0) >> 4); if ((nibbles & 0x80)) { nibble0 |= 0xFFFFFFF0UL; }
                nibble1 = ((nibbles & 0x0F) >> 0); if ((nibbles & 0x08)) { nibble1 |= 0xFFFFFFF0UL; }

                if (pWav->channels == 1) {
                    /* Mono. */
                    drwav_int32 newSample0;
                    drwav_int32 newSample1;

                    newSample0  = ((pWav->msadpcm.prevSamples[0][1] * coeff1Table[pWav->msadpcm.predictor[0]]) + (pWav->msadpcm.prevSamples[0][0] * coeff2Table[pWav->msadpcm.predictor[0]])) >> 8;
                    newSample0 += nibble0 * pWav->msadpcm.delta[0];
                    newSample0  = drwav_clamp(newSample0, -32768, 32767);

                    pWav->msadpcm.delta[0] = (adaptationTable[((nibbles & 0xF0) >> 4)] * pWav->msadpcm.delta[0]) >> 8;
                    if (pWav->msadpcm.delta[0] < 16) {
                        pWav->msadpcm.delta[0] = 16;
                    }

                    pWav->msadpcm.prevSamples[0][0] = pWav->msadpcm.prevSamples[0][1];
                    pWav->msadpcm.prevSamples[0][1] = newSample0;


                    newSample1  = ((pWav->msadpcm.prevSamples[0][1] * coeff1Table[pWav->msadpcm.predictor[0]]) + (pWav->msadpcm.prevSamples[0][0] * coeff2Table[pWav->msadpcm.predictor[0]])) >> 8;
                    newSample1 += nibble1 * pWav->msadpcm.delta[0];
                    newSample1  = drwav_clamp(newSample1, -32768, 32767);

                    pWav->msadpcm.delta[0] = (adaptationTable[((nibbles & 0x0F) >> 0)] * pWav->msadpcm.delta[0]) >> 8;
                    if (pWav->msadpcm.delta[0] < 16) {
                        pWav->msadpcm.delta[0] = 16;
                    }

                    pWav->msadpcm.prevSamples[0][0] = pWav->msadpcm.prevSamples[0][1];
                    pWav->msadpcm.prevSamples[0][1] = newSample1;


                    pWav->msadpcm.cachedSamples[2] = newSample0;
                    pWav->msadpcm.cachedSamples[3] = newSample1;
                    pWav->msadpcm.cachedSampleCount = 2;
                } else {
                    /* Stereo. */
                    drwav_int32 newSample0;
                    drwav_int32 newSample1;

                    /* Left. */
                    newSample0  = ((pWav->msadpcm.prevSamples[0][1] * coeff1Table[pWav->msadpcm.predictor[0]]) + (pWav->msadpcm.prevSamples[0][0] * coeff2Table[pWav->msadpcm.predictor[0]])) >> 8;
                    newSample0 += nibble0 * pWav->msadpcm.delta[0];
                    newSample0  = drwav_clamp(newSample0, -32768, 32767);

                    pWav->msadpcm.delta[0] = (adaptationTable[((nibbles & 0xF0) >> 4)] * pWav->msadpcm.delta[0]) >> 8;
                    if (pWav->msadpcm.delta[0] < 16) {
                        pWav->msadpcm.delta[0] = 16;
                    }

                    pWav->msadpcm.prevSamples[0][0] = pWav->msadpcm.prevSamples[0][1];
                    pWav->msadpcm.prevSamples[0][1] = newSample0;


                    /* Right. */
                    newSample1  = ((pWav->msadpcm.prevSamples[1][1] * coeff1Table[pWav->msadpcm.predictor[1]]) + (pWav->msadpcm.prevSamples[1][0] * coeff2Table[pWav->msadpcm.predictor[1]])) >> 8;
                    newSample1 += nibble1 * pWav->msadpcm.delta[1];
                    newSample1  = drwav_clamp(newSample1, -32768, 32767);

                    pWav->msadpcm.delta[1] = (adaptationTable[((nibbles & 0x0F) >> 0)] * pWav->msadpcm.delta[1]) >> 8;
                    if (pWav->msadpcm.delta[1] < 16) {
                        pWav->msadpcm.delta[1] = 16;
                    }

                    pWav->msadpcm.prevSamples[1][0] = pWav->msadpcm.prevSamples[1][1];
                    pWav->msadpcm.prevSamples[1][1] = newSample1;

                    pWav->msadpcm.cachedSamples[2] = newSample0;
                    pWav->msadpcm.cachedSamples[3] = newSample1;
                    pWav->msadpcm.cachedSampleCount = 2;
                }
            }
        }
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s16__ima(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    drwav_uint64 totalSamplesRead = 0;

    drwav_assert(pWav != NULL);
    drwav_assert(samplesToRead > 0);
    drwav_assert(pBufferOut != NULL);

    /* TODO: Lots of room for optimization here. */

    while (samplesToRead > 0 && pWav->compressed.iCurrentSample < pWav->totalSampleCount) {
        /* If there are no cached samples we need to load a new block. */
        if (pWav->ima.cachedSampleCount == 0 && pWav->ima.bytesRemainingInBlock == 0) {
            if (pWav->channels == 1) {
                /* Mono. */
                drwav_uint8 header[4];
                if (pWav->onRead(pWav->pUserData, header, sizeof(header)) != sizeof(header)) {
                    return totalSamplesRead;
                }
                pWav->ima.bytesRemainingInBlock = pWav->fmt.blockAlign - sizeof(header);

                pWav->ima.predictor[0] = drwav__bytes_to_s16(header + 0);
                pWav->ima.stepIndex[0] = header[2];
                pWav->ima.cachedSamples[drwav_countof(pWav->ima.cachedSamples) - 1] = pWav->ima.predictor[0];
                pWav->ima.cachedSampleCount = 1;
            } else {
                /* Stereo. */
                drwav_uint8 header[8];
                if (pWav->onRead(pWav->pUserData, header, sizeof(header)) != sizeof(header)) {
                    return totalSamplesRead;
                }
                pWav->ima.bytesRemainingInBlock = pWav->fmt.blockAlign - sizeof(header);

                pWav->ima.predictor[0] = drwav__bytes_to_s16(header + 0);
                pWav->ima.stepIndex[0] = header[2];
                pWav->ima.predictor[1] = drwav__bytes_to_s16(header + 4);
                pWav->ima.stepIndex[1] = header[6];

                pWav->ima.cachedSamples[drwav_countof(pWav->ima.cachedSamples) - 2] = pWav->ima.predictor[0];
                pWav->ima.cachedSamples[drwav_countof(pWav->ima.cachedSamples) - 1] = pWav->ima.predictor[1];
                pWav->ima.cachedSampleCount = 2;
            }
        }

        /* Output anything that's cached. */
        while (samplesToRead > 0 && pWav->ima.cachedSampleCount > 0 && pWav->compressed.iCurrentSample < pWav->totalSampleCount) {
            pBufferOut[0] = (drwav_int16)pWav->ima.cachedSamples[drwav_countof(pWav->ima.cachedSamples) - pWav->ima.cachedSampleCount];
            pWav->ima.cachedSampleCount -= 1;

            pBufferOut += 1;
            samplesToRead -= 1;
            totalSamplesRead += 1;
            pWav->compressed.iCurrentSample += 1;
        }

        if (samplesToRead == 0) {
            return totalSamplesRead;
        }

        /*
        If there's nothing left in the cache, just go ahead and load more. If there's nothing left to load in the current block we just continue to the next
        loop iteration which will trigger the loading of a new block.
        */
        if (pWav->ima.cachedSampleCount == 0) {
            if (pWav->ima.bytesRemainingInBlock == 0) {
                continue;
            } else {
                static drwav_int32 indexTable[16] = {
                    -1, -1, -1, -1, 2, 4, 6, 8,
                    -1, -1, -1, -1, 2, 4, 6, 8
                };

                static drwav_int32 stepTable[89] = { 
                    7,     8,     9,     10,    11,    12,    13,    14,    16,    17, 
                    19,    21,    23,    25,    28,    31,    34,    37,    41,    45, 
                    50,    55,    60,    66,    73,    80,    88,    97,    107,   118, 
                    130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
                    337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
                    876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066, 
                    2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
                    5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899, 
                    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 
                };

                drwav_uint32 iChannel;

                /*
                From what I can tell with stereo streams, it looks like every 4 bytes (8 samples) is for one channel. So it goes 4 bytes for the
                left channel, 4 bytes for the right channel.
                */
                pWav->ima.cachedSampleCount = 8 * pWav->channels;
                for (iChannel = 0; iChannel < pWav->channels; ++iChannel) {
                    drwav_uint32 iByte;
                    drwav_uint8 nibbles[4];
                    if (pWav->onRead(pWav->pUserData, &nibbles, 4) != 4) {
                        return totalSamplesRead;
                    }
                    pWav->ima.bytesRemainingInBlock -= 4;

                    for (iByte = 0; iByte < 4; ++iByte) {
                        drwav_uint8 nibble0 = ((nibbles[iByte] & 0x0F) >> 0);
                        drwav_uint8 nibble1 = ((nibbles[iByte] & 0xF0) >> 4);

                        drwav_int32 step      = stepTable[pWav->ima.stepIndex[iChannel]];
                        drwav_int32 predictor = pWav->ima.predictor[iChannel];

                        drwav_int32      diff  = step >> 3;
                        if (nibble0 & 1) diff += step >> 2;
                        if (nibble0 & 2) diff += step >> 1;
                        if (nibble0 & 4) diff += step;
                        if (nibble0 & 8) diff  = -diff;

                        predictor = drwav_clamp(predictor + diff, -32768, 32767);
                        pWav->ima.predictor[iChannel] = predictor;
                        pWav->ima.stepIndex[iChannel] = drwav_clamp(pWav->ima.stepIndex[iChannel] + indexTable[nibble0], 0, (drwav_int32)drwav_countof(stepTable)-1);
                        pWav->ima.cachedSamples[(drwav_countof(pWav->ima.cachedSamples) - pWav->ima.cachedSampleCount) + (iByte*2+0)*pWav->channels + iChannel] = predictor;


                        step      = stepTable[pWav->ima.stepIndex[iChannel]];
                        predictor = pWav->ima.predictor[iChannel];

                                         diff  = step >> 3;
                        if (nibble1 & 1) diff += step >> 2;
                        if (nibble1 & 2) diff += step >> 1;
                        if (nibble1 & 4) diff += step;
                        if (nibble1 & 8) diff  = -diff;

                        predictor = drwav_clamp(predictor + diff, -32768, 32767);
                        pWav->ima.predictor[iChannel] = predictor;
                        pWav->ima.stepIndex[iChannel] = drwav_clamp(pWav->ima.stepIndex[iChannel] + indexTable[nibble1], 0, (drwav_int32)drwav_countof(stepTable)-1);
                        pWav->ima.cachedSamples[(drwav_countof(pWav->ima.cachedSamples) - pWav->ima.cachedSampleCount) + (iByte*2+1)*pWav->channels + iChannel] = predictor;
                    }
                }
            }
        }
    }

    return totalSamplesRead;
}


#ifndef DR_WAV_NO_CONVERSION_API
static unsigned short g_drwavAlawTable[256] = {
    0xEA80, 0xEB80, 0xE880, 0xE980, 0xEE80, 0xEF80, 0xEC80, 0xED80, 0xE280, 0xE380, 0xE080, 0xE180, 0xE680, 0xE780, 0xE480, 0xE580, 
    0xF540, 0xF5C0, 0xF440, 0xF4C0, 0xF740, 0xF7C0, 0xF640, 0xF6C0, 0xF140, 0xF1C0, 0xF040, 0xF0C0, 0xF340, 0xF3C0, 0xF240, 0xF2C0, 
    0xAA00, 0xAE00, 0xA200, 0xA600, 0xBA00, 0xBE00, 0xB200, 0xB600, 0x8A00, 0x8E00, 0x8200, 0x8600, 0x9A00, 0x9E00, 0x9200, 0x9600, 
    0xD500, 0xD700, 0xD100, 0xD300, 0xDD00, 0xDF00, 0xD900, 0xDB00, 0xC500, 0xC700, 0xC100, 0xC300, 0xCD00, 0xCF00, 0xC900, 0xCB00, 
    0xFEA8, 0xFEB8, 0xFE88, 0xFE98, 0xFEE8, 0xFEF8, 0xFEC8, 0xFED8, 0xFE28, 0xFE38, 0xFE08, 0xFE18, 0xFE68, 0xFE78, 0xFE48, 0xFE58, 
    0xFFA8, 0xFFB8, 0xFF88, 0xFF98, 0xFFE8, 0xFFF8, 0xFFC8, 0xFFD8, 0xFF28, 0xFF38, 0xFF08, 0xFF18, 0xFF68, 0xFF78, 0xFF48, 0xFF58, 
    0xFAA0, 0xFAE0, 0xFA20, 0xFA60, 0xFBA0, 0xFBE0, 0xFB20, 0xFB60, 0xF8A0, 0xF8E0, 0xF820, 0xF860, 0xF9A0, 0xF9E0, 0xF920, 0xF960, 
    0xFD50, 0xFD70, 0xFD10, 0xFD30, 0xFDD0, 0xFDF0, 0xFD90, 0xFDB0, 0xFC50, 0xFC70, 0xFC10, 0xFC30, 0xFCD0, 0xFCF0, 0xFC90, 0xFCB0, 
    0x1580, 0x1480, 0x1780, 0x1680, 0x1180, 0x1080, 0x1380, 0x1280, 0x1D80, 0x1C80, 0x1F80, 0x1E80, 0x1980, 0x1880, 0x1B80, 0x1A80, 
    0x0AC0, 0x0A40, 0x0BC0, 0x0B40, 0x08C0, 0x0840, 0x09C0, 0x0940, 0x0EC0, 0x0E40, 0x0FC0, 0x0F40, 0x0CC0, 0x0C40, 0x0DC0, 0x0D40, 
    0x5600, 0x5200, 0x5E00, 0x5A00, 0x4600, 0x4200, 0x4E00, 0x4A00, 0x7600, 0x7200, 0x7E00, 0x7A00, 0x6600, 0x6200, 0x6E00, 0x6A00, 
    0x2B00, 0x2900, 0x2F00, 0x2D00, 0x2300, 0x2100, 0x2700, 0x2500, 0x3B00, 0x3900, 0x3F00, 0x3D00, 0x3300, 0x3100, 0x3700, 0x3500, 
    0x0158, 0x0148, 0x0178, 0x0168, 0x0118, 0x0108, 0x0138, 0x0128, 0x01D8, 0x01C8, 0x01F8, 0x01E8, 0x0198, 0x0188, 0x01B8, 0x01A8, 
    0x0058, 0x0048, 0x0078, 0x0068, 0x0018, 0x0008, 0x0038, 0x0028, 0x00D8, 0x00C8, 0x00F8, 0x00E8, 0x0098, 0x0088, 0x00B8, 0x00A8, 
    0x0560, 0x0520, 0x05E0, 0x05A0, 0x0460, 0x0420, 0x04E0, 0x04A0, 0x0760, 0x0720, 0x07E0, 0x07A0, 0x0660, 0x0620, 0x06E0, 0x06A0, 
    0x02B0, 0x0290, 0x02F0, 0x02D0, 0x0230, 0x0210, 0x0270, 0x0250, 0x03B0, 0x0390, 0x03F0, 0x03D0, 0x0330, 0x0310, 0x0370, 0x0350
};

static unsigned short g_drwavMulawTable[256] = {
    0x8284, 0x8684, 0x8A84, 0x8E84, 0x9284, 0x9684, 0x9A84, 0x9E84, 0xA284, 0xA684, 0xAA84, 0xAE84, 0xB284, 0xB684, 0xBA84, 0xBE84, 
    0xC184, 0xC384, 0xC584, 0xC784, 0xC984, 0xCB84, 0xCD84, 0xCF84, 0xD184, 0xD384, 0xD584, 0xD784, 0xD984, 0xDB84, 0xDD84, 0xDF84, 
    0xE104, 0xE204, 0xE304, 0xE404, 0xE504, 0xE604, 0xE704, 0xE804, 0xE904, 0xEA04, 0xEB04, 0xEC04, 0xED04, 0xEE04, 0xEF04, 0xF004, 
    0xF0C4, 0xF144, 0xF1C4, 0xF244, 0xF2C4, 0xF344, 0xF3C4, 0xF444, 0xF4C4, 0xF544, 0xF5C4, 0xF644, 0xF6C4, 0xF744, 0xF7C4, 0xF844, 
    0xF8A4, 0xF8E4, 0xF924, 0xF964, 0xF9A4, 0xF9E4, 0xFA24, 0xFA64, 0xFAA4, 0xFAE4, 0xFB24, 0xFB64, 0xFBA4, 0xFBE4, 0xFC24, 0xFC64, 
    0xFC94, 0xFCB4, 0xFCD4, 0xFCF4, 0xFD14, 0xFD34, 0xFD54, 0xFD74, 0xFD94, 0xFDB4, 0xFDD4, 0xFDF4, 0xFE14, 0xFE34, 0xFE54, 0xFE74, 
    0xFE8C, 0xFE9C, 0xFEAC, 0xFEBC, 0xFECC, 0xFEDC, 0xFEEC, 0xFEFC, 0xFF0C, 0xFF1C, 0xFF2C, 0xFF3C, 0xFF4C, 0xFF5C, 0xFF6C, 0xFF7C, 
    0xFF88, 0xFF90, 0xFF98, 0xFFA0, 0xFFA8, 0xFFB0, 0xFFB8, 0xFFC0, 0xFFC8, 0xFFD0, 0xFFD8, 0xFFE0, 0xFFE8, 0xFFF0, 0xFFF8, 0x0000, 
    0x7D7C, 0x797C, 0x757C, 0x717C, 0x6D7C, 0x697C, 0x657C, 0x617C, 0x5D7C, 0x597C, 0x557C, 0x517C, 0x4D7C, 0x497C, 0x457C, 0x417C, 
    0x3E7C, 0x3C7C, 0x3A7C, 0x387C, 0x367C, 0x347C, 0x327C, 0x307C, 0x2E7C, 0x2C7C, 0x2A7C, 0x287C, 0x267C, 0x247C, 0x227C, 0x207C, 
    0x1EFC, 0x1DFC, 0x1CFC, 0x1BFC, 0x1AFC, 0x19FC, 0x18FC, 0x17FC, 0x16FC, 0x15FC, 0x14FC, 0x13FC, 0x12FC, 0x11FC, 0x10FC, 0x0FFC, 
    0x0F3C, 0x0EBC, 0x0E3C, 0x0DBC, 0x0D3C, 0x0CBC, 0x0C3C, 0x0BBC, 0x0B3C, 0x0ABC, 0x0A3C, 0x09BC, 0x093C, 0x08BC, 0x083C, 0x07BC, 
    0x075C, 0x071C, 0x06DC, 0x069C, 0x065C, 0x061C, 0x05DC, 0x059C, 0x055C, 0x051C, 0x04DC, 0x049C, 0x045C, 0x041C, 0x03DC, 0x039C, 
    0x036C, 0x034C, 0x032C, 0x030C, 0x02EC, 0x02CC, 0x02AC, 0x028C, 0x026C, 0x024C, 0x022C, 0x020C, 0x01EC, 0x01CC, 0x01AC, 0x018C, 
    0x0174, 0x0164, 0x0154, 0x0144, 0x0134, 0x0124, 0x0114, 0x0104, 0x00F4, 0x00E4, 0x00D4, 0x00C4, 0x00B4, 0x00A4, 0x0094, 0x0084, 
    0x0078, 0x0070, 0x0068, 0x0060, 0x0058, 0x0050, 0x0048, 0x0040, 0x0038, 0x0030, 0x0028, 0x0020, 0x0018, 0x0010, 0x0008, 0x0000
};

static DRWAV_INLINE drwav_int16 drwav__alaw_to_s16(drwav_uint8 sampleIn)
{
    return (short)g_drwavAlawTable[sampleIn];
}

static DRWAV_INLINE drwav_int16 drwav__mulaw_to_s16(drwav_uint8 sampleIn)
{
    return (short)g_drwavMulawTable[sampleIn];
}



static void drwav__pcm_to_s16(drwav_int16* pOut, const unsigned char* pIn, size_t totalSampleCount, unsigned int bytesPerSample)
{
    unsigned int i;

    /* Special case for 8-bit sample data because it's treated as unsigned. */
    if (bytesPerSample == 1) {
        drwav_u8_to_s16(pOut, pIn, totalSampleCount);
        return;
    }


    /* Slightly more optimal implementation for common formats. */
    if (bytesPerSample == 2) {
        for (i = 0; i < totalSampleCount; ++i) {
           *pOut++ = ((const drwav_int16*)pIn)[i];
        }
        return;
    }
    if (bytesPerSample == 3) {
        drwav_s24_to_s16(pOut, pIn, totalSampleCount);
        return;
    }
    if (bytesPerSample == 4) {
        drwav_s32_to_s16(pOut, (const drwav_int32*)pIn, totalSampleCount);
        return;
    }


    /* Anything more than 64 bits per sample is not supported. */
    if (bytesPerSample > 8) {
        drwav_zero_memory(pOut, totalSampleCount * sizeof(*pOut));
        return;
    }


    /* Generic, slow converter. */
    for (i = 0; i < totalSampleCount; ++i) {
        drwav_uint64 sample = 0;
        unsigned int shift  = (8 - bytesPerSample) * 8;

        unsigned int j;
        for (j = 0; j < bytesPerSample && j < 8; j += 1) {
            sample |= (drwav_uint64)(pIn[j]) << shift;
            shift  += 8;
        }

        pIn += j;
        *pOut++ = (drwav_int16)((drwav_int64)sample >> 48);
    }
}

static void drwav__ieee_to_s16(drwav_int16* pOut, const unsigned char* pIn, size_t totalSampleCount, unsigned int bytesPerSample)
{
    if (bytesPerSample == 4) {
        drwav_f32_to_s16(pOut, (const float*)pIn, totalSampleCount);
        return;
    } else if (bytesPerSample == 8) {
        drwav_f64_to_s16(pOut, (const double*)pIn, totalSampleCount);
        return;
    } else {
        /* Only supporting 32- and 64-bit float. Output silence in all other cases. Contributions welcome for 16-bit float. */
        drwav_zero_memory(pOut, totalSampleCount * sizeof(*pOut));
        return;
    }
}

drwav_uint64 drwav_read_s16__pcm(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    drwav_uint32 bytesPerSample;
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    /* Fast path. */
    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_PCM && pWav->bitsPerSample == 16) {
        return drwav_read(pWav, samplesToRead, pBufferOut);
    }
    
    bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;
    
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav__pcm_to_s16(pBufferOut, sampleData, (size_t)samplesRead, bytesPerSample);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s16__ieee(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;
    
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav__ieee_to_s16(pBufferOut, sampleData, (size_t)samplesRead, bytesPerSample);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s16__alaw(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;
    
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav_alaw_to_s16(pBufferOut, sampleData, (size_t)samplesRead);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s16__mulaw(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav_mulaw_to_s16(pBufferOut, sampleData, (size_t)samplesRead);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s16(drwav* pWav, drwav_uint64 samplesToRead, drwav_int16* pBufferOut)
{
    if (pWav == NULL || samplesToRead == 0 || pBufferOut == NULL) {
        return 0;
    }

    /* Don't try to read more samples than can potentially fit in the output buffer. */
    if (samplesToRead * sizeof(drwav_int16) > DRWAV_SIZE_MAX) {
        samplesToRead = DRWAV_SIZE_MAX / sizeof(drwav_int16);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_PCM) {
        return drwav_read_s16__pcm(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
        return drwav_read_s16__msadpcm(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT) {
        return drwav_read_s16__ieee(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ALAW) {
        return drwav_read_s16__alaw(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_MULAW) {
        return drwav_read_s16__mulaw(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
        return drwav_read_s16__ima(pWav, samplesToRead, pBufferOut);
    }

    return 0;
}

drwav_uint64 drwav_read_pcm_frames_s16(drwav* pWav, drwav_uint64 framesToRead, drwav_int16* pBufferOut)
{
    return drwav_read_s16(pWav, framesToRead * pWav->channels, pBufferOut) / pWav->channels;
}

void drwav_u8_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    int r;
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        int x = pIn[i];
        r = x - 128;
        r = r << 8;
        pOut[i] = (short)r;
    }
}

void drwav_s24_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    int r;
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        int x = ((int)(((unsigned int)(((const unsigned char*)pIn)[i*3+0]) << 8) | ((unsigned int)(((const unsigned char*)pIn)[i*3+1]) << 16) | ((unsigned int)(((const unsigned char*)pIn)[i*3+2])) << 24)) >> 8;
        r = x >> 8;
        pOut[i] = (short)r;
    }
}

void drwav_s32_to_s16(drwav_int16* pOut, const drwav_int32* pIn, size_t sampleCount)
{
    int r;
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        int x = pIn[i];
        r = x >> 16;
        pOut[i] = (short)r;
    }
}

void drwav_f32_to_s16(drwav_int16* pOut, const float* pIn, size_t sampleCount)
{
    int r;
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        float x = pIn[i];
        float c;
        c = ((x < -1) ? -1 : ((x > 1) ? 1 : x));
        c = c + 1;
        r = (int)(c * 32767.5f);
        r = r - 32768;
        pOut[i] = (short)r;
    }
}

void drwav_f64_to_s16(drwav_int16* pOut, const double* pIn, size_t sampleCount)
{
    int r;
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        double x = pIn[i];
        double c;
        c = ((x < -1) ? -1 : ((x > 1) ? 1 : x));
        c = c + 1;
        r = (int)(c * 32767.5);
        r = r - 32768;
        pOut[i] = (short)r;
    }
}

void drwav_alaw_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        pOut[i] = drwav__alaw_to_s16(pIn[i]);
    }
}

void drwav_mulaw_to_s16(drwav_int16* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;
    for (i = 0; i < sampleCount; ++i) {
        pOut[i] = drwav__mulaw_to_s16(pIn[i]);
    }
}



static void drwav__pcm_to_f32(float* pOut, const unsigned char* pIn, size_t sampleCount, unsigned int bytesPerSample)
{
    unsigned int i;

    /* Special case for 8-bit sample data because it's treated as unsigned. */
    if (bytesPerSample == 1) {
        drwav_u8_to_f32(pOut, pIn, sampleCount);
        return;
    }

    /* Slightly more optimal implementation for common formats. */
    if (bytesPerSample == 2) {
        drwav_s16_to_f32(pOut, (const drwav_int16*)pIn, sampleCount);
        return;
    }
    if (bytesPerSample == 3) {
        drwav_s24_to_f32(pOut, pIn, sampleCount);
        return;
    }
    if (bytesPerSample == 4) {
        drwav_s32_to_f32(pOut, (const drwav_int32*)pIn, sampleCount);
        return;
    }


    /* Anything more than 64 bits per sample is not supported. */
    if (bytesPerSample > 8) {
        drwav_zero_memory(pOut, sampleCount * sizeof(*pOut));
        return;
    }


    /* Generic, slow converter. */
    for (i = 0; i < sampleCount; ++i) {
        drwav_uint64 sample = 0;
        unsigned int shift  = (8 - bytesPerSample) * 8;

        unsigned int j;
        for (j = 0; j < bytesPerSample && j < 8; j += 1) {
            sample |= (drwav_uint64)(pIn[j]) << shift;
            shift  += 8;
        }

        pIn += j;
        *pOut++ = (float)((drwav_int64)sample / 9223372036854775807.0);
    }
}

static void drwav__ieee_to_f32(float* pOut, const unsigned char* pIn, size_t sampleCount, unsigned int bytesPerSample)
{
    if (bytesPerSample == 4) {
        unsigned int i;
        for (i = 0; i < sampleCount; ++i) {
            *pOut++ = ((const float*)pIn)[i];
        }
        return;
    } else if (bytesPerSample == 8) {
        drwav_f64_to_f32(pOut, (const double*)pIn, sampleCount);
        return;
    } else {
        /* Only supporting 32- and 64-bit float. Output silence in all other cases. Contributions welcome for 16-bit float. */
        drwav_zero_memory(pOut, sampleCount * sizeof(*pOut));
        return;
    }
}


drwav_uint64 drwav_read_f32__pcm(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav__pcm_to_f32(pBufferOut, sampleData, (size_t)samplesRead, bytesPerSample);
        pBufferOut += samplesRead;

        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_f32__msadpcm(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    /*
    We're just going to borrow the implementation from the drwav_read_s16() since ADPCM is a little bit more complicated than other formats and I don't
    want to duplicate that code.
    */
    drwav_uint64 totalSamplesRead = 0;
    drwav_int16 samples16[2048];
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read_s16(pWav, drwav_min(samplesToRead, 2048), samples16);
        if (samplesRead == 0) {
            break;
        }

        drwav_s16_to_f32(pBufferOut, samples16, (size_t)samplesRead);   /* <-- Safe cast because we're clamping to 2048. */

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_f32__ima(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    /*
    We're just going to borrow the implementation from the drwav_read_s16() since IMA-ADPCM is a little bit more complicated than other formats and I don't
    want to duplicate that code.
    */
    drwav_uint64 totalSamplesRead = 0;
    drwav_int16 samples16[2048];
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read_s16(pWav, drwav_min(samplesToRead, 2048), samples16);
        if (samplesRead == 0) {
            break;
        }

        drwav_s16_to_f32(pBufferOut, samples16, (size_t)samplesRead);   /* <-- Safe cast because we're clamping to 2048. */

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_f32__ieee(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];
    drwav_uint32 bytesPerSample;

    /* Fast path. */
    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT && pWav->bitsPerSample == 32) {
        return drwav_read(pWav, samplesToRead, pBufferOut);
    }
    
    bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav__ieee_to_f32(pBufferOut, sampleData, (size_t)samplesRead, bytesPerSample);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_f32__alaw(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];
    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav_alaw_to_f32(pBufferOut, sampleData, (size_t)samplesRead);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_f32__mulaw(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav_mulaw_to_f32(pBufferOut, sampleData, (size_t)samplesRead);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_f32(drwav* pWav, drwav_uint64 samplesToRead, float* pBufferOut)
{
    if (pWav == NULL || samplesToRead == 0 || pBufferOut == NULL) {
        return 0;
    }

    /* Don't try to read more samples than can potentially fit in the output buffer. */
    if (samplesToRead * sizeof(float) > DRWAV_SIZE_MAX) {
        samplesToRead = DRWAV_SIZE_MAX / sizeof(float);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_PCM) {
        return drwav_read_f32__pcm(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
        return drwav_read_f32__msadpcm(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT) {
        return drwav_read_f32__ieee(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ALAW) {
        return drwav_read_f32__alaw(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_MULAW) {
        return drwav_read_f32__mulaw(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
        return drwav_read_f32__ima(pWav, samplesToRead, pBufferOut);
    }

    return 0;
}

drwav_uint64 drwav_read_pcm_frames_f32(drwav* pWav, drwav_uint64 framesToRead, float* pBufferOut)
{
    return drwav_read_f32(pWav, framesToRead * pWav->channels, pBufferOut) / pWav->channels;
}

void drwav_u8_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

#ifdef DR_WAV_LIBSNDFILE_COMPAT
    /*
    It appears libsndfile uses slightly different logic for the u8 -> f32 conversion to dr_wav, which in my opinion is incorrect. It appears
    libsndfile performs the conversion something like "f32 = (u8 / 256) * 2 - 1", however I think it should be "f32 = (u8 / 255) * 2 - 1" (note
    the divisor of 256 vs 255). I use libsndfile as a benchmark for testing, so I'm therefore leaving this block here just for my automated
    correctness testing. This is disabled by default.
    */
    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = (pIn[i] / 256.0f) * 2 - 1;
    }
#else
    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = (pIn[i] / 255.0f) * 2 - 1;
    }
#endif
}

void drwav_s16_to_f32(float* pOut, const drwav_int16* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = pIn[i] / 32768.0f;
    }
}

void drwav_s24_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        unsigned int s0 = pIn[i*3 + 0];
        unsigned int s1 = pIn[i*3 + 1];
        unsigned int s2 = pIn[i*3 + 2];

        int sample32 = (int)((s0 << 8) | (s1 << 16) | (s2 << 24));
        *pOut++ = (float)(sample32 / 2147483648.0);
    }
}

void drwav_s32_to_f32(float* pOut, const drwav_int32* pIn, size_t sampleCount)
{
    size_t i;
    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = (float)(pIn[i] / 2147483648.0);
    }
}

void drwav_f64_to_f32(float* pOut, const double* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = (float)pIn[i];
    }
}

void drwav_alaw_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = drwav__alaw_to_s16(pIn[i]) / 32768.0f;
    }
}

void drwav_mulaw_to_f32(float* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = drwav__mulaw_to_s16(pIn[i]) / 32768.0f;
    }
}



static void drwav__pcm_to_s32(drwav_int32* pOut, const unsigned char* pIn, size_t totalSampleCount, unsigned int bytesPerSample)
{
    unsigned int i;

    /* Special case for 8-bit sample data because it's treated as unsigned. */
    if (bytesPerSample == 1) {
        drwav_u8_to_s32(pOut, pIn, totalSampleCount);
        return;
    }

    /* Slightly more optimal implementation for common formats. */
    if (bytesPerSample == 2) {
        drwav_s16_to_s32(pOut, (const drwav_int16*)pIn, totalSampleCount);
        return;
    }
    if (bytesPerSample == 3) {
        drwav_s24_to_s32(pOut, pIn, totalSampleCount);
        return;
    }
    if (bytesPerSample == 4) {
        for (i = 0; i < totalSampleCount; ++i) {
           *pOut++ = ((const drwav_int32*)pIn)[i];
        }
        return;
    }


    /* Anything more than 64 bits per sample is not supported. */
    if (bytesPerSample > 8) {
        drwav_zero_memory(pOut, totalSampleCount * sizeof(*pOut));
        return;
    }


    /* Generic, slow converter. */
    for (i = 0; i < totalSampleCount; ++i) {
        drwav_uint64 sample = 0;
        unsigned int shift  = (8 - bytesPerSample) * 8;

        unsigned int j;
        for (j = 0; j < bytesPerSample && j < 8; j += 1) {
            sample |= (drwav_uint64)(pIn[j]) << shift;
            shift  += 8;
        }

        pIn += j;
        *pOut++ = (drwav_int32)((drwav_int64)sample >> 32);
    }
}

static void drwav__ieee_to_s32(drwav_int32* pOut, const unsigned char* pIn, size_t totalSampleCount, unsigned int bytesPerSample)
{
    if (bytesPerSample == 4) {
        drwav_f32_to_s32(pOut, (const float*)pIn, totalSampleCount);
        return;
    } else if (bytesPerSample == 8) {
        drwav_f64_to_s32(pOut, (const double*)pIn, totalSampleCount);
        return;
    } else {
        /* Only supporting 32- and 64-bit float. Output silence in all other cases. Contributions welcome for 16-bit float. */
        drwav_zero_memory(pOut, totalSampleCount * sizeof(*pOut));
        return;
    }
}


drwav_uint64 drwav_read_s32__pcm(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];
    drwav_uint32 bytesPerSample;

    /* Fast path. */
    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_PCM && pWav->bitsPerSample == 32) {
        return drwav_read(pWav, samplesToRead, pBufferOut);
    }
    
    bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav__pcm_to_s32(pBufferOut, sampleData, (size_t)samplesRead, bytesPerSample);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s32__msadpcm(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    /*
    We're just going to borrow the implementation from the drwav_read_s16() since ADPCM is a little bit more complicated than other formats and I don't
    want to duplicate that code.
    */
    drwav_uint64 totalSamplesRead = 0;
    drwav_int16 samples16[2048];
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read_s16(pWav, drwav_min(samplesToRead, 2048), samples16);
        if (samplesRead == 0) {
            break;
        }

        drwav_s16_to_s32(pBufferOut, samples16, (size_t)samplesRead);   /* <-- Safe cast because we're clamping to 2048. */

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s32__ima(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    /*
    We're just going to borrow the implementation from the drwav_read_s16() since IMA-ADPCM is a little bit more complicated than other formats and I don't
    want to duplicate that code.
    */
    drwav_uint64 totalSamplesRead = 0;
    drwav_int16 samples16[2048];
    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read_s16(pWav, drwav_min(samplesToRead, 2048), samples16);
        if (samplesRead == 0) {
            break;
        }

        drwav_s16_to_s32(pBufferOut, samples16, (size_t)samplesRead);   /* <-- Safe cast because we're clamping to 2048. */

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s32__ieee(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav__ieee_to_s32(pBufferOut, sampleData, (size_t)samplesRead, bytesPerSample);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s32__alaw(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav_alaw_to_s32(pBufferOut, sampleData, (size_t)samplesRead);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s32__mulaw(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    drwav_uint64 totalSamplesRead;
    unsigned char sampleData[4096];

    drwav_uint32 bytesPerSample = drwav_get_bytes_per_sample(pWav);
    if (bytesPerSample == 0) {
        return 0;
    }

    totalSamplesRead = 0;

    while (samplesToRead > 0) {
        drwav_uint64 samplesRead = drwav_read(pWav, drwav_min(samplesToRead, sizeof(sampleData)/bytesPerSample), sampleData);
        if (samplesRead == 0) {
            break;
        }

        drwav_mulaw_to_s32(pBufferOut, sampleData, (size_t)samplesRead);

        pBufferOut       += samplesRead;
        samplesToRead    -= samplesRead;
        totalSamplesRead += samplesRead;
    }

    return totalSamplesRead;
}

drwav_uint64 drwav_read_s32(drwav* pWav, drwav_uint64 samplesToRead, drwav_int32* pBufferOut)
{
    if (pWav == NULL || samplesToRead == 0 || pBufferOut == NULL) {
        return 0;
    }

    /* Don't try to read more samples than can potentially fit in the output buffer. */
    if (samplesToRead * sizeof(drwav_int32) > DRWAV_SIZE_MAX) {
        samplesToRead = DRWAV_SIZE_MAX / sizeof(drwav_int32);
    }


    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_PCM) {
        return drwav_read_s32__pcm(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
        return drwav_read_s32__msadpcm(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_IEEE_FLOAT) {
        return drwav_read_s32__ieee(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_ALAW) {
        return drwav_read_s32__alaw(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_MULAW) {
        return drwav_read_s32__mulaw(pWav, samplesToRead, pBufferOut);
    }

    if (pWav->translatedFormatTag == DR_WAVE_FORMAT_DVI_ADPCM) {
        return drwav_read_s32__ima(pWav, samplesToRead, pBufferOut);
    }

    return 0;
}

drwav_uint64 drwav_read_pcm_frames_s32(drwav* pWav, drwav_uint64 framesToRead, drwav_int32* pBufferOut)
{
    return drwav_read_s32(pWav, framesToRead * pWav->channels, pBufferOut) / pWav->channels;
}

void drwav_u8_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = ((int)pIn[i] - 128) << 24;
    }
}

void drwav_s16_to_s32(drwav_int32* pOut, const drwav_int16* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = pIn[i] << 16;
    }
}

void drwav_s24_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        unsigned int s0 = pIn[i*3 + 0];
        unsigned int s1 = pIn[i*3 + 1];
        unsigned int s2 = pIn[i*3 + 2];

        drwav_int32 sample32 = (drwav_int32)((s0 << 8) | (s1 << 16) | (s2 << 24));
        *pOut++ = sample32;
    }
}

void drwav_f32_to_s32(drwav_int32* pOut, const float* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = (drwav_int32)(2147483648.0 * pIn[i]);
    }
}

void drwav_f64_to_s32(drwav_int32* pOut, const double* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = (drwav_int32)(2147483648.0 * pIn[i]);
    }
}

void drwav_alaw_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i = 0; i < sampleCount; ++i) {
        *pOut++ = ((drwav_int32)drwav__alaw_to_s16(pIn[i])) << 16;
    }
}

void drwav_mulaw_to_s32(drwav_int32* pOut, const drwav_uint8* pIn, size_t sampleCount)
{
    size_t i;

    if (pOut == NULL || pIn == NULL) {
        return;
    }

    for (i= 0; i < sampleCount; ++i) {
        *pOut++ = ((drwav_int32)drwav__mulaw_to_s16(pIn[i])) << 16;
    }
}



drwav_int16* drwav__read_and_close_s16(drwav* pWav, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav_uint64 sampleDataSize;
    drwav_int16* pSampleData;
    drwav_uint64 samplesRead;

    drwav_assert(pWav != NULL);

    sampleDataSize = pWav->totalSampleCount * sizeof(drwav_int16);
    if (sampleDataSize > DRWAV_SIZE_MAX) {
        drwav_uninit(pWav);
        return NULL;    /* File's too big. */
    }

    pSampleData = (drwav_int16*)DRWAV_MALLOC((size_t)sampleDataSize);    /* <-- Safe cast due to the check above. */
    if (pSampleData == NULL) {
        drwav_uninit(pWav);
        return NULL;    /* Failed to allocate memory. */
    }

    samplesRead = drwav_read_s16(pWav, (size_t)pWav->totalSampleCount, pSampleData);
    if (samplesRead != pWav->totalSampleCount) {
        DRWAV_FREE(pSampleData);
        drwav_uninit(pWav);
        return NULL;    /* There was an error reading the samples. */
    }

    drwav_uninit(pWav);

    if (sampleRate) {
        *sampleRate = pWav->sampleRate;
    }
    if (channels) {
        *channels = pWav->channels;
    }
    if (totalSampleCount) {
        *totalSampleCount = pWav->totalSampleCount;
    }

    return pSampleData;
}

float* drwav__read_and_close_f32(drwav* pWav, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav_uint64 sampleDataSize;
    float* pSampleData;
    drwav_uint64 samplesRead;

    drwav_assert(pWav != NULL);

    sampleDataSize = pWav->totalSampleCount * sizeof(float);
    if (sampleDataSize > DRWAV_SIZE_MAX) {
        drwav_uninit(pWav);
        return NULL;    /* File's too big. */
    }

    pSampleData = (float*)DRWAV_MALLOC((size_t)sampleDataSize);    /* <-- Safe cast due to the check above. */
    if (pSampleData == NULL) {
        drwav_uninit(pWav);
        return NULL;    /* Failed to allocate memory. */
    }

    samplesRead = drwav_read_f32(pWav, (size_t)pWav->totalSampleCount, pSampleData);
    if (samplesRead != pWav->totalSampleCount) {
        DRWAV_FREE(pSampleData);
        drwav_uninit(pWav);
        return NULL;    /* There was an error reading the samples. */
    }

    drwav_uninit(pWav);

    if (sampleRate) {
        *sampleRate = pWav->sampleRate;
    }
    if (channels) {
        *channels = pWav->channels;
    }
    if (totalSampleCount) {
        *totalSampleCount = pWav->totalSampleCount;
    }

    return pSampleData;
}

drwav_int32* drwav__read_and_close_s32(drwav* pWav, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav_uint64 sampleDataSize;
    drwav_int32* pSampleData;
    drwav_uint64 samplesRead;

    drwav_assert(pWav != NULL);

    sampleDataSize = pWav->totalSampleCount * sizeof(drwav_int32);
    if (sampleDataSize > DRWAV_SIZE_MAX) {
        drwav_uninit(pWav);
        return NULL;    /* File's too big. */
    }

    pSampleData = (drwav_int32*)DRWAV_MALLOC((size_t)sampleDataSize);    /* <-- Safe cast due to the check above. */
    if (pSampleData == NULL) {
        drwav_uninit(pWav);
        return NULL;    /* Failed to allocate memory. */
    }

    samplesRead = drwav_read_s32(pWav, (size_t)pWav->totalSampleCount, pSampleData);
    if (samplesRead != pWav->totalSampleCount) {
        DRWAV_FREE(pSampleData);
        drwav_uninit(pWav);
        return NULL;    /* There was an error reading the samples. */
    }

    drwav_uninit(pWav);

    if (sampleRate) {
        *sampleRate = pWav->sampleRate;
    }
    if (channels) {
        *channels = pWav->channels;
    }
    if (totalSampleCount) {
        *totalSampleCount = pWav->totalSampleCount;
    }

    return pSampleData;
}


drwav_int16* drwav_open_and_read_s16(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (channels) {
        *channels = 0;
    }
    if (sampleRate) {
        *sampleRate = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init(&wav, onRead, onSeek, pUserData)) {
        return NULL;
    }

    return drwav__read_and_close_s16(&wav, channels, sampleRate, totalSampleCount);
}

drwav_int16* drwav_open_and_read_pcm_frames_s16(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    drwav_int16* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_and_read_s16(onRead, onSeek, pUserData, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

float* drwav_open_and_read_f32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init(&wav, onRead, onSeek, pUserData)) {
        return NULL;
    }

    return drwav__read_and_close_f32(&wav, channels, sampleRate, totalSampleCount);
}

float* drwav_open_and_read_pcm_frames_f32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    float* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_and_read_f32(onRead, onSeek, pUserData, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

drwav_int32* drwav_open_and_read_s32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init(&wav, onRead, onSeek, pUserData)) {
        return NULL;
    }

    return drwav__read_and_close_s32(&wav, channels, sampleRate, totalSampleCount);
}

drwav_int32* drwav_open_and_read_pcm_frames_s32(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    drwav_int32* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_and_read_s32(onRead, onSeek, pUserData, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

#ifndef DR_WAV_NO_STDIO
drwav_int16* drwav_open_file_and_read_s16(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init_file(&wav, filename)) {
        return NULL;
    }

    return drwav__read_and_close_s16(&wav, channels, sampleRate, totalSampleCount);
}

drwav_int16* drwav_open_file_and_read_pcm_frames_s16(const char* filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    drwav_int16* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_file_and_read_s16(filename, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

float* drwav_open_file_and_read_f32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init_file(&wav, filename)) {
        return NULL;
    }

    return drwav__read_and_close_f32(&wav, channels, sampleRate, totalSampleCount);
}

float* drwav_open_file_and_read_pcm_frames_f32(const char* filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    float* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_file_and_read_f32(filename, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

drwav_int32* drwav_open_file_and_read_s32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init_file(&wav, filename)) {
        return NULL;
    }

    return drwav__read_and_close_s32(&wav, channels, sampleRate, totalSampleCount);
}

drwav_int32* drwav_open_file_and_read_pcm_frames_s32(const char* filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    drwav_int32* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_file_and_read_s32(filename, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}
#endif

drwav_int16* drwav_open_memory_and_read_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init_memory(&wav, data, dataSize)) {
        return NULL;
    }

    return drwav__read_and_close_s16(&wav, channels, sampleRate, totalSampleCount);
}

drwav_int16* drwav_open_memory_and_read_pcm_frames_s16(const void* data, size_t dataSize, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    drwav_int16* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_memory_and_read_s16(data, dataSize, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

float* drwav_open_memory_and_read_f32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init_memory(&wav, data, dataSize)) {
        return NULL;
    }

    return drwav__read_and_close_f32(&wav, channels, sampleRate, totalSampleCount);
}

float* drwav_open_memory_and_read_pcm_frames_f32(const void* data, size_t dataSize, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    float* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_memory_and_read_f32(data, dataSize, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}

drwav_int32* drwav_open_memory_and_read_s32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drwav_uint64* totalSampleCount)
{
    drwav wav;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalSampleCount) {
        *totalSampleCount = 0;
    }

    if (!drwav_init_memory(&wav, data, dataSize)) {
        return NULL;
    }

    return drwav__read_and_close_s32(&wav, channels, sampleRate, totalSampleCount);
}

drwav_int32* drwav_open_memory_and_read_pcm_frames_s32(const void* data, size_t dataSize, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalSampleCount;
    drwav_int32* result;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = 0;
    }

    result = drwav_open_memory_and_read_s32(data, dataSize, &channels, &sampleRate, &totalSampleCount);
    if (result == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalFrameCountOut) {
        *totalFrameCountOut = totalSampleCount / channels;
    }

    return result;
}
#endif  /* DR_WAV_NO_CONVERSION_API */


void drwav_free(void* pDataReturnedByOpenAndRead)
{
    DRWAV_FREE(pDataReturnedByOpenAndRead);
}

#endif  /* DR_WAV_IMPLEMENTATION */


/*
REVISION HISTORY
================
v0.9.1 - 2019-05-05
  - Add support for C89.
  - Change license to choice of public domain or MIT-0.

v0.9.0 - 2018-12-16
  - API CHANGE: Add new reading APIs for reading by PCM frames instead of samples. Old APIs have been deprecated and
    will be removed in v0.10.0. Deprecated APIs and their replacements:
      drwav_read()                     -> drwav_read_pcm_frames()
      drwav_read_s16()                 -> drwav_read_pcm_frames_s16()
      drwav_read_f32()                 -> drwav_read_pcm_frames_f32()
      drwav_read_s32()                 -> drwav_read_pcm_frames_s32()
      drwav_seek_to_sample()           -> drwav_seek_to_pcm_frame()
      drwav_write()                    -> drwav_write_pcm_frames()
      drwav_open_and_read_s16()        -> drwav_open_and_read_pcm_frames_s16()
      drwav_open_and_read_f32()        -> drwav_open_and_read_pcm_frames_f32()
      drwav_open_and_read_s32()        -> drwav_open_and_read_pcm_frames_s32()
      drwav_open_file_and_read_s16()   -> drwav_open_file_and_read_pcm_frames_s16()
      drwav_open_file_and_read_f32()   -> drwav_open_file_and_read_pcm_frames_f32()
      drwav_open_file_and_read_s32()   -> drwav_open_file_and_read_pcm_frames_s32()
      drwav_open_memory_and_read_s16() -> drwav_open_memory_and_read_pcm_frames_s16()
      drwav_open_memory_and_read_f32() -> drwav_open_memory_and_read_pcm_frames_f32()
      drwav_open_memory_and_read_s32() -> drwav_open_memory_and_read_pcm_frames_s32()
      drwav::totalSampleCount          -> drwav::totalPCMFrameCount
  - API CHANGE: Rename drwav_open_and_read_file_*() to drwav_open_file_and_read_*().
  - API CHANGE: Rename drwav_open_and_read_memory_*() to drwav_open_memory_and_read_*().
  - Add built-in support for smpl chunks.
  - Add support for firing a callback for each chunk in the file at initialization time.
    - This is enabled through the drwav_init_ex(), etc. family of APIs.
  - Handle invalid FMT chunks more robustly.

v0.8.5 - 2018-09-11
  - Const correctness.
  - Fix a potential stack overflow.

v0.8.4 - 2018-08-07
  - Improve 64-bit detection.

v0.8.3 - 2018-08-05
  - Fix C++ build on older versions of GCC.

v0.8.2 - 2018-08-02
  - Fix some big-endian bugs.

v0.8.1 - 2018-06-29
  - Add support for sequential writing APIs.
  - Disable seeking in write mode.
  - Fix bugs with Wave64.
  - Fix typos.

v0.8 - 2018-04-27
  - Bug fix.
  - Start using major.minor.revision versioning.

v0.7f - 2018-02-05
  - Restrict ADPCM formats to a maximum of 2 channels.

v0.7e - 2018-02-02
  - Fix a crash.

v0.7d - 2018-02-01
  - Fix a crash.

v0.7c - 2018-02-01
  - Set drwav.bytesPerSample to 0 for all compressed formats.
  - Fix a crash when reading 16-bit floating point WAV files. In this case dr_wav will output silence for
    all format conversion reading APIs (*_s16, *_s32, *_f32 APIs).
  - Fix some divide-by-zero errors.

v0.7b - 2018-01-22
  - Fix errors with seeking of compressed formats.
  - Fix compilation error when DR_WAV_NO_CONVERSION_API

v0.7a - 2017-11-17
  - Fix some GCC warnings.

v0.7 - 2017-11-04
  - Add writing APIs.

v0.6 - 2017-08-16
  - API CHANGE: Rename dr_* types to drwav_*.
  - Add support for custom implementations of malloc(), realloc(), etc.
  - Add support for Microsoft ADPCM.
  - Add support for IMA ADPCM (DVI, format code 0x11).
  - Optimizations to drwav_read_s16().
  - Bug fixes.

v0.5g - 2017-07-16
  - Change underlying type for booleans to unsigned.

v0.5f - 2017-04-04
  - Fix a minor bug with drwav_open_and_read_s16() and family.

v0.5e - 2016-12-29
  - Added support for reading samples as signed 16-bit integers. Use the _s16() family of APIs for this.
  - Minor fixes to documentation.

v0.5d - 2016-12-28
  - Use drwav_int* and drwav_uint* sized types to improve compiler support.

v0.5c - 2016-11-11
  - Properly handle JUNK chunks that come before the FMT chunk.

v0.5b - 2016-10-23
  - A minor change to drwav_bool8 and drwav_bool32 types.

v0.5a - 2016-10-11
  - Fixed a bug with drwav_open_and_read() and family due to incorrect argument ordering.
  - Improve A-law and mu-law efficiency.

v0.5 - 2016-09-29
  - API CHANGE. Swap the order of "channels" and "sampleRate" parameters in drwav_open_and_read*(). Rationale for this is to
    keep it consistent with dr_audio and dr_flac.

v0.4b - 2016-09-18
  - Fixed a typo in documentation.

v0.4a - 2016-09-18
  - Fixed a typo.
  - Change date format to ISO 8601 (YYYY-MM-DD)

v0.4 - 2016-07-13
  - API CHANGE. Make onSeek consistent with dr_flac.
  - API CHANGE. Rename drwav_seek() to drwav_seek_to_sample() for clarity and consistency with dr_flac.
  - Added support for Sony Wave64.

v0.3a - 2016-05-28
  - API CHANGE. Return drwav_bool32 instead of int in onSeek callback.
  - Fixed a memory leak.

v0.3 - 2016-05-22
  - Lots of API changes for consistency.

v0.2a - 2016-05-16
  - Fixed Linux/GCC build.

v0.2 - 2016-05-11
  - Added support for reading data as signed 32-bit PCM for consistency with dr_flac.

v0.1a - 2016-05-07
  - Fixed a bug in drwav_open_file() where the file handle would not be closed if the loader failed to initialize.

v0.1 - 2016-05-04
  - Initial versioned release.
*/

/*
This software is available as a choice of the following licenses. Choose
whichever you prefer.

===============================================================================
ALTERNATIVE 1 - Public Domain (www.unlicense.org)
===============================================================================
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

===============================================================================
ALTERNATIVE 2 - MIT No Attribution
===============================================================================
Copyright 2018 David Reid

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
