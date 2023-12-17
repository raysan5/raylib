//========================================================================
// Iconify/restore test program
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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
//
// This program is used to test the iconify/restore functionality for
// both full screen and windowed mode windows
//
//========================================================================

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static int windowed_xpos, windowed_ypos, windowed_width = 640, windowed_height = 480;

static void usage(void)
{
    printf("Usage: iconify [-h] [-f [-a] [-n]]\n");
    printf("Options:\n");
    printf("  -a create windows for all monitors\n");
    printf("  -f create full screen window(s)\n");
    printf("  -h show this help\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    printf("%0.2f Key %s\n",
           glfwGetTime(),
           action == GLFW_PRESS ? "pressed" : "released");

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_I:
            glfwIconifyWindow(window);
            break;
        case GLFW_KEY_M:
            glfwMaximizeWindow(window);
            break;
        case GLFW_KEY_R:
            glfwRestoreWindow(window);
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_A:
            glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, !glfwGetWindowAttrib(window, GLFW_AUTO_ICONIFY));
            break;
        case GLFW_KEY_B:
            glfwSetWindowAttrib(window, GLFW_RESIZABLE, !glfwGetWindowAttrib(window, GLFW_RESIZABLE));
            break;
        case GLFW_KEY_D:
            glfwSetWindowAttrib(window, GLFW_DECORATED, !glfwGetWindowAttrib(window, GLFW_DECORATED));
            break;
        case GLFW_KEY_F:
            glfwSetWindowAttrib(window, GLFW_FLOATING, !glfwGetWindowAttrib(window, GLFW_FLOATING));
            break;
        case GLFW_KEY_F11:
        case GLFW_KEY_ENTER:
        {
            if (mods != GLFW_MOD_ALT)
                return;

            if (glfwGetWindowMonitor(window))
            {
                glfwSetWindowMonitor(window, NULL,
                                     windowed_xpos, windowed_ypos,
                                     windowed_width, windowed_height,
                                     0);
            }
            else
            {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                if (monitor)
                {
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    glfwGetWindowPos(window, &windowed_xpos, &windowed_ypos);
                    glfwGetWindowSize(window, &windowed_width, &windowed_height);
                    glfwSetWindowMonitor(window, monitor,
                                         0, 0, mode->width, mode->height,
                                         mode->refreshRate);
                }
            }

            break;
        }
    }
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    printf("%0.2f Window resized to %ix%i\n", glfwGetTime(), width, height);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    printf("%0.2f Framebuffer resized to %ix%i\n", glfwGetTime(), width, height);
}

static void window_focus_callback(GLFWwindow* window, int focused)
{
    printf("%0.2f Window %s\n",
           glfwGetTime(),
           focused ? "focused" : "defocused");
}

static void window_iconify_callback(GLFWwindow* window, int iconified)
{
    printf("%0.2f Window %s\n",
           glfwGetTime(),
           iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(GLFWwindow* window, int maximized)
{
    printf("%0.2f Window %s\n",
           glfwGetTime(),
           maximized ? "maximized" : "unmaximized");
}

static void window_refresh_callback(GLFWwindow* window)
{
    printf("%0.2f Window refresh\n", glfwGetTime());

    glfwMakeContextCurrent(window);

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
}

static GLFWwindow* create_window(GLFWmonitor* monitor)
{
    int width, height;
    GLFWwindow* window;

    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = windowed_width;
        height = windowed_height;
    }

    window = glfwCreateWindow(width, height, "Iconify", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    return window;
}

int main(int argc, char** argv)
{
    int ch, i, window_count;
    int fullscreen = GLFW_FALSE, all_monitors = GLFW_FALSE;
    GLFWwindow** windows;

    while ((ch = getopt(argc, argv, "afhn")) != -1)
    {
        switch (ch)
        {
            case 'a':
                all_monitors = GLFW_TRUE;
                break;

            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = GLFW_TRUE;
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (fullscreen && all_monitors)
    {
        int monitor_count;
        GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

        window_count = monitor_count;
        windows = calloc(window_count, sizeof(GLFWwindow*));

        for (i = 0;  i < monitor_count;  i++)
        {
            windows[i] = create_window(monitors[i]);
            if (!windows[i])
                break;
        }
    }
    else
    {
        GLFWmonitor* monitor = NULL;

        if (fullscreen)
            monitor = glfwGetPrimaryMonitor();

        window_count = 1;
        windows = calloc(window_count, sizeof(GLFWwindow*));
        windows[0] = create_window(monitor);
    }

    for (i = 0;  i < window_count;  i++)
    {
        glfwSetKeyCallback(windows[i], key_callback);
        glfwSetFramebufferSizeCallback(windows[i], framebuffer_size_callback);
        glfwSetWindowSizeCallback(windows[i], window_size_callback);
        glfwSetWindowFocusCallback(windows[i], window_focus_callback);
        glfwSetWindowIconifyCallback(windows[i], window_iconify_callback);
        glfwSetWindowMaximizeCallback(windows[i], window_maximize_callback);
        glfwSetWindowRefreshCallback(windows[i], window_refresh_callback);

        window_refresh_callback(windows[i]);

        printf("Window is %s and %s\n",
            glfwGetWindowAttrib(windows[i], GLFW_ICONIFIED) ? "iconified" : "restored",
            glfwGetWindowAttrib(windows[i], GLFW_FOCUSED) ? "focused" : "defocused");
    }

    for (;;)
    {
        glfwWaitEvents();

        for (i = 0;  i < window_count;  i++)
        {
            if (glfwWindowShouldClose(windows[i]))
                break;
        }

        if (i < window_count)
            break;

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

