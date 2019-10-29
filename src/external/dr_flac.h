/*
FLAC audio decoder. Choice of public domain or MIT-0. See license statements at the end of this file.
dr_flac - v0.11.10 - 2019-06-26

David Reid - mackron@gmail.com
*/

/*
USAGE
=====
dr_flac is a single-file library. To use it, do something like the following in one .c file.
    #define DR_FLAC_IMPLEMENTATION
    #include "dr_flac.h"

You can then #include this file in other parts of the program as you would with any other header file. To decode audio data,
do something like the following:

    drflac* pFlac = drflac_open_file("MySong.flac");
    if (pFlac == NULL) {
        // Failed to open FLAC file
    }

    drflac_int32* pSamples = malloc(pFlac->totalPCMFrameCount * pFlac->channels * sizeof(drflac_int32));
    drflac_uint64 numberOfInterleavedSamplesActuallyRead = drflac_read_pcm_frames_s32(pFlac, pFlac->totalPCMFrameCount, pSamples);

The drflac object represents the decoder. It is a transparent type so all the information you need, such as the number of
channels and the bits per sample, should be directly accessible - just make sure you don't change their values. Samples are
always output as interleaved signed 32-bit PCM. In the example above a native FLAC stream was opened, however dr_flac has
seamless support for Ogg encapsulated FLAC streams as well.

You do not need to decode the entire stream in one go - you just specify how many samples you'd like at any given time and
the decoder will give you as many samples as it can, up to the amount requested. Later on when you need the next batch of
samples, just call it again. Example:

    while (drflac_read_pcm_frames_s32(pFlac, chunkSizeInPCMFrames, pChunkSamples) > 0) {
        do_something();
    }

You can seek to a specific sample with drflac_seek_to_sample(). The given sample is based on interleaving. So for example,
if you were to seek to the sample at index 0 in a stereo stream, you'll be seeking to the first sample of the left channel.
The sample at index 1 will be the first sample of the right channel. The sample at index 2 will be the second sample of the
left channel, etc.


If you just want to quickly decode an entire FLAC file in one go you can do something like this:

    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int32* pSampleData = drflac_open_file_and_read_pcm_frames_s32("MySong.flac", &channels, &sampleRate, &totalPCMFrameCount);
    if (pSampleData == NULL) {
        // Failed to open and decode FLAC file.
    }

    ...

    drflac_free(pSampleData);


You can read samples as signed 16-bit integer and 32-bit floating-point PCM with the *_s16() and *_f32() family of APIs
respectively, but note that these should be considered lossy.


If you need access to metadata (album art, etc.), use drflac_open_with_metadata(), drflac_open_file_with_metdata() or
drflac_open_memory_with_metadata(). The rationale for keeping these APIs separate is that they're slightly slower than the
normal versions and also just a little bit harder to use.

dr_flac reports metadata to the application through the use of a callback, and every metadata block is reported before
drflac_open_with_metdata() returns.


The main opening APIs (drflac_open(), etc.) will fail if the header is not present. The presents a problem in certain
scenarios such as broadcast style streams like internet radio where the header may not be present because the user has
started playback mid-stream. To handle this, use the relaxed APIs: drflac_open_relaxed() and drflac_open_with_metadata_relaxed().

It is not recommended to use these APIs for file based streams because a missing header would usually indicate a
corrupted or perverse file. In addition, these APIs can take a long time to initialize because they may need to spend
a lot of time finding the first frame.



OPTIONS
=======
#define these options before including this file.

#define DR_FLAC_NO_STDIO
  Disable drflac_open_file() and family.

#define DR_FLAC_NO_OGG
  Disables support for Ogg/FLAC streams.

#define DR_FLAC_BUFFER_SIZE <number>
  Defines the size of the internal buffer to store data from onRead(). This buffer is used to reduce the number of calls
  back to the client for more data. Larger values means more memory, but better performance. My tests show diminishing
  returns after about 4KB (which is the default). Consider reducing this if you have a very efficient implementation of
  onRead(), or increase it if it's very inefficient. Must be a multiple of 8.

#define DR_FLAC_NO_CRC
  Disables CRC checks. This will offer a performance boost when CRC is unnecessary.

#define DR_FLAC_NO_SIMD
  Disables SIMD optimizations (SSE on x86/x64 architectures). Use this if you are having compatibility issues with your
  compiler.



QUICK NOTES
===========
- dr_flac does not currently support changing the sample rate nor channel count mid stream.
- Audio data is output as signed 32-bit PCM, regardless of the bits per sample the FLAC stream is encoded as.
- This has not been tested on big-endian architectures.
- dr_flac is not thread-safe, but its APIs can be called from any thread so long as you do your own synchronization.
- When using Ogg encapsulation, a corrupted metadata block will result in drflac_open_with_metadata() and drflac_open()
  returning inconsistent samples.
*/

#ifndef dr_flac_h
#define dr_flac_h

#include <stddef.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    drflac_int8;
typedef unsigned char    drflac_uint8;
typedef   signed short   drflac_int16;
typedef unsigned short   drflac_uint16;
typedef   signed int     drflac_int32;
typedef unsigned int     drflac_uint32;
typedef   signed __int64 drflac_int64;
typedef unsigned __int64 drflac_uint64;
#else
#include <stdint.h>
typedef int8_t           drflac_int8;
typedef uint8_t          drflac_uint8;
typedef int16_t          drflac_int16;
typedef uint16_t         drflac_uint16;
typedef int32_t          drflac_int32;
typedef uint32_t         drflac_uint32;
typedef int64_t          drflac_int64;
typedef uint64_t         drflac_uint64;
#endif
typedef drflac_uint8     drflac_bool8;
typedef drflac_uint32    drflac_bool32;
#define DRFLAC_TRUE      1
#define DRFLAC_FALSE     0

#if defined(_MSC_VER) && _MSC_VER >= 1700   /* Visual Studio 2012 */
    #define DRFLAC_DEPRECATED       __declspec(deprecated)
#elif (defined(__GNUC__) && __GNUC__ >= 4)  /* GCC 4 */
    #define DRFLAC_DEPRECATED       __attribute__((deprecated))
#elif defined(__has_feature)                /* Clang */
    #if __has_feature(attribute_deprecated)
        #define DRFLAC_DEPRECATED   __attribute__((deprecated))
    #else
        #define DRFLAC_DEPRECATED
    #endif
#else
    #define DRFLAC_DEPRECATED
#endif

/*
As data is read from the client it is placed into an internal buffer for fast access. This controls the
size of that buffer. Larger values means more speed, but also more memory. In my testing there is diminishing
returns after about 4KB, but you can fiddle with this to suit your own needs. Must be a multiple of 8.
*/
#ifndef DR_FLAC_BUFFER_SIZE
#define DR_FLAC_BUFFER_SIZE   4096
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Check if we can enable 64-bit optimizations. */
#if defined(_WIN64) || defined(_LP64) || defined(__LP64__)
#define DRFLAC_64BIT
#endif

#ifdef DRFLAC_64BIT
typedef drflac_uint64 drflac_cache_t;
#else
typedef drflac_uint32 drflac_cache_t;
#endif

/* The various metadata block types. */
#define DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO       0
#define DRFLAC_METADATA_BLOCK_TYPE_PADDING          1
#define DRFLAC_METADATA_BLOCK_TYPE_APPLICATION      2
#define DRFLAC_METADATA_BLOCK_TYPE_SEEKTABLE        3
#define DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT   4
#define DRFLAC_METADATA_BLOCK_TYPE_CUESHEET         5
#define DRFLAC_METADATA_BLOCK_TYPE_PICTURE          6
#define DRFLAC_METADATA_BLOCK_TYPE_INVALID          127

/* The various picture types specified in the PICTURE block. */
#define DRFLAC_PICTURE_TYPE_OTHER                   0
#define DRFLAC_PICTURE_TYPE_FILE_ICON               1
#define DRFLAC_PICTURE_TYPE_OTHER_FILE_ICON         2
#define DRFLAC_PICTURE_TYPE_COVER_FRONT             3
#define DRFLAC_PICTURE_TYPE_COVER_BACK              4
#define DRFLAC_PICTURE_TYPE_LEAFLET_PAGE            5
#define DRFLAC_PICTURE_TYPE_MEDIA                   6
#define DRFLAC_PICTURE_TYPE_LEAD_ARTIST             7
#define DRFLAC_PICTURE_TYPE_ARTIST                  8
#define DRFLAC_PICTURE_TYPE_CONDUCTOR               9
#define DRFLAC_PICTURE_TYPE_BAND                    10
#define DRFLAC_PICTURE_TYPE_COMPOSER                11
#define DRFLAC_PICTURE_TYPE_LYRICIST                12
#define DRFLAC_PICTURE_TYPE_RECORDING_LOCATION      13
#define DRFLAC_PICTURE_TYPE_DURING_RECORDING        14
#define DRFLAC_PICTURE_TYPE_DURING_PERFORMANCE      15
#define DRFLAC_PICTURE_TYPE_SCREEN_CAPTURE          16
#define DRFLAC_PICTURE_TYPE_BRIGHT_COLORED_FISH     17
#define DRFLAC_PICTURE_TYPE_ILLUSTRATION            18
#define DRFLAC_PICTURE_TYPE_BAND_LOGOTYPE           19
#define DRFLAC_PICTURE_TYPE_PUBLISHER_LOGOTYPE      20

typedef enum
{
    drflac_container_native,
    drflac_container_ogg,
    drflac_container_unknown
} drflac_container;

typedef enum
{
    drflac_seek_origin_start,
    drflac_seek_origin_current
} drflac_seek_origin;

/* Packing is important on this structure because we map this directly to the raw data within the SEEKTABLE metadata block. */
#pragma pack(2)
typedef struct
{
    drflac_uint64 firstSample;
    drflac_uint64 frameOffset;   /* The offset from the first byte of the header of the first frame. */
    drflac_uint16 sampleCount;
} drflac_seekpoint;
#pragma pack()

typedef struct
{
    drflac_uint16 minBlockSize;
    drflac_uint16 maxBlockSize;
    drflac_uint32 minFrameSize;
    drflac_uint32 maxFrameSize;
    drflac_uint32 sampleRate;
    drflac_uint8  channels;
    drflac_uint8  bitsPerSample;
    drflac_uint64 totalSampleCount;
    drflac_uint8  md5[16];
} drflac_streaminfo;

typedef struct
{
    /* The metadata type. Use this to know how to interpret the data below. */
    drflac_uint32 type;

    /*
    A pointer to the raw data. This points to a temporary buffer so don't hold on to it. It's best to
    not modify the contents of this buffer. Use the structures below for more meaningful and structured
    information about the metadata. It's possible for this to be null.
    */
    const void* pRawData;

    /* The size in bytes of the block and the buffer pointed to by pRawData if it's non-NULL. */
    drflac_uint32 rawDataSize;

    union
    {
        drflac_streaminfo streaminfo;

        struct
        {
            int unused;
        } padding;

        struct
        {
            drflac_uint32 id;
            const void* pData;
            drflac_uint32 dataSize;
        } application;

        struct
        {
            drflac_uint32 seekpointCount;
            const drflac_seekpoint* pSeekpoints;
        } seektable;

        struct
        {
            drflac_uint32 vendorLength;
            const char* vendor;
            drflac_uint32 commentCount;
            const void* pComments;
        } vorbis_comment;

        struct
        {
            char catalog[128];
            drflac_uint64 leadInSampleCount;
            drflac_bool32 isCD;
            drflac_uint8 trackCount;
            const void* pTrackData;
        } cuesheet;

        struct
        {
            drflac_uint32 type;
            drflac_uint32 mimeLength;
            const char* mime;
            drflac_uint32 descriptionLength;
            const char* description;
            drflac_uint32 width;
            drflac_uint32 height;
            drflac_uint32 colorDepth;
            drflac_uint32 indexColorCount;
            drflac_uint32 pictureDataSize;
            const drflac_uint8* pPictureData;
        } picture;
    } data;
} drflac_metadata;


/*
Callback for when data needs to be read from the client.

pUserData   [in]  The user data that was passed to drflac_open() and family.
pBufferOut  [out] The output buffer.
bytesToRead [in]  The number of bytes to read.

Returns the number of bytes actually read.

A return value of less than bytesToRead indicates the end of the stream. Do _not_ return from this callback until
either the entire bytesToRead is filled or you have reached the end of the stream.
*/
typedef size_t (* drflac_read_proc)(void* pUserData, void* pBufferOut, size_t bytesToRead);

/*
Callback for when data needs to be seeked.

pUserData [in] The user data that was passed to drflac_open() and family.
offset    [in] The number of bytes to move, relative to the origin. Will never be negative.
origin    [in] The origin of the seek - the current position or the start of the stream.

Returns whether or not the seek was successful.

The offset will never be negative. Whether or not it is relative to the beginning or current position is determined
by the "origin" parameter which will be either drflac_seek_origin_start or drflac_seek_origin_current.
*/
typedef drflac_bool32 (* drflac_seek_proc)(void* pUserData, int offset, drflac_seek_origin origin);

/*
Callback for when a metadata block is read.

pUserData [in] The user data that was passed to drflac_open() and family.
pMetadata [in] A pointer to a structure containing the data of the metadata block.

Use pMetadata->type to determine which metadata block is being handled and how to read the data.
*/
typedef void (* drflac_meta_proc)(void* pUserData, drflac_metadata* pMetadata);


/* Structure for internal use. Only used for decoders opened with drflac_open_memory. */
typedef struct
{
    const drflac_uint8* data;
    size_t dataSize;
    size_t currentReadPos;
} drflac__memory_stream;

/* Structure for internal use. Used for bit streaming. */
typedef struct
{
    /* The function to call when more data needs to be read. */
    drflac_read_proc onRead;

    /* The function to call when the current read position needs to be moved. */
    drflac_seek_proc onSeek;

    /* The user data to pass around to onRead and onSeek. */
    void* pUserData;


    /*
    The number of unaligned bytes in the L2 cache. This will always be 0 until the end of the stream is hit. At the end of the
    stream there will be a number of bytes that don't cleanly fit in an L1 cache line, so we use this variable to know whether
    or not the bistreamer needs to run on a slower path to read those last bytes. This will never be more than sizeof(drflac_cache_t).
    */
    size_t unalignedByteCount;

    /* The content of the unaligned bytes. */
    drflac_cache_t unalignedCache;

    /* The index of the next valid cache line in the "L2" cache. */
    drflac_uint32 nextL2Line;

    /* The number of bits that have been consumed by the cache. This is used to determine how many valid bits are remaining. */
    drflac_uint32 consumedBits;

    /*
    The cached data which was most recently read from the client. There are two levels of cache. Data flows as such:
    Client -> L2 -> L1. The L2 -> L1 movement is aligned and runs on a fast path in just a few instructions.
    */
    drflac_cache_t cacheL2[DR_FLAC_BUFFER_SIZE/sizeof(drflac_cache_t)];
    drflac_cache_t cache;

    /*
    CRC-16. This is updated whenever bits are read from the bit stream. Manually set this to 0 to reset the CRC. For FLAC, this
    is reset to 0 at the beginning of each frame.
    */
    drflac_uint16 crc16;
    drflac_cache_t crc16Cache;              /* A cache for optimizing CRC calculations. This is filled when when the L1 cache is reloaded. */
    drflac_uint32 crc16CacheIgnoredBytes;   /* The number of bytes to ignore when updating the CRC-16 from the CRC-16 cache. */
} drflac_bs;

typedef struct
{
    /* The type of the subframe: SUBFRAME_CONSTANT, SUBFRAME_VERBATIM, SUBFRAME_FIXED or SUBFRAME_LPC. */
    drflac_uint8 subframeType;

    /* The number of wasted bits per sample as specified by the sub-frame header. */
    drflac_uint8 wastedBitsPerSample;

    /* The order to use for the prediction stage for SUBFRAME_FIXED and SUBFRAME_LPC. */
    drflac_uint8 lpcOrder;

    /*
    The number of bits per sample for this subframe. This is not always equal to the current frame's bit per sample because
    an extra bit is required for side channels when interchannel decorrelation is being used.
    */
    drflac_uint32 bitsPerSample;

    /*
    A pointer to the buffer containing the decoded samples in the subframe. This pointer is an offset from drflac::pExtraData. Note that
    it's a signed 32-bit integer for each value.
    */
    drflac_int32* pDecodedSamples;
} drflac_subframe;

typedef struct
{
    /*
    If the stream uses variable block sizes, this will be set to the index of the first sample. If fixed block sizes are used, this will
    always be set to 0.
    */
    drflac_uint64 sampleNumber;

    /* If the stream uses fixed block sizes, this will be set to the frame number. If variable block sizes are used, this will always be 0. */
    drflac_uint32 frameNumber;

    /* The sample rate of this frame. */
    drflac_uint32 sampleRate;

    /* The number of samples in each sub-frame within this frame. */
    drflac_uint16 blockSize;

    /*
    The channel assignment of this frame. This is not always set to the channel count. If interchannel decorrelation is being used this
    will be set to DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE, DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE or DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE.
    */
    drflac_uint8 channelAssignment;

    /* The number of bits per sample within this frame. */
    drflac_uint8 bitsPerSample;

    /* The frame's CRC. */
    drflac_uint8 crc8;
} drflac_frame_header;

typedef struct
{
    /* The header. */
    drflac_frame_header header;

    /*
    The number of samples left to be read in this frame. This is initially set to the block size multiplied by the channel count. As samples
    are read, this will be decremented. When it reaches 0, the decoder will see this frame as fully consumed and load the next frame.
    */
    drflac_uint32 samplesRemaining;

    /* The list of sub-frames within the frame. There is one sub-frame for each channel, and there's a maximum of 8 channels. */
    drflac_subframe subframes[8];
} drflac_frame;

typedef struct
{
    /* The function to call when a metadata block is read. */
    drflac_meta_proc onMeta;

    /* The user data posted to the metadata callback function. */
    void* pUserDataMD;


    /* The sample rate. Will be set to something like 44100. */
    drflac_uint32 sampleRate;

    /*
    The number of channels. This will be set to 1 for monaural streams, 2 for stereo, etc. Maximum 8. This is set based on the
    value specified in the STREAMINFO block.
    */
    drflac_uint8 channels;

    /* The bits per sample. Will be set to something like 16, 24, etc. */
    drflac_uint8 bitsPerSample;

    /* The maximum block size, in samples. This number represents the number of samples in each channel (not combined). */
    drflac_uint16 maxBlockSize;

    /*
    The total number of samples making up the stream. This includes every channel. For example, if the stream has 2 channels,
    with each channel having a total of 4096, this value will be set to 2*4096 = 8192. Can be 0 in which case it's still a
    valid stream, but just means the total sample count is unknown. Likely the case with streams like internet radio.
    */
    drflac_uint64 totalSampleCount;
    drflac_uint64 totalPCMFrameCount;   /* <-- Equal to totalSampleCount / channels. */


    /* The container type. This is set based on whether or not the decoder was opened from a native or Ogg stream. */
    drflac_container container;

    /* The number of seekpoints in the seektable. */
    drflac_uint32 seekpointCount;


    /* Information about the frame the decoder is currently sitting on. */
    drflac_frame currentFrame;

    /* The index of the sample the decoder is currently sitting on. This is only used for seeking. */
    drflac_uint64 currentSample;

    /* The position of the first frame in the stream. This is only ever used for seeking. */
    drflac_uint64 firstFramePos;


    /* A hack to avoid a malloc() when opening a decoder with drflac_open_memory(). */
    drflac__memory_stream memoryStream;


    /* A pointer to the decoded sample data. This is an offset of pExtraData. */
    drflac_int32* pDecodedSamples;

    /* A pointer to the seek table. This is an offset of pExtraData, or NULL if there is no seek table. */
    drflac_seekpoint* pSeekpoints;

    /* Internal use only. Only used with Ogg containers. Points to a drflac_oggbs object. This is an offset of pExtraData. */
    void* _oggbs;

    /* The bit streamer. The raw FLAC data is fed through this object. */
    drflac_bs bs;

    /* Variable length extra data. We attach this to the end of the object so we can avoid unnecessary mallocs. */
    drflac_uint8 pExtraData[1];
} drflac;


/*
Opens a FLAC decoder.

onRead    [in]           The function to call when data needs to be read from the client.
onSeek    [in]           The function to call when the read position of the client data needs to move.
pUserData [in, optional] A pointer to application defined data that will be passed to onRead and onSeek.

Returns a pointer to an object representing the decoder.

Close the decoder with drflac_close().

This function will automatically detect whether or not you are attempting to open a native or Ogg encapsulated
FLAC, both of which should work seamlessly without any manual intervention. Ogg encapsulation also works with
multiplexed streams which basically means it can play FLAC encoded audio tracks in videos.

This is the lowest level function for opening a FLAC stream. You can also use drflac_open_file() and drflac_open_memory()
to open the stream from a file or from a block of memory respectively.

The STREAMINFO block must be present for this to succeed. Use drflac_open_relaxed() to open a FLAC stream where
the header may not be present.

See also: drflac_open_file(), drflac_open_memory(), drflac_open_with_metadata(), drflac_close()
*/
drflac* drflac_open(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData);

/*
The same as drflac_open(), except attempts to open the stream even when a header block is not present.

Because the header is not necessarily available, the caller must explicitly define the container (Native or Ogg). Do
not set this to drflac_container_unknown - that is for internal use only.

Opening in relaxed mode will continue reading data from onRead until it finds a valid frame. If a frame is never
found it will continue forever. To abort, force your onRead callback to return 0, which dr_flac will use as an
indicator that the end of the stream was found.
*/
drflac* drflac_open_relaxed(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_container container, void* pUserData);

/*
Opens a FLAC decoder and notifies the caller of the metadata chunks (album art, etc.).

onRead    [in]           The function to call when data needs to be read from the client.
onSeek    [in]           The function to call when the read position of the client data needs to move.
onMeta    [in]           The function to call for every metadata block.
pUserData [in, optional] A pointer to application defined data that will be passed to onRead, onSeek and onMeta.

Returns a pointer to an object representing the decoder.

Close the decoder with drflac_close().

This is slower than drflac_open(), so avoid this one if you don't need metadata. Internally, this will do a DRFLAC_MALLOC()
and DRFLAC_FREE() for every metadata block except for STREAMINFO and PADDING blocks.

The caller is notified of the metadata via the onMeta callback. All metadata blocks will be handled before the function
returns.

The STREAMINFO block must be present for this to succeed. Use drflac_open_with_metadata_relaxed() to open a FLAC
stream where the header may not be present.

Note that this will behave inconsistently with drflac_open() if the stream is an Ogg encapsulated stream and a metadata
block is corrupted. This is due to the way the Ogg stream recovers from corrupted pages. When drflac_open_with_metadata()
is being used, the open routine will try to read the contents of the metadata block, whereas drflac_open() will simply
seek past it (for the sake of efficiency). This inconsistency can result in different samples being returned depending on
whether or not the stream is being opened with metadata.

See also: drflac_open_file_with_metadata(), drflac_open_memory_with_metadata(), drflac_open(), drflac_close()
*/
drflac* drflac_open_with_metadata(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, void* pUserData);

/*
The same as drflac_open_with_metadata(), except attempts to open the stream even when a header block is not present.

See also: drflac_open_with_metadata(), drflac_open_relaxed()
*/
drflac* drflac_open_with_metadata_relaxed(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, drflac_container container, void* pUserData);

/*
Closes the given FLAC decoder.

pFlac [in] The decoder to close.

This will destroy the decoder object.
*/
void drflac_close(drflac* pFlac);


/*
Reads sample data from the given FLAC decoder, output as interleaved signed 32-bit PCM.

pFlac        [in]            The decoder.
framesToRead [in]            The number of PCM frames to read.
pBufferOut   [out, optional] A pointer to the buffer that will receive the decoded samples.

Returns the number of PCM frames actually read.

pBufferOut can be null, in which case the call will act as a seek, and the return value will be the number of frames
seeked.
*/
drflac_uint64 drflac_read_pcm_frames_s32(drflac* pFlac, drflac_uint64 framesToRead, drflac_int32* pBufferOut);

/*
Same as drflac_read_pcm_frames_s32(), except outputs samples as 16-bit integer PCM rather than 32-bit.

Note that this is lossy for streams where the bits per sample is larger than 16.
*/
drflac_uint64 drflac_read_pcm_frames_s16(drflac* pFlac, drflac_uint64 framesToRead, drflac_int16* pBufferOut);

/*
Same as drflac_read_pcm_frames_s32(), except outputs samples as 32-bit floating-point PCM.

Note that this should be considered lossy due to the nature of floating point numbers not being able to exactly
represent every possible number.
*/
drflac_uint64 drflac_read_pcm_frames_f32(drflac* pFlac, drflac_uint64 framesToRead, float* pBufferOut);

/*
Seeks to the PCM frame at the given index.

pFlac         [in] The decoder.
pcmFrameIndex [in] The index of the PCM frame to seek to. See notes below.

Returns DRFLAC_TRUE if successful; DRFLAC_FALSE otherwise.
*/
drflac_bool32 drflac_seek_to_pcm_frame(drflac* pFlac, drflac_uint64 pcmFrameIndex);



#ifndef DR_FLAC_NO_STDIO
/*
Opens a FLAC decoder from the file at the given path.

filename [in] The path of the file to open, either absolute or relative to the current directory.

Returns a pointer to an object representing the decoder.

Close the decoder with drflac_close().

This will hold a handle to the file until the decoder is closed with drflac_close(). Some platforms will restrict the
number of files a process can have open at any given time, so keep this mind if you have many decoders open at the
same time.

See also: drflac_open(), drflac_open_file_with_metadata(), drflac_close()
*/
drflac* drflac_open_file(const char* filename);

/*
Opens a FLAC decoder from the file at the given path and notifies the caller of the metadata chunks (album art, etc.)

Look at the documentation for drflac_open_with_metadata() for more information on how metadata is handled.
*/
drflac* drflac_open_file_with_metadata(const char* filename, drflac_meta_proc onMeta, void* pUserData);
#endif

/*
Opens a FLAC decoder from a pre-allocated block of memory

This does not create a copy of the data. It is up to the application to ensure the buffer remains valid for
the lifetime of the decoder.
*/
drflac* drflac_open_memory(const void* data, size_t dataSize);

/*
Opens a FLAC decoder from a pre-allocated block of memory and notifies the caller of the metadata chunks (album art, etc.)

Look at the documentation for drflac_open_with_metadata() for more information on how metadata is handled.
*/
drflac* drflac_open_memory_with_metadata(const void* data, size_t dataSize, drflac_meta_proc onMeta, void* pUserData);



/* High Level APIs */

/*
Opens a FLAC stream from the given callbacks and fully decodes it in a single operation. The return value is a
pointer to the sample data as interleaved signed 32-bit PCM. The returned data must be freed with DRFLAC_FREE().

Sometimes a FLAC file won't keep track of the total sample count. In this situation the function will continuously
read samples into a dynamically sized buffer on the heap until no samples are left.

Do not call this function on a broadcast type of stream (like internet radio streams and whatnot).
*/
drflac_int32* drflac_open_and_read_pcm_frames_s32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Same as drflac_open_and_read_pcm_frames_s32(), except returns signed 16-bit integer samples. */
drflac_int16* drflac_open_and_read_pcm_frames_s16(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Same as drflac_open_and_read_pcm_frames_s32(), except returns 32-bit floating-point samples. */
float* drflac_open_and_read_pcm_frames_f32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

#ifndef DR_FLAC_NO_STDIO
/* Same as drflac_open_and_read_pcm_frames_s32() except opens the decoder from a file. */
drflac_int32* drflac_open_file_and_read_pcm_frames_s32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Same as drflac_open_file_and_read_pcm_frames_s32(), except returns signed 16-bit integer samples. */
drflac_int16* drflac_open_file_and_read_pcm_frames_s16(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Same as drflac_open_file_and_read_pcm_frames_s32(), except returns 32-bit floating-point samples. */
float* drflac_open_file_and_read_pcm_frames_f32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);
#endif

/* Same as drflac_open_and_read_pcm_frames_s32() except opens the decoder from a block of memory. */
drflac_int32* drflac_open_memory_and_read_pcm_frames_s32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Same as drflac_open_memory_and_read_pcm_frames_s32(), except returns signed 16-bit integer samples. */
drflac_int16* drflac_open_memory_and_read_pcm_frames_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Same as drflac_open_memory_and_read_pcm_frames_s32(), except returns 32-bit floating-point samples. */
float* drflac_open_memory_and_read_pcm_frames_f32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount);

/* Frees memory that was allocated internally by dr_flac. */
void drflac_free(void* p);


/* Structure representing an iterator for vorbis comments in a VORBIS_COMMENT metadata block. */
typedef struct
{
    drflac_uint32 countRemaining;
    const char* pRunningData;
} drflac_vorbis_comment_iterator;

/*
Initializes a vorbis comment iterator. This can be used for iterating over the vorbis comments in a VORBIS_COMMENT
metadata block.
*/
void drflac_init_vorbis_comment_iterator(drflac_vorbis_comment_iterator* pIter, drflac_uint32 commentCount, const void* pComments);

/*
Goes to the next vorbis comment in the given iterator. If null is returned it means there are no more comments. The
returned string is NOT null terminated.
*/
const char* drflac_next_vorbis_comment(drflac_vorbis_comment_iterator* pIter, drflac_uint32* pCommentLengthOut);


/* Structure representing an iterator for cuesheet tracks in a CUESHEET metadata block. */
typedef struct
{
    drflac_uint32 countRemaining;
    const char* pRunningData;
} drflac_cuesheet_track_iterator;

/* Packing is important on this structure because we map this directly to the raw data within the CUESHEET metadata block. */
#pragma pack(4)
typedef struct
{
    drflac_uint64 offset;
    drflac_uint8 index;
    drflac_uint8 reserved[3];
} drflac_cuesheet_track_index;
#pragma pack()

typedef struct
{
    drflac_uint64 offset;
    drflac_uint8 trackNumber;
    char ISRC[12];
    drflac_bool8 isAudio;
    drflac_bool8 preEmphasis;
    drflac_uint8 indexCount;
    const drflac_cuesheet_track_index* pIndexPoints;
} drflac_cuesheet_track;

/*
Initializes a cuesheet track iterator. This can be used for iterating over the cuesheet tracks in a CUESHEET metadata
block.
*/
void drflac_init_cuesheet_track_iterator(drflac_cuesheet_track_iterator* pIter, drflac_uint32 trackCount, const void* pTrackData);

/* Goes to the next cuesheet track in the given iterator. If DRFLAC_FALSE is returned it means there are no more comments. */
drflac_bool32 drflac_next_cuesheet_track(drflac_cuesheet_track_iterator* pIter, drflac_cuesheet_track* pCuesheetTrack);


/* Deprecated APIs */
DRFLAC_DEPRECATED drflac_uint64 drflac_read_s32(drflac* pFlac, drflac_uint64 samplesToRead, drflac_int32* pBufferOut);    /* Use drflac_read_pcm_frames_s32() instead. */
DRFLAC_DEPRECATED drflac_uint64 drflac_read_s16(drflac* pFlac, drflac_uint64 samplesToRead, drflac_int16* pBufferOut);    /* Use drflac_read_pcm_frames_s16() instead. */
DRFLAC_DEPRECATED drflac_uint64 drflac_read_f32(drflac* pFlac, drflac_uint64 samplesToRead, float* pBufferOut);           /* Use drflac_read_pcm_frames_f32() instead. */
DRFLAC_DEPRECATED drflac_bool32 drflac_seek_to_sample(drflac* pFlac, drflac_uint64 sampleIndex);                          /* Use drflac_seek_to_pcm_frame() instead. */
DRFLAC_DEPRECATED drflac_int32* drflac_open_and_decode_s32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount); /* Use drflac_open_and_read_pcm_frames_s32(). */
DRFLAC_DEPRECATED drflac_int16* drflac_open_and_decode_s16(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount); /* Use drflac_open_and_read_pcm_frames_s16(). */
DRFLAC_DEPRECATED float* drflac_open_and_decode_f32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);        /* Use drflac_open_and_read_pcm_frames_f32(). */
DRFLAC_DEPRECATED drflac_int32* drflac_open_and_decode_file_s32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);                                         /* Use drflac_open_file_and_read_pcm_frames_s32(). */
DRFLAC_DEPRECATED drflac_int16* drflac_open_and_decode_file_s16(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);                                         /* Use drflac_open_file_and_read_pcm_frames_s16(). */
DRFLAC_DEPRECATED float* drflac_open_and_decode_file_f32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);                                                /* Use drflac_open_file_and_read_pcm_frames_f32(). */
DRFLAC_DEPRECATED drflac_int32* drflac_open_and_decode_memory_s32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);                          /* Use drflac_open_memory_and_read_pcm_frames_s32(). */
DRFLAC_DEPRECATED drflac_int16* drflac_open_and_decode_memory_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);                          /* Use drflac_open_memory_and_read_pcm_frames_s16(). */
DRFLAC_DEPRECATED float* drflac_open_and_decode_memory_f32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalSampleCount);                                 /* Use drflac_open_memory_and_read_pcm_frames_f32(). */

#ifdef __cplusplus
}
#endif
#endif  /* dr_flac_h */


/************************************************************************************************************************************************************
 ************************************************************************************************************************************************************

 IMPLEMENTATION

 ************************************************************************************************************************************************************
 ************************************************************************************************************************************************************/
#ifdef DR_FLAC_IMPLEMENTATION

/* Disable some annoying warnings. */
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #if __GNUC__ >= 7
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
    #endif
#endif

#ifdef __linux__
    #ifndef _BSD_SOURCE
        #define _BSD_SOURCE
    #endif
    #ifndef __USE_BSD
        #define __USE_BSD
    #endif
    #include <endian.h>
#endif

#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define DRFLAC_INLINE __forceinline
#else
#ifdef __GNUC__
#define DRFLAC_INLINE __inline__ __attribute__((always_inline))
#else
#define DRFLAC_INLINE
#endif
#endif

/* CPU architecture. */
#if defined(__x86_64__) || defined(_M_X64)
    #define DRFLAC_X64
#elif defined(__i386) || defined(_M_IX86)
    #define DRFLAC_X86
#elif defined(__arm__) || defined(_M_ARM)
    #define DRFLAC_ARM
#endif

/* Intrinsics Support */
#if !defined(DR_FLAC_NO_SIMD)
    #if defined(DRFLAC_X64) || defined(DRFLAC_X86)
        #if defined(_MSC_VER) && !defined(__clang__)
            /* MSVC. */
            #if _MSC_VER >= 1400 && !defined(DRFLAC_NO_SSE2)    /* 2005 */
                #define DRFLAC_SUPPORT_SSE2
            #endif
            #if _MSC_VER >= 1600 && !defined(DRFLAC_NO_SSE41)   /* 2010 */
                #define DRFLAC_SUPPORT_SSE41
            #endif
        #else
            /* Assume GNUC-style. */
            #if defined(__SSE2__) && !defined(DRFLAC_NO_SSE2)
                #define DRFLAC_SUPPORT_SSE2
            #endif
            #if defined(__SSE4_1__) && !defined(DRFLAC_NO_SSE41)
                #define DRFLAC_SUPPORT_SSE41
            #endif
        #endif

        /* If at this point we still haven't determined compiler support for the intrinsics just fall back to __has_include. */
        #if !defined(__GNUC__) && !defined(__clang__) && defined(__has_include)
            #if !defined(DRFLAC_SUPPORT_SSE2) && !defined(DRFLAC_NO_SSE2) && __has_include(<emmintrin.h>)
                #define DRFLAC_SUPPORT_SSE2
            #endif
            #if !defined(DRFLAC_SUPPORT_SSE41) && !defined(DRFLAC_NO_SSE41) && __has_include(<smmintrin.h>)
                #define DRFLAC_SUPPORT_SSE41
            #endif
        #endif

        #if defined(DRFLAC_SUPPORT_SSE41)
            #include <smmintrin.h>
        #elif defined(DRFLAC_SUPPORT_SSE2)
            #include <emmintrin.h>
        #endif
    #endif

    #if defined(DRFLAC_ARM)
        #if !defined(DRFLAC_NO_NEON) && (defined(__ARM_NEON) || defined(__aarch64__) || defined(_M_ARM64))
            #define DRFLAC_SUPPORT_NEON
        #endif

        /* Fall back to looking for the #include file. */
        #if !defined(__GNUC__) && !defined(__clang__) && defined(__has_include)
            #if !defined(DRFLAC_SUPPORT_NEON) && !defined(DRFLAC_NO_NEON) && __has_include(<arm_neon.h>)
                #define DRFLAC_SUPPORT_NEON
            #endif
        #endif

        #if defined(DRFLAC_SUPPORT_NEON)
            #include <arm_neon.h>
        #endif
    #endif
#endif

/* Compile-time CPU feature support. */
#if !defined(DR_FLAC_NO_SIMD) && (defined(DRFLAC_X86) || defined(DRFLAC_X64))
    #if defined(_MSC_VER) && !defined(__clang__)
        #if _MSC_VER >= 1400
            #include <intrin.h>
            static void drflac__cpuid(int info[4], int fid)
            {
                __cpuid(info, fid);
            }
        #else
            #define DRFLAC_NO_CPUID
        #endif
    #else
        #if defined(__GNUC__) || defined(__clang__)
            static void drflac__cpuid(int info[4], int fid)
            {
                /*
                It looks like the -fPIC option uses the ebx register which GCC complains about. We can work around this by just using a different register, the
                specific register of which I'm letting the compiler decide on. The "k" prefix is used to specify a 32-bit register. The {...} syntax is for
                supporting different assembly dialects.
                
                What's basically happening is that we're saving and restoring the ebx register manually.
                */
                #if defined(DRFLAC_X86) && defined(__PIC__)
                    __asm__ __volatile__ (
                        "xchg{l} {%%}ebx, %k1;"
                        "cpuid;"
                        "xchg{l} {%%}ebx, %k1;"
                        : "=a"(info[0]), "=&r"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(fid), "c"(0)
                    );
                #else
                    __asm__ __volatile__ (
                        "cpuid" : "=a"(info[0]), "=b"(info[1]), "=c"(info[2]), "=d"(info[3]) : "a"(fid), "c"(0)
                    );
                #endif
            }
        #else
            #define DRFLAC_NO_CPUID
        #endif
    #endif
#else
    #define DRFLAC_NO_CPUID
#endif

static DRFLAC_INLINE drflac_bool32 drflac_has_sse2()
{
#if defined(DRFLAC_SUPPORT_SSE2)
    #if (defined(DRFLAC_X64) || defined(DRFLAC_X86)) && !defined(DRFLAC_NO_SSE2)
        #if defined(DRFLAC_X64)
            return DRFLAC_TRUE;    /* 64-bit targets always support SSE2. */
        #elif (defined(_M_IX86_FP) && _M_IX86_FP == 2) || defined(__SSE2__)
            return DRFLAC_TRUE;    /* If the compiler is allowed to freely generate SSE2 code we can assume support. */
        #else
            #if defined(DRFLAC_NO_CPUID)
                return DRFLAC_FALSE;
            #else
                int info[4];
                drflac__cpuid(info, 1);
                return (info[3] & (1 << 26)) != 0;
            #endif
        #endif
    #else
        return DRFLAC_FALSE;       /* SSE2 is only supported on x86 and x64 architectures. */
    #endif
#else
    return DRFLAC_FALSE;           /* No compiler support. */
#endif
}

static DRFLAC_INLINE drflac_bool32 drflac_has_sse41()
{
#if defined(DRFLAC_SUPPORT_SSE41)
    #if (defined(DRFLAC_X64) || defined(DRFLAC_X86)) && !defined(DRFLAC_NO_SSE41)
        #if defined(DRFLAC_X64)
            return DRFLAC_TRUE;    /* 64-bit targets always support SSE4.1. */
        #elif (defined(_M_IX86_FP) && _M_IX86_FP == 2) || defined(__SSE4_1__)
            return DRFLAC_TRUE;    /* If the compiler is allowed to freely generate SSE41 code we can assume support. */
        #else
            #if defined(DRFLAC_NO_CPUID)
                return DRFLAC_FALSE;
            #else
                int info[4];
                drflac__cpuid(info, 1);
                return (info[2] & (1 << 19)) != 0;
            #endif
        #endif
    #else
        return DRFLAC_FALSE;       /* SSE41 is only supported on x86 and x64 architectures. */
    #endif
#else
    return DRFLAC_FALSE;           /* No compiler support. */
#endif
}


#if defined(_MSC_VER) && _MSC_VER >= 1500 && (defined(DRFLAC_X86) || defined(DRFLAC_X64))
    #define DRFLAC_HAS_LZCNT_INTRINSIC
#elif (defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)))
    #define DRFLAC_HAS_LZCNT_INTRINSIC
#elif defined(__clang__)
    #if __has_builtin(__builtin_clzll) || __has_builtin(__builtin_clzl)
        #define DRFLAC_HAS_LZCNT_INTRINSIC
    #endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1300
    #define DRFLAC_HAS_BYTESWAP16_INTRINSIC
    #define DRFLAC_HAS_BYTESWAP32_INTRINSIC
    #define DRFLAC_HAS_BYTESWAP64_INTRINSIC
#elif defined(__clang__)
    #if __has_builtin(__builtin_bswap16)
        #define DRFLAC_HAS_BYTESWAP16_INTRINSIC
    #endif
    #if __has_builtin(__builtin_bswap32)
        #define DRFLAC_HAS_BYTESWAP32_INTRINSIC
    #endif
    #if __has_builtin(__builtin_bswap64)
        #define DRFLAC_HAS_BYTESWAP64_INTRINSIC
    #endif
#elif defined(__GNUC__)
    #if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
        #define DRFLAC_HAS_BYTESWAP32_INTRINSIC
        #define DRFLAC_HAS_BYTESWAP64_INTRINSIC
    #endif
    #if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
        #define DRFLAC_HAS_BYTESWAP16_INTRINSIC
    #endif
#endif


/* Standard library stuff. */
#ifndef DRFLAC_ASSERT
#include <assert.h>
#define DRFLAC_ASSERT(expression)           assert(expression)
#endif
#ifndef DRFLAC_MALLOC
#define DRFLAC_MALLOC(sz)                   malloc((sz))
#endif
#ifndef DRFLAC_REALLOC
#define DRFLAC_REALLOC(p, sz)               realloc((p), (sz))
#endif
#ifndef DRFLAC_FREE
#define DRFLAC_FREE(p)                      free((p))
#endif
#ifndef DRFLAC_COPY_MEMORY
#define DRFLAC_COPY_MEMORY(dst, src, sz)    memcpy((dst), (src), (sz))
#endif
#ifndef DRFLAC_ZERO_MEMORY
#define DRFLAC_ZERO_MEMORY(p, sz)           memset((p), 0, (sz))
#endif

#define DRFLAC_MAX_SIMD_VECTOR_SIZE                     64  /* 64 for AVX-512 in the future. */

typedef drflac_int32 drflac_result;
#define DRFLAC_SUCCESS                                  0
#define DRFLAC_ERROR                                    -1  /* A generic error. */
#define DRFLAC_INVALID_ARGS                             -2
#define DRFLAC_END_OF_STREAM                            -128
#define DRFLAC_CRC_MISMATCH                             -129

#define DRFLAC_SUBFRAME_CONSTANT                        0
#define DRFLAC_SUBFRAME_VERBATIM                        1
#define DRFLAC_SUBFRAME_FIXED                           8
#define DRFLAC_SUBFRAME_LPC                             32
#define DRFLAC_SUBFRAME_RESERVED                        255

#define DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE  0
#define DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2 1

#define DRFLAC_CHANNEL_ASSIGNMENT_INDEPENDENT           0
#define DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE             8
#define DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE            9
#define DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE              10

/*
Keeps track of the number of leading samples for each sub-frame. This is required because the SSE pipeline will occasionally
reference excess prior samples.
*/
#define DRFLAC_LEADING_SAMPLES                          32


#define drflac_align(x, a)                              ((((x) + (a) - 1) / (a)) * (a))
#define drflac_assert                                   DRFLAC_ASSERT
#define drflac_copy_memory                              DRFLAC_COPY_MEMORY
#define drflac_zero_memory                              DRFLAC_ZERO_MEMORY


/* CPU caps. */
static drflac_bool32 drflac__gIsLZCNTSupported = DRFLAC_FALSE;
#ifndef DRFLAC_NO_CPUID
/*
I've had a bug report that Clang's ThreadSanitizer presents a warning in this function. Having reviewed this, this does
actually make sense. However, since CPU caps should never differ for a running process, I don't think the trade off of
complicating internal API's by passing around CPU caps versus just disabling the warnings is worthwhile. I'm therefore
just going to disable these warnings.
*/
#if defined(__has_feature)
    #if __has_feature(thread_sanitizer)
        #define DRFLAC_NO_THREAD_SANITIZE __attribute__((no_sanitize("thread")))
    #else
        #define DRFLAC_NO_THREAD_SANITIZE
    #endif
#else
    #define DRFLAC_NO_THREAD_SANITIZE
#endif
static drflac_bool32 drflac__gIsSSE2Supported  = DRFLAC_FALSE;
static drflac_bool32 drflac__gIsSSE41Supported = DRFLAC_FALSE;
DRFLAC_NO_THREAD_SANITIZE static void drflac__init_cpu_caps()
{
    static drflac_bool32 isCPUCapsInitialized = DRFLAC_FALSE;

    if (!isCPUCapsInitialized) {
        int info[4] = {0};

        /* LZCNT */
        drflac__cpuid(info, 0x80000001);
        drflac__gIsLZCNTSupported = (info[2] & (1 << 5)) != 0;

        /* SSE2 */
        drflac__gIsSSE2Supported = drflac_has_sse2();

        /* SSE4.1 */
        drflac__gIsSSE41Supported = drflac_has_sse41();

        /* Initialized. */
        isCPUCapsInitialized = DRFLAC_TRUE;
    }
}
#endif


/* Endian Management */
static DRFLAC_INLINE drflac_bool32 drflac__is_little_endian()
{
#if defined(DRFLAC_X86) || defined(DRFLAC_X64)
    return DRFLAC_TRUE;
#elif defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && __BYTE_ORDER == __LITTLE_ENDIAN
    return DRFLAC_TRUE;
#else
    int n = 1;
    return (*(char*)&n) == 1;
#endif
}

static DRFLAC_INLINE drflac_uint16 drflac__swap_endian_uint16(drflac_uint16 n)
{
#ifdef DRFLAC_HAS_BYTESWAP16_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_ushort(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap16(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & 0xFF00) >> 8) |
           ((n & 0x00FF) << 8);
#endif
}

static DRFLAC_INLINE drflac_uint32 drflac__swap_endian_uint32(drflac_uint32 n)
{
#ifdef DRFLAC_HAS_BYTESWAP32_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_ulong(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap32(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & 0xFF000000) >> 24) |
           ((n & 0x00FF0000) >>  8) |
           ((n & 0x0000FF00) <<  8) |
           ((n & 0x000000FF) << 24);
#endif
}

static DRFLAC_INLINE drflac_uint64 drflac__swap_endian_uint64(drflac_uint64 n)
{
#ifdef DRFLAC_HAS_BYTESWAP64_INTRINSIC
    #if defined(_MSC_VER)
        return _byteswap_uint64(n);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_bswap64(n);
    #else
        #error "This compiler does not support the byte swap intrinsic."
    #endif
#else
    return ((n & (drflac_uint64)0xFF00000000000000) >> 56) |
           ((n & (drflac_uint64)0x00FF000000000000) >> 40) |
           ((n & (drflac_uint64)0x0000FF0000000000) >> 24) |
           ((n & (drflac_uint64)0x000000FF00000000) >>  8) |
           ((n & (drflac_uint64)0x00000000FF000000) <<  8) |
           ((n & (drflac_uint64)0x0000000000FF0000) << 24) |
           ((n & (drflac_uint64)0x000000000000FF00) << 40) |
           ((n & (drflac_uint64)0x00000000000000FF) << 56);
#endif
}


static DRFLAC_INLINE drflac_uint16 drflac__be2host_16(drflac_uint16 n)
{
    if (drflac__is_little_endian()) {
        return drflac__swap_endian_uint16(n);
    }

    return n;
}

static DRFLAC_INLINE drflac_uint32 drflac__be2host_32(drflac_uint32 n)
{
    if (drflac__is_little_endian()) {
        return drflac__swap_endian_uint32(n);
    }

    return n;
}

static DRFLAC_INLINE drflac_uint64 drflac__be2host_64(drflac_uint64 n)
{
    if (drflac__is_little_endian()) {
        return drflac__swap_endian_uint64(n);
    }

    return n;
}


static DRFLAC_INLINE drflac_uint32 drflac__le2host_32(drflac_uint32 n)
{
    if (!drflac__is_little_endian()) {
        return drflac__swap_endian_uint32(n);
    }

    return n;
}


static DRFLAC_INLINE drflac_uint32 drflac__unsynchsafe_32(drflac_uint32 n)
{
    drflac_uint32 result = 0;
    result |= (n & 0x7F000000) >> 3;
    result |= (n & 0x007F0000) >> 2;
    result |= (n & 0x00007F00) >> 1;
    result |= (n & 0x0000007F) >> 0;

    return result;
}



/* The CRC code below is based on this document: http://zlib.net/crc_v3.txt */
static drflac_uint8 drflac__crc8_table[] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

static drflac_uint16 drflac__crc16_table[] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

static DRFLAC_INLINE drflac_uint8 drflac_crc8_byte(drflac_uint8 crc, drflac_uint8 data)
{
    return drflac__crc8_table[crc ^ data];
}

static DRFLAC_INLINE drflac_uint8 drflac_crc8(drflac_uint8 crc, drflac_uint32 data, drflac_uint32 count)
{
#ifdef DR_FLAC_NO_CRC
    (void)crc;
    (void)data;
    (void)count;
    return 0;
#else
#if 0
    /* REFERENCE (use of this implementation requires an explicit flush by doing "drflac_crc8(crc, 0, 8);") */
    drflac_uint8 p = 0x07;
    for (int i = count-1; i >= 0; --i) {
        drflac_uint8 bit = (data & (1 << i)) >> i;
        if (crc & 0x80) {
            crc = ((crc << 1) | bit) ^ p;
        } else {
            crc = ((crc << 1) | bit);
        }
    }
    return crc;
#else
    drflac_uint32 wholeBytes;
    drflac_uint32 leftoverBits;
    drflac_uint64 leftoverDataMask;

    static drflac_uint64 leftoverDataMaskTable[8] = {
        0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
    };
    
    drflac_assert(count <= 32);

    wholeBytes = count >> 3;
    leftoverBits = count - (wholeBytes*8);
    leftoverDataMask = leftoverDataMaskTable[leftoverBits];

    switch (wholeBytes) {
        case 4: crc = drflac_crc8_byte(crc, (drflac_uint8)((data & (0xFF000000UL << leftoverBits)) >> (24 + leftoverBits)));
        case 3: crc = drflac_crc8_byte(crc, (drflac_uint8)((data & (0x00FF0000UL << leftoverBits)) >> (16 + leftoverBits)));
        case 2: crc = drflac_crc8_byte(crc, (drflac_uint8)((data & (0x0000FF00UL << leftoverBits)) >> ( 8 + leftoverBits)));
        case 1: crc = drflac_crc8_byte(crc, (drflac_uint8)((data & (0x000000FFUL << leftoverBits)) >> ( 0 + leftoverBits)));
        case 0: if (leftoverBits > 0) crc = (crc << leftoverBits) ^ drflac__crc8_table[(crc >> (8 - leftoverBits)) ^ (data & leftoverDataMask)];
    }
    return crc;
#endif
#endif
}

static DRFLAC_INLINE drflac_uint16 drflac_crc16_byte(drflac_uint16 crc, drflac_uint8 data)
{
    return (crc << 8) ^ drflac__crc16_table[(drflac_uint8)(crc >> 8) ^ data];
}

static DRFLAC_INLINE drflac_uint16 drflac_crc16_bytes(drflac_uint16 crc, drflac_cache_t data, drflac_uint32 byteCount)
{
    switch (byteCount)
    {
#ifdef DRFLAC_64BIT
    case 8: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >> 56) & 0xFF));
    case 7: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >> 48) & 0xFF));
    case 6: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >> 40) & 0xFF));
    case 5: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >> 32) & 0xFF));
#endif
    case 4: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >> 24) & 0xFF));
    case 3: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >> 16) & 0xFF));
    case 2: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >>  8) & 0xFF));
    case 1: crc = drflac_crc16_byte(crc, (drflac_uint8)((data >>  0) & 0xFF));
    }

    return crc;
}

static DRFLAC_INLINE drflac_uint16 drflac_crc16__32bit(drflac_uint16 crc, drflac_uint32 data, drflac_uint32 count)
{
#ifdef DR_FLAC_NO_CRC
    (void)crc;
    (void)data;
    (void)count;
    return 0;
#else
#if 0
    /* REFERENCE (use of this implementation requires an explicit flush by doing "drflac_crc16(crc, 0, 16);") */
    drflac_uint16 p = 0x8005;
    for (int i = count-1; i >= 0; --i) {
        drflac_uint16 bit = (data & (1ULL << i)) >> i;
        if (r & 0x8000) {
            r = ((r << 1) | bit) ^ p;
        } else {
            r = ((r << 1) | bit);
        }
    }

    return crc;
#else
    drflac_uint32 wholeBytes;
    drflac_uint32 leftoverBits;
    drflac_uint64 leftoverDataMask;

    static drflac_uint64 leftoverDataMaskTable[8] = {
        0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
    };

    drflac_assert(count <= 64);
    
    wholeBytes = count >> 3;
    leftoverBits = count - (wholeBytes*8);
    leftoverDataMask = leftoverDataMaskTable[leftoverBits];

    switch (wholeBytes) {
        default:
        case 4: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (0xFF000000UL << leftoverBits)) >> (24 + leftoverBits)));
        case 3: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (0x00FF0000UL << leftoverBits)) >> (16 + leftoverBits)));
        case 2: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (0x0000FF00UL << leftoverBits)) >> ( 8 + leftoverBits)));
        case 1: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (0x000000FFUL << leftoverBits)) >> ( 0 + leftoverBits)));
        case 0: if (leftoverBits > 0) crc = (crc << leftoverBits) ^ drflac__crc16_table[(crc >> (16 - leftoverBits)) ^ (data & leftoverDataMask)];
    }
    return crc;
#endif
#endif
}

static DRFLAC_INLINE drflac_uint16 drflac_crc16__64bit(drflac_uint16 crc, drflac_uint64 data, drflac_uint32 count)
{
#ifdef DR_FLAC_NO_CRC
    (void)crc;
    (void)data;
    (void)count;
    return 0;
#else
    drflac_uint32 wholeBytes;
    drflac_uint32 leftoverBits;
    drflac_uint64 leftoverDataMask;

    static drflac_uint64 leftoverDataMaskTable[8] = {
        0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
    };
    
    drflac_assert(count <= 64);

    wholeBytes = count >> 3;
    leftoverBits = count - (wholeBytes*8);
    leftoverDataMask = leftoverDataMaskTable[leftoverBits];

    switch (wholeBytes) {
        default:
        case 8: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0xFF000000 << 32) << leftoverBits)) >> (56 + leftoverBits)));    /* Weird "<< 32" bitshift is required for C89 because it doesn't support 64-bit constants. Should be optimized out by a good compiler. */
        case 7: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0x00FF0000 << 32) << leftoverBits)) >> (48 + leftoverBits)));
        case 6: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0x0000FF00 << 32) << leftoverBits)) >> (40 + leftoverBits)));
        case 5: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0x000000FF << 32) << leftoverBits)) >> (32 + leftoverBits)));
        case 4: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0xFF000000      ) << leftoverBits)) >> (24 + leftoverBits)));
        case 3: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0x00FF0000      ) << leftoverBits)) >> (16 + leftoverBits)));
        case 2: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0x0000FF00      ) << leftoverBits)) >> ( 8 + leftoverBits)));
        case 1: crc = drflac_crc16_byte(crc, (drflac_uint8)((data & (((drflac_uint64)0x000000FF      ) << leftoverBits)) >> ( 0 + leftoverBits)));
        case 0: if (leftoverBits > 0) crc = (crc << leftoverBits) ^ drflac__crc16_table[(crc >> (16 - leftoverBits)) ^ (data & leftoverDataMask)];
    }
    return crc;
#endif
}


static DRFLAC_INLINE drflac_uint16 drflac_crc16(drflac_uint16 crc, drflac_cache_t data, drflac_uint32 count)
{
#ifdef DRFLAC_64BIT
    return drflac_crc16__64bit(crc, data, count);
#else
    return drflac_crc16__32bit(crc, data, count);
#endif
}


#ifdef DRFLAC_64BIT
#define drflac__be2host__cache_line drflac__be2host_64
#else
#define drflac__be2host__cache_line drflac__be2host_32
#endif

/*
BIT READING ATTEMPT #2

This uses a 32- or 64-bit bit-shifted cache - as bits are read, the cache is shifted such that the first valid bit is sitting
on the most significant bit. It uses the notion of an L1 and L2 cache (borrowed from CPU architecture), where the L1 cache
is a 32- or 64-bit unsigned integer (depending on whether or not a 32- or 64-bit build is being compiled) and the L2 is an
array of "cache lines", with each cache line being the same size as the L1. The L2 is a buffer of about 4KB and is where data
from onRead() is read into.
*/
#define DRFLAC_CACHE_L1_SIZE_BYTES(bs)                      (sizeof((bs)->cache))
#define DRFLAC_CACHE_L1_SIZE_BITS(bs)                       (sizeof((bs)->cache)*8)
#define DRFLAC_CACHE_L1_BITS_REMAINING(bs)                  (DRFLAC_CACHE_L1_SIZE_BITS(bs) - (bs)->consumedBits)
#define DRFLAC_CACHE_L1_SELECTION_MASK(_bitCount)           (~((~(drflac_cache_t)0) >> (_bitCount)))
#define DRFLAC_CACHE_L1_SELECTION_SHIFT(bs, _bitCount)      (DRFLAC_CACHE_L1_SIZE_BITS(bs) - (_bitCount))
#define DRFLAC_CACHE_L1_SELECT(bs, _bitCount)               (((bs)->cache) & DRFLAC_CACHE_L1_SELECTION_MASK(_bitCount))
#define DRFLAC_CACHE_L1_SELECT_AND_SHIFT(bs, _bitCount)     (DRFLAC_CACHE_L1_SELECT((bs), (_bitCount)) >>  DRFLAC_CACHE_L1_SELECTION_SHIFT((bs), (_bitCount)))
#define DRFLAC_CACHE_L1_SELECT_AND_SHIFT_SAFE(bs, _bitCount)(DRFLAC_CACHE_L1_SELECT((bs), (_bitCount)) >> (DRFLAC_CACHE_L1_SELECTION_SHIFT((bs), (_bitCount)) & (DRFLAC_CACHE_L1_SIZE_BITS(bs)-1)))
#define DRFLAC_CACHE_L2_SIZE_BYTES(bs)                      (sizeof((bs)->cacheL2))
#define DRFLAC_CACHE_L2_LINE_COUNT(bs)                      (DRFLAC_CACHE_L2_SIZE_BYTES(bs) / sizeof((bs)->cacheL2[0]))
#define DRFLAC_CACHE_L2_LINES_REMAINING(bs)                 (DRFLAC_CACHE_L2_LINE_COUNT(bs) - (bs)->nextL2Line)


#ifndef DR_FLAC_NO_CRC
static DRFLAC_INLINE void drflac__reset_crc16(drflac_bs* bs)
{
    bs->crc16 = 0;
    bs->crc16CacheIgnoredBytes = bs->consumedBits >> 3;
}

static DRFLAC_INLINE void drflac__update_crc16(drflac_bs* bs)
{
    bs->crc16 = drflac_crc16_bytes(bs->crc16, bs->crc16Cache, DRFLAC_CACHE_L1_SIZE_BYTES(bs) - bs->crc16CacheIgnoredBytes);
    bs->crc16CacheIgnoredBytes = 0;
}

static DRFLAC_INLINE drflac_uint16 drflac__flush_crc16(drflac_bs* bs)
{
    /* We should never be flushing in a situation where we are not aligned on a byte boundary. */
    drflac_assert((DRFLAC_CACHE_L1_BITS_REMAINING(bs) & 7) == 0);

    /*
    The bits that were read from the L1 cache need to be accumulated. The number of bytes needing to be accumulated is determined
    by the number of bits that have been consumed.
    */
    if (DRFLAC_CACHE_L1_BITS_REMAINING(bs) == 0) {
        drflac__update_crc16(bs);
    } else {
        /* We only accumulate the consumed bits. */
        bs->crc16 = drflac_crc16_bytes(bs->crc16, bs->crc16Cache >> DRFLAC_CACHE_L1_BITS_REMAINING(bs), (bs->consumedBits >> 3) - bs->crc16CacheIgnoredBytes);

        /*
        The bits that we just accumulated should never be accumulated again. We need to keep track of how many bytes were accumulated
        so we can handle that later.
        */
        bs->crc16CacheIgnoredBytes = bs->consumedBits >> 3;
    }

    return bs->crc16;
}
#endif

static DRFLAC_INLINE drflac_bool32 drflac__reload_l1_cache_from_l2(drflac_bs* bs)
{
    size_t bytesRead;
    size_t alignedL1LineCount;

    /* Fast path. Try loading straight from L2. */
    if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
        bs->cache = bs->cacheL2[bs->nextL2Line++];
        return DRFLAC_TRUE;
    }

    /*
    If we get here it means we've run out of data in the L2 cache. We'll need to fetch more from the client, if there's
    any left.
    */
    if (bs->unalignedByteCount > 0) {
        return DRFLAC_FALSE;   /* If we have any unaligned bytes it means there's no more aligned bytes left in the client. */
    }

    bytesRead = bs->onRead(bs->pUserData, bs->cacheL2, DRFLAC_CACHE_L2_SIZE_BYTES(bs));

    bs->nextL2Line = 0;
    if (bytesRead == DRFLAC_CACHE_L2_SIZE_BYTES(bs)) {
        bs->cache = bs->cacheL2[bs->nextL2Line++];
        return DRFLAC_TRUE;
    }


    /*
    If we get here it means we were unable to retrieve enough data to fill the entire L2 cache. It probably
    means we've just reached the end of the file. We need to move the valid data down to the end of the buffer
    and adjust the index of the next line accordingly. Also keep in mind that the L2 cache must be aligned to
    the size of the L1 so we'll need to seek backwards by any misaligned bytes.
    */
    alignedL1LineCount = bytesRead / DRFLAC_CACHE_L1_SIZE_BYTES(bs);

    /* We need to keep track of any unaligned bytes for later use. */
    bs->unalignedByteCount = bytesRead - (alignedL1LineCount * DRFLAC_CACHE_L1_SIZE_BYTES(bs));
    if (bs->unalignedByteCount > 0) {
        bs->unalignedCache = bs->cacheL2[alignedL1LineCount];
    }

    if (alignedL1LineCount > 0) {
        size_t offset = DRFLAC_CACHE_L2_LINE_COUNT(bs) - alignedL1LineCount;
        size_t i;
        for (i = alignedL1LineCount; i > 0; --i) {
            bs->cacheL2[i-1 + offset] = bs->cacheL2[i-1];
        }

        bs->nextL2Line = (drflac_uint32)offset;
        bs->cache = bs->cacheL2[bs->nextL2Line++];
        return DRFLAC_TRUE;
    } else {
        /* If we get into this branch it means we weren't able to load any L1-aligned data. */
        bs->nextL2Line = DRFLAC_CACHE_L2_LINE_COUNT(bs);
        return DRFLAC_FALSE;
    }
}

static drflac_bool32 drflac__reload_cache(drflac_bs* bs)
{
    size_t bytesRead;

#ifndef DR_FLAC_NO_CRC
    drflac__update_crc16(bs);
#endif

    /* Fast path. Try just moving the next value in the L2 cache to the L1 cache. */
    if (drflac__reload_l1_cache_from_l2(bs)) {
        bs->cache = drflac__be2host__cache_line(bs->cache);
        bs->consumedBits = 0;
#ifndef DR_FLAC_NO_CRC
        bs->crc16Cache = bs->cache;
#endif
        return DRFLAC_TRUE;
    }

    /* Slow path. */

    /*
    If we get here it means we have failed to load the L1 cache from the L2. Likely we've just reached the end of the stream and the last
    few bytes did not meet the alignment requirements for the L2 cache. In this case we need to fall back to a slower path and read the
    data from the unaligned cache.
    */
    bytesRead = bs->unalignedByteCount;
    if (bytesRead == 0) {
        bs->consumedBits = DRFLAC_CACHE_L1_SIZE_BITS(bs);   /* <-- The stream has been exhausted, so marked the bits as consumed. */
        return DRFLAC_FALSE;
    }

    drflac_assert(bytesRead < DRFLAC_CACHE_L1_SIZE_BYTES(bs));
    bs->consumedBits = (drflac_uint32)(DRFLAC_CACHE_L1_SIZE_BYTES(bs) - bytesRead) * 8;

    bs->cache = drflac__be2host__cache_line(bs->unalignedCache);
    bs->cache &= DRFLAC_CACHE_L1_SELECTION_MASK(DRFLAC_CACHE_L1_BITS_REMAINING(bs));    /* <-- Make sure the consumed bits are always set to zero. Other parts of the library depend on this property. */
    bs->unalignedByteCount = 0;     /* <-- At this point the unaligned bytes have been moved into the cache and we thus have no more unaligned bytes. */

#ifndef DR_FLAC_NO_CRC
    bs->crc16Cache = bs->cache >> bs->consumedBits;
    bs->crc16CacheIgnoredBytes = bs->consumedBits >> 3;
#endif
    return DRFLAC_TRUE;
}

static void drflac__reset_cache(drflac_bs* bs)
{
    bs->nextL2Line   = DRFLAC_CACHE_L2_LINE_COUNT(bs);  /* <-- This clears the L2 cache. */
    bs->consumedBits = DRFLAC_CACHE_L1_SIZE_BITS(bs);   /* <-- This clears the L1 cache. */
    bs->cache = 0;
    bs->unalignedByteCount = 0;                         /* <-- This clears the trailing unaligned bytes. */
    bs->unalignedCache = 0;

#ifndef DR_FLAC_NO_CRC
    bs->crc16Cache = 0;
    bs->crc16CacheIgnoredBytes = 0;
#endif
}


static DRFLAC_INLINE drflac_bool32 drflac__read_uint32(drflac_bs* bs, unsigned int bitCount, drflac_uint32* pResultOut)
{
    drflac_assert(bs != NULL);
    drflac_assert(pResultOut != NULL);
    drflac_assert(bitCount > 0);
    drflac_assert(bitCount <= 32);

    if (bs->consumedBits == DRFLAC_CACHE_L1_SIZE_BITS(bs)) {
        if (!drflac__reload_cache(bs)) {
            return DRFLAC_FALSE;
        }
    }

    if (bitCount <= DRFLAC_CACHE_L1_BITS_REMAINING(bs)) {
        /*
        If we want to load all 32-bits from a 32-bit cache we need to do it slightly differently because we can't do
        a 32-bit shift on a 32-bit integer. This will never be the case on 64-bit caches, so we can have a slightly
        more optimal solution for this.
        */
#ifdef DRFLAC_64BIT
        *pResultOut = (drflac_uint32)DRFLAC_CACHE_L1_SELECT_AND_SHIFT(bs, bitCount);
        bs->consumedBits += bitCount;
        bs->cache <<= bitCount;
#else
        if (bitCount < DRFLAC_CACHE_L1_SIZE_BITS(bs)) {
            *pResultOut = (drflac_uint32)DRFLAC_CACHE_L1_SELECT_AND_SHIFT(bs, bitCount);
            bs->consumedBits += bitCount;
            bs->cache <<= bitCount;
        } else {
            /* Cannot shift by 32-bits, so need to do it differently. */
            *pResultOut = (drflac_uint32)bs->cache;
            bs->consumedBits = DRFLAC_CACHE_L1_SIZE_BITS(bs);
            bs->cache = 0;
        }
#endif

        return DRFLAC_TRUE;
    } else {
        /* It straddles the cached data. It will never cover more than the next chunk. We just read the number in two parts and combine them. */
        drflac_uint32 bitCountHi = DRFLAC_CACHE_L1_BITS_REMAINING(bs);
        drflac_uint32 bitCountLo = bitCount - bitCountHi;
        drflac_uint32 resultHi = (drflac_uint32)DRFLAC_CACHE_L1_SELECT_AND_SHIFT(bs, bitCountHi);

        if (!drflac__reload_cache(bs)) {
            return DRFLAC_FALSE;
        }

        *pResultOut = (resultHi << bitCountLo) | (drflac_uint32)DRFLAC_CACHE_L1_SELECT_AND_SHIFT(bs, bitCountLo);
        bs->consumedBits += bitCountLo;
        bs->cache <<= bitCountLo;
        return DRFLAC_TRUE;
    }
}

static drflac_bool32 drflac__read_int32(drflac_bs* bs, unsigned int bitCount, drflac_int32* pResult)
{
    drflac_uint32 result;
    drflac_uint32 signbit;

    drflac_assert(bs != NULL);
    drflac_assert(pResult != NULL);
    drflac_assert(bitCount > 0);
    drflac_assert(bitCount <= 32);

    if (!drflac__read_uint32(bs, bitCount, &result)) {
        return DRFLAC_FALSE;
    }

    signbit = ((result >> (bitCount-1)) & 0x01);
    result |= (~signbit + 1) << bitCount;

    *pResult = (drflac_int32)result;
    return DRFLAC_TRUE;
}

#ifdef DRFLAC_64BIT
static drflac_bool32 drflac__read_uint64(drflac_bs* bs, unsigned int bitCount, drflac_uint64* pResultOut)
{
    drflac_uint32 resultHi;
    drflac_uint32 resultLo;

    drflac_assert(bitCount <= 64);
    drflac_assert(bitCount >  32);

    if (!drflac__read_uint32(bs, bitCount - 32, &resultHi)) {
        return DRFLAC_FALSE;
    }

    if (!drflac__read_uint32(bs, 32, &resultLo)) {
        return DRFLAC_FALSE;
    }

    *pResultOut = (((drflac_uint64)resultHi) << 32) | ((drflac_uint64)resultLo);
    return DRFLAC_TRUE;
}
#endif

/* Function below is unused, but leaving it here in case I need to quickly add it again. */
#if 0
static drflac_bool32 drflac__read_int64(drflac_bs* bs, unsigned int bitCount, drflac_int64* pResultOut)
{
    drflac_uint64 result;
    drflac_uint64 signbit;

    drflac_assert(bitCount <= 64);

    if (!drflac__read_uint64(bs, bitCount, &result)) {
        return DRFLAC_FALSE;
    }

    signbit = ((result >> (bitCount-1)) & 0x01);
    result |= (~signbit + 1) << bitCount;

    *pResultOut = (drflac_int64)result;
    return DRFLAC_TRUE;
}
#endif

static drflac_bool32 drflac__read_uint16(drflac_bs* bs, unsigned int bitCount, drflac_uint16* pResult)
{
    drflac_uint32 result;

    drflac_assert(bs != NULL);
    drflac_assert(pResult != NULL);
    drflac_assert(bitCount > 0);
    drflac_assert(bitCount <= 16);

    if (!drflac__read_uint32(bs, bitCount, &result)) {
        return DRFLAC_FALSE;
    }

    *pResult = (drflac_uint16)result;
    return DRFLAC_TRUE;
}

#if 0
static drflac_bool32 drflac__read_int16(drflac_bs* bs, unsigned int bitCount, drflac_int16* pResult)
{
    drflac_int32 result;

    drflac_assert(bs != NULL);
    drflac_assert(pResult != NULL);
    drflac_assert(bitCount > 0);
    drflac_assert(bitCount <= 16);

    if (!drflac__read_int32(bs, bitCount, &result)) {
        return DRFLAC_FALSE;
    }

    *pResult = (drflac_int16)result;
    return DRFLAC_TRUE;
}
#endif

static drflac_bool32 drflac__read_uint8(drflac_bs* bs, unsigned int bitCount, drflac_uint8* pResult)
{
    drflac_uint32 result;

    drflac_assert(bs != NULL);
    drflac_assert(pResult != NULL);
    drflac_assert(bitCount > 0);
    drflac_assert(bitCount <= 8);

    if (!drflac__read_uint32(bs, bitCount, &result)) {
        return DRFLAC_FALSE;
    }

    *pResult = (drflac_uint8)result;
    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__read_int8(drflac_bs* bs, unsigned int bitCount, drflac_int8* pResult)
{
    drflac_int32 result;

    drflac_assert(bs != NULL);
    drflac_assert(pResult != NULL);
    drflac_assert(bitCount > 0);
    drflac_assert(bitCount <= 8);

    if (!drflac__read_int32(bs, bitCount, &result)) {
        return DRFLAC_FALSE;
    }

    *pResult = (drflac_int8)result;
    return DRFLAC_TRUE;
}


static drflac_bool32 drflac__seek_bits(drflac_bs* bs, size_t bitsToSeek)
{
    if (bitsToSeek <= DRFLAC_CACHE_L1_BITS_REMAINING(bs)) {
        bs->consumedBits += (drflac_uint32)bitsToSeek;
        bs->cache <<= bitsToSeek;
        return DRFLAC_TRUE;
    } else {
        /* It straddles the cached data. This function isn't called too frequently so I'm favouring simplicity here. */
        bitsToSeek       -= DRFLAC_CACHE_L1_BITS_REMAINING(bs);
        bs->consumedBits += DRFLAC_CACHE_L1_BITS_REMAINING(bs);
        bs->cache         = 0;

        /* Simple case. Seek in groups of the same number as bits that fit within a cache line. */
#ifdef DRFLAC_64BIT
        while (bitsToSeek >= DRFLAC_CACHE_L1_SIZE_BITS(bs)) {
            drflac_uint64 bin;
            if (!drflac__read_uint64(bs, DRFLAC_CACHE_L1_SIZE_BITS(bs), &bin)) {
                return DRFLAC_FALSE;
            }
            bitsToSeek -= DRFLAC_CACHE_L1_SIZE_BITS(bs);
        }
#else
        while (bitsToSeek >= DRFLAC_CACHE_L1_SIZE_BITS(bs)) {
            drflac_uint32 bin;
            if (!drflac__read_uint32(bs, DRFLAC_CACHE_L1_SIZE_BITS(bs), &bin)) {
                return DRFLAC_FALSE;
            }
            bitsToSeek -= DRFLAC_CACHE_L1_SIZE_BITS(bs);
        }
#endif

        /* Whole leftover bytes. */
        while (bitsToSeek >= 8) {
            drflac_uint8 bin;
            if (!drflac__read_uint8(bs, 8, &bin)) {
                return DRFLAC_FALSE;
            }
            bitsToSeek -= 8;
        }

        /* Leftover bits. */
        if (bitsToSeek > 0) {
            drflac_uint8 bin;
            if (!drflac__read_uint8(bs, (drflac_uint32)bitsToSeek, &bin)) {
                return DRFLAC_FALSE;
            }
            bitsToSeek = 0; /* <-- Necessary for the assert below. */
        }

        drflac_assert(bitsToSeek == 0);
        return DRFLAC_TRUE;
    }
}


/* This function moves the bit streamer to the first bit after the sync code (bit 15 of the of the frame header). It will also update the CRC-16. */
static drflac_bool32 drflac__find_and_seek_to_next_sync_code(drflac_bs* bs)
{
    drflac_assert(bs != NULL);

    /*
    The sync code is always aligned to 8 bits. This is convenient for us because it means we can do byte-aligned movements. The first
    thing to do is align to the next byte.
    */
    if (!drflac__seek_bits(bs, DRFLAC_CACHE_L1_BITS_REMAINING(bs) & 7)) {
        return DRFLAC_FALSE;
    }

    for (;;) {
        drflac_uint8 hi;

#ifndef DR_FLAC_NO_CRC
        drflac__reset_crc16(bs);
#endif

        if (!drflac__read_uint8(bs, 8, &hi)) {
            return DRFLAC_FALSE;
        }

        if (hi == 0xFF) {
            drflac_uint8 lo;
            if (!drflac__read_uint8(bs, 6, &lo)) {
                return DRFLAC_FALSE;
            }

            if (lo == 0x3E) {
                return DRFLAC_TRUE;
            } else {
                if (!drflac__seek_bits(bs, DRFLAC_CACHE_L1_BITS_REMAINING(bs) & 7)) {
                    return DRFLAC_FALSE;
                }
            }
        }
    }

    /* Should never get here. */
    /*return DRFLAC_FALSE;*/
}


#if !defined(DR_FLAC_NO_SIMD) && defined(DRFLAC_HAS_LZCNT_INTRINSIC)
#define DRFLAC_IMPLEMENT_CLZ_LZCNT
#endif
#if  defined(_MSC_VER) && _MSC_VER >= 1400 && (defined(DRFLAC_X64) || defined(DRFLAC_X86))
#define DRFLAC_IMPLEMENT_CLZ_MSVC
#endif

static DRFLAC_INLINE drflac_uint32 drflac__clz_software(drflac_cache_t x)
{
    drflac_uint32 n;
    static drflac_uint32 clz_table_4[] = {
        0,
        4,
        3, 3,
        2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1
    };

    if (x == 0) {
        return sizeof(x)*8;
    }

    n = clz_table_4[x >> (sizeof(x)*8 - 4)];
    if (n == 0) {
#ifdef DRFLAC_64BIT
        if ((x & ((drflac_uint64)0xFFFFFFFF << 32)) == 0) { n  = 32; x <<= 32; }
        if ((x & ((drflac_uint64)0xFFFF0000 << 32)) == 0) { n += 16; x <<= 16; }
        if ((x & ((drflac_uint64)0xFF000000 << 32)) == 0) { n += 8;  x <<= 8;  }
        if ((x & ((drflac_uint64)0xF0000000 << 32)) == 0) { n += 4;  x <<= 4;  }
#else
        if ((x & 0xFFFF0000) == 0) { n  = 16; x <<= 16; }
        if ((x & 0xFF000000) == 0) { n += 8;  x <<= 8;  }
        if ((x & 0xF0000000) == 0) { n += 4;  x <<= 4;  }
#endif
        n += clz_table_4[x >> (sizeof(x)*8 - 4)];
    }

    return n - 1;
}

#ifdef DRFLAC_IMPLEMENT_CLZ_LZCNT
static DRFLAC_INLINE drflac_bool32 drflac__is_lzcnt_supported()
{
    /* If the compiler itself does not support the intrinsic then we'll need to return false. */
#ifdef DRFLAC_HAS_LZCNT_INTRINSIC
    return drflac__gIsLZCNTSupported;
#else
    return DRFLAC_FALSE;
#endif
}

static DRFLAC_INLINE drflac_uint32 drflac__clz_lzcnt(drflac_cache_t x)
{
#if defined(_MSC_VER) && !defined(__clang__)
    #ifdef DRFLAC_64BIT
        return (drflac_uint32)__lzcnt64(x);
    #else
        return (drflac_uint32)__lzcnt(x);
    #endif
#else
    #if defined(__GNUC__) || defined(__clang__)
        if (x == 0) {
            return sizeof(x)*8;
        }
        #ifdef DRFLAC_64BIT
            return (drflac_uint32)__builtin_clzll((drflac_uint64)x);
        #else
            return (drflac_uint32)__builtin_clzl((drflac_uint32)x);
        #endif
    #else
        /* Unsupported compiler. */
        #error "This compiler does not support the lzcnt intrinsic."
    #endif
#endif
}
#endif

#ifdef DRFLAC_IMPLEMENT_CLZ_MSVC
#include <intrin.h> /* For BitScanReverse(). */

static DRFLAC_INLINE drflac_uint32 drflac__clz_msvc(drflac_cache_t x)
{
    drflac_uint32 n;

    if (x == 0) {
        return sizeof(x)*8;
    }

#ifdef DRFLAC_64BIT
    _BitScanReverse64((unsigned long*)&n, x);
#else
    _BitScanReverse((unsigned long*)&n, x);
#endif
    return sizeof(x)*8 - n - 1;
}
#endif

static DRFLAC_INLINE drflac_uint32 drflac__clz(drflac_cache_t x)
{
#ifdef DRFLAC_IMPLEMENT_CLZ_LZCNT
    if (drflac__is_lzcnt_supported()) {
        return drflac__clz_lzcnt(x);
    } else
#endif
    {
#ifdef DRFLAC_IMPLEMENT_CLZ_MSVC
        return drflac__clz_msvc(x);
#else
        return drflac__clz_software(x);
#endif
    }
}


static DRFLAC_INLINE drflac_bool32 drflac__seek_past_next_set_bit(drflac_bs* bs, unsigned int* pOffsetOut)
{
    drflac_uint32 zeroCounter = 0;
    drflac_uint32 setBitOffsetPlus1;

    while (bs->cache == 0) {
        zeroCounter += (drflac_uint32)DRFLAC_CACHE_L1_BITS_REMAINING(bs);
        if (!drflac__reload_cache(bs)) {
            return DRFLAC_FALSE;
        }
    }

    setBitOffsetPlus1 = drflac__clz(bs->cache);
    setBitOffsetPlus1 += 1;

    bs->consumedBits += setBitOffsetPlus1;
    bs->cache <<= setBitOffsetPlus1;

    *pOffsetOut = zeroCounter + setBitOffsetPlus1 - 1;
    return DRFLAC_TRUE;
}



static drflac_bool32 drflac__seek_to_byte(drflac_bs* bs, drflac_uint64 offsetFromStart)
{
    drflac_assert(bs != NULL);
    drflac_assert(offsetFromStart > 0);

    /*
    Seeking from the start is not quite as trivial as it sounds because the onSeek callback takes a signed 32-bit integer (which
    is intentional because it simplifies the implementation of the onSeek callbacks), however offsetFromStart is unsigned 64-bit.
    To resolve we just need to do an initial seek from the start, and then a series of offset seeks to make up the remainder.
    */
    if (offsetFromStart > 0x7FFFFFFF) {
        drflac_uint64 bytesRemaining = offsetFromStart;
        if (!bs->onSeek(bs->pUserData, 0x7FFFFFFF, drflac_seek_origin_start)) {
            return DRFLAC_FALSE;
        }
        bytesRemaining -= 0x7FFFFFFF;

        while (bytesRemaining > 0x7FFFFFFF) {
            if (!bs->onSeek(bs->pUserData, 0x7FFFFFFF, drflac_seek_origin_current)) {
                return DRFLAC_FALSE;
            }
            bytesRemaining -= 0x7FFFFFFF;
        }

        if (bytesRemaining > 0) {
            if (!bs->onSeek(bs->pUserData, (int)bytesRemaining, drflac_seek_origin_current)) {
                return DRFLAC_FALSE;
            }
        }
    } else {
        if (!bs->onSeek(bs->pUserData, (int)offsetFromStart, drflac_seek_origin_start)) {
            return DRFLAC_FALSE;
        }
    }

    /* The cache should be reset to force a reload of fresh data from the client. */
    drflac__reset_cache(bs);
    return DRFLAC_TRUE;
}


static drflac_result drflac__read_utf8_coded_number(drflac_bs* bs, drflac_uint64* pNumberOut, drflac_uint8* pCRCOut)
{
    drflac_uint8 crc;
    drflac_uint64 result;
    unsigned char utf8[7] = {0};
    int byteCount;
    int i;

    drflac_assert(bs != NULL);
    drflac_assert(pNumberOut != NULL);
    drflac_assert(pCRCOut != NULL);

    crc = *pCRCOut;

    if (!drflac__read_uint8(bs, 8, utf8)) {
        *pNumberOut = 0;
        return DRFLAC_END_OF_STREAM;
    }
    crc = drflac_crc8(crc, utf8[0], 8);

    if ((utf8[0] & 0x80) == 0) {
        *pNumberOut = utf8[0];
        *pCRCOut = crc;
        return DRFLAC_SUCCESS;
    }

    byteCount = 1;
    if ((utf8[0] & 0xE0) == 0xC0) {
        byteCount = 2;
    } else if ((utf8[0] & 0xF0) == 0xE0) {
        byteCount = 3;
    } else if ((utf8[0] & 0xF8) == 0xF0) {
        byteCount = 4;
    } else if ((utf8[0] & 0xFC) == 0xF8) {
        byteCount = 5;
    } else if ((utf8[0] & 0xFE) == 0xFC) {
        byteCount = 6;
    } else if ((utf8[0] & 0xFF) == 0xFE) {
        byteCount = 7;
    } else {
        *pNumberOut = 0;
        return DRFLAC_CRC_MISMATCH;     /* Bad UTF-8 encoding. */
    }

    /* Read extra bytes. */
    drflac_assert(byteCount > 1);

    result = (drflac_uint64)(utf8[0] & (0xFF >> (byteCount + 1)));
    for (i = 1; i < byteCount; ++i) {
        if (!drflac__read_uint8(bs, 8, utf8 + i)) {
            *pNumberOut = 0;
            return DRFLAC_END_OF_STREAM;
        }
        crc = drflac_crc8(crc, utf8[i], 8);

        result = (result << 6) | (utf8[i] & 0x3F);
    }

    *pNumberOut = result;
    *pCRCOut = crc;
    return DRFLAC_SUCCESS;
}



/*
The next two functions are responsible for calculating the prediction.

When the bits per sample is >16 we need to use 64-bit integer arithmetic because otherwise we'll run out of precision. It's
safe to assume this will be slower on 32-bit platforms so we use a more optimal solution when the bits per sample is <=16.
*/
static DRFLAC_INLINE drflac_int32 drflac__calculate_prediction_32(drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pDecodedSamples)
{
    drflac_int32 prediction = 0;

    drflac_assert(order <= 32);

    /* 32-bit version. */

    /* VC++ optimizes this to a single jmp. I've not yet verified this for other compilers. */
    switch (order)
    {
    case 32: prediction += coefficients[31] * pDecodedSamples[-32];
    case 31: prediction += coefficients[30] * pDecodedSamples[-31];
    case 30: prediction += coefficients[29] * pDecodedSamples[-30];
    case 29: prediction += coefficients[28] * pDecodedSamples[-29];
    case 28: prediction += coefficients[27] * pDecodedSamples[-28];
    case 27: prediction += coefficients[26] * pDecodedSamples[-27];
    case 26: prediction += coefficients[25] * pDecodedSamples[-26];
    case 25: prediction += coefficients[24] * pDecodedSamples[-25];
    case 24: prediction += coefficients[23] * pDecodedSamples[-24];
    case 23: prediction += coefficients[22] * pDecodedSamples[-23];
    case 22: prediction += coefficients[21] * pDecodedSamples[-22];
    case 21: prediction += coefficients[20] * pDecodedSamples[-21];
    case 20: prediction += coefficients[19] * pDecodedSamples[-20];
    case 19: prediction += coefficients[18] * pDecodedSamples[-19];
    case 18: prediction += coefficients[17] * pDecodedSamples[-18];
    case 17: prediction += coefficients[16] * pDecodedSamples[-17];
    case 16: prediction += coefficients[15] * pDecodedSamples[-16];
    case 15: prediction += coefficients[14] * pDecodedSamples[-15];
    case 14: prediction += coefficients[13] * pDecodedSamples[-14];
    case 13: prediction += coefficients[12] * pDecodedSamples[-13];
    case 12: prediction += coefficients[11] * pDecodedSamples[-12];
    case 11: prediction += coefficients[10] * pDecodedSamples[-11];
    case 10: prediction += coefficients[ 9] * pDecodedSamples[-10];
    case  9: prediction += coefficients[ 8] * pDecodedSamples[- 9];
    case  8: prediction += coefficients[ 7] * pDecodedSamples[- 8];
    case  7: prediction += coefficients[ 6] * pDecodedSamples[- 7];
    case  6: prediction += coefficients[ 5] * pDecodedSamples[- 6];
    case  5: prediction += coefficients[ 4] * pDecodedSamples[- 5];
    case  4: prediction += coefficients[ 3] * pDecodedSamples[- 4];
    case  3: prediction += coefficients[ 2] * pDecodedSamples[- 3];
    case  2: prediction += coefficients[ 1] * pDecodedSamples[- 2];
    case  1: prediction += coefficients[ 0] * pDecodedSamples[- 1];
    }

    return (drflac_int32)(prediction >> shift);
}

static DRFLAC_INLINE drflac_int32 drflac__calculate_prediction_64(drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pDecodedSamples)
{
    drflac_int64 prediction;

    drflac_assert(order <= 32);

    /* 64-bit version. */

    /* This method is faster on the 32-bit build when compiling with VC++. See note below. */
#ifndef DRFLAC_64BIT
    if (order == 8)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2] * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3] * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4] * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5] * (drflac_int64)pDecodedSamples[-6];
        prediction += coefficients[6] * (drflac_int64)pDecodedSamples[-7];
        prediction += coefficients[7] * (drflac_int64)pDecodedSamples[-8];
    }
    else if (order == 7)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2] * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3] * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4] * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5] * (drflac_int64)pDecodedSamples[-6];
        prediction += coefficients[6] * (drflac_int64)pDecodedSamples[-7];
    }
    else if (order == 3)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2] * (drflac_int64)pDecodedSamples[-3];
    }
    else if (order == 6)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2] * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3] * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4] * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5] * (drflac_int64)pDecodedSamples[-6];
    }
    else if (order == 5)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2] * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3] * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4] * (drflac_int64)pDecodedSamples[-5];
    }
    else if (order == 4)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2] * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3] * (drflac_int64)pDecodedSamples[-4];
    }
    else if (order == 12)
    {
        prediction  = coefficients[0]  * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1]  * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2]  * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3]  * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4]  * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5]  * (drflac_int64)pDecodedSamples[-6];
        prediction += coefficients[6]  * (drflac_int64)pDecodedSamples[-7];
        prediction += coefficients[7]  * (drflac_int64)pDecodedSamples[-8];
        prediction += coefficients[8]  * (drflac_int64)pDecodedSamples[-9];
        prediction += coefficients[9]  * (drflac_int64)pDecodedSamples[-10];
        prediction += coefficients[10] * (drflac_int64)pDecodedSamples[-11];
        prediction += coefficients[11] * (drflac_int64)pDecodedSamples[-12];
    }
    else if (order == 2)
    {
        prediction  = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1] * (drflac_int64)pDecodedSamples[-2];
    }
    else if (order == 1)
    {
        prediction = coefficients[0] * (drflac_int64)pDecodedSamples[-1];
    }
    else if (order == 10)
    {
        prediction  = coefficients[0]  * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1]  * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2]  * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3]  * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4]  * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5]  * (drflac_int64)pDecodedSamples[-6];
        prediction += coefficients[6]  * (drflac_int64)pDecodedSamples[-7];
        prediction += coefficients[7]  * (drflac_int64)pDecodedSamples[-8];
        prediction += coefficients[8]  * (drflac_int64)pDecodedSamples[-9];
        prediction += coefficients[9]  * (drflac_int64)pDecodedSamples[-10];
    }
    else if (order == 9)
    {
        prediction  = coefficients[0]  * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1]  * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2]  * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3]  * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4]  * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5]  * (drflac_int64)pDecodedSamples[-6];
        prediction += coefficients[6]  * (drflac_int64)pDecodedSamples[-7];
        prediction += coefficients[7]  * (drflac_int64)pDecodedSamples[-8];
        prediction += coefficients[8]  * (drflac_int64)pDecodedSamples[-9];
    }
    else if (order == 11)
    {
        prediction  = coefficients[0]  * (drflac_int64)pDecodedSamples[-1];
        prediction += coefficients[1]  * (drflac_int64)pDecodedSamples[-2];
        prediction += coefficients[2]  * (drflac_int64)pDecodedSamples[-3];
        prediction += coefficients[3]  * (drflac_int64)pDecodedSamples[-4];
        prediction += coefficients[4]  * (drflac_int64)pDecodedSamples[-5];
        prediction += coefficients[5]  * (drflac_int64)pDecodedSamples[-6];
        prediction += coefficients[6]  * (drflac_int64)pDecodedSamples[-7];
        prediction += coefficients[7]  * (drflac_int64)pDecodedSamples[-8];
        prediction += coefficients[8]  * (drflac_int64)pDecodedSamples[-9];
        prediction += coefficients[9]  * (drflac_int64)pDecodedSamples[-10];
        prediction += coefficients[10] * (drflac_int64)pDecodedSamples[-11];
    }
    else
    {
        int j;

        prediction = 0;
        for (j = 0; j < (int)order; ++j) {
            prediction += coefficients[j] * (drflac_int64)pDecodedSamples[-j-1];
        }
    }
#endif

    /*
    VC++ optimizes this to a single jmp instruction, but only the 64-bit build. The 32-bit build generates less efficient code for some
    reason. The ugly version above is faster so we'll just switch between the two depending on the target platform.
    */
#ifdef DRFLAC_64BIT
    prediction = 0;
    switch (order)
    {
    case 32: prediction += coefficients[31] * (drflac_int64)pDecodedSamples[-32];
    case 31: prediction += coefficients[30] * (drflac_int64)pDecodedSamples[-31];
    case 30: prediction += coefficients[29] * (drflac_int64)pDecodedSamples[-30];
    case 29: prediction += coefficients[28] * (drflac_int64)pDecodedSamples[-29];
    case 28: prediction += coefficients[27] * (drflac_int64)pDecodedSamples[-28];
    case 27: prediction += coefficients[26] * (drflac_int64)pDecodedSamples[-27];
    case 26: prediction += coefficients[25] * (drflac_int64)pDecodedSamples[-26];
    case 25: prediction += coefficients[24] * (drflac_int64)pDecodedSamples[-25];
    case 24: prediction += coefficients[23] * (drflac_int64)pDecodedSamples[-24];
    case 23: prediction += coefficients[22] * (drflac_int64)pDecodedSamples[-23];
    case 22: prediction += coefficients[21] * (drflac_int64)pDecodedSamples[-22];
    case 21: prediction += coefficients[20] * (drflac_int64)pDecodedSamples[-21];
    case 20: prediction += coefficients[19] * (drflac_int64)pDecodedSamples[-20];
    case 19: prediction += coefficients[18] * (drflac_int64)pDecodedSamples[-19];
    case 18: prediction += coefficients[17] * (drflac_int64)pDecodedSamples[-18];
    case 17: prediction += coefficients[16] * (drflac_int64)pDecodedSamples[-17];
    case 16: prediction += coefficients[15] * (drflac_int64)pDecodedSamples[-16];
    case 15: prediction += coefficients[14] * (drflac_int64)pDecodedSamples[-15];
    case 14: prediction += coefficients[13] * (drflac_int64)pDecodedSamples[-14];
    case 13: prediction += coefficients[12] * (drflac_int64)pDecodedSamples[-13];
    case 12: prediction += coefficients[11] * (drflac_int64)pDecodedSamples[-12];
    case 11: prediction += coefficients[10] * (drflac_int64)pDecodedSamples[-11];
    case 10: prediction += coefficients[ 9] * (drflac_int64)pDecodedSamples[-10];
    case  9: prediction += coefficients[ 8] * (drflac_int64)pDecodedSamples[- 9];
    case  8: prediction += coefficients[ 7] * (drflac_int64)pDecodedSamples[- 8];
    case  7: prediction += coefficients[ 6] * (drflac_int64)pDecodedSamples[- 7];
    case  6: prediction += coefficients[ 5] * (drflac_int64)pDecodedSamples[- 6];
    case  5: prediction += coefficients[ 4] * (drflac_int64)pDecodedSamples[- 5];
    case  4: prediction += coefficients[ 3] * (drflac_int64)pDecodedSamples[- 4];
    case  3: prediction += coefficients[ 2] * (drflac_int64)pDecodedSamples[- 3];
    case  2: prediction += coefficients[ 1] * (drflac_int64)pDecodedSamples[- 2];
    case  1: prediction += coefficients[ 0] * (drflac_int64)pDecodedSamples[- 1];
    }
#endif

    return (drflac_int32)(prediction >> shift);
}

static DRFLAC_INLINE void drflac__calculate_prediction_64_x4(drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, const drflac_uint32 riceParamParts[4], drflac_int32* pDecodedSamples)
{
    drflac_int64 prediction0 = 0;
    drflac_int64 prediction1 = 0;
    drflac_int64 prediction2 = 0;
    drflac_int64 prediction3 = 0;

    drflac_assert(order <= 32);

    switch (order)
    {
    case 32:
        prediction0 += coefficients[31] * (drflac_int64)pDecodedSamples[-32];
        prediction1 += coefficients[31] * (drflac_int64)pDecodedSamples[-31];
        prediction2 += coefficients[31] * (drflac_int64)pDecodedSamples[-30];
        prediction3 += coefficients[31] * (drflac_int64)pDecodedSamples[-29];
    case 31:
        prediction0 += coefficients[30] * (drflac_int64)pDecodedSamples[-31];
        prediction1 += coefficients[30] * (drflac_int64)pDecodedSamples[-30];
        prediction2 += coefficients[30] * (drflac_int64)pDecodedSamples[-29];
        prediction3 += coefficients[30] * (drflac_int64)pDecodedSamples[-28];
    case 30:
        prediction0 += coefficients[29] * (drflac_int64)pDecodedSamples[-30];
        prediction1 += coefficients[29] * (drflac_int64)pDecodedSamples[-29];
        prediction2 += coefficients[29] * (drflac_int64)pDecodedSamples[-28];
        prediction3 += coefficients[29] * (drflac_int64)pDecodedSamples[-27];
    case 29:
        prediction0 += coefficients[28] * (drflac_int64)pDecodedSamples[-29];
        prediction1 += coefficients[28] * (drflac_int64)pDecodedSamples[-28];
        prediction2 += coefficients[28] * (drflac_int64)pDecodedSamples[-27];
        prediction3 += coefficients[28] * (drflac_int64)pDecodedSamples[-26];
    case 28:
        prediction0 += coefficients[27] * (drflac_int64)pDecodedSamples[-28];
        prediction1 += coefficients[27] * (drflac_int64)pDecodedSamples[-27];
        prediction2 += coefficients[27] * (drflac_int64)pDecodedSamples[-26];
        prediction3 += coefficients[27] * (drflac_int64)pDecodedSamples[-25];
    case 27:
        prediction0 += coefficients[26] * (drflac_int64)pDecodedSamples[-27];
        prediction1 += coefficients[26] * (drflac_int64)pDecodedSamples[-26];
        prediction2 += coefficients[26] * (drflac_int64)pDecodedSamples[-25];
        prediction3 += coefficients[26] * (drflac_int64)pDecodedSamples[-24];
    case 26:
        prediction0 += coefficients[25] * (drflac_int64)pDecodedSamples[-26];
        prediction1 += coefficients[25] * (drflac_int64)pDecodedSamples[-25];
        prediction2 += coefficients[25] * (drflac_int64)pDecodedSamples[-24];
        prediction3 += coefficients[25] * (drflac_int64)pDecodedSamples[-23];
    case 25:
        prediction0 += coefficients[24] * (drflac_int64)pDecodedSamples[-25];
        prediction1 += coefficients[24] * (drflac_int64)pDecodedSamples[-24];
        prediction2 += coefficients[24] * (drflac_int64)pDecodedSamples[-23];
        prediction3 += coefficients[24] * (drflac_int64)pDecodedSamples[-22];
    case 24:
        prediction0 += coefficients[23] * (drflac_int64)pDecodedSamples[-24];
        prediction1 += coefficients[23] * (drflac_int64)pDecodedSamples[-23];
        prediction2 += coefficients[23] * (drflac_int64)pDecodedSamples[-22];
        prediction3 += coefficients[23] * (drflac_int64)pDecodedSamples[-21];
    case 23:
        prediction0 += coefficients[22] * (drflac_int64)pDecodedSamples[-23];
        prediction1 += coefficients[22] * (drflac_int64)pDecodedSamples[-22];
        prediction2 += coefficients[22] * (drflac_int64)pDecodedSamples[-21];
        prediction3 += coefficients[22] * (drflac_int64)pDecodedSamples[-20];
    case 22:
        prediction0 += coefficients[21] * (drflac_int64)pDecodedSamples[-22];
        prediction1 += coefficients[21] * (drflac_int64)pDecodedSamples[-21];
        prediction2 += coefficients[21] * (drflac_int64)pDecodedSamples[-20];
        prediction3 += coefficients[21] * (drflac_int64)pDecodedSamples[-19];
    case 21:
        prediction0 += coefficients[20] * (drflac_int64)pDecodedSamples[-21];
        prediction1 += coefficients[20] * (drflac_int64)pDecodedSamples[-20];
        prediction2 += coefficients[20] * (drflac_int64)pDecodedSamples[-19];
        prediction3 += coefficients[20] * (drflac_int64)pDecodedSamples[-18];
    case 20:
        prediction0 += coefficients[19] * (drflac_int64)pDecodedSamples[-20];
        prediction1 += coefficients[19] * (drflac_int64)pDecodedSamples[-19];
        prediction2 += coefficients[19] * (drflac_int64)pDecodedSamples[-18];
        prediction3 += coefficients[19] * (drflac_int64)pDecodedSamples[-17];
    case 19:
        prediction0 += coefficients[18] * (drflac_int64)pDecodedSamples[-19];
        prediction1 += coefficients[18] * (drflac_int64)pDecodedSamples[-18];
        prediction2 += coefficients[18] * (drflac_int64)pDecodedSamples[-17];
        prediction3 += coefficients[18] * (drflac_int64)pDecodedSamples[-16];
    case 18:
        prediction0 += coefficients[17] * (drflac_int64)pDecodedSamples[-18];
        prediction1 += coefficients[17] * (drflac_int64)pDecodedSamples[-17];
        prediction2 += coefficients[17] * (drflac_int64)pDecodedSamples[-16];
        prediction3 += coefficients[17] * (drflac_int64)pDecodedSamples[-15];
    case 17:
        prediction0 += coefficients[16] * (drflac_int64)pDecodedSamples[-17];
        prediction1 += coefficients[16] * (drflac_int64)pDecodedSamples[-16];
        prediction2 += coefficients[16] * (drflac_int64)pDecodedSamples[-15];
        prediction3 += coefficients[16] * (drflac_int64)pDecodedSamples[-14];

    case 16:
        prediction0 += coefficients[15] * (drflac_int64)pDecodedSamples[-16];
        prediction1 += coefficients[15] * (drflac_int64)pDecodedSamples[-15];
        prediction2 += coefficients[15] * (drflac_int64)pDecodedSamples[-14];
        prediction3 += coefficients[15] * (drflac_int64)pDecodedSamples[-13];
    case 15:
        prediction0 += coefficients[14] * (drflac_int64)pDecodedSamples[-15];
        prediction1 += coefficients[14] * (drflac_int64)pDecodedSamples[-14];
        prediction2 += coefficients[14] * (drflac_int64)pDecodedSamples[-13];
        prediction3 += coefficients[14] * (drflac_int64)pDecodedSamples[-12];
    case 14:
        prediction0 += coefficients[13] * (drflac_int64)pDecodedSamples[-14];
        prediction1 += coefficients[13] * (drflac_int64)pDecodedSamples[-13];
        prediction2 += coefficients[13] * (drflac_int64)pDecodedSamples[-12];
        prediction3 += coefficients[13] * (drflac_int64)pDecodedSamples[-11];
    case 13:
        prediction0 += coefficients[12] * (drflac_int64)pDecodedSamples[-13];
        prediction1 += coefficients[12] * (drflac_int64)pDecodedSamples[-12];
        prediction2 += coefficients[12] * (drflac_int64)pDecodedSamples[-11];
        prediction3 += coefficients[12] * (drflac_int64)pDecodedSamples[-10];
    case 12:
        prediction0 += coefficients[11] * (drflac_int64)pDecodedSamples[-12];
        prediction1 += coefficients[11] * (drflac_int64)pDecodedSamples[-11];
        prediction2 += coefficients[11] * (drflac_int64)pDecodedSamples[-10];
        prediction3 += coefficients[11] * (drflac_int64)pDecodedSamples[- 9];
    case 11:
        prediction0 += coefficients[10] * (drflac_int64)pDecodedSamples[-11];
        prediction1 += coefficients[10] * (drflac_int64)pDecodedSamples[-10];
        prediction2 += coefficients[10] * (drflac_int64)pDecodedSamples[- 9];
        prediction3 += coefficients[10] * (drflac_int64)pDecodedSamples[- 8];
    case 10:
        prediction0 += coefficients[9] * (drflac_int64)pDecodedSamples[-10];
        prediction1 += coefficients[9] * (drflac_int64)pDecodedSamples[- 9];
        prediction2 += coefficients[9] * (drflac_int64)pDecodedSamples[- 8];
        prediction3 += coefficients[9] * (drflac_int64)pDecodedSamples[- 7];
    case  9:
        prediction0 += coefficients[8] * (drflac_int64)pDecodedSamples[- 9];
        prediction1 += coefficients[8] * (drflac_int64)pDecodedSamples[- 8];
        prediction2 += coefficients[8] * (drflac_int64)pDecodedSamples[- 7];
        prediction3 += coefficients[8] * (drflac_int64)pDecodedSamples[- 6];
    case  8:
        prediction0 += coefficients[7] * (drflac_int64)pDecodedSamples[- 8];
        prediction1 += coefficients[7] * (drflac_int64)pDecodedSamples[- 7];
        prediction2 += coefficients[7] * (drflac_int64)pDecodedSamples[- 6];
        prediction3 += coefficients[7] * (drflac_int64)pDecodedSamples[- 5];
    case  7:
        prediction0 += coefficients[6] * (drflac_int64)pDecodedSamples[- 7];
        prediction1 += coefficients[6] * (drflac_int64)pDecodedSamples[- 6];
        prediction2 += coefficients[6] * (drflac_int64)pDecodedSamples[- 5];
        prediction3 += coefficients[6] * (drflac_int64)pDecodedSamples[- 4];
    case  6:
        prediction0 += coefficients[5] * (drflac_int64)pDecodedSamples[- 6];
        prediction1 += coefficients[5] * (drflac_int64)pDecodedSamples[- 5];
        prediction2 += coefficients[5] * (drflac_int64)pDecodedSamples[- 4];
        prediction3 += coefficients[5] * (drflac_int64)pDecodedSamples[- 3];
    case  5:
        prediction0 += coefficients[4] * (drflac_int64)pDecodedSamples[- 5];
        prediction1 += coefficients[4] * (drflac_int64)pDecodedSamples[- 4];
        prediction2 += coefficients[4] * (drflac_int64)pDecodedSamples[- 3];
        prediction3 += coefficients[4] * (drflac_int64)pDecodedSamples[- 2];
    case  4:
        prediction0 += coefficients[3] * (drflac_int64)pDecodedSamples[- 4];
        prediction1 += coefficients[3] * (drflac_int64)pDecodedSamples[- 3];
        prediction2 += coefficients[3] * (drflac_int64)pDecodedSamples[- 2];
        prediction3 += coefficients[3] * (drflac_int64)pDecodedSamples[- 1];
        order = 3;
    }

    switch (order)
    {
    case 3: prediction0 += coefficients[ 2] * (drflac_int64)pDecodedSamples[- 3];
    case 2: prediction0 += coefficients[ 1] * (drflac_int64)pDecodedSamples[- 2];
    case 1: prediction0 += coefficients[ 0] * (drflac_int64)pDecodedSamples[- 1];
    }
    pDecodedSamples[0] = riceParamParts[0] + (drflac_int32)(prediction0 >> shift);

    switch (order)
    {
    case 3: prediction1 += coefficients[ 2] * (drflac_int64)pDecodedSamples[- 2];
    case 2: prediction1 += coefficients[ 1] * (drflac_int64)pDecodedSamples[- 1];
    case 1: prediction1 += coefficients[ 0] * (drflac_int64)pDecodedSamples[  0];
    }
    pDecodedSamples[1] = riceParamParts[1] + (drflac_int32)(prediction1 >> shift);

    switch (order)
    {
    case 3: prediction2 += coefficients[ 2] * (drflac_int64)pDecodedSamples[- 1];
    case 2: prediction2 += coefficients[ 1] * (drflac_int64)pDecodedSamples[  0];
    case 1: prediction2 += coefficients[ 0] * (drflac_int64)pDecodedSamples[  1];
    }
    pDecodedSamples[2] = riceParamParts[2] + (drflac_int32)(prediction2 >> shift);

    switch (order)
    {
    case 3: prediction3 += coefficients[ 2] * (drflac_int64)pDecodedSamples[  0];
    case 2: prediction3 += coefficients[ 1] * (drflac_int64)pDecodedSamples[  1];
    case 1: prediction3 += coefficients[ 0] * (drflac_int64)pDecodedSamples[  2];
    }
    pDecodedSamples[3] = riceParamParts[3] + (drflac_int32)(prediction3 >> shift);
}

#if defined(DRFLAC_SUPPORT_SSE41)
static DRFLAC_INLINE drflac_int32 drflac__calculate_prediction_64__sse41(drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pDecodedSamples)
{
    __m128i prediction = _mm_setzero_si128();

    drflac_assert(order <= 32);

    switch (order)
    {
    case 32:
    case 31: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[31], 0, coefficients[30]), _mm_set_epi32(0, pDecodedSamples[-32], 0, pDecodedSamples[-31])));
    case 30:
    case 29: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[29], 0, coefficients[28]), _mm_set_epi32(0, pDecodedSamples[-30], 0, pDecodedSamples[-29])));
    case 28:
    case 27: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[27], 0, coefficients[26]), _mm_set_epi32(0, pDecodedSamples[-28], 0, pDecodedSamples[-27])));
    case 26:
    case 25: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[25], 0, coefficients[24]), _mm_set_epi32(0, pDecodedSamples[-26], 0, pDecodedSamples[-25])));
    case 24:
    case 23: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[23], 0, coefficients[22]), _mm_set_epi32(0, pDecodedSamples[-24], 0, pDecodedSamples[-23])));
    case 22:
    case 21: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[21], 0, coefficients[20]), _mm_set_epi32(0, pDecodedSamples[-22], 0, pDecodedSamples[-21])));
    case 20:
    case 19: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[19], 0, coefficients[18]), _mm_set_epi32(0, pDecodedSamples[-20], 0, pDecodedSamples[-19])));
    case 18:
    case 17: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[17], 0, coefficients[16]), _mm_set_epi32(0, pDecodedSamples[-18], 0, pDecodedSamples[-17])));
    case 16:
    case 15: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[15], 0, coefficients[14]), _mm_set_epi32(0, pDecodedSamples[-16], 0, pDecodedSamples[-15])));
    case 14:
    case 13: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[13], 0, coefficients[12]), _mm_set_epi32(0, pDecodedSamples[-14], 0, pDecodedSamples[-13])));
    case 12:
    case 11: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[11], 0, coefficients[10]), _mm_set_epi32(0, pDecodedSamples[-12], 0, pDecodedSamples[-11])));
    case 10:
    case  9: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 9], 0, coefficients[ 8]), _mm_set_epi32(0, pDecodedSamples[-10], 0, pDecodedSamples[- 9])));
    case  8:
    case  7: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 7], 0, coefficients[ 6]), _mm_set_epi32(0, pDecodedSamples[- 8], 0, pDecodedSamples[- 7])));
    case  6:
    case  5: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 5], 0, coefficients[ 4]), _mm_set_epi32(0, pDecodedSamples[- 6], 0, pDecodedSamples[- 5])));
    case  4:
    case  3: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 3], 0, coefficients[ 2]), _mm_set_epi32(0, pDecodedSamples[- 4], 0, pDecodedSamples[- 3])));
    case  2:
    case  1: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 1], 0, coefficients[ 0]), _mm_set_epi32(0, pDecodedSamples[- 2], 0, pDecodedSamples[- 1])));
    }

    return (drflac_int32)((
        ((drflac_uint64*)&prediction)[0] +
        ((drflac_uint64*)&prediction)[1]) >> shift);
}

static DRFLAC_INLINE void drflac__calculate_prediction_64_x2__sse41(drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, const drflac_uint32 riceParamParts[4], drflac_int32* pDecodedSamples)
{
    __m128i prediction = _mm_setzero_si128();
    drflac_int64 predictions[2] = {0, 0};

    drflac_assert(order <= 32);

    switch (order)
    {
    case 32: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[31], 0, coefficients[31]), _mm_set_epi32(0, pDecodedSamples[-31], 0, pDecodedSamples[-32])));
    case 31: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[30], 0, coefficients[30]), _mm_set_epi32(0, pDecodedSamples[-30], 0, pDecodedSamples[-31])));
    case 30: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[29], 0, coefficients[29]), _mm_set_epi32(0, pDecodedSamples[-29], 0, pDecodedSamples[-30])));
    case 29: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[28], 0, coefficients[28]), _mm_set_epi32(0, pDecodedSamples[-28], 0, pDecodedSamples[-29])));
    case 28: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[27], 0, coefficients[27]), _mm_set_epi32(0, pDecodedSamples[-27], 0, pDecodedSamples[-28])));
    case 27: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[26], 0, coefficients[26]), _mm_set_epi32(0, pDecodedSamples[-26], 0, pDecodedSamples[-27])));
    case 26: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[25], 0, coefficients[25]), _mm_set_epi32(0, pDecodedSamples[-25], 0, pDecodedSamples[-26])));
    case 25: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[24], 0, coefficients[24]), _mm_set_epi32(0, pDecodedSamples[-24], 0, pDecodedSamples[-25])));
    case 24: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[23], 0, coefficients[23]), _mm_set_epi32(0, pDecodedSamples[-23], 0, pDecodedSamples[-24])));
    case 23: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[22], 0, coefficients[22]), _mm_set_epi32(0, pDecodedSamples[-22], 0, pDecodedSamples[-23])));
    case 22: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[21], 0, coefficients[21]), _mm_set_epi32(0, pDecodedSamples[-21], 0, pDecodedSamples[-22])));
    case 21: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[20], 0, coefficients[20]), _mm_set_epi32(0, pDecodedSamples[-20], 0, pDecodedSamples[-21])));
    case 20: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[19], 0, coefficients[19]), _mm_set_epi32(0, pDecodedSamples[-19], 0, pDecodedSamples[-20])));
    case 19: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[18], 0, coefficients[18]), _mm_set_epi32(0, pDecodedSamples[-18], 0, pDecodedSamples[-19])));
    case 18: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[17], 0, coefficients[17]), _mm_set_epi32(0, pDecodedSamples[-17], 0, pDecodedSamples[-18])));
    case 17: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[16], 0, coefficients[16]), _mm_set_epi32(0, pDecodedSamples[-16], 0, pDecodedSamples[-17])));
    case 16: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[15], 0, coefficients[15]), _mm_set_epi32(0, pDecodedSamples[-15], 0, pDecodedSamples[-16])));
    case 15: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[14], 0, coefficients[14]), _mm_set_epi32(0, pDecodedSamples[-14], 0, pDecodedSamples[-15])));
    case 14: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[13], 0, coefficients[13]), _mm_set_epi32(0, pDecodedSamples[-13], 0, pDecodedSamples[-14])));
    case 13: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[12], 0, coefficients[12]), _mm_set_epi32(0, pDecodedSamples[-12], 0, pDecodedSamples[-13])));
    case 12: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[11], 0, coefficients[11]), _mm_set_epi32(0, pDecodedSamples[-11], 0, pDecodedSamples[-12])));
    case 11: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[10], 0, coefficients[10]), _mm_set_epi32(0, pDecodedSamples[-10], 0, pDecodedSamples[-11])));
    case 10: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 9], 0, coefficients[ 9]), _mm_set_epi32(0, pDecodedSamples[- 9], 0, pDecodedSamples[-10])));
    case  9: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 8], 0, coefficients[ 8]), _mm_set_epi32(0, pDecodedSamples[- 8], 0, pDecodedSamples[- 9])));
    case  8: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 7], 0, coefficients[ 7]), _mm_set_epi32(0, pDecodedSamples[- 7], 0, pDecodedSamples[- 8])));
    case  7: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 6], 0, coefficients[ 6]), _mm_set_epi32(0, pDecodedSamples[- 6], 0, pDecodedSamples[- 7])));
    case  6: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 5], 0, coefficients[ 5]), _mm_set_epi32(0, pDecodedSamples[- 5], 0, pDecodedSamples[- 6])));
    case  5: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 4], 0, coefficients[ 4]), _mm_set_epi32(0, pDecodedSamples[- 4], 0, pDecodedSamples[- 5])));
    case  4: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 3], 0, coefficients[ 3]), _mm_set_epi32(0, pDecodedSamples[- 3], 0, pDecodedSamples[- 4])));
    case  3: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 2], 0, coefficients[ 2]), _mm_set_epi32(0, pDecodedSamples[- 2], 0, pDecodedSamples[- 3])));
    case  2: prediction = _mm_add_epi64(prediction, _mm_mul_epi32(_mm_set_epi32(0, coefficients[ 1], 0, coefficients[ 1]), _mm_set_epi32(0, pDecodedSamples[- 1], 0, pDecodedSamples[- 2])));
        order = 1;
    }

    _mm_storeu_si128((__m128i*)predictions, prediction);

    switch (order)
    {
    case 1: predictions[0] += coefficients[ 0] * (drflac_int64)pDecodedSamples[- 1];
    }
    pDecodedSamples[0] = riceParamParts[0] + (drflac_int32)(predictions[0] >> shift);

    switch (order)
    {
    case 1: predictions[1] += coefficients[ 0] * (drflac_int64)pDecodedSamples[  0];
    }
    pDecodedSamples[1] = riceParamParts[1] + (drflac_int32)(predictions[1] >> shift);
}


static DRFLAC_INLINE __m128i drflac__mm_not_si128(__m128i a)
{
    return _mm_xor_si128(a, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
}

static DRFLAC_INLINE __m128i drflac__mm_slide1_epi32(__m128i a, __m128i b)
{
    /* a3a2a1a0/b3b2b1b0 -> a2a1a0b3 */

    /* Result = a2a1a0b3 */
    __m128i b3a3b2a2 = _mm_unpackhi_epi32(a, b);
    __m128i a2b3a2b3 = _mm_shuffle_epi32(b3a3b2a2, _MM_SHUFFLE(0, 3, 0, 3));
    __m128i a1a2a0b3 = _mm_unpacklo_epi32(a2b3a2b3, a);
    __m128i a2a1a0b3 = _mm_shuffle_epi32(a1a2a0b3, _MM_SHUFFLE(2, 3, 1, 0));
    return a2a1a0b3;
}

static DRFLAC_INLINE __m128i drflac__mm_slide2_epi32(__m128i a, __m128i b)
{
    /* Result = a1a0b3b2 */
    __m128i b1b0b3b2 = _mm_shuffle_epi32(b, _MM_SHUFFLE(1, 0, 3, 2));
    __m128i a1b3a0b2 = _mm_unpacklo_epi32(b1b0b3b2, a);
    __m128i a1a0b3b2 = _mm_shuffle_epi32(a1b3a0b2, _MM_SHUFFLE(3, 1, 2, 0));
    return a1a0b3b2;
}

static DRFLAC_INLINE __m128i drflac__mm_slide3_epi32(__m128i a, __m128i b)
{
    /* Result = a0b3b2b1 */
    __m128i b1a1b0a0 = _mm_unpacklo_epi32(a, b);
    __m128i a0b1a0b1 = _mm_shuffle_epi32(b1a1b0a0, _MM_SHUFFLE(0, 3, 0, 3));
    __m128i b3a0b2b1 = _mm_unpackhi_epi32(a0b1a0b1, b);
    __m128i a0b3b2b1 = _mm_shuffle_epi32(b3a0b2b1, _MM_SHUFFLE(2, 3, 1, 0));
    return a0b3b2b1;
}

static DRFLAC_INLINE void drflac__calculate_prediction_32_x4__sse41(drflac_uint32 order, drflac_int32 shift, const __m128i* coefficients128, const __m128i riceParamParts128, drflac_int32* pDecodedSamples)
{
    drflac_assert(order <= 32);

    /* I don't think this is as efficient as it could be. More work needs to be done on this. */
    if (order > 0) {
        drflac_int32 predictions[4];
        drflac_uint32 riceParamParts[4];

        __m128i s_09_10_11_12 = _mm_loadu_si128((const __m128i*)(pDecodedSamples - 12));
        __m128i s_05_06_07_08 = _mm_loadu_si128((const __m128i*)(pDecodedSamples -  8));
        __m128i s_01_02_03_04 = _mm_loadu_si128((const __m128i*)(pDecodedSamples -  4));

        __m128i prediction = _mm_setzero_si128();

        /*
        The idea with this switch is to do do a single jump based on the value of "order". In my test library, "order" is never larger than 12, so
        I have decided to do a less optimal, but simpler solution in the order > 12 case.
        */
        switch (order)
        {
        case 32: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[31], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 32))));
        case 31: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[30], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 31))));
        case 30: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[29], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 30))));
        case 29: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[28], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 29))));
        case 28: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[27], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 28))));
        case 27: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[26], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 27))));
        case 26: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[25], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 26))));
        case 25: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[24], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 25))));
        case 24: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[23], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 24))));
        case 23: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[22], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 23))));
        case 22: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[21], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 22))));
        case 21: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[20], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 21))));
        case 20: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[19], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 20))));
        case 19: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[18], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 19))));
        case 18: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[17], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 18))));
        case 17: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[16], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 17))));
        case 16: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[15], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 16))));
        case 15: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[14], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 15))));
        case 14: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[13], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 14))));
        case 13: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[12], _mm_loadu_si128((const __m128i*)(pDecodedSamples - 13))));

        case 12: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[11], s_09_10_11_12));
        case 11: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[10], drflac__mm_slide3_epi32(s_05_06_07_08, s_09_10_11_12)));
        case 10: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 9], drflac__mm_slide2_epi32(s_05_06_07_08, s_09_10_11_12)));
        case  9: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 8], drflac__mm_slide1_epi32(s_05_06_07_08, s_09_10_11_12)));
        case  8: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 7], s_05_06_07_08));
        case  7: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 6], drflac__mm_slide3_epi32(s_01_02_03_04, s_05_06_07_08)));
        case  6: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 5], drflac__mm_slide2_epi32(s_01_02_03_04, s_05_06_07_08)));
        case  5: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 4], drflac__mm_slide1_epi32(s_01_02_03_04, s_05_06_07_08)));
        case  4: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 3], s_01_02_03_04)); order = 3;    /* <-- Don't forget to set order to 3 here! */
        case  3: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 2], drflac__mm_slide3_epi32(_mm_setzero_si128(), s_01_02_03_04)));
        case  2: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 1], drflac__mm_slide2_epi32(_mm_setzero_si128(), s_01_02_03_04)));
        case  1: prediction = _mm_add_epi32(prediction, _mm_mullo_epi32(coefficients128[ 0], drflac__mm_slide1_epi32(_mm_setzero_si128(), s_01_02_03_04)));
        }

        _mm_storeu_si128((__m128i*)predictions, prediction);
        _mm_storeu_si128((__m128i*)riceParamParts, riceParamParts128);

        predictions[0] = riceParamParts[0] + (predictions[0] >> shift);

        switch (order)
        {
        case 3: predictions[3] += ((const drflac_int32*)&coefficients128[ 2])[0] * predictions[  0];
        case 2: predictions[2] += ((const drflac_int32*)&coefficients128[ 1])[0] * predictions[  0];
        case 1: predictions[1] += ((const drflac_int32*)&coefficients128[ 0])[0] * predictions[  0];
        }
        predictions[1] = riceParamParts[1] + (predictions[1] >> shift);

        switch (order)
        {
        case 3:
        case 2: predictions[3] += ((const drflac_int32*)&coefficients128[ 1])[0] * predictions[  1];
        case 1: predictions[2] += ((const drflac_int32*)&coefficients128[ 0])[0] * predictions[  1];
        }
        predictions[2] = riceParamParts[2] + (predictions[2] >> shift);

        switch (order)
        {
        case 3:
        case 2:
        case 1: predictions[3] += ((const drflac_int32*)&coefficients128[ 0])[0] * predictions[  2];
        }
        predictions[3] = riceParamParts[3] + (predictions[3] >> shift);

        pDecodedSamples[0] = predictions[0];
        pDecodedSamples[1] = predictions[1];
        pDecodedSamples[2] = predictions[2];
        pDecodedSamples[3] = predictions[3];
    } else {
        _mm_storeu_si128((__m128i*)pDecodedSamples, riceParamParts128);
    }
}
#endif

#if 0
/*
Reference implementation for reading and decoding samples with residual. This is intentionally left unoptimized for the
sake of readability and should only be used as a reference.
*/
static drflac_bool32 drflac__decode_samples_with_residual__rice__reference(drflac_bs* bs, drflac_uint32 bitsPerSample, drflac_uint32 count, drflac_uint8 riceParam, drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pSamplesOut)
{
    drflac_uint32 i;

    drflac_assert(bs != NULL);
    drflac_assert(count > 0);
    drflac_assert(pSamplesOut != NULL);

    for (i = 0; i < count; ++i) {
        drflac_uint32 zeroCounter = 0;
        for (;;) {
            drflac_uint8 bit;
            if (!drflac__read_uint8(bs, 1, &bit)) {
                return DRFLAC_FALSE;
            }

            if (bit == 0) {
                zeroCounter += 1;
            } else {
                break;
            }
        }

        drflac_uint32 decodedRice;
        if (riceParam > 0) {
            if (!drflac__read_uint32(bs, riceParam, &decodedRice)) {
                return DRFLAC_FALSE;
            }
        } else {
            decodedRice = 0;
        }

        decodedRice |= (zeroCounter << riceParam);
        if ((decodedRice & 0x01)) {
            decodedRice = ~(decodedRice >> 1);
        } else {
            decodedRice =  (decodedRice >> 1);
        }


        if (bitsPerSample > 16) {
            pSamplesOut[i] = decodedRice + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + i);
        } else {
            pSamplesOut[i] = decodedRice + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + i);
        }
    }

    return DRFLAC_TRUE;
}
#endif

#if 0
static drflac_bool32 drflac__read_rice_parts__reference(drflac_bs* bs, drflac_uint8 riceParam, drflac_uint32* pZeroCounterOut, drflac_uint32* pRiceParamPartOut)
{
    drflac_uint32 zeroCounter = 0;
    drflac_uint32 decodedRice;

    for (;;) {
        drflac_uint8 bit;
        if (!drflac__read_uint8(bs, 1, &bit)) {
            return DRFLAC_FALSE;
        }

        if (bit == 0) {
            zeroCounter += 1;
        } else {
            break;
        }
    }

    if (riceParam > 0) {
        if (!drflac__read_uint32(bs, riceParam, &decodedRice)) {
            return DRFLAC_FALSE;
        }
    } else {
        decodedRice = 0;
    }

    *pZeroCounterOut = zeroCounter;
    *pRiceParamPartOut = decodedRice;
    return DRFLAC_TRUE;
}
#endif

#if 0
static DRFLAC_INLINE drflac_bool32 drflac__read_rice_parts(drflac_bs* bs, drflac_uint8 riceParam, drflac_uint32* pZeroCounterOut, drflac_uint32* pRiceParamPartOut)
{
    drflac_cache_t riceParamMask;
    drflac_uint32 zeroCounter;
    drflac_uint32 setBitOffsetPlus1;
    drflac_uint32 riceParamPart;
    drflac_uint32 riceLength;

    drflac_assert(riceParam > 0);   /* <-- riceParam should never be 0. drflac__read_rice_parts__param_equals_zero() should be used instead for this case. */

    riceParamMask = DRFLAC_CACHE_L1_SELECTION_MASK(riceParam);

    zeroCounter = 0;
    while (bs->cache == 0) {
        zeroCounter += (drflac_uint32)DRFLAC_CACHE_L1_BITS_REMAINING(bs);
        if (!drflac__reload_cache(bs)) {
            return DRFLAC_FALSE;
        }
    }

    setBitOffsetPlus1 = drflac__clz(bs->cache);
    zeroCounter += setBitOffsetPlus1;
    setBitOffsetPlus1 += 1;

    riceLength = setBitOffsetPlus1 + riceParam;
    if (riceLength < DRFLAC_CACHE_L1_BITS_REMAINING(bs)) {
        riceParamPart = (drflac_uint32)((bs->cache & (riceParamMask >> setBitOffsetPlus1)) >> DRFLAC_CACHE_L1_SELECTION_SHIFT(bs, riceLength));

        bs->consumedBits += riceLength;
        bs->cache <<= riceLength;
    } else {
        drflac_uint32 bitCountLo;
        drflac_cache_t resultHi;

        bs->consumedBits += riceLength;
        bs->cache <<= setBitOffsetPlus1 & (DRFLAC_CACHE_L1_SIZE_BITS(bs)-1);    /* <-- Equivalent to "if (setBitOffsetPlus1 < DRFLAC_CACHE_L1_SIZE_BITS(bs)) { bs->cache <<= setBitOffsetPlus1; }" */

        /* It straddles the cached data. It will never cover more than the next chunk. We just read the number in two parts and combine them. */
        bitCountLo = bs->consumedBits - DRFLAC_CACHE_L1_SIZE_BITS(bs);
        resultHi = DRFLAC_CACHE_L1_SELECT_AND_SHIFT(bs, riceParam);  /* <-- Use DRFLAC_CACHE_L1_SELECT_AND_SHIFT_SAFE() if ever this function allows riceParam=0. */

        if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
#ifndef DR_FLAC_NO_CRC
            drflac__update_crc16(bs);
#endif
            bs->cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
            bs->consumedBits = 0;
#ifndef DR_FLAC_NO_CRC
            bs->crc16Cache = bs->cache;
#endif
        } else {
            /* Slow path. We need to fetch more data from the client. */
            if (!drflac__reload_cache(bs)) {
                return DRFLAC_FALSE;
            }
        }

        riceParamPart = (drflac_uint32)(resultHi | DRFLAC_CACHE_L1_SELECT_AND_SHIFT_SAFE(bs, bitCountLo));

        bs->consumedBits += bitCountLo;
        bs->cache <<= bitCountLo;
    }

    pZeroCounterOut[0] = zeroCounter;
    pRiceParamPartOut[0] = riceParamPart;

    return DRFLAC_TRUE;
}
#endif

static DRFLAC_INLINE drflac_bool32 drflac__read_rice_parts_x1(drflac_bs* bs, drflac_uint8 riceParam, drflac_uint32* pZeroCounterOut, drflac_uint32* pRiceParamPartOut)
{
    drflac_uint32  riceParamPlus1 = riceParam + 1;
    /*drflac_cache_t riceParamPlus1Mask  = DRFLAC_CACHE_L1_SELECTION_MASK(riceParamPlus1);*/
    drflac_uint32  riceParamPlus1Shift = DRFLAC_CACHE_L1_SELECTION_SHIFT(bs, riceParamPlus1);
    drflac_uint32  riceParamPlus1MaxConsumedBits = DRFLAC_CACHE_L1_SIZE_BITS(bs) - riceParamPlus1;

    /*
    The idea here is to use local variables for the cache in an attempt to encourage the compiler to store them in registers. I have
    no idea how this will work in practice...
    */
    drflac_cache_t bs_cache = bs->cache;
    drflac_uint32  bs_consumedBits = bs->consumedBits;

    /* The first thing to do is find the first unset bit. Most likely a bit will be set in the current cache line. */
    drflac_uint32  lzcount = drflac__clz(bs_cache);
    if (lzcount < sizeof(bs_cache)*8) {
        pZeroCounterOut[0] = lzcount;

        /*
        It is most likely that the riceParam part (which comes after the zero counter) is also on this cache line. When extracting
        this, we include the set bit from the unary coded part because it simplifies cache management. This bit will be handled
        outside of this function at a higher level.
        */
    extract_rice_param_part:
        bs_cache       <<= lzcount;
        bs_consumedBits += lzcount;

        if (bs_consumedBits <= riceParamPlus1MaxConsumedBits) {
            /* Getting here means the rice parameter part is wholly contained within the current cache line. */
            pRiceParamPartOut[0] = (drflac_uint32)(bs_cache >> riceParamPlus1Shift);
            bs_cache       <<= riceParamPlus1;
            bs_consumedBits += riceParamPlus1;
        } else {
            drflac_uint32 riceParamPartHi;
            drflac_uint32 riceParamPartLo;
            drflac_uint32 riceParamPartLoBitCount;

            /*
            Getting here means the rice parameter part straddles the cache line. We need to read from the tail of the current cache
            line, reload the cache, and then combine it with the head of the next cache line.
            */

            /* Grab the high part of the rice parameter part. */
            riceParamPartHi = (drflac_uint32)(bs_cache >> riceParamPlus1Shift);

            /* Before reloading the cache we need to grab the size in bits of the low part. */
            riceParamPartLoBitCount = bs_consumedBits - riceParamPlus1MaxConsumedBits;
            drflac_assert(riceParamPartLoBitCount > 0 && riceParamPartLoBitCount < 32);
                
            /* Now reload the cache. */
            if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
            #ifndef DR_FLAC_NO_CRC
                drflac__update_crc16(bs);
            #endif
                bs_cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
                bs_consumedBits = riceParamPartLoBitCount;
            #ifndef DR_FLAC_NO_CRC
                bs->crc16Cache = bs_cache;
            #endif
            } else {
                /* Slow path. We need to fetch more data from the client. */
                if (!drflac__reload_cache(bs)) {
                    return DRFLAC_FALSE;
                }

                bs_cache = bs->cache;
                bs_consumedBits = bs->consumedBits + riceParamPartLoBitCount;
            }

            /* We should now have enough information to construct the rice parameter part. */
            riceParamPartLo = (drflac_uint32)(bs_cache >> (DRFLAC_CACHE_L1_SELECTION_SHIFT(bs, riceParamPartLoBitCount)));
            pRiceParamPartOut[0] = riceParamPartHi | riceParamPartLo;

            bs_cache <<= riceParamPartLoBitCount;
        }
    } else {
        /*
        Getting here means there are no bits set on the cache line. This is a less optimal case because we just wasted a call
        to drflac__clz() and we need to reload the cache.
        */
        drflac_uint32 zeroCounter = (drflac_uint32)(DRFLAC_CACHE_L1_SIZE_BITS(bs) - bs_consumedBits);
        for (;;) {
            if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
            #ifndef DR_FLAC_NO_CRC
                drflac__update_crc16(bs);
            #endif
                bs_cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
                bs_consumedBits = 0;
            #ifndef DR_FLAC_NO_CRC
                bs->crc16Cache = bs_cache;
            #endif
            } else {
                /* Slow path. We need to fetch more data from the client. */
                if (!drflac__reload_cache(bs)) {
                    return DRFLAC_FALSE;
                }

                bs_cache = bs->cache;
                bs_consumedBits = bs->consumedBits;
            }

            lzcount = drflac__clz(bs_cache);
            zeroCounter += lzcount;

            if (lzcount < sizeof(bs_cache)*8) {
                break;
            }
        }

        pZeroCounterOut[0] = zeroCounter;
        goto extract_rice_param_part;
    }

    /* Make sure the cache is restored at the end of it all. */
    bs->cache = bs_cache;
    bs->consumedBits = bs_consumedBits;

    return DRFLAC_TRUE;
}

static DRFLAC_INLINE drflac_bool32 drflac__read_rice_parts_x4(drflac_bs* bs, drflac_uint8 riceParam, drflac_uint32* pZeroCounterOut, drflac_uint32* pRiceParamPartOut)
{
    drflac_uint32  riceParamPlus1 = riceParam + 1;
    /*drflac_cache_t riceParamPlus1Mask  = DRFLAC_CACHE_L1_SELECTION_MASK(riceParamPlus1);*/
    drflac_uint32  riceParamPlus1Shift = DRFLAC_CACHE_L1_SELECTION_SHIFT(bs, riceParamPlus1);
    drflac_uint32  riceParamPlus1MaxConsumedBits = DRFLAC_CACHE_L1_SIZE_BITS(bs) - riceParamPlus1;

    /*
    The idea here is to use local variables for the cache in an attempt to encourage the compiler to store them in registers. I have
    no idea how this will work in practice...
    */
    drflac_cache_t bs_cache = bs->cache;
    drflac_uint32  bs_consumedBits = bs->consumedBits;

    /*
    What this is doing is trying to efficiently extract 4 rice parts at a time, the idea being that we can exploit certain properties
    to our advantage to make things more efficient.
    */
    int i;
    for (i = 0; i < 4; ++i) {
        /* The first thing to do is find the first unset bit. Most likely a bit will be set in the current cache line. */
        drflac_uint32  lzcount = drflac__clz(bs_cache);
        if (lzcount < sizeof(bs_cache)*8) {
            pZeroCounterOut[i] = lzcount;

            /*
            It is most likely that the riceParam part (which comes after the zero counter) is also on this cache line. When extracting
            this, we include the set bit from the unary coded part because it simplifies cache management. This bit will be handled
            outside of this function at a higher level.
            */
        extract_rice_param_part:
            bs_cache       <<= lzcount;
            bs_consumedBits += lzcount;

            if (bs_consumedBits <= riceParamPlus1MaxConsumedBits) {
                /* Getting here means the rice parameter part is wholly contained within the current cache line. */
                pRiceParamPartOut[i] = (drflac_uint32)(bs_cache >> riceParamPlus1Shift);
                bs_cache       <<= riceParamPlus1;
                bs_consumedBits += riceParamPlus1;
            } else {
                drflac_uint32 riceParamPartHi;
                drflac_uint32 riceParamPartLo;
                drflac_uint32 riceParamPartLoBitCount;

                /*
                Getting here means the rice parameter part straddles the cache line. We need to read from the tail of the current cache
                line, reload the cache, and then combine it with the head of the next cache line.
                */

                /* Grab the high part of the rice parameter part. */
                riceParamPartHi = (drflac_uint32)(bs_cache >> riceParamPlus1Shift);

                /* Before reloading the cache we need to grab the size in bits of the low part. */
                riceParamPartLoBitCount = bs_consumedBits - riceParamPlus1MaxConsumedBits;
                
                /* Now reload the cache. */
                if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
                #ifndef DR_FLAC_NO_CRC
                    drflac__update_crc16(bs);
                #endif
                    bs_cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
                    bs_consumedBits = riceParamPartLoBitCount;
                #ifndef DR_FLAC_NO_CRC
                    bs->crc16Cache = bs_cache;
                #endif
                } else {
                    /* Slow path. We need to fetch more data from the client. */
                    if (!drflac__reload_cache(bs)) {
                        return DRFLAC_FALSE;
                    }

                    bs_cache = bs->cache;
                    bs_consumedBits = bs->consumedBits + riceParamPartLoBitCount;
                }

                /* We should now have enough information to construct the rice parameter part. */
                riceParamPartLo = (drflac_uint32)(bs_cache >> (DRFLAC_CACHE_L1_SELECTION_SHIFT(bs, riceParamPartLoBitCount)));
                pRiceParamPartOut[i] = riceParamPartHi | riceParamPartLo;

                bs_cache <<= riceParamPartLoBitCount;
            }
        } else {
            /*
            Getting here means there are no bits set on the cache line. This is a less optimal case because we just wasted a call
            to drflac__clz() and we need to reload the cache.
            */
            drflac_uint32 zeroCounter = (drflac_uint32)(DRFLAC_CACHE_L1_SIZE_BITS(bs) - bs_consumedBits);
            for (;;) {
                if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
                #ifndef DR_FLAC_NO_CRC
                    drflac__update_crc16(bs);
                #endif
                    bs_cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
                    bs_consumedBits = 0;
                #ifndef DR_FLAC_NO_CRC
                    bs->crc16Cache = bs_cache;
                #endif
                } else {
                    /* Slow path. We need to fetch more data from the client. */
                    if (!drflac__reload_cache(bs)) {
                        return DRFLAC_FALSE;
                    }

                    bs_cache = bs->cache;
                    bs_consumedBits = bs->consumedBits;
                }

                lzcount = drflac__clz(bs_cache);
                zeroCounter += lzcount;

                if (lzcount < sizeof(bs_cache)*8) {
                    break;
                }
            }

            pZeroCounterOut[i] = zeroCounter;
            goto extract_rice_param_part;
        }
    }

    /* Make sure the cache is restored at the end of it all. */
    bs->cache = bs_cache;
    bs->consumedBits = bs_consumedBits;

    return DRFLAC_TRUE;
}

static DRFLAC_INLINE drflac_bool32 drflac__seek_rice_parts(drflac_bs* bs, drflac_uint8 riceParam)
{
    drflac_uint32  riceParamPlus1 = riceParam + 1;
    drflac_uint32  riceParamPlus1MaxConsumedBits = DRFLAC_CACHE_L1_SIZE_BITS(bs) - riceParamPlus1;

    /*
    The idea here is to use local variables for the cache in an attempt to encourage the compiler to store them in registers. I have
    no idea how this will work in practice...
    */
    drflac_cache_t bs_cache = bs->cache;
    drflac_uint32  bs_consumedBits = bs->consumedBits;

    /* The first thing to do is find the first unset bit. Most likely a bit will be set in the current cache line. */
    drflac_uint32  lzcount = drflac__clz(bs_cache);
    if (lzcount < sizeof(bs_cache)*8) {
        /*
        It is most likely that the riceParam part (which comes after the zero counter) is also on this cache line. When extracting
        this, we include the set bit from the unary coded part because it simplifies cache management. This bit will be handled
        outside of this function at a higher level.
        */
    extract_rice_param_part:
        bs_cache       <<= lzcount;
        bs_consumedBits += lzcount;

        if (bs_consumedBits <= riceParamPlus1MaxConsumedBits) {
            /* Getting here means the rice parameter part is wholly contained within the current cache line. */
            bs_cache       <<= riceParamPlus1;
            bs_consumedBits += riceParamPlus1;
        } else {
            /*
            Getting here means the rice parameter part straddles the cache line. We need to read from the tail of the current cache
            line, reload the cache, and then combine it with the head of the next cache line.
            */

            /* Before reloading the cache we need to grab the size in bits of the low part. */
            drflac_uint32 riceParamPartLoBitCount = bs_consumedBits - riceParamPlus1MaxConsumedBits;
            drflac_assert(riceParamPartLoBitCount > 0 && riceParamPartLoBitCount < 32);
                
            /* Now reload the cache. */
            if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
            #ifndef DR_FLAC_NO_CRC
                drflac__update_crc16(bs);
            #endif
                bs_cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
                bs_consumedBits = riceParamPartLoBitCount;
            #ifndef DR_FLAC_NO_CRC
                bs->crc16Cache = bs_cache;
            #endif
            } else {
                /* Slow path. We need to fetch more data from the client. */
                if (!drflac__reload_cache(bs)) {
                    return DRFLAC_FALSE;
                }

                bs_cache = bs->cache;
                bs_consumedBits = bs->consumedBits + riceParamPartLoBitCount;
            }

            bs_cache <<= riceParamPartLoBitCount;
        }
    } else {
        /*
        Getting here means there are no bits set on the cache line. This is a less optimal case because we just wasted a call
        to drflac__clz() and we need to reload the cache.
        */
        for (;;) {
            if (bs->nextL2Line < DRFLAC_CACHE_L2_LINE_COUNT(bs)) {
            #ifndef DR_FLAC_NO_CRC
                drflac__update_crc16(bs);
            #endif
                bs_cache = drflac__be2host__cache_line(bs->cacheL2[bs->nextL2Line++]);
                bs_consumedBits = 0;
            #ifndef DR_FLAC_NO_CRC
                bs->crc16Cache = bs_cache;
            #endif
            } else {
                /* Slow path. We need to fetch more data from the client. */
                if (!drflac__reload_cache(bs)) {
                    return DRFLAC_FALSE;
                }

                bs_cache = bs->cache;
                bs_consumedBits = bs->consumedBits;
            }

            lzcount = drflac__clz(bs_cache);
            if (lzcount < sizeof(bs_cache)*8) {
                break;
            }
        }

        goto extract_rice_param_part;
    }

    /* Make sure the cache is restored at the end of it all. */
    bs->cache = bs_cache;
    bs->consumedBits = bs_consumedBits;

    return DRFLAC_TRUE;
}


static drflac_bool32 drflac__decode_samples_with_residual__rice__scalar(drflac_bs* bs, drflac_uint32 bitsPerSample, drflac_uint32 count, drflac_uint8 riceParam, drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pSamplesOut)
{
    drflac_uint32 t[2] = {0x00000000, 0xFFFFFFFF};
    drflac_uint32 zeroCountPart0;
    drflac_uint32 zeroCountPart1;
    drflac_uint32 zeroCountPart2;
    drflac_uint32 zeroCountPart3;
    drflac_uint32 riceParamPart0;
    drflac_uint32 riceParamPart1;
    drflac_uint32 riceParamPart2;
    drflac_uint32 riceParamPart3;
    drflac_uint32 riceParamMask;
    const drflac_int32* pSamplesOutEnd;
    drflac_uint32 i;

    drflac_assert(bs != NULL);
    drflac_assert(count > 0);
    drflac_assert(pSamplesOut != NULL);

    riceParamMask  = ~((~0UL) << riceParam);
    pSamplesOutEnd = pSamplesOut + ((count >> 2) << 2);

    if (bitsPerSample >= 24) {
        while (pSamplesOut < pSamplesOutEnd) {
            /*
            Rice extraction. It's faster to do this one at a time against local variables than it is to use the x4 version
            against an array. Not sure why, but perhaps it's making more efficient use of registers?
            */
            if (!drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart0, &riceParamPart0) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart1, &riceParamPart1) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart2, &riceParamPart2) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart3, &riceParamPart3)) {
                return DRFLAC_FALSE;
            }

            riceParamPart0 &= riceParamMask;
            riceParamPart1 &= riceParamMask;
            riceParamPart2 &= riceParamMask;
            riceParamPart3 &= riceParamMask;

            riceParamPart0 |= (zeroCountPart0 << riceParam);
            riceParamPart1 |= (zeroCountPart1 << riceParam);
            riceParamPart2 |= (zeroCountPart2 << riceParam);
            riceParamPart3 |= (zeroCountPart3 << riceParam);

            riceParamPart0  = (riceParamPart0 >> 1) ^ t[riceParamPart0 & 0x01];
            riceParamPart1  = (riceParamPart1 >> 1) ^ t[riceParamPart1 & 0x01];
            riceParamPart2  = (riceParamPart2 >> 1) ^ t[riceParamPart2 & 0x01];
            riceParamPart3  = (riceParamPart3 >> 1) ^ t[riceParamPart3 & 0x01];

            pSamplesOut[0] = riceParamPart0 + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + 0);
            pSamplesOut[1] = riceParamPart1 + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + 1);
            pSamplesOut[2] = riceParamPart2 + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + 2);
            pSamplesOut[3] = riceParamPart3 + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + 3);

            pSamplesOut += 4;
        }
    } else {
        while (pSamplesOut < pSamplesOutEnd) {
            if (!drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart0, &riceParamPart0) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart1, &riceParamPart1) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart2, &riceParamPart2) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart3, &riceParamPart3)) {
                return DRFLAC_FALSE;
            }

            riceParamPart0 &= riceParamMask;
            riceParamPart1 &= riceParamMask;
            riceParamPart2 &= riceParamMask;
            riceParamPart3 &= riceParamMask;

            riceParamPart0 |= (zeroCountPart0 << riceParam);
            riceParamPart1 |= (zeroCountPart1 << riceParam);
            riceParamPart2 |= (zeroCountPart2 << riceParam);
            riceParamPart3 |= (zeroCountPart3 << riceParam);

            riceParamPart0  = (riceParamPart0 >> 1) ^ t[riceParamPart0 & 0x01];
            riceParamPart1  = (riceParamPart1 >> 1) ^ t[riceParamPart1 & 0x01];
            riceParamPart2  = (riceParamPart2 >> 1) ^ t[riceParamPart2 & 0x01];
            riceParamPart3  = (riceParamPart3 >> 1) ^ t[riceParamPart3 & 0x01];

            pSamplesOut[0] = riceParamPart0 + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 0);
            pSamplesOut[1] = riceParamPart1 + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 1);
            pSamplesOut[2] = riceParamPart2 + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 2);
            pSamplesOut[3] = riceParamPart3 + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 3);

            pSamplesOut += 4;
        }
    }

    i = ((count >> 2) << 2);
    while (i < count) {
        /* Rice extraction. */
        if (!drflac__read_rice_parts_x1(bs, riceParam, &zeroCountPart0, &riceParamPart0)) {
            return DRFLAC_FALSE;
        }

        /* Rice reconstruction. */
        riceParamPart0 &= riceParamMask;
        riceParamPart0 |= (zeroCountPart0 << riceParam);
        riceParamPart0  = (riceParamPart0 >> 1) ^ t[riceParamPart0 & 0x01];
        /*riceParamPart0  = (riceParamPart0 >> 1) ^ (~(riceParamPart0 & 0x01) + 1);*/

        /* Sample reconstruction. */
        if (bitsPerSample >= 24) {
            pSamplesOut[0] = riceParamPart0 + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + 0);
        } else {
            pSamplesOut[0] = riceParamPart0 + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 0);
        }

        i += 1;
        pSamplesOut += 1;
    }
    
    return DRFLAC_TRUE;
}

#if defined(DRFLAC_SUPPORT_SSE41)
static drflac_bool32 drflac__decode_samples_with_residual__rice__sse41(drflac_bs* bs, drflac_uint32 bitsPerSample, drflac_uint32 count, drflac_uint8 riceParam, drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pSamplesOut)
{
    static drflac_uint32 t[2] = {0x00000000, 0xFFFFFFFF};

    /*drflac_uint32 zeroCountParts[4];*/
    /*drflac_uint32 riceParamParts[4];*/

    drflac_uint32 zeroCountParts0;
    drflac_uint32 zeroCountParts1;
    drflac_uint32 zeroCountParts2;
    drflac_uint32 zeroCountParts3;
    drflac_uint32 riceParamParts0;
    drflac_uint32 riceParamParts1;
    drflac_uint32 riceParamParts2;
    drflac_uint32 riceParamParts3;
    drflac_uint32 riceParamMask;
    const drflac_int32* pSamplesOutEnd;
    __m128i riceParamMask128;
    __m128i one;
    drflac_uint32 i;

    drflac_assert(bs != NULL);
    drflac_assert(count > 0);
    drflac_assert(pSamplesOut != NULL);

    riceParamMask = ~((~0UL) << riceParam);
    riceParamMask128 = _mm_set1_epi32(riceParamMask);
    one = _mm_set1_epi32(0x01);

    pSamplesOutEnd = pSamplesOut + ((count >> 2) << 2);

    if (bitsPerSample >= 24) {
        while (pSamplesOut < pSamplesOutEnd) {
            __m128i zeroCountPart128;
            __m128i riceParamPart128;
            drflac_uint32 riceParamParts[4];

            /* Rice extraction. */
            if (!drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts0, &riceParamParts0) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts1, &riceParamParts1) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts2, &riceParamParts2) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts3, &riceParamParts3)) {
                return DRFLAC_FALSE;
            }

            zeroCountPart128 = _mm_set_epi32(zeroCountParts3, zeroCountParts2, zeroCountParts1, zeroCountParts0);
            riceParamPart128 = _mm_set_epi32(riceParamParts3, riceParamParts2, riceParamParts1, riceParamParts0);

            riceParamPart128 = _mm_and_si128(riceParamPart128, riceParamMask128);
            riceParamPart128 = _mm_or_si128(riceParamPart128, _mm_slli_epi32(zeroCountPart128, riceParam));
            riceParamPart128 = _mm_xor_si128(_mm_srli_epi32(riceParamPart128, 1), _mm_mullo_epi32(_mm_and_si128(riceParamPart128, one), _mm_set1_epi32(0xFFFFFFFF)));   /* <-- Only supported from SSE4.1 */
            /*riceParamPart128 = _mm_xor_si128(_mm_srli_epi32(riceParamPart128, 1), _mm_add_epi32(drflac__mm_not_si128(_mm_and_si128(riceParamPart128, one)), one));*/  /* <-- SSE2 compatible */

            _mm_storeu_si128((__m128i*)riceParamParts, riceParamPart128);

        #if defined(DRFLAC_64BIT)
            /* The scalar implementation seems to be faster on 64-bit in my testing. */
            drflac__calculate_prediction_64_x4(order, shift, coefficients, riceParamParts, pSamplesOut);
        #else
            pSamplesOut[0] = riceParamParts[0] + drflac__calculate_prediction_64__sse41(order, shift, coefficients, pSamplesOut + 0);
            pSamplesOut[1] = riceParamParts[1] + drflac__calculate_prediction_64__sse41(order, shift, coefficients, pSamplesOut + 1);
            pSamplesOut[2] = riceParamParts[2] + drflac__calculate_prediction_64__sse41(order, shift, coefficients, pSamplesOut + 2);
            pSamplesOut[3] = riceParamParts[3] + drflac__calculate_prediction_64__sse41(order, shift, coefficients, pSamplesOut + 3);
        #endif

            pSamplesOut += 4;
        }
    } else {
        drflac_int32 coefficientsUnaligned[32*4 + 4] = {0};
        drflac_int32* coefficients128 = (drflac_int32*)(((size_t)coefficientsUnaligned + 15) & ~15);

        for (i = 0; i < order; ++i) {
            coefficients128[i*4+0] = coefficients[i];
            coefficients128[i*4+1] = coefficients[i];
            coefficients128[i*4+2] = coefficients[i];
            coefficients128[i*4+3] = coefficients[i];
        }

        while (pSamplesOut < pSamplesOutEnd) {
            __m128i zeroCountPart128;
            __m128i riceParamPart128;
            /*drflac_int32 riceParamParts[4];*/

            /* Rice extraction. */
#if 1
            if (!drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts0, &riceParamParts0) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts1, &riceParamParts1) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts2, &riceParamParts2) ||
                !drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts3, &riceParamParts3)) {
                return DRFLAC_FALSE;
            }

            zeroCountPart128 = _mm_set_epi32(zeroCountParts3, zeroCountParts2, zeroCountParts1, zeroCountParts0);
            riceParamPart128 = _mm_set_epi32(riceParamParts3, riceParamParts2, riceParamParts1, riceParamParts0);
#else
            if (!drflac__read_rice_parts_x4(bs, riceParam, zeroCountParts, riceParamParts)) {
                return DRFLAC_FALSE;
            }

            zeroCountPart128 = _mm_set_epi32(zeroCountParts[3], zeroCountParts[2], zeroCountParts[1], zeroCountParts[0]);
            riceParamPart128 = _mm_set_epi32(riceParamParts[3], riceParamParts[2], riceParamParts[1], riceParamParts[0]);
#endif

            riceParamPart128 = _mm_and_si128(riceParamPart128, riceParamMask128);
            riceParamPart128 = _mm_or_si128(riceParamPart128, _mm_slli_epi32(zeroCountPart128, riceParam));
            riceParamPart128 = _mm_xor_si128(_mm_srli_epi32(riceParamPart128, 1), _mm_mullo_epi32(_mm_and_si128(riceParamPart128, one), _mm_set1_epi32(0xFFFFFFFF)));

#if 1
            drflac__calculate_prediction_32_x4__sse41(order, shift, (const __m128i*)coefficients128, riceParamPart128, pSamplesOut);
#else
            _mm_storeu_si128((__m128i*)riceParamParts, riceParamPart128);

            pSamplesOut[0] = riceParamParts[0] + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 0);
            pSamplesOut[1] = riceParamParts[1] + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 1);
            pSamplesOut[2] = riceParamParts[2] + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 2);
            pSamplesOut[3] = riceParamParts[3] + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 3);
#endif

            pSamplesOut += 4;
        }
    }


    i = ((count >> 2) << 2);
    while (i < count) {
        /* Rice extraction. */
        if (!drflac__read_rice_parts_x1(bs, riceParam, &zeroCountParts0, &riceParamParts0)) {
            return DRFLAC_FALSE;
        }

        /* Rice reconstruction. */
        riceParamParts0 &= riceParamMask;
        riceParamParts0 |= (zeroCountParts0 << riceParam);
        riceParamParts0  = (riceParamParts0 >> 1) ^ t[riceParamParts0 & 0x01];

        /* Sample reconstruction. */
        if (bitsPerSample >= 24) {
            pSamplesOut[0] = riceParamParts0 + drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + 0);
        } else {
            pSamplesOut[0] = riceParamParts0 + drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + 0);
        }

        i += 1;
        pSamplesOut += 1;
    }

    return DRFLAC_TRUE;
}
#endif

static drflac_bool32 drflac__decode_samples_with_residual__rice(drflac_bs* bs, drflac_uint32 bitsPerSample, drflac_uint32 count, drflac_uint8 riceParam, drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pSamplesOut)
{
#if defined(DRFLAC_SUPPORT_SSE41)
    if (drflac__gIsSSE41Supported) {
        return drflac__decode_samples_with_residual__rice__sse41(bs, bitsPerSample, count, riceParam, order, shift, coefficients, pSamplesOut);
    } else
#endif
    {
        /* Scalar fallback. */
    #if 0
        return drflac__decode_samples_with_residual__rice__reference(bs, bitsPerSample, count, riceParam, order, shift, coefficients, pSamplesOut);
    #else
        return drflac__decode_samples_with_residual__rice__scalar(bs, bitsPerSample, count, riceParam, order, shift, coefficients, pSamplesOut);
    #endif
    }
}

/* Reads and seeks past a string of residual values as Rice codes. The decoder should be sitting on the first bit of the Rice codes. */
static drflac_bool32 drflac__read_and_seek_residual__rice(drflac_bs* bs, drflac_uint32 count, drflac_uint8 riceParam)
{
    drflac_uint32 i;

    drflac_assert(bs != NULL);
    drflac_assert(count > 0);

    for (i = 0; i < count; ++i) {
        if (!drflac__seek_rice_parts(bs, riceParam)) {
            return DRFLAC_FALSE;
        }
    }

    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__decode_samples_with_residual__unencoded(drflac_bs* bs, drflac_uint32 bitsPerSample, drflac_uint32 count, drflac_uint8 unencodedBitsPerSample, drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pSamplesOut)
{
    drflac_uint32 i;

    drflac_assert(bs != NULL);
    drflac_assert(count > 0);
    drflac_assert(unencodedBitsPerSample <= 31);    /* <-- unencodedBitsPerSample is a 5 bit number, so cannot exceed 31. */
    drflac_assert(pSamplesOut != NULL);

    for (i = 0; i < count; ++i) {
        if (unencodedBitsPerSample > 0) {
            if (!drflac__read_int32(bs, unencodedBitsPerSample, pSamplesOut + i)) {
                return DRFLAC_FALSE;
            }
        } else {
            pSamplesOut[i] = 0;
        }

        if (bitsPerSample > 16) {
            pSamplesOut[i] += drflac__calculate_prediction_64(order, shift, coefficients, pSamplesOut + i);
        } else {
            pSamplesOut[i] += drflac__calculate_prediction_32(order, shift, coefficients, pSamplesOut + i);
        }
    }

    return DRFLAC_TRUE;
}


/*
Reads and decodes the residual for the sub-frame the decoder is currently sitting on. This function should be called
when the decoder is sitting at the very start of the RESIDUAL block. The first <order> residuals will be ignored. The
<blockSize> and <order> parameters are used to determine how many residual values need to be decoded.
*/
static drflac_bool32 drflac__decode_samples_with_residual(drflac_bs* bs, drflac_uint32 bitsPerSample, drflac_uint32 blockSize, drflac_uint32 order, drflac_int32 shift, const drflac_int32* coefficients, drflac_int32* pDecodedSamples)
{
    drflac_uint8 residualMethod;
    drflac_uint8 partitionOrder;
    drflac_uint32 samplesInPartition;
    drflac_uint32 partitionsRemaining;

    drflac_assert(bs != NULL);
    drflac_assert(blockSize != 0);
    drflac_assert(pDecodedSamples != NULL);       /* <-- Should we allow NULL, in which case we just seek past the residual rather than do a full decode? */

    if (!drflac__read_uint8(bs, 2, &residualMethod)) {
        return DRFLAC_FALSE;
    }

    if (residualMethod != DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE && residualMethod != DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2) {
        return DRFLAC_FALSE;    /* Unknown or unsupported residual coding method. */
    }

    /* Ignore the first <order> values. */
    pDecodedSamples += order;

    if (!drflac__read_uint8(bs, 4, &partitionOrder)) {
        return DRFLAC_FALSE;
    }

    /*
    From the FLAC spec:
      The Rice partition order in a Rice-coded residual section must be less than or equal to 8.
    */
    if (partitionOrder > 8) {
        return DRFLAC_FALSE;
    }

    /* Validation check. */
    if ((blockSize / (1 << partitionOrder)) <= order) {
        return DRFLAC_FALSE;
    }

    samplesInPartition = (blockSize / (1 << partitionOrder)) - order;
    partitionsRemaining = (1 << partitionOrder);
    for (;;) {
        drflac_uint8 riceParam = 0;
        if (residualMethod == DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE) {
            if (!drflac__read_uint8(bs, 4, &riceParam)) {
                return DRFLAC_FALSE;
            }
            if (riceParam == 15) {
                riceParam = 0xFF;
            }
        } else if (residualMethod == DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2) {
            if (!drflac__read_uint8(bs, 5, &riceParam)) {
                return DRFLAC_FALSE;
            }
            if (riceParam == 31) {
                riceParam = 0xFF;
            }
        }

        if (riceParam != 0xFF) {
            if (!drflac__decode_samples_with_residual__rice(bs, bitsPerSample, samplesInPartition, riceParam, order, shift, coefficients, pDecodedSamples)) {
                return DRFLAC_FALSE;
            }
        } else {
            unsigned char unencodedBitsPerSample = 0;
            if (!drflac__read_uint8(bs, 5, &unencodedBitsPerSample)) {
                return DRFLAC_FALSE;
            }

            if (!drflac__decode_samples_with_residual__unencoded(bs, bitsPerSample, samplesInPartition, unencodedBitsPerSample, order, shift, coefficients, pDecodedSamples)) {
                return DRFLAC_FALSE;
            }
        }

        pDecodedSamples += samplesInPartition;

        if (partitionsRemaining == 1) {
            break;
        }

        partitionsRemaining -= 1;

        if (partitionOrder != 0) {
            samplesInPartition = blockSize / (1 << partitionOrder);
        }
    }

    return DRFLAC_TRUE;
}

/*
Reads and seeks past the residual for the sub-frame the decoder is currently sitting on. This function should be called
when the decoder is sitting at the very start of the RESIDUAL block. The first <order> residuals will be set to 0. The
<blockSize> and <order> parameters are used to determine how many residual values need to be decoded.
*/
static drflac_bool32 drflac__read_and_seek_residual(drflac_bs* bs, drflac_uint32 blockSize, drflac_uint32 order)
{
    drflac_uint8 residualMethod;
    drflac_uint8 partitionOrder;
    drflac_uint32 samplesInPartition;
    drflac_uint32 partitionsRemaining;

    drflac_assert(bs != NULL);
    drflac_assert(blockSize != 0);

    if (!drflac__read_uint8(bs, 2, &residualMethod)) {
        return DRFLAC_FALSE;
    }

    if (residualMethod != DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE && residualMethod != DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2) {
        return DRFLAC_FALSE;    /* Unknown or unsupported residual coding method. */
    }

    if (!drflac__read_uint8(bs, 4, &partitionOrder)) {
        return DRFLAC_FALSE;
    }

    /*
    From the FLAC spec:
      The Rice partition order in a Rice-coded residual section must be less than or equal to 8.
    */
    if (partitionOrder > 8) {
        return DRFLAC_FALSE;
    }

    /* Validation check. */
    if ((blockSize / (1 << partitionOrder)) <= order) {
        return DRFLAC_FALSE;
    }

    samplesInPartition = (blockSize / (1 << partitionOrder)) - order;
    partitionsRemaining = (1 << partitionOrder);
    for (;;)
    {
        drflac_uint8 riceParam = 0;
        if (residualMethod == DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE) {
            if (!drflac__read_uint8(bs, 4, &riceParam)) {
                return DRFLAC_FALSE;
            }
            if (riceParam == 15) {
                riceParam = 0xFF;
            }
        } else if (residualMethod == DRFLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2) {
            if (!drflac__read_uint8(bs, 5, &riceParam)) {
                return DRFLAC_FALSE;
            }
            if (riceParam == 31) {
                riceParam = 0xFF;
            }
        }

        if (riceParam != 0xFF) {
            if (!drflac__read_and_seek_residual__rice(bs, samplesInPartition, riceParam)) {
                return DRFLAC_FALSE;
            }
        } else {
            unsigned char unencodedBitsPerSample = 0;
            if (!drflac__read_uint8(bs, 5, &unencodedBitsPerSample)) {
                return DRFLAC_FALSE;
            }

            if (!drflac__seek_bits(bs, unencodedBitsPerSample * samplesInPartition)) {
                return DRFLAC_FALSE;
            }
        }


        if (partitionsRemaining == 1) {
            break;
        }

        partitionsRemaining -= 1;
        samplesInPartition = blockSize / (1 << partitionOrder);
    }

    return DRFLAC_TRUE;
}


static drflac_bool32 drflac__decode_samples__constant(drflac_bs* bs, drflac_uint32 blockSize, drflac_uint32 bitsPerSample, drflac_int32* pDecodedSamples)
{
    drflac_uint32 i;

    /* Only a single sample needs to be decoded here. */
    drflac_int32 sample;
    if (!drflac__read_int32(bs, bitsPerSample, &sample)) {
        return DRFLAC_FALSE;
    }

    /*
    We don't really need to expand this, but it does simplify the process of reading samples. If this becomes a performance issue (unlikely)
    we'll want to look at a more efficient way.
    */
    for (i = 0; i < blockSize; ++i) {
        pDecodedSamples[i] = sample;
    }

    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__decode_samples__verbatim(drflac_bs* bs, drflac_uint32 blockSize, drflac_uint32 bitsPerSample, drflac_int32* pDecodedSamples)
{
    drflac_uint32 i;

    for (i = 0; i < blockSize; ++i) {
        drflac_int32 sample;
        if (!drflac__read_int32(bs, bitsPerSample, &sample)) {
            return DRFLAC_FALSE;
        }

        pDecodedSamples[i] = sample;
    }

    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__decode_samples__fixed(drflac_bs* bs, drflac_uint32 blockSize, drflac_uint32 bitsPerSample, drflac_uint8 lpcOrder, drflac_int32* pDecodedSamples)
{
    drflac_uint32 i;

    static drflac_int32 lpcCoefficientsTable[5][4] = {
        {0,  0, 0,  0},
        {1,  0, 0,  0},
        {2, -1, 0,  0},
        {3, -3, 1,  0},
        {4, -6, 4, -1}
    };

    /* Warm up samples and coefficients. */
    for (i = 0; i < lpcOrder; ++i) {
        drflac_int32 sample;
        if (!drflac__read_int32(bs, bitsPerSample, &sample)) {
            return DRFLAC_FALSE;
        }

        pDecodedSamples[i] = sample;
    }

    if (!drflac__decode_samples_with_residual(bs, bitsPerSample, blockSize, lpcOrder, 0, lpcCoefficientsTable[lpcOrder], pDecodedSamples)) {
        return DRFLAC_FALSE;
    }

    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__decode_samples__lpc(drflac_bs* bs, drflac_uint32 blockSize, drflac_uint32 bitsPerSample, drflac_uint8 lpcOrder, drflac_int32* pDecodedSamples)
{
    drflac_uint8 i;
    drflac_uint8 lpcPrecision;
    drflac_int8 lpcShift;
    drflac_int32 coefficients[32];

    /* Warm up samples. */
    for (i = 0; i < lpcOrder; ++i) {
        drflac_int32 sample;
        if (!drflac__read_int32(bs, bitsPerSample, &sample)) {
            return DRFLAC_FALSE;
        }

        pDecodedSamples[i] = sample;
    }

    if (!drflac__read_uint8(bs, 4, &lpcPrecision)) {
        return DRFLAC_FALSE;
    }
    if (lpcPrecision == 15) {
        return DRFLAC_FALSE;    /* Invalid. */
    }
    lpcPrecision += 1;

    if (!drflac__read_int8(bs, 5, &lpcShift)) {
        return DRFLAC_FALSE;
    }

    drflac_zero_memory(coefficients, sizeof(coefficients));
    for (i = 0; i < lpcOrder; ++i) {
        if (!drflac__read_int32(bs, lpcPrecision, coefficients + i)) {
            return DRFLAC_FALSE;
        }
    }

    if (!drflac__decode_samples_with_residual(bs, bitsPerSample, blockSize, lpcOrder, lpcShift, coefficients, pDecodedSamples)) {
        return DRFLAC_FALSE;
    }

    return DRFLAC_TRUE;
}


static drflac_bool32 drflac__read_next_flac_frame_header(drflac_bs* bs, drflac_uint8 streaminfoBitsPerSample, drflac_frame_header* header)
{
    const drflac_uint32 sampleRateTable[12]  = {0, 88200, 176400, 192000, 8000, 16000, 22050, 24000, 32000, 44100, 48000, 96000};
    const drflac_uint8 bitsPerSampleTable[8] = {0, 8, 12, (drflac_uint8)-1, 16, 20, 24, (drflac_uint8)-1};   /* -1 = reserved. */

    drflac_assert(bs != NULL);
    drflac_assert(header != NULL);

    /* Keep looping until we find a valid sync code. */
    for (;;) {
        drflac_uint8 crc8 = 0xCE; /* 0xCE = drflac_crc8(0, 0x3FFE, 14); */
        drflac_uint8 reserved = 0;
        drflac_uint8 blockingStrategy = 0;
        drflac_uint8 blockSize = 0;
        drflac_uint8 sampleRate = 0;
        drflac_uint8 channelAssignment = 0;
        drflac_uint8 bitsPerSample = 0;
        drflac_bool32 isVariableBlockSize;

        if (!drflac__find_and_seek_to_next_sync_code(bs)) {
            return DRFLAC_FALSE;
        }

        if (!drflac__read_uint8(bs, 1, &reserved)) {
            return DRFLAC_FALSE;
        }
        if (reserved == 1) {
            continue;
        }
        crc8 = drflac_crc8(crc8, reserved, 1);

        if (!drflac__read_uint8(bs, 1, &blockingStrategy)) {
            return DRFLAC_FALSE;
        }
        crc8 = drflac_crc8(crc8, blockingStrategy, 1);

        if (!drflac__read_uint8(bs, 4, &blockSize)) {
            return DRFLAC_FALSE;
        }
        if (blockSize == 0) {
            continue;
        }
        crc8 = drflac_crc8(crc8, blockSize, 4);

        if (!drflac__read_uint8(bs, 4, &sampleRate)) {
            return DRFLAC_FALSE;
        }
        crc8 = drflac_crc8(crc8, sampleRate, 4);

        if (!drflac__read_uint8(bs, 4, &channelAssignment)) {
            return DRFLAC_FALSE;
        }
        if (channelAssignment > 10) {
            continue;
        }
        crc8 = drflac_crc8(crc8, channelAssignment, 4);

        if (!drflac__read_uint8(bs, 3, &bitsPerSample)) {
            return DRFLAC_FALSE;
        }
        if (bitsPerSample == 3 || bitsPerSample == 7) {
            continue;
        }
        crc8 = drflac_crc8(crc8, bitsPerSample, 3);


        if (!drflac__read_uint8(bs, 1, &reserved)) {
            return DRFLAC_FALSE;
        }
        if (reserved == 1) {
            continue;
        }
        crc8 = drflac_crc8(crc8, reserved, 1);


        isVariableBlockSize = blockingStrategy == 1;
        if (isVariableBlockSize) {
            drflac_uint64 sampleNumber;
            drflac_result result = drflac__read_utf8_coded_number(bs, &sampleNumber, &crc8);
            if (result != DRFLAC_SUCCESS) {
                if (result == DRFLAC_END_OF_STREAM) {
                    return DRFLAC_FALSE;
                } else {
                    continue;
                }
            }
            header->frameNumber  = 0;
            header->sampleNumber = sampleNumber;
        } else {
            drflac_uint64 frameNumber = 0;
            drflac_result result = drflac__read_utf8_coded_number(bs, &frameNumber, &crc8);
            if (result != DRFLAC_SUCCESS) {
                if (result == DRFLAC_END_OF_STREAM) {
                    return DRFLAC_FALSE;
                } else {
                    continue;
                }
            }
            header->frameNumber  = (drflac_uint32)frameNumber;   /* <-- Safe cast. */
            header->sampleNumber = 0;
        }


        if (blockSize == 1) {
            header->blockSize = 192;
        } else if (blockSize >= 2 && blockSize <= 5) {
            header->blockSize = 576 * (1 << (blockSize - 2));
        } else if (blockSize == 6) {
            if (!drflac__read_uint16(bs, 8, &header->blockSize)) {
                return DRFLAC_FALSE;
            }
            crc8 = drflac_crc8(crc8, header->blockSize, 8);
            header->blockSize += 1;
        } else if (blockSize == 7) {
            if (!drflac__read_uint16(bs, 16, &header->blockSize)) {
                return DRFLAC_FALSE;
            }
            crc8 = drflac_crc8(crc8, header->blockSize, 16);
            header->blockSize += 1;
        } else {
            header->blockSize = 256 * (1 << (blockSize - 8));
        }


        if (sampleRate <= 11) {
            header->sampleRate = sampleRateTable[sampleRate];
        } else if (sampleRate == 12) {
            if (!drflac__read_uint32(bs, 8, &header->sampleRate)) {
                return DRFLAC_FALSE;
            }
            crc8 = drflac_crc8(crc8, header->sampleRate, 8);
            header->sampleRate *= 1000;
        } else if (sampleRate == 13) {
            if (!drflac__read_uint32(bs, 16, &header->sampleRate)) {
                return DRFLAC_FALSE;
            }
            crc8 = drflac_crc8(crc8, header->sampleRate, 16);
        } else if (sampleRate == 14) {
            if (!drflac__read_uint32(bs, 16, &header->sampleRate)) {
                return DRFLAC_FALSE;
            }
            crc8 = drflac_crc8(crc8, header->sampleRate, 16);
            header->sampleRate *= 10;
        } else {
            continue;  /* Invalid. Assume an invalid block. */
        }


        header->channelAssignment = channelAssignment;

        header->bitsPerSample = bitsPerSampleTable[bitsPerSample];
        if (header->bitsPerSample == 0) {
            header->bitsPerSample = streaminfoBitsPerSample;
        }

        if (!drflac__read_uint8(bs, 8, &header->crc8)) {
            return DRFLAC_FALSE;
        }

#ifndef DR_FLAC_NO_CRC
        if (header->crc8 != crc8) {
            continue;    /* CRC mismatch. Loop back to the top and find the next sync code. */
        }
#endif
        return DRFLAC_TRUE;
    }
}

static drflac_bool32 drflac__read_subframe_header(drflac_bs* bs, drflac_subframe* pSubframe)
{
    drflac_uint8 header;
    int type;

    if (!drflac__read_uint8(bs, 8, &header)) {
        return DRFLAC_FALSE;
    }

    /* First bit should always be 0. */
    if ((header & 0x80) != 0) {
        return DRFLAC_FALSE;
    }

    type = (header & 0x7E) >> 1;
    if (type == 0) {
        pSubframe->subframeType = DRFLAC_SUBFRAME_CONSTANT;
    } else if (type == 1) {
        pSubframe->subframeType = DRFLAC_SUBFRAME_VERBATIM;
    } else {
        if ((type & 0x20) != 0) {
            pSubframe->subframeType = DRFLAC_SUBFRAME_LPC;
            pSubframe->lpcOrder = (type & 0x1F) + 1;
        } else if ((type & 0x08) != 0) {
            pSubframe->subframeType = DRFLAC_SUBFRAME_FIXED;
            pSubframe->lpcOrder = (type & 0x07);
            if (pSubframe->lpcOrder > 4) {
                pSubframe->subframeType = DRFLAC_SUBFRAME_RESERVED;
                pSubframe->lpcOrder = 0;
            }
        } else {
            pSubframe->subframeType = DRFLAC_SUBFRAME_RESERVED;
        }
    }

    if (pSubframe->subframeType == DRFLAC_SUBFRAME_RESERVED) {
        return DRFLAC_FALSE;
    }

    /* Wasted bits per sample. */
    pSubframe->wastedBitsPerSample = 0;
    if ((header & 0x01) == 1) {
        unsigned int wastedBitsPerSample;
        if (!drflac__seek_past_next_set_bit(bs, &wastedBitsPerSample)) {
            return DRFLAC_FALSE;
        }
        pSubframe->wastedBitsPerSample = (unsigned char)wastedBitsPerSample + 1;
    }

    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__decode_subframe(drflac_bs* bs, drflac_frame* frame, int subframeIndex, drflac_int32* pDecodedSamplesOut)
{
    drflac_subframe* pSubframe;

    drflac_assert(bs != NULL);
    drflac_assert(frame != NULL);

    pSubframe = frame->subframes + subframeIndex;
    if (!drflac__read_subframe_header(bs, pSubframe)) {
        return DRFLAC_FALSE;
    }

    /* Side channels require an extra bit per sample. Took a while to figure that one out... */
    pSubframe->bitsPerSample = frame->header.bitsPerSample;
    if ((frame->header.channelAssignment == DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE || frame->header.channelAssignment == DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE) && subframeIndex == 1) {
        pSubframe->bitsPerSample += 1;
    } else if (frame->header.channelAssignment == DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE && subframeIndex == 0) {
        pSubframe->bitsPerSample += 1;
    }

    /* Need to handle wasted bits per sample. */
    if (pSubframe->wastedBitsPerSample >= pSubframe->bitsPerSample) {
        return DRFLAC_FALSE;
    }
    pSubframe->bitsPerSample -= pSubframe->wastedBitsPerSample;
    pSubframe->pDecodedSamples = pDecodedSamplesOut;

    switch (pSubframe->subframeType)
    {
        case DRFLAC_SUBFRAME_CONSTANT:
        {
            drflac__decode_samples__constant(bs, frame->header.blockSize, pSubframe->bitsPerSample, pSubframe->pDecodedSamples);
        } break;

        case DRFLAC_SUBFRAME_VERBATIM:
        {
            drflac__decode_samples__verbatim(bs, frame->header.blockSize, pSubframe->bitsPerSample, pSubframe->pDecodedSamples);
        } break;

        case DRFLAC_SUBFRAME_FIXED:
        {
            drflac__decode_samples__fixed(bs, frame->header.blockSize, pSubframe->bitsPerSample, pSubframe->lpcOrder, pSubframe->pDecodedSamples);
        } break;

        case DRFLAC_SUBFRAME_LPC:
        {
            drflac__decode_samples__lpc(bs, frame->header.blockSize, pSubframe->bitsPerSample, pSubframe->lpcOrder, pSubframe->pDecodedSamples);
        } break;

        default: return DRFLAC_FALSE;
    }

    return DRFLAC_TRUE;
}

static drflac_bool32 drflac__seek_subframe(drflac_bs* bs, drflac_frame* frame, int subframeIndex)
{
    drflac_subframe* pSubframe;

    drflac_assert(bs != NULL);
    drflac_assert(frame != NULL);

    pSubframe = frame->subframes + subframeIndex;
    if (!drflac__read_subframe_header(bs, pSubframe)) {
        return DRFLAC_FALSE;
    }

    /* Side channels require an extra bit per sample. Took a while to figure that one out... */
    pSubframe->bitsPerSample = frame->header.bitsPerSample;
    if ((frame->header.channelAssignment == DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE || frame->header.channelAssignment == DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE) && subframeIndex == 1) {
        pSubframe->bitsPerSample += 1;
    } else if (frame->header.channelAssignment == DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE && subframeIndex == 0) {
        pSubframe->bitsPerSample += 1;
    }

    /* Need to handle wasted bits per sample. */
    if (pSubframe->wastedBitsPerSample >= pSubframe->bitsPerSample) {
        return DRFLAC_FALSE;
    }
    pSubframe->bitsPerSample -= pSubframe->wastedBitsPerSample;
    pSubframe->pDecodedSamples = NULL;

    switch (pSubframe->subframeType)
    {
        case DRFLAC_SUBFRAME_CONSTANT:
        {
            if (!drflac__seek_bits(bs, pSubframe->bitsPerSample)) {
                return DRFLAC_FALSE;
            }
        } break;

        case DRFLAC_SUBFRAME_VERBATIM:
        {
            unsigned int bitsToSeek = frame->header.blockSize * pSubframe->bitsPerSample;
            if (!drflac__seek_bits(bs, bitsToSeek)) {
                return DRFLAC_FALSE;
            }
        } break;

        case DRFLAC_SUBFRAME_FIXED:
        {
            unsigned int bitsToSeek = pSubframe->lpcOrder * pSubframe->bitsPerSample;
            if (!drflac__seek_bits(bs, bitsToSeek)) {
                return DRFLAC_FALSE;
            }

            if (!drflac__read_and_seek_residual(bs, frame->header.blockSize, pSubframe->lpcOrder)) {
                return DRFLAC_FALSE;
            }
        } break;

        case DRFLAC_SUBFRAME_LPC:
        {
            unsigned char lpcPrecision;

            unsigned int bitsToSeek = pSubframe->lpcOrder * pSubframe->bitsPerSample;
            if (!drflac__seek_bits(bs, bitsToSeek)) {
                return DRFLAC_FALSE;
            }

            if (!drflac__read_uint8(bs, 4, &lpcPrecision)) {
                return DRFLAC_FALSE;
            }
            if (lpcPrecision == 15) {
                return DRFLAC_FALSE;    /* Invalid. */
            }
            lpcPrecision += 1;


            bitsToSeek = (pSubframe->lpcOrder * lpcPrecision) + 5;    /* +5 for shift. */
            if (!drflac__seek_bits(bs, bitsToSeek)) {
                return DRFLAC_FALSE;
            }

            if (!drflac__read_and_seek_residual(bs, frame->header.blockSize, pSubframe->lpcOrder)) {
                return DRFLAC_FALSE;
            }
        } break;

        default: return DRFLAC_FALSE;
    }

    return DRFLAC_TRUE;
}


static DRFLAC_INLINE drflac_uint8 drflac__get_channel_count_from_channel_assignment(drflac_int8 channelAssignment)
{
    drflac_uint8 lookup[] = {1, 2, 3, 4, 5, 6, 7, 8, 2, 2, 2};

    drflac_assert(channelAssignment <= 10);
    return lookup[channelAssignment];
}

static drflac_result drflac__decode_flac_frame(drflac* pFlac)
{
    int channelCount;
    int i;
    drflac_uint8 paddingSizeInBits;
    drflac_uint16 desiredCRC16;
#ifndef DR_FLAC_NO_CRC
    drflac_uint16 actualCRC16;
#endif

    /* This function should be called while the stream is sitting on the first byte after the frame header. */
    drflac_zero_memory(pFlac->currentFrame.subframes, sizeof(pFlac->currentFrame.subframes));

    /* The frame block size must never be larger than the maximum block size defined by the FLAC stream. */
    if (pFlac->currentFrame.header.blockSize > pFlac->maxBlockSize) {
        return DRFLAC_ERROR;
    }

    /* The number of channels in the frame must match the channel count from the STREAMINFO block. */
    channelCount = drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
    if (channelCount != (int)pFlac->channels) {
        return DRFLAC_ERROR;
    }

    for (i = 0; i < channelCount; ++i) {
        if (!drflac__decode_subframe(&pFlac->bs, &pFlac->currentFrame, i, pFlac->pDecodedSamples + ((pFlac->currentFrame.header.blockSize+DRFLAC_LEADING_SAMPLES) * i) + DRFLAC_LEADING_SAMPLES)) {
            return DRFLAC_ERROR;
        }
    }

    paddingSizeInBits = DRFLAC_CACHE_L1_BITS_REMAINING(&pFlac->bs) & 7;
    if (paddingSizeInBits > 0) {
        drflac_uint8 padding = 0;
        if (!drflac__read_uint8(&pFlac->bs, paddingSizeInBits, &padding)) {
            return DRFLAC_END_OF_STREAM;
        }
    }

#ifndef DR_FLAC_NO_CRC
    actualCRC16 = drflac__flush_crc16(&pFlac->bs);
#endif
    if (!drflac__read_uint16(&pFlac->bs, 16, &desiredCRC16)) {
        return DRFLAC_END_OF_STREAM;
    }

#ifndef DR_FLAC_NO_CRC
    if (actualCRC16 != desiredCRC16) {
        return DRFLAC_CRC_MISMATCH;    /* CRC mismatch. */
    }
#endif

    pFlac->currentFrame.samplesRemaining = pFlac->currentFrame.header.blockSize * channelCount;

    return DRFLAC_SUCCESS;
}

static drflac_result drflac__seek_flac_frame(drflac* pFlac)
{
    int channelCount;
    int i;
    drflac_uint16 desiredCRC16;
#ifndef DR_FLAC_NO_CRC
    drflac_uint16 actualCRC16;
#endif

    channelCount = drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
    for (i = 0; i < channelCount; ++i) {
        if (!drflac__seek_subframe(&pFlac->bs, &pFlac->currentFrame, i)) {
            return DRFLAC_ERROR;
        }
    }

    /* Padding. */
    if (!drflac__seek_bits(&pFlac->bs, DRFLAC_CACHE_L1_BITS_REMAINING(&pFlac->bs) & 7)) {
        return DRFLAC_ERROR;
    }

    /* CRC. */
#ifndef DR_FLAC_NO_CRC
    actualCRC16 = drflac__flush_crc16(&pFlac->bs);
#endif
    if (!drflac__read_uint16(&pFlac->bs, 16, &desiredCRC16)) {
        return DRFLAC_END_OF_STREAM;
    }

#ifndef DR_FLAC_NO_CRC
    if (actualCRC16 != desiredCRC16) {
        return DRFLAC_CRC_MISMATCH;    /* CRC mismatch. */
    }
#endif

    return DRFLAC_SUCCESS;
}

static drflac_bool32 drflac__read_and_decode_next_flac_frame(drflac* pFlac)
{
    drflac_assert(pFlac != NULL);

    for (;;) {
        drflac_result result;

        if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
            return DRFLAC_FALSE;
        }

        result = drflac__decode_flac_frame(pFlac);
        if (result != DRFLAC_SUCCESS) {
            if (result == DRFLAC_CRC_MISMATCH) {
                continue;   /* CRC mismatch. Skip to the next frame. */
            } else {
                return DRFLAC_FALSE;
            }
        }

        return DRFLAC_TRUE;
    }
}


static void drflac__get_current_frame_sample_range(drflac* pFlac, drflac_uint64* pFirstSampleInFrameOut, drflac_uint64* pLastSampleInFrameOut)
{
    unsigned int channelCount;
    drflac_uint64 firstSampleInFrame;
    drflac_uint64 lastSampleInFrame;

    drflac_assert(pFlac != NULL);

    channelCount = drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);

    firstSampleInFrame = pFlac->currentFrame.header.sampleNumber*channelCount;
    if (firstSampleInFrame == 0) {
        firstSampleInFrame = pFlac->currentFrame.header.frameNumber * pFlac->maxBlockSize*channelCount;
    }

    lastSampleInFrame = firstSampleInFrame + (pFlac->currentFrame.header.blockSize*channelCount);
    if (lastSampleInFrame > 0) {
        lastSampleInFrame -= 1; /* Needs to be zero based. */
    }

    if (pFirstSampleInFrameOut) {
        *pFirstSampleInFrameOut = firstSampleInFrame;
    }
    if (pLastSampleInFrameOut) {
        *pLastSampleInFrameOut = lastSampleInFrame;
    }
}

/* This function will be replacing drflac__get_current_frame_sample_range(), but it's not currently used so I have commented it out to silence a compiler warning. */
#if 0
static void drflac__get_pcm_frame_range_of_current_flac_frame(drflac* pFlac, drflac_uint64* pFirstPCMFrame, drflac_uint64* pLastPCMFrame)
{
    drflac_uint64 firstPCMFrame;
    drflac_uint64 lastPCMFrame;

    drflac_assert(pFlac != NULL);

    firstPCMFrame = pFlac->currentFrame.header.sampleNumber;
    if (firstPCMFrame == 0) {
        firstPCMFrame = pFlac->currentFrame.header.frameNumber * pFlac->maxBlockSize;
    }

    lastPCMFrame = firstPCMFrame + (pFlac->currentFrame.header.blockSize);
    if (lastPCMFrame > 0) {
        lastPCMFrame -= 1; /* Needs to be zero based. */
    }

    if (pFirstPCMFrame) {
        *pFirstPCMFrame = firstPCMFrame;
    }
    if (pLastPCMFrame) {
        *pLastPCMFrame = lastPCMFrame;
    }
}
#endif

static drflac_bool32 drflac__seek_to_first_frame(drflac* pFlac)
{
    drflac_bool32 result;

    drflac_assert(pFlac != NULL);

    result = drflac__seek_to_byte(&pFlac->bs, pFlac->firstFramePos);

    drflac_zero_memory(&pFlac->currentFrame, sizeof(pFlac->currentFrame));
    pFlac->currentSample = 0;

    return result;
}

static DRFLAC_INLINE drflac_result drflac__seek_to_next_flac_frame(drflac* pFlac)
{
    /* This function should only ever be called while the decoder is sitting on the first byte past the FRAME_HEADER section. */
    drflac_assert(pFlac != NULL);
    return drflac__seek_flac_frame(pFlac);
}

drflac_uint64 drflac__seek_forward_by_samples(drflac* pFlac, drflac_uint64 samplesToRead)
{
    drflac_uint64 samplesRead = 0;
    while (samplesToRead > 0) {
        if (pFlac->currentFrame.samplesRemaining == 0) {
            if (!drflac__read_and_decode_next_flac_frame(pFlac)) {
                break;  /* Couldn't read the next frame, so just break from the loop and return. */
            }
        } else {
            if (pFlac->currentFrame.samplesRemaining > samplesToRead) {
                samplesRead   += samplesToRead;
                pFlac->currentFrame.samplesRemaining -= (drflac_uint32)samplesToRead;   /* <-- Safe cast. Will always be < currentFrame.samplesRemaining < 65536. */
                samplesToRead  = 0;
            } else {
                samplesRead   += pFlac->currentFrame.samplesRemaining;
                samplesToRead -= pFlac->currentFrame.samplesRemaining;
                pFlac->currentFrame.samplesRemaining = 0;
            }
        }
    }

    pFlac->currentSample += samplesRead;
    return samplesRead;
}

drflac_uint64 drflac__seek_forward_by_pcm_frames(drflac* pFlac, drflac_uint64 pcmFramesToSeek)
{
    return drflac__seek_forward_by_samples(pFlac, pcmFramesToSeek*pFlac->channels);
}

static drflac_bool32 drflac__seek_to_sample__brute_force(drflac* pFlac, drflac_uint64 sampleIndex)
{
    drflac_bool32 isMidFrame = DRFLAC_FALSE;
    drflac_uint64 runningSampleCount;

    drflac_assert(pFlac != NULL);

    /* If we are seeking forward we start from the current position. Otherwise we need to start all the way from the start of the file. */
    if (sampleIndex >= pFlac->currentSample) {
        /* Seeking forward. Need to seek from the current position. */
        runningSampleCount = pFlac->currentSample;

        /* The frame header for the first frame may not yet have been read. We need to do that if necessary. */
        if (pFlac->currentSample == 0 && pFlac->currentFrame.samplesRemaining == 0) {
            if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
                return DRFLAC_FALSE;
            }
        } else {
            isMidFrame = DRFLAC_TRUE;
        }
    } else {
        /* Seeking backwards. Need to seek from the start of the file. */
        runningSampleCount = 0;

        /* Move back to the start. */
        if (!drflac__seek_to_first_frame(pFlac)) {
            return DRFLAC_FALSE;
        }

        /* Decode the first frame in preparation for sample-exact seeking below. */
        if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
            return DRFLAC_FALSE;
        }
    }

    /*
    We need to as quickly as possible find the frame that contains the target sample. To do this, we iterate over each frame and inspect its
    header. If based on the header we can determine that the frame contains the sample, we do a full decode of that frame.
    */
    for (;;) {
        drflac_uint64 sampleCountInThisFrame;
        drflac_uint64 firstSampleInFrame = 0;
        drflac_uint64 lastSampleInFrame = 0;

        drflac__get_current_frame_sample_range(pFlac, &firstSampleInFrame, &lastSampleInFrame);

        sampleCountInThisFrame = (lastSampleInFrame - firstSampleInFrame) + 1;
        if (sampleIndex < (runningSampleCount + sampleCountInThisFrame)) {
            /*
            The sample should be in this frame. We need to fully decode it, however if it's an invalid frame (a CRC mismatch), we need to pretend
            it never existed and keep iterating.
            */
            drflac_uint64 samplesToDecode = sampleIndex - runningSampleCount;

            if (!isMidFrame) {
                drflac_result result = drflac__decode_flac_frame(pFlac);
                if (result == DRFLAC_SUCCESS) {
                    /* The frame is valid. We just need to skip over some samples to ensure it's sample-exact. */
                    return drflac__seek_forward_by_samples(pFlac, samplesToDecode) == samplesToDecode;  /* <-- If this fails, something bad has happened (it should never fail). */
                } else {
                    if (result == DRFLAC_CRC_MISMATCH) {
                        goto next_iteration;   /* CRC mismatch. Pretend this frame never existed. */
                    } else {
                        return DRFLAC_FALSE;
                    }
                }
            } else {
                /* We started seeking mid-frame which means we need to skip the frame decoding part. */
                return drflac__seek_forward_by_samples(pFlac, samplesToDecode) == samplesToDecode;
            }
        } else {
            /*
            It's not in this frame. We need to seek past the frame, but check if there was a CRC mismatch. If so, we pretend this
            frame never existed and leave the running sample count untouched.
            */
            if (!isMidFrame) {
                drflac_result result = drflac__seek_to_next_flac_frame(pFlac);
                if (result == DRFLAC_SUCCESS) {
                    runningSampleCount += sampleCountInThisFrame;
                } else {
                    if (result == DRFLAC_CRC_MISMATCH) {
                        goto next_iteration;   /* CRC mismatch. Pretend this frame never existed. */
                    } else {
                        return DRFLAC_FALSE;
                    }
                }
            } else {
                /*
                We started seeking mid-frame which means we need to seek by reading to the end of the frame instead of with
                drflac__seek_to_next_flac_frame() which only works if the decoder is sitting on the byte just after the frame header.
                */
                runningSampleCount += pFlac->currentFrame.samplesRemaining;
                pFlac->currentFrame.samplesRemaining = 0;
                isMidFrame = DRFLAC_FALSE;
            }
        }

    next_iteration:
        /* Grab the next frame in preparation for the next iteration. */
        if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
            return DRFLAC_FALSE;
        }
    }
}


static drflac_bool32 drflac__seek_to_sample__seek_table(drflac* pFlac, drflac_uint64 sampleIndex)
{
    drflac_uint32 iClosestSeekpoint = 0;
    drflac_bool32 isMidFrame = DRFLAC_FALSE;
    drflac_uint64 runningSampleCount;
    drflac_uint32 iSeekpoint;

    drflac_assert(pFlac != NULL);

    if (pFlac->pSeekpoints == NULL || pFlac->seekpointCount == 0) {
        return DRFLAC_FALSE;
    }

    for (iSeekpoint = 0; iSeekpoint < pFlac->seekpointCount; ++iSeekpoint) {
        if (pFlac->pSeekpoints[iSeekpoint].firstSample*pFlac->channels >= sampleIndex) {
            break;
        }

        iClosestSeekpoint = iSeekpoint;
    }

    /*
    At this point we should have found the seekpoint closest to our sample. If we are seeking forward and the closest seekpoint is _before_ the current sample, we
    just seek forward from where we are. Otherwise we start seeking from the seekpoint's first sample.
    */
    if ((sampleIndex >= pFlac->currentSample) && (pFlac->pSeekpoints[iClosestSeekpoint].firstSample*pFlac->channels <= pFlac->currentSample)) {
        /* Optimized case. Just seek forward from where we are. */
        runningSampleCount = pFlac->currentSample;

        /* The frame header for the first frame may not yet have been read. We need to do that if necessary. */
        if (pFlac->currentSample == 0 && pFlac->currentFrame.samplesRemaining == 0) {
            if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
                return DRFLAC_FALSE;
            }
        } else {
            isMidFrame = DRFLAC_TRUE;
        }
    } else {
        /* Slower case. Seek to the start of the seekpoint and then seek forward from there. */
        runningSampleCount = pFlac->pSeekpoints[iClosestSeekpoint].firstSample*pFlac->channels;

        if (!drflac__seek_to_byte(&pFlac->bs, pFlac->firstFramePos + pFlac->pSeekpoints[iClosestSeekpoint].frameOffset)) {
            return DRFLAC_FALSE;
        }

        /* Grab the frame the seekpoint is sitting on in preparation for the sample-exact seeking below. */
        if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
            return DRFLAC_FALSE;
        }
    }

    for (;;) {
        drflac_uint64 sampleCountInThisFrame;
        drflac_uint64 firstSampleInFrame = 0;
        drflac_uint64 lastSampleInFrame = 0;
        drflac__get_current_frame_sample_range(pFlac, &firstSampleInFrame, &lastSampleInFrame);

        sampleCountInThisFrame = (lastSampleInFrame - firstSampleInFrame) + 1;
        if (sampleIndex < (runningSampleCount + sampleCountInThisFrame)) {
            /*
            The sample should be in this frame. We need to fully decode it, but if it's an invalid frame (a CRC mismatch) we need to pretend
            it never existed and keep iterating.
            */
            drflac_uint64 samplesToDecode = sampleIndex - runningSampleCount;

            if (!isMidFrame) {
                drflac_result result = drflac__decode_flac_frame(pFlac);
                if (result == DRFLAC_SUCCESS) {
                    /* The frame is valid. We just need to skip over some samples to ensure it's sample-exact. */
                    return drflac__seek_forward_by_samples(pFlac, samplesToDecode) == samplesToDecode;  /* <-- If this fails, something bad has happened (it should never fail). */
                } else {
                    if (result == DRFLAC_CRC_MISMATCH) {
                        goto next_iteration;   /* CRC mismatch. Pretend this frame never existed. */
                    } else {
                        return DRFLAC_FALSE;
                    }
                }
            } else {
                /* We started seeking mid-frame which means we need to skip the frame decoding part. */
                return drflac__seek_forward_by_samples(pFlac, samplesToDecode) == samplesToDecode;
            }
        } else {
            /*
            It's not in this frame. We need to seek past the frame, but check if there was a CRC mismatch. If so, we pretend this
            frame never existed and leave the running sample count untouched.
            */
            if (!isMidFrame) {
                drflac_result result = drflac__seek_to_next_flac_frame(pFlac);
                if (result == DRFLAC_SUCCESS) {
                    runningSampleCount += sampleCountInThisFrame;
                } else {
                    if (result == DRFLAC_CRC_MISMATCH) {
                        goto next_iteration;   /* CRC mismatch. Pretend this frame never existed. */
                    } else {
                        return DRFLAC_FALSE;
                    }
                }
            } else {
                /*
                We started seeking mid-frame which means we need to seek by reading to the end of the frame instead of with
                drflac__seek_to_next_flac_frame() which only works if the decoder is sitting on the byte just after the frame header.
                */
                runningSampleCount += pFlac->currentFrame.samplesRemaining;
                pFlac->currentFrame.samplesRemaining = 0;
                isMidFrame = DRFLAC_FALSE;
            }
        }

    next_iteration:
        /* Grab the next frame in preparation for the next iteration. */
        if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
            return DRFLAC_FALSE;
        }
    }
}


#ifndef DR_FLAC_NO_OGG
typedef struct
{
    drflac_uint8 capturePattern[4];  /* Should be "OggS" */
    drflac_uint8 structureVersion;   /* Always 0. */
    drflac_uint8 headerType;
    drflac_uint64 granulePosition;
    drflac_uint32 serialNumber;
    drflac_uint32 sequenceNumber;
    drflac_uint32 checksum;
    drflac_uint8 segmentCount;
    drflac_uint8 segmentTable[255];
} drflac_ogg_page_header;
#endif

typedef struct
{
    drflac_read_proc onRead;
    drflac_seek_proc onSeek;
    drflac_meta_proc onMeta;
    drflac_container container;
    void* pUserData;
    void* pUserDataMD;
    drflac_uint32 sampleRate;
    drflac_uint8  channels;
    drflac_uint8  bitsPerSample;
    drflac_uint64 totalSampleCount;
    drflac_uint16 maxBlockSize;
    drflac_uint64 runningFilePos;
    drflac_bool32 hasStreamInfoBlock;
    drflac_bool32 hasMetadataBlocks;
    drflac_bs bs;                           /* <-- A bit streamer is required for loading data during initialization. */
    drflac_frame_header firstFrameHeader;   /* <-- The header of the first frame that was read during relaxed initalization. Only set if there is no STREAMINFO block. */

#ifndef DR_FLAC_NO_OGG
    drflac_uint32 oggSerial;
    drflac_uint64 oggFirstBytePos;
    drflac_ogg_page_header oggBosHeader;
#endif
} drflac_init_info;

static DRFLAC_INLINE void drflac__decode_block_header(drflac_uint32 blockHeader, drflac_uint8* isLastBlock, drflac_uint8* blockType, drflac_uint32* blockSize)
{
    blockHeader = drflac__be2host_32(blockHeader);
    *isLastBlock = (blockHeader & 0x80000000UL) >> 31;
    *blockType   = (blockHeader & 0x7F000000UL) >> 24;
    *blockSize   = (blockHeader & 0x00FFFFFFUL);
}

static DRFLAC_INLINE drflac_bool32 drflac__read_and_decode_block_header(drflac_read_proc onRead, void* pUserData, drflac_uint8* isLastBlock, drflac_uint8* blockType, drflac_uint32* blockSize)
{
    drflac_uint32 blockHeader;
    if (onRead(pUserData, &blockHeader, 4) != 4) {
        return DRFLAC_FALSE;
    }

    drflac__decode_block_header(blockHeader, isLastBlock, blockType, blockSize);
    return DRFLAC_TRUE;
}

drflac_bool32 drflac__read_streaminfo(drflac_read_proc onRead, void* pUserData, drflac_streaminfo* pStreamInfo)
{
    drflac_uint32 blockSizes;
    drflac_uint64 frameSizes = 0;
    drflac_uint64 importantProps;
    drflac_uint8 md5[16];

    /* min/max block size. */
    if (onRead(pUserData, &blockSizes, 4) != 4) {
        return DRFLAC_FALSE;
    }

    /* min/max frame size. */
    if (onRead(pUserData, &frameSizes, 6) != 6) {
        return DRFLAC_FALSE;
    }

    /* Sample rate, channels, bits per sample and total sample count. */
    if (onRead(pUserData, &importantProps, 8) != 8) {
        return DRFLAC_FALSE;
    }

    /* MD5 */
    if (onRead(pUserData, md5, sizeof(md5)) != sizeof(md5)) {
        return DRFLAC_FALSE;
    }

    blockSizes     = drflac__be2host_32(blockSizes);
    frameSizes     = drflac__be2host_64(frameSizes);
    importantProps = drflac__be2host_64(importantProps);

    pStreamInfo->minBlockSize     = (blockSizes & 0xFFFF0000) >> 16;
    pStreamInfo->maxBlockSize     = (blockSizes & 0x0000FFFF);
    pStreamInfo->minFrameSize     = (drflac_uint32)((frameSizes     &  (((drflac_uint64)0x00FFFFFF << 16) << 24)) >> 40);
    pStreamInfo->maxFrameSize     = (drflac_uint32)((frameSizes     &  (((drflac_uint64)0x00FFFFFF << 16) <<  0)) >> 16);
    pStreamInfo->sampleRate       = (drflac_uint32)((importantProps &  (((drflac_uint64)0x000FFFFF << 16) << 28)) >> 44);
    pStreamInfo->channels         = (drflac_uint8 )((importantProps &  (((drflac_uint64)0x0000000E << 16) << 24)) >> 41) + 1;
    pStreamInfo->bitsPerSample    = (drflac_uint8 )((importantProps &  (((drflac_uint64)0x0000001F << 16) << 20)) >> 36) + 1;
    pStreamInfo->totalSampleCount =                ((importantProps & ((((drflac_uint64)0x0000000F << 16) << 16) | 0xFFFFFFFF))) * pStreamInfo->channels;
    drflac_copy_memory(pStreamInfo->md5, md5, sizeof(md5));

    return DRFLAC_TRUE;
}

drflac_bool32 drflac__read_and_decode_metadata(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, void* pUserData, void* pUserDataMD, drflac_uint64* pFirstFramePos, drflac_uint64* pSeektablePos, drflac_uint32* pSeektableSize)
{
    /*
    We want to keep track of the byte position in the stream of the seektable. At the time of calling this function we know that
    we'll be sitting on byte 42.
    */
    drflac_uint64 runningFilePos = 42;
    drflac_uint64 seektablePos   = 0;
    drflac_uint32 seektableSize  = 0;

    for (;;) {
        drflac_metadata metadata;
        drflac_uint8 isLastBlock = 0;
        drflac_uint8 blockType;
        drflac_uint32 blockSize;
        if (!drflac__read_and_decode_block_header(onRead, pUserData, &isLastBlock, &blockType, &blockSize)) {
            return DRFLAC_FALSE;
        }
        runningFilePos += 4;

        metadata.type = blockType;
        metadata.pRawData = NULL;
        metadata.rawDataSize = 0;

        switch (blockType)
        {
            case DRFLAC_METADATA_BLOCK_TYPE_APPLICATION:
            {
                if (blockSize < 4) {
                    return DRFLAC_FALSE;
                }

                if (onMeta) {
                    void* pRawData = DRFLAC_MALLOC(blockSize);
                    if (pRawData == NULL) {
                        return DRFLAC_FALSE;
                    }

                    if (onRead(pUserData, pRawData, blockSize) != blockSize) {
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    metadata.pRawData = pRawData;
                    metadata.rawDataSize = blockSize;
                    metadata.data.application.id       = drflac__be2host_32(*(drflac_uint32*)pRawData);
                    metadata.data.application.pData    = (const void*)((drflac_uint8*)pRawData + sizeof(drflac_uint32));
                    metadata.data.application.dataSize = blockSize - sizeof(drflac_uint32);
                    onMeta(pUserDataMD, &metadata);

                    DRFLAC_FREE(pRawData);
                }
            } break;

            case DRFLAC_METADATA_BLOCK_TYPE_SEEKTABLE:
            {
                seektablePos  = runningFilePos;
                seektableSize = blockSize;

                if (onMeta) {
                    drflac_uint32 iSeekpoint;
                    void* pRawData;

                    pRawData = DRFLAC_MALLOC(blockSize);
                    if (pRawData == NULL) {
                        return DRFLAC_FALSE;
                    }

                    if (onRead(pUserData, pRawData, blockSize) != blockSize) {
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    metadata.pRawData = pRawData;
                    metadata.rawDataSize = blockSize;
                    metadata.data.seektable.seekpointCount = blockSize/sizeof(drflac_seekpoint);
                    metadata.data.seektable.pSeekpoints = (const drflac_seekpoint*)pRawData;

                    /* Endian swap. */
                    for (iSeekpoint = 0; iSeekpoint < metadata.data.seektable.seekpointCount; ++iSeekpoint) {
                        drflac_seekpoint* pSeekpoint = (drflac_seekpoint*)pRawData + iSeekpoint;
                        pSeekpoint->firstSample = drflac__be2host_64(pSeekpoint->firstSample);
                        pSeekpoint->frameOffset = drflac__be2host_64(pSeekpoint->frameOffset);
                        pSeekpoint->sampleCount = drflac__be2host_16(pSeekpoint->sampleCount);
                    }

                    onMeta(pUserDataMD, &metadata);

                    DRFLAC_FREE(pRawData);
                }
            } break;

            case DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT:
            {
                if (blockSize < 8) {
                    return DRFLAC_FALSE;
                }

                if (onMeta) {
                    void* pRawData;
                    const char* pRunningData;
                    const char* pRunningDataEnd;
                    drflac_uint32 i;

                    pRawData = DRFLAC_MALLOC(blockSize);
                    if (pRawData == NULL) {
                        return DRFLAC_FALSE;
                    }

                    if (onRead(pUserData, pRawData, blockSize) != blockSize) {
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    metadata.pRawData = pRawData;
                    metadata.rawDataSize = blockSize;

                    pRunningData    = (const char*)pRawData;
                    pRunningDataEnd = (const char*)pRawData + blockSize;

                    metadata.data.vorbis_comment.vendorLength = drflac__le2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;

                    /* Need space for the rest of the block */
                    if ((pRunningDataEnd - pRunningData) - 4 < (drflac_int64)metadata.data.vorbis_comment.vendorLength) { /* <-- Note the order of operations to avoid overflow to a valid value */
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }
                    metadata.data.vorbis_comment.vendor       = pRunningData;                                            pRunningData += metadata.data.vorbis_comment.vendorLength;
                    metadata.data.vorbis_comment.commentCount = drflac__le2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;

                    /* Need space for 'commentCount' comments after the block, which at minimum is a drflac_uint32 per comment */
                    if ((pRunningDataEnd - pRunningData) / sizeof(drflac_uint32) < metadata.data.vorbis_comment.commentCount) { /* <-- Note the order of operations to avoid overflow to a valid value */
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }
                    metadata.data.vorbis_comment.pComments    = pRunningData;

                    /* Check that the comments section is valid before passing it to the callback */
                    for (i = 0; i < metadata.data.vorbis_comment.commentCount; ++i) {
                        drflac_uint32 commentLength;

                        if (pRunningDataEnd - pRunningData < 4) {
                            DRFLAC_FREE(pRawData);
                            return DRFLAC_FALSE;
                        }

                        commentLength = drflac__le2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                        if (pRunningDataEnd - pRunningData < (drflac_int64)commentLength) { /* <-- Note the order of operations to avoid overflow to a valid value */
                            DRFLAC_FREE(pRawData);
                            return DRFLAC_FALSE;
                        }
                        pRunningData += commentLength;
                    }

                    onMeta(pUserDataMD, &metadata);

                    DRFLAC_FREE(pRawData);
                }
            } break;

            case DRFLAC_METADATA_BLOCK_TYPE_CUESHEET:
            {
                if (blockSize < 396) {
                    return DRFLAC_FALSE;
                }

                if (onMeta) {
                    void* pRawData;
                    const char* pRunningData;
                    const char* pRunningDataEnd;
                    drflac_uint8 iTrack;
                    drflac_uint8 iIndex;

                    pRawData = DRFLAC_MALLOC(blockSize);
                    if (pRawData == NULL) {
                        return DRFLAC_FALSE;
                    }

                    if (onRead(pUserData, pRawData, blockSize) != blockSize) {
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    metadata.pRawData = pRawData;
                    metadata.rawDataSize = blockSize;

                    pRunningData    = (const char*)pRawData;
                    pRunningDataEnd = (const char*)pRawData + blockSize;

                    drflac_copy_memory(metadata.data.cuesheet.catalog, pRunningData, 128);                              pRunningData += 128;
                    metadata.data.cuesheet.leadInSampleCount = drflac__be2host_64(*(const drflac_uint64*)pRunningData); pRunningData += 8;
                    metadata.data.cuesheet.isCD              = (pRunningData[0] & 0x80) != 0;                           pRunningData += 259;
                    metadata.data.cuesheet.trackCount        = pRunningData[0];                                         pRunningData += 1;
                    metadata.data.cuesheet.pTrackData        = pRunningData;

                    /* Check that the cuesheet tracks are valid before passing it to the callback */
                    for (iTrack = 0; iTrack < metadata.data.cuesheet.trackCount; ++iTrack) {
                        drflac_uint8 indexCount;
                        drflac_uint32 indexPointSize;

                        if (pRunningDataEnd - pRunningData < 36) {
                            DRFLAC_FREE(pRawData);
                            return DRFLAC_FALSE;
                        }

                        /* Skip to the index point count */
                        pRunningData += 35;
                        indexCount = pRunningData[0]; pRunningData += 1;
                        indexPointSize = indexCount * sizeof(drflac_cuesheet_track_index);
                        if (pRunningDataEnd - pRunningData < (drflac_int64)indexPointSize) {
                            DRFLAC_FREE(pRawData);
                            return DRFLAC_FALSE;
                        }

                        /* Endian swap. */
                        for (iIndex = 0; iIndex < indexCount; ++iIndex) {
                            drflac_cuesheet_track_index* pTrack = (drflac_cuesheet_track_index*)pRunningData;
                            pRunningData += sizeof(drflac_cuesheet_track_index);
                            pTrack->offset = drflac__be2host_64(pTrack->offset);
                        }
                    }

                    onMeta(pUserDataMD, &metadata);

                    DRFLAC_FREE(pRawData);
                }
            } break;

            case DRFLAC_METADATA_BLOCK_TYPE_PICTURE:
            {
                if (blockSize < 32) {
                    return DRFLAC_FALSE;
                }

                if (onMeta) {
                    void* pRawData;
                    const char* pRunningData;
                    const char* pRunningDataEnd;

                    pRawData = DRFLAC_MALLOC(blockSize);
                    if (pRawData == NULL) {
                        return DRFLAC_FALSE;
                    }

                    if (onRead(pUserData, pRawData, blockSize) != blockSize) {
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    metadata.pRawData = pRawData;
                    metadata.rawDataSize = blockSize;

                    pRunningData    = (const char*)pRawData;
                    pRunningDataEnd = (const char*)pRawData + blockSize;

                    metadata.data.picture.type       = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                    metadata.data.picture.mimeLength = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;

                    /* Need space for the rest of the block */
                    if ((pRunningDataEnd - pRunningData) - 24 < (drflac_int64)metadata.data.picture.mimeLength) { /* <-- Note the order of operations to avoid overflow to a valid value */
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }
                    metadata.data.picture.mime              = pRunningData;                                            pRunningData += metadata.data.picture.mimeLength;
                    metadata.data.picture.descriptionLength = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;

                    /* Need space for the rest of the block */
                    if ((pRunningDataEnd - pRunningData) - 20 < (drflac_int64)metadata.data.picture.descriptionLength) { /* <-- Note the order of operations to avoid overflow to a valid value */
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }
                    metadata.data.picture.description     = pRunningData;                                            pRunningData += metadata.data.picture.descriptionLength;
                    metadata.data.picture.width           = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                    metadata.data.picture.height          = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                    metadata.data.picture.colorDepth      = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                    metadata.data.picture.indexColorCount = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                    metadata.data.picture.pictureDataSize = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
                    metadata.data.picture.pPictureData    = (const drflac_uint8*)pRunningData;

                    /* Need space for the picture after the block */
                    if (pRunningDataEnd - pRunningData < (drflac_int64)metadata.data.picture.pictureDataSize) { /* <-- Note the order of operations to avoid overflow to a valid value */
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    onMeta(pUserDataMD, &metadata);

                    DRFLAC_FREE(pRawData);
                }
            } break;

            case DRFLAC_METADATA_BLOCK_TYPE_PADDING:
            {
                if (onMeta) {
                    metadata.data.padding.unused = 0;

                    /* Padding doesn't have anything meaningful in it, so just skip over it, but make sure the caller is aware of it by firing the callback. */
                    if (!onSeek(pUserData, blockSize, drflac_seek_origin_current)) {
                        isLastBlock = DRFLAC_TRUE;  /* An error occurred while seeking. Attempt to recover by treating this as the last block which will in turn terminate the loop. */
                    } else {
                        onMeta(pUserDataMD, &metadata);
                    }
                }
            } break;

            case DRFLAC_METADATA_BLOCK_TYPE_INVALID:
            {
                /* Invalid chunk. Just skip over this one. */
                if (onMeta) {
                    if (!onSeek(pUserData, blockSize, drflac_seek_origin_current)) {
                        isLastBlock = DRFLAC_TRUE;  /* An error occurred while seeking. Attempt to recover by treating this as the last block which will in turn terminate the loop. */
                    }
                }
            } break;

            default:
            {
                /*
                It's an unknown chunk, but not necessarily invalid. There's a chance more metadata blocks might be defined later on, so we
                can at the very least report the chunk to the application and let it look at the raw data.
                */
                if (onMeta) {
                    void* pRawData = DRFLAC_MALLOC(blockSize);
                    if (pRawData == NULL) {
                        return DRFLAC_FALSE;
                    }

                    if (onRead(pUserData, pRawData, blockSize) != blockSize) {
                        DRFLAC_FREE(pRawData);
                        return DRFLAC_FALSE;
                    }

                    metadata.pRawData = pRawData;
                    metadata.rawDataSize = blockSize;
                    onMeta(pUserDataMD, &metadata);

                    DRFLAC_FREE(pRawData);
                }
            } break;
        }

        /* If we're not handling metadata, just skip over the block. If we are, it will have been handled earlier in the switch statement above. */
        if (onMeta == NULL && blockSize > 0) {
            if (!onSeek(pUserData, blockSize, drflac_seek_origin_current)) {
                isLastBlock = DRFLAC_TRUE;
            }
        }

        runningFilePos += blockSize;
        if (isLastBlock) {
            break;
        }
    }

    *pSeektablePos = seektablePos;
    *pSeektableSize = seektableSize;
    *pFirstFramePos = runningFilePos;

    return DRFLAC_TRUE;
}

drflac_bool32 drflac__init_private__native(drflac_init_info* pInit, drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, void* pUserData, void* pUserDataMD, drflac_bool32 relaxed)
{
    /* Pre Condition: The bit stream should be sitting just past the 4-byte id header. */

    drflac_uint8 isLastBlock;
    drflac_uint8 blockType;
    drflac_uint32 blockSize;

    (void)onSeek;

    pInit->container = drflac_container_native;

    /* The first metadata block should be the STREAMINFO block. */
    if (!drflac__read_and_decode_block_header(onRead, pUserData, &isLastBlock, &blockType, &blockSize)) {
        return DRFLAC_FALSE;
    }

    if (blockType != DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO || blockSize != 34) {
        if (!relaxed) {
            /* We're opening in strict mode and the first block is not the STREAMINFO block. Error. */
            return DRFLAC_FALSE;
        } else {
            /*
            Relaxed mode. To open from here we need to just find the first frame and set the sample rate, etc. to whatever is defined
            for that frame.
            */
            pInit->hasStreamInfoBlock = DRFLAC_FALSE;
            pInit->hasMetadataBlocks  = DRFLAC_FALSE;

            if (!drflac__read_next_flac_frame_header(&pInit->bs, 0, &pInit->firstFrameHeader)) {
                return DRFLAC_FALSE;    /* Couldn't find a frame. */
            }

            if (pInit->firstFrameHeader.bitsPerSample == 0) {
                return DRFLAC_FALSE;    /* Failed to initialize because the first frame depends on the STREAMINFO block, which does not exist. */
            }

            pInit->sampleRate    = pInit->firstFrameHeader.sampleRate;
            pInit->channels      = drflac__get_channel_count_from_channel_assignment(pInit->firstFrameHeader.channelAssignment);
            pInit->bitsPerSample = pInit->firstFrameHeader.bitsPerSample;
            pInit->maxBlockSize  = 65535;   /* <-- See notes here: https://xiph.org/flac/format.html#metadata_block_streaminfo */
            return DRFLAC_TRUE;
        }
    } else {
        drflac_streaminfo streaminfo;
        if (!drflac__read_streaminfo(onRead, pUserData, &streaminfo)) {
            return DRFLAC_FALSE;
        }

        pInit->hasStreamInfoBlock = DRFLAC_TRUE;
        pInit->sampleRate         = streaminfo.sampleRate;
        pInit->channels           = streaminfo.channels;
        pInit->bitsPerSample      = streaminfo.bitsPerSample;
        pInit->totalSampleCount   = streaminfo.totalSampleCount;
        pInit->maxBlockSize       = streaminfo.maxBlockSize;    /* Don't care about the min block size - only the max (used for determining the size of the memory allocation). */
        pInit->hasMetadataBlocks  = !isLastBlock;

        if (onMeta) {
            drflac_metadata metadata;
            metadata.type = DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO;
            metadata.pRawData = NULL;
            metadata.rawDataSize = 0;
            metadata.data.streaminfo = streaminfo;
            onMeta(pUserDataMD, &metadata);
        }

        return DRFLAC_TRUE;
    }
}

#ifndef DR_FLAC_NO_OGG
#define DRFLAC_OGG_MAX_PAGE_SIZE            65307
#define DRFLAC_OGG_CAPTURE_PATTERN_CRC32    1605413199  /* CRC-32 of "OggS". */

typedef enum
{
    drflac_ogg_recover_on_crc_mismatch,
    drflac_ogg_fail_on_crc_mismatch
} drflac_ogg_crc_mismatch_recovery;

#ifndef DR_FLAC_NO_CRC
static drflac_uint32 drflac__crc32_table[] = {
    0x00000000L, 0x04C11DB7L, 0x09823B6EL, 0x0D4326D9L,
    0x130476DCL, 0x17C56B6BL, 0x1A864DB2L, 0x1E475005L,
    0x2608EDB8L, 0x22C9F00FL, 0x2F8AD6D6L, 0x2B4BCB61L,
    0x350C9B64L, 0x31CD86D3L, 0x3C8EA00AL, 0x384FBDBDL,
    0x4C11DB70L, 0x48D0C6C7L, 0x4593E01EL, 0x4152FDA9L,
    0x5F15ADACL, 0x5BD4B01BL, 0x569796C2L, 0x52568B75L,
    0x6A1936C8L, 0x6ED82B7FL, 0x639B0DA6L, 0x675A1011L,
    0x791D4014L, 0x7DDC5DA3L, 0x709F7B7AL, 0x745E66CDL,
    0x9823B6E0L, 0x9CE2AB57L, 0x91A18D8EL, 0x95609039L,
    0x8B27C03CL, 0x8FE6DD8BL, 0x82A5FB52L, 0x8664E6E5L,
    0xBE2B5B58L, 0xBAEA46EFL, 0xB7A96036L, 0xB3687D81L,
    0xAD2F2D84L, 0xA9EE3033L, 0xA4AD16EAL, 0xA06C0B5DL,
    0xD4326D90L, 0xD0F37027L, 0xDDB056FEL, 0xD9714B49L,
    0xC7361B4CL, 0xC3F706FBL, 0xCEB42022L, 0xCA753D95L,
    0xF23A8028L, 0xF6FB9D9FL, 0xFBB8BB46L, 0xFF79A6F1L,
    0xE13EF6F4L, 0xE5FFEB43L, 0xE8BCCD9AL, 0xEC7DD02DL,
    0x34867077L, 0x30476DC0L, 0x3D044B19L, 0x39C556AEL,
    0x278206ABL, 0x23431B1CL, 0x2E003DC5L, 0x2AC12072L,
    0x128E9DCFL, 0x164F8078L, 0x1B0CA6A1L, 0x1FCDBB16L,
    0x018AEB13L, 0x054BF6A4L, 0x0808D07DL, 0x0CC9CDCAL,
    0x7897AB07L, 0x7C56B6B0L, 0x71159069L, 0x75D48DDEL,
    0x6B93DDDBL, 0x6F52C06CL, 0x6211E6B5L, 0x66D0FB02L,
    0x5E9F46BFL, 0x5A5E5B08L, 0x571D7DD1L, 0x53DC6066L,
    0x4D9B3063L, 0x495A2DD4L, 0x44190B0DL, 0x40D816BAL,
    0xACA5C697L, 0xA864DB20L, 0xA527FDF9L, 0xA1E6E04EL,
    0xBFA1B04BL, 0xBB60ADFCL, 0xB6238B25L, 0xB2E29692L,
    0x8AAD2B2FL, 0x8E6C3698L, 0x832F1041L, 0x87EE0DF6L,
    0x99A95DF3L, 0x9D684044L, 0x902B669DL, 0x94EA7B2AL,
    0xE0B41DE7L, 0xE4750050L, 0xE9362689L, 0xEDF73B3EL,
    0xF3B06B3BL, 0xF771768CL, 0xFA325055L, 0xFEF34DE2L,
    0xC6BCF05FL, 0xC27DEDE8L, 0xCF3ECB31L, 0xCBFFD686L,
    0xD5B88683L, 0xD1799B34L, 0xDC3ABDEDL, 0xD8FBA05AL,
    0x690CE0EEL, 0x6DCDFD59L, 0x608EDB80L, 0x644FC637L,
    0x7A089632L, 0x7EC98B85L, 0x738AAD5CL, 0x774BB0EBL,
    0x4F040D56L, 0x4BC510E1L, 0x46863638L, 0x42472B8FL,
    0x5C007B8AL, 0x58C1663DL, 0x558240E4L, 0x51435D53L,
    0x251D3B9EL, 0x21DC2629L, 0x2C9F00F0L, 0x285E1D47L,
    0x36194D42L, 0x32D850F5L, 0x3F9B762CL, 0x3B5A6B9BL,
    0x0315D626L, 0x07D4CB91L, 0x0A97ED48L, 0x0E56F0FFL,
    0x1011A0FAL, 0x14D0BD4DL, 0x19939B94L, 0x1D528623L,
    0xF12F560EL, 0xF5EE4BB9L, 0xF8AD6D60L, 0xFC6C70D7L,
    0xE22B20D2L, 0xE6EA3D65L, 0xEBA91BBCL, 0xEF68060BL,
    0xD727BBB6L, 0xD3E6A601L, 0xDEA580D8L, 0xDA649D6FL,
    0xC423CD6AL, 0xC0E2D0DDL, 0xCDA1F604L, 0xC960EBB3L,
    0xBD3E8D7EL, 0xB9FF90C9L, 0xB4BCB610L, 0xB07DABA7L,
    0xAE3AFBA2L, 0xAAFBE615L, 0xA7B8C0CCL, 0xA379DD7BL,
    0x9B3660C6L, 0x9FF77D71L, 0x92B45BA8L, 0x9675461FL,
    0x8832161AL, 0x8CF30BADL, 0x81B02D74L, 0x857130C3L,
    0x5D8A9099L, 0x594B8D2EL, 0x5408ABF7L, 0x50C9B640L,
    0x4E8EE645L, 0x4A4FFBF2L, 0x470CDD2BL, 0x43CDC09CL,
    0x7B827D21L, 0x7F436096L, 0x7200464FL, 0x76C15BF8L,
    0x68860BFDL, 0x6C47164AL, 0x61043093L, 0x65C52D24L,
    0x119B4BE9L, 0x155A565EL, 0x18197087L, 0x1CD86D30L,
    0x029F3D35L, 0x065E2082L, 0x0B1D065BL, 0x0FDC1BECL,
    0x3793A651L, 0x3352BBE6L, 0x3E119D3FL, 0x3AD08088L,
    0x2497D08DL, 0x2056CD3AL, 0x2D15EBE3L, 0x29D4F654L,
    0xC5A92679L, 0xC1683BCEL, 0xCC2B1D17L, 0xC8EA00A0L,
    0xD6AD50A5L, 0xD26C4D12L, 0xDF2F6BCBL, 0xDBEE767CL,
    0xE3A1CBC1L, 0xE760D676L, 0xEA23F0AFL, 0xEEE2ED18L,
    0xF0A5BD1DL, 0xF464A0AAL, 0xF9278673L, 0xFDE69BC4L,
    0x89B8FD09L, 0x8D79E0BEL, 0x803AC667L, 0x84FBDBD0L,
    0x9ABC8BD5L, 0x9E7D9662L, 0x933EB0BBL, 0x97FFAD0CL,
    0xAFB010B1L, 0xAB710D06L, 0xA6322BDFL, 0xA2F33668L,
    0xBCB4666DL, 0xB8757BDAL, 0xB5365D03L, 0xB1F740B4L
};
#endif

static DRFLAC_INLINE drflac_uint32 drflac_crc32_byte(drflac_uint32 crc32, drflac_uint8 data)
{
#ifndef DR_FLAC_NO_CRC
    return (crc32 << 8) ^ drflac__crc32_table[(drflac_uint8)((crc32 >> 24) & 0xFF) ^ data];
#else
    (void)data;
    return crc32;
#endif
}

#if 0
static DRFLAC_INLINE drflac_uint32 drflac_crc32_uint32(drflac_uint32 crc32, drflac_uint32 data)
{
    crc32 = drflac_crc32_byte(crc32, (drflac_uint8)((data >> 24) & 0xFF));
    crc32 = drflac_crc32_byte(crc32, (drflac_uint8)((data >> 16) & 0xFF));
    crc32 = drflac_crc32_byte(crc32, (drflac_uint8)((data >>  8) & 0xFF));
    crc32 = drflac_crc32_byte(crc32, (drflac_uint8)((data >>  0) & 0xFF));
    return crc32;
}

static DRFLAC_INLINE drflac_uint32 drflac_crc32_uint64(drflac_uint32 crc32, drflac_uint64 data)
{
    crc32 = drflac_crc32_uint32(crc32, (drflac_uint32)((data >> 32) & 0xFFFFFFFF));
    crc32 = drflac_crc32_uint32(crc32, (drflac_uint32)((data >>  0) & 0xFFFFFFFF));
    return crc32;
}
#endif

static DRFLAC_INLINE drflac_uint32 drflac_crc32_buffer(drflac_uint32 crc32, drflac_uint8* pData, drflac_uint32 dataSize)
{
    /* This can be optimized. */
    drflac_uint32 i;
    for (i = 0; i < dataSize; ++i) {
        crc32 = drflac_crc32_byte(crc32, pData[i]);
    }
    return crc32;
}


static DRFLAC_INLINE drflac_bool32 drflac_ogg__is_capture_pattern(drflac_uint8 pattern[4])
{
    return pattern[0] == 'O' && pattern[1] == 'g' && pattern[2] == 'g' && pattern[3] == 'S';
}

static DRFLAC_INLINE drflac_uint32 drflac_ogg__get_page_header_size(drflac_ogg_page_header* pHeader)
{
    return 27 + pHeader->segmentCount;
}

static DRFLAC_INLINE drflac_uint32 drflac_ogg__get_page_body_size(drflac_ogg_page_header* pHeader)
{
    drflac_uint32 pageBodySize = 0;
    int i;

    for (i = 0; i < pHeader->segmentCount; ++i) {
        pageBodySize += pHeader->segmentTable[i];
    }

    return pageBodySize;
}

drflac_result drflac_ogg__read_page_header_after_capture_pattern(drflac_read_proc onRead, void* pUserData, drflac_ogg_page_header* pHeader, drflac_uint32* pBytesRead, drflac_uint32* pCRC32)
{
    drflac_uint8 data[23];
    drflac_uint32 i;

    drflac_assert(*pCRC32 == DRFLAC_OGG_CAPTURE_PATTERN_CRC32);

    if (onRead(pUserData, data, 23) != 23) {
        return DRFLAC_END_OF_STREAM;
    }
    *pBytesRead += 23;

    pHeader->structureVersion = data[0];
    pHeader->headerType       = data[1];
    drflac_copy_memory(&pHeader->granulePosition, &data[ 2], 8);
    drflac_copy_memory(&pHeader->serialNumber,    &data[10], 4);
    drflac_copy_memory(&pHeader->sequenceNumber,  &data[14], 4);
    drflac_copy_memory(&pHeader->checksum,        &data[18], 4);
    pHeader->segmentCount     = data[22];

    /* Calculate the CRC. Note that for the calculation the checksum part of the page needs to be set to 0. */
    data[18] = 0;
    data[19] = 0;
    data[20] = 0;
    data[21] = 0;

    for (i = 0; i < 23; ++i) {
        *pCRC32 = drflac_crc32_byte(*pCRC32, data[i]);
    }


    if (onRead(pUserData, pHeader->segmentTable, pHeader->segmentCount) != pHeader->segmentCount) {
        return DRFLAC_END_OF_STREAM;
    }
    *pBytesRead += pHeader->segmentCount;

    for (i = 0; i < pHeader->segmentCount; ++i) {
        *pCRC32 = drflac_crc32_byte(*pCRC32, pHeader->segmentTable[i]);
    }

    return DRFLAC_SUCCESS;
}

drflac_result drflac_ogg__read_page_header(drflac_read_proc onRead, void* pUserData, drflac_ogg_page_header* pHeader, drflac_uint32* pBytesRead, drflac_uint32* pCRC32)
{
    drflac_uint8 id[4];

    *pBytesRead = 0;

    if (onRead(pUserData, id, 4) != 4) {
        return DRFLAC_END_OF_STREAM;
    }
    *pBytesRead += 4;

    /* We need to read byte-by-byte until we find the OggS capture pattern. */
    for (;;) {
        if (drflac_ogg__is_capture_pattern(id)) {
            drflac_result result;

            *pCRC32 = DRFLAC_OGG_CAPTURE_PATTERN_CRC32;

            result = drflac_ogg__read_page_header_after_capture_pattern(onRead, pUserData, pHeader, pBytesRead, pCRC32);
            if (result == DRFLAC_SUCCESS) {
                return DRFLAC_SUCCESS;
            } else {
                if (result == DRFLAC_CRC_MISMATCH) {
                    continue;
                } else {
                    return result;
                }
            }
        } else {
            /* The first 4 bytes did not equal the capture pattern. Read the next byte and try again. */
            id[0] = id[1];
            id[1] = id[2];
            id[2] = id[3];
            if (onRead(pUserData, &id[3], 1) != 1) {
                return DRFLAC_END_OF_STREAM;
            }
            *pBytesRead += 1;
        }
    }
}


/*
The main part of the Ogg encapsulation is the conversion from the physical Ogg bitstream to the native FLAC bitstream. It works
in three general stages: Ogg Physical Bitstream -> Ogg/FLAC Logical Bitstream -> FLAC Native Bitstream. dr_flac is designed
in such a way that the core sections assume everything is delivered in native format. Therefore, for each encapsulation type
dr_flac is supporting there needs to be a layer sitting on top of the onRead and onSeek callbacks that ensures the bits read from
the physical Ogg bitstream are converted and delivered in native FLAC format.
*/
typedef struct
{
    drflac_read_proc onRead;                /* The original onRead callback from drflac_open() and family. */
    drflac_seek_proc onSeek;                /* The original onSeek callback from drflac_open() and family. */
    void* pUserData;                        /* The user data passed on onRead and onSeek. This is the user data that was passed on drflac_open() and family. */
    drflac_uint64 currentBytePos;           /* The position of the byte we are sitting on in the physical byte stream. Used for efficient seeking. */
    drflac_uint64 firstBytePos;             /* The position of the first byte in the physical bitstream. Points to the start of the "OggS" identifier of the FLAC bos page. */
    drflac_uint32 serialNumber;             /* The serial number of the FLAC audio pages. This is determined by the initial header page that was read during initialization. */
    drflac_ogg_page_header bosPageHeader;   /* Used for seeking. */
    drflac_ogg_page_header currentPageHeader;
    drflac_uint32 bytesRemainingInPage;
    drflac_uint32 pageDataSize;
    drflac_uint8 pageData[DRFLAC_OGG_MAX_PAGE_SIZE];
} drflac_oggbs; /* oggbs = Ogg Bitstream */

static size_t drflac_oggbs__read_physical(drflac_oggbs* oggbs, void* bufferOut, size_t bytesToRead)
{
    size_t bytesActuallyRead = oggbs->onRead(oggbs->pUserData, bufferOut, bytesToRead);
    oggbs->currentBytePos += bytesActuallyRead;

    return bytesActuallyRead;
}

static drflac_bool32 drflac_oggbs__seek_physical(drflac_oggbs* oggbs, drflac_uint64 offset, drflac_seek_origin origin)
{
    if (origin == drflac_seek_origin_start) {
        if (offset <= 0x7FFFFFFF) {
            if (!oggbs->onSeek(oggbs->pUserData, (int)offset, drflac_seek_origin_start)) {
                return DRFLAC_FALSE;
            }
            oggbs->currentBytePos = offset;

            return DRFLAC_TRUE;
        } else {
            if (!oggbs->onSeek(oggbs->pUserData, 0x7FFFFFFF, drflac_seek_origin_start)) {
                return DRFLAC_FALSE;
            }
            oggbs->currentBytePos = offset;

            return drflac_oggbs__seek_physical(oggbs, offset - 0x7FFFFFFF, drflac_seek_origin_current);
        }
    } else {
        while (offset > 0x7FFFFFFF) {
            if (!oggbs->onSeek(oggbs->pUserData, 0x7FFFFFFF, drflac_seek_origin_current)) {
                return DRFLAC_FALSE;
            }
            oggbs->currentBytePos += 0x7FFFFFFF;
            offset -= 0x7FFFFFFF;
        }

        if (!oggbs->onSeek(oggbs->pUserData, (int)offset, drflac_seek_origin_current)) {    /* <-- Safe cast thanks to the loop above. */
            return DRFLAC_FALSE;
        }
        oggbs->currentBytePos += offset;

        return DRFLAC_TRUE;
    }
}

static drflac_bool32 drflac_oggbs__goto_next_page(drflac_oggbs* oggbs, drflac_ogg_crc_mismatch_recovery recoveryMethod)
{
    drflac_ogg_page_header header;
    for (;;) {
        drflac_uint32 crc32 = 0;
        drflac_uint32 bytesRead;
        drflac_uint32 pageBodySize;
#ifndef DR_FLAC_NO_CRC
        drflac_uint32 actualCRC32;
#endif

        if (drflac_ogg__read_page_header(oggbs->onRead, oggbs->pUserData, &header, &bytesRead, &crc32) != DRFLAC_SUCCESS) {
            return DRFLAC_FALSE;
        }
        oggbs->currentBytePos += bytesRead;

        pageBodySize = drflac_ogg__get_page_body_size(&header);
        if (pageBodySize > DRFLAC_OGG_MAX_PAGE_SIZE) {
            continue;   /* Invalid page size. Assume it's corrupted and just move to the next page. */
        }

        if (header.serialNumber != oggbs->serialNumber) {
            /* It's not a FLAC page. Skip it. */
            if (pageBodySize > 0 && !drflac_oggbs__seek_physical(oggbs, pageBodySize, drflac_seek_origin_current)) {
                return DRFLAC_FALSE;
            }
            continue;
        }


        /* We need to read the entire page and then do a CRC check on it. If there's a CRC mismatch we need to skip this page. */
        if (drflac_oggbs__read_physical(oggbs, oggbs->pageData, pageBodySize) != pageBodySize) {
            return DRFLAC_FALSE;
        }
        oggbs->pageDataSize = pageBodySize;

#ifndef DR_FLAC_NO_CRC
        actualCRC32 = drflac_crc32_buffer(crc32, oggbs->pageData, oggbs->pageDataSize);
        if (actualCRC32 != header.checksum) {
            if (recoveryMethod == drflac_ogg_recover_on_crc_mismatch) {
                continue;   /* CRC mismatch. Skip this page. */
            } else {
                /*
                Even though we are failing on a CRC mismatch, we still want our stream to be in a good state. Therefore we
                go to the next valid page to ensure we're in a good state, but return false to let the caller know that the
                seek did not fully complete.
                */
                drflac_oggbs__goto_next_page(oggbs, drflac_ogg_recover_on_crc_mismatch);
                return DRFLAC_FALSE;
            }
        }
#else
        (void)recoveryMethod;   /* <-- Silence a warning. */
#endif

        oggbs->currentPageHeader = header;
        oggbs->bytesRemainingInPage = pageBodySize;
        return DRFLAC_TRUE;
    }
}

/* Function below is unused at the moment, but I might be re-adding it later. */
#if 0
static drflac_uint8 drflac_oggbs__get_current_segment_index(drflac_oggbs* oggbs, drflac_uint8* pBytesRemainingInSeg)
{
    drflac_uint32 bytesConsumedInPage = drflac_ogg__get_page_body_size(&oggbs->currentPageHeader) - oggbs->bytesRemainingInPage;
    drflac_uint8 iSeg = 0;
    drflac_uint32 iByte = 0;
    while (iByte < bytesConsumedInPage) {
        drflac_uint8 segmentSize = oggbs->currentPageHeader.segmentTable[iSeg];
        if (iByte + segmentSize > bytesConsumedInPage) {
            break;
        } else {
            iSeg += 1;
            iByte += segmentSize;
        }
    }

    *pBytesRemainingInSeg = oggbs->currentPageHeader.segmentTable[iSeg] - (drflac_uint8)(bytesConsumedInPage - iByte);
    return iSeg;
}

static drflac_bool32 drflac_oggbs__seek_to_next_packet(drflac_oggbs* oggbs)
{
    /* The current packet ends when we get to the segment with a lacing value of < 255 which is not at the end of a page. */
    for (;;) {
        drflac_bool32 atEndOfPage = DRFLAC_FALSE;

        drflac_uint8 bytesRemainingInSeg;
        drflac_uint8 iFirstSeg = drflac_oggbs__get_current_segment_index(oggbs, &bytesRemainingInSeg);

        drflac_uint32 bytesToEndOfPacketOrPage = bytesRemainingInSeg;
        for (drflac_uint8 iSeg = iFirstSeg; iSeg < oggbs->currentPageHeader.segmentCount; ++iSeg) {
            drflac_uint8 segmentSize = oggbs->currentPageHeader.segmentTable[iSeg];
            if (segmentSize < 255) {
                if (iSeg == oggbs->currentPageHeader.segmentCount-1) {
                    atEndOfPage = DRFLAC_TRUE;
                }

                break;
            }

            bytesToEndOfPacketOrPage += segmentSize;
        }

        /*
        At this point we will have found either the packet or the end of the page. If were at the end of the page we'll
        want to load the next page and keep searching for the end of the packet.
        */
        drflac_oggbs__seek_physical(oggbs, bytesToEndOfPacketOrPage, drflac_seek_origin_current);
        oggbs->bytesRemainingInPage -= bytesToEndOfPacketOrPage;

        if (atEndOfPage) {
            /*
            We're potentially at the next packet, but we need to check the next page first to be sure because the packet may
            straddle pages.
            */
            if (!drflac_oggbs__goto_next_page(oggbs)) {
                return DRFLAC_FALSE;
            }

            /* If it's a fresh packet it most likely means we're at the next packet. */
            if ((oggbs->currentPageHeader.headerType & 0x01) == 0) {
                return DRFLAC_TRUE;
            }
        } else {
            /* We're at the next packet. */
            return DRFLAC_TRUE;
        }
    }
}

static drflac_bool32 drflac_oggbs__seek_to_next_frame(drflac_oggbs* oggbs)
{
    /* The bitstream should be sitting on the first byte just after the header of the frame. */

    /* What we're actually doing here is seeking to the start of the next packet. */
    return drflac_oggbs__seek_to_next_packet(oggbs);
}
#endif

static size_t drflac__on_read_ogg(void* pUserData, void* bufferOut, size_t bytesToRead)
{
    drflac_oggbs* oggbs = (drflac_oggbs*)pUserData;
    drflac_uint8* pRunningBufferOut = (drflac_uint8*)bufferOut;
    size_t bytesRead = 0;

    drflac_assert(oggbs != NULL);
    drflac_assert(pRunningBufferOut != NULL);

    /* Reading is done page-by-page. If we've run out of bytes in the page we need to move to the next one. */
    while (bytesRead < bytesToRead) {
        size_t bytesRemainingToRead = bytesToRead - bytesRead;

        if (oggbs->bytesRemainingInPage >= bytesRemainingToRead) {
            drflac_copy_memory(pRunningBufferOut, oggbs->pageData + (oggbs->pageDataSize - oggbs->bytesRemainingInPage), bytesRemainingToRead);
            bytesRead += bytesRemainingToRead;
            oggbs->bytesRemainingInPage -= (drflac_uint32)bytesRemainingToRead;
            break;
        }

        /* If we get here it means some of the requested data is contained in the next pages. */
        if (oggbs->bytesRemainingInPage > 0) {
            drflac_copy_memory(pRunningBufferOut, oggbs->pageData + (oggbs->pageDataSize - oggbs->bytesRemainingInPage), oggbs->bytesRemainingInPage);
            bytesRead += oggbs->bytesRemainingInPage;
            pRunningBufferOut += oggbs->bytesRemainingInPage;
            oggbs->bytesRemainingInPage = 0;
        }

        drflac_assert(bytesRemainingToRead > 0);
        if (!drflac_oggbs__goto_next_page(oggbs, drflac_ogg_recover_on_crc_mismatch)) {
            break;  /* Failed to go to the next page. Might have simply hit the end of the stream. */
        }
    }

    return bytesRead;
}

static drflac_bool32 drflac__on_seek_ogg(void* pUserData, int offset, drflac_seek_origin origin)
{
    drflac_oggbs* oggbs = (drflac_oggbs*)pUserData;
    int bytesSeeked = 0;

    drflac_assert(oggbs != NULL);
    drflac_assert(offset >= 0);  /* <-- Never seek backwards. */

    /* Seeking is always forward which makes things a lot simpler. */
    if (origin == drflac_seek_origin_start) {
        if (!drflac_oggbs__seek_physical(oggbs, (int)oggbs->firstBytePos, drflac_seek_origin_start)) {
            return DRFLAC_FALSE;
        }

        if (!drflac_oggbs__goto_next_page(oggbs, drflac_ogg_fail_on_crc_mismatch)) {
            return DRFLAC_FALSE;
        }

        return drflac__on_seek_ogg(pUserData, offset, drflac_seek_origin_current);
    }

    drflac_assert(origin == drflac_seek_origin_current);

    while (bytesSeeked < offset) {
        int bytesRemainingToSeek = offset - bytesSeeked;
        drflac_assert(bytesRemainingToSeek >= 0);

        if (oggbs->bytesRemainingInPage >= (size_t)bytesRemainingToSeek) {
            bytesSeeked += bytesRemainingToSeek;
            oggbs->bytesRemainingInPage -= bytesRemainingToSeek;
            break;
        }

        /* If we get here it means some of the requested data is contained in the next pages. */
        if (oggbs->bytesRemainingInPage > 0) {
            bytesSeeked += (int)oggbs->bytesRemainingInPage;
            oggbs->bytesRemainingInPage = 0;
        }

        drflac_assert(bytesRemainingToSeek > 0);
        if (!drflac_oggbs__goto_next_page(oggbs, drflac_ogg_fail_on_crc_mismatch)) {
            /* Failed to go to the next page. We either hit the end of the stream or had a CRC mismatch. */
            return DRFLAC_FALSE;
        }
    }

    return DRFLAC_TRUE;
}

drflac_bool32 drflac_ogg__seek_to_sample(drflac* pFlac, drflac_uint64 sampleIndex)
{
    drflac_oggbs* oggbs = (drflac_oggbs*)pFlac->_oggbs;
    drflac_uint64 originalBytePos;
    drflac_uint64 runningGranulePosition;
    drflac_uint64 runningFrameBytePos;
    drflac_uint64 runningSampleCount;

    drflac_assert(oggbs != NULL);

    originalBytePos = oggbs->currentBytePos;   /* For recovery. */

    /* First seek to the first frame. */
    if (!drflac__seek_to_byte(&pFlac->bs, pFlac->firstFramePos)) {
        return DRFLAC_FALSE;
    }
    oggbs->bytesRemainingInPage = 0;

    runningGranulePosition = 0;
    runningFrameBytePos = oggbs->currentBytePos;   /* <-- Points to the OggS identifier. */
    for (;;) {
        if (!drflac_oggbs__goto_next_page(oggbs, drflac_ogg_recover_on_crc_mismatch)) {
            drflac_oggbs__seek_physical(oggbs, originalBytePos, drflac_seek_origin_start);
            return DRFLAC_FALSE;   /* Never did find that sample... */
        }

        runningFrameBytePos = oggbs->currentBytePos - drflac_ogg__get_page_header_size(&oggbs->currentPageHeader) - oggbs->pageDataSize;
        if (oggbs->currentPageHeader.granulePosition*pFlac->channels >= sampleIndex) {
            break; /* The sample is somewhere in the previous page. */
        }

        /*
        At this point we know the sample is not in the previous page. It could possibly be in this page. For simplicity we
        disregard any pages that do not begin a fresh packet.
        */
        if ((oggbs->currentPageHeader.headerType & 0x01) == 0) {    /* <-- Is it a fresh page? */
            if (oggbs->currentPageHeader.segmentTable[0] >= 2) {
                drflac_uint8 firstBytesInPage[2];
                firstBytesInPage[0] = oggbs->pageData[0];
                firstBytesInPage[1] = oggbs->pageData[1];

                if ((firstBytesInPage[0] == 0xFF) && (firstBytesInPage[1] & 0xFC) == 0xF8) {    /* <-- Does the page begin with a frame's sync code? */
                    runningGranulePosition = oggbs->currentPageHeader.granulePosition*pFlac->channels;
                }

                continue;
            }
        }
    }

    /*
    We found the page that that is closest to the sample, so now we need to find it. The first thing to do is seek to the
    start of that page. In the loop above we checked that it was a fresh page which means this page is also the start of
    a new frame. This property means that after we've seeked to the page we can immediately start looping over frames until
    we find the one containing the target sample.
    */
    if (!drflac_oggbs__seek_physical(oggbs, runningFrameBytePos, drflac_seek_origin_start)) {
        return DRFLAC_FALSE;
    }
    if (!drflac_oggbs__goto_next_page(oggbs, drflac_ogg_recover_on_crc_mismatch)) {
        return DRFLAC_FALSE;
    }

    /*
    At this point we'll be sitting on the first byte of the frame header of the first frame in the page. We just keep
    looping over these frames until we find the one containing the sample we're after.
    */
    runningSampleCount = runningGranulePosition;
    for (;;) {
        /*
        There are two ways to find the sample and seek past irrelevant frames:
          1) Use the native FLAC decoder.
          2) Use Ogg's framing system.
        
        Both of these options have their own pros and cons. Using the native FLAC decoder is slower because it needs to
        do a full decode of the frame. Using Ogg's framing system is faster, but more complicated and involves some code
        duplication for the decoding of frame headers.
        
        Another thing to consider is that using the Ogg framing system will perform direct seeking of the physical Ogg
        bitstream. This is important to consider because it means we cannot read data from the drflac_bs object using the
        standard drflac__*() APIs because that will read in extra data for its own internal caching which in turn breaks
        the positioning of the read pointer of the physical Ogg bitstream. Therefore, anything that would normally be read
        using the native FLAC decoding APIs, such as drflac__read_next_flac_frame_header(), need to be re-implemented so as to
        avoid the use of the drflac_bs object.
        
        Considering these issues, I have decided to use the slower native FLAC decoding method for the following reasons:
          1) Seeking is already partially accelerated using Ogg's paging system in the code block above.
          2) Seeking in an Ogg encapsulated FLAC stream is probably quite uncommon.
          3) Simplicity.
        */
        drflac_uint64 firstSampleInFrame = 0;
        drflac_uint64 lastSampleInFrame = 0;
        drflac_uint64 sampleCountInThisFrame;

        if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
            return DRFLAC_FALSE;
        }

        drflac__get_current_frame_sample_range(pFlac, &firstSampleInFrame, &lastSampleInFrame);

        sampleCountInThisFrame = (lastSampleInFrame - firstSampleInFrame) + 1;
        if (sampleIndex < (runningSampleCount + sampleCountInThisFrame)) {
            /*
            The sample should be in this frame. We need to fully decode it, however if it's an invalid frame (a CRC mismatch), we need to pretend
            it never existed and keep iterating.
            */
            drflac_result result = drflac__decode_flac_frame(pFlac);
            if (result == DRFLAC_SUCCESS) {
                /* The frame is valid. We just need to skip over some samples to ensure it's sample-exact. */
                drflac_uint64 samplesToDecode = (size_t)(sampleIndex - runningSampleCount);    /* <-- Safe cast because the maximum number of samples in a frame is 65535. */
                if (samplesToDecode == 0) {
                    return DRFLAC_TRUE;
                }
                return drflac__seek_forward_by_samples(pFlac, samplesToDecode) == samplesToDecode;  /* <-- If this fails, something bad has happened (it should never fail). */
            } else {
                if (result == DRFLAC_CRC_MISMATCH) {
                    continue;   /* CRC mismatch. Pretend this frame never existed. */
                } else {
                    return DRFLAC_FALSE;
                }
            }
        } else {
            /*
            It's not in this frame. We need to seek past the frame, but check if there was a CRC mismatch. If so, we pretend this
            frame never existed and leave the running sample count untouched.
            */
            drflac_result result = drflac__seek_to_next_flac_frame(pFlac);
            if (result == DRFLAC_SUCCESS) {
                runningSampleCount += sampleCountInThisFrame;
            } else {
                if (result == DRFLAC_CRC_MISMATCH) {
                    continue;   /* CRC mismatch. Pretend this frame never existed. */
                } else {
                    return DRFLAC_FALSE;
                }
            }
        }
    }
}


drflac_bool32 drflac__init_private__ogg(drflac_init_info* pInit, drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, void* pUserData, void* pUserDataMD, drflac_bool32 relaxed)
{
    drflac_ogg_page_header header;
    drflac_uint32 crc32 = DRFLAC_OGG_CAPTURE_PATTERN_CRC32;
    drflac_uint32 bytesRead = 0;

    /* Pre Condition: The bit stream should be sitting just past the 4-byte OggS capture pattern. */
    (void)relaxed;

    pInit->container = drflac_container_ogg;
    pInit->oggFirstBytePos = 0;

    /*
    We'll get here if the first 4 bytes of the stream were the OggS capture pattern, however it doesn't necessarily mean the
    stream includes FLAC encoded audio. To check for this we need to scan the beginning-of-stream page markers and check if
    any match the FLAC specification. Important to keep in mind that the stream may be multiplexed.
    */
    if (drflac_ogg__read_page_header_after_capture_pattern(onRead, pUserData, &header, &bytesRead, &crc32) != DRFLAC_SUCCESS) {
        return DRFLAC_FALSE;
    }
    pInit->runningFilePos += bytesRead;

    for (;;) {
        int pageBodySize;

        /* Break if we're past the beginning of stream page. */
        if ((header.headerType & 0x02) == 0) {
            return DRFLAC_FALSE;
        }

        /* Check if it's a FLAC header. */
        pageBodySize = drflac_ogg__get_page_body_size(&header);
        if (pageBodySize == 51) {   /* 51 = the lacing value of the FLAC header packet. */
            /* It could be a FLAC page... */
            drflac_uint32 bytesRemainingInPage = pageBodySize;
            drflac_uint8 packetType;

            if (onRead(pUserData, &packetType, 1) != 1) {
                return DRFLAC_FALSE;
            }

            bytesRemainingInPage -= 1;
            if (packetType == 0x7F) {
                /* Increasingly more likely to be a FLAC page... */
                drflac_uint8 sig[4];
                if (onRead(pUserData, sig, 4) != 4) {
                    return DRFLAC_FALSE;
                }

                bytesRemainingInPage -= 4;
                if (sig[0] == 'F' && sig[1] == 'L' && sig[2] == 'A' && sig[3] == 'C') {
                    /* Almost certainly a FLAC page... */
                    drflac_uint8 mappingVersion[2];
                    if (onRead(pUserData, mappingVersion, 2) != 2) {
                        return DRFLAC_FALSE;
                    }

                    if (mappingVersion[0] != 1) {
                        return DRFLAC_FALSE;   /* Only supporting version 1.x of the Ogg mapping. */
                    }

                    /*
                    The next 2 bytes are the non-audio packets, not including this one. We don't care about this because we're going to
                    be handling it in a generic way based on the serial number and packet types.
                    */
                    if (!onSeek(pUserData, 2, drflac_seek_origin_current)) {
                        return DRFLAC_FALSE;
                    }

                    /* Expecting the native FLAC signature "fLaC". */
                    if (onRead(pUserData, sig, 4) != 4) {
                        return DRFLAC_FALSE;
                    }

                    if (sig[0] == 'f' && sig[1] == 'L' && sig[2] == 'a' && sig[3] == 'C') {
                        /* The remaining data in the page should be the STREAMINFO block. */
                        drflac_streaminfo streaminfo;
                        drflac_uint8 isLastBlock;
                        drflac_uint8 blockType;
                        drflac_uint32 blockSize;
                        if (!drflac__read_and_decode_block_header(onRead, pUserData, &isLastBlock, &blockType, &blockSize)) {
                            return DRFLAC_FALSE;
                        }

                        if (blockType != DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO || blockSize != 34) {
                            return DRFLAC_FALSE;    /* Invalid block type. First block must be the STREAMINFO block. */
                        }

                        if (drflac__read_streaminfo(onRead, pUserData, &streaminfo)) {
                            /* Success! */
                            pInit->hasStreamInfoBlock = DRFLAC_TRUE;
                            pInit->sampleRate         = streaminfo.sampleRate;
                            pInit->channels           = streaminfo.channels;
                            pInit->bitsPerSample      = streaminfo.bitsPerSample;
                            pInit->totalSampleCount   = streaminfo.totalSampleCount;
                            pInit->maxBlockSize       = streaminfo.maxBlockSize;
                            pInit->hasMetadataBlocks  = !isLastBlock;

                            if (onMeta) {
                                drflac_metadata metadata;
                                metadata.type = DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO;
                                metadata.pRawData = NULL;
                                metadata.rawDataSize = 0;
                                metadata.data.streaminfo = streaminfo;
                                onMeta(pUserDataMD, &metadata);
                            }

                            pInit->runningFilePos  += pageBodySize;
                            pInit->oggFirstBytePos  = pInit->runningFilePos - 79;   /* Subtracting 79 will place us right on top of the "OggS" identifier of the FLAC bos page. */
                            pInit->oggSerial        = header.serialNumber;
                            pInit->oggBosHeader     = header;
                            break;
                        } else {
                            /* Failed to read STREAMINFO block. Aww, so close... */
                            return DRFLAC_FALSE;
                        }
                    } else {
                        /* Invalid file. */
                        return DRFLAC_FALSE;
                    }
                } else {
                    /* Not a FLAC header. Skip it. */
                    if (!onSeek(pUserData, bytesRemainingInPage, drflac_seek_origin_current)) {
                        return DRFLAC_FALSE;
                    }
                }
            } else {
                /* Not a FLAC header. Seek past the entire page and move on to the next. */
                if (!onSeek(pUserData, bytesRemainingInPage, drflac_seek_origin_current)) {
                    return DRFLAC_FALSE;
                }
            }
        } else {
            if (!onSeek(pUserData, pageBodySize, drflac_seek_origin_current)) {
                return DRFLAC_FALSE;
            }
        }

        pInit->runningFilePos += pageBodySize;


        /* Read the header of the next page. */
        if (drflac_ogg__read_page_header(onRead, pUserData, &header, &bytesRead, &crc32) != DRFLAC_SUCCESS) {
            return DRFLAC_FALSE;
        }
        pInit->runningFilePos += bytesRead;
    }

    /*
    If we get here it means we found a FLAC audio stream. We should be sitting on the first byte of the header of the next page. The next
    packets in the FLAC logical stream contain the metadata. The only thing left to do in the initialization phase for Ogg is to create the
    Ogg bistream object.
    */
    pInit->hasMetadataBlocks = DRFLAC_TRUE;    /* <-- Always have at least VORBIS_COMMENT metadata block. */
    return DRFLAC_TRUE;
}
#endif

drflac_bool32 drflac__init_private(drflac_init_info* pInit, drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, drflac_container container, void* pUserData, void* pUserDataMD)
{
    drflac_bool32 relaxed;
    drflac_uint8 id[4];

    if (pInit == NULL || onRead == NULL || onSeek == NULL) {
        return DRFLAC_FALSE;
    }

    drflac_zero_memory(pInit, sizeof(*pInit));
    pInit->onRead       = onRead;
    pInit->onSeek       = onSeek;
    pInit->onMeta       = onMeta;
    pInit->container    = container;
    pInit->pUserData    = pUserData;
    pInit->pUserDataMD  = pUserDataMD;

    pInit->bs.onRead    = onRead;
    pInit->bs.onSeek    = onSeek;
    pInit->bs.pUserData = pUserData;
    drflac__reset_cache(&pInit->bs);


    /* If the container is explicitly defined then we can try opening in relaxed mode. */
    relaxed = container != drflac_container_unknown;

    /* Skip over any ID3 tags. */
    for (;;) {
        if (onRead(pUserData, id, 4) != 4) {
            return DRFLAC_FALSE;    /* Ran out of data. */
        }
        pInit->runningFilePos += 4;

        if (id[0] == 'I' && id[1] == 'D' && id[2] == '3') {
            drflac_uint8 header[6];
            drflac_uint8 flags;
            drflac_uint32 headerSize;

            if (onRead(pUserData, header, 6) != 6) {
                return DRFLAC_FALSE;    /* Ran out of data. */
            }
            pInit->runningFilePos += 6;

            flags = header[1];

            drflac_copy_memory(&headerSize, header+2, 4);
            headerSize = drflac__unsynchsafe_32(drflac__be2host_32(headerSize));
            if (flags & 0x10) {
                headerSize += 10;
            }

            if (!onSeek(pUserData, headerSize, drflac_seek_origin_current)) {
                return DRFLAC_FALSE;    /* Failed to seek past the tag. */
            }
            pInit->runningFilePos += headerSize;
        } else {
            break;
        }
    }

    if (id[0] == 'f' && id[1] == 'L' && id[2] == 'a' && id[3] == 'C') {
        return drflac__init_private__native(pInit, onRead, onSeek, onMeta, pUserData, pUserDataMD, relaxed);
    }
#ifndef DR_FLAC_NO_OGG
    if (id[0] == 'O' && id[1] == 'g' && id[2] == 'g' && id[3] == 'S') {
        return drflac__init_private__ogg(pInit, onRead, onSeek, onMeta, pUserData, pUserDataMD, relaxed);
    }
#endif

    /* If we get here it means we likely don't have a header. Try opening in relaxed mode, if applicable. */
    if (relaxed) {
        if (container == drflac_container_native) {
            return drflac__init_private__native(pInit, onRead, onSeek, onMeta, pUserData, pUserDataMD, relaxed);
        }
#ifndef DR_FLAC_NO_OGG
        if (container == drflac_container_ogg) {
            return drflac__init_private__ogg(pInit, onRead, onSeek, onMeta, pUserData, pUserDataMD, relaxed);
        }
#endif
    }

    /* Unsupported container. */
    return DRFLAC_FALSE;
}

void drflac__init_from_info(drflac* pFlac, drflac_init_info* pInit)
{
    drflac_assert(pFlac != NULL);
    drflac_assert(pInit != NULL);

    drflac_zero_memory(pFlac, sizeof(*pFlac));
    pFlac->bs                 = pInit->bs;
    pFlac->onMeta             = pInit->onMeta;
    pFlac->pUserDataMD        = pInit->pUserDataMD;
    pFlac->maxBlockSize       = pInit->maxBlockSize;
    pFlac->sampleRate         = pInit->sampleRate;
    pFlac->channels           = (drflac_uint8)pInit->channels;
    pFlac->bitsPerSample      = (drflac_uint8)pInit->bitsPerSample;
    pFlac->totalSampleCount   = pInit->totalSampleCount;
    pFlac->totalPCMFrameCount = pInit->totalSampleCount / pFlac->channels;
    pFlac->container          = pInit->container;
}

drflac* drflac_open_with_metadata_private(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, drflac_container container, void* pUserData, void* pUserDataMD)
{
    drflac_init_info init;
    drflac_uint32 allocationSize;
    drflac_uint32 wholeSIMDVectorCountPerChannel;
    drflac_uint32 decodedSamplesAllocationSize;
#ifndef DR_FLAC_NO_OGG
    drflac_uint32 oggbsAllocationSize;
    drflac_oggbs oggbs;
#endif
    drflac_uint64 firstFramePos;
    drflac_uint64 seektablePos;
    drflac_uint32 seektableSize;
    drflac* pFlac;

#ifndef DRFLAC_NO_CPUID
    /* CPU support first. */
    drflac__init_cpu_caps();
#endif

    if (!drflac__init_private(&init, onRead, onSeek, onMeta, container, pUserData, pUserDataMD)) {
        return NULL;
    }

    /*
    The size of the allocation for the drflac object needs to be large enough to fit the following:
      1) The main members of the drflac structure
      2) A block of memory large enough to store the decoded samples of the largest frame in the stream
      3) If the container is Ogg, a drflac_oggbs object
    
    The complicated part of the allocation is making sure there's enough room the decoded samples, taking into consideration
    the different SIMD instruction sets.
    */
    allocationSize = sizeof(drflac);

    /*
    The allocation size for decoded frames depends on the number of 32-bit integers that fit inside the largest SIMD vector
    we are supporting.
    */
    if (((init.maxBlockSize+DRFLAC_LEADING_SAMPLES) % (DRFLAC_MAX_SIMD_VECTOR_SIZE / sizeof(drflac_int32))) == 0) {
        wholeSIMDVectorCountPerChannel = ((init.maxBlockSize+DRFLAC_LEADING_SAMPLES) / (DRFLAC_MAX_SIMD_VECTOR_SIZE / sizeof(drflac_int32)));
    } else {
        wholeSIMDVectorCountPerChannel = ((init.maxBlockSize+DRFLAC_LEADING_SAMPLES) / (DRFLAC_MAX_SIMD_VECTOR_SIZE / sizeof(drflac_int32))) + 1;
    }

    decodedSamplesAllocationSize = wholeSIMDVectorCountPerChannel * DRFLAC_MAX_SIMD_VECTOR_SIZE * init.channels;

    allocationSize += decodedSamplesAllocationSize;
    allocationSize += DRFLAC_MAX_SIMD_VECTOR_SIZE;  /* Allocate extra bytes to ensure we have enough for alignment. */

#ifndef DR_FLAC_NO_OGG
    /* There's additional data required for Ogg streams. */
    oggbsAllocationSize = 0;
    if (init.container == drflac_container_ogg) {
        oggbsAllocationSize = sizeof(drflac_oggbs);
        allocationSize += oggbsAllocationSize;
    }

    drflac_zero_memory(&oggbs, sizeof(oggbs));
    if (init.container == drflac_container_ogg) {
        oggbs.onRead = onRead;
        oggbs.onSeek = onSeek;
        oggbs.pUserData = pUserData;
        oggbs.currentBytePos = init.oggFirstBytePos;
        oggbs.firstBytePos = init.oggFirstBytePos;
        oggbs.serialNumber = init.oggSerial;
        oggbs.bosPageHeader = init.oggBosHeader;
        oggbs.bytesRemainingInPage = 0;
    }
#endif

    /*
    This part is a bit awkward. We need to load the seektable so that it can be referenced in-memory, but I want the drflac object to
    consist of only a single heap allocation. To this, the size of the seek table needs to be known, which we determine when reading
    and decoding the metadata.
    */
    firstFramePos = 42;   /* <-- We know we are at byte 42 at this point. */
    seektablePos  = 0;
    seektableSize = 0;
    if (init.hasMetadataBlocks) {
        drflac_read_proc onReadOverride = onRead;
        drflac_seek_proc onSeekOverride = onSeek;
        void* pUserDataOverride = pUserData;

#ifndef DR_FLAC_NO_OGG
        if (init.container == drflac_container_ogg) {
            onReadOverride = drflac__on_read_ogg;
            onSeekOverride = drflac__on_seek_ogg;
            pUserDataOverride = (void*)&oggbs;
        }
#endif

        if (!drflac__read_and_decode_metadata(onReadOverride, onSeekOverride, onMeta, pUserDataOverride, pUserDataMD, &firstFramePos, &seektablePos, &seektableSize)) {
            return NULL;
        }

        allocationSize += seektableSize;
    }


    pFlac = (drflac*)DRFLAC_MALLOC(allocationSize);
    drflac__init_from_info(pFlac, &init);
    pFlac->pDecodedSamples = (drflac_int32*)drflac_align((size_t)pFlac->pExtraData, DRFLAC_MAX_SIMD_VECTOR_SIZE);

#ifndef DR_FLAC_NO_OGG
    if (init.container == drflac_container_ogg) {
        drflac_oggbs* pInternalOggbs = (drflac_oggbs*)((drflac_uint8*)pFlac->pDecodedSamples + decodedSamplesAllocationSize + seektableSize);
        *pInternalOggbs = oggbs;

        /* The Ogg bistream needs to be layered on top of the original bitstream. */
        pFlac->bs.onRead = drflac__on_read_ogg;
        pFlac->bs.onSeek = drflac__on_seek_ogg;
        pFlac->bs.pUserData = (void*)pInternalOggbs;
        pFlac->_oggbs = (void*)pInternalOggbs;
    }
#endif

    pFlac->firstFramePos = firstFramePos;

    /* NOTE: Seektables are not currently compatible with Ogg encapsulation (Ogg has its own accelerated seeking system). I may change this later, so I'm leaving this here for now. */
#ifndef DR_FLAC_NO_OGG
    if (init.container == drflac_container_ogg)
    {
        pFlac->pSeekpoints = NULL;
        pFlac->seekpointCount = 0;
    }
    else
#endif
    {
        /* If we have a seektable we need to load it now, making sure we move back to where we were previously. */
        if (seektablePos != 0) {
            pFlac->seekpointCount = seektableSize / sizeof(*pFlac->pSeekpoints);
            pFlac->pSeekpoints = (drflac_seekpoint*)((drflac_uint8*)pFlac->pDecodedSamples + decodedSamplesAllocationSize);

            /* Seek to the seektable, then just read directly into our seektable buffer. */
            if (pFlac->bs.onSeek(pFlac->bs.pUserData, (int)seektablePos, drflac_seek_origin_start)) {
                if (pFlac->bs.onRead(pFlac->bs.pUserData, pFlac->pSeekpoints, seektableSize) == seektableSize) {
                    /* Endian swap. */
                    drflac_uint32 iSeekpoint;
                    for (iSeekpoint = 0; iSeekpoint < pFlac->seekpointCount; ++iSeekpoint) {
                        pFlac->pSeekpoints[iSeekpoint].firstSample = drflac__be2host_64(pFlac->pSeekpoints[iSeekpoint].firstSample);
                        pFlac->pSeekpoints[iSeekpoint].frameOffset = drflac__be2host_64(pFlac->pSeekpoints[iSeekpoint].frameOffset);
                        pFlac->pSeekpoints[iSeekpoint].sampleCount = drflac__be2host_16(pFlac->pSeekpoints[iSeekpoint].sampleCount);
                    }
                } else {
                    /* Failed to read the seektable. Pretend we don't have one. */
                    pFlac->pSeekpoints = NULL;
                    pFlac->seekpointCount = 0;
                }

                /* We need to seek back to where we were. If this fails it's a critical error. */
                if (!pFlac->bs.onSeek(pFlac->bs.pUserData, (int)pFlac->firstFramePos, drflac_seek_origin_start)) {
                    DRFLAC_FREE(pFlac);
                    return NULL;
                }
            } else {
                /* Failed to seek to the seektable. Ominous sign, but for now we can just pretend we don't have one. */
                pFlac->pSeekpoints = NULL;
                pFlac->seekpointCount = 0;
            }
        }
    }

    
    /*
    If we get here, but don't have a STREAMINFO block, it means we've opened the stream in relaxed mode and need to decode
    the first frame.
    */
    if (!init.hasStreamInfoBlock) {
        pFlac->currentFrame.header = init.firstFrameHeader;
        do
        {
            drflac_result result = drflac__decode_flac_frame(pFlac);
            if (result == DRFLAC_SUCCESS) {
                break;
            } else {
                if (result == DRFLAC_CRC_MISMATCH) {
                    if (!drflac__read_next_flac_frame_header(&pFlac->bs, pFlac->bitsPerSample, &pFlac->currentFrame.header)) {
                        DRFLAC_FREE(pFlac);
                        return NULL;
                    }
                    continue;
                } else {
                    DRFLAC_FREE(pFlac);
                    return NULL;
                }
            }
        } while (1);
    }

    return pFlac;
}



#ifndef DR_FLAC_NO_STDIO
#include <stdio.h>

static size_t drflac__on_read_stdio(void* pUserData, void* bufferOut, size_t bytesToRead)
{
    return fread(bufferOut, 1, bytesToRead, (FILE*)pUserData);
}

static drflac_bool32 drflac__on_seek_stdio(void* pUserData, int offset, drflac_seek_origin origin)
{
    drflac_assert(offset >= 0);  /* <-- Never seek backwards. */

    return fseek((FILE*)pUserData, offset, (origin == drflac_seek_origin_current) ? SEEK_CUR : SEEK_SET) == 0;
}

static FILE* drflac__fopen(const char* filename)
{
    FILE* pFile;
#if defined(_MSC_VER) && _MSC_VER >= 1400
    if (fopen_s(&pFile, filename, "rb") != 0) {
        return NULL;
    }
#else
    pFile = fopen(filename, "rb");
    if (pFile == NULL) {
        return NULL;
    }
#endif

    return pFile;
}


drflac* drflac_open_file(const char* filename)
{
    drflac* pFlac;
    FILE* pFile;

    pFile = drflac__fopen(filename);
    if (pFile == NULL) {
        return NULL;
    }

    pFlac = drflac_open(drflac__on_read_stdio, drflac__on_seek_stdio, (void*)pFile);
    if (pFlac == NULL) {
        fclose(pFile);
        return NULL;
    }

    return pFlac;
}

drflac* drflac_open_file_with_metadata(const char* filename, drflac_meta_proc onMeta, void* pUserData)
{
    drflac* pFlac;
    FILE* pFile;

    pFile = drflac__fopen(filename);
    if (pFile == NULL) {
        return NULL;
    }

    pFlac = drflac_open_with_metadata_private(drflac__on_read_stdio, drflac__on_seek_stdio, onMeta, drflac_container_unknown, (void*)pFile, pUserData);
    if (pFlac == NULL) {
        fclose(pFile);
        return pFlac;
    }

    return pFlac;
}
#endif  /* DR_FLAC_NO_STDIO */

static size_t drflac__on_read_memory(void* pUserData, void* bufferOut, size_t bytesToRead)
{
    drflac__memory_stream* memoryStream = (drflac__memory_stream*)pUserData;
    size_t bytesRemaining;

    drflac_assert(memoryStream != NULL);
    drflac_assert(memoryStream->dataSize >= memoryStream->currentReadPos);

    bytesRemaining = memoryStream->dataSize - memoryStream->currentReadPos;
    if (bytesToRead > bytesRemaining) {
        bytesToRead = bytesRemaining;
    }

    if (bytesToRead > 0) {
        drflac_copy_memory(bufferOut, memoryStream->data + memoryStream->currentReadPos, bytesToRead);
        memoryStream->currentReadPos += bytesToRead;
    }

    return bytesToRead;
}

static drflac_bool32 drflac__on_seek_memory(void* pUserData, int offset, drflac_seek_origin origin)
{
    drflac__memory_stream* memoryStream = (drflac__memory_stream*)pUserData;

    drflac_assert(memoryStream != NULL);
    drflac_assert(offset >= 0); /* <-- Never seek backwards. */

    if (offset > (drflac_int64)memoryStream->dataSize) {
        return DRFLAC_FALSE;
    }

    if (origin == drflac_seek_origin_current) {
        if (memoryStream->currentReadPos + offset <= memoryStream->dataSize) {
            memoryStream->currentReadPos += offset;
        } else {
            return DRFLAC_FALSE;  /* Trying to seek too far forward. */
        }
    } else {
        if ((drflac_uint32)offset <= memoryStream->dataSize) {
            memoryStream->currentReadPos = offset;
        } else {
            return DRFLAC_FALSE;  /* Trying to seek too far forward. */
        }
    }

    return DRFLAC_TRUE;
}

drflac* drflac_open_memory(const void* data, size_t dataSize)
{
    drflac__memory_stream memoryStream;
    drflac* pFlac;

    memoryStream.data = (const unsigned char*)data;
    memoryStream.dataSize = dataSize;
    memoryStream.currentReadPos = 0;
    pFlac = drflac_open(drflac__on_read_memory, drflac__on_seek_memory, &memoryStream);
    if (pFlac == NULL) {
        return NULL;
    }

    pFlac->memoryStream = memoryStream;

    /* This is an awful hack... */
#ifndef DR_FLAC_NO_OGG
    if (pFlac->container == drflac_container_ogg)
    {
        drflac_oggbs* oggbs = (drflac_oggbs*)pFlac->_oggbs;
        oggbs->pUserData = &pFlac->memoryStream;
    }
    else
#endif
    {
        pFlac->bs.pUserData = &pFlac->memoryStream;
    }

    return pFlac;
}

drflac* drflac_open_memory_with_metadata(const void* data, size_t dataSize, drflac_meta_proc onMeta, void* pUserData)
{
    drflac__memory_stream memoryStream;
    drflac* pFlac;

    memoryStream.data = (const unsigned char*)data;
    memoryStream.dataSize = dataSize;
    memoryStream.currentReadPos = 0;
    pFlac = drflac_open_with_metadata_private(drflac__on_read_memory, drflac__on_seek_memory, onMeta, drflac_container_unknown, &memoryStream, pUserData);
    if (pFlac == NULL) {
        return NULL;
    }

    pFlac->memoryStream = memoryStream;

    /* This is an awful hack... */
#ifndef DR_FLAC_NO_OGG
    if (pFlac->container == drflac_container_ogg)
    {
        drflac_oggbs* oggbs = (drflac_oggbs*)pFlac->_oggbs;
        oggbs->pUserData = &pFlac->memoryStream;
    }
    else
#endif
    {
        pFlac->bs.pUserData = &pFlac->memoryStream;
    }

    return pFlac;
}



drflac* drflac_open(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData)
{
    return drflac_open_with_metadata_private(onRead, onSeek, NULL, drflac_container_unknown, pUserData, pUserData);
}
drflac* drflac_open_relaxed(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_container container, void* pUserData)
{
    return drflac_open_with_metadata_private(onRead, onSeek, NULL, container, pUserData, pUserData);
}

drflac* drflac_open_with_metadata(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, void* pUserData)
{
    return drflac_open_with_metadata_private(onRead, onSeek, onMeta, drflac_container_unknown, pUserData, pUserData);
}
drflac* drflac_open_with_metadata_relaxed(drflac_read_proc onRead, drflac_seek_proc onSeek, drflac_meta_proc onMeta, drflac_container container, void* pUserData)
{
    return drflac_open_with_metadata_private(onRead, onSeek, onMeta, container, pUserData, pUserData);
}

void drflac_close(drflac* pFlac)
{
    if (pFlac == NULL) {
        return;
    }

#ifndef DR_FLAC_NO_STDIO
    /*
    If we opened the file with drflac_open_file() we will want to close the file handle. We can know whether or not drflac_open_file()
    was used by looking at the callbacks.
    */
    if (pFlac->bs.onRead == drflac__on_read_stdio) {
        fclose((FILE*)pFlac->bs.pUserData);
    }

#ifndef DR_FLAC_NO_OGG
    /* Need to clean up Ogg streams a bit differently due to the way the bit streaming is chained. */
    if (pFlac->container == drflac_container_ogg) {
        drflac_oggbs* oggbs = (drflac_oggbs*)pFlac->_oggbs;
        drflac_assert(pFlac->bs.onRead == drflac__on_read_ogg);

        if (oggbs->onRead == drflac__on_read_stdio) {
            fclose((FILE*)oggbs->pUserData);
        }
    }
#endif
#endif

    DRFLAC_FREE(pFlac);
}

drflac_uint64 drflac__read_s32__misaligned(drflac* pFlac, drflac_uint64 samplesToRead, drflac_int32* bufferOut)
{
    unsigned int channelCount = drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
    drflac_uint64 samplesRead;

    /* We should never be calling this when the number of samples to read is >= the sample count. */
    drflac_assert(samplesToRead < channelCount);
    drflac_assert(pFlac->currentFrame.samplesRemaining > 0 && samplesToRead <= pFlac->currentFrame.samplesRemaining);

    samplesRead = 0;
    while (samplesToRead > 0) {
        drflac_uint64 totalSamplesInFrame = pFlac->currentFrame.header.blockSize * channelCount;
        drflac_uint64 samplesReadFromFrameSoFar = totalSamplesInFrame - pFlac->currentFrame.samplesRemaining;
        drflac_uint64 channelIndex = samplesReadFromFrameSoFar % channelCount;
        drflac_uint64 nextSampleInFrame = samplesReadFromFrameSoFar / channelCount;
        int decodedSample = 0;

        switch (pFlac->currentFrame.header.channelAssignment)
        {
            case DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE:
            {
                if (channelIndex == 0) {
                    decodedSample = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);
                } else {
                    int side = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);
                    int left = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex - 1].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex - 1].wastedBitsPerSample);
                    decodedSample = left - side;
                }
            } break;

            case DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE:
            {
                if (channelIndex == 0) {
                    int side  = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);
                    int right = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 1].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 1].wastedBitsPerSample);
                    decodedSample = side + right;
                } else {
                    decodedSample = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);
                }
            } break;

            case DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE:
            {
                int mid;
                int side;
                if (channelIndex == 0) {
                    mid  = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);
                    side = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 1].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 1].wastedBitsPerSample);

                    mid = (((unsigned int)mid) << 1) | (side & 0x01);
                    decodedSample = (mid + side) >> 1;
                } else {
                    mid  = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex - 1].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex - 1].wastedBitsPerSample);
                    side = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);

                    mid = (((unsigned int)mid) << 1) | (side & 0x01);
                    decodedSample = (mid - side) >> 1;
                }
            } break;

            case DRFLAC_CHANNEL_ASSIGNMENT_INDEPENDENT:
            default:
            {
                decodedSample = (int)((drflac_uint32)pFlac->currentFrame.subframes[channelIndex + 0].pDecodedSamples[nextSampleInFrame] << pFlac->currentFrame.subframes[channelIndex + 0].wastedBitsPerSample);
            } break;
        }

        decodedSample = (int)((drflac_uint32)decodedSample << (32 - pFlac->bitsPerSample));

        if (bufferOut) {
            *bufferOut++ = decodedSample;
        }

        samplesRead += 1;
        pFlac->currentFrame.samplesRemaining -= 1;
        samplesToRead -= 1;
    }

    return samplesRead;
}

drflac_uint64 drflac_read_s32(drflac* pFlac, drflac_uint64 samplesToRead, drflac_int32* bufferOut)
{
    drflac_uint64 samplesRead;

    /* Note that <bufferOut> is allowed to be null, in which case this will act like a seek. */
    if (pFlac == NULL || samplesToRead == 0) {
        return 0;
    }

    if (bufferOut == NULL) {
        return drflac__seek_forward_by_samples(pFlac, samplesToRead);
    }

    samplesRead = 0;
    while (samplesToRead > 0) {
        /* If we've run out of samples in this frame, go to the next. */
        if (pFlac->currentFrame.samplesRemaining == 0) {
            if (!drflac__read_and_decode_next_flac_frame(pFlac)) {
                break;  /* Couldn't read the next frame, so just break from the loop and return. */
            }
        } else {
            /* Here is where we grab the samples and interleave them. */
            unsigned int channelCount = drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
            drflac_uint64 totalSamplesInFrame = pFlac->currentFrame.header.blockSize * channelCount;
            drflac_uint64 samplesReadFromFrameSoFar = totalSamplesInFrame - pFlac->currentFrame.samplesRemaining;
            drflac_uint64 misalignedSampleCount = samplesReadFromFrameSoFar % channelCount;
            drflac_uint64 alignedSampleCountPerChannel;
            drflac_uint64 firstAlignedSampleInFrame;
            unsigned int unusedBitsPerSample;
            drflac_uint64 alignedSamplesRead;

            if (misalignedSampleCount > 0) {
                drflac_uint64 misalignedSamplesRead = drflac__read_s32__misaligned(pFlac, misalignedSampleCount, bufferOut);
                samplesRead               += misalignedSamplesRead;
                samplesReadFromFrameSoFar += misalignedSamplesRead;
                bufferOut                 += misalignedSamplesRead;
                samplesToRead             -= misalignedSamplesRead;
                pFlac->currentSample      += misalignedSamplesRead;
            }


            alignedSampleCountPerChannel = samplesToRead / channelCount;
            if (alignedSampleCountPerChannel > pFlac->currentFrame.samplesRemaining / channelCount) {
                alignedSampleCountPerChannel = pFlac->currentFrame.samplesRemaining / channelCount;
            }

            firstAlignedSampleInFrame = samplesReadFromFrameSoFar / channelCount;
            unusedBitsPerSample = 32 - pFlac->bitsPerSample;

            switch (pFlac->currentFrame.header.channelAssignment)
            {
                case DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE:
                {
                    drflac_uint64 i;
                    const drflac_int32* pDecodedSamples0 = pFlac->currentFrame.subframes[0].pDecodedSamples + firstAlignedSampleInFrame;
                    const drflac_int32* pDecodedSamples1 = pFlac->currentFrame.subframes[1].pDecodedSamples + firstAlignedSampleInFrame;

                    for (i = 0; i < alignedSampleCountPerChannel; ++i) {
                        int left  = (int)((drflac_uint32)pDecodedSamples0[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample));
                        int side  = (int)((drflac_uint32)pDecodedSamples1[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample));
                        int right = left - side;

                        bufferOut[i*2+0] = left;
                        bufferOut[i*2+1] = right;
                    }
                } break;

                case DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE:
                {
                    drflac_uint64 i;
                    const drflac_int32* pDecodedSamples0 = pFlac->currentFrame.subframes[0].pDecodedSamples + firstAlignedSampleInFrame;
                    const drflac_int32* pDecodedSamples1 = pFlac->currentFrame.subframes[1].pDecodedSamples + firstAlignedSampleInFrame;

                    for (i = 0; i < alignedSampleCountPerChannel; ++i) {
                        int side  = (int)((drflac_uint32)pDecodedSamples0[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample));
                        int right = (int)((drflac_uint32)pDecodedSamples1[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample));
                        int left  = right + side;

                        bufferOut[i*2+0] = left;
                        bufferOut[i*2+1] = right;
                    }
                } break;

                case DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE:
                {
                    drflac_uint64 i;
                    const drflac_int32* pDecodedSamples0 = pFlac->currentFrame.subframes[0].pDecodedSamples + firstAlignedSampleInFrame;
                    const drflac_int32* pDecodedSamples1 = pFlac->currentFrame.subframes[1].pDecodedSamples + firstAlignedSampleInFrame;

                    for (i = 0; i < alignedSampleCountPerChannel; ++i) {
                        int mid  = (int)((drflac_uint32)pDecodedSamples0[i] << pFlac->currentFrame.subframes[0].wastedBitsPerSample);
                        int side = (int)((drflac_uint32)pDecodedSamples1[i] << pFlac->currentFrame.subframes[1].wastedBitsPerSample);
                        
                        mid = (((drflac_uint32)mid) << 1) | (side & 0x01);

                        bufferOut[i*2+0] = (drflac_int32)((drflac_uint32)((mid + side) >> 1) << (unusedBitsPerSample));
                        bufferOut[i*2+1] = (drflac_int32)((drflac_uint32)((mid - side) >> 1) << (unusedBitsPerSample));
                    }
                } break;

                case DRFLAC_CHANNEL_ASSIGNMENT_INDEPENDENT:
                default:
                {
                    if (pFlac->currentFrame.header.channelAssignment == 1) /* 1 = Stereo */
                    {
                        /* Stereo optimized inner loop unroll. */
                        drflac_uint64 i;
                        const drflac_int32* pDecodedSamples0 = pFlac->currentFrame.subframes[0].pDecodedSamples + firstAlignedSampleInFrame;
                        const drflac_int32* pDecodedSamples1 = pFlac->currentFrame.subframes[1].pDecodedSamples + firstAlignedSampleInFrame;

                        for (i = 0; i < alignedSampleCountPerChannel; ++i) {
                            bufferOut[i*2+0] = (drflac_int32)((drflac_uint32)pDecodedSamples0[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample));
                            bufferOut[i*2+1] = (drflac_int32)((drflac_uint32)pDecodedSamples1[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample));
                        }
                    }
                    else
                    {
                        /* Generic interleaving. */
                        drflac_uint64 i;
                        for (i = 0; i < alignedSampleCountPerChannel; ++i) {
                            unsigned int j;
                            for (j = 0; j < channelCount; ++j) {
                                bufferOut[(i*channelCount)+j] = (drflac_int32)((drflac_uint32)(pFlac->currentFrame.subframes[j].pDecodedSamples[firstAlignedSampleInFrame + i]) << (unusedBitsPerSample + pFlac->currentFrame.subframes[j].wastedBitsPerSample));
                            }
                        }
                    }
                } break;
            }

            alignedSamplesRead = alignedSampleCountPerChannel * channelCount;
            samplesRead               += alignedSamplesRead;
            samplesReadFromFrameSoFar += alignedSamplesRead;
            bufferOut                 += alignedSamplesRead;
            samplesToRead             -= alignedSamplesRead;
            pFlac->currentSample      += alignedSamplesRead;
            pFlac->currentFrame.samplesRemaining -= (unsigned int)alignedSamplesRead;


            /* At this point we may still have some excess samples left to read. */
            if (samplesToRead > 0 && pFlac->currentFrame.samplesRemaining > 0) {
                drflac_uint64 excessSamplesRead = 0;
                if (samplesToRead < pFlac->currentFrame.samplesRemaining) {
                    excessSamplesRead = drflac__read_s32__misaligned(pFlac, samplesToRead, bufferOut);
                } else {
                    excessSamplesRead = drflac__read_s32__misaligned(pFlac, pFlac->currentFrame.samplesRemaining, bufferOut);
                }

                samplesRead               += excessSamplesRead;
                samplesReadFromFrameSoFar += excessSamplesRead;
                bufferOut                 += excessSamplesRead;
                samplesToRead             -= excessSamplesRead;
                pFlac->currentSample      += excessSamplesRead;
            }
        }
    }

    return samplesRead;
}

drflac_uint64 drflac_read_pcm_frames_s32(drflac* pFlac, drflac_uint64 framesToRead, drflac_int32* pBufferOut)
{
#if defined(_MSC_VER) && !defined(__clang__)
    #pragma warning(push)
    #pragma warning(disable:4996)   /* was declared deprecated */
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    return drflac_read_s32(pFlac, framesToRead*pFlac->channels, pBufferOut) / pFlac->channels;
#if defined(_MSC_VER) && !defined(__clang__)
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
}


drflac_uint64 drflac_read_s16(drflac* pFlac, drflac_uint64 samplesToRead, drflac_int16* pBufferOut)
{
    /* This reads samples in 2 passes and can probably be optimized. */
    drflac_uint64 totalSamplesRead = 0;

#if defined(_MSC_VER) && !defined(__clang__)
    #pragma warning(push)
    #pragma warning(disable:4996)   /* was declared deprecated */
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

    while (samplesToRead > 0) {
        drflac_uint64 i;
        drflac_int32 samples32[4096];
        drflac_uint64 samplesJustRead = drflac_read_s32(pFlac, (samplesToRead > 4096) ? 4096 : samplesToRead, samples32);
        if (samplesJustRead == 0) {
            break;  /* Reached the end. */
        }

        /* s32 -> s16 */
        for (i = 0; i < samplesJustRead; ++i) {
            pBufferOut[i] = (drflac_int16)(samples32[i] >> 16);
        }

        totalSamplesRead += samplesJustRead;
        samplesToRead    -= samplesJustRead;
        pBufferOut       += samplesJustRead;
    }

#if defined(_MSC_VER) && !defined(__clang__)
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

    return totalSamplesRead;
}

drflac_uint64 drflac_read_pcm_frames_s16(drflac* pFlac, drflac_uint64 framesToRead, drflac_int16* pBufferOut)
{
    /* This reads samples in 2 passes and can probably be optimized. */
    drflac_uint64 totalPCMFramesRead = 0;

    while (framesToRead > 0) {
        drflac_uint64 iFrame;
        drflac_int32 samples32[4096];
        drflac_uint64 framesJustRead = drflac_read_pcm_frames_s32(pFlac, (framesToRead > 4096/pFlac->channels) ? 4096/pFlac->channels : framesToRead, samples32);
        if (framesJustRead == 0) {
            break;  /* Reached the end. */
        }

        /* s32 -> s16 */
        for (iFrame = 0; iFrame < framesJustRead; ++iFrame) {
            drflac_uint32 iChannel;
            for (iChannel = 0; iChannel < pFlac->channels; ++iChannel) {
                drflac_uint64 iSample = iFrame*pFlac->channels + iChannel;
                pBufferOut[iSample] = (drflac_int16)(samples32[iSample] >> 16);
            }
        }

        totalPCMFramesRead += framesJustRead;
        framesToRead       -= framesJustRead;
        pBufferOut         += framesJustRead * pFlac->channels;
    }

    return totalPCMFramesRead;
}


drflac_uint64 drflac_read_f32(drflac* pFlac, drflac_uint64 samplesToRead, float* pBufferOut)
{
    /* This reads samples in 2 passes and can probably be optimized. */
    drflac_uint64 totalSamplesRead = 0;

#if defined(_MSC_VER) && !defined(__clang__)
    #pragma warning(push)
    #pragma warning(disable:4996)   /* was declared deprecated */
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

    while (samplesToRead > 0) {
        drflac_uint64 i;
        drflac_int32 samples32[4096];
        drflac_uint64 samplesJustRead = drflac_read_s32(pFlac, (samplesToRead > 4096) ? 4096 : samplesToRead, samples32);
        if (samplesJustRead == 0) {
            break;  /* Reached the end. */
        }

        /* s32 -> f32 */
        for (i = 0; i < samplesJustRead; ++i) {
            pBufferOut[i] = (float)(samples32[i] / 2147483648.0);
        }

        totalSamplesRead += samplesJustRead;
        samplesToRead    -= samplesJustRead;
        pBufferOut       += samplesJustRead;
    }

#if defined(_MSC_VER) && !defined(__clang__)
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif

    return totalSamplesRead;
}

#if 0
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_left_side__reference(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    for (i = 0; i < frameCount; ++i) {
        int left  = pInputSamples0[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample);
        int side  = pInputSamples1[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample);
        int right = left - side;

        pOutputSamples[i*2+0] = (float)(left / 2147483648.0);
        pOutputSamples[i*2+1] = (float)(right / 2147483648.0);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_left_side__scalar(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    drflac_uint64 frameCount4 = frameCount >> 2;

    float factor = 1 / 2147483648.0;

    drflac_int32 shift0 = unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample;
    drflac_int32 shift1 = unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample;
    for (i = 0; i < frameCount4; ++i) {
        drflac_int32 left0 = pInputSamples0[i*4+0] << shift0;
        drflac_int32 left1 = pInputSamples0[i*4+1] << shift0;
        drflac_int32 left2 = pInputSamples0[i*4+2] << shift0;
        drflac_int32 left3 = pInputSamples0[i*4+3] << shift0;

        drflac_int32 side0 = pInputSamples1[i*4+0] << shift1;
        drflac_int32 side1 = pInputSamples1[i*4+1] << shift1;
        drflac_int32 side2 = pInputSamples1[i*4+2] << shift1;
        drflac_int32 side3 = pInputSamples1[i*4+3] << shift1;

        drflac_int32 right0 = left0 - side0;
        drflac_int32 right1 = left1 - side1;
        drflac_int32 right2 = left2 - side2;
        drflac_int32 right3 = left3 - side3;

        pOutputSamples[i*8+0] = left0  * factor;
        pOutputSamples[i*8+1] = right0 * factor;
        pOutputSamples[i*8+2] = left1  * factor;
        pOutputSamples[i*8+3] = right1 * factor;
        pOutputSamples[i*8+4] = left2  * factor;
        pOutputSamples[i*8+5] = right2 * factor;
        pOutputSamples[i*8+6] = left3  * factor;
        pOutputSamples[i*8+7] = right3 * factor;
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        int left  = pInputSamples0[i] << shift0;
        int side  = pInputSamples1[i] << shift1;
        int right = left - side;

        pOutputSamples[i*2+0] = (float)(left  * factor);
        pOutputSamples[i*2+1] = (float)(right * factor);
    }
}

#if defined(DRFLAC_SUPPORT_SSE2)
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_left_side__sse2(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 frameCount4;
    __m128 factor;
    int shift0;
    int shift1;
    drflac_uint64 i;

    drflac_assert(pFlac->bitsPerSample <= 24);

    frameCount4 = frameCount >> 2;

    factor = _mm_set1_ps(1.0f / 8388608.0f);
    shift0 = (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample) - 8;
    shift1 = (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample) - 8;

    for (i = 0; i < frameCount4; ++i) {
        __m128i inputSample0 = _mm_loadu_si128((const __m128i*)pInputSamples0 + i);
        __m128i inputSample1 = _mm_loadu_si128((const __m128i*)pInputSamples1 + i);

        __m128i left  = _mm_slli_epi32(inputSample0, shift0);
        __m128i side  = _mm_slli_epi32(inputSample1, shift1);
        __m128i right = _mm_sub_epi32(left, side);
        __m128 leftf  = _mm_mul_ps(_mm_cvtepi32_ps(left),  factor);
        __m128 rightf = _mm_mul_ps(_mm_cvtepi32_ps(right), factor);

        pOutputSamples[i*8+0] = ((float*)&leftf)[0];
        pOutputSamples[i*8+1] = ((float*)&rightf)[0];
        pOutputSamples[i*8+2] = ((float*)&leftf)[1];
        pOutputSamples[i*8+3] = ((float*)&rightf)[1];
        pOutputSamples[i*8+4] = ((float*)&leftf)[2];
        pOutputSamples[i*8+5] = ((float*)&rightf)[2];
        pOutputSamples[i*8+6] = ((float*)&leftf)[3];
        pOutputSamples[i*8+7] = ((float*)&rightf)[3];
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        int left  = pInputSamples0[i] << shift0;
        int side  = pInputSamples1[i] << shift1;
        int right = left - side;

        pOutputSamples[i*2+0] = (float)(left  / 8388608.0f);
        pOutputSamples[i*2+1] = (float)(right / 8388608.0f);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_left_side(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
#if defined(DRFLAC_SUPPORT_SSE2)
    if (drflac__gIsSSE2Supported && pFlac->bitsPerSample <= 24) {
        drflac_read_pcm_frames_f32__decode_left_side__sse2(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
    } else
#endif
    {
        /* Scalar fallback. */
#if 0
        drflac_read_pcm_frames_f32__decode_left_side__reference(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#else
        drflac_read_pcm_frames_f32__decode_left_side__scalar(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#endif
    }
}


#if 0
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_right_side__reference(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    for (i = 0; i < frameCount; ++i) {
        int side  = pInputSamples0[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample);
        int right = pInputSamples1[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample);
        int left  = right + side;

        pOutputSamples[i*2+0] = (float)(left / 2147483648.0);
        pOutputSamples[i*2+1] = (float)(right / 2147483648.0);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_right_side__scalar(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    drflac_uint64 frameCount4 = frameCount >> 2;

    float factor = 1 / 2147483648.0;

    drflac_int32 shift0 = unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample;
    drflac_int32 shift1 = unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample;
    for (i = 0; i < frameCount4; ++i) {
        drflac_int32 side0  = pInputSamples0[i*4+0] << shift0;
        drflac_int32 side1  = pInputSamples0[i*4+1] << shift0;
        drflac_int32 side2  = pInputSamples0[i*4+2] << shift0;
        drflac_int32 side3  = pInputSamples0[i*4+3] << shift0;

        drflac_int32 right0 = pInputSamples1[i*4+0] << shift1;
        drflac_int32 right1 = pInputSamples1[i*4+1] << shift1;
        drflac_int32 right2 = pInputSamples1[i*4+2] << shift1;
        drflac_int32 right3 = pInputSamples1[i*4+3] << shift1;

        drflac_int32 left0 = right0 + side0;
        drflac_int32 left1 = right1 + side1;
        drflac_int32 left2 = right2 + side2;
        drflac_int32 left3 = right3 + side3;

        pOutputSamples[i*8+0] = left0  * factor;
        pOutputSamples[i*8+1] = right0 * factor;
        pOutputSamples[i*8+2] = left1  * factor;
        pOutputSamples[i*8+3] = right1 * factor;
        pOutputSamples[i*8+4] = left2  * factor;
        pOutputSamples[i*8+5] = right2 * factor;
        pOutputSamples[i*8+6] = left3  * factor;
        pOutputSamples[i*8+7] = right3 * factor;
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        int side  = pInputSamples0[i] << shift0;
        int right = pInputSamples1[i] << shift1;
        int left  = right + side;

        pOutputSamples[i*2+0] = (float)(left  * factor);
        pOutputSamples[i*2+1] = (float)(right * factor);
    }
}

#if defined(DRFLAC_SUPPORT_SSE2)
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_right_side__sse2(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 frameCount4;
    __m128 factor;
    int shift0;
    int shift1;
    drflac_uint64 i;

    drflac_assert(pFlac->bitsPerSample <= 24);

    frameCount4 = frameCount >> 2;

    factor = _mm_set1_ps(1.0f / 8388608.0f);
    shift0 = (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample) - 8;
    shift1 = (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample) - 8;

    for (i = 0; i < frameCount4; ++i) {
        __m128i inputSample0 = _mm_loadu_si128((const __m128i*)pInputSamples0 + i);
        __m128i inputSample1 = _mm_loadu_si128((const __m128i*)pInputSamples1 + i);

        __m128i side  = _mm_slli_epi32(inputSample0, shift0);
        __m128i right = _mm_slli_epi32(inputSample1, shift1);
        __m128i left  = _mm_add_epi32(right, side);
        __m128 leftf  = _mm_mul_ps(_mm_cvtepi32_ps(left),  factor);
        __m128 rightf = _mm_mul_ps(_mm_cvtepi32_ps(right), factor);

        pOutputSamples[i*8+0] = ((float*)&leftf)[0];
        pOutputSamples[i*8+1] = ((float*)&rightf)[0];
        pOutputSamples[i*8+2] = ((float*)&leftf)[1];
        pOutputSamples[i*8+3] = ((float*)&rightf)[1];
        pOutputSamples[i*8+4] = ((float*)&leftf)[2];
        pOutputSamples[i*8+5] = ((float*)&rightf)[2];
        pOutputSamples[i*8+6] = ((float*)&leftf)[3];
        pOutputSamples[i*8+7] = ((float*)&rightf)[3];
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        int side  = pInputSamples0[i] << shift0;
        int right = pInputSamples1[i] << shift1;
        int left  = right + side;

        pOutputSamples[i*2+0] = (float)(left  / 8388608.0f);
        pOutputSamples[i*2+1] = (float)(right / 8388608.0f);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_right_side(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
#if defined(DRFLAC_SUPPORT_SSE2)
    if (drflac__gIsSSE2Supported && pFlac->bitsPerSample <= 24) {
        drflac_read_pcm_frames_f32__decode_right_side__sse2(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
    } else
#endif
    {
        /* Scalar fallback. */
#if 0
        drflac_read_pcm_frames_f32__decode_right_side__reference(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#else
        drflac_read_pcm_frames_f32__decode_right_side__scalar(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#endif
    }
}


#if 0
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_mid_side__reference(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    for (drflac_uint64 i = 0; i < frameCount; ++i) {
        int mid  = pInputSamples0[i] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
        int side = pInputSamples1[i] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
                        
        mid = (((drflac_uint32)mid) << 1) | (side & 0x01);

        pOutputSamples[i*2+0] = (float)((((mid + side) >> 1) << (unusedBitsPerSample)) / 2147483648.0);
        pOutputSamples[i*2+1] = (float)((((mid - side) >> 1) << (unusedBitsPerSample)) / 2147483648.0);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_mid_side__scalar(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    drflac_uint64 frameCount4 = frameCount >> 2;

    float factor = 1 / 2147483648.0;

    int shift = unusedBitsPerSample;
    if (shift > 0) {
        shift -= 1;
        for (i = 0; i < frameCount4; ++i) {
            int temp0L;
            int temp1L;
            int temp2L;
            int temp3L;
            int temp0R;
            int temp1R;
            int temp2R;
            int temp3R;

            int mid0  = pInputSamples0[i*4+0] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int mid1  = pInputSamples0[i*4+1] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int mid2  = pInputSamples0[i*4+2] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int mid3  = pInputSamples0[i*4+3] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;

            int side0 = pInputSamples1[i*4+0] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
            int side1 = pInputSamples1[i*4+1] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
            int side2 = pInputSamples1[i*4+2] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
            int side3 = pInputSamples1[i*4+3] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;

            mid0 = (((drflac_uint32)mid0) << 1) | (side0 & 0x01);
            mid1 = (((drflac_uint32)mid1) << 1) | (side1 & 0x01);
            mid2 = (((drflac_uint32)mid2) << 1) | (side2 & 0x01);
            mid3 = (((drflac_uint32)mid3) << 1) | (side3 & 0x01);

            temp0L = ((mid0 + side0) << shift);
            temp1L = ((mid1 + side1) << shift);
            temp2L = ((mid2 + side2) << shift);
            temp3L = ((mid3 + side3) << shift);

            temp0R = ((mid0 - side0) << shift);
            temp1R = ((mid1 - side1) << shift);
            temp2R = ((mid2 - side2) << shift);
            temp3R = ((mid3 - side3) << shift);

            pOutputSamples[i*8+0] = (float)(temp0L * factor);
            pOutputSamples[i*8+1] = (float)(temp0R * factor);
            pOutputSamples[i*8+2] = (float)(temp1L * factor);
            pOutputSamples[i*8+3] = (float)(temp1R * factor);
            pOutputSamples[i*8+4] = (float)(temp2L * factor);
            pOutputSamples[i*8+5] = (float)(temp2R * factor);
            pOutputSamples[i*8+6] = (float)(temp3L * factor);
            pOutputSamples[i*8+7] = (float)(temp3R * factor);
        }
    } else {
        for (i = 0; i < frameCount4; ++i) {
            int temp0L;
            int temp1L;
            int temp2L;
            int temp3L;
            int temp0R;
            int temp1R;
            int temp2R;
            int temp3R;

            int mid0  = pInputSamples0[i*4+0] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int mid1  = pInputSamples0[i*4+1] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int mid2  = pInputSamples0[i*4+2] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int mid3  = pInputSamples0[i*4+3] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;

            int side0 = pInputSamples1[i*4+0] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
            int side1 = pInputSamples1[i*4+1] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
            int side2 = pInputSamples1[i*4+2] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
            int side3 = pInputSamples1[i*4+3] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;

            mid0 = (((drflac_uint32)mid0) << 1) | (side0 & 0x01);
            mid1 = (((drflac_uint32)mid1) << 1) | (side1 & 0x01);
            mid2 = (((drflac_uint32)mid2) << 1) | (side2 & 0x01);
            mid3 = (((drflac_uint32)mid3) << 1) | (side3 & 0x01);

            temp0L = ((mid0 + side0) >> 1);
            temp1L = ((mid1 + side1) >> 1);
            temp2L = ((mid2 + side2) >> 1);
            temp3L = ((mid3 + side3) >> 1);

            temp0R = ((mid0 - side0) >> 1);
            temp1R = ((mid1 - side1) >> 1);
            temp2R = ((mid2 - side2) >> 1);
            temp3R = ((mid3 - side3) >> 1);

            pOutputSamples[i*8+0] = (float)(temp0L * factor);
            pOutputSamples[i*8+1] = (float)(temp0R * factor);
            pOutputSamples[i*8+2] = (float)(temp1L * factor);
            pOutputSamples[i*8+3] = (float)(temp1R * factor);
            pOutputSamples[i*8+4] = (float)(temp2L * factor);
            pOutputSamples[i*8+5] = (float)(temp2R * factor);
            pOutputSamples[i*8+6] = (float)(temp3L * factor);
            pOutputSamples[i*8+7] = (float)(temp3R * factor);
        }
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        int mid  = pInputSamples0[i] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
        int side = pInputSamples1[i] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
                        
        mid = (((drflac_uint32)mid) << 1) | (side & 0x01);

        pOutputSamples[i*2+0] = (float)((((mid + side) >> 1) << unusedBitsPerSample) * factor);
        pOutputSamples[i*2+1] = (float)((((mid - side) >> 1) << unusedBitsPerSample) * factor);
    }
}

#if defined(DRFLAC_SUPPORT_SSE2)
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_mid_side__sse2(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    drflac_uint64 frameCount4;
    float factor;
    int shift;
    __m128 factor128;

    drflac_assert(pFlac->bitsPerSample <= 24);

    frameCount4 = frameCount >> 2;

    factor = 1.0f / 8388608.0f;
    factor128 = _mm_set1_ps(1.0f / 8388608.0f);

    shift = unusedBitsPerSample - 8;
    if (shift == 0) {
        for (i = 0; i < frameCount4; ++i) {
            __m128i tempL;
            __m128i tempR;
            __m128  leftf;
            __m128  rightf;

            __m128i inputSample0 = _mm_loadu_si128((const __m128i*)pInputSamples0 + i);
            __m128i inputSample1 = _mm_loadu_si128((const __m128i*)pInputSamples1 + i);

            __m128i mid  = _mm_slli_epi32(inputSample0, pFlac->currentFrame.subframes[0].wastedBitsPerSample);
            __m128i side = _mm_slli_epi32(inputSample1, pFlac->currentFrame.subframes[1].wastedBitsPerSample);

            mid = _mm_or_si128(_mm_slli_epi32(mid, 1), _mm_and_si128(side, _mm_set1_epi32(0x01)));

            tempL = _mm_add_epi32(mid, side);
            tempR = _mm_sub_epi32(mid, side);

            /* Signed bit shift. */
            tempL = _mm_or_si128(_mm_srli_epi32(tempL, 1), _mm_and_si128(tempL, _mm_set1_epi32(0x80000000)));
            tempR = _mm_or_si128(_mm_srli_epi32(tempR, 1), _mm_and_si128(tempR, _mm_set1_epi32(0x80000000)));

            leftf  = _mm_mul_ps(_mm_cvtepi32_ps(tempL), factor128);
            rightf = _mm_mul_ps(_mm_cvtepi32_ps(tempR), factor128);

            pOutputSamples[i*8+0] = ((float*)&leftf)[0];
            pOutputSamples[i*8+1] = ((float*)&rightf)[0];
            pOutputSamples[i*8+2] = ((float*)&leftf)[1];
            pOutputSamples[i*8+3] = ((float*)&rightf)[1];
            pOutputSamples[i*8+4] = ((float*)&leftf)[2];
            pOutputSamples[i*8+5] = ((float*)&rightf)[2];
            pOutputSamples[i*8+6] = ((float*)&leftf)[3];
            pOutputSamples[i*8+7] = ((float*)&rightf)[3];
        }

        for (i = (frameCount4 << 2); i < frameCount; ++i) {
            int mid  = pInputSamples0[i] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int side = pInputSamples1[i] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
                        
            mid = (((drflac_uint32)mid) << 1) | (side & 0x01);

            pOutputSamples[i*2+0] = (float)(((mid + side) >> 1) * factor);
            pOutputSamples[i*2+1] = (float)(((mid - side) >> 1) * factor);
        }
    } else {
        for (i = 0; i < frameCount4; ++i) {
            __m128i inputSample0;
            __m128i inputSample1;
            __m128i mid;
            __m128i side;
            __m128i tempL;
            __m128i tempR;
            __m128 leftf;
            __m128 rightf;

            inputSample0 = _mm_loadu_si128((const __m128i*)pInputSamples0 + i);
            inputSample1 = _mm_loadu_si128((const __m128i*)pInputSamples1 + i);

            mid  = _mm_slli_epi32(inputSample0, pFlac->currentFrame.subframes[0].wastedBitsPerSample);
            side = _mm_slli_epi32(inputSample1, pFlac->currentFrame.subframes[1].wastedBitsPerSample);

            mid = _mm_or_si128(_mm_slli_epi32(mid, 1), _mm_and_si128(side, _mm_set1_epi32(0x01)));

            tempL = _mm_slli_epi32(_mm_srli_epi32(_mm_add_epi32(mid, side), 1), shift);
            tempR = _mm_slli_epi32(_mm_srli_epi32(_mm_sub_epi32(mid, side), 1), shift);

            leftf  = _mm_mul_ps(_mm_cvtepi32_ps(tempL), factor128);
            rightf = _mm_mul_ps(_mm_cvtepi32_ps(tempR), factor128);

            pOutputSamples[i*8+0] = ((float*)&leftf)[0];
            pOutputSamples[i*8+1] = ((float*)&rightf)[0];
            pOutputSamples[i*8+2] = ((float*)&leftf)[1];
            pOutputSamples[i*8+3] = ((float*)&rightf)[1];
            pOutputSamples[i*8+4] = ((float*)&leftf)[2];
            pOutputSamples[i*8+5] = ((float*)&rightf)[2];
            pOutputSamples[i*8+6] = ((float*)&leftf)[3];
            pOutputSamples[i*8+7] = ((float*)&rightf)[3];
        }

        for (i = (frameCount4 << 2); i < frameCount; ++i) {
            int mid  = pInputSamples0[i] << pFlac->currentFrame.subframes[0].wastedBitsPerSample;
            int side = pInputSamples1[i] << pFlac->currentFrame.subframes[1].wastedBitsPerSample;
                        
            mid = (((drflac_uint32)mid) << 1) | (side & 0x01);

            pOutputSamples[i*2+0] = (float)((((mid + side) >> 1) << shift) * factor);
            pOutputSamples[i*2+1] = (float)((((mid - side) >> 1) << shift) * factor);
        }
    }
}
#endif


static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_mid_side(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
#if defined(DRFLAC_SUPPORT_SSE2)
    if (drflac__gIsSSE2Supported && pFlac->bitsPerSample <= 24) {
        drflac_read_pcm_frames_f32__decode_mid_side__sse2(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
    } else
#endif
    {
        /* Scalar fallback. */
#if 0
        drflac_read_pcm_frames_f32__decode_mid_side__reference(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#else
        drflac_read_pcm_frames_f32__decode_mid_side__scalar(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#endif
    }
}

#if 0
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_independent_stereo__reference(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    for (drflac_uint64 i = 0; i < frameCount; ++i) {
        pOutputSamples[i*2+0] = (float)((pInputSamples0[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample)) / 2147483648.0);
        pOutputSamples[i*2+1] = (float)((pInputSamples1[i] << (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample)) / 2147483648.0);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_independent_stereo__scalar(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    drflac_uint64 frameCount4 = frameCount >> 2;

    float factor = 1 / 2147483648.0;

    int shift0 = (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample);
    int shift1 = (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample);

    for (i = 0; i < frameCount4; ++i) {
        int tempL0 = pInputSamples0[i*4+0] << shift0;
        int tempL1 = pInputSamples0[i*4+1] << shift0;
        int tempL2 = pInputSamples0[i*4+2] << shift0;
        int tempL3 = pInputSamples0[i*4+3] << shift0;

        int tempR0 = pInputSamples1[i*4+0] << shift1;
        int tempR1 = pInputSamples1[i*4+1] << shift1;
        int tempR2 = pInputSamples1[i*4+2] << shift1;
        int tempR3 = pInputSamples1[i*4+3] << shift1;

        pOutputSamples[i*8+0] = (float)(tempL0 * factor);
        pOutputSamples[i*8+1] = (float)(tempR0 * factor);
        pOutputSamples[i*8+2] = (float)(tempL1 * factor);
        pOutputSamples[i*8+3] = (float)(tempR1 * factor);
        pOutputSamples[i*8+4] = (float)(tempL2 * factor);
        pOutputSamples[i*8+5] = (float)(tempR2 * factor);
        pOutputSamples[i*8+6] = (float)(tempL3 * factor);
        pOutputSamples[i*8+7] = (float)(tempR3 * factor);
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        pOutputSamples[i*2+0] = (float)((pInputSamples0[i] << shift0) * factor);
        pOutputSamples[i*2+1] = (float)((pInputSamples1[i] << shift1) * factor);
    }
}

#if defined(DRFLAC_SUPPORT_SSE2)
static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_independent_stereo__sse2(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
    drflac_uint64 i;
    drflac_uint64 frameCount4 = frameCount >> 2;

    float factor = 1.0f / 8388608.0f;
    __m128 factor128 = _mm_set1_ps(1.0f / 8388608.0f);

    int shift0 = (unusedBitsPerSample + pFlac->currentFrame.subframes[0].wastedBitsPerSample) - 8;
    int shift1 = (unusedBitsPerSample + pFlac->currentFrame.subframes[1].wastedBitsPerSample) - 8;

    for (i = 0; i < frameCount4; ++i) {
        __m128i inputSample0 = _mm_loadu_si128((const __m128i*)pInputSamples0 + i);
        __m128i inputSample1 = _mm_loadu_si128((const __m128i*)pInputSamples1 + i);

        __m128i i32L = _mm_slli_epi32(inputSample0, shift0);
        __m128i i32R = _mm_slli_epi32(inputSample1, shift1);

        __m128 f32L = _mm_mul_ps(_mm_cvtepi32_ps(i32L), factor128);
        __m128 f32R = _mm_mul_ps(_mm_cvtepi32_ps(i32R), factor128);

        pOutputSamples[i*8+0] = ((float*)&f32L)[0];
        pOutputSamples[i*8+1] = ((float*)&f32R)[0];
        pOutputSamples[i*8+2] = ((float*)&f32L)[1];
        pOutputSamples[i*8+3] = ((float*)&f32R)[1];
        pOutputSamples[i*8+4] = ((float*)&f32L)[2];
        pOutputSamples[i*8+5] = ((float*)&f32R)[2];
        pOutputSamples[i*8+6] = ((float*)&f32L)[3];
        pOutputSamples[i*8+7] = ((float*)&f32R)[3];
    }

    for (i = (frameCount4 << 2); i < frameCount; ++i) {
        pOutputSamples[i*2+0] = (float)((pInputSamples0[i] << shift0) * factor);
        pOutputSamples[i*2+1] = (float)((pInputSamples1[i] << shift1) * factor);
    }
}
#endif

static DRFLAC_INLINE void drflac_read_pcm_frames_f32__decode_independent_stereo(drflac* pFlac, drflac_uint64 frameCount, drflac_int32 unusedBitsPerSample, const drflac_int32* pInputSamples0, const drflac_int32* pInputSamples1, float* pOutputSamples)
{
#if defined(DRFLAC_SUPPORT_SSE2)
    if (drflac__gIsSSE2Supported && pFlac->bitsPerSample <= 24) {
        drflac_read_pcm_frames_f32__decode_independent_stereo__sse2(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
    } else
#endif
    {
        /* Scalar fallback. */
#if 0
        drflac_read_pcm_frames_f32__decode_independent_stereo__reference(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#else
        drflac_read_pcm_frames_f32__decode_independent_stereo__scalar(pFlac, frameCount, unusedBitsPerSample, pInputSamples0, pInputSamples1, pOutputSamples);
#endif
    }
}

drflac_uint64 drflac_read_pcm_frames_f32(drflac* pFlac, drflac_uint64 framesToRead, float* pBufferOut)
{
    drflac_uint64 framesRead;

    if (pFlac == NULL || framesToRead == 0) {
        return 0;
    }

    if (pBufferOut == NULL) {
        return drflac__seek_forward_by_pcm_frames(pFlac, framesToRead);
    }

    framesRead = 0;
    while (framesToRead > 0) {
        /* If we've run out of samples in this frame, go to the next. */
        if (pFlac->currentFrame.samplesRemaining == 0) {
            if (!drflac__read_and_decode_next_flac_frame(pFlac)) {
                break;  /* Couldn't read the next frame, so just break from the loop and return. */
            }
        } else {
            unsigned int channelCount = drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
            drflac_uint64 totalFramesInPacket = pFlac->currentFrame.header.blockSize;
            drflac_uint64 framesReadFromPacketSoFar = totalFramesInPacket - (pFlac->currentFrame.samplesRemaining/channelCount);
            drflac_uint64 iFirstPCMFrame = framesReadFromPacketSoFar;
            drflac_int32 unusedBitsPerSample = 32 - pFlac->bitsPerSample;
            drflac_uint64 frameCountThisIteration = framesToRead;
            drflac_uint64 samplesReadThisIteration;

            if (frameCountThisIteration > pFlac->currentFrame.samplesRemaining / channelCount) {
                frameCountThisIteration = pFlac->currentFrame.samplesRemaining / channelCount;
            }

            if (channelCount == 2) {
                const drflac_int32* pDecodedSamples0 = pFlac->currentFrame.subframes[0].pDecodedSamples + iFirstPCMFrame;
                const drflac_int32* pDecodedSamples1 = pFlac->currentFrame.subframes[1].pDecodedSamples + iFirstPCMFrame;

                switch (pFlac->currentFrame.header.channelAssignment)
                {
                    case DRFLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE:
                    {
                        drflac_read_pcm_frames_f32__decode_left_side(pFlac, frameCountThisIteration, unusedBitsPerSample, pDecodedSamples0, pDecodedSamples1, pBufferOut);
                    } break;

                    case DRFLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE:
                    {
                        drflac_read_pcm_frames_f32__decode_right_side(pFlac, frameCountThisIteration, unusedBitsPerSample, pDecodedSamples0, pDecodedSamples1, pBufferOut);
                    } break;
                
                    case DRFLAC_CHANNEL_ASSIGNMENT_MID_SIDE:
                    {
                        drflac_read_pcm_frames_f32__decode_mid_side(pFlac, frameCountThisIteration, unusedBitsPerSample, pDecodedSamples0, pDecodedSamples1, pBufferOut);
                    } break;

                    case DRFLAC_CHANNEL_ASSIGNMENT_INDEPENDENT:
                    default:
                    {
                        drflac_read_pcm_frames_f32__decode_independent_stereo(pFlac, frameCountThisIteration, unusedBitsPerSample, pDecodedSamples0, pDecodedSamples1, pBufferOut);
                    } break;
                }
            } else {
                /* Generic interleaving. */
                drflac_uint64 i;
                for (i = 0; i < frameCountThisIteration; ++i) {
                    unsigned int j;
                    for (j = 0; j < channelCount; ++j) {
                        pBufferOut[(i*channelCount)+j] = (float)(((pFlac->currentFrame.subframes[j].pDecodedSamples[iFirstPCMFrame + i]) << (unusedBitsPerSample + pFlac->currentFrame.subframes[j].wastedBitsPerSample)) / 2147483648.0);
                    }
                }
            }

            samplesReadThisIteration = frameCountThisIteration * channelCount;
            framesRead                += frameCountThisIteration;
            framesReadFromPacketSoFar += frameCountThisIteration;
            pBufferOut                += samplesReadThisIteration;
            framesToRead              -= frameCountThisIteration;
            pFlac->currentSample      += samplesReadThisIteration;
            pFlac->currentFrame.samplesRemaining -= (unsigned int)samplesReadThisIteration;
        }
    }

    return framesRead;
}

drflac_bool32 drflac_seek_to_sample(drflac* pFlac, drflac_uint64 sampleIndex)
{
    if (pFlac == NULL) {
        return DRFLAC_FALSE;
    }

    /*
    If we don't know where the first frame begins then we can't seek. This will happen when the STREAMINFO block was not present
    when the decoder was opened.
    */
    if (pFlac->firstFramePos == 0) {
        return DRFLAC_FALSE;
    }

    if (sampleIndex == 0) {
        pFlac->currentSample = 0;
        return drflac__seek_to_first_frame(pFlac);
    } else {
        drflac_bool32 wasSuccessful = DRFLAC_FALSE;

        /* Clamp the sample to the end. */
        if (sampleIndex >= pFlac->totalSampleCount) {
            sampleIndex  = pFlac->totalSampleCount - 1;
        }

        /* If the target sample and the current sample are in the same frame we just move the position forward. */
        if (sampleIndex > pFlac->currentSample) {
            /* Forward. */
            drflac_uint32 offset = (drflac_uint32)(sampleIndex - pFlac->currentSample);
            if (pFlac->currentFrame.samplesRemaining >  offset) {
                pFlac->currentFrame.samplesRemaining -= offset;
                pFlac->currentSample = sampleIndex;
                return DRFLAC_TRUE;
            }
        } else {
            /* Backward. */
            drflac_uint32 offsetAbs = (drflac_uint32)(pFlac->currentSample - sampleIndex);
            drflac_uint32 currentFrameSampleCount = pFlac->currentFrame.header.blockSize * drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
            drflac_uint32 currentFrameSamplesConsumed = (drflac_uint32)(currentFrameSampleCount - pFlac->currentFrame.samplesRemaining);
            if (currentFrameSamplesConsumed > offsetAbs) {
                pFlac->currentFrame.samplesRemaining += offsetAbs;
                pFlac->currentSample = sampleIndex;
                return DRFLAC_TRUE;
            }
        }

        /*
        Different techniques depending on encapsulation. Using the native FLAC seektable with Ogg encapsulation is a bit awkward so
        we'll instead use Ogg's natural seeking facility.
        */
#ifndef DR_FLAC_NO_OGG
        if (pFlac->container == drflac_container_ogg)
        {
            wasSuccessful = drflac_ogg__seek_to_sample(pFlac, sampleIndex);
        }
        else
#endif
        {
            /* First try seeking via the seek table. If this fails, fall back to a brute force seek which is much slower. */
            wasSuccessful = drflac__seek_to_sample__seek_table(pFlac, sampleIndex);
            if (!wasSuccessful) {
                wasSuccessful = drflac__seek_to_sample__brute_force(pFlac, sampleIndex);
            }
        }

        pFlac->currentSample = sampleIndex;
        return wasSuccessful;
    }
}

drflac_bool32 drflac_seek_to_pcm_frame(drflac* pFlac, drflac_uint64 pcmFrameIndex)
{
    if (pFlac == NULL) {
        return DRFLAC_FALSE;
    }

    /*
    If we don't know where the first frame begins then we can't seek. This will happen when the STREAMINFO block was not present
    when the decoder was opened.
    */
    if (pFlac->firstFramePos == 0) {
        return DRFLAC_FALSE;
    }

    if (pcmFrameIndex == 0) {
        pFlac->currentSample = 0;
        return drflac__seek_to_first_frame(pFlac);
    } else {
        drflac_bool32 wasSuccessful = DRFLAC_FALSE;

        /* Clamp the sample to the end. */
        if (pcmFrameIndex >= pFlac->totalPCMFrameCount) {
            pcmFrameIndex  = pFlac->totalPCMFrameCount - 1;
        }

        /* If the target sample and the current sample are in the same frame we just move the position forward. */
        if (pcmFrameIndex*pFlac->channels > pFlac->currentSample) {
            /* Forward. */
            drflac_uint32 offset = (drflac_uint32)(pcmFrameIndex*pFlac->channels - pFlac->currentSample);
            if (pFlac->currentFrame.samplesRemaining >  offset) {
                pFlac->currentFrame.samplesRemaining -= offset;
                pFlac->currentSample = pcmFrameIndex*pFlac->channels;
                return DRFLAC_TRUE;
            }
        } else {
            /* Backward. */
            drflac_uint32 offsetAbs = (drflac_uint32)(pFlac->currentSample - pcmFrameIndex*pFlac->channels);
            drflac_uint32 currentFrameSampleCount = pFlac->currentFrame.header.blockSize * drflac__get_channel_count_from_channel_assignment(pFlac->currentFrame.header.channelAssignment);
            drflac_uint32 currentFrameSamplesConsumed = (drflac_uint32)(currentFrameSampleCount - pFlac->currentFrame.samplesRemaining);
            if (currentFrameSamplesConsumed > offsetAbs) {
                pFlac->currentFrame.samplesRemaining += offsetAbs;
                pFlac->currentSample = pcmFrameIndex*pFlac->channels;
                return DRFLAC_TRUE;
            }
        }

        /*
        Different techniques depending on encapsulation. Using the native FLAC seektable with Ogg encapsulation is a bit awkward so
        we'll instead use Ogg's natural seeking facility.
        */
#ifndef DR_FLAC_NO_OGG
        if (pFlac->container == drflac_container_ogg)
        {
            wasSuccessful = drflac_ogg__seek_to_sample(pFlac, pcmFrameIndex*pFlac->channels);
        }
        else
#endif
        {
            /* First try seeking via the seek table. If this fails, fall back to a brute force seek which is much slower. */
            wasSuccessful = drflac__seek_to_sample__seek_table(pFlac, pcmFrameIndex*pFlac->channels);
            if (!wasSuccessful) {
                wasSuccessful = drflac__seek_to_sample__brute_force(pFlac, pcmFrameIndex*pFlac->channels);
            }
        }

        pFlac->currentSample = pcmFrameIndex*pFlac->channels;
        return wasSuccessful;
    }
}



/* High Level APIs */

#if defined(SIZE_MAX)
    #define DRFLAC_SIZE_MAX  SIZE_MAX
#else
    #if defined(DRFLAC_64BIT)
        #define DRFLAC_SIZE_MAX  ((drflac_uint64)0xFFFFFFFFFFFFFFFF)
    #else
        #define DRFLAC_SIZE_MAX  0xFFFFFFFF
    #endif
#endif


/* Using a macro as the definition of the drflac__full_decode_and_close_*() API family. Sue me. */
#define DRFLAC_DEFINE_FULL_READ_AND_CLOSE(extension, type) \
static type* drflac__full_read_and_close_ ## extension (drflac* pFlac, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalPCMFrameCountOut)\
{                                                                                                                                                                   \
    type* pSampleData = NULL;                                                                                                                                       \
    drflac_uint64 totalPCMFrameCount;                                                                                                                               \
                                                                                                                                                                    \
    drflac_assert(pFlac != NULL);                                                                                                                                   \
                                                                                                                                                                    \
    totalPCMFrameCount = pFlac->totalPCMFrameCount;                                                                                                                 \
                                                                                                                                                                    \
    if (totalPCMFrameCount == 0) {                                                                                                                                  \
        type buffer[4096];                                                                                                                                          \
        drflac_uint64 pcmFramesRead;                                                                                                                                \
        size_t sampleDataBufferSize = sizeof(buffer);                                                                                                               \
                                                                                                                                                                    \
        pSampleData = (type*)DRFLAC_MALLOC(sampleDataBufferSize);                                                                                                   \
        if (pSampleData == NULL) {                                                                                                                                  \
            goto on_error;                                                                                                                                          \
        }                                                                                                                                                           \
                                                                                                                                                                    \
        while ((pcmFramesRead = (drflac_uint64)drflac_read_pcm_frames_##extension(pFlac, sizeof(buffer)/sizeof(buffer[0])/pFlac->channels, buffer)) > 0) {          \
            if (((totalPCMFrameCount + pcmFramesRead) * pFlac->channels * sizeof(type)) > sampleDataBufferSize) {                                                   \
                type* pNewSampleData;                                                                                                                               \
                                                                                                                                                                    \
                sampleDataBufferSize *= 2;                                                                                                                          \
                pNewSampleData = (type*)DRFLAC_REALLOC(pSampleData, sampleDataBufferSize);                                                                          \
                if (pNewSampleData == NULL) {                                                                                                                       \
                    DRFLAC_FREE(pSampleData);                                                                                                                       \
                    goto on_error;                                                                                                                                  \
                }                                                                                                                                                   \
                                                                                                                                                                    \
                pSampleData = pNewSampleData;                                                                                                                       \
            }                                                                                                                                                       \
                                                                                                                                                                    \
            drflac_copy_memory(pSampleData + (totalPCMFrameCount*pFlac->channels), buffer, (size_t)(pcmFramesRead*pFlac->channels*sizeof(type)));                   \
            totalPCMFrameCount += pcmFramesRead;                                                                                                                    \
        }                                                                                                                                                           \
                                                                                                                                                                    \
        /* At this point everything should be decoded, but we just want to fill the unused part buffer with silence - need to                                       \
           protect those ears from random noise! */                                                                                                                 \
        drflac_zero_memory(pSampleData + (totalPCMFrameCount*pFlac->channels), (size_t)(sampleDataBufferSize - totalPCMFrameCount*pFlac->channels*sizeof(type)));   \
    } else {                                                                                                                                                        \
        drflac_uint64 dataSize = totalPCMFrameCount*pFlac->channels*sizeof(type);                                                                                   \
        if (dataSize > DRFLAC_SIZE_MAX) {                                                                                                                           \
            goto on_error;  /* The decoded data is too big. */                                                                                                      \
        }                                                                                                                                                           \
                                                                                                                                                                    \
        pSampleData = (type*)DRFLAC_MALLOC((size_t)dataSize);    /* <-- Safe cast as per the check above. */                                                        \
        if (pSampleData == NULL) {                                                                                                                                  \
            goto on_error;                                                                                                                                          \
        }                                                                                                                                                           \
                                                                                                                                                                    \
        totalPCMFrameCount = drflac_read_pcm_frames_##extension(pFlac, pFlac->totalPCMFrameCount, pSampleData);                                                     \
    }                                                                                                                                                               \
                                                                                                                                                                    \
    if (sampleRateOut) *sampleRateOut = pFlac->sampleRate;                                                                                                          \
    if (channelsOut) *channelsOut = pFlac->channels;                                                                                                                \
    if (totalPCMFrameCountOut) *totalPCMFrameCountOut = totalPCMFrameCount;                                                                                         \
                                                                                                                                                                    \
    drflac_close(pFlac);                                                                                                                                            \
    return pSampleData;                                                                                                                                             \
                                                                                                                                                                    \
on_error:                                                                                                                                                           \
    drflac_close(pFlac);                                                                                                                                            \
    return NULL;                                                                                                                                                    \
}

DRFLAC_DEFINE_FULL_READ_AND_CLOSE(s32, drflac_int32)
DRFLAC_DEFINE_FULL_READ_AND_CLOSE(s16, drflac_int16)
DRFLAC_DEFINE_FULL_READ_AND_CLOSE(f32, float)

drflac_int32* drflac_open_and_read_pcm_frames_s32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalPCMFrameCountOut)
{
    drflac* pFlac;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalPCMFrameCountOut) {
        *totalPCMFrameCountOut = 0;
    }

    pFlac = drflac_open(onRead, onSeek, pUserData);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_s32(pFlac, channelsOut, sampleRateOut, totalPCMFrameCountOut);
}

drflac_int32* drflac_open_and_decode_s32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int32* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_and_read_pcm_frames_s32(onRead, onSeek, pUserData, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}



drflac_int16* drflac_open_and_read_pcm_frames_s16(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalPCMFrameCountOut)
{
    drflac* pFlac;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalPCMFrameCountOut) {
        *totalPCMFrameCountOut = 0;
    }

    pFlac = drflac_open(onRead, onSeek, pUserData);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_s16(pFlac, channelsOut, sampleRateOut, totalPCMFrameCountOut);
}

drflac_int16* drflac_open_and_decode_s16(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int16* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_and_read_pcm_frames_s16(onRead, onSeek, pUserData, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}


float* drflac_open_and_read_pcm_frames_f32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalPCMFrameCountOut)
{
    drflac* pFlac;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalPCMFrameCountOut) {
        *totalPCMFrameCountOut = 0;
    }

    pFlac = drflac_open(onRead, onSeek, pUserData);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_f32(pFlac, channelsOut, sampleRateOut, totalPCMFrameCountOut);
}

float* drflac_open_and_decode_f32(drflac_read_proc onRead, drflac_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    float* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_and_read_pcm_frames_f32(onRead, onSeek, pUserData, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}

#ifndef DR_FLAC_NO_STDIO
drflac_int32* drflac_open_file_and_read_pcm_frames_s32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount)
{
    drflac* pFlac;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalPCMFrameCount) {
        *totalPCMFrameCount = 0;
    }

    pFlac = drflac_open_file(filename);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_s32(pFlac, channels, sampleRate, totalPCMFrameCount);
}

drflac_int32* drflac_open_and_decode_file_s32(const char* filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int32* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_file_and_read_pcm_frames_s32(filename, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}


drflac_int16* drflac_open_file_and_read_pcm_frames_s16(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount)
{
    drflac* pFlac;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalPCMFrameCount) {
        *totalPCMFrameCount = 0;
    }

    pFlac = drflac_open_file(filename);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_s16(pFlac, channels, sampleRate, totalPCMFrameCount);
}

drflac_int16* drflac_open_and_decode_file_s16(const char* filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int16* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }
    
    pResult = drflac_open_file_and_read_pcm_frames_s16(filename, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}


float* drflac_open_file_and_read_pcm_frames_f32(const char* filename, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount)
{
    drflac* pFlac;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalPCMFrameCount) {
        *totalPCMFrameCount = 0;
    }

    pFlac = drflac_open_file(filename);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_f32(pFlac, channels, sampleRate, totalPCMFrameCount);
}

float* drflac_open_and_decode_file_f32(const char* filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    float* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_file_and_read_pcm_frames_f32(filename, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}
#endif

drflac_int32* drflac_open_memory_and_read_pcm_frames_s32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount)
{
    drflac* pFlac;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalPCMFrameCount) {
        *totalPCMFrameCount = 0;
    }

    pFlac = drflac_open_memory(data, dataSize);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_s32(pFlac, channels, sampleRate, totalPCMFrameCount);
}

drflac_int32* drflac_open_and_decode_memory_s32(const void* data, size_t dataSize, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int32* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_memory_and_read_pcm_frames_s32(data, dataSize, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}


drflac_int16* drflac_open_memory_and_read_pcm_frames_s16(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount)
{
    drflac* pFlac;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalPCMFrameCount) {
        *totalPCMFrameCount = 0;
    }

    pFlac = drflac_open_memory(data, dataSize);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_s16(pFlac, channels, sampleRate, totalPCMFrameCount);
}

drflac_int16* drflac_open_and_decode_memory_s16(const void* data, size_t dataSize, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    drflac_int16* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_memory_and_read_pcm_frames_s16(data, dataSize, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}


float* drflac_open_memory_and_read_pcm_frames_f32(const void* data, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, drflac_uint64* totalPCMFrameCount)
{
    drflac* pFlac;

    if (sampleRate) {
        *sampleRate = 0;
    }
    if (channels) {
        *channels = 0;
    }
    if (totalPCMFrameCount) {
        *totalPCMFrameCount = 0;
    }

    pFlac = drflac_open_memory(data, dataSize);
    if (pFlac == NULL) {
        return NULL;
    }

    return drflac__full_read_and_close_f32(pFlac, channels, sampleRate, totalPCMFrameCount);
}

float* drflac_open_and_decode_memory_f32(const void* data, size_t dataSize, unsigned int* channelsOut, unsigned int* sampleRateOut, drflac_uint64* totalSampleCountOut)
{
    unsigned int channels;
    unsigned int sampleRate;
    drflac_uint64 totalPCMFrameCount;
    float* pResult;

    if (channelsOut) {
        *channelsOut = 0;
    }
    if (sampleRateOut) {
        *sampleRateOut = 0;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = 0;
    }

    pResult = drflac_open_memory_and_read_pcm_frames_f32(data, dataSize, &channels, &sampleRate, &totalPCMFrameCount);
    if (pResult == NULL) {
        return NULL;
    }

    if (channelsOut) {
        *channelsOut = channels;
    }
    if (sampleRateOut) {
        *sampleRateOut = sampleRate;
    }
    if (totalSampleCountOut) {
        *totalSampleCountOut = totalPCMFrameCount * channels;
    }

    return pResult;
}


void drflac_free(void* pSampleDataReturnedByOpenAndDecode)
{
    DRFLAC_FREE(pSampleDataReturnedByOpenAndDecode);
}




void drflac_init_vorbis_comment_iterator(drflac_vorbis_comment_iterator* pIter, drflac_uint32 commentCount, const void* pComments)
{
    if (pIter == NULL) {
        return;
    }

    pIter->countRemaining = commentCount;
    pIter->pRunningData   = (const char*)pComments;
}

const char* drflac_next_vorbis_comment(drflac_vorbis_comment_iterator* pIter, drflac_uint32* pCommentLengthOut)
{
    drflac_int32 length;
    const char* pComment;
    
    /* Safety. */
    if (pCommentLengthOut) {
        *pCommentLengthOut = 0;
    }

    if (pIter == NULL || pIter->countRemaining == 0 || pIter->pRunningData == NULL) {
        return NULL;
    }

    length = drflac__le2host_32(*(const drflac_uint32*)pIter->pRunningData);
    pIter->pRunningData += 4;

    pComment = pIter->pRunningData;
    pIter->pRunningData += length;
    pIter->countRemaining -= 1;

    if (pCommentLengthOut) {
        *pCommentLengthOut = length;
    }

    return pComment;
}




void drflac_init_cuesheet_track_iterator(drflac_cuesheet_track_iterator* pIter, drflac_uint32 trackCount, const void* pTrackData)
{
    if (pIter == NULL) {
        return;
    }

    pIter->countRemaining = trackCount;
    pIter->pRunningData   = (const char*)pTrackData;
}

drflac_bool32 drflac_next_cuesheet_track(drflac_cuesheet_track_iterator* pIter, drflac_cuesheet_track* pCuesheetTrack)
{
    drflac_cuesheet_track cuesheetTrack;
    const char* pRunningData;
    drflac_uint64 offsetHi;
    drflac_uint64 offsetLo;

    if (pIter == NULL || pIter->countRemaining == 0 || pIter->pRunningData == NULL) {
        return DRFLAC_FALSE;
    }

    pRunningData = pIter->pRunningData;

    offsetHi                   = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
    offsetLo                   = drflac__be2host_32(*(const drflac_uint32*)pRunningData); pRunningData += 4;
    cuesheetTrack.offset       = offsetLo | (offsetHi << 32);
    cuesheetTrack.trackNumber  = pRunningData[0];                                         pRunningData += 1;
    drflac_copy_memory(cuesheetTrack.ISRC, pRunningData, sizeof(cuesheetTrack.ISRC));     pRunningData += 12;
    cuesheetTrack.isAudio      = (pRunningData[0] & 0x80) != 0;
    cuesheetTrack.preEmphasis  = (pRunningData[0] & 0x40) != 0;                           pRunningData += 14;
    cuesheetTrack.indexCount   = pRunningData[0];                                         pRunningData += 1;
    cuesheetTrack.pIndexPoints = (const drflac_cuesheet_track_index*)pRunningData;        pRunningData += cuesheetTrack.indexCount * sizeof(drflac_cuesheet_track_index);

    pIter->pRunningData = pRunningData;
    pIter->countRemaining -= 1;

    if (pCuesheetTrack) {
        *pCuesheetTrack = cuesheetTrack;
    }

    return DRFLAC_TRUE;
}

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
#endif  /* DR_FLAC_IMPLEMENTATION */


/*
REVISION HISTORY
================
v0.11.10 - 2019-06-26
  - Fix a compiler error.

v0.11.9 - 2019-06-16
  - Silence some ThreadSanitizer warnings.

v0.11.8 - 2019-05-21
  - Fix warnings.

v0.11.7 - 2019-05-06
  - C89 fixes.

v0.11.6 - 2019-05-05
  - Add support for C89.
  - Fix a compiler warning when CRC is disabled.
  - Change license to choice of public domain or MIT-0.

v0.11.5 - 2019-04-19
  - Fix a compiler error with GCC. 

v0.11.4 - 2019-04-17
  - Fix some warnings with GCC when compiling with -std=c99.

v0.11.3 - 2019-04-07
  - Silence warnings with GCC.

v0.11.2 - 2019-03-10
  - Fix a warning.

v0.11.1 - 2019-02-17
  - Fix a potential bug with seeking.

v0.11.0 - 2018-12-16
  - API CHANGE: Deprecated drflac_read_s32(), drflac_read_s16() and drflac_read_f32() and replaced them with 
    drflac_read_pcm_frames_s32(), drflac_read_pcm_frames_s16() and drflac_read_pcm_frames_f32(). The new APIs take
    and return PCM frame counts instead of sample counts. To upgrade you will need to change the input count by
    dividing it by the channel count, and then do the same with the return value.
  - API_CHANGE: Deprecated drflac_seek_to_sample() and replaced with drflac_seek_to_pcm_frame(). Same rules as
    the changes to drflac_read_*() apply.
  - API CHANGE: Deprecated drflac_open_and_decode_*() and replaced with drflac_open_*_and_read_*(). Same rules as
    the changes to drflac_read_*() apply.
  - Optimizations.

v0.10.0 - 2018-09-11
  - Remove the DR_FLAC_NO_WIN32_IO option and the Win32 file IO functionality. If you need to use Win32 file IO you
    need to do it yourself via the callback API.
  - Fix the clang build.
  - Fix undefined behavior.
  - Fix errors with CUESHEET metdata blocks.
  - Add an API for iterating over each cuesheet track in the CUESHEET metadata block. This works the same way as the
    Vorbis comment API.
  - Other miscellaneous bug fixes, mostly relating to invalid FLAC streams.
  - Minor optimizations.

v0.9.11 - 2018-08-29
  - Fix a bug with sample reconstruction.

v0.9.10 - 2018-08-07
  - Improve 64-bit detection.

v0.9.9 - 2018-08-05
  - Fix C++ build on older versions of GCC.

v0.9.8 - 2018-07-24
  - Fix compilation errors.

v0.9.7 - 2018-07-05
  - Fix a warning.

v0.9.6 - 2018-06-29
  - Fix some typos.

v0.9.5 - 2018-06-23
  - Fix some warnings.

v0.9.4 - 2018-06-14
  - Optimizations to seeking.
  - Clean up.

v0.9.3 - 2018-05-22
  - Bug fix.

v0.9.2 - 2018-05-12
  - Fix a compilation error due to a missing break statement.

v0.9.1 - 2018-04-29
  - Fix compilation error with Clang.

v0.9 - 2018-04-24
  - Fix Clang build.
  - Start using major.minor.revision versioning.

v0.8g - 2018-04-19
  - Fix build on non-x86/x64 architectures.

v0.8f - 2018-02-02
  - Stop pretending to support changing rate/channels mid stream.

v0.8e - 2018-02-01
  - Fix a crash when the block size of a frame is larger than the maximum block size defined by the FLAC stream.
  - Fix a crash the the Rice partition order is invalid.

v0.8d - 2017-09-22
  - Add support for decoding streams with ID3 tags. ID3 tags are just skipped.

v0.8c - 2017-09-07
  - Fix warning on non-x86/x64 architectures.

v0.8b - 2017-08-19
  - Fix build on non-x86/x64 architectures.

v0.8a - 2017-08-13
  - A small optimization for the Clang build.

v0.8 - 2017-08-12
  - API CHANGE: Rename dr_* types to drflac_*.
  - Optimizations. This brings dr_flac back to about the same class of efficiency as the reference implementation.
  - Add support for custom implementations of malloc(), realloc(), etc.
  - Add CRC checking to Ogg encapsulated streams.
  - Fix VC++ 6 build. This is only for the C++ compiler. The C compiler is not currently supported.
  - Bug fixes.

v0.7 - 2017-07-23
  - Add support for opening a stream without a header block. To do this, use drflac_open_relaxed() / drflac_open_with_metadata_relaxed().

v0.6 - 2017-07-22
  - Add support for recovering from invalid frames. With this change, dr_flac will simply skip over invalid frames as if they
    never existed. Frames are checked against their sync code, the CRC-8 of the frame header and the CRC-16 of the whole frame.

v0.5 - 2017-07-16
  - Fix typos.
  - Change drflac_bool* types to unsigned.
  - Add CRC checking. This makes dr_flac slower, but can be disabled with #define DR_FLAC_NO_CRC.

v0.4f - 2017-03-10
  - Fix a couple of bugs with the bitstreaming code.

v0.4e - 2017-02-17
  - Fix some warnings.

v0.4d - 2016-12-26
  - Add support for 32-bit floating-point PCM decoding.
  - Use drflac_int* and drflac_uint* sized types to improve compiler support.
  - Minor improvements to documentation.

v0.4c - 2016-12-26
  - Add support for signed 16-bit integer PCM decoding.

v0.4b - 2016-10-23
  - A minor change to drflac_bool8 and drflac_bool32 types.

v0.4a - 2016-10-11
  - Rename drBool32 to drflac_bool32 for styling consistency.

v0.4 - 2016-09-29
  - API/ABI CHANGE: Use fixed size 32-bit booleans instead of the built-in bool type.
  - API CHANGE: Rename drflac_open_and_decode*() to drflac_open_and_decode*_s32().
  - API CHANGE: Swap the order of "channels" and "sampleRate" parameters in drflac_open_and_decode*(). Rationale for this is to
    keep it consistent with drflac_audio.

v0.3f - 2016-09-21
  - Fix a warning with GCC.

v0.3e - 2016-09-18
  - Fixed a bug where GCC 4.3+ was not getting properly identified.
  - Fixed a few typos.
  - Changed date formats to ISO 8601 (YYYY-MM-DD).

v0.3d - 2016-06-11
  - Minor clean up.

v0.3c - 2016-05-28
  - Fixed compilation error.

v0.3b - 2016-05-16
  - Fixed Linux/GCC build.
  - Updated documentation.

v0.3a - 2016-05-15
  - Minor fixes to documentation.

v0.3 - 2016-05-11
  - Optimizations. Now at about parity with the reference implementation on 32-bit builds.
  - Lots of clean up.

v0.2b - 2016-05-10
  - Bug fixes.

v0.2a - 2016-05-10
  - Made drflac_open_and_decode() more robust.
  - Removed an unused debugging variable

v0.2 - 2016-05-09
  - Added support for Ogg encapsulation.
  - API CHANGE. Have the onSeek callback take a third argument which specifies whether or not the seek
    should be relative to the start or the current position. Also changes the seeking rules such that
    seeking offsets will never be negative.
  - Have drflac_open_and_decode() fail gracefully if the stream has an unknown total sample count.

v0.1b - 2016-05-07
  - Properly close the file handle in drflac_open_file() and family when the decoder fails to initialize.
  - Removed a stale comment.

v0.1a - 2016-05-05
  - Minor formatting changes.
  - Fixed a warning on the GCC build.

v0.1 - 2016-05-03
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
