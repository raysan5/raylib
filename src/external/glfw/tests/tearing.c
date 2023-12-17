//========================================================================
// Vsync enabling test
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
// This test renders a high contrast, horizontally moving bar, allowing for
// visual verification of whether the set swap interval is indeed obeyed
//
//========================================================================

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "linmath.h"

static const struct
{
    float x, y;
} vertices[4] =
{
    { -0.25f, -1.f },
    {  0.25f, -1.f },
    {  0.25f,  1.f },
    { -0.25f,  1.f }
};

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec2 vPos;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(1.0);\n"
"}\n";

static int swap_tear;
static int swap_interval;
static double frame_rate;

static void update_window_title(GLFWwindow* window)
{
    char title[256];

    snprintf(title, sizeof(title), "Tearing detector (interval %i%s, %0.1f Hz)",
             swap_interval,
             (swap_tear && swap_interval < 0) ? " (swap tear)" : "",
             frame_rate);

    glfwSetWindowTitle(window, title);
}

static void set_swap_interval(GLFWwindow* window, int interval)
{
    swap_interval = interval;
    glfwSwapInterval(swap_interval);
    update_window_title(window);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_UP:
        {
            if (swap_interval + 1 > swap_interval)
                set_swap_interval(window, swap_interval + 1);
            break;
        }

        case GLFW_KEY_DOWN:
        {
            if (swap_tear)
            {
                if (swap_interval - 1 < swap_interval)
                    set_swap_interval(window, swap_interval - 1);
            }
            else
            {
                if (swap_interval - 1 >= 0)
                    set_swap_interval(window, swap_interval - 1);
            }
            break;
        }

        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, 1);
            break;

        case GLFW_KEY_F11:
        case GLFW_KEY_ENTER:
        {
            static int x, y, width, height;

            if (mods != GLFW_MOD_ALT)
                return;

            if (glfwGetWindowMonitor(window))
                glfwSetWindowMonitor(window, NULL, x, y, width, height, 0);
            else
            {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwGetWindowPos(window, &x, &y);
                glfwGetWindowSize(window, &width, &height);
                glfwSetWindowMonitor(window, monitor,
                                     0, 0, mode->width, mode->height,
                                     mode->refreshRate);
            }

            break;
        }
    }
}

int main(int argc, char** argv)
{
    unsigned long frame_count = 0;
    double last_time, current_time;
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Tearing detector", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    set_swap_interval(window, 0);

    last_time = glfwGetTime();
    frame_rate = 0.0;
    swap_tear = (glfwExtensionSupported("WGL_EXT_swap_control_tear") ||
                 glfwExtensionSupported("GLX_EXT_swap_control_tear"));

    glfwSetKeyCallback(window, key_callback);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        mat4x4 m, p, mvp;
        float position = cosf((float) glfwGetTime() * 4.f) * 0.75f;

        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_ortho(p, -1.f, 1.f, -1.f, 1.f, 0.f, 1.f);
        mat4x4_translate(m, position, 0.f, 0.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;

        current_time = glfwGetTime();
        if (current_time - last_time > 1.0)
        {
            frame_rate = frame_count / (current_time - last_time);
            frame_count = 0;
            last_time = current_time;
            update_window_title(window);
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

