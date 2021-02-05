/*
HOW TO USE:

    In exactly one translation unit (.c or .cpp file), #define MSF_GIF_IMPL before including the header, like so:

    #define MSF_GIF_IMPL
    #include "msf_gif.h"

    Everywhere else, just include the header like normal.


USAGE EXAMPLE:

    int width = 480, height = 320, centisecondsPerFrame = 5, bitDepth = 16;
    MsfGifState gifState = {};
    msf_gif_begin(&gifState, width, height);
    msf_gif_frame(&gifState, ..., centisecondsPerFrame, bitDepth, width * 4); //frame 1
    msf_gif_frame(&gifState, ..., centisecondsPerFrame, bitDepth, width * 4); //frame 2
    msf_gif_frame(&gifState, ..., centisecondsPerFrame, bitDepth, width * 4); //frame 3, etc...
    MsfGifResult result = msf_gif_end(&gifState);
    FILE * fp = fopen("MyGif.gif", "wb");
    fwrite(result.data, result.dataSize, 1, fp);
    fclose(fp);
    msf_gif_free(result);

Detailed function documentation can be found in the header section below.


REPLACING MALLOC:

    This library uses malloc+realloc+free internally for memory allocation.
    To facilitate integration with custom memory allocators, these calls go through macros, which can be redefined.
    The expected function signature equivalents of the macros are as follows:

    void * MSF_GIF_MALLOC(void * context, size_t newSize)
    void * MSF_GIF_REALLOC(void * context, void * oldMemory, size_t oldSize, size_t newSize)
    void MSF_GIF_FREE(void * context, void * oldMemory, size_t oldSize)

    If your allocator needs a context pointer, you can set the `customAllocatorContext` field of the MsfGifState struct
    before calling msf_gif_begin(), and it will be passed to all subsequent allocator macro calls.

See end of file for license information.
*/

//version 2.1

#ifndef MSF_GIF_H
#define MSF_GIF_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    void * data;
    size_t dataSize;

    size_t allocSize; //internal use
    void * contextPointer; //internal use
} MsfGifResult;

typedef struct { //internal use
    uint32_t * pixels;
    int depth, count, rbits, gbits, bbits;
} MsfCookedFrame;

typedef struct {
    MsfCookedFrame previousFrame;
    uint8_t * listHead;
    uint8_t * listTail;
    int width, height;
    void * customAllocatorContext;
} MsfGifState;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/**
 * @param width                Image width in pixels.
 * @param height               Image height in pixels.
 * @return                     Non-zero on success, 0 on error.
 */
int msf_gif_begin(MsfGifState * handle, int width, int height);

/**
 * @param pixelData            Pointer to raw framebuffer data. Rows must be contiguous in memory, in RGBA8 format.
 *                             Note: This function does NOT free `pixelData`. You must free it yourself afterwards.
 * @param centiSecondsPerFrame How many hundredths of a second this frame should be displayed for.
 *                             Note: This being specified in centiseconds is a limitation of the GIF format.
 * @param maxBitDepth          Limits how many bits per pixel can be used when quantizing the gif.
 *                             The actual bit depth chosen for a given frame will be less than or equal to
 *                             the supplied maximum, depending on the variety of colors used in the frame.
 *                             `maxBitDepth` will be clamped between 1 and 16. The recommended default is 16.
 *                             Lowering this value can result in faster exports and smaller gifs,
 *                             but the quality may suffer.
 *                             Please experiment with this value to find what works best for your application.
 * @param pitchInBytes         The number of bytes from the beginning of one row of pixels to the beginning of the next.
 *                             If you want to flip the image, just pass in a negative pitch.
 * @return                     Non-zero on success, 0 on error.
 */
int msf_gif_frame(MsfGifState * handle, uint8_t * pixelData, int centiSecondsPerFame, int maxBitDepth, int pitchInBytes);

/**
 * @return                     A block of memory containing the gif file data, or NULL on error.
 *                             You are responsible for freeing this via `msf_gif_free()`.
 */
MsfGifResult msf_gif_end(MsfGifState * handle);

/**
 * @param result                The MsfGifResult struct, verbatim as it was returned from `msf_gif_end()`.
 */
void msf_gif_free(MsfGifResult result);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //MSF_GIF_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// IMPLEMENTATION                                                                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef MSF_GIF_IMPL
#ifndef MSF_GIF_ALREADY_IMPLEMENTED_IN_THIS_TRANSLATION_UNIT
#define MSF_GIF_ALREADY_IMPLEMENTED_IN_THIS_TRANSLATION_UNIT

#ifndef MSF_GIF_BUFFER_INIT_SIZE
#define MSF_GIF_BUFFER_INIT_SIZE 1024 * 1024 * 4 //4MB by default, you can increase this if you want to realloc less
#endif

//ensure the library user has either defined all of malloc/realloc/free, or none
#if defined(MSF_GIF_MALLOC) && defined(MSF_GIF_REALLOC) && defined(MSF_GIF_FREE) //ok
#elif !defined(MSF_GIF_MALLOC) && !defined(MSF_GIF_REALLOC) && !defined(MSF_GIF_FREE) //ok
#else
#error "You must either define all of MSF_GIF_MALLOC, MSF_GIF_REALLOC, and MSF_GIF_FREE, or define none of them"
#endif

//provide default allocator definitions that redirect to the standard global allocator
#if !defined(MSF_GIF_MALLOC)
#include <stdlib.h> //malloc, etc.
#define MSF_GIF_MALLOC(contextPointer, newSize) malloc(newSize)
#define MSF_GIF_REALLOC(contextPointer, oldMemory, oldSize, newSize) realloc(oldMemory, newSize)
#define MSF_GIF_FREE(contextPointer, oldMemory, oldSize) free(oldMemory)
#endif

//instrumentation for capturing profiling traces (useless for the library user, but useful for the library author)
#ifdef MSF_GIF_ENABLE_TRACING
#define MsfTimeFunc TimeFunc
#define MsfTimeLoop TimeLoop
#define msf_init_profiling_thread init_profiling_thread
#else
#define MsfTimeFunc
#define MsfTimeLoop(name)
#define msf_init_profiling_thread()
#endif //MSF_GIF_ENABLE_TRACING

#include <string.h> //memcpy

//TODO: use compiler-specific notation to force-inline functions currently marked inline
#if defined(__GNUC__) //gcc, clang
static inline int msf_bit_log(int i) { return 32 - __builtin_clz(i); }
#elif defined(_MSC_VER) //msvc
#include <intrin.h>
static inline int msf_bit_log(int i) { unsigned long idx; _BitScanReverse(&idx, i); return idx + 1; }
#else //fallback implementation for other compilers
//from https://stackoverflow.com/a/31718095/3064745 - thanks!
static inline int msf_bit_log(int i) {
    static const int MultiplyDeBruijnBitPosition[32] = {
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31,
    };
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    return MultiplyDeBruijnBitPosition[(uint32_t)(i * 0x07C4ACDDU) >> 27] + 1;
}
#endif
static inline int msf_imin(int a, int b) { return a < b? a : b; }
static inline int msf_imax(int a, int b) { return b < a? a : b; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Frame Cooking                                                                                                    ///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (defined (__SSE2__) || defined (_M_X64) || _M_IX86_FP == 2) && !defined(MSF_GIF_NO_SSE2)
#include <emmintrin.h>
#endif

static MsfCookedFrame msf_cook_frame(void * allocContext, uint8_t * raw, uint8_t * used,
                                     int width, int height, int pitch, int depth)
{ MsfTimeFunc
    //bit depth for each channel
    const static int rdepths[17] = { 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5 };
    const static int gdepths[17] = { 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6 };
    const static int bdepths[17] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5 };

    const static int ditherKernel[16] = {
         0 << 12,  8 << 12,  2 << 12, 10 << 12,
        12 << 12,  4 << 12, 14 << 12,  6 << 12,
         3 << 12, 11 << 12,  1 << 12,  9 << 12,
        15 << 12,  7 << 12, 13 << 12,  5 << 12,
    };

    uint32_t * cooked = (uint32_t *) MSF_GIF_MALLOC(allocContext, width * height * sizeof(uint32_t));
    if (!cooked) { MsfCookedFrame blank = {0}; return blank; }

    int count = 0;
    MsfTimeLoop("do") do {
        int rbits = rdepths[depth], gbits = gdepths[depth], bbits = bdepths[depth];
        int paletteSize = 1 << (rbits + gbits + bbits);
        memset(used, 0, paletteSize * sizeof(uint8_t));

        //TODO: document what this math does and why it's correct
        int rdiff = (1 << (8 - rbits)) - 1;
        int gdiff = (1 << (8 - gbits)) - 1;
        int bdiff = (1 << (8 - bbits)) - 1;
        short rmul = (short) ((255.0f - rdiff) / 255.0f * 257);
        short gmul = (short) ((255.0f - gdiff) / 255.0f * 257);
        short bmul = (short) ((255.0f - bdiff) / 255.0f * 257);

        int gmask = ((1 << gbits) - 1) << rbits;
        int bmask = ((1 << bbits) - 1) << rbits << gbits;

        MsfTimeLoop("cook") for (int y = 0; y < height; ++y) {
            int x = 0;

            #if (defined (__SSE2__) || defined (_M_X64) || _M_IX86_FP == 2) && !defined(MSF_GIF_NO_SSE2)
                __m128i k = _mm_loadu_si128((__m128i *) &ditherKernel[(y & 3) * 4]);
                __m128i k2 = _mm_or_si128(_mm_srli_epi32(k, rbits), _mm_slli_epi32(_mm_srli_epi32(k, bbits), 16));
                // MsfTimeLoop("SIMD")
                for (; x < width - 3; x += 4) {
                    uint8_t * pixels = &raw[y * pitch + x * 4];
                    __m128i p = _mm_loadu_si128((__m128i *) pixels);

                    __m128i rb = _mm_and_si128(p, _mm_set1_epi32(0x00FF00FF));
                    __m128i rb1 = _mm_mullo_epi16(rb, _mm_set_epi16(bmul, rmul, bmul, rmul, bmul, rmul, bmul, rmul));
                    __m128i rb2 = _mm_adds_epu16(rb1, k2);
                    __m128i r3 = _mm_srli_epi32(_mm_and_si128(rb2, _mm_set1_epi32(0x0000FFFF)), 16 - rbits);
                    __m128i b3 = _mm_and_si128(_mm_srli_epi32(rb2, 32 - rbits - gbits - bbits), _mm_set1_epi32(bmask));

                    __m128i g = _mm_and_si128(_mm_srli_epi32(p, 8), _mm_set1_epi32(0x000000FF));
                    __m128i g1 = _mm_mullo_epi16(g, _mm_set1_epi32(gmul));
                    __m128i g2 = _mm_adds_epu16(g1, _mm_srli_epi32(k, gbits));
                    __m128i g3 = _mm_and_si128(_mm_srli_epi32(g2, 16 - rbits - gbits), _mm_set1_epi32(gmask));

                    //TODO: does storing this as a __m128i then reading it back as a uint32_t violate strict aliasing?
                    uint32_t * c = &cooked[y * width + x];
                    __m128i out = _mm_or_si128(_mm_or_si128(r3, g3), b3);
                    _mm_storeu_si128((__m128i *) c, out);
                }
            #endif

            //scalar cleanup loop
            // MsfTimeLoop("scalar")
            for (; x < width; ++x) {
                uint8_t * p = &raw[y * pitch + x * 4];
                int dx = x & 3, dy = y & 3;
                int k = ditherKernel[dy * 4 + dx];
                cooked[y * width + x] =
                    (msf_imin(65535, p[2] * bmul + (k >> bbits)) >> (16 - rbits - gbits - bbits) & bmask) |
                    (msf_imin(65535, p[1] * gmul + (k >> gbits)) >> (16 - rbits - gbits        ) & gmask) |
                     msf_imin(65535, p[0] * rmul + (k >> rbits)) >> (16 - rbits                );
            }
        }

        count = 0;
        MsfTimeLoop("mark and count") for (int i = 0; i < width * height; ++i) {
            used[cooked[i]] = 1;
        }

        //count used colors
        MsfTimeLoop("count") for (int j = 0; j < paletteSize; ++j) {
            count += used[j];
        }
    } while (count >= 256 && --depth);

    MsfCookedFrame ret = { cooked, depth, count, rdepths[depth], gdepths[depth], bdepths[depth] };
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Frame Compression                                                                                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t * next;
    size_t size;
} MsfBufferHeader;

static inline int msf_put_code(uint8_t * * writeHead, uint32_t * blockBits, int len, uint32_t code) {
    //insert new code into block buffer
    int idx = *blockBits / 8;
    int bit = *blockBits % 8;
    (*writeHead)[idx + 0] |= code <<       bit ;
    (*writeHead)[idx + 1] |= code >> ( 8 - bit);
    (*writeHead)[idx + 2] |= code >> (16 - bit);
    *blockBits += len;

    //prep the next block buffer if the current one is full
    if (*blockBits >= 256 * 8) {
        *blockBits -= 255 * 8;
        (*writeHead) += 256;
        (*writeHead)[2] = (*writeHead)[1];
        (*writeHead)[1] = (*writeHead)[0];
        (*writeHead)[0] = 255;
        memset((*writeHead) + 4, 0, 256);
    }

    return 1;
}

typedef struct {
    int16_t * data;
    int len;
    int stride;
} MsfStridedList;

static inline void msf_lzw_reset(MsfStridedList * lzw, int tableSize, int stride) { MsfTimeFunc
    memset(lzw->data, 0xFF, 4096 * stride * sizeof(int16_t));
    lzw->len = tableSize + 2;
    lzw->stride = stride;
}

static uint8_t * msf_compress_frame(void * allocContext, int width, int height, int centiSeconds,
                                    MsfCookedFrame frame, MsfCookedFrame previous, uint8_t * used)
{ MsfTimeFunc
    //NOTE: we reserve enough memory for theoretical the worst case upfront because it's a reasonable amount,
    //      and prevents us from ever having to check size or realloc during compression
    int maxBufSize = sizeof(MsfBufferHeader) + 32 + 256 * 3 + width * height * 3 / 2; //headers + color table + data
    uint8_t * allocation = (uint8_t *) MSF_GIF_MALLOC(allocContext, maxBufSize);
    if (!allocation) { return NULL; }
    uint8_t * writeBase = allocation + sizeof(MsfBufferHeader);
    uint8_t * writeHead = writeBase;
    int lzwAllocSize = 4096 * (frame.count + 1) * sizeof(int16_t);
    MsfStridedList lzw = { (int16_t *) MSF_GIF_MALLOC(allocContext, lzwAllocSize) };
    if (!lzw.data) { MSF_GIF_FREE(allocContext, allocation, maxBufSize); return NULL; }

    //allocate tlb
    int totalBits = frame.rbits + frame.gbits + frame.bbits;
    int tlbSize = 1 << totalBits;
    uint8_t tlb[1 << 16]; //only 64k, so stack allocating is fine

    //generate palette
    typedef struct { uint8_t r, g, b; } Color3;
    Color3 table[256] = { {0} };
    int tableIdx = 1; //we start counting at 1 because 0 is the transparent color
    MsfTimeLoop("table") for (int i = 0; i < tlbSize; ++i) {
        if (used[i]) {
            tlb[i] = tableIdx;
            int rmask = (1 << frame.rbits) - 1;
            int gmask = (1 << frame.gbits) - 1;
            //isolate components
            int r = i & rmask;
            int g = i >> frame.rbits & gmask;
            int b = i >> (frame.rbits + frame.gbits);
            //shift into highest bits
            r <<= 8 - frame.rbits;
            g <<= 8 - frame.gbits;
            b <<= 8 - frame.bbits;
            table[tableIdx].r = r | r >> frame.rbits | r >> (frame.rbits * 2) | r >> (frame.rbits * 3);
            table[tableIdx].g = g | g >> frame.gbits | g >> (frame.gbits * 2) | g >> (frame.gbits * 3);
            table[tableIdx].b = b | b >> frame.bbits | b >> (frame.bbits * 2) | b >> (frame.bbits * 3);
            ++tableIdx;
        }
    }

    //SPEC: "Because of some algorithmic constraints however, black & white images which have one color bit
    //       must be indicated as having a code size of 2."
    int tableBits = msf_imax(2, msf_bit_log(tableIdx - 1));
    int tableSize = 1 << tableBits;
    //NOTE: we don't just compare `depth` field here because it will be wrong for the first frame and we will segfault
    int hasSamePal = frame.rbits == previous.rbits && frame.gbits == previous.gbits && frame.bbits == previous.bbits;

    //NOTE: because __attribute__((__packed__)) is annoyingly compiler-specific, we do this unreadable weirdness
    char headerBytes[19] = "\x21\xF9\x04\x05\0\0\0\0" "\x2C\0\0\0\0\0\0\0\0\x80";
    memcpy(&headerBytes[4], &centiSeconds, 2);
    memcpy(&headerBytes[13], &width, 2);
    memcpy(&headerBytes[15], &height, 2);
    headerBytes[17] |= tableBits - 1;
    memcpy(writeHead, headerBytes, 18);
    writeHead += 18;

    //local color table
    memcpy(writeHead, table, tableSize * sizeof(Color3));
    writeHead += tableSize * sizeof(Color3);
    *writeHead++ = tableBits;

    //prep block
    memset(writeHead, 0, 260);
    writeHead[0] = 255;
    uint32_t blockBits = 8; //relative to block.head

    //SPEC: "Encoders should output a Clear code as the first code of each image data stream."
    msf_lzw_reset(&lzw, tableSize, tableIdx);
    msf_put_code(&writeHead, &blockBits, msf_bit_log(lzw.len - 1), tableSize);

    int lastCode = hasSamePal && frame.pixels[0] == previous.pixels[0]? 0 : tlb[frame.pixels[0]];
    MsfTimeLoop("compress") for (int i = 1; i < width * height; ++i) {
        //PERF: branching vs. branchless version of this line is observed to have no discernable impact on speed
        int color = hasSamePal && frame.pixels[i] == previous.pixels[i]? 0 : tlb[frame.pixels[i]];
        //PERF: branchless version must use && otherwise it will segfault on frame 1, but it's well-predicted so OK
        // int color = (!(hasSamePal && frame.pixels[i] == previous.pixels[i])) * tlb[frame.pixels[i]];
        int code = (&lzw.data[lastCode * lzw.stride])[color];
        if (code < 0) {
            //write to code stream
            int codeBits = msf_bit_log(lzw.len - 1);
            msf_put_code(&writeHead, &blockBits, codeBits, lastCode);

            if (lzw.len > 4095) {
                //reset buffer code table
                msf_put_code(&writeHead, &blockBits, codeBits, tableSize);
                msf_lzw_reset(&lzw, tableSize, tableIdx);
            } else {
                (&lzw.data[lastCode * lzw.stride])[color] = lzw.len;
                ++lzw.len;
            }

            lastCode = color;
        } else {
            lastCode = code;
        }
    }

    MSF_GIF_FREE(allocContext, lzw.data, lzwAllocSize);
    MSF_GIF_FREE(allocContext, previous.pixels, width * height * sizeof(uint32_t));

    //write code for leftover index buffer contents, then the end code
    msf_put_code(&writeHead, &blockBits, msf_imin(12, msf_bit_log(lzw.len - 1)), lastCode);
    msf_put_code(&writeHead, &blockBits, msf_imin(12, msf_bit_log(lzw.len)), tableSize + 1);

    //flush remaining data
    if (blockBits > 8) {
        int bytes = (blockBits + 7) / 8; //round up
        writeHead[0] = bytes - 1;
        writeHead += bytes;
    }
    *writeHead++ = 0; //terminating block

    //filling in buffer header and shrink buffer to fit data
    MsfBufferHeader * header = (MsfBufferHeader *) allocation;
    header->next = NULL;
    header->size = writeHead - writeBase;
    uint8_t * moved = (uint8_t *) MSF_GIF_REALLOC(allocContext, allocation, maxBufSize, writeHead - allocation);
    if (!moved) { MSF_GIF_FREE(allocContext, allocation, maxBufSize); return NULL; }
    return moved;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Incremental API                                                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int msf_gif_begin(MsfGifState * handle, int width, int height) { MsfTimeFunc
    MsfCookedFrame empty = {0}; //god I hate MSVC...
    handle->previousFrame = empty;
    handle->width = width;
    handle->height = height;

    //setup header buffer header (lol)
    handle->listHead = (uint8_t *) MSF_GIF_MALLOC(handle->customAllocatorContext, sizeof(MsfBufferHeader) + 32);
    if (!handle->listHead) { return 0; }
    handle->listTail = handle->listHead;
    MsfBufferHeader * header = (MsfBufferHeader *) handle->listHead;
    header->next = NULL;
    header->size = 32;

    //NOTE: because __attribute__((__packed__)) is annoyingly compiler-specific, we do this unreadable weirdness
    char headerBytes[33] = "GIF89a\0\0\0\0\x10\0\0" "\x21\xFF\x0BNETSCAPE2.0\x03\x01\0\0\0";
    memcpy(&headerBytes[6], &width, 2);
    memcpy(&headerBytes[8], &height, 2);
    memcpy(handle->listHead + sizeof(MsfBufferHeader), headerBytes, 32);
    return 1;
}

int msf_gif_frame(MsfGifState * handle, uint8_t * pixelData, int centiSecondsPerFame, int maxBitDepth, int pitchInBytes)
{ MsfTimeFunc
    if (!handle->listHead) { return 0; }

    maxBitDepth = msf_imax(1, msf_imin(16, maxBitDepth));
    if (pitchInBytes == 0) pitchInBytes = handle->width * 4;
    if (pitchInBytes < 0) pixelData -= pitchInBytes * (handle->height - 1);

    uint8_t used[1 << 16]; //only 64k, so stack allocating is fine
    MsfCookedFrame frame =
        msf_cook_frame(handle->customAllocatorContext, pixelData, used, handle->width, handle->height, pitchInBytes,
            msf_imin(maxBitDepth, handle->previousFrame.depth + 160 / msf_imax(1, handle->previousFrame.count)));
    //TODO: de-duplicate cleanup code
    if (!frame.pixels) {
        MSF_GIF_FREE(handle->customAllocatorContext,
                     handle->previousFrame.pixels, handle->width * handle->height * sizeof(uint32_t));
        for (uint8_t * node = handle->listHead; node;) {
            MsfBufferHeader * header = (MsfBufferHeader *) node;
            node = header->next;
            MSF_GIF_FREE(handle->customAllocatorContext, header, sizeof(MsfBufferHeader) + header->size);
        }
        handle->listHead = handle->listTail = NULL;
        return 0;
    }

    uint8_t * buffer = msf_compress_frame(handle->customAllocatorContext,
        handle->width, handle->height, centiSecondsPerFame, frame, handle->previousFrame, used);
    ((MsfBufferHeader *) handle->listTail)->next = buffer;
    handle->listTail = buffer;
    if (!buffer) {
        MSF_GIF_FREE(handle->customAllocatorContext, frame.pixels, handle->width * handle->height * sizeof(uint32_t));
        MSF_GIF_FREE(handle->customAllocatorContext,
                     handle->previousFrame.pixels, handle->width * handle->height * sizeof(uint32_t));
        for (uint8_t * node = handle->listHead; node;) {
            MsfBufferHeader * header = (MsfBufferHeader *) node;
            node = header->next;
            MSF_GIF_FREE(handle->customAllocatorContext, header, sizeof(MsfBufferHeader) + header->size);
        }
        handle->listHead = handle->listTail = NULL;
        return 0;
    }

    handle->previousFrame = frame;
    return 1;
}

MsfGifResult msf_gif_end(MsfGifState * handle) { MsfTimeFunc
    if (!handle->listHead) { MsfGifResult empty = {0}; return empty; }

    MSF_GIF_FREE(handle->customAllocatorContext,
                 handle->previousFrame.pixels, handle->width * handle->height * sizeof(uint32_t));

    //first pass: determine total size
    size_t total = 1; //1 byte for trailing marker
    for (uint8_t * node = handle->listHead; node;) {
        MsfBufferHeader * header = (MsfBufferHeader *) node;
        node = header->next;
        total += header->size;
    }

    //second pass: write data
    uint8_t * buffer = (uint8_t *) MSF_GIF_MALLOC(handle->customAllocatorContext, total);
    if (buffer) {
        uint8_t * writeHead = buffer;
        for (uint8_t * node = handle->listHead; node;) {
            MsfBufferHeader * header = (MsfBufferHeader *) node;
            memcpy(writeHead, node + sizeof(MsfBufferHeader), header->size);
            writeHead += header->size;
            node = header->next;
        }
        *writeHead++ = 0x3B;
    }

    //third pass: free buffers
    for (uint8_t * node = handle->listHead; node;) {
        MsfBufferHeader * header = (MsfBufferHeader *) node;
        node = header->next;
        MSF_GIF_FREE(handle->customAllocatorContext, header, sizeof(MsfBufferHeader) + header->size);
    }

    MsfGifResult ret = { buffer, total, total, handle->customAllocatorContext };
    return ret;
}

void msf_gif_free(MsfGifResult result) {
    if (result.data) { MSF_GIF_FREE(result.contextPointer, result.data, result.allocSize); }
}

#endif //MSF_GIF_ALREADY_IMPLEMENTED_IN_THIS_TRANSLATION_UNIT
#endif //MSF_GIF_IMPL

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Miles Fogle
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
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
------------------------------------------------------------------------------
*/
