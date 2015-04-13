/**********************************************************************************************
*
*   rlgl - raylib OpenGL abstraction layer
*
*   raylib now uses OpenGL 1.1 style functions (rlVertex) that are mapped to selected OpenGL version:
*       OpenGL 1.1  - Direct map rl* -> gl*
*       OpenGL 3.3+ - Vertex data is stored in VAOs, call rlglDraw() to render
*       OpenGL ES 2 - Same behaviour as OpenGL 3.3+
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "rlgl.h"

#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Declares malloc() and free() for memory management, rand()

#if defined(GRAPHICS_API_OPENGL_11)
    #ifdef __APPLE__            // OpenGL include for OSX
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>      // Basic OpenGL include
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_33)
    #define GLEW_STATIC
    #ifdef __APPLE__            // OpenGL include for OSX
        #include <OpenGL/gl3.h>
    #else
        #include <GL/glew.h>    // Extensions loading lib
        //#include "glad.h"     // TODO: Other extensions loading lib? --> REVIEW
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #include <EGL/egl.h>
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MATRIX_STACK_SIZE          16   // Matrix stack max size
#define MAX_DRAWS_BY_TEXTURE      256   // Draws are organized by texture changes
#define TEMP_VERTEX_BUFFER_SIZE  4096   // Temporal Vertex Buffer (required for vertex-transformations)
                                        // NOTE: Every vertex are 3 floats (12 bytes)

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_ETC1_RGB8_OES
    #define GL_ETC1_RGB8_OES                 0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2          0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC     0x9278
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Vertex buffer (position + color arrays)
// NOTE: Used for lines and triangles VAOs
typedef struct {
    int vCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    unsigned char *colors;      // 4 components per vertex
} VertexPositionColorBuffer;

// Vertex buffer (position + texcoords + color arrays)
// NOTE: Not used
typedef struct {
    int vCounter;
    int tcCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    unsigned char *colors;      // 4 components per vertex
} VertexPositionColorTextureBuffer;

// Vertex buffer (position + texcoords + normals arrays)
// NOTE: Not used
typedef struct {
    int vCounter;
    int tcCounter;
    int nCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    float *normals;             // 3 components per vertex
    //short *normals;           // NOTE: Less data load... but padding issues and normalizing required!
} VertexPositionTextureNormalBuffer;

// Vertex buffer (position + texcoords + colors + indices arrays)
// NOTE: Used for quads VAO
typedef struct {
    int vCounter;
    int tcCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    unsigned char *colors;      // 4 components per vertex
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      // 6 indices per quad (could be int)
#elif defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // 6 indices per quad (must be short)
                                // NOTE: 6*2 byte = 12 byte, not alignment problem!
#endif
} VertexPositionColorTextureIndexBuffer;

// Draw call type
// NOTE: Used to track required draw-calls, organized by texture
typedef struct {
    GLuint textureId;
    int vertexCount;
} DrawCall;

// pixel type (same as Color type)
// NOTE: Used exclusively in mipmap generation functions
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} pixel;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static Matrix stack[MATRIX_STACK_SIZE];
static int stackCounter = 0;

static Matrix modelview;
static Matrix projection;
static Matrix *currentMatrix;
static int currentMatrixMode;

static DrawMode currentDrawMode;

// Vertex arrays for lines, triangles and quads
static VertexPositionColorBuffer lines;         // No texture support
static VertexPositionColorBuffer triangles;     // No texture support
static VertexPositionColorTextureIndexBuffer quads;

// Shader Programs
static Shader defaultShader, simpleShader;

// Vertex Array Objects (VAO)
static GLuint vaoLines, vaoTriangles, vaoQuads;

// Vertex Buffer Objects (VBO)
static GLuint linesBuffer[2];
static GLuint trianglesBuffer[2];
static GLuint quadsBuffer[4];

static DrawCall *draws;
static int drawsCounter;

// Temp vertex buffer to be used with rlTranslate, rlRotate, rlScale
static Vector3 *tempBuffer;
static int tempBufferCount = 0;
static bool useTempBuffer = false;

// Support for VAOs (OpenGL ES2 could not support VAO extensions)
static bool vaoSupported = false;

// Framebuffer object and texture
static GLuint fbo, fboColorTexture, fboDepthTexture;
static Model postproQuad;
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
// NOTE: VAO functionality is exposed through extensions (OES)
// emscripten does not support VAOs
static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays;
static PFNGLISVERTEXARRAYOESPROC glIsVertexArray;
#endif

// White texture useful for plain color polys (required by shader)
// NOTE: It's required in shapes and models modules!
unsigned int whiteTexture;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static Shader LoadDefaultShader(void);
static Shader LoadSimpleShader(void);
static void InitializeBuffers(void);
static void InitializeBuffersGPU(void);
static void UpdateBuffers(void);
static void LoadCompressedTexture(unsigned char *data, int width, int height, int mipmapCount, int compressedFormat);

// Custom shader files loading (external)
static char *TextFileRead(char *fn);
#endif

#if defined(GRAPHICS_API_OPENGL_11)
static int GenerateMipmaps(unsigned char *data, int baseWidth, int baseHeight);
static pixel *GenNextMipmap(pixel *srcData, int srcWidth, int srcHeight);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix operations
//----------------------------------------------------------------------------------

#if defined(GRAPHICS_API_OPENGL_11)

// Fallback to OpenGL 1.1 function calls
//---------------------------------------
void rlMatrixMode(int mode)
{
    switch (mode)
    {
        case RL_PROJECTION: glMatrixMode(GL_PROJECTION); break;
        case RL_MODELVIEW: glMatrixMode(GL_MODELVIEW); break;
        case RL_TEXTURE: glMatrixMode(GL_TEXTURE); break;
        default: break;
    }
}

void rlFrustum(double left, double right, double bottom, double top, double near, double far)
{
    glFrustum(left, right, bottom, top, near, far);
}

void rlOrtho(double left, double right, double bottom, double top, double near, double far)
{
    glOrtho(left, right, bottom, top, near, far);
}

void rlPushMatrix(void) { glPushMatrix(); }
void rlPopMatrix(void) { glPopMatrix(); }
void rlLoadIdentity(void) { glLoadIdentity(); }
void rlTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
void rlRotatef(float angleDeg, float x, float y, float z) { glRotatef(angleDeg, x, y, z); }
void rlScalef(float x, float y, float z) { glScalef(x, y, z); }
void rlMultMatrixf(float *mat) { glMultMatrixf(mat); }

#elif defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) currentMatrix = &projection;
    else if (mode == RL_MODELVIEW) currentMatrix = &modelview;
    //else if (mode == RL_TEXTURE) // Not supported

    currentMatrixMode = mode;
}

// Push the current matrix to stack
void rlPushMatrix(void)
{
    if (stackCounter == MATRIX_STACK_SIZE - 1)
    {
        TraceLog(ERROR, "Stack Buffer Overflow (MAX %i Matrix)", MATRIX_STACK_SIZE);
    }

    stack[stackCounter] = *currentMatrix;
    rlLoadIdentity();
    stackCounter++;

    if (currentMatrixMode == RL_MODELVIEW) useTempBuffer = true;
}

// Pop lattest inserted matrix from stack
void rlPopMatrix(void)
{
    if (stackCounter > 0)
    {
        Matrix mat = stack[stackCounter - 1];
        *currentMatrix = mat;
        stackCounter--;
    }
}

// Reset current matrix to identity matrix
void rlLoadIdentity(void)
{
    *currentMatrix = MatrixIdentity();
}

// Multiply the current matrix by a translation matrix
void rlTranslatef(float x, float y, float z)
{
    Matrix mat = MatrixTranslate(x, y, z);
    MatrixTranspose(&mat);

    *currentMatrix = MatrixMultiply(*currentMatrix, mat);
}

// Multiply the current matrix by a rotation matrix
void rlRotatef(float angleDeg, float x, float y, float z)
{
    // TODO: Support rotation in multiple axes
    Matrix rotation = MatrixIdentity();

    // OPTION 1: It works...
    //if (x == 1) rot = MatrixRotateX(angleDeg*DEG2RAD);
    //else if (y == 1) rot = MatrixRotateY(angleDeg*DEG2RAD);
    //else if (z == 1) rot = MatrixRotateZ(angleDeg*DEG2RAD);

    // OPTION 2: Requires review...
    Vector3 axis = (Vector3){ x, y, z };
    VectorNormalize(&axis);
    rotation = MatrixRotateY(angleDeg*DEG2RAD); //MatrixFromAxisAngle(axis, angleDeg*DEG2RAD);

    // OPTION 3: TODO: Review, it doesn't work!
    //Vector3 vec = (Vector3){ x, y, z };
    //VectorNormalize(&vec);
    //rot = MatrixRotate(angleDeg*vec.x, angleDeg*vec.x, angleDeg*vec.x);

    MatrixTranspose(&rotation);

    *currentMatrix = MatrixMultiply(*currentMatrix, rotation);
}

// Multiply the current matrix by a scaling matrix
void rlScalef(float x, float y, float z)
{
    Matrix mat = MatrixScale(x, y, z);
    MatrixTranspose(&mat);

    *currentMatrix = MatrixMultiply(*currentMatrix, mat);
}

// Multiply the current matrix by another matrix
void rlMultMatrixf(float *m)
{
    // Matrix creation from array
    Matrix mat = { m[0], m[1], m[2], m[3],
                   m[4], m[5], m[6], m[7],
                   m[8], m[9], m[10], m[11],
                   m[12], m[13], m[14], m[15] };

    *currentMatrix = MatrixMultiply(*currentMatrix, mat);
}

// Multiply the current matrix by a perspective matrix generated by parameters
void rlFrustum(double left, double right, double bottom, double top, double near, double far)
{
    Matrix matPerps = MatrixFrustum(left, right, bottom, top, near, far);
    MatrixTranspose(&matPerps);

    *currentMatrix = MatrixMultiply(*currentMatrix, matPerps);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
void rlOrtho(double left, double right, double bottom, double top, double near, double far)
{
    Matrix matOrtho = MatrixOrtho(left, right, bottom, top, near, far);
    MatrixTranspose(&matOrtho);

    *currentMatrix = MatrixMultiply(*currentMatrix, matOrtho);
}

#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Vertex level operations
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_11)

// Fallback to OpenGL 1.1 function calls
//---------------------------------------
void rlBegin(int mode)
{
    switch (mode)
    {
        case RL_LINES: glBegin(GL_LINES); break;
        case RL_TRIANGLES: glBegin(GL_TRIANGLES); break;
        case RL_QUADS: glBegin(GL_QUADS); break;
        default: break;
    }
}

void rlEnd() { glEnd(); }
void rlVertex2i(int x, int y) { glVertex2i(x, y); }
void rlVertex2f(float x, float y) { glVertex2f(x, y); }
void rlVertex3f(float x, float y, float z) { glVertex3f(x, y, z); }
void rlTexCoord2f(float x, float y) { glTexCoord2f(x, y); }
void rlNormal3f(float x, float y, float z) { glNormal3f(x, y, z); }
void rlColor4ub(byte r, byte g, byte b, byte a) { glColor4ub(r, g, b, a); }
void rlColor3f(float x, float y, float z) { glColor3f(x, y, z); }
void rlColor4f(float x, float y, float z, float w) { glColor4f(x, y, z, w); }

#elif defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

// Initialize drawing mode (how to organize vertex)
void rlBegin(int mode)
{
    // Draw mode can only be RL_LINES, RL_TRIANGLES and RL_QUADS
    currentDrawMode = mode;
}

// Finish vertex providing
void rlEnd(void)
{
    if (useTempBuffer)
    {
        // NOTE: In this case, *currentMatrix is already transposed because transposing has been applied
        // independently to translation-scale-rotation matrices -> t(M1 x M2) = t(M2) x t(M1)
        // This way, rlTranslatef(), rlRotatef()... behaviour is the same than OpenGL 1.1

        // Apply transformation matrix to all temp vertices
        for (int i = 0; i < tempBufferCount; i++) VectorTransform(&tempBuffer[i], *currentMatrix);

        // Deactivate tempBuffer usage to allow rlVertex3f do its job
        useTempBuffer = false;

        // Copy all transformed vertices to right VAO
        for (int i = 0; i < tempBufferCount; i++) rlVertex3f(tempBuffer[i].x, tempBuffer[i].y, tempBuffer[i].z);

        // Reset temp buffer
        tempBufferCount = 0;
    }

    // Make sure vertexCount is the same for vertices-texcoords-normals-colors
    // NOTE: In OpenGL 1.1, one glColor call can be made for all the subsequent glVertex calls.
    switch (currentDrawMode)
    {
        case RL_LINES:
        {
            if (lines.vCounter != lines.cCounter)
            {
                int addColors = lines.vCounter - lines.cCounter;

                for (int i = 0; i < addColors; i++)
                {
                    lines.colors[4*lines.cCounter] = lines.colors[4*lines.cCounter - 4];
                    lines.colors[4*lines.cCounter + 1] = lines.colors[4*lines.cCounter - 3];
                    lines.colors[4*lines.cCounter + 2] = lines.colors[4*lines.cCounter - 2];
                    lines.colors[4*lines.cCounter + 3] = lines.colors[4*lines.cCounter - 1];

                    lines.cCounter++;
                }
            }
        } break;
        case RL_TRIANGLES:
        {
            if (triangles.vCounter != triangles.cCounter)
            {
                int addColors = triangles.vCounter - triangles.cCounter;

                for (int i = 0; i < addColors; i++)
                {
                    triangles.colors[4*triangles.cCounter] = triangles.colors[4*triangles.cCounter - 4];
                    triangles.colors[4*triangles.cCounter + 1] = triangles.colors[4*triangles.cCounter - 3];
                    triangles.colors[4*triangles.cCounter + 2] = triangles.colors[4*triangles.cCounter - 2];
                    triangles.colors[4*triangles.cCounter + 3] = triangles.colors[4*triangles.cCounter - 1];

                    triangles.cCounter++;
                }
            }
        } break;
        case RL_QUADS:
        {
            // Make sure colors count match vertex count
            if (quads.vCounter != quads.cCounter)
            {
                int addColors = quads.vCounter - quads.cCounter;

                for (int i = 0; i < addColors; i++)
                {
                    quads.colors[4*quads.cCounter] = quads.colors[4*quads.cCounter - 4];
                    quads.colors[4*quads.cCounter + 1] = quads.colors[4*quads.cCounter - 3];
                    quads.colors[4*quads.cCounter + 2] = quads.colors[4*quads.cCounter - 2];
                    quads.colors[4*quads.cCounter + 3] = quads.colors[4*quads.cCounter - 1];

                    quads.cCounter++;
                }
            }

            // Make sure texcoords count match vertex count
            if (quads.vCounter != quads.tcCounter)
            {
                int addTexCoords = quads.vCounter - quads.tcCounter;

                for (int i = 0; i < addTexCoords; i++)
                {
                    quads.texcoords[2*quads.tcCounter] = 0.0f;
                    quads.texcoords[2*quads.tcCounter + 1] = 0.0f;

                    quads.tcCounter++;
                }
            }

            // TODO: Make sure normals count match vertex count... if normals support is added in a future... :P

        } break;
        default: break;
    }
}

// Define one vertex (position)
void rlVertex3f(float x, float y, float z)
{
    if (useTempBuffer)
    {
        tempBuffer[tempBufferCount].x = x;
        tempBuffer[tempBufferCount].y = y;
        tempBuffer[tempBufferCount].z = z;
        tempBufferCount++;
    }
    else
    {
        switch (currentDrawMode)
        {
            case RL_LINES:
            {
                // Verify that MAX_LINES_BATCH limit not reached
                if (lines.vCounter / 2 < MAX_LINES_BATCH)
                {
                    lines.vertices[3*lines.vCounter] = x;
                    lines.vertices[3*lines.vCounter + 1] = y;
                    lines.vertices[3*lines.vCounter + 2] = z;

                    lines.vCounter++;
                }
                else TraceLog(ERROR, "MAX_LINES_BATCH overflow");

            } break;
            case RL_TRIANGLES:
            {
                // Verify that MAX_TRIANGLES_BATCH limit not reached
                if (triangles.vCounter / 3 < MAX_TRIANGLES_BATCH)
                {
                    triangles.vertices[3*triangles.vCounter] = x;
                    triangles.vertices[3*triangles.vCounter + 1] = y;
                    triangles.vertices[3*triangles.vCounter + 2] = z;

                    triangles.vCounter++;
                }
                else TraceLog(ERROR, "MAX_TRIANGLES_BATCH overflow");

            } break;
            case RL_QUADS:
            {
                // Verify that MAX_QUADS_BATCH limit not reached
                if (quads.vCounter / 4 < MAX_QUADS_BATCH)
                {
                    quads.vertices[3*quads.vCounter] = x;
                    quads.vertices[3*quads.vCounter + 1] = y;
                    quads.vertices[3*quads.vCounter + 2] = z;

                    quads.vCounter++;

                    draws[drawsCounter - 1].vertexCount++;
                }
                else TraceLog(ERROR, "MAX_QUADS_BATCH overflow");

            } break;
            default: break;
        }
    }
}

// Define one vertex (position)
void rlVertex2f(float x, float y)
{
    rlVertex3f(x, y, 0.0f);
}

// Define one vertex (position)
void rlVertex2i(int x, int y)
{
    rlVertex3f((float)x, (float)y, 0.0f);
}

// Define one vertex (texture coordinate)
// NOTE: Texture coordinates are limited to QUADS only
void rlTexCoord2f(float x, float y)
{
    if (currentDrawMode == RL_QUADS)
    {
        quads.texcoords[2*quads.tcCounter] = x;
        quads.texcoords[2*quads.tcCounter + 1] = y;

        quads.tcCounter++;
    }
}

// Define one vertex (normal)
// NOTE: Normals limited to TRIANGLES only ?
void rlNormal3f(float x, float y, float z)
{
    // TODO: Normals usage...
}

// Define one vertex (color)
void rlColor4ub(byte x, byte y, byte z, byte w)
{
    switch (currentDrawMode)
    {
        case RL_LINES:
        {
            lines.colors[4*lines.cCounter] = x;
            lines.colors[4*lines.cCounter + 1] = y;
            lines.colors[4*lines.cCounter + 2] = z;
            lines.colors[4*lines.cCounter + 3] = w;

            lines.cCounter++;

        } break;
        case RL_TRIANGLES:
        {
            triangles.colors[4*triangles.cCounter] = x;
            triangles.colors[4*triangles.cCounter + 1] = y;
            triangles.colors[4*triangles.cCounter + 2] = z;
            triangles.colors[4*triangles.cCounter + 3] = w;

            triangles.cCounter++;

        } break;
        case RL_QUADS:
        {
            quads.colors[4*quads.cCounter] = x;
            quads.colors[4*quads.cCounter + 1] = y;
            quads.colors[4*quads.cCounter + 2] = z;
            quads.colors[4*quads.cCounter + 3] = w;

            quads.cCounter++;

        } break;
        default: break;
    }
}

// Define one vertex (color)
void rlColor4f(float r, float g, float b, float a)
{
    rlColor4ub((byte)(r*255), (byte)(g*255), (byte)(b*255), (byte)(a*255));
}

// Define one vertex (color)
void rlColor3f(float x, float y, float z)
{
    rlColor4ub((byte)(x*255), (byte)(y*255), (byte)(z*255), 255);
}

#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - OpenGL equivalent functions (common to 1.1, 3.3+, ES2)
//----------------------------------------------------------------------------------

// Enable texture usage
void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (draws[drawsCounter - 1].textureId != id)
    {
        if (draws[drawsCounter - 1].vertexCount > 0) drawsCounter++;

        draws[drawsCounter - 1].textureId = id;
        draws[drawsCounter - 1].vertexCount = 0;
    }
#endif
}

// Disable texture usage
void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

// Unload texture from GPU memory
void rlDeleteTextures(unsigned int id)
{
    glDeleteTextures(1, &id);
}

// Enable rendering to postprocessing FBO
void rlEnableFBO(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
#endif
}

// Unload shader from GPU memory
void rlDeleteShader(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteProgram(id);
#endif
}

// Unload vertex data (VAO) from GPU memory
void rlDeleteVertexArrays(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (vaoSupported) glDeleteVertexArrays(1, &id);
#endif
}

// Unload vertex data (VBO) from GPU memory
void rlDeleteBuffers(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteBuffers(1, &id);
#endif
}

// Clear color buffer with color
void rlClearColor(byte r, byte g, byte b, byte a)
{
    // Color values clamp to 0.0f(0) and 1.0f(255)
    float cr = (float)r / 255;
    float cg = (float)g / 255;
    float cb = (float)b / 255;
    float ca = (float)a / 255;

    glClearColor(cr, cg, cb, ca);
}

// Clear used screen buffers (color and depth)
void rlClearScreenBuffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers: Color and Depth (Depth is used for 3D)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     // Stencil buffer not used...
}

// Returns current OpenGL version
int rlGetVersion(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    return OPENGL_11;
#elif defined(GRAPHICS_API_OPENGL_33)
    return OPENGL_33;
#elif defined(GRAPHICS_API_OPENGL_ES2)
    return OPENGL_ES_20;
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - rlgl Functions
//----------------------------------------------------------------------------------

// Init OpenGL 3.3+ required data
void rlglInit(void)
{
#if defined(GRAPHICS_API_OPENGL_33)
    // Loading extensions the hard way (Example)
/*
    GLint numExt;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    for (int i = 0; i < numExt; i++)
    {
        const GLubyte *extensionName = glGetStringi(GL_EXTENSIONS, i);
        if (strcmp(extensionName, (const GLubyte *)"GL_ARB_vertex_array_object") == 0)
        {
            // The extension is supported by our hardware and driver, try to get related functions pointers
            glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)wglGetProcAddress("glGenVertexArrays");
            glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)wglGetProcAddress("glBindVertexArray");
            glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)wglGetProcAddress("glDeleteVertexArrays");
            glIsVertexArray = (PFNGLISVERTEXARRAYOESPROC)wglGetProcAddress("glIsVertexArray");
        }
    }
*/

    // Initialize extensions using GLEW
    glewExperimental = 1;       // Needed for core profile

    GLenum error = glewInit();

    if (error != GLEW_OK) TraceLog(ERROR, "Failed to initialize GLEW - Error Code: %s\n", glewGetErrorString(error));

    if (glewIsSupported("GL_VERSION_3_3")) TraceLog(INFO, "OpenGL 3.3 extensions supported");

    // NOTE: GLEW is a big library that loads ALL extensions, using glad we can only load required ones...
    //if (!gladLoadGL()) TraceLog("ERROR: Failed to initialize glad\n");

    vaoSupported = true;
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: emscripten does not support VAOs natively, it uses emulation and it reduces overall performance...
#if !defined(PLATFORM_WEB)
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
    glIsVertexArray = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");
#endif

    if (glGenVertexArrays == NULL) TraceLog(WARNING, "Could not initialize VAO extensions, VAOs not supported");
    else
    {
        vaoSupported = true;
        TraceLog(INFO, "VAO extensions initialized successfully");
    }
#endif

    // Print current OpenGL and GLSL version
    TraceLog(INFO, "GPU: Vendor:   %s", glGetString(GL_VENDOR));
    TraceLog(INFO, "GPU: Renderer: %s", glGetString(GL_RENDERER));
    TraceLog(INFO, "GPU: Version:  %s", glGetString(GL_VERSION));
    TraceLog(INFO, "GPU: GLSL:     %s", glGetString(0x8B8C));  //GL_SHADING_LANGUAGE_VERSION

    // NOTE: We can get a bunch of extra information about GPU capabilities (glGet*)
    //int maxTexSize;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    //TraceLog(INFO, "GL_MAX_TEXTURE_SIZE: %i", maxTexSize);

    //int numAuxBuffers;
    //glGetIntegerv(GL_AUX_BUFFERS, &numAuxBuffers);
    //TraceLog(INFO, "GL_AUX_BUFFERS: %i", numAuxBuffers);

    // Show supported extensions
    // NOTE: We don't need that much data on screen... right now...

#if defined(GRAPHICS_API_OPENGL_33)
    GLint numExt;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    for (int i = 0; i < numExt; i++)
    {
        //TraceLog(INFO, "Supported extension: %s", glGetStringi(GL_EXTENSIONS, i));
        /*
        if (strcmp(glGetStringi(GL_EXTENSIONS, i),"GL_EXT_texture_compression_s3tc") == 0)
        {
            // DDS texture compression support
            
            // TODO: Check required tokens
        }
        else if (strcmp(glGetStringi(GL_EXTENSIONS, i),"GL_OES_compressed_ETC1_RGB8_texture") == 0)
        {
            // ETC1 texture compression support
        }
        else if (strcmp(glGetStringi(GL_EXTENSIONS, i),"GL_ARB_ES3_compatibility") == 0)
        {
            //OES_compressed_ETC2_RGB8_texture,
            //OES_compressed_ETC2_RGBA8_texture,
            // ETC2/EAC texture compression support
        }
        else if (strcmp(glGetStringi(GL_EXTENSIONS, i),"GL_IMG_texture_compression_pvrtc") == 0)
        {
            // PVR texture compression support
        }
        else if (strcmp(glGetStringi(GL_EXTENSIONS, i),"GL_KHR_texture_compression_astc_hdr") == 0)
        {
            // ASTC texture compression support
        }
        */
    }
#elif defined(GRAPHICS_API_OPENGL_ES2)
    char *extensions = (char *)glGetString(GL_EXTENSIONS);  // One big string

    // NOTE: String could be splitted using strtok() function (string.h)
    TraceLog(INFO, "Supported extension: %s", extensions);
#endif

/*
    GLint numComp = 0;
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numComp);
    
    GLint format[32] = { 0 };
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, format);

    for (int i = 0; i < numComp; i++)
    {
        TraceLog(INFO, "Supported compressed format: 0x%x", format[i]);
    }
*/
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Set default draw mode
    currentDrawMode = RL_TRIANGLES;

    // Reset projection and modelview matrices
    projection = MatrixIdentity();
    modelview = MatrixIdentity();
    currentMatrix = &modelview;

    // Initialize matrix stack
    for (int i = 0; i < MATRIX_STACK_SIZE; i++) stack[i] = MatrixIdentity();

    // Init default Shader (GLSL 110) -> Common for GL 3.3+ and ES2
    defaultShader = LoadDefaultShader();
    simpleShader = LoadSimpleShader();
    //customShader = rlglLoadShader("custom.vs", "custom.fs");     // Works ok

    InitializeBuffers();        // Init vertex arrays
    InitializeBuffersGPU();     // Init VBO and VAO

    // Init temp vertex buffer, used when transformation required (translate, rotate, scale)
    tempBuffer = (Vector3 *)malloc(sizeof(Vector3)*TEMP_VERTEX_BUFFER_SIZE);

    for (int i = 0; i < TEMP_VERTEX_BUFFER_SIZE; i++) tempBuffer[i] = VectorZero();

    // Create default white texture for plain colors (required by shader)
    unsigned char pixels[4] = { 255, 255, 255, 255 };   // 1 pixel RGBA (4 bytes)

    whiteTexture = rlglLoadTexture(pixels, 1, 1, UNCOMPRESSED_R8G8B8A8, 1, false);

    if (whiteTexture != 0) TraceLog(INFO, "[TEX ID %i] Base white texture loaded successfully", whiteTexture);
    else TraceLog(WARNING, "Base white texture could not be loaded");

    // Init draw calls tracking system
    draws = (DrawCall *)malloc(sizeof(DrawCall)*MAX_DRAWS_BY_TEXTURE);

    for (int i = 0; i < MAX_DRAWS_BY_TEXTURE; i++)
    {
        draws[i].textureId = 0;
        draws[i].vertexCount = 0;
    }

    drawsCounter = 1;
    draws[drawsCounter - 1].textureId = whiteTexture;
#endif
}

// Init postpro system
void rlglInitPostpro(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Create the texture that will serve as the color attachment for the framebuffer
    glGenTextures(1, &fboColorTexture);
    glBindTexture(GL_TEXTURE_2D, fboColorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GetScreenWidth(), GetScreenHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the texture that will serve as the depth attachment for the framebuffer.
    glGenTextures(1, &fboDepthTexture);
    glBindTexture(GL_TEXTURE_2D, fboDepthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GetScreenWidth(), GetScreenHeight(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the framebuffer object
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Attach color texture and depth texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fboDepthTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) TraceLog(WARNING, "Framebuffer object could not be created...");
    else TraceLog(INFO, "[FBO ID %i] Framebuffer object created successfully", fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create a simple quad model to render fbo texture
    VertexData quadData;
    
    quadData.vertexCount = 6;
    
    float w = GetScreenWidth();
    float h = GetScreenHeight();
    
    float quadPositions[6*3] = { w, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, h, 0.0, 0, h, 0.0, w, h, 0.0, w, 0.0, 0.0 }; 
    float quadTexcoords[6*2] = { 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0 };
    float quadNormals[6*3] = { 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0 };
    unsigned char quadColors[6*4] = { 255 };
    
    quadData.vertices = quadPositions;
    quadData.texcoords = quadTexcoords;
    quadData.normals = quadNormals;
    quadData.colors = quadColors;
    
    postproQuad = rlglLoadModel(quadData);
    
    Texture2D texture;
    texture.id = fboColorTexture;
    texture.width = w;
    texture.height = h;
    
    SetModelTexture(&postproQuad, texture);
#endif
}

// Set postprocessing shader
void rlglSetPostproShader(Shader shader)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    SetModelShader(&postproQuad, shader);
#endif
}

// Vertex Buffer Object deinitialization (memory free)
void rlglClose(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Unbind everything
    if (vaoSupported) glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);

    // Delete VBOs
    glDeleteBuffers(1, &linesBuffer[0]);
    glDeleteBuffers(1, &linesBuffer[1]);
    glDeleteBuffers(1, &trianglesBuffer[0]);
    glDeleteBuffers(1, &trianglesBuffer[1]);
    glDeleteBuffers(1, &quadsBuffer[0]);
    glDeleteBuffers(1, &quadsBuffer[1]);
    glDeleteBuffers(1, &quadsBuffer[2]);
    glDeleteBuffers(1, &quadsBuffer[3]);

    if (vaoSupported)
    {
        // Delete VAOs
        glDeleteVertexArrays(1, &vaoLines);
        glDeleteVertexArrays(1, &vaoTriangles);
        glDeleteVertexArrays(1, &vaoQuads);
    }

    //glDetachShader(defaultShaderProgram, v);
    //glDetachShader(defaultShaderProgram, f);
    //glDeleteShader(v);
    //glDeleteShader(f);
    glDeleteProgram(defaultShader.id);
    glDeleteProgram(simpleShader.id);

    // Free vertex arrays memory
    free(lines.vertices);
    free(lines.colors);

    free(triangles.vertices);
    free(triangles.colors);

    free(quads.vertices);
    free(quads.texcoords);
    free(quads.colors);
    free(quads.indices);

    // Free GPU texture
    glDeleteTextures(1, &whiteTexture);

    if (fbo != 0)
    {
        glDeleteFramebuffers(1, &fbo);
        
        UnloadModel(postproQuad);
    }

    free(draws);
#endif
}

void rlglDraw(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    UpdateBuffers();

    if ((lines.vCounter > 0) || (triangles.vCounter > 0) || (quads.vCounter > 0))
    {
        glUseProgram(defaultShader.id);

        glUniformMatrix4fv(defaultShader.projectionLoc, 1, false, GetMatrixVector(projection));
        glUniformMatrix4fv(defaultShader.modelviewLoc, 1, false, GetMatrixVector(modelview));
        glUniform1i(defaultShader.textureLoc, 0);
    }

    // NOTE: We draw in this order: triangle shapes, textured quads and lines

    if (triangles.vCounter > 0)
    {
        glBindTexture(GL_TEXTURE_2D, whiteTexture);

        if (vaoSupported)
        {
            glBindVertexArray(vaoTriangles);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[0]);
            glVertexAttribPointer(defaultShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(defaultShader.vertexLoc);

            glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[1]);
            glVertexAttribPointer(defaultShader.colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
            glEnableVertexAttribArray(defaultShader.colorLoc);
        }

        glDrawArrays(GL_TRIANGLES, 0, triangles.vCounter);

        if (!vaoSupported) glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (quads.vCounter > 0)
    {
        int quadsCount = 0;
        int numIndicesToProcess = 0;
        int indicesOffset = 0;

        if (vaoSupported)
        {
            glBindVertexArray(vaoQuads);
        }
        else
        {
            // Enable vertex attributes
            glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[0]);
            glVertexAttribPointer(defaultShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(defaultShader.vertexLoc);

            glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[1]);
            glVertexAttribPointer(defaultShader.texcoordLoc, 2, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(defaultShader.texcoordLoc);

            glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[2]);
            glVertexAttribPointer(defaultShader.colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
            glEnableVertexAttribArray(defaultShader.colorLoc);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadsBuffer[3]);
        }

        //TraceLog(DEBUG, "Draws required per frame: %i", drawsCounter);

        for (int i = 0; i < drawsCounter; i++)
        {
            quadsCount = draws[i].vertexCount/4;
            numIndicesToProcess = quadsCount*6;  // Get number of Quads * 6 index by Quad

            //TraceLog(DEBUG, "Quads to render: %i - Vertex Count: %i", quadsCount, draws[i].vertexCount);

            glBindTexture(GL_TEXTURE_2D, draws[i].textureId);

            // NOTE: The final parameter tells the GPU the offset in bytes from the start of the index buffer to the location of the first index to process
#if defined(GRAPHICS_API_OPENGL_33)
            glDrawElements(GL_TRIANGLES, numIndicesToProcess, GL_UNSIGNED_INT, (GLvoid*) (sizeof(GLuint) * indicesOffset));
#elif defined(GRAPHICS_API_OPENGL_ES2)
            glDrawElements(GL_TRIANGLES, numIndicesToProcess, GL_UNSIGNED_SHORT, (GLvoid*) (sizeof(GLushort) * indicesOffset));
#endif
            //GLenum err;
            //if ((err = glGetError()) != GL_NO_ERROR) TraceLog(INFO, "OpenGL error: %i", (int)err);    //GL_INVALID_ENUM!

            indicesOffset += draws[i].vertexCount/4*6;
        }

        if (!vaoSupported)
        {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        glBindTexture(GL_TEXTURE_2D, 0);  // Unbind textures
    }

    if (lines.vCounter > 0)
    {
        glBindTexture(GL_TEXTURE_2D, whiteTexture);

        if (vaoSupported)
        {
            glBindVertexArray(vaoLines);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[0]);
            glVertexAttribPointer(defaultShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(defaultShader.vertexLoc);

            glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[1]);
            glVertexAttribPointer(defaultShader.colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
            glEnableVertexAttribArray(defaultShader.colorLoc);
        }

        glDrawArrays(GL_LINES, 0, lines.vCounter);

        if (!vaoSupported) glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (vaoSupported) glBindVertexArray(0);   // Unbind VAO

    glUseProgram(0);    // Unbind shader program

    // Reset draws counter
    drawsCounter = 1;
    draws[0].textureId = whiteTexture;
    draws[0].vertexCount = 0;

    // Reset vertex counters for next frame
    lines.vCounter = 0;
    lines.cCounter = 0;

    triangles.vCounter = 0;
    triangles.cCounter = 0;

    quads.vCounter = 0;
    quads.tcCounter = 0;
    quads.cCounter = 0;
#endif
}

// Draw with postprocessing shader
void rlglDrawPostpro(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    rlglDrawModel(postproQuad, (Vector3){0,0,0}, 0.0f, (Vector3){0,0,0}, (Vector3){1.0f, 1.0f, 1.0f}, WHITE, false);
#endif
}

// Draw a 3d model
void rlglDrawModel(Model model, Vector3 position, float rotationAngle, Vector3 rotationAxis, Vector3 scale, Color color, bool wires)
{
#if defined (GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    if (wires) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, model.texture.id);

    // NOTE: On OpenGL 1.1 we use Vertex Arrays to draw model
    glEnableClientState(GL_VERTEX_ARRAY);                     // Enable vertex array
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);              // Enable texture coords array
    glEnableClientState(GL_NORMAL_ARRAY);                     // Enable normals array

    glVertexPointer(3, GL_FLOAT, 0, model.mesh.vertices);     // Pointer to vertex coords array
    glTexCoordPointer(2, GL_FLOAT, 0, model.mesh.texcoords);  // Pointer to texture coords array
    glNormalPointer(GL_FLOAT, 0, model.mesh.normals);         // Pointer to normals array
    //glColorPointer(4, GL_UNSIGNED_BYTE, 0, model.mesh.colors);   // Pointer to colors array (NOT USED)

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlScalef(scale.x, scale.y, scale.z);
        rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

        // TODO: If rotate in multiple axis, get rotation matrix and use rlMultMatrix()

        rlColor4ub(color.r, color.g, color.b, color.a);

        glDrawArrays(GL_TRIANGLES, 0, model.mesh.vertexCount);
    rlPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);                     // Disable vertex array
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);              // Disable texture coords array
    glDisableClientState(GL_NORMAL_ARRAY);                     // Disable normals array

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glUseProgram(model.shader.id);

    // TODO: Use model.transform matrix

    Vector3 rotation = { 0.0f, 0.0f, 0.0f };

    // Get transform matrix (rotation -> scale -> translation)
    Matrix transform = MatrixTransform(position, rotation, scale);  // Object-space transformation
    Matrix modelviewworld = MatrixMultiply(transform, modelview);   // World-space transformation

    // Projection: Screen-space transformation

    // NOTE: Drawing in OpenGL 3.3+, transform is passed to shader
    glUniformMatrix4fv(model.shader.projectionLoc, 1, false, GetMatrixVector(projection));
    glUniformMatrix4fv(model.shader.modelviewLoc, 1, false, GetMatrixVector(modelviewworld));
    glUniform1i(model.shader.textureLoc, 0);

    // Apply color tinting to model
    // NOTE: Just update one uniform on fragment shader
    float vColor[4] = { (float)color.r/255, (float)color.g/255, (float)color.b/255, (float)color.a/255 };

    glUniform4fv(model.shader.tintColorLoc, 1, vColor);

    if (vaoSupported)
    {
        glBindVertexArray(model.mesh.vaoId);
    }
    else
    {
        // Bind model VBOs data
        glBindBuffer(GL_ARRAY_BUFFER, model.mesh.vboId[0]);
        glVertexAttribPointer(model.shader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(model.shader.vertexLoc);

        glBindBuffer(GL_ARRAY_BUFFER, model.mesh.vboId[1]);
        glVertexAttribPointer(model.shader.texcoordLoc, 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(model.shader.texcoordLoc);

        // Add normals support
        glBindBuffer(GL_ARRAY_BUFFER, model.mesh.vboId[2]);
        glVertexAttribPointer(model.shader.normalLoc, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(model.shader.normalLoc);
    }

    glBindTexture(GL_TEXTURE_2D, model.texture.id);

    glDrawArrays(GL_TRIANGLES, 0, model.mesh.vertexCount);

    glBindTexture(GL_TEXTURE_2D, 0);            // Unbind textures

    if (vaoSupported) glBindVertexArray(0);     // Unbind VAO
    else glBindBuffer(GL_ARRAY_BUFFER, 0);      // Unbind VBOs

    glUseProgram(0);
#endif

#if defined (GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    if (wires) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

// Initialize Graphics Device (OpenGL stuff)
void rlglInitGraphics(int offsetX, int offsetY, int width, int height)
{
    // NOTE: Required! viewport must be recalculated if screen resized!
    glViewport(offsetX/2, offsetY/2, width - offsetX, height - offsetY);    // Set viewport width and height

    // NOTE: Don't confuse glViewport with the transformation matrix
    // NOTE: glViewport just defines the area of the context that you will actually draw to.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers, depth buffer is used for 3D
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   // Set background color (black)
    //glClearDepth(1.0f);                                   // Clear depth buffer (default)

    glEnable(GL_DEPTH_TEST);                                // Enables depth testing (required for 3D)
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply

    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)

#if defined(GRAPHICS_API_OPENGL_11)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation (Deprecated in OGL 3.0)
                                                            // Other options: GL_FASTEST, GL_DONT_CARE (default)
#endif

    rlMatrixMode(RL_PROJECTION);                // Switch to PROJECTION matrix
    rlLoadIdentity();                           // Reset current matrix (PROJECTION)

    rlOrtho(0, width - offsetX, height - offsetY, 0, 0, 1); // Config orthographic mode: top-left corner --> (0,0)

    rlMatrixMode(RL_MODELVIEW);                 // Switch back to MODELVIEW matrix
    rlLoadIdentity();                           // Reset current matrix (MODELVIEW)

    // NOTE: All shapes/models triangles are drawn CCW

    glEnable(GL_CULL_FACE);       // Enable backface culling (Disabled by default)
    //glCullFace(GL_BACK);        // Cull the Back face (default)
    //glFrontFace(GL_CCW);        // Front face are defined counter clockwise (default)

#if defined(GRAPHICS_API_OPENGL_11)
    glShadeModel(GL_SMOOTH);      // Smooth shading between vertex (vertex colors interpolation) (Deprecated on OpenGL 3.3+)
                                  // Possible options: GL_SMOOTH (Color interpolation) or GL_FLAT (no interpolation)
#endif

    // TODO: Review this comment when called from window resize callback
    TraceLog(INFO, "OpenGL Graphics initialized successfully");
}

// Get world coordinates from screen coordinates
Vector3 rlglUnproject(Vector3 source, Matrix proj, Matrix view)
{
    //GLint viewport[4];
    //glGetIntegerv(GL_VIEWPORT, viewport);

    // Viewport data
    int x = 0;
    int y = 0;
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
/*
    Matrix modelviewprojection = MatrixMultiply(modelview, projection);
    MatrixInvert(&modelviewprojection);

    Vector3 vector;

    vector.x = (((source.x - x) / ((float)width)) * 2.0f) - 1.0f;
    vector.y = -((((source.y - y) / ((float)height)) * 2.0f) - 1.0f);
    vector.z = (source.z - minDepth) / (maxDepth - minDepth);

    //float a = (((vector.x * matrix.M14) + (vector.y * matrix.M24)) + (vector.z * matrix.M34)) + matrix.M44;
    //float a = (((vector.x * modelviewprojection.m3) + (vector.y * modelviewprojection.m7)) + (vector.z * modelviewprojection.m11)) + modelviewprojection.m15;
    VectorTransform(&vector, modelviewprojection);

    //if (!MathUtil.IsOne(a)) vector = (vector / a);
    //VectorScale(&vector, 1/a);

    return vector;
*/
/*
    Vector3 worldPoint;

    // Transformation matrices
    Matrix modelviewprojection = MatrixIdentity();
    Quaternion quat;

    // Calculation for inverting a matrix, compute projection x modelview
    modelviewprojection = MatrixMultiply(proj, view);
    MatrixInvert(&modelviewprojection);

    // Transformation of normalized coordinates between -1 and 1
    quat.x = ((source.x - (float)x)/(float)width*2.0) - 1.0f;
    quat.y = ((source.y - (float)y)/(float)height*2.0) - 1.0f;
    quat.z = 2.0*source.z - 1.0;
    quat.w = 1.0;

    // Objects coordinates
    QuaternionTransform(&quat, modelviewprojection);

    //if (quat.w == 0.0) return 0;

    worldPoint.x = quat.x/quat.w;
    worldPoint.y = quat.y/quat.w;
    worldPoint.z = quat.z/quat.w;

    return worldPoint;
    */
/*
    Quaternion quat;
    Vector3 vec;

    quat.x = 2.0f * GetMousePosition().x / (float)width - 1;
    quat.y = -(2.0f * GetMousePosition().y / (float)height - 1);
    quat.z = 0;
    quat.w = 1;

    Matrix invView;
    MatrixInvert(&view);
    Matrix invProj;
    MatrixInvert(&proj);

    quat.x = invProj.m0 * quat.x + invProj.m4 * quat.y + invProj.m8 * quat.z + invProj.m12 * quat.w;
    quat.y = invProj.m1 * quat.x + invProj.m5 * quat.y + invProj.m9 * quat.z + invProj.m13 * quat.w;
    quat.z = invProj.m2 * quat.x + invProj.m6 * quat.y + invProj.m10 * quat.z + invProj.m14 * quat.w;
    quat.w = invProj.m3 * quat.x + invProj.m7 * quat.y + invProj.m11 * quat.z + invProj.m15 * quat.w;

    quat.x = invView.m0 * quat.x + invView.m4 * quat.y + invView.m8 * quat.z + invView.m12 * quat.w;
    quat.y = invView.m1 * quat.x + invView.m5 * quat.y + invView.m9 * quat.z + invView.m13 * quat.w;
    quat.z = invView.m2 * quat.x + invView.m6 * quat.y + invView.m10 * quat.z + invView.m14 * quat.w;
    quat.w = invView.m3 * quat.x + invView.m7 * quat.y + invView.m11 * quat.z + invView.m15 * quat.w;

    vec.x /= quat.w;
    vec.y /= quat.w;
    vec.z /= quat.w;

    return vec;
    */
/*
    Vector3 worldPoint;

    // Transformation matrices
    Matrix modelviewprojection;
    Quaternion quat;

    // Calculation for inverting a matrix, compute projection x modelview
    modelviewprojection = MatrixMultiply(view, proj);

    // Now compute the inverse of matrix A
    MatrixInvert(&modelviewprojection);

    // Transformation of normalized coordinates between -1 and 1
    quat.x = ((source.x - (float)x)/(float)width*2.0) - 1.0f;
    quat.y = ((source.y - (float)y)/(float)height*2.0) - 1.0f;
    quat.z = 2.0*source.z - 1.0;
    quat.w = 1.0;

    // Traspose quaternion and multiply
    Quaternion result;
    result.x = modelviewprojection.m0 * quad.x + modelviewprojection.m4 * quad.y + modelviewprojection.m8 * quad.z + modelviewprojection.m12 * quad.w;
    result.y = modelviewprojection.m1 * quad.x + modelviewprojection.m5 * quad.y + modelviewprojection.m9 * quad.z + modelviewprojection.m13 * quad.w;
    result.z = modelviewprojection.m2 * quad.x + modelviewprojection.m6 * quad.y + modelviewprojection.m10 * quad.z + modelviewprojection.m14 * quad.w;
    result.w = modelviewprojection.m3 * quad.x + modelviewprojection.m7 * quad.y + modelviewprojection.m11 * quad.z + modelviewprojection.m15 * quad.w;

    // Invert
    result.w = 1.0f / result.w;

    //if (quat.w == 0.0) return 0;

    worldPoint.x = quat.x * quat.w;
    worldPoint.y = quat.y * quat.w;
    worldPoint.z = quat.z * quat.w;

    return worldPoint;
    */
/*
    // Needed Vectors
    Vector3 normalDeviceCoordinates;
    Quaternion rayClip;
    Quaternion rayEye;
    Vector3 rayWorld;

    // Getting normal device coordinates
    float x = (2.0 * mousePosition.x) / GetScreenWidth() - 1.0;
    float y = 1.0 - (2.0 * mousePosition.y) / GetScreenHeight();
    float z = 1.0;
    normalDeviceCoordinates = (Vector3){ x, y, z };

    // Getting clip vector
    rayClip = (Quaternion){ normalDeviceCoordinates.x, normalDeviceCoordinates.y, -1, 1 };

    Matrix invProjection = projection;
    MatrixInvert(&invProjection);

    rayEye = MatrixQuaternionMultiply(invProjection, rayClip);
    rayEye = (Quaternion){ rayEye.x, rayEye.y, -1, 0 };

    Matrix invModelview = modelview;
    MatrixInvert(&invModelview);

    rayWorld = MatrixVector3Multiply(invModelview, (Vector3){rayEye.x, rayEye.y, rayEye.z} );
    VectorNormalize(&rayWorld);

    return rayWorld;
*/
    return (Vector3){ 0, 0, 0 };
}

// Convert image data to OpenGL texture (returns OpenGL valid Id)
unsigned int rlglLoadTexture(void *data, int width, int height, int textureFormat, int mipmapCount, bool genMipmaps)
{
    glBindTexture(GL_TEXTURE_2D, 0);    // Free any old binding

    GLuint id;
    
    // TODO: Review compressed textures support by OpenGL version
    /* (rlGetVersion() == OPENGL_11)
    if ((textureFormat == COMPRESSED_DXT1_RGB) || (textureFormat == COMPRESSED_DXT3_RGBA) || (textureFormat == COMPRESSED_DXT5_RGBA) ||
        (textureFormat == COMPRESSED_ETC1_RGB8) || (textureFormat == COMPRESSED_ETC2_RGB8) || (textureFormat == COMPRESSED_ETC2_EAC_RGBA8))
    {
        id = 0;
        TraceLog(WARNING, "GPU compressed textures not supported on OpenGL 1.1");
        return id;
    }
    */
    
    glGenTextures(1, &id);              // Generate Pointer to the texture

    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    // NOTE: glTexParameteri does NOT affect texture uploading, just the way it's used!
#if defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so CLAMP_TO_EDGE must be used
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       // Set texture to clamp on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       // Set texture to clamp on y-axis
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
#endif

    bool texIsPOT = false;

    // Check if width and height are power-of-two (POT)
    if (((width > 0) && ((width & (width - 1)) == 0)) && ((height > 0) && ((height & (height - 1)) == 0))) texIsPOT = true;

    if (genMipmaps && !texIsPOT)
    {
        TraceLog(WARNING, "[TEX ID %i] Texture is not power-of-two, mipmaps can not be generated", id);

        genMipmaps = false;
    }

    // TODO: Support mipmaps --> if (mipmapCount > 1)
    
    // If mipmaps are being used, we configure mag-min filters accordingly
    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so only GL_LINEAR or GL_NEAREST can be used
    if (genMipmaps)
    {
        // Trilinear filtering with mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);   // Activate use of mipmaps (must be available)
    }
    else
    {
        // Not using mipmappings
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR
    }

#if defined(GRAPHICS_API_OPENGL_11)
    if (genMipmaps)
    {
        TraceLog(WARNING, "[TEX ID %i] Mipmaps generated manually on CPU side", id);

        // Compute required mipmaps
        // NOTE: data size is reallocated to fit mipmaps data
        int mipmapCount = GenerateMipmaps(data, width, height);

        int offset = 0;
        int size = 0;

        int mipWidth = width;
        int mipHeight = height;

        // Load the mipmaps
        for (int level = 0; level < mipmapCount; level++)
        {
            glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, mipWidth, mipHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data + offset);

            size = mipWidth*mipHeight*4;
            offset += size;

            mipWidth /= 2;
            mipHeight /= 2;
        }
    }
    else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
#endif

#if defined(GRAPHICS_API_OPENGL_33)
    // NOTE: We define internal (GPU) format as GL_RGBA8 (probably BGRA8 in practice, driver takes care)
    // NOTE: On embedded systems, we let the driver choose the best internal format

    // Support for multiple color modes (16bit color modes and grayscale)
    // (sized)internalFormat    format          type
    // GL_R                     GL_RED      GL_UNSIGNED_BYTE
    // GL_RGB565                GL_RGB      GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5
    // GL_RGB5_A1               GL_RGBA     GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_5_5_1
    // GL_RGBA4                 GL_RGBA     GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_4_4_4_4
    // GL_RGBA8                 GL_RGBA     GL_UNSIGNED_BYTE
    // GL_RGB8                  GL_RGB      GL_UNSIGNED_BYTE
    
    switch (textureFormat)
    {
        case UNCOMPRESSED_GRAYSCALE:
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, (unsigned char *)data);
            
            // With swizzleMask we define how a one channel texture will be mapped to RGBA
            // Required GL >= 3.3 or EXT_texture_swizzle/ARB_texture_swizzle
            GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, 1.0f };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            
            TraceLog(INFO, "Grayscale texture loaded and swizzled!");
        } break;
        case UNCOMPRESSED_R5G6B5: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (unsigned short *)data); break;
        case UNCOMPRESSED_R8G8B8: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (unsigned char *)data); break;
        case UNCOMPRESSED_R5G5B5A1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, (unsigned short *)data); break;
        case UNCOMPRESSED_R4G4B4A4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, (unsigned short *)data); break;
        case UNCOMPRESSED_R8G8B8A8: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)data); break;
        case COMPRESSED_DXT1_RGB: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGB_S3TC_DXT1_EXT); break;
        case COMPRESSED_DXT1_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT); break;
        case COMPRESSED_DXT3_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT); break;
        case COMPRESSED_DXT5_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT); break;
        case COMPRESSED_ETC1_RGB: TraceLog(WARNING, "ETC compression not supported"); break;   // NOTE: Requires OpenGL ES 2.0 or OpenGL 4.3
        case COMPRESSED_ETC2_RGB: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGB8_ETC2); break;//TraceLog(WARNING, "ETC compression not supported"); break;   // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case COMPRESSED_ETC2_EAC_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA8_ETC2_EAC); break;//TraceLog(WARNING, "ETC compression not supported"); break;  // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        //case COMPRESSED_ASTC_RGBA_4x4: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_ASTC_4x4_KHR); break; // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
        default: TraceLog(WARNING, "Texture format not recognized"); break;
    }

    if ((mipmapCount == 1) && (genMipmaps))
    {
        glGenerateMipmap(GL_TEXTURE_2D);  // Generate mipmaps automatically
        TraceLog(INFO, "[TEX ID %i] Mipmaps generated automatically for new texture", id);
    }
#elif defined(GRAPHICS_API_OPENGL_ES2)

    // NOTE: on OpenGL ES 2.0 (WebGL), internalFormat must match format and options allowed are: GL_LUMINANCE, GL_RGB, GL_RGBA
    switch (textureFormat)
    {
        case UNCOMPRESSED_GRAYSCALE: glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, (unsigned char *)data); break;
        case UNCOMPRESSED_R5G6B5: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (unsigned short *)data); break;
        case UNCOMPRESSED_R8G8B8: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, (unsigned char *)data); break;
        case UNCOMPRESSED_R5G5B5A1: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, (unsigned short *)data); break;
        case UNCOMPRESSED_R4G4B4A4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, (unsigned short *)data); break;
        case UNCOMPRESSED_R8G8B8A8: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)data); break;
        case COMPRESSED_DXT1_RGB: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGB_S3TC_DXT1_EXT); break;
        case COMPRESSED_DXT1_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGB_S3TC_DXT1_EXT); break;
        case COMPRESSED_DXT3_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT); break;     // NOTE: Not supported by WebGL
        case COMPRESSED_DXT5_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT); break;     // NOTE: Not supported by WebGL
        case COMPRESSED_ETC1_RGB: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_ETC1_RGB8_OES); break;
        case COMPRESSED_ETC2_RGB: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGB8_ETC2); break;   // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case COMPRESSED_ETC2_EAC_RGBA: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA8_ETC2_EAC); break;  // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        //case COMPRESSED_ASTC_RGBA_4x4: LoadCompressedTexture((unsigned char *)data, width, height, mipmapCount, GL_COMPRESSED_RGBA_ASTC_4x4_KHR); break; // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
        default: TraceLog(WARNING, "Texture format not recognized"); break;
    }

    if ((mipmapCount == 1) && (genMipmaps))
    {
        glGenerateMipmap(GL_TEXTURE_2D);  // Generate mipmaps automatically
        TraceLog(INFO, "[TEX ID %i] Mipmaps generated automatically for new texture", id);
    }
#endif

    // At this point we have the image converted to texture and uploaded to GPU

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    TraceLog(INFO, "[TEX ID %i] Texture created successfully (%ix%i)", id, width, height);

    return id;
}

// Load vertex data into a VAO (if supported) and VBO
Model rlglLoadModel(VertexData mesh)
{
    Model model;

    model.mesh = mesh;
    model.transform = MatrixIdentity();

#if defined(GRAPHICS_API_OPENGL_11)
    model.mesh.vaoId = 0;       // Vertex Array Object
    model.mesh.vboId[0] = 0;    // Vertex position VBO
    model.mesh.vboId[1] = 0;    // Texcoords VBO
    model.mesh.vboId[2] = 0;    // Normals VBO
    model.texture.id = 0;       // No texture required
    model.shader.id = 0;        // No shader used

#elif defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    model.texture.id = whiteTexture;     // Default whiteTexture
    model.texture.width = 1;     // Default whiteTexture width
    model.texture.height = 1;    // Default whiteTexture height

    GLuint vaoModel = 0;         // Vertex Array Objects (VAO)
    GLuint vertexBuffer[3];      // Vertex Buffer Objects (VBO)

    if (vaoSupported)
    {
        // Initialize Quads VAO (Buffer A)
        glGenVertexArrays(1, &vaoModel);
        glBindVertexArray(vaoModel);
    }

    // Create buffers for our vertex data (positions, texcoords, normals)
    glGenBuffers(3, vertexBuffer);

    // Enable vertex attributes: position
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh.vertexCount, mesh.vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(simpleShader.vertexLoc);
    glVertexAttribPointer(simpleShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);

    // Enable vertex attributes: texcoords
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh.vertexCount, mesh.texcoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(simpleShader.texcoordLoc);
    glVertexAttribPointer(simpleShader.texcoordLoc, 2, GL_FLOAT, 0, 0, 0);

    // Enable vertex attributes: normals
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh.vertexCount, mesh.normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(simpleShader.normalLoc);
    glVertexAttribPointer(simpleShader.normalLoc, 3, GL_FLOAT, 0, 0, 0);

    model.shader = simpleShader;    // By default, simple shader will be used

    model.mesh.vboId[0] = vertexBuffer[0];     // Vertex position VBO
    model.mesh.vboId[1] = vertexBuffer[1];     // Texcoords VBO
    model.mesh.vboId[2] = vertexBuffer[2];     // Normals VBO

    if (vaoSupported)
    {
        if (vaoModel > 0)
        {
            model.mesh.vaoId = vaoModel;
            TraceLog(INFO, "[VAO ID %i] Model uploaded successfully to VRAM (GPU)", vaoModel);
        }
        else TraceLog(WARNING, "Model could not be uploaded to VRAM (GPU)");
    }
    else
    {
        TraceLog(INFO, "[VBO ID %i][VBO ID %i][VBO ID %i] Model uploaded successfully to VRAM (GPU)", model.mesh.vboId[0], model.mesh.vboId[1], model.mesh.vboId[2]);
    }
#endif

    return model;
}


#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Load a shader (vertex shader + fragment shader) from text data
unsigned int rlglLoadShaderFromText(char *vShaderStr, char *fShaderStr)
{
    unsigned int program;
    GLuint vertexShader;
    GLuint fragmentShader;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *pvs = vShaderStr;
    const char *pfs = fShaderStr;

    glShaderSource(vertexShader, 1, &pvs, NULL);
    glShaderSource(fragmentShader, 1, &pfs, NULL);

    GLint success = 0;

    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE)
    {
        TraceLog(WARNING, "[VSHDR ID %i] Failed to compile vertex shader...", vertexShader);

        int maxLength = 0;
        int length;

        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        char log[maxLength];

        glGetShaderInfoLog(vertexShader, maxLength, &length, log);

        TraceLog(INFO, "%s", log);
    }
    else TraceLog(INFO, "[VSHDR ID %i] Vertex shader compiled successfully", vertexShader);

    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (success != GL_TRUE)
    {
        TraceLog(WARNING, "[FSHDR ID %i] Failed to compile fragment shader...", fragmentShader);

        int maxLength = 0;
        int length;

        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        char log[maxLength];

        glGetShaderInfoLog(fragmentShader, maxLength, &length, log);

        TraceLog(INFO, "%s", log);
    }
    else TraceLog(INFO, "[FSHDR ID %i] Fragment shader compiled successfully", fragmentShader);

    program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TraceLog(WARNING, "[SHDR ID %i] Failed to link shader program...", program);

        int maxLength = 0;
        int length;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        char log[maxLength];

        glGetProgramInfoLog(program, maxLength, &length, log);

        TraceLog(INFO, "%s", log);

        glDeleteProgram(program);

        program = 0;
    }
    else TraceLog(INFO, "[SHDR ID %i] Shader program loaded successfully", program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
#endif

// Read screen pixel data (color buffer)
unsigned char *rlglReadScreenPixels(int width, int height)
{
    unsigned char *screenData = (unsigned char *)malloc(width * height * sizeof(unsigned char) * 4);

    // NOTE: glReadPixels returns image flipped vertically -> (0,0) is the bottom left corner of the framebuffer
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);

    // Flip image vertically!
    unsigned char *imgData = (unsigned char *)malloc(width * height * sizeof(unsigned char) * 4);

    for (int y = height-1; y >= 0; y--)
    {
        for (int x = 0; x < (width*4); x++)
        {
            imgData[x + (height - y - 1)*width*4] = screenData[x + (y*width*4)];
        }
    }

    free(screenData);

    return imgData;     // NOTE: image data should be freed
}

// Load a shader (vertex shader + fragment shader) from files
Shader rlglLoadShader(char *vsFileName, char *fsFileName)
{
    Shader shader;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Shaders loading from external text file
    char *vShaderStr = TextFileRead(vsFileName);
    char *fShaderStr = TextFileRead(fsFileName);

    shader.id = rlglLoadShaderFromText(vShaderStr, fShaderStr);

    if (shader.id != 0) TraceLog(INFO, "[SHDR ID %i] Custom shader loaded successfully", shader.id);
    else TraceLog(WARNING, "[SHDR ID %i] Custom shader could not be loaded", shader.id);

    // Shader strings must be freed
    free(vShaderStr);
    free(fShaderStr);

    // Get handles to GLSL input attibute locations
    //-------------------------------------------------------------------
    shader.vertexLoc = glGetAttribLocation(shader.id, "vertexPosition");
    shader.texcoordLoc = glGetAttribLocation(shader.id, "vertexTexCoord");
    shader.normalLoc = glGetAttribLocation(shader.id, "vertexNormal");
    // NOTE: custom shader does not use colorLoc

    // Get handles to GLSL uniform locations (vertex shader)
    shader.modelviewLoc  = glGetUniformLocation(shader.id, "modelviewMatrix");
    shader.projectionLoc = glGetUniformLocation(shader.id, "projectionMatrix");

    // Get handles to GLSL uniform locations (fragment shader)
    shader.textureLoc = glGetUniformLocation(shader.id, "texture0");
    shader.tintColorLoc = glGetUniformLocation(shader.id, "tintColor");
    //--------------------------------------------------------------------
#endif

    return shader;
}

// Link shader to model
void rlglSetModelShader(Model *model, Shader shader)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    model->shader = shader;

    if (vaoSupported) glBindVertexArray(model->mesh.vaoId);

    // Enable vertex attributes: position
    glBindBuffer(GL_ARRAY_BUFFER, model->mesh.vboId[0]);
    glEnableVertexAttribArray(shader.vertexLoc);
    glVertexAttribPointer(shader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);

    // Enable vertex attributes: texcoords
    glBindBuffer(GL_ARRAY_BUFFER, model->mesh.vboId[1]);
    glEnableVertexAttribArray(shader.texcoordLoc);
    glVertexAttribPointer(shader.texcoordLoc, 2, GL_FLOAT, 0, 0, 0);

    // Enable vertex attributes: normals
    glBindBuffer(GL_ARRAY_BUFFER, model->mesh.vboId[2]);
    glEnableVertexAttribArray(shader.normalLoc);
    glVertexAttribPointer(shader.normalLoc, 3, GL_FLOAT, 0, 0, 0);

    if (vaoSupported) glBindVertexArray(0);     // Unbind VAO
#endif
}

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
void PrintProjectionMatrix()
{
    PrintMatrix(projection);
}

void PrintModelviewMatrix()
{
    PrintMatrix(modelview);
}
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Convert image data to OpenGL texture (returns OpenGL valid Id)
// NOTE: Expected compressed image data and POT image
static void LoadCompressedTexture(unsigned char *data, int width, int height, int mipmapCount, int compressedFormat)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int blockSize = 0;      // Bytes every block
    int offset = 0;

    if ((compressedFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ||
        (compressedFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
#if defined(GRAPHICS_API_OPENGL_ES2)
        (compressedFormat == GL_ETC1_RGB8_OES) ||
#endif
        (compressedFormat == GL_COMPRESSED_RGB8_ETC2)) blockSize = 8;
    else blockSize = 16;

    // Load the mipmap levels
    for (int level = 0; level < mipmapCount && (width || height); level++)
    {
        unsigned int size = 0;
        
        size = ((width + 3)/4)*((height + 3)/4)*blockSize;

        glCompressedTexImage2D(GL_TEXTURE_2D, level, compressedFormat, width, height, 0, size, data + offset);

        offset += size;
        width  /= 2;
        height /= 2;

        // Security check for NPOT textures
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }
}

// Load Shader (Vertex and Fragment)
// NOTE: This shader program is used only for batch buffers (lines, triangles, quads)
static Shader LoadDefaultShader(void)
{
    Shader shader;

    // NOTE: Shaders are written using GLSL 110 (desktop), that is equivalent to GLSL 100 on ES2
    // NOTE: Detected an error on ATI cards if defined #version 110 while OpenGL 3.3+
    // Just defined #version 330 despite shader is #version 110

    // Vertex shader directly defined, no external file required
#if defined(GRAPHICS_API_OPENGL_33)
    char vShaderStr[] = " #version 110      \n"     // NOTE: Actually, #version 110 (quivalent to #version 100 on ES2)
#elif defined(GRAPHICS_API_OPENGL_ES2)
    char vShaderStr[] = " #version 100      \n"     // NOTE: Must be defined this way! 110 doesn't work!
#endif
        "uniform mat4 projectionMatrix;     \n"
        "uniform mat4 modelviewMatrix;      \n"
        "attribute vec3 vertexPosition;     \n"
        "attribute vec2 vertexTexCoord;     \n"
        "attribute vec4 vertexColor;        \n"
        "varying vec2 fragTexCoord;         \n"
        "varying vec4 fragColor;            \n"
        "void main()                        \n"
        "{                                  \n"
        "    fragTexCoord = vertexTexCoord; \n"
        "    fragColor = vertexColor;       \n"
        "    gl_Position = projectionMatrix * modelviewMatrix * vec4(vertexPosition, 1.0); \n"
        "}                                  \n";

    // Fragment shader directly defined, no external file required
#if defined(GRAPHICS_API_OPENGL_33)
    char fShaderStr[] = " #version 110      \n"     // NOTE: Actually, #version 110 (quivalent to #version 100 on ES2)
#elif defined(GRAPHICS_API_OPENGL_ES2)
    char fShaderStr[] = " #version 100      \n"     // NOTE: Must be defined this way! 110 doesn't work!
        "precision mediump float;           \n"     // WebGL, required for emscripten
#endif
        "uniform sampler2D texture0;        \n"
        "varying vec2 fragTexCoord;         \n"
        "varying vec4 fragColor;            \n"
        "void main()                        \n"
        "{                                  \n"
        "    gl_FragColor = texture2D(texture0, fragTexCoord) * fragColor; \n"
        "}                                  \n";

    shader.id = rlglLoadShaderFromText(vShaderStr, fShaderStr);

    if (shader.id != 0) TraceLog(INFO, "[SHDR ID %i] Default shader loaded successfully", shader.id);
    else TraceLog(WARNING, "[SHDR ID %i] Default shader could not be loaded", shader.id);

    // Get handles to GLSL input attibute locations
    //-------------------------------------------------------------------
    shader.vertexLoc = glGetAttribLocation(shader.id, "vertexPosition");
    shader.texcoordLoc = glGetAttribLocation(shader.id, "vertexTexCoord");
    shader.colorLoc = glGetAttribLocation(shader.id, "vertexColor");
    // NOTE: default shader does not use normalLoc

    // Get handles to GLSL uniform locations (vertex shader)
    shader.modelviewLoc = glGetUniformLocation(shader.id, "modelviewMatrix");
    shader.projectionLoc = glGetUniformLocation(shader.id, "projectionMatrix");

    // Get handles to GLSL uniform locations (fragment shader)
    shader.textureLoc = glGetUniformLocation(shader.id, "texture0");
    //--------------------------------------------------------------------

    return shader;
}

// Load Simple Shader (Vertex and Fragment)
// NOTE: This shader program is used to render models
static Shader LoadSimpleShader(void)
{
    Shader shader;

    // NOTE: Shaders are written using GLSL 110 (desktop), that is equivalent to GLSL 100 on ES2
    // NOTE: Detected an error on ATI cards if defined #version 110 while OpenGL 3.3+
    // Just defined #version 330 despite shader is #version 110

    // Vertex shader directly defined, no external file required
#if defined(GRAPHICS_API_OPENGL_33)
    char vShaderStr[] = " #version 330      \n"     // NOTE: Actually, #version 110 (quivalent to #version 100 on ES2)
#elif defined(GRAPHICS_API_OPENGL_ES2)
    char vShaderStr[] = " #version 100      \n"     // NOTE: Must be defined this way! 110 doesn't work!
#endif
        "uniform mat4 projectionMatrix;     \n"
        "uniform mat4 modelviewMatrix;      \n"
        "attribute vec3 vertexPosition;     \n"
        "attribute vec2 vertexTexCoord;     \n"
        "attribute vec3 vertexNormal;       \n"
        "varying vec2 fragTexCoord;         \n"
        "void main()                        \n"
        "{                                  \n"
        "    fragTexCoord = vertexTexCoord; \n"
        "    gl_Position = projectionMatrix * modelviewMatrix * vec4(vertexPosition, 1.0); \n"
        "}                                  \n";

    // Fragment shader directly defined, no external file required
#if defined(GRAPHICS_API_OPENGL_33)
    char fShaderStr[] = " #version 330      \n"     // NOTE: Actually, #version 110 (quivalent to #version 100 on ES2)
#elif defined(GRAPHICS_API_OPENGL_ES2)
    char fShaderStr[] = " #version 100      \n"     // NOTE: Must be defined this way! 110 doesn't work!
        "precision mediump float;           \n"     // precision required for OpenGL ES2 (WebGL)
#endif
        "uniform sampler2D texture0;        \n"
        "varying vec2 fragTexCoord;         \n"
        "uniform vec4 tintColor;            \n"
        "void main()                        \n"
        "{                                  \n"
        "    gl_FragColor = texture2D(texture0, fragTexCoord) * tintColor; \n"
        "}                                  \n";

    shader.id = rlglLoadShaderFromText(vShaderStr, fShaderStr);

    if (shader.id != 0) TraceLog(INFO, "[SHDR ID %i] Simple shader loaded successfully", shader.id);
    else TraceLog(WARNING, "[SHDR ID %i] Simple shader could not be loaded", shader.id);

    // Get handles to GLSL input attibute locations
    //-------------------------------------------------------------------
    shader.vertexLoc = glGetAttribLocation(shader.id, "vertexPosition");
    shader.texcoordLoc = glGetAttribLocation(shader.id, "vertexTexCoord");
    shader.normalLoc = glGetAttribLocation(shader.id, "vertexNormal");
    // NOTE: simple shader does not use colorLoc

    // Get handles to GLSL uniform locations (vertex shader)
    shader.modelviewLoc  = glGetUniformLocation(shader.id, "modelviewMatrix");
    shader.projectionLoc = glGetUniformLocation(shader.id, "projectionMatrix");

    // Get handles to GLSL uniform locations (fragment shader)
    shader.textureLoc = glGetUniformLocation(shader.id, "texture0");
    shader.tintColorLoc = glGetUniformLocation(shader.id, "tintColor");
    //--------------------------------------------------------------------

    return shader;
}

// Read text file
// NOTE: text chars array should be freed manually
static char *TextFileRead(char *fileName)
{
    FILE *textFile;
    char *text = NULL;

    int count = 0;

    if (fileName != NULL)
    {
        textFile = fopen(fileName,"rt");

        if (textFile != NULL)
        {
            fseek(textFile, 0, SEEK_END);
            count = ftell(textFile);
            rewind(textFile);

            if (count > 0)
            {
                text = (char *)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, textFile);
                text[count] = '\0';
            }

            fclose(textFile);
        }
        else TraceLog(WARNING, "[%s] Text file could not be opened", fileName);
    }

    return text;
}

// Allocate and initialize float array buffers to store vertex data (lines, triangles, quads)
static void InitializeBuffers(void)
{
    // Initialize lines arrays (vertex position and color data)
    lines.vertices = (float *)malloc(sizeof(float)*3*2*MAX_LINES_BATCH);        // 3 float by vertex, 2 vertex by line
    lines.colors = (unsigned char *)malloc(sizeof(unsigned char)*4*2*MAX_LINES_BATCH);  // 4 float by color, 2 colors by line

    for (int i = 0; i < (3*2*MAX_LINES_BATCH); i++) lines.vertices[i] = 0.0f;
    for (int i = 0; i < (4*2*MAX_LINES_BATCH); i++) lines.colors[i] = 0;

    lines.vCounter = 0;
    lines.cCounter = 0;

    // Initialize triangles arrays (vertex position and color data)
    triangles.vertices = (float *)malloc(sizeof(float)*3*3*MAX_TRIANGLES_BATCH);        // 3 float by vertex, 3 vertex by triangle
    triangles.colors = (unsigned char *)malloc(sizeof(unsigned char)*4*3*MAX_TRIANGLES_BATCH);  // 4 float by color, 3 colors by triangle

    for (int i = 0; i < (3*3*MAX_TRIANGLES_BATCH); i++) triangles.vertices[i] = 0.0f;
    for (int i = 0; i < (4*3*MAX_TRIANGLES_BATCH); i++) triangles.colors[i] = 0;

    triangles.vCounter = 0;
    triangles.cCounter = 0;

    // Initialize quads arrays (vertex position, texcoord and color data... and indexes)
    quads.vertices = (float *)malloc(sizeof(float)*3*4*MAX_QUADS_BATCH);        // 3 float by vertex, 4 vertex by quad
    quads.texcoords = (float *)malloc(sizeof(float)*2*4*MAX_QUADS_BATCH);       // 2 float by texcoord, 4 texcoord by quad
    quads.colors = (unsigned char *)malloc(sizeof(unsigned char)*4*4*MAX_QUADS_BATCH);  // 4 float by color, 4 colors by quad
#if defined(GRAPHICS_API_OPENGL_33)
    quads.indices = (unsigned int *)malloc(sizeof(int)*6*MAX_QUADS_BATCH);      // 6 int by quad (indices)
#elif defined(GRAPHICS_API_OPENGL_ES2)
    quads.indices = (unsigned short *)malloc(sizeof(short)*6*MAX_QUADS_BATCH);  // 6 int by quad (indices)
#endif


    for (int i = 0; i < (3*4*MAX_QUADS_BATCH); i++) quads.vertices[i] = 0.0f;
    for (int i = 0; i < (2*4*MAX_QUADS_BATCH); i++) quads.texcoords[i] = 0.0f;
    for (int i = 0; i < (4*4*MAX_QUADS_BATCH); i++) quads.colors[i] = 0;

    int k = 0;

    // Indices can be initialized right now
    for (int i = 0; i < (6*MAX_QUADS_BATCH); i+=6)
    {
        quads.indices[i] = 4*k;
        quads.indices[i+1] = 4*k+1;
        quads.indices[i+2] = 4*k+2;
        quads.indices[i+3] = 4*k;
        quads.indices[i+4] = 4*k+2;
        quads.indices[i+5] = 4*k+3;

        k++;
    }

    quads.vCounter = 0;
    quads.tcCounter = 0;
    quads.cCounter = 0;

    TraceLog(INFO, "CPU buffers (lines, triangles, quads) initialized successfully");
}

// Initialize Vertex Array Objects (Contain VBO)
// NOTE: lines, triangles and quads buffers use defaultShader
static void InitializeBuffersGPU(void)
{
    if (vaoSupported)
    {
        // Initialize Lines VAO
        glGenVertexArrays(1, &vaoLines);
        glBindVertexArray(vaoLines);
    }

    // Create buffers for our vertex data
    glGenBuffers(2, linesBuffer);

    // Lines - Vertex positions buffer binding and attributes enable
    glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*2*MAX_LINES_BATCH, lines.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.vertexLoc);
    glVertexAttribPointer(defaultShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);

    // Lines - colors buffer
    glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*2*MAX_LINES_BATCH, lines.colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.colorLoc);
    glVertexAttribPointer(defaultShader.colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

    if (vaoSupported) TraceLog(INFO, "[VAO ID %i] Lines VAO initialized successfully", vaoLines);
    else TraceLog(INFO, "[VBO ID %i][VBO ID %i] Lines VBOs initialized successfully", linesBuffer[0], linesBuffer[1]);
    //--------------------------------------------------------------

    if (vaoSupported)
    {
        // Initialize Triangles VAO
        glGenVertexArrays(1, &vaoTriangles);
        glBindVertexArray(vaoTriangles);
    }

    // Create buffers for our vertex data
    glGenBuffers(2, trianglesBuffer);

    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*3*MAX_TRIANGLES_BATCH, triangles.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.vertexLoc);
    glVertexAttribPointer(defaultShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*3*MAX_TRIANGLES_BATCH, triangles.colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.colorLoc);
    glVertexAttribPointer(defaultShader.colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

    if (vaoSupported) TraceLog(INFO, "[VAO ID %i] Triangles VAO initialized successfully", vaoTriangles);
    else TraceLog(INFO, "[VBO ID %i][VBO ID %i] Triangles VBOs initialized successfully", trianglesBuffer[0], trianglesBuffer[1]);
    //--------------------------------------------------------------

    if (vaoSupported)
    {
        // Initialize Quads VAO
        glGenVertexArrays(1, &vaoQuads);
        glBindVertexArray(vaoQuads);
    }

    // Create buffers for our vertex data
    glGenBuffers(4, quadsBuffer);

    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*MAX_QUADS_BATCH, quads.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.vertexLoc);
    glVertexAttribPointer(defaultShader.vertexLoc, 3, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*MAX_QUADS_BATCH, quads.texcoords, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.texcoordLoc);
    glVertexAttribPointer(defaultShader.texcoordLoc, 2, GL_FLOAT, 0, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*4*MAX_QUADS_BATCH, quads.colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(defaultShader.colorLoc);
    glVertexAttribPointer(defaultShader.colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

    // Fill index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadsBuffer[3]);
#if defined(GRAPHICS_API_OPENGL_33)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6*MAX_QUADS_BATCH, quads.indices, GL_STATIC_DRAW);
#elif defined(GRAPHICS_API_OPENGL_ES2)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(short)*6*MAX_QUADS_BATCH, quads.indices, GL_STATIC_DRAW);
#endif

    if (vaoSupported) TraceLog(INFO, "[VAO ID %i] Quads VAO initialized successfully", vaoQuads);
    else TraceLog(INFO, "[VBO ID %i][VBO ID %i][VBO ID %i][VBO ID %i] Quads VBOs initialized successfully", quadsBuffer[0], quadsBuffer[1], quadsBuffer[2], quadsBuffer[3]);

    // Unbind the current VAO
    if (vaoSupported) glBindVertexArray(0);
}

// Update VBOs with vertex array data
// NOTE: If there is not vertex data, buffers doesn't need to be updated (vertexCount > 0)
// TODO: If no data changed on the CPU arrays --> No need to update GPU arrays every frame!
static void UpdateBuffers(void)
{
    if (lines.vCounter > 0)
    {
        // Activate Lines VAO
        if (vaoSupported) glBindVertexArray(vaoLines);

        // Lines - vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[0]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*2*MAX_LINES_BATCH, lines.vertices, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*lines.vCounter, lines.vertices);    // target - offset (in bytes) - size (in bytes) - data pointer

        // Lines - colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[1]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*2*MAX_LINES_BATCH, lines.colors, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(unsigned char)*4*lines.cCounter, lines.colors);
    }
    //--------------------------------------------------------------

    if (triangles.vCounter > 0)
    {
        // Activate Triangles VAO
        if (vaoSupported) glBindVertexArray(vaoTriangles);

        // Triangles - vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[0]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*3*MAX_TRIANGLES_BATCH, triangles.vertices, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*triangles.vCounter, triangles.vertices);

        // Triangles - colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[1]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*3*MAX_TRIANGLES_BATCH, triangles.colors, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(unsigned char)*4*triangles.cCounter, triangles.colors);
    }
    //--------------------------------------------------------------

    if (quads.vCounter > 0)
    {
        // Activate Quads VAO
        if (vaoSupported) glBindVertexArray(vaoQuads);

        // Quads - vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[0]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*MAX_QUADS_BATCH, quads.vertices, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*quads.vCounter, quads.vertices);

        // Quads - texture coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[1]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*MAX_QUADS_BATCH, quads.texcoords, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*2*quads.vCounter, quads.texcoords);

        // Quads - colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[2]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*MAX_QUADS_BATCH, quads.colors, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(unsigned char)*4*quads.vCounter, quads.colors);

        // Another option would be using buffer mapping...
        //triangles.vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
        // Now we can modify vertices
        //glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    //--------------------------------------------------------------

    // Unbind the current VAO
    if (vaoSupported) glBindVertexArray(0);
}

#endif //defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)

#if defined(GRAPHICS_API_OPENGL_11)

// Mipmaps data is generated after image data
static int GenerateMipmaps(unsigned char *data, int baseWidth, int baseHeight)
{
    int mipmapCount = 1;                // Required mipmap levels count (including base level)
    int width = baseWidth;
    int height = baseHeight;
    int size = baseWidth*baseHeight*4;  // Size in bytes (will include mipmaps...)

    // Count mipmap levels required
    while ((width != 1) && (height != 1))
    {
        if (width != 1) width /= 2;
        if (height != 1) height /= 2;

        TraceLog(DEBUG, "Next mipmap size: %i x %i", width, height);

        mipmapCount++;

        size += (width*height*4);       // Add mipmap size (in bytes)
    }

    TraceLog(DEBUG, "Total mipmaps required: %i", mipmapCount);
    TraceLog(DEBUG, "Total size of data required: %i", size);

    unsigned char *temp = realloc(data, size);

    if (temp != NULL) data = temp;
    else TraceLog(WARNING, "Mipmaps required memory could not be allocated");

    width = baseWidth;
    height = baseHeight;
    size = (width*height*4);

    // Generate mipmaps
    // NOTE: Every mipmap data is stored after data
    pixel *image = (pixel *)malloc(width*height*sizeof(pixel));
    pixel *mipmap = NULL;
    int offset = 0;
    int j = 0;

    for (int i = 0; i < size; i += 4)
    {
        image[j].r = data[i];
        image[j].g = data[i + 1];
        image[j].b = data[i + 2];
        image[j].a = data[i + 3];
        j++;
    }

    TraceLog(DEBUG, "Mipmap base (%ix%i)", width, height);

    for (int mip = 1; mip < mipmapCount; mip++)
    {
        mipmap = GenNextMipmap(image, width, height);

        offset += (width*height*4); // Size of last mipmap
        j = 0;

        width /= 2;
        height /= 2;
        size = (width*height*4);    // Mipmap size to store after offset

        // Add mipmap to data
        for (int i = 0; i < size; i += 4)
        {
            data[offset + i] = mipmap[j].r;
            data[offset + i + 1] = mipmap[j].g;
            data[offset + i + 2] = mipmap[j].b;
            data[offset + i + 3] = mipmap[j].a;
            j++;
        }

        free(image);

        image = mipmap;
        mipmap = NULL;
    }

    free(mipmap);       // free mipmap data

    return mipmapCount;
}

// Manual mipmap generation (basic scaling algorithm)
static pixel *GenNextMipmap(pixel *srcData, int srcWidth, int srcHeight)
{
    int x2, y2;
    pixel prow, pcol;

    int width = srcWidth / 2;
    int height = srcHeight / 2;

    pixel *mipmap = (pixel *)malloc(width*height*sizeof(pixel));

    // Scaling algorithm works perfectly (box-filter)
    for (int y = 0; y < height; y++)
    {
        y2 = 2 * y;

        for (int x = 0; x < width; x++)
        {
            x2 = 2 * x;

            prow.r = (srcData[y2*srcWidth + x2].r + srcData[y2*srcWidth + x2 + 1].r)/2;
            prow.g = (srcData[y2*srcWidth + x2].g + srcData[y2*srcWidth + x2 + 1].g)/2;
            prow.b = (srcData[y2*srcWidth + x2].b + srcData[y2*srcWidth + x2 + 1].b)/2;
            prow.a = (srcData[y2*srcWidth + x2].a + srcData[y2*srcWidth + x2 + 1].a)/2;

            pcol.r = (srcData[(y2+1)*srcWidth + x2].r + srcData[(y2+1)*srcWidth + x2 + 1].r)/2;
            pcol.g = (srcData[(y2+1)*srcWidth + x2].g + srcData[(y2+1)*srcWidth + x2 + 1].g)/2;
            pcol.b = (srcData[(y2+1)*srcWidth + x2].b + srcData[(y2+1)*srcWidth + x2 + 1].b)/2;
            pcol.a = (srcData[(y2+1)*srcWidth + x2].a + srcData[(y2+1)*srcWidth + x2 + 1].a)/2;

            mipmap[y*width + x].r = (prow.r + pcol.r)/2;
            mipmap[y*width + x].g = (prow.g + pcol.g)/2;
            mipmap[y*width + x].b = (prow.b + pcol.b)/2;
            mipmap[y*width + x].a = (prow.a + pcol.a)/2;
        }
    }

    TraceLog(DEBUG, "Mipmap generated successfully (%ix%i)", width, height);

    return mipmap;
}
#endif

#if defined(RLGL_STANDALONE)

typedef enum { INFO = 0, ERROR, WARNING, DEBUG, OTHER } TraceLogType;

// Output a trace log message
// NOTE: Expected msgType: (0)Info, (1)Error, (2)Warning
void TraceLog(int msgType, const char *text, ...)
{
    va_list args;
    va_start(args, text);

    switch(msgType)
    {
        case INFO: fprintf(stdout, "INFO: "); break;
        case ERROR: fprintf(stdout, "ERROR: "); break;
        case WARNING: fprintf(stdout, "WARNING: "); break;
        case DEBUG: fprintf(stdout, "DEBUG: "); break;
        default: break;
    }

    vfprintf(stdout, text, args);
    fprintf(stdout, "\n");

    va_end(args);

    if (msgType == ERROR) exit(1);
}
#endif