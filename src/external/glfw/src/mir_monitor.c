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

#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _glfwPollMonitorsMir(void)
{
    int i;
    MirDisplayConfig* displayConfig =
        mir_connection_create_display_configuration(_glfw.mir.connection);

    int numOutputs = mir_display_config_get_num_outputs(displayConfig);

    for (i = 0;  i < numOutputs;  i++)
    {
        const MirOutput* output        = mir_display_config_get_output(displayConfig, i);
        MirOutputConnectionState state = mir_output_get_connection_state(output);
        bool enabled = mir_output_is_enabled(output);

        if (enabled && state == mir_output_connection_state_connected)
        {
            int widthMM  = mir_output_get_physical_width_mm(output);
            int heightMM = mir_output_get_physical_height_mm(output);
            int x  = mir_output_get_position_x(output);
            int y  = mir_output_get_position_y(output);
            int id = mir_output_get_id(output);
            size_t currentMode = mir_output_get_current_mode_index(output);
            const char* name   = mir_output_type_name(mir_output_get_type(output));

            _GLFWmonitor* monitor = _glfwAllocMonitor(name,
                                                      widthMM,
                                                      heightMM);
            monitor->mir.x        = x;
            monitor->mir.y        = y;
            monitor->mir.outputId = id;
            monitor->mir.curMode  = currentMode;
            monitor->modes = _glfwPlatformGetVideoModes(monitor, &monitor->modeCount);

            _glfwInputMonitor(monitor, GLFW_CONNECTED, _GLFW_INSERT_LAST);
        }
    }

    mir_display_config_release(displayConfig);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = monitor->mir.x;
    if (ypos)
        *ypos = monitor->mir.y;
}

void _glfwPlatformGetMonitorContentScale(_GLFWmonitor* monitor,
                                         float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

static void FillInRGBBitsFromPixelFormat(GLFWvidmode* mode, const MirPixelFormat pf)
{
    switch (pf)
    {
      case mir_pixel_format_rgb_565:
          mode->redBits   = 5;
          mode->greenBits = 6;
          mode->blueBits  = 5;
          break;
      case mir_pixel_format_rgba_5551:
          mode->redBits   = 5;
          mode->greenBits = 5;
          mode->blueBits  = 5;
          break;
      case mir_pixel_format_rgba_4444:
          mode->redBits   = 4;
          mode->greenBits = 4;
          mode->blueBits  = 4;
          break;
      case mir_pixel_format_abgr_8888:
      case mir_pixel_format_xbgr_8888:
      case mir_pixel_format_argb_8888:
      case mir_pixel_format_xrgb_8888:
      case mir_pixel_format_bgr_888:
      case mir_pixel_format_rgb_888:
      default:
          mode->redBits   = 8;
          mode->greenBits = 8;
          mode->blueBits  = 8;
          break;
    }
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
    int i;
    GLFWvidmode* modes = NULL;
    MirDisplayConfig* displayConfig =
        mir_connection_create_display_configuration(_glfw.mir.connection);

    int numOutputs = mir_display_config_get_num_outputs(displayConfig);

    for (i = 0;  i < numOutputs;  i++)
    {
        const MirOutput* output = mir_display_config_get_output(displayConfig, i);
        int id = mir_output_get_id(output);

        if (id != monitor->mir.outputId)
            continue;

        MirOutputConnectionState state = mir_output_get_connection_state(output);
        bool enabled = mir_output_is_enabled(output);

        // We must have been disconnected
        if (!enabled || state != mir_output_connection_state_connected)
        {
            _glfwInputError(GLFW_PLATFORM_ERROR,
                            "Mir: Monitor no longer connected");
            return NULL;
        }

        int numModes = mir_output_get_num_modes(output);
        modes = calloc(numModes, sizeof(GLFWvidmode));

        for (*found = 0;  *found < numModes;  (*found)++)
        {
            const MirOutputMode* mode = mir_output_get_mode(output, *found);
            int width  = mir_output_mode_get_width(mode);
            int height = mir_output_mode_get_height(mode);
            double refreshRate = mir_output_mode_get_refresh_rate(mode);
            MirPixelFormat currentFormat = mir_output_get_current_pixel_format(output);

            modes[*found].width  = width;
            modes[*found].height = height;
            modes[*found].refreshRate = refreshRate;

            FillInRGBBitsFromPixelFormat(&modes[*found], currentFormat);
        }

        break;
    }

    mir_display_config_release(displayConfig);

    return modes;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
    *mode = monitor->modes[monitor->mir.curMode];
}

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    _glfwInputError(GLFW_PLATFORM_ERROR,
                    "Mir: Unsupported function %s", __PRETTY_FUNCTION__);
}


//////////////////////////////////////////////////////////////////////////
//////                        GLFW native API                       //////
//////////////////////////////////////////////////////////////////////////

GLFWAPI int glfwGetMirMonitor(GLFWmonitor* handle)
{
    _GLFWmonitor* monitor = (_GLFWmonitor*) handle;
    _GLFW_REQUIRE_INIT_OR_RETURN(0);
    return monitor->mir.outputId;
}
