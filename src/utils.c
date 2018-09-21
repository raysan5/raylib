/**********************************************************************************************
*
*   raylib.utils - Some common utility functions
*
*   CONFIGURATION:
*
*   #define SUPPORT_TRACELOG
*       Show TraceLog() output messages
*       NOTE: By default LOG_DEBUG traces not shown
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
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

#include "config.h"

#include "raylib.h"                 // WARNING: Required for: LogType enum
#include "utils.h"

#if defined(PLATFORM_ANDROID)
    #include <errno.h>
    #include <android/log.h>
    #include <android/asset_manager.h>
#endif

#include <stdlib.h>                 // Required for: malloc(), free()
#include <stdio.h>                  // Required for: fopen(), fclose(), fputc(), fwrite(), printf(), fprintf(), funopen()
#include <stdarg.h>                 // Required for: va_list, va_start(), vfprintf(), va_end()
#include <string.h>                 // Required for: strlen(), strrchr(), strcmp()

/* This should be in <stdio.h>, but Travis doesn't find it... */
FILE *funopen(const void *cookie, int (*readfn)(void *, char *, int),
              int (*writefn)(void *, const char *, int),
              fpos_t (*seekfn)(void *, fpos_t, int), int (*closefn)(void *));

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// Log types messages supported flags (bit based)
static unsigned char logTypeFlags = LOG_INFO | LOG_WARNING | LOG_ERROR;
static TraceLogCallback logCallback = NULL;

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

// Enable trace log message types (bit flags based)
void SetTraceLog(unsigned char types)
{
    logTypeFlags = types;
}

// Set a trace log callback to enable custom logging bypassing raylib's one
void SetTraceLogCallback(TraceLogCallback callback)
{
    logCallback = callback;
}

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int msgType, const char *text, ...)
{
#if defined(SUPPORT_TRACELOG)
    static char buffer[128];
    va_list args;
    va_start(args, text);

    if (logCallback)
    {
        logCallback(msgType, text, args);
        va_end(args);
        return;
    }

    switch(msgType)
    {
        case LOG_INFO: strcpy(buffer, "INFO: "); break;
        case LOG_ERROR: strcpy(buffer, "ERROR: "); break;
        case LOG_WARNING: strcpy(buffer, "WARNING: "); break;
        case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        default: break;
    }

    strcat(buffer, text);
    strcat(buffer, "\n");

#if defined(PLATFORM_ANDROID)
    switch(msgType)
    {
        case LOG_INFO: if (logTypeFlags & LOG_INFO) __android_log_vprint(ANDROID_LOG_INFO, "raylib", buffer, args); break;
        case LOG_WARNING: if (logTypeFlags & LOG_WARNING) __android_log_vprint(ANDROID_LOG_WARN, "raylib", buffer, args); break;
        case LOG_ERROR: if (logTypeFlags & LOG_ERROR) __android_log_vprint(ANDROID_LOG_ERROR, "raylib", buffer, args); break;
        case LOG_DEBUG: if (logTypeFlags & LOG_DEBUG) __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", buffer, args); break;
        default: break;
    }
#else
    switch(msgType)
    {
        case LOG_INFO: if (logTypeFlags & LOG_INFO) vprintf(buffer, args); break;
        case LOG_WARNING: if (logTypeFlags & LOG_WARNING) vprintf(buffer, args); break;
        case LOG_ERROR: if (logTypeFlags & LOG_ERROR) vprintf(buffer, args); break;
        case LOG_DEBUG: if (logTypeFlags & LOG_DEBUG) vprintf(buffer, args); break;
        default: break;
    }
#endif

    va_end(args);

    if (msgType == LOG_ERROR) exit(1);  // If LOG_ERROR message, exit program

#endif  // SUPPORT_TRACELOG
}

// Keep track of memory allocated
// NOTE: mallocType defines the type of data allocated
/*
void RecordMalloc(int mallocType, int mallocSize, const char *msg)
{
    // TODO: Investigate how to record memory allocation data...
    // Maybe creating my own malloc function...
}
*/

#if defined(PLATFORM_ANDROID)
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
    TraceLog(LOG_ERROR, "Can't provide write access to the APK");

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
