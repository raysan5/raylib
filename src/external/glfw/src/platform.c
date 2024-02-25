//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"

#include <string.h>
#include <stdlib.h>

// These construct a string literal from individual numeric constants
#define _GLFW_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define _GLFW_MAKE_VERSION(m, n, r) _GLFW_CONCAT_VERSION(m, n, r)

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

static const struct
{
    int ID;
    GLFWbool (*connect)(int,_GLFWplatform*);
} supportedPlatforms[] =
{
#if defined(_GLFW_WIN32)
    { GLFW_PLATFORM_WIN32, _glfwConnectWin32 },
#endif
#if defined(_GLFW_COCOA)
    { GLFW_PLATFORM_COCOA, _glfwConnectCocoa },
#endif
#if defined(_GLFW_WAYLAND)
    { GLFW_PLATFORM_WAYLAND, _glfwConnectWayland },
#endif
#if defined(_GLFW_X11)
    { GLFW_PLATFORM_X11, _glfwConnectX11 },
#endif
};

GLFWbool _glfwSelectPlatform(int desiredID, _GLFWplatform* platform)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (desiredID != GLFW_ANY_PLATFORM &&
        desiredID != GLFW_PLATFORM_WIN32 &&
        desiredID != GLFW_PLATFORM_COCOA &&
        desiredID != GLFW_PLATFORM_WAYLAND &&
        desiredID != GLFW_PLATFORM_X11 &&
        desiredID != GLFW_PLATFORM_NULL)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "Invalid platform ID 0x%08X", desiredID);
        return GLFW_FALSE;
    }

    // Only allow the Null platform if specifically requested
    if (desiredID == GLFW_PLATFORM_NULL)
        return _glfwConnectNull(desiredID, platform);
    else if (count == 0)
    {
        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "This binary only supports the Null platform");
        return GLFW_FALSE;
    }

#if defined(_GLFW_WAYLAND) && defined(_GLFW_X11)
    if (desiredID == GLFW_ANY_PLATFORM)
    {
        const char* const session = getenv("XDG_SESSION_TYPE");
        if (session)
        {
            // Only follow XDG_SESSION_TYPE if it is set correctly and the
            // environment looks plausble; otherwise fall back to detection
            if (strcmp(session, "wayland") == 0 && getenv("WAYLAND_DISPLAY"))
                desiredID = GLFW_PLATFORM_WAYLAND;
            else if (strcmp(session, "x11") == 0 && getenv("DISPLAY"))
                desiredID = GLFW_PLATFORM_X11;
        }
    }
#endif

    if (desiredID == GLFW_ANY_PLATFORM)
    {
        // If there is exactly one platform available for auto-selection, let it emit the
        // error on failure as the platform-specific error description may be more helpful
        if (count == 1)
            return supportedPlatforms[0].connect(supportedPlatforms[0].ID, platform);

        for (i = 0;  i < count;  i++)
        {
            if (supportedPlatforms[i].connect(desiredID, platform))
                return GLFW_TRUE;
        }

        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "Failed to detect any supported platform");
    }
    else
    {
        for (i = 0;  i < count;  i++)
        {
            if (supportedPlatforms[i].ID == desiredID)
                return supportedPlatforms[i].connect(desiredID, platform);
        }

        _glfwInputError(GLFW_PLATFORM_UNAVAILABLE, "The requested platform is not supported");
    }

    return GLFW_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                        GLFW public API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetPlatform(void)
{
    _GLFW_REQUIRE_INIT_OR_RETURN(0);
    return _glfw.platform.platformID;
}

GLFWAPI int glfwPlatformSupported(int platformID)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (platformID != GLFW_PLATFORM_WIN32 &&
        platformID != GLFW_PLATFORM_COCOA &&
        platformID != GLFW_PLATFORM_WAYLAND &&
        platformID != GLFW_PLATFORM_X11 &&
        platformID != GLFW_PLATFORM_NULL)
    {
        _glfwInputError(GLFW_INVALID_ENUM, "Invalid platform ID 0x%08X", platformID);
        return GLFW_FALSE;
    }

    if (platformID == GLFW_PLATFORM_NULL)
        return GLFW_TRUE;

    for (i = 0;  i < count;  i++)
    {
        if (platformID == supportedPlatforms[i].ID)
            return GLFW_TRUE;
    }

    return GLFW_FALSE;
}

GLFWAPI const char* glfwGetVersionString(void)
{
    return _GLFW_MAKE_VERSION(GLFW_VERSION_MAJOR,
                              GLFW_VERSION_MINOR,
                              GLFW_VERSION_REVISION)
#if defined(_GLFW_WIN32)
        " Win32 WGL"
#endif
#if defined(_GLFW_COCOA)
        " Cocoa NSGL"
#endif
#if defined(_GLFW_WAYLAND)
        " Wayland"
#endif
#if defined(_GLFW_X11)
        " X11 GLX"
#endif
        " Null"
        " EGL"
        " OSMesa"
#if defined(__MINGW64_VERSION_MAJOR)
        " MinGW-w64"
#elif defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#endif
#if defined(_GLFW_USE_HYBRID_HPG) || defined(_GLFW_USE_OPTIMUS_HPG)
        " hybrid-GPU"
#endif
#if defined(_POSIX_MONOTONIC_CLOCK)
        " monotonic"
#endif
#if defined(_GLFW_BUILD_DLL)
#if defined(_WIN32)
        " DLL"
#elif defined(__APPLE__)
        " dynamic"
#else
        " shared"
#endif
#endif
        ;
}

