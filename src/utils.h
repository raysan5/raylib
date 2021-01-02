/**********************************************************************************************
*
*   raylib.utils - Some common utility functions
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2021 Ramon Santamaria (@raysan5)
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

#ifndef UTILS_H
#define UTILS_H

#if defined(PLATFORM_ANDROID)
    #include <stdio.h>                      // Required for: FILE
    #include <android/asset_manager.h>      // Required for: AAssetManager
#endif

#if defined(SUPPORT_TRACELOG)
    #define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)

    #if defined(SUPPORT_TRACELOG_DEBUG)
        #define TRACELOGD(...) TraceLog(LOG_DEBUG, __VA_ARGS__)
    #else
        #define TRACELOGD(...) (void)0
    #endif
#else
    #define TRACELOG(level, ...) (void)0
    #define TRACELOGD(...) (void)0
#endif

//----------------------------------------------------------------------------------
// Some basic Defines
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
    #define fopen(name, mode) android_fopen(name, mode)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// Nop...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if defined(PLATFORM_ANDROID)
void InitAssetManager(AAssetManager *manager, const char *dataPath);   // Initialize asset manager from android app
FILE *android_fopen(const char *fileName, const char *mode);            // Replacement for fopen() -> Read-only!
#endif

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
