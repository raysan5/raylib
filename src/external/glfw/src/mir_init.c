//========================================================================
// GLFW 3.3 Mir - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014-2017 Brandon Schaefer <brandon.schaefer@canonical.com>
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

#include <linux/input.h>
#include <stdlib.h>
#include <string.h>


// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_glfw.mir.keycodes, -1, sizeof(_glfw.mir.keycodes));
    memset(_glfw.mir.scancodes, -1, sizeof(_glfw.mir.scancodes));

    _glfw.mir.keycodes[KEY_GRAVE]      = GLFW_KEY_GRAVE_ACCENT;
    _glfw.mir.keycodes[KEY_1]          = GLFW_KEY_1;
    _glfw.mir.keycodes[KEY_2]          = GLFW_KEY_2;
    _glfw.mir.keycodes[KEY_3]          = GLFW_KEY_3;
    _glfw.mir.keycodes[KEY_4]          = GLFW_KEY_4;
    _glfw.mir.keycodes[KEY_5]          = GLFW_KEY_5;
    _glfw.mir.keycodes[KEY_6]          = GLFW_KEY_6;
    _glfw.mir.keycodes[KEY_7]          = GLFW_KEY_7;
    _glfw.mir.keycodes[KEY_8]          = GLFW_KEY_8;
    _glfw.mir.keycodes[KEY_9]          = GLFW_KEY_9;
    _glfw.mir.keycodes[KEY_0]          = GLFW_KEY_0;
    _glfw.mir.keycodes[KEY_SPACE]      = GLFW_KEY_SPACE;
    _glfw.mir.keycodes[KEY_MINUS]      = GLFW_KEY_MINUS;
    _glfw.mir.keycodes[KEY_EQUAL]      = GLFW_KEY_EQUAL;
    _glfw.mir.keycodes[KEY_Q]          = GLFW_KEY_Q;
    _glfw.mir.keycodes[KEY_W]          = GLFW_KEY_W;
    _glfw.mir.keycodes[KEY_E]          = GLFW_KEY_E;
    _glfw.mir.keycodes[KEY_R]          = GLFW_KEY_R;
    _glfw.mir.keycodes[KEY_T]          = GLFW_KEY_T;
    _glfw.mir.keycodes[KEY_Y]          = GLFW_KEY_Y;
    _glfw.mir.keycodes[KEY_U]          = GLFW_KEY_U;
    _glfw.mir.keycodes[KEY_I]          = GLFW_KEY_I;
    _glfw.mir.keycodes[KEY_O]          = GLFW_KEY_O;
    _glfw.mir.keycodes[KEY_P]          = GLFW_KEY_P;
    _glfw.mir.keycodes[KEY_LEFTBRACE]  = GLFW_KEY_LEFT_BRACKET;
    _glfw.mir.keycodes[KEY_RIGHTBRACE] = GLFW_KEY_RIGHT_BRACKET;
    _glfw.mir.keycodes[KEY_A]          = GLFW_KEY_A;
    _glfw.mir.keycodes[KEY_S]          = GLFW_KEY_S;
    _glfw.mir.keycodes[KEY_D]          = GLFW_KEY_D;
    _glfw.mir.keycodes[KEY_F]          = GLFW_KEY_F;
    _glfw.mir.keycodes[KEY_G]          = GLFW_KEY_G;
    _glfw.mir.keycodes[KEY_H]          = GLFW_KEY_H;
    _glfw.mir.keycodes[KEY_J]          = GLFW_KEY_J;
    _glfw.mir.keycodes[KEY_K]          = GLFW_KEY_K;
    _glfw.mir.keycodes[KEY_L]          = GLFW_KEY_L;
    _glfw.mir.keycodes[KEY_SEMICOLON]  = GLFW_KEY_SEMICOLON;
    _glfw.mir.keycodes[KEY_APOSTROPHE] = GLFW_KEY_APOSTROPHE;
    _glfw.mir.keycodes[KEY_Z]          = GLFW_KEY_Z;
    _glfw.mir.keycodes[KEY_X]          = GLFW_KEY_X;
    _glfw.mir.keycodes[KEY_C]          = GLFW_KEY_C;
    _glfw.mir.keycodes[KEY_V]          = GLFW_KEY_V;
    _glfw.mir.keycodes[KEY_B]          = GLFW_KEY_B;
    _glfw.mir.keycodes[KEY_N]          = GLFW_KEY_N;
    _glfw.mir.keycodes[KEY_M]          = GLFW_KEY_M;
    _glfw.mir.keycodes[KEY_COMMA]      = GLFW_KEY_COMMA;
    _glfw.mir.keycodes[KEY_DOT]        = GLFW_KEY_PERIOD;
    _glfw.mir.keycodes[KEY_SLASH]      = GLFW_KEY_SLASH;
    _glfw.mir.keycodes[KEY_BACKSLASH]  = GLFW_KEY_BACKSLASH;
    _glfw.mir.keycodes[KEY_ESC]        = GLFW_KEY_ESCAPE;
    _glfw.mir.keycodes[KEY_TAB]        = GLFW_KEY_TAB;
    _glfw.mir.keycodes[KEY_LEFTSHIFT]  = GLFW_KEY_LEFT_SHIFT;
    _glfw.mir.keycodes[KEY_RIGHTSHIFT] = GLFW_KEY_RIGHT_SHIFT;
    _glfw.mir.keycodes[KEY_LEFTCTRL]   = GLFW_KEY_LEFT_CONTROL;
    _glfw.mir.keycodes[KEY_RIGHTCTRL]  = GLFW_KEY_RIGHT_CONTROL;
    _glfw.mir.keycodes[KEY_LEFTALT]    = GLFW_KEY_LEFT_ALT;
    _glfw.mir.keycodes[KEY_RIGHTALT]   = GLFW_KEY_RIGHT_ALT;
    _glfw.mir.keycodes[KEY_LEFTMETA]   = GLFW_KEY_LEFT_SUPER;
    _glfw.mir.keycodes[KEY_RIGHTMETA]  = GLFW_KEY_RIGHT_SUPER;
    _glfw.mir.keycodes[KEY_MENU]       = GLFW_KEY_MENU;
    _glfw.mir.keycodes[KEY_NUMLOCK]    = GLFW_KEY_NUM_LOCK;
    _glfw.mir.keycodes[KEY_CAPSLOCK]   = GLFW_KEY_CAPS_LOCK;
    _glfw.mir.keycodes[KEY_PRINT]      = GLFW_KEY_PRINT_SCREEN;
    _glfw.mir.keycodes[KEY_SCROLLLOCK] = GLFW_KEY_SCROLL_LOCK;
    _glfw.mir.keycodes[KEY_PAUSE]      = GLFW_KEY_PAUSE;
    _glfw.mir.keycodes[KEY_DELETE]     = GLFW_KEY_DELETE;
    _glfw.mir.keycodes[KEY_BACKSPACE]  = GLFW_KEY_BACKSPACE;
    _glfw.mir.keycodes[KEY_ENTER]      = GLFW_KEY_ENTER;
    _glfw.mir.keycodes[KEY_HOME]       = GLFW_KEY_HOME;
    _glfw.mir.keycodes[KEY_END]        = GLFW_KEY_END;
    _glfw.mir.keycodes[KEY_PAGEUP]     = GLFW_KEY_PAGE_UP;
    _glfw.mir.keycodes[KEY_PAGEDOWN]   = GLFW_KEY_PAGE_DOWN;
    _glfw.mir.keycodes[KEY_INSERT]     = GLFW_KEY_INSERT;
    _glfw.mir.keycodes[KEY_LEFT]       = GLFW_KEY_LEFT;
    _glfw.mir.keycodes[KEY_RIGHT]      = GLFW_KEY_RIGHT;
    _glfw.mir.keycodes[KEY_DOWN]       = GLFW_KEY_DOWN;
    _glfw.mir.keycodes[KEY_UP]         = GLFW_KEY_UP;
    _glfw.mir.keycodes[KEY_F1]         = GLFW_KEY_F1;
    _glfw.mir.keycodes[KEY_F2]         = GLFW_KEY_F2;
    _glfw.mir.keycodes[KEY_F3]         = GLFW_KEY_F3;
    _glfw.mir.keycodes[KEY_F4]         = GLFW_KEY_F4;
    _glfw.mir.keycodes[KEY_F5]         = GLFW_KEY_F5;
    _glfw.mir.keycodes[KEY_F6]         = GLFW_KEY_F6;
    _glfw.mir.keycodes[KEY_F7]         = GLFW_KEY_F7;
    _glfw.mir.keycodes[KEY_F8]         = GLFW_KEY_F8;
    _glfw.mir.keycodes[KEY_F9]         = GLFW_KEY_F9;
    _glfw.mir.keycodes[KEY_F10]        = GLFW_KEY_F10;
    _glfw.mir.keycodes[KEY_F11]        = GLFW_KEY_F11;
    _glfw.mir.keycodes[KEY_F12]        = GLFW_KEY_F12;
    _glfw.mir.keycodes[KEY_F13]        = GLFW_KEY_F13;
    _glfw.mir.keycodes[KEY_F14]        = GLFW_KEY_F14;
    _glfw.mir.keycodes[KEY_F15]        = GLFW_KEY_F15;
    _glfw.mir.keycodes[KEY_F16]        = GLFW_KEY_F16;
    _glfw.mir.keycodes[KEY_F17]        = GLFW_KEY_F17;
    _glfw.mir.keycodes[KEY_F18]        = GLFW_KEY_F18;
    _glfw.mir.keycodes[KEY_F19]        = GLFW_KEY_F19;
    _glfw.mir.keycodes[KEY_F20]        = GLFW_KEY_F20;
    _glfw.mir.keycodes[KEY_F21]        = GLFW_KEY_F21;
    _glfw.mir.keycodes[KEY_F22]        = GLFW_KEY_F22;
    _glfw.mir.keycodes[KEY_F23]        = GLFW_KEY_F23;
    _glfw.mir.keycodes[KEY_F24]        = GLFW_KEY_F24;
    _glfw.mir.keycodes[KEY_KPSLASH]    = GLFW_KEY_KP_DIVIDE;
    _glfw.mir.keycodes[KEY_KPDOT]      = GLFW_KEY_KP_MULTIPLY;
    _glfw.mir.keycodes[KEY_KPMINUS]    = GLFW_KEY_KP_SUBTRACT;
    _glfw.mir.keycodes[KEY_KPPLUS]     = GLFW_KEY_KP_ADD;
    _glfw.mir.keycodes[KEY_KP0]        = GLFW_KEY_KP_0;
    _glfw.mir.keycodes[KEY_KP1]        = GLFW_KEY_KP_1;
    _glfw.mir.keycodes[KEY_KP2]        = GLFW_KEY_KP_2;
    _glfw.mir.keycodes[KEY_KP3]        = GLFW_KEY_KP_3;
    _glfw.mir.keycodes[KEY_KP4]        = GLFW_KEY_KP_4;
    _glfw.mir.keycodes[KEY_KP5]        = GLFW_KEY_KP_5;
    _glfw.mir.keycodes[KEY_KP6]        = GLFW_KEY_KP_6;
    _glfw.mir.keycodes[KEY_KP7]        = GLFW_KEY_KP_7;
    _glfw.mir.keycodes[KEY_KP8]        = GLFW_KEY_KP_8;
    _glfw.mir.keycodes[KEY_KP9]        = GLFW_KEY_KP_9;
    _glfw.mir.keycodes[KEY_KPCOMMA]    = GLFW_KEY_KP_DECIMAL;
    _glfw.mir.keycodes[KEY_KPEQUAL]    = GLFW_KEY_KP_EQUAL;
    _glfw.mir.keycodes[KEY_KPENTER]    = GLFW_KEY_KP_ENTER;

    for (scancode = 0;  scancode < 256;  scancode++)
    {
        if (_glfw.mir.keycodes[scancode] > 0)
            _glfw.mir.scancodes[_glfw.mir.keycodes[scancode]] = scancode;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void)
{
    int error;

    _glfw.mir.connection = mir_connect_sync(NULL, __PRETTY_FUNCTION__);

    if (!mir_connection_is_valid(_glfw.mir.connection))
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Unable to connect to server: %s",
                        mir_connection_get_error_message(_glfw.mir.connection));

        return GLFW_FALSE;
    }

    _glfw.mir.display =
        mir_connection_get_egl_native_display(_glfw.mir.connection);

    createKeyTables();

    if (!_glfwInitJoysticksLinux())
        return GLFW_FALSE;

    _glfwInitTimerPOSIX();

    _glfw.mir.eventQueue = calloc(1, sizeof(EventQueue));
    _glfwInitEventQueueMir(_glfw.mir.eventQueue);

    error = pthread_mutex_init(&_glfw.mir.eventMutex, NULL);
    if (error)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Mir: Failed to create event mutex: %s",
                        strerror(error));
        return GLFW_FALSE;
    }

    _glfwPollMonitorsMir();
    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void)
{
    _glfwTerminateEGL();
    _glfwTerminateJoysticksLinux();

    _glfwDeleteEventQueueMir(_glfw.mir.eventQueue);

    pthread_mutex_destroy(&_glfw.mir.eventMutex);

    mir_connection_release(_glfw.mir.connection);
}

const char* _glfwPlatformGetVersionString(void)
{
    return _GLFW_VERSION_NUMBER " Mir EGL"
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
        " clock_gettime"
#else
        " gettimeofday"
#endif
        " evdev"
#if defined(_GLFW_BUILD_DLL)
        " shared"
#endif
        ;
}

