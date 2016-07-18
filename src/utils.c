/**********************************************************************************************
*
*   raylib.utils
*
*   Utils Functions Definitions
*
*   Uses external libs:
*       tinfl - zlib DEFLATE algorithm decompression lib
*       stb_image_write - PNG writting functions
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

#include "utils.h"

#if defined(PLATFORM_ANDROID)
    #include <errno.h>
    #include <android/log.h>
    #include <android/asset_manager.h>
#endif

#include <stdlib.h>                 // Required for: malloc(), free()
#include <stdio.h>                  // Required for: fopen(), fclose(), fputc(), fwrite(), printf(), fprintf(), funopen()
#include <stdarg.h>                 // Required for: va_list, va_start(), vfprintf(), va_end()
//#include <string.h>               // Required for: strlen(), strrchr(), strcmp()

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
    #define STB_IMAGE_WRITE_IMPLEMENTATION
    #include "external/stb_image_write.h"    // Required for: stbi_write_png()
#endif

#include "external/tinfl.c"         // Required for: tinfl_decompress_mem_to_mem()
                                    // NOTE: Deflate algorythm data decompression

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
AAssetManager *assetManager;
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
static int android_read(void *cookie, char *buf, int size);
static int android_write(void *cookie, const char *buf, int size);
static fpos_t android_seek(void *cookie, fpos_t offset, int whence);
static int android_close(void *cookie);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Utilities
//----------------------------------------------------------------------------------

// Data decompression function
// NOTE: Allocated data MUST be freed!
unsigned char *DecompressData(const unsigned char *data, unsigned long compSize, int uncompSize)
{
    int tempUncompSize;
    unsigned char *pUncomp;

    // Allocate buffer to hold decompressed data
    pUncomp = (mz_uint8 *)malloc((size_t)uncompSize);

    // Check correct memory allocation
    if (pUncomp == NULL)
    {
        TraceLog(WARNING, "Out of memory while decompressing data");
    }
    else
    {
        // Decompress data
        tempUncompSize = tinfl_decompress_mem_to_mem(pUncomp, (size_t)uncompSize, data, compSize, 1);

        if (tempUncompSize == -1)
        {
            TraceLog(WARNING, "Data decompression failed");
            free(pUncomp);
        }

        if (uncompSize != (int)tempUncompSize)
        {
            TraceLog(WARNING, "Expected uncompressed size do not match, data may be corrupted");
            TraceLog(WARNING, " -- Expected uncompressed size: %i", uncompSize);
            TraceLog(WARNING, " -- Returned uncompressed size: %i", tempUncompSize);
        }

        TraceLog(INFO, "Data decompressed successfully from %u bytes to %u bytes", (mz_uint32)compSize, (mz_uint32)tempUncompSize);
    }

    return pUncomp;
}

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI)
// Creates a bitmap (BMP) file from an array of pixel data
// NOTE: This function is not explicitly available to raylib users
void WriteBitmap(const char *fileName, unsigned char *imgData, int width, int height)
{
    int filesize = 54 + 3*width*height;

    unsigned char bmpFileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};    // Standard BMP file header
    unsigned char bmpInfoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};   // Standard BMP info header

    bmpFileHeader[2] = (unsigned char)(filesize);
    bmpFileHeader[3] = (unsigned char)(filesize>>8);
    bmpFileHeader[4] = (unsigned char)(filesize>>16);
    bmpFileHeader[5] = (unsigned char)(filesize>>24);

    bmpInfoHeader[4] = (unsigned char)(width);
    bmpInfoHeader[5] = (unsigned char)(width>>8);
    bmpInfoHeader[6] = (unsigned char)(width>>16);
    bmpInfoHeader[7] = (unsigned char)(width>>24);
    bmpInfoHeader[8] = (unsigned char)(height);
    bmpInfoHeader[9] = (unsigned char)(height>>8);
    bmpInfoHeader[10] = (unsigned char)(height>>16);
    bmpInfoHeader[11] = (unsigned char)(height>>24);

    FILE *bmpFile = fopen(fileName, "wb");    // Define a pointer to bitmap file and open it in write-binary mode

    if (bmpFile == NULL)
    {
        TraceLog(WARNING, "[%s] BMP file could not be created", fileName);
    }
    else
    {
        // NOTE: fwrite parameters are: data pointer, size in bytes of each element to be written, number of elements, file-to-write pointer
        fwrite(bmpFileHeader, sizeof(unsigned char), 14, bmpFile);    // Write BMP file header data
        fwrite(bmpInfoHeader, sizeof(unsigned char), 40, bmpFile);    // Write BMP info header data

        // Write pixel data to file
        for (int y = 0; y < height ; y++)
        {
            for (int x = 0; x < width; x++)
            {
                fputc(imgData[(x*4)+2 + (y*width*4)], bmpFile);
                fputc(imgData[(x*4)+1 + (y*width*4)], bmpFile);
                fputc(imgData[(x*4) + (y*width*4)], bmpFile);
            }
        }
    }

    fclose(bmpFile);        // Close bitmap file
}

// Creates a PNG image file from an array of pixel data
// NOTE: Uses stb_image_write
void WritePNG(const char *fileName, unsigned char *imgData, int width, int height, int compSize)
{
    stbi_write_png(fileName, width, height, compSize, imgData, width*compSize);
}
#endif

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_RPI) || defined(PLATFORM_WEB)
// Outputs a trace log message (INFO, ERROR, WARNING)
// NOTE: If a file has been init, output log is written there
void TraceLog(int msgType, const char *text, ...)
{
    va_list args;
    int traceDebugMsgs = 1;

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

#if defined(PLATFORM_ANDROID)
void TraceLog(int msgType, const char *text, ...)
{
    static char buffer[100];
    int traceDebugMsgs = 1;
    
#ifdef DO_NOT_TRACE_DEBUG_MSGS
    traceDebugMsgs = 0;
#endif

    switch(msgType)
    {
        case INFO: strcpy(buffer, "INFO: "); break;
        case ERROR: strcpy(buffer, "ERROR: "); break;
        case WARNING: strcpy(buffer, "WARNING: "); break;
        case DEBUG: strcpy(buffer, "DEBUG: "); break;
        default: break;
    }

    strcat(buffer, text);
    strcat(buffer, "\n");

    va_list args;
    va_start(args, buffer);

    switch(msgType)
    {
        case INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", buffer, args); break;
        case ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", buffer, args); break;
        case WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", buffer, args); break;
        case DEBUG: if (traceDebugMsgs) __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", buffer, args); break;
        default: break;
    }

    va_end(args);

    if (msgType == ERROR) exit(1);
}

// Initialize asset manager from android app
void InitAssetManager(AAssetManager *manager)
{
    assetManager = manager;
}

// Replacement for fopen
FILE *android_fopen(const char *fileName, const char *mode)
{
    if (mode[0] == 'w') return NULL;

    AAsset *asset = AAssetManager_open(assetManager, fileName, 0);

    if (!asset) return NULL;

    return funopen(asset, android_read, android_write, android_seek, android_close);
}
#endif

// Keep track of memory allocated
// NOTE: mallocType defines the type of data allocated
/*
void RecordMalloc(int mallocType, int mallocSize, const char *msg)
{
    // TODO: Investigate how to record memory allocation data...
    // Maybe creating my own malloc function...
}
*/

// Get the extension for a filename
const char *GetExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');
    if (!dot || dot == fileName) return "";
    return (dot + 1);
}

// Calculate next power-of-two value for a given num
int GetNextPOT(int num)
{
    if (num != 0)
    {
        num--;
        num |= (num >> 1);     // Or first 2 bits
        num |= (num >> 2);     // Or next 2 bits
        num |= (num >> 4);     // Or next 4 bits
        num |= (num >> 8);     // Or next 8 bits
        num |= (num >> 16);    // Or next 16 bits
        num++;
    }

    return num;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
static int android_read(void *cookie, char *buf, int size)
{
    return AAsset_read((AAsset *)cookie, buf, size);
}

static int android_write(void *cookie, const char *buf, int size)
{
    TraceLog(ERROR, "Can't provide write access to the APK");

    return EACCES;
}

static fpos_t android_seek(void *cookie, fpos_t offset, int whence)
{
    return AAsset_seek((AAsset *)cookie, offset, whence);
}

static int android_close(void *cookie)
{
    AAsset_close((AAsset *)cookie);
    return 0;
}
#endif
