//========================================================================
// Context sharing example
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

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"
#include "linmath.h"

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec2 vPos;\n"
"varying vec2 texcoord;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    texcoord = vPos;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"uniform sampler2D texture;\n"
"uniform vec3 color;\n"
"varying vec2 texcoord;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color * texture2D(texture, texcoord).rgb, 1.0);\n"
"}\n";

static const vec2 vertices[4] =
{
    { 0.f, 0.f },
    { 1.f, 0.f },
    { 1.f, 1.f },
    { 0.f, 1.f }
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char** argv)
{
    GLFWwindow* windows[2];
    GLuint texture, program, vertex_buffer;
    GLint mvp_location, vpos_location, color_location, texture_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    windows[0] = glfwCreateWindow(400, 400, "First", NULL, NULL);
    if (!windows[0])
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(windows[0], key_callback);

    glfwMakeContextCurrent(windows[0]);

    // Only enable vsync for the first of the windows to be swapped to
    // avoid waiting out the interval for each window
    glfwSwapInterval(1);

    // The contexts are created with the same APIs so the function
    // pointers should be re-usable between them
    gladLoadGL(glfwGetProcAddress);

    // Create the OpenGL objects inside the first context, created above
    // All objects will be shared with the second context, created below
    {
        int x, y;
        char pixels[16 * 16];
        GLuint vertex_shader, fragment_shader;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        srand((unsigned int) glfwGetTimerValue());

        for (y = 0;  y < 16;  y++)
        {
            for (x = 0;  x < 16;  x++)
                pixels[y * 16 + x] = rand() % 256;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
        color_location = glGetUniformLocation(program, "color");
        texture_location = glGetUniformLocation(program, "texture");
        vpos_location = glGetAttribLocation(program, "vPos");

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    glUseProgram(program);
    glUniform1i(texture_location, 0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);

    windows[1] = glfwCreateWindow(400, 400, "Second", NULL, windows[0]);
    if (!windows[1])
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Place the second window to the right of the first
    {
        int xpos, ypos, left, right, width;

        glfwGetWindowSize(windows[0], &width, NULL);
        glfwGetWindowFrameSize(windows[0], &left, NULL, &right, NULL);
        glfwGetWindowPos(windows[0], &xpos, &ypos);

        glfwSetWindowPos(windows[1], xpos + width + left + right, ypos);
    }

    glfwSetKeyCallback(windows[1], key_callback);

    glfwMakeContextCurrent(windows[1]);

    // While objects are shared, the global context state is not and will
    // need to be set up for each context

    glUseProgram(program);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);

    while (!glfwWindowShouldClose(windows[0]) &&
           !glfwWindowShouldClose(windows[1]))
    {
        int i;
        const vec3 colors[2] =
        {
            { 0.8f, 0.4f, 1.f },
            { 0.3f, 0.4f, 1.f }
        };

        for (i = 0;  i < 2;  i++)
        {
            int width, height;
            mat4x4 mvp;

            glfwGetFramebufferSize(windows[i], &width, &height);
            glfwMakeContextCurrent(windows[i]);

            glViewport(0, 0, width, height);

            mat4x4_ortho(mvp, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f);
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
            glUniform3fv(color_location, 1, colors[i]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glfwSwapBuffers(windows[i]);
        }

        glfwWaitEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

