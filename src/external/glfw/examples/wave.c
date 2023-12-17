/*****************************************************************************
 * Wave Simulation in OpenGL
 * (C) 2002 Jakob Thomsen
 * http://home.in.tum.de/~thomsen
 * Modified for GLFW by Sylvain Hellegouarch - sh@programmationworld.com
 * Modified for variable frame rate by Marcus Geelnard
 * 2003-Jan-31: Minor cleanups and speedups / MG
 * 2010-10-24: Formatting and cleanup - Camilla Löwy
 *****************************************************************************/

#if defined(_MSC_VER)
 // Make MS math.h define M_PI
 #define _USE_MATH_DEFINES
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <linmath.h>

// Maximum delta T to allow for differential calculations
#define MAX_DELTA_T 0.01

// Animation speed (10.0 looks good)
#define ANIMATION_SPEED 10.0

GLfloat alpha = 210.f, beta = -70.f;
GLfloat zoom = 2.f;

double cursorX;
double cursorY;

struct Vertex
{
    GLfloat x, y, z;
    GLfloat r, g, b;
};

#define GRIDW 50
#define GRIDH 50
#define VERTEXNUM (GRIDW*GRIDH)

#define QUADW (GRIDW - 1)
#define QUADH (GRIDH - 1)
#define QUADNUM (QUADW*QUADH)

GLuint quad[4 * QUADNUM];
struct Vertex vertex[VERTEXNUM];

/* The grid will look like this:
 *
 *      3   4   5
 *      *---*---*
 *      |   |   |
 *      | 0 | 1 |
 *      |   |   |
 *      *---*---*
 *      0   1   2
 */

//========================================================================
// Initialize grid geometry
//========================================================================

void init_vertices(void)
{
    int x, y, p;

    // Place the vertices in a grid
    for (y = 0;  y < GRIDH;  y++)
    {
        for (x = 0;  x < GRIDW;  x++)
        {
            p = y * GRIDW + x;

            vertex[p].x = (GLfloat) (x - GRIDW / 2) / (GLfloat) (GRIDW / 2);
            vertex[p].y = (GLfloat) (y - GRIDH / 2) / (GLfloat) (GRIDH / 2);
            vertex[p].z = 0;

            if ((x % 4 < 2) ^ (y % 4 < 2))
                vertex[p].r = 0.0;
            else
                vertex[p].r = 1.0;

            vertex[p].g = (GLfloat) y / (GLfloat) GRIDH;
            vertex[p].b = 1.f - ((GLfloat) x / (GLfloat) GRIDW + (GLfloat) y / (GLfloat) GRIDH) / 2.f;
        }
    }

    for (y = 0;  y < QUADH;  y++)
    {
        for (x = 0;  x < QUADW;  x++)
        {
            p = 4 * (y * QUADW + x);

            quad[p + 0] = y       * GRIDW + x;     // Some point
            quad[p + 1] = y       * GRIDW + x + 1; // Neighbor at the right side
            quad[p + 2] = (y + 1) * GRIDW + x + 1; // Upper right neighbor
            quad[p + 3] = (y + 1) * GRIDW + x;     // Upper neighbor
        }
    }
}

double dt;
double p[GRIDW][GRIDH];
double vx[GRIDW][GRIDH], vy[GRIDW][GRIDH];
double ax[GRIDW][GRIDH], ay[GRIDW][GRIDH];

//========================================================================
// Initialize grid
//========================================================================

void init_grid(void)
{
    int x, y;
    double dx, dy, d;

    for (y = 0; y < GRIDH;  y++)
    {
        for (x = 0; x < GRIDW;  x++)
        {
            dx = (double) (x - GRIDW / 2);
            dy = (double) (y - GRIDH / 2);
            d = sqrt(dx * dx + dy * dy);
            if (d < 0.1 * (double) (GRIDW / 2))
            {
                d = d * 10.0;
                p[x][y] = -cos(d * (M_PI / (double)(GRIDW * 4))) * 100.0;
            }
            else
                p[x][y] = 0.0;

            vx[x][y] = 0.0;
            vy[x][y] = 0.0;
        }
    }
}


//========================================================================
// Draw scene
//========================================================================

void draw_scene(GLFWwindow* window)
{
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // We don't want to modify the projection matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Move back
    glTranslatef(0.0, 0.0, -zoom);
    // Rotate the view
    glRotatef(beta, 1.0, 0.0, 0.0);
    glRotatef(alpha, 0.0, 0.0, 1.0);

    glDrawElements(GL_QUADS, 4 * QUADNUM, GL_UNSIGNED_INT, quad);

    glfwSwapBuffers(window);
}


//========================================================================
// Initialize Miscellaneous OpenGL state
//========================================================================

void init_opengl(void)
{
    // Use Gouraud (smooth) shading
    glShadeModel(GL_SMOOTH);

    // Switch on the z-buffer
    glEnable(GL_DEPTH_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), vertex);
    glColorPointer(3, GL_FLOAT, sizeof(struct Vertex), &vertex[0].r); // Pointer to the first color

    glPointSize(2.0);

    // Background color is black
    glClearColor(0, 0, 0, 0);
}


//========================================================================
// Modify the height of each vertex according to the pressure
//========================================================================

void adjust_grid(void)
{
    int pos;
    int x, y;

    for (y = 0; y < GRIDH;  y++)
    {
        for (x = 0;  x < GRIDW;  x++)
        {
            pos = y * GRIDW + x;
            vertex[pos].z = (float) (p[x][y] * (1.0 / 50.0));
        }
    }
}


//========================================================================
// Calculate wave propagation
//========================================================================

void calc_grid(void)
{
    int x, y, x2, y2;
    double time_step = dt * ANIMATION_SPEED;

    // Compute accelerations
    for (x = 0;  x < GRIDW;  x++)
    {
        x2 = (x + 1) % GRIDW;
        for(y = 0; y < GRIDH; y++)
            ax[x][y] = p[x][y] - p[x2][y];
    }

    for (y = 0;  y < GRIDH;  y++)
    {
        y2 = (y + 1) % GRIDH;
        for(x = 0; x < GRIDW; x++)
            ay[x][y] = p[x][y] - p[x][y2];
    }

    // Compute speeds
    for (x = 0;  x < GRIDW;  x++)
    {
        for (y = 0;  y < GRIDH;  y++)
        {
            vx[x][y] = vx[x][y] + ax[x][y] * time_step;
            vy[x][y] = vy[x][y] + ay[x][y] * time_step;
        }
    }

    // Compute pressure
    for (x = 1;  x < GRIDW;  x++)
    {
        x2 = x - 1;
        for (y = 1;  y < GRIDH;  y++)
        {
            y2 = y - 1;
            p[x][y] = p[x][y] + (vx[x2][y] - vx[x][y] + vy[x][y2] - vy[x][y]) * time_step;
        }
    }
}


//========================================================================
// Print errors
//========================================================================

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


//========================================================================
// Handle key strokes
//========================================================================

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_SPACE:
            init_grid();
            break;
        case GLFW_KEY_LEFT:
            alpha += 5;
            break;
        case GLFW_KEY_RIGHT:
            alpha -= 5;
            break;
        case GLFW_KEY_UP:
            beta -= 5;
            break;
        case GLFW_KEY_DOWN:
            beta += 5;
            break;
        case GLFW_KEY_PAGE_UP:
            zoom -= 0.25f;
            if (zoom < 0.f)
                zoom = 0.f;
            break;
        case GLFW_KEY_PAGE_DOWN:
            zoom += 0.25f;
            break;
        default:
            break;
    }
}


//========================================================================
// Callback function for mouse button events
//========================================================================

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &cursorX, &cursorY);
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}


//========================================================================
// Callback function for cursor motion events
//========================================================================

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        alpha += (GLfloat) (x - cursorX) / 10.f;
        beta += (GLfloat) (y - cursorY) / 10.f;

        cursorX = x;
        cursorY = y;
    }
}


//========================================================================
// Callback function for scroll events
//========================================================================

void scroll_callback(GLFWwindow* window, double x, double y)
{
    zoom += (float) y / 4.f;
    if (zoom < 0)
        zoom = 0;
}


//========================================================================
// Callback function for framebuffer resize events
//========================================================================

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    float ratio = 1.f;
    mat4x4 projection;

    if (height > 0)
        ratio = (float) width / (float) height;

    // Setup viewport
    glViewport(0, 0, width, height);

    // Change to the projection matrix and set our viewing volume
    glMatrixMode(GL_PROJECTION);
    mat4x4_perspective(projection,
                       60.f * (float) M_PI / 180.f,
                       ratio,
                       1.f, 1024.f);
    glLoadMatrixf((const GLfloat*) projection);
}


//========================================================================
// main
//========================================================================

int main(int argc, char* argv[])
{
    GLFWwindow* window;
    double t, dt_total, t_old;
    int width, height;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "Wave Simulation", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    // Initialize OpenGL
    init_opengl();

    // Initialize simulation
    init_vertices();
    init_grid();
    adjust_grid();

    // Initialize timer
    t_old = glfwGetTime() - 0.01;

    while (!glfwWindowShouldClose(window))
    {
        t = glfwGetTime();
        dt_total = t - t_old;
        t_old = t;

        // Safety - iterate if dt_total is too large
        while (dt_total > 0.f)
        {
            // Select iteration time step
            dt = dt_total > MAX_DELTA_T ? MAX_DELTA_T : dt_total;
            dt_total -= dt;

            // Calculate wave propagation
            calc_grid();
        }

        // Compute height of each vertex
        adjust_grid();

        // Draw wave grid to OpenGL display
        draw_scene(window);

        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

