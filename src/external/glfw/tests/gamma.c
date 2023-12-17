//========================================================================
// Gamma correction test program
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
// This program is used to test the gamma correction functionality for
// both full screen and windowed mode windows
//
//========================================================================

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void chart_ramp_array(struct nk_context* nk,
                             struct nk_color color,
                             int count, unsigned short int* values)
{
    if (nk_chart_begin_colored(nk, NK_CHART_LINES,
                               color, nk_rgb(255, 255, 255),
                               count, 0, 65535))
    {
        int i;
        for (i = 0;  i < count;  i++)
        {
            char buffer[1024];
            if (nk_chart_push(nk, values[i]))
            {
                snprintf(buffer, sizeof(buffer), "#%u: %u (%0.5f) ",
                         i, values[i], values[i] / 65535.f);
                nk_tooltip(nk, buffer);
            }
        }

        nk_chart_end(nk);
    }
}

int main(int argc, char** argv)
{
    GLFWmonitor* monitor = NULL;
    GLFWwindow* window;
    GLFWgammaramp orig_ramp;
    struct nk_context* nk;
    struct nk_font_atlas* atlas;
    float gamma_value = 1.f;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    monitor = glfwGetPrimaryMonitor();

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    window = glfwCreateWindow(800, 400, "Gamma Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    {
        const GLFWgammaramp* ramp = glfwGetGammaRamp(monitor);
        if (!ramp)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        const size_t array_size = ramp->size * sizeof(short);
        orig_ramp.size = ramp->size;
        orig_ramp.red = malloc(array_size);
        orig_ramp.green = malloc(array_size);
        orig_ramp.blue = malloc(array_size);
        memcpy(orig_ramp.red, ramp->red, array_size);
        memcpy(orig_ramp.green, ramp->green, array_size);
        memcpy(orig_ramp.blue, ramp->blue, array_size);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        struct nk_rect area;

        glfwGetWindowSize(window, &width, &height);
        area = nk_rect(0.f, 0.f, (float) width, (float) height);
        nk_window_set_bounds(nk, "", area);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_new_frame();
        if (nk_begin(nk, "", area, 0))
        {
            const GLFWgammaramp* ramp;

            nk_layout_row_dynamic(nk, 30, 3);
            if (nk_slider_float(nk, 0.1f, &gamma_value, 5.f, 0.1f))
                glfwSetGamma(monitor, gamma_value);
            nk_labelf(nk, NK_TEXT_LEFT, "%0.1f", gamma_value);
            if (nk_button_label(nk, "Revert"))
                glfwSetGammaRamp(monitor, &orig_ramp);

            ramp = glfwGetGammaRamp(monitor);

            nk_layout_row_dynamic(nk, height - 60.f, 3);
            chart_ramp_array(nk, nk_rgb(255, 0, 0), ramp->size, ramp->red);
            chart_ramp_array(nk, nk_rgb(0, 255, 0), ramp->size, ramp->green);
            chart_ramp_array(nk, nk_rgb(0, 0, 255), ramp->size, ramp->blue);
        }

        nk_end(nk);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        glfwSwapBuffers(window);
        glfwWaitEventsTimeout(1.0);
    }

    free(orig_ramp.red);
    free(orig_ramp.green);
    free(orig_ramp.blue);

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

