/*********************************************************************************************
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

#include "raymath.h"        // Required for data type Matrix and Matrix functions

#ifdef USE_OPENGL_11
    #include <GL/gl.h>      // Extensions loading lib
#endif

#ifdef USE_OPENGL_33
    #define GLEW_STATIC         
    #include <GL/glew.h>    // Extensions loading lib
#endif

//#include "glad.h"         // Extensions loading lib? --> REVIEW

#define USE_VBO_DOUBLE_BUFFERS    // Enable VBO double buffers usage --> REVIEW!

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MATRIX_STACK_SIZE          16   // Matrix stack max size
#define MAX_DRAWS_BY_TEXTURE      256   // Draws are organized by texture changes
#define TEMP_VERTEX_BUFFER_SIZE  1024   // Temporal Vertex Buffer (required for vertex-transformations)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    int numQuads;
    int texId;
} QuadsByTexture;

typedef struct {
    int vCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    float *colors;              // 4 components per vertex
} VertexPositionColorBuffer;
/*
typedef struct {
    int vCounter;
    int tcCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    float *colors;              // 4 components per vertex
} VertexPositionColorTextureBuffer;
*/
/*
typedef struct {
    int vCounter;
    int tcCounter;
    int nCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    float *normals;             // 3 components per vertex
} VertexPositionTextureNormalBuffer;
*/
typedef struct {
    int vCounter;
    int tcCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    float *colors;              // 4 components per vertex
    unsigned int *indices;      // 6 indices per quad
} VertexPositionColorTextureIndexBuffer;

typedef struct {
    GLuint texId;
    int firstVertex;    // Actually, when using glDrawElements, this parameter is useless..
    int vCount;
} DrawCall;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
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

// Vetex-Fragment Shader Program ID
static GLuint shaderProgram;       

// Shader program attibutes binding locations
static GLuint vertexLoc, texcoordLoc, colorLoc;
static GLuint projectionMatrixLoc, modelviewMatrixLoc;
static GLuint textureLoc;

// Vertex Array Objects (VAO)
static GLuint vaoLines, vaoTriangles, vaoQuads;

// Vertex Buffer Objects (VBO)
static GLuint linesBuffer[2];
static GLuint trianglesBuffer[2];
static GLuint quadsBuffer[4];

#ifdef USE_VBO_DOUBLE_BUFFERS
// Double buffering
// TODO: REVIEW -> Not getting any performance improvement... why?
static GLuint vaoQuadsB;
static GLuint quadsBufferB[4];
static bool useBufferB = false;
#endif

static DrawCall *draws;
static int drawsCounter;

// Temp vertex buffer to be used with rlTranslate, rlRotate, rlScale
static Vector3 *tempBuffer;
static int tempBufferCount = 0;
static bool useTempBuffer = false;

// White texture useful for plain color polys (required by shader)
static GLuint whiteTexture;
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
static GLuint LoadDefaultShaders();
static void InitializeBuffers();
static void InitializeVAOs();
static void UpdateBuffers();

// Shader files loading (external) - Not used but useful...
static GLuint LoadShaders(char *vertexFileName, char *fragmentFileName);
static char *TextFileRead(char *fn);
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix operations
//----------------------------------------------------------------------------------

#ifdef USE_OPENGL_11

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

void rlPushMatrix() { glPushMatrix(); }
void rlPopMatrix() { glPopMatrix(); }
void rlLoadIdentity() { glLoadIdentity(); }
void rlTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
void rlRotatef(float angleDeg, float x, float y, float z) { glRotatef(angleDeg, x, y, z); }
void rlScalef(float x, float y, float z) { glScalef(x, y, z); }
void rlMultMatrixf(float *mat) { glMultMatrixf(mat); }

#else

// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) currentMatrix = &projection;
    else if (mode == RL_MODELVIEW) currentMatrix = &modelview;
    //else if (mode == GL_TEXTURE) TODO: NEVER USED!
    
    currentMatrixMode = mode;
}

// Push the current matrix to stack
void rlPushMatrix()
{
    if (stackCounter == MATRIX_STACK_SIZE - 1)
    {
        printf("ERROR: Stack Buffer Overflow! (MAX 16 MATRIX)");
        
        exit(1);
    }

    stack[stackCounter] = *currentMatrix;
    rlLoadIdentity();
    stackCounter++;
    
    if (currentMatrixMode == RL_MODELVIEW) useTempBuffer = true;
}

// Pop lattest inserted matrix from stack
void rlPopMatrix()
{
    if (stackCounter > 0)
    {   
        Matrix mat = stack[stackCounter - 1];       
        *currentMatrix = mat;
        stackCounter--;
    }
}

// Reset current matrix to identity matrix
void rlLoadIdentity()
{
    *currentMatrix = MatrixIdentity();
}

// Multiply the current matrix by a translation matrix
void rlTranslatef(float x, float y, float z)
{
    Matrix mat = MatrixTranslate(x, y, z);
    
    *currentMatrix = MatrixMultiply(*currentMatrix, mat);
}

// Multiply the current matrix by a rotation matrix
void rlRotatef(float angleDeg, float x, float y, float z)
{
    // TODO: Rotation matrix --> REVIEW!
    Matrix rot = MatrixIdentity();
    
    if (x == 1) rot = MatrixRotateX(angleDeg*DEG2RAD);
    else if (y == 1) rot = MatrixRotateY(angleDeg*DEG2RAD);
    else if (z == 1) rot = MatrixRotateZ(angleDeg*DEG2RAD);
    
    *currentMatrix = MatrixMultiply(*currentMatrix, rot);
}

// Multiply the current matrix by a scaling matrix
void rlScalef(float x, float y, float z)
{
    Matrix mat = MatrixScale(x, y, z);
    
    *currentMatrix = MatrixMultiply(*currentMatrix, mat);
}

// Multiply the current matrix by another matrix
void rlMultMatrixf(float *m)
{
    // TODO: review Matrix creation from array
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
#ifdef USE_OPENGL_11

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

#else

// Initialize drawing mode (how to organize vertex)
void rlBegin(int mode)
{
    // Draw mode can only be RL_LINES, RL_TRIANGLES and RL_QUADS
    currentDrawMode = mode;
}

// Finish vertex providing
void rlEnd()
{
    if (useTempBuffer)
    {
        // IT WORKS!!! --> Refactor...
        Matrix mat = *currentMatrix;
        MatrixTranspose(&mat);

        // Apply transformation matrix to all temp vertices
        for (int i = 0; i < tempBufferCount; i++) VectorTransform(&tempBuffer[i], mat);
        
        // Deactivate tempBuffer usage to allow rlVertex3f do its job
        useTempBuffer = false;
        
        // Copy all transformed vertices to right VAO
        for (int i = 0; i < tempBufferCount; i++) rlVertex3f(tempBuffer[i].x, tempBuffer[i].y, tempBuffer[i].z);
        
        // Reset temp buffer
        tempBufferCount = 0;
    }

    // Make sure vertexCounter is the same for vertices-texcoords-normals-colors
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
            
            // TODO: Make sure normals count match vertex count
            
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
                lines.vertices[3*lines.vCounter] = x;
                lines.vertices[3*lines.vCounter + 1] = y;
                lines.vertices[3*lines.vCounter + 2] = z;
                
                lines.vCounter++;
                
            } break;
            case RL_TRIANGLES:
            {
                triangles.vertices[3*triangles.vCounter] = x;
                triangles.vertices[3*triangles.vCounter + 1] = y;
                triangles.vertices[3*triangles.vCounter + 2] = z;
                
                triangles.vCounter++;
                
            } break;
            case RL_QUADS:
            {
                quads.vertices[3*quads.vCounter] = x;
                quads.vertices[3*quads.vCounter + 1] = y;
                quads.vertices[3*quads.vCounter + 2] = z;
                
                quads.vCounter++;
                
                draws[drawsCounter - 1].vCount++;
                
            } break;
            default: break;
        }
    }
}

// Define one vertex (position)
void rlVertex2f(float x, float y)
{
    rlVertex3f(x, y, 0.0);
}

// Define one vertex (position)
void rlVertex2i(int x, int y)
{
    rlVertex3f((float)x, (float)y, 0.0);
}

// Define one vertex (texture coordinate)
// NOTE: Texture coordinates are limited to TRIANGLES only
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
void rlColor4f(float x, float y, float z, float w)
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
void rlColor4ub(byte r, byte g, byte b, byte a)
{
    rlColor4f((float)r/255, (float)g/255, (float)b/255, (float)a/255); 
}

// Define one vertex (color)
void rlColor3f(float x, float y, float z)
{
    rlColor4f(x, y, z, 1.0);
}

#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - OpenGL equivalent functions (common to 1.1, 3.3+, ES2)
//----------------------------------------------------------------------------------

// Enable texture usage
void rlEnableTexture(unsigned int id)
{
#ifdef USE_OPENGL_11
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
#endif

#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
    if (draws[drawsCounter - 1].texId != id)
    {
        if (draws[drawsCounter - 1].vCount > 0) drawsCounter++;
        
        draws[drawsCounter - 1].texId = id;
        draws[drawsCounter - 1].firstVertex = draws[drawsCounter - 2].vCount;
        draws[drawsCounter - 1].vCount = 0;
    }
#endif
}

// Disable texture usage
void rlDisableTexture()
{
#ifdef USE_OPENGL_11
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

// Unload texture from GPU memory
void rlDeleteTextures(unsigned int id)
{
    glDeleteTextures(1, &id);
}

// Unload vertex data from GPU memory
void rlDeleteVertexArrays(unsigned int id)
{
#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)
    glDeleteVertexArrays(1, &id);
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
void rlClearScreenBuffers()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers: Color and Depth (Depth is used for 3D)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     // Stencil buffer not used...
}

//----------------------------------------------------------------------------------
// Module Functions Definition - rlgl Functions
//----------------------------------------------------------------------------------

#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)

// Init OpenGL 3.3+ required data
void rlglInit()
{                 
    // Initialize GLEW
    glewExperimental = 1;       // Needed for core profile

    GLenum error = glewInit();
    
    if (error != GLEW_OK) 
    {
        fprintf(stderr, "Failed to initialize GLEW - Error: %s\n", glewGetErrorString(error));
        exit(1);
    }

    if (glewIsSupported("GL_VERSION_3_3")) printf("OpenGL 3.3 initialized\n"); 
/*
    // TODO: GLEW is a big library that loads ALL extensions, maybe using glad we can only load required ones...
    if (!gladLoadGL()) 
    {
        fprintf(stderr, printf("Failed to initialize glad.\n");
        exit(1);
    }
*/    
    // Set default draw mode
    currentDrawMode = RL_TRIANGLES;
    
    // Reset projection and modelview matrices
    projection = MatrixIdentity();
    modelview = MatrixIdentity();
    currentMatrix = &modelview;
    
    // Initialize matrix stack
    for (int i = 0; i < MATRIX_STACK_SIZE; i++) stack[i] = MatrixIdentity();

    // Init default Shader (GLSL 110) -> Common for GL 3.3+ and ES2
    shaderProgram = LoadDefaultShaders();
    //shaderProgram = LoadShaders("simple150.vert", "simple150.frag");
    
    // Get handles to GLSL input vars locations
    vertexLoc = glGetAttribLocation(shaderProgram, "vertexPosition");
    texcoordLoc = glGetAttribLocation(shaderProgram, "vertexTexCoord");
    colorLoc = glGetAttribLocation(shaderProgram, "vertexColor");
    
    // Get handles to GLSL uniform vars locations (vertex-shader)
    modelviewMatrixLoc = glGetUniformLocation(shaderProgram, "modelviewMatrix");
    projectionMatrixLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
    
    // Get handles to GLSL uniform vars locations (fragment-shader)
    textureLoc  = glGetUniformLocation(shaderProgram, "texture0");
    
    printf("Default shaders loaded\n\n");
    
    // Print OpenGL and GLSL version
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("GLSL:     %s\n\n", glGetString(0x8B8C));  //GL_SHADING_LANGUAGE_VERSION
    
    InitializeBuffers();    // Init vertex arrays
    InitializeVAOs();       // Init VBO and VAO
    
    // Init temp vertex buffer, used when transformation required (translate, rotate, scale)
    tempBuffer = (Vector3 *)malloc(sizeof(Vector3)*TEMP_VERTEX_BUFFER_SIZE);
    
    for (int i = 0; i < TEMP_VERTEX_BUFFER_SIZE; i++) tempBuffer[i] = VectorZero();
    
    // Create default white texture for plain colors (required by shader)
    unsigned char pixels[4] = { 255, 255, 255, 255 };   // 1 pixel RGBA (4 bytes)
    
    whiteTexture = rlglLoadTexture(1, 1, pixels);     
    
    // Init draw calls tracking system
    draws = (DrawCall *)malloc(sizeof(DrawCall)*MAX_DRAWS_BY_TEXTURE);
    
    for (int i = 0; i < MAX_DRAWS_BY_TEXTURE; i++)
    {
        draws[i].texId = 0;
        draws[i].firstVertex = 0;
        draws[i].vCount = 0;
    }
    
    drawsCounter = 1;
    draws[drawsCounter - 1].texId = whiteTexture;
}

// Vertex Buffer Object deinitialization (memory free)
void rlglClose()
{
    // Unbind everything
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(0);

    // Delete VAOs and VBOs
    glDeleteBuffers(1, &linesBuffer[0]);
    glDeleteBuffers(1, &linesBuffer[1]);
    glDeleteBuffers(1, &trianglesBuffer[0]);
    glDeleteBuffers(1, &trianglesBuffer[1]);
    glDeleteBuffers(1, &quadsBuffer[0]);
    glDeleteBuffers(1, &quadsBuffer[1]);
    glDeleteBuffers(1, &quadsBuffer[2]);
    glDeleteBuffers(1, &quadsBuffer[3]);

    glDeleteVertexArrays(1, &vaoLines);
    glDeleteVertexArrays(1, &vaoTriangles);
    glDeleteVertexArrays(1, &vaoQuads);

    //glDetachShader(shaderProgram, v);
    //glDetachShader(shaderProgram, f);
    //glDeleteShader(v);
    //glDeleteShader(f);
    glDeleteProgram(shaderProgram);

    // Free vertex arrays memory
    free(lines.vertices);
    free(lines.colors);

    free(triangles.vertices);
    free(triangles.colors);
    
    free(quads.vertices);
    free(quads.texcoords);
    free(quads.colors);
    
    // Free GPU texture
    glDeleteTextures(1, &whiteTexture);
}

void rlglDraw()
{
    glUseProgram(shaderProgram);        // Use our shader
    
    glUniformMatrix4fv(projectionMatrixLoc, 1, false, GetMatrixVector(projection));
    glUniformMatrix4fv(modelviewMatrixLoc, 1, false, GetMatrixVector(modelview));
    glUniform1i(textureLoc, 0);

    UpdateBuffers();

    if (lines.vCounter > 0)
    {
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
    
        glBindVertexArray(vaoLines);
        glDrawArrays(GL_LINES, 0, lines.vCounter);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    if (triangles.vCounter > 0)
    {
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
    
        glBindVertexArray(vaoTriangles);
        glDrawArrays(GL_TRIANGLES, 0, triangles.vCounter);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    if (quads.vCounter > 0)
    {
        int numQuads = 0;
        int numIndicesToProcess = 0;
        int indicesOffset = 0;

#ifdef USE_VBO_DOUBLE_BUFFERS
        // Depending on useBufferB, use Buffer A or Buffer B
        if (useBufferB) glBindVertexArray(vaoQuadsB);
        else
#endif
        { 
            glBindVertexArray(vaoQuads);
        }
        
        //printf("\nRequired Draws: %i\n", drawsCounter);
     
        for (int i = 0; i < drawsCounter; i++)
        {
            numQuads = draws[i].vCount/4;
            numIndicesToProcess = numQuads*6;  // Get number of Quads * 6 index by Quad
            
            //printf("Quads to render: %i - ", numQuads);
            //printf("Vertex Count: %i - ", draws[i].vCount);
            //printf("Binding texture: %i\n", draws[i].texId);

            glBindTexture(GL_TEXTURE_2D, draws[i].texId);
            
            // NOTE: The final parameter tells the GPU the offset in bytes from the start of the index buffer to the location of the first index to process
            glDrawElements(GL_TRIANGLES, numIndicesToProcess, GL_UNSIGNED_INT, (GLvoid*) (sizeof(GLuint) * indicesOffset));

            indicesOffset += draws[i].vCount/4*6;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind textures
    glBindVertexArray(0);   // Unbind VAO
    
    // Reset draws counter
    drawsCounter = 1;
    draws[0].texId = whiteTexture;
    draws[0].firstVertex = 0;
    draws[0].vCount = 0;
    
    // Reset vertex counters for next frame
    lines.vCounter = 0;
    lines.cCounter = 0;
    
    triangles.vCounter = 0;
    triangles.cCounter = 0;
    
    quads.vCounter = 0;
    quads.tcCounter = 0;
    quads.cCounter = 0;
 
    // TODO: Review double buffer performance -> no improvement! (?)
#ifdef USE_VBO_DOUBLE_BUFFERS
    useBufferB = !useBufferB;   // Change buffers usage!
#endif
}

void rlglDrawModel(Model model, bool wires)
{
    if (wires) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
#ifdef USE_OPENGL_11
    // NOTE: For models we use Vertex Arrays (OpenGL 1.1)
    glEnableClientState(GL_VERTEX_ARRAY);                     // Enable vertex array
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);              // Enable texture coords array
    glEnableClientState(GL_NORMAL_ARRAY);                     // Enable normals array
        
    glVertexPointer(3, GL_FLOAT, 0, model.vertices);          // Pointer to vertex coords array
    glTexCoordPointer(2, GL_FLOAT, 0, model.texcoords);       // Pointer to texture coords array
    glNormalPointer(GL_FLOAT, 0, model.normals);              // Pointer to normals array
    //glColorPointer(4, GL_UNSIGNED_BYTE, 0, model.colors);   // Pointer to colors array (NOT USED)
        
    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        //glRotatef(rotation * GetFrameTime(), 0, 1, 0);
        rlScalef(scale, scale, scale);
        
        rlColor4ub(color.r, color.g, color.b, color.a);

        glDrawArrays(GL_TRIANGLES, 0, model.numVertices);
    rlPopMatrix();
    
    glDisableClientState(GL_VERTEX_ARRAY);                     // Disable vertex array
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);              // Disable texture coords array
    glDisableClientState(GL_NORMAL_ARRAY);                     // Disable normals array
#endif

#ifdef USE_OPENGL_33
    glUseProgram(shaderProgram);        // Use our shader
    
    Matrix modelview2 = MatrixMultiply(model.transform, modelview);
    
    // NOTE: Drawing in OpenGL 3.3+, transform is passed to shader
    glUniformMatrix4fv(projectionMatrixLoc, 1, false, GetMatrixVector(projection));
    glUniformMatrix4fv(modelviewMatrixLoc, 1, false, GetMatrixVector(modelview2));
    glUniform1i(textureLoc, 0);
   
    glBindVertexArray(model.vaoId);
    //glBindTexture(GL_TEXTURE_2D, model.textureId);

    glDrawArrays(GL_TRIANGLES, 0, model.numVertices);

    //glBindTexture(GL_TEXTURE_2D, 0);    // Unbind textures
    glBindVertexArray(0);               // Unbind VAO
#endif

    if (wires) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

#endif

// Initialize Graphics Device (OpenGL stuff)
void rlglInitGraphicsDevice(int fbWidth, int fbHeight)
{
    //glViewport(0, 0, fbWidth, fbHeight);  // Set viewport width and height
                                            // NOTE: Not required, viewport will be full window space

    // NOTE: Don't confuse glViewport with the transformation matrix
    // NOTE: glViewport just defines the area of the context that you will actually draw to.
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers, depth buffer is used for 3D
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   // Set background color (black)
    //glClearDepth(1.0f);                                   // Clear depth buffer (default)
    
    glEnable(GL_DEPTH_TEST);                                // Enables depth testing (required for 3D)
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)

#ifdef USE_OPENGL_11    
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation (Deprecated in OGL 3.0)
                                                            // Other options: GL_FASTEST, GL_DONT_CARE (default)
#endif
    
    rlMatrixMode(RL_PROJECTION);                // Switch to PROJECTION matrix
    rlLoadIdentity();                           // Reset current matrix (PROJECTION)
    
    rlOrtho(0, fbWidth, fbHeight, 0, 0, 1);     // Config orthographic mode: top-left corner --> (0,0)
    
    rlMatrixMode(RL_MODELVIEW);                 // Switch back to MODELVIEW matrix
    rlLoadIdentity();                           // Reset current matrix (MODELVIEW)
    
    // TODO: Review all shapes/models are drawn CCW and enable backface culling

    glEnable(GL_CULL_FACE);       // Enable backface culling (Disabled by default)
    //glCullFace(GL_BACK);        // Cull the Back face (default)
    //glFrontFace(GL_CCW);        // Front face are defined counter clockwise (default)

#ifdef USE_OPENGL_11    
    glShadeModel(GL_SMOOTH);      // Smooth shading between vertex (vertex colors interpolation) (Deprecated on OpenGL 3.3+)
                                  // Possible options: GL_SMOOTH (Color interpolation) or GL_FLAT (no interpolation)
#endif
}

// Convert image data to OpenGL texture (returns OpenGL valid Id)
// NOTE: Image is not unloaded, it should be done manually...
unsigned int rlglLoadTexture(int width, int height, unsigned char *pixels)
{
    glBindTexture(GL_TEXTURE_2D,0); // Free any old binding

    GLuint id;
    glGenTextures(1, &id);         // Generate Pointer to the texture
    
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    
    // NOTE: glTexParameteri does NOT affect texture uploading, just the way it's used!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repead on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repead on y-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Filter for pixel-perfect drawing, alternative: GL_LINEAR
 
#ifdef USE_OPENGL_33 
    // Trilinear filtering
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);   // Activate use of mipmaps (must be available)
    //glGenerateMipmap(GL_TEXTURE_2D);    // OpenGL 3.3!
#endif  

    // NOTE: Not using mipmappings (texture for 2D drawing)
    // At this point we have the image converted to texture and uploaded to GPU
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    // At this point we have the image converted to texture and uploaded to GPU
    
    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    printf("New texture created, id: %i (%i x %i)\n", id, width, height);
    
    return id;
}

#ifdef USE_OPENGL_33 
unsigned int rlglLoadModel(VertexData data)
{
    GLuint vaoModel;            // Vertex Array Objects (VAO)
    GLuint vertexBuffer[3];     // Vertex Buffer Objects (VBO)

    // Initialize Quads VAO (Buffer A)
    glGenVertexArrays(1, &vaoModel);
    glBindVertexArray(vaoModel);
 
    // Create buffers for our vertex data (positions, texcoords, normals)
    glGenBuffers(3, vertexBuffer);
 
    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*data.numVertices, data.vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*data.numVertices, data.texcoords, GL_STATIC_DRAW);      
    glEnableVertexAttribArray(texcoordLoc);
    glVertexAttribPointer(texcoordLoc, 2, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*data.numVertices, data.normals, GL_STATIC_DRAW);   
    //glEnableVertexAttribArray(normalLoc);
    //glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);
    
    return vaoModel;
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

#ifdef USE_OPENGL_33

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

#if defined(USE_OPENGL_33) || defined(USE_OPENGL_ES2)

// Load Shaders (Vertex and Fragment)
static GLuint LoadDefaultShaders()
{
    // NOTE: Shaders are written using GLSL 110 (desktop), that is equivalent to GLSL 100 on ES2 

    // Vertex shader directly defined, no external file required
    char vShaderStr[] = " #version 110      \n"     // Equivalent to version 100 on ES2
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
    char fShaderStr[] = " #version 110      \n"     // Equivalent to version 100 on ES2
        "uniform sampler2D texture0;        \n"
        "varying vec2 fragTexCoord;         \n"
        "varying vec4 fragColor;            \n"
        "void main()                        \n"
        "{                                  \n"
        "    gl_FragColor = texture2D(texture0, fragTexCoord) * fragColor; \n"
        "}                                  \n";

    GLuint program;
    GLuint vertexShader;
    GLuint fragmentShader;
 
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
 
    const char *pvs = vShaderStr;
    const char *pfs = fShaderStr;
 
    glShaderSource(vertexShader, 1, &pvs, NULL);
    glShaderSource(fragmentShader, 1, &pfs, NULL);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
 
    program = glCreateProgram();
    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
 
    glLinkProgram(program);
 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
 
    return(program);
}


// Load Shaders
static GLuint LoadShaders(char *vertexFileName, char *fragmentFileName)
{
    // Shaders loading from external text file
    char *vShaderStr = TextFileRead(vertexFileName);
    char *fShaderStr = TextFileRead(fragmentFileName);

    GLuint program;
    GLuint vertexShader;
    GLuint fragmentShader;
 
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
 
    const char *pvs = vShaderStr;
    const char *pfs = fShaderStr;
 
    glShaderSource(vertexShader, 1, &pvs, NULL);
    glShaderSource(fragmentShader, 1, &pfs, NULL);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
 
    program = glCreateProgram();
    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
 
    glLinkProgram(program);
 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
 
    return(program);
}

// Read shader text file
static char *TextFileRead(char *fn) 
{
    FILE *fp;
    char *content = NULL;

    int count=0;

    if (fn != NULL) 
    {
        fp = fopen(fn,"rt");

        if (fp != NULL) 
        {
            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) 
            {
                content = (char *)malloc(sizeof(char) * (count+1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            
            fclose(fp);
        }
    }
    return content;
}

// Allocate and initialize float array buffers to store vertex data (lines, triangles, quads)
static void InitializeBuffers()
{
    // Initialize lines arrays (vertex position and color data)
    lines.vertices = (float *)malloc(sizeof(float)*3*2*MAX_LINES_BATCH);    // 3 float by vertex, 2 vertex by line
    lines.colors = (float *)malloc(sizeof(float)*4*2*MAX_LINES_BATCH);      // 4 float by color, 2 colors by line

    for (int i = 0; i < (3*2*MAX_LINES_BATCH); i++) lines.vertices[i] = 0.0;
    for (int i = 0; i < (4*2*MAX_LINES_BATCH); i++) lines.colors[i] = 0.0;
    
    lines.vCounter = 0;
    lines.cCounter = 0;
    
    // Initialize triangles arrays (vertex position and color data)
    triangles.vertices = (float *)malloc(sizeof(float)*3*3*MAX_TRIANGLES_BATCH);    // 3 float by vertex, 3 vertex by triangle
    triangles.colors = (float *)malloc(sizeof(float)*4*3*MAX_TRIANGLES_BATCH);      // 4 float by color, 3 colors by triangle

    for (int i = 0; i < (3*3*MAX_TRIANGLES_BATCH); i++) triangles.vertices[i] = 0.0;
    for (int i = 0; i < (4*3*MAX_TRIANGLES_BATCH); i++) triangles.colors[i] = 0.0;
    
    triangles.vCounter = 0;
    triangles.cCounter = 0;
    
    // Initialize quads arrays (vertex position, texcoord and color data... and indexes)
    quads.vertices = (float *)malloc(sizeof(float)*3*4*MAX_QUADS_BATCH);    // 3 float by vertex, 4 vertex by quad
    quads.texcoords = (float *)malloc(sizeof(float)*2*4*MAX_QUADS_BATCH);   // 2 float by texcoord, 4 texcoord by quad
    quads.colors = (float *)malloc(sizeof(float)*4*4*MAX_QUADS_BATCH);      // 4 float by color, 4 colors by quad
    quads.indices = (unsigned int *)malloc(sizeof(int)*6*MAX_QUADS_BATCH);  // 6 int by quad (indices)
    
    for (int i = 0; i < (3*4*MAX_QUADS_BATCH); i++) quads.vertices[i] = 0.0;
    for (int i = 0; i < (2*4*MAX_QUADS_BATCH); i++) quads.texcoords[i] = 0.0;
    for (int i = 0; i < (4*4*MAX_QUADS_BATCH); i++) quads.colors[i] = 0.0;
    
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
}

// Initialize Vertex Array Objects (Contain VBO)
static void InitializeVAOs()
{
    // Initialize Lines VAO
    glGenVertexArrays(1, &vaoLines);
    glBindVertexArray(vaoLines);
 
    // Create buffers for our vertex data
    glGenBuffers(2, linesBuffer);
 
    // Lines - Vertex positions buffer binding and attributes enable
    glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*2*MAX_LINES_BATCH, lines.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
    
    // Lines - colors buffer
    glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*2*MAX_LINES_BATCH, lines.colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);

    //-------------------------------------------------------------- 
    
    // Initialize Triangles VAO
    glGenVertexArrays(1, &vaoTriangles);
    glBindVertexArray(vaoTriangles);
 
    // Create buffers for our vertex data
    glGenBuffers(2, trianglesBuffer);
 
    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*3*MAX_TRIANGLES_BATCH, triangles.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*3*MAX_TRIANGLES_BATCH, triangles.colors, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);

    //-------------------------------------------------------------- 
    
    // Initialize Quads VAO (Buffer A)
    glGenVertexArrays(1, &vaoQuads);
    glBindVertexArray(vaoQuads);
 
    // Create buffers for our vertex data
    glGenBuffers(4, quadsBuffer);
 
    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*MAX_QUADS_BATCH, quads.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*MAX_QUADS_BATCH, quads.texcoords, GL_DYNAMIC_DRAW);      
    glEnableVertexAttribArray(texcoordLoc);
    glVertexAttribPointer(texcoordLoc, 2, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadsBuffer[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*MAX_QUADS_BATCH, quads.colors, GL_DYNAMIC_DRAW);   
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);
    
    // Fill index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadsBuffer[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6*MAX_QUADS_BATCH, quads.indices, GL_STATIC_DRAW);
    
#ifdef USE_VBO_DOUBLE_BUFFERS
    // Initialize Quads VAO (Buffer B)
    glGenVertexArrays(1, &vaoQuadsB);
    glBindVertexArray(vaoQuadsB);
 
    // Create buffers for our vertex data
    glGenBuffers(4, quadsBufferB);
 
    // Enable vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, quadsBufferB[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*MAX_QUADS_BATCH, quads.vertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadsBufferB[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*MAX_QUADS_BATCH, quads.texcoords, GL_DYNAMIC_DRAW);      
    glEnableVertexAttribArray(texcoordLoc);
    glVertexAttribPointer(texcoordLoc, 2, GL_FLOAT, 0, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadsBufferB[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*MAX_QUADS_BATCH, quads.colors, GL_DYNAMIC_DRAW);   
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);
    
    // Fill index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadsBufferB[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*6*MAX_QUADS_BATCH, quads.indices, GL_STATIC_DRAW);
    
    printf("Using VBO double buffering\n");
#endif
 
    printf("Vertex buffers initialized (lines, triangles, quads)\n\n");
 
    // Unbind the current VAO
    glBindVertexArray(0);
}

// Update VBOs with vertex array data
static void UpdateBuffers()
{
    // Activate Lines VAO
    glBindVertexArray(vaoLines);
 
    // Lines - vertex positions buffer
    glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[0]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*2*MAX_LINES_BATCH, lines.vertices, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*lines.vCounter, lines.vertices);    // target - offset (in bytes) - size (in bytes) - data pointer
    
    // Lines - colors buffer
    glBindBuffer(GL_ARRAY_BUFFER, linesBuffer[1]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*2*MAX_LINES_BATCH, lines.colors, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*4*lines.vCounter, lines.colors);
    
    //--------------------------------------------------------------    
    
    // Activate Triangles VAO
    glBindVertexArray(vaoTriangles);
 
    // Triangles - vertex positions buffer
    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[0]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*3*MAX_TRIANGLES_BATCH, triangles.vertices, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*triangles.vCounter, triangles.vertices);
    
    // Triangles - colors buffer
    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[1]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*3*MAX_TRIANGLES_BATCH, triangles.colors, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*4*triangles.cCounter, triangles.colors);
    
    //--------------------------------------------------------------

    // Depending on useBufferB, update Buffer A or Buffer B
#ifdef USE_VBO_DOUBLE_BUFFERS
    if (useBufferB)
    {
        // Activate Quads VAO (Buffer B)
        glBindVertexArray(vaoQuadsB);
     
        // Quads - vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadsBufferB[0]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*MAX_QUADS_BATCH, quads.vertices, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*quads.vCounter, quads.vertices);

        // Quads - texture coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadsBufferB[1]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*MAX_QUADS_BATCH, quads.texcoords, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*2*quads.vCounter, quads.texcoords);
        
        // Quads - colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, quadsBufferB[2]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*MAX_QUADS_BATCH, quads.colors, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*4*quads.vCounter, quads.colors);
    }
    else
#endif
    {
        // Activate Quads VAO (Buffer A)
        glBindVertexArray(vaoQuads);
     
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
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*4*quads.vCounter, quads.colors);
    }
    
        
    // Another option would be using buffer mapping...
    //triangles.vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    // Now we can modify vertices
    //glUnmapBuffer(GL_ARRAY_BUFFER);
    
    //--------------------------------------------------------------
    
    // Unbind the current VAO
    glBindVertexArray(0);
}

#endif