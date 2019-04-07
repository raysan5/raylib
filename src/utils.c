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
*   Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
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

#include "raylib.h"                     // WARNING: Required for: LogType enum

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"         // Defines module configuration flags
#endif

#include "utils.h"

#if defined(PLATFORM_ANDROID)
    #include <errno.h>                  // Required for: Android error types
    #include <android/log.h>            // Required for: Android log system: __android_log_vprint()
    #include <android/asset_manager.h>  // Required for: Android assets manager: AAsset, AAssetManager_open(), ...
#endif

#include <stdlib.h>                     // Required for: malloc(), free()
#include <stdio.h>                      // Required for: fopen(), fclose(), fputc(), fwrite(), printf(), fprintf(), funopen()
#include <stdarg.h>                     // Required for: va_list, va_start(), vfprintf(), va_end()
#include <string.h>                     // Required for: strlen(), strrchr(), strcmp()

#define MAX_TRACELOG_BUFFER_SIZE   128  // Max length of one trace-log message

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

// Log types messages
static int logTypeLevel = LOG_INFO;
static int logTypeExit = LOG_ERROR;
static TraceLogCallback logCallback = NULL;

#if defined(PLATFORM_ANDROID)
AAssetManager *assetManager;
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
/* This should be in <stdio.h>, but Travis doesn't find it... */
FILE *funopen(const void *cookie, int (*readfn)(void *, char *, int), int (*writefn)(void *, const char *, int),
              fpos_t (*seekfn)(void *, fpos_t, int), int (*closefn)(void *));

static int android_read(void *cookie, char *buf, int size);
static int android_write(void *cookie, const char *buf, int size);
static fpos_t android_seek(void *cookie, fpos_t offset, int whence);
static int android_close(void *cookie);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Utilities
//----------------------------------------------------------------------------------

// Set the current threshold (minimum) log level
void SetTraceLogLevel(int logType)
{
    logTypeLevel = logType;
}

// Set the exit threshold (minimum) log level
void SetTraceLogExit(int logType)
{
    logTypeExit = logType;
}

// Set a trace log callback to enable custom logging
void SetTraceLogCallback(TraceLogCallback callback)
{
    logCallback = callback;
}

// Show trace log messages (LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG)
void TraceLog(int logType, const char *text, ...)
{
#if defined(SUPPORT_TRACELOG)
    // Message has level below current threshold, don't emit
    if (logType < logTypeLevel) return;

    va_list args;
    va_start(args, text);

    if (logCallback)
    {
        logCallback(logType, text, args);
        va_end(args);
        return;
    }

#if defined(PLATFORM_ANDROID)
    switch(logType)
    {
        case LOG_TRACE: __android_log_vprint(ANDROID_LOG_VERBOSE, "raylib", text, args); break;
        case LOG_DEBUG: __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", text, args); break;
        case LOG_INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", text, args); break;
        case LOG_WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", text, args); break;
        case LOG_ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", text, args); break;
        case LOG_FATAL: __android_log_vprint(ANDROID_LOG_FATAL, "raylib", text, args); break;
        default: break;
    }
#else
    char buffer[MAX_TRACELOG_BUFFER_SIZE] = { 0 };

    switch(logType)
    {
        case LOG_TRACE: strcpy(buffer, "TRACE: "); break;
        case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        case LOG_INFO: strcpy(buffer, "INFO: "); break;
        case LOG_WARNING: strcpy(buffer, "WARNING: "); break;
        case LOG_ERROR: strcpy(buffer, "ERROR: "); break;
        case LOG_FATAL: strcpy(buffer, "FATAL: "); break;
        default: break;
    }

    strcat(buffer, text);
    strcat(buffer, "\n");
    vprintf(buffer, args);
#endif

    va_end(args);

    if (logType >= logTypeExit) exit(1);  // If exit message, exit program

#endif  // SUPPORT_TRACELOG
}

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
