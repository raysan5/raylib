/**********************************************************************************************
*
*   raylib - UIKit application entry point for iOS, iPadOS and tvOS
*
*   This software is provided under the zlib/libpng license. See LICENSE for details.
*
**********************************************************************************************/

#ifndef RAYLIB_APPLE_H
#define RAYLIB_APPLE_H

#include "raylib.h"

#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if (TARGET_OS_IOS || TARGET_OS_TV) && !TARGET_OS_MACCATALYST

#if defined(__cplusplus)
extern "C" {
#endif

// Start UIKit from main() and run the C game on its rendering thread
RLAPI int RaylibRunAppleApp(int argc, char **argv, int (*gameMain)(void));

#if defined(__cplusplus)
}
#endif

    #endif
#endif

#endif  // RAYLIB_APPLE_H
