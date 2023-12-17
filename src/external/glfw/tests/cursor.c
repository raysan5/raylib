//========================================================================
// Cursor & input mode tests
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
// This test provides an interface to the cursor image and cursor mode
// parts of the API.
//
// Custom cursor image generation by urraka.
//
//========================================================================

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "linmath.h"

#define CURSOR_FRAME_COUNT 60

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

static double cursor_x;
static double cursor_y;
static int swap_interval = 1;
static int wait_events = GLFW_TRUE;
static int animate_cursor = GLFW_FALSE;
static int track_cursor = GLFW_FALSE;
static GLFWcursor* standard_cursors[6];
static GLFWcursor* tracking_cursor = NULL;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static float star(int x, int y, float t)
{
    const float c = 64 / 2.f;

    const float i = (0.25f * (float) sin(2.f * M_PI * t) + 0.75f);
    const float k = 64 * 0.046875f * i;

    const float dist = (float) sqrt((x - c) * (x - c) + (y - c) * (y - c));

    const float salpha = 1.f - dist / c;
    const float xalpha = (float) x == c ? c : k / (float) fabs(x - c);
    const float yalpha = (float) y == c ? c : k / (float) fabs(y - c);

    return (float) fmax(0.f, fmin(1.f, i * salpha * 0.2f + salpha * xalpha * yalpha));
}

static GLFWcursor* create_cursor_frame(float t)
{
    int i = 0, x, y;
    unsigned char buffer[64 * 64 * 4];
    const GLFWimage image = { 64, 64, buffer };

    for (y = 0;  y < image.width;  y++)
    {
        for (x = 0;  x < image.height;  x++)
        {
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = 255;
            buffer[i++] = (unsigned char) (255 * star(x, y, t));
        }
    }

    return glfwCreateCursor(&image, image.width / 2, image.height / 2);
}

static GLFWcursor* create_tracking_cursor(void)
{
    int i = 0, x, y;
    unsigned char buffer[32 * 32 * 4];
    const GLFWimage image = { 32, 32, buffer };

    for (y = 0;  y < image.width;  y++)
    {
        for (x = 0;  x < image.height;  x++)
        {
            if (x == 7 || y == 7)
            {
                buffer[i++] = 255;
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 255;
            }
            else
            {
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 0;
                buffer[i++] = 0;
            }
        }
    }

    return glfwCreateCursor(&image, 7, 7);
}

static void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    printf("%0.3f: Cursor position: %f %f (%+f %+f)\n",
           glfwGetTime(),
           x, y, x - cursor_x, y - cursor_y);

    cursor_x = x;
    cursor_y = y;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_A:
        {
            animate_cursor = !animate_cursor;
            if (!animate_cursor)
                glfwSetCursor(window, NULL);

            break;
        }

        case GLFW_KEY_ESCAPE:
        {
            if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            }

            /* FALLTHROUGH */
        }

        case GLFW_KEY_N:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwGetCursorPos(window, &cursor_x, &cursor_y);
            printf("(( cursor is normal ))\n");
            break;

        case GLFW_KEY_D:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            printf("(( cursor is disabled ))\n");
            break;

        case GLFW_KEY_H:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            printf("(( cursor is hidden ))\n");
            break;

        case GLFW_KEY_R:
            if (!glfwRawMouseMotionSupported())
                break;

            if (glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION))
            {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
                printf("(( raw input is disabled ))\n");
            }
            else
            {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                printf("(( raw input is enabled ))\n");
            }
            break;

        case GLFW_KEY_SPACE:
            swap_interval = 1 - swap_interval;
            printf("(( swap interval: %i ))\n", swap_interval);
            glfwSwapInterval(swap_interval);
            break;

        case GLFW_KEY_W:
            wait_events = !wait_events;
            printf("(( %sing for events ))\n", wait_events ? "wait" : "poll");
            break;

        case GLFW_KEY_T:
            track_cursor = !track_cursor;
            if (track_cursor)
                glfwSetCursor(window, tracking_cursor);
            else
                glfwSetCursor(window, NULL);

            break;

        case GLFW_KEY_P:
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            printf("Query before set: %f %f (%+f %+f)\n",
                   x, y, x - cursor_x, y - cursor_y);
            cursor_x = x;
            cursor_y = y;

            glfwSetCursorPos(window, cursor_x, cursor_y);
            glfwGetCursorPos(window, &x, &y);

            printf("Query after set: %f %f (%+f %+f)\n",
                   x, y, x - cursor_x, y - cursor_y);
            cursor_x = x;
            cursor_y = y;
            break;
        }

        case GLFW_KEY_UP:
            glfwSetCursorPos(window, 0, 0);
            glfwGetCursorPos(window, &cursor_x, &cursor_y);
            break;

        case GLFW_KEY_DOWN:
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width - 1, height - 1);
            glfwGetCursorPos(window, &cursor_x, &cursor_y);
            break;
        }

        case GLFW_KEY_0:
            glfwSetCursor(window, NULL);
            break;

        case GLFW_KEY_1:
            glfwSetCursor(window, standard_cursors[0]);
            break;

        case GLFW_KEY_2:
            glfwSetCursor(window, standard_cursors[1]);
            break;

        case GLFW_KEY_3:
            glfwSetCursor(window, standard_cursors[2]);
            break;

        case GLFW_KEY_4:
            glfwSetCursor(window, standard_cursors[3]);
            break;

        case GLFW_KEY_5:
            glfwSetCursor(window, standard_cursors[4]);
            break;

        case GLFW_KEY_6:
            glfwSetCursor(window, standard_cursors[5]);
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

            glfwGetCursorPos(window, &cursor_x, &cursor_y);
            break;
        }
    }
}

int main(void)
{
    int i;
    GLFWwindow* window;
    GLFWcursor* star_cursors[CURSOR_FRAME_COUNT];
    GLFWcursor* current_frame = NULL;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    tracking_cursor = create_tracking_cursor();
    if (!tracking_cursor)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    for (i = 0;  i < CURSOR_FRAME_COUNT;  i++)
    {
        star_cursors[i] = create_cursor_frame(i / (float) CURSOR_FRAME_COUNT);
        if (!star_cursors[i])
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0;  i < sizeof(standard_cursors) / sizeof(standard_cursors[0]);  i++)
    {
        const int shapes[] = {
            GLFW_ARROW_CURSOR,
            GLFW_IBEAM_CURSOR,
            GLFW_CROSSHAIR_CURSOR,
            GLFW_HAND_CURSOR,
            GLFW_HRESIZE_CURSOR,
            GLFW_VRESIZE_CURSOR
        };

        standard_cursors[i] = glfwCreateStandardCursor(shapes[i]);
        if (!standard_cursors[i])
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Cursor Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

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
                          sizeof(vec2), (void*) 0);
    glUseProgram(program);

    glfwGetCursorPos(window, &cursor_x, &cursor_y);
    printf("Cursor position: %f %f\n", cursor_x, cursor_y);

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        if (track_cursor)
        {
            int wnd_width, wnd_height, fb_width, fb_height;
            float scale;
            vec2 vertices[4];
            mat4x4 mvp;

            glfwGetWindowSize(window, &wnd_width, &wnd_height);
            glfwGetFramebufferSize(window, &fb_width, &fb_height);

            glViewport(0, 0, fb_width, fb_height);

            scale = (float) fb_width / (float) wnd_width;
            vertices[0][0] = 0.5f;
            vertices[0][1] = (float) (fb_height - floor(cursor_y * scale) - 1.f + 0.5f);
            vertices[1][0] = (float) fb_width + 0.5f;
            vertices[1][1] = (float) (fb_height - floor(cursor_y * scale) - 1.f + 0.5f);
            vertices[2][0] = (float) floor(cursor_x * scale) + 0.5f;
            vertices[2][1] = 0.5f;
            vertices[3][0] = (float) floor(cursor_x * scale) + 0.5f;
            vertices[3][1] = (float) fb_height + 0.5f;

            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(vertices),
                         vertices,
                         GL_STREAM_DRAW);

            mat4x4_ortho(mvp, 0.f, (float) fb_width, 0.f, (float) fb_height, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

            glDrawArrays(GL_LINES, 0, 4);
        }

        glfwSwapBuffers(window);

        if (animate_cursor)
        {
            const int i = (int) (glfwGetTime() * 30.0) % CURSOR_FRAME_COUNT;
            if (current_frame != star_cursors[i])
            {
                glfwSetCursor(window, star_cursors[i]);
                current_frame = star_cursors[i];
            }
        }
        else
            current_frame = NULL;

        if (wait_events)
        {
            if (animate_cursor)
                glfwWaitEventsTimeout(1.0 / 30.0);
            else
                glfwWaitEvents();
        }
        else
            glfwPollEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    glfwDestroyWindow(window);

    for (i = 0;  i < CURSOR_FRAME_COUNT;  i++)
        glfwDestroyCursor(star_cursors[i]);

    for (i = 0;  i < sizeof(standard_cursors) / sizeof(standard_cursors[0]);  i++)
        glfwDestroyCursor(standard_cursors[i]);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

