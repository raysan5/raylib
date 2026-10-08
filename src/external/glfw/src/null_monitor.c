//========================================================================
// GLFW 3.5 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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

#include <stdlib.h>
#include <string.h>
#include <math.h>

// The the sole (fake) video mode of our (sole) fake monitor
//
static GLFWvidmode getVideoMode(void)
{
    GLFWvidmode mode;
    mode.width = 1920;
    mode.height = 1080;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = 60;
    return mode;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPollMonitorsNull(void)
{
    const float dpi = 141.f;
    const GLFWvidmode mode = getVideoMode();
    _GLFWmonitor* monitor = _glfwAllocMonitor("Null SuperNoop 0",
                                              (int) (mode.width * 25.4f / dpi),
                                              (int) (mode.height * 25.4f / dpi));
    _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_FIRST);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwFreeMonitorNull(_GLFWmonitor* monitor)
{
    _glfwFreeGammaArrays(&monitor->null.ramp);
}

void _glfwGetMonitorPosNull(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
}

void _glfwGetMonitorContentScaleNull(_GLFWmonitor* monitor,
                                     float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _glfwGetMonitorWorkareaNull(_GLFWmonitor* monitor,
                                 int* xpos, int* ypos,
                                 int* width, int* height)
{
    const GLFWvidmode mode = getVideoMode();

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 10;
    if (width)
        *width = mode.width;
    if (height)
        *height = mode.height - 10;
}

GLFWvidmode* _glfwGetVideoModesNull(_GLFWmonitor* monitor, int* found)
{
    GLFWvidmode* mode = _glfw_calloc(1, sizeof(GLFWvidmode));
    *mode = getVideoMode();
    *found = 1;
    return mode;
}

GLFWbool _glfwGetVideoModeNull(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    *mode = getVideoMode();
    return GLFW_TRUE;
}

GLFWbool _glfwGetGammaRampNull(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    if (!monitor->null.ramp.size)
    {
        unsigned int i;

        _glfwAllocGammaArrays(&monitor->null.ramp, 256);

        for (i = 0;  i < monitor->null.ramp.size;  i++)
        {
            const float gamma = 2.2f;
            float value;
            value = i / (float) (monitor->null.ramp.size - 1);
            value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
            value = fminf(value, 65535.f);

            monitor->null.ramp.red[i]   = (unsigned short) value;
            monitor->null.ramp.green[i] = (unsigned short) value;
            monitor->null.ramp.blue[i]  = (unsigned short) value;
        }
    }

    _glfwAllocGammaArrays(ramp, monitor->null.ramp.size);
    memcpy(ramp->red,   monitor->null.ramp.red,   sizeof(short) * ramp->size);
    memcpy(ramp->green, monitor->null.ramp.green, sizeof(short) * ramp->size);
    memcpy(ramp->blue,  monitor->null.ramp.blue,  sizeof(short) * ramp->size);
    return GLFW_TRUE;
}

void _glfwSetGammaRampNull(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    if (monitor->null.ramp.size != ramp->size)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "Null: Gamma ramp size must match current ramp size");
        return;
    }

    memcpy(monitor->null.ramp.red,   ramp->red,   sizeof(short) * ramp->size);
    memcpy(monitor->null.ramp.green, ramp->green, sizeof(short) * ramp->size);
    memcpy(monitor->null.ramp.blue,  ramp->blue,  sizeof(short) * ramp->size);
}

