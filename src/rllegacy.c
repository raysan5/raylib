/*********************************************************************************************
* 
*   rllegacy - raylib OpenGL 1.1 functions replacement
*    
*   OpenGL 1.1 functions mapping to OpenGL 3.2+ (and OpenGL ES 2.0)
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

#include "rllegacy.h"

#include <stdio.h>          // Standard input / output lib
#include <stdlib.h>         // Declares malloc() and free() for memory management, rand()

#include "raymath.h"        // Required for data type Matrix and Matrix functions

#define GLEW_STATIC         
#include <GL/glew.h>        // Extensions loading lib

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MATRIX_STACK_SIZE         16   
#define MAX_DRAWS_BY_TEXTURE     256    // Draws are organized by texture changes

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { LINES, TRIANGLES, QUADS } DrawMode;

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

typedef struct {
    int vCounter;
    int tcCounter;
    int cCounter;
    float *vertices;            // 3 components per vertex
    float *texcoords;           // 2 components per vertex
    float *colors;              // 4 components per vertex
} VertexPositionColorTextureBuffer;

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
static Matrix stack[MATRIX_STACK_SIZE];
static int stackCounter = 0;

static Matrix modelview;
static Matrix projection;
static Matrix *currentMatrix;

static DrawMode currentDrawMode;

// Vertex arrays for lines, triangles and quads
static VertexPositionColorBuffer lines;
static VertexPositionColorTextureBuffer triangles;
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

// TODO: Review -> Do not get any performance improvement... why?
static GLuint vaoQuadsB;
static GLuint quadsBufferB[4];
static int useBufferB = 0;
    
static DrawCall *draws;
static int drawsCounter;

// White texture useful for plain color polys (required by shader)
static GLuint whiteTexture;

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static GLuint LoadShaders(char *vertexFileName, char *fragmentFileName);
static char *TextFileRead(char *fn);
static void CreateDefaultTexture();
static void InitializeBuffers();
static void InitializeVAOs();
static void UpdateBuffers();
static void rlDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, void *indices);

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix operations
//----------------------------------------------------------------------------------

// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) currentMatrix = &projection;
    else if (mode == RL_MODELVIEW) currentMatrix = &modelview;
    //else if (mode == GL_TEXTURE) TODO: NEVER USED!
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
}

// Pop lattest inserted matrix from stack
void rlPopMatrix()
{
    if (stackCounter > 0)
    {
        Matrix mat = stack[stackCounter - 1];

        stackCounter--;
        
        *currentMatrix = mat;
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
    Matrix mat = MatrixFrustum(left, right, bottom, top, near, far);

    *currentMatrix = MatrixMultiply(*currentMatrix, mat);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
void rlOrtho(double left, double right, double bottom, double top, double near, double far) 
{
    Matrix matOrtho = MatrixOrtho(left, right, bottom, top, near, far);
    MatrixTranspose(&matOrtho);
    
    *currentMatrix = MatrixMultiply(*currentMatrix, matOrtho);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Vertex level operations
//----------------------------------------------------------------------------------

// Initialize drawing mode (how to organize vertex)
void rlBegin(int mode)
{
    // TODO: Code shouldn't include other modes than RL_POINTS, RL_LINES and RL_TRIANGLES
    switch (mode)
    {
        case RL_POINTS: currentDrawMode = LINES; break;
        case RL_LINES: currentDrawMode = LINES; break;
        case RL_LINE_LOOP: currentDrawMode = LINES; break;
        case RL_TRIANGLE_FAN: currentDrawMode = TRIANGLES; break;
        case RL_QUADS: currentDrawMode = QUADS; break;
        case RL_QUAD_STRIP: currentDrawMode = QUADS; break;
        default: break;
    }
    
    int id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &id);
    
    if (id == 0)
    {    
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        id = 1;
    }
    
    if (draws[drawsCounter - 1].texId != id)
    {   
        drawsCounter++;
    
        draws[drawsCounter - 1].texId = id;
        draws[drawsCounter - 1].firstVertex = draws[drawsCounter - 2].vCount;
        draws[drawsCounter - 1].vCount = 0;
    }
}

// Finish vertex providing
void rlEnd()
{
    // Make sure vertexCounter is the same for vertices-texcoords-normals-colors
    // NOTE: In OpenGL 1.1, one glColor call can be made for all the subsequent glVertex calls.
    switch (currentDrawMode)
    {
        case LINES:
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
        case TRIANGLES:
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
        case QUADS:
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
    switch (currentDrawMode)
    {
        case LINES:
        {
            lines.vertices[3*lines.vCounter] = x;
            lines.vertices[3*lines.vCounter + 1] = y;
            lines.vertices[3*lines.vCounter + 2] = z;
            
            lines.vCounter++;
            
        } break;
        case TRIANGLES:
        {
            triangles.vertices[3*triangles.vCounter] = x;
            triangles.vertices[3*triangles.vCounter + 1] = y;
            triangles.vertices[3*triangles.vCounter + 2] = z;
            
            triangles.vCounter++;
            
        } break;
        case QUADS:
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
    if (currentDrawMode == QUADS)
    {
        quads.texcoords[2*quads.tcCounter] = x;
        quads.texcoords[2*quads.tcCounter + 1] = y;

        quads.tcCounter++;
    }
}

// Define one vertex (normal)
// NOTE: Normals are limited to TRIANGLES only
void rlNormal3f(float x, float y, float z)
{
    // TODO: Normals usage...
}

// Define one vertex (color)
void rlColor4f(float x, float y, float z, float w)
{
    switch (currentDrawMode)
    {
        case LINES:
        {
            lines.colors[4*lines.cCounter] = x;
            lines.colors[4*lines.cCounter + 1] = y;
            lines.colors[4*lines.cCounter + 2] = z;
            lines.colors[4*lines.cCounter + 3] = w;
            
            lines.cCounter++;
            
        } break;
        case TRIANGLES:
        {
            triangles.colors[4*triangles.cCounter] = x;
            triangles.colors[4*triangles.cCounter + 1] = y;
            triangles.colors[4*triangles.cCounter + 2] = z;
            triangles.colors[4*triangles.cCounter + 3] = w;
            
            triangles.cCounter++;
            
        } break;
        case QUADS:
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

//----------------------------------------------------------------------------------
// Module Functions Definition - RlLegacy Init-Close
//----------------------------------------------------------------------------------

// Init OpenGL 3.3+ required data
void InitRlLegacy()
{                           
    // Initialize GLEW
	glewExperimental = 1;       // Needed for core profile

    GLenum error = glewInit();
    
    if (error != GLEW_OK) 
    {
		fprintf(stderr, "Failed to initialize GLEW - Error: %s\n", glewGetErrorString(error));
		exit(1);
	}

    if (glewIsSupported("GL_VERSION_3_3")) printf("Ready for OpenGL 3.3\n"); 

    // Set default draw mode
    currentDrawMode = TRIANGLES;
    
    // Reset projection and modelview matrices
    projection = MatrixIdentity();
    modelview = MatrixIdentity();
    currentMatrix = &modelview;
    
    // Initialize matrix stack
    for (int i = 0; i < MATRIX_STACK_SIZE; i++) stack[i] = MatrixIdentity();

    // Init default Shader (GLSL 150)
    shaderProgram = LoadShaders("simple150.vert", "simple150.frag");
    
    // Get handles to GLSL input vars locations
    vertexLoc = glGetAttribLocation(shaderProgram, "vertexPosition");
    texcoordLoc = glGetAttribLocation(shaderProgram, "vertexTexCoord");
    colorLoc = glGetAttribLocation(shaderProgram, "vertexColor");
    
    // Get handles to GLSL uniform vars locations (vertex-shader)
	modelviewMatrixLoc = glGetUniformLocation(shaderProgram, "modelviewMatrix");
    projectionMatrixLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
    
    // Get handles to GLSL uniform vars locations (fragment-shader)
	textureLoc  = glGetUniformLocation(shaderProgram, "texture0");
    
    printf("Shaders loaded!\n");
    
    InitializeBuffers();    // Init vertex arrays
    InitializeVAOs();       // Init VBO and VAO
    
    draws = (DrawCall *)malloc(sizeof(DrawCall)*MAX_DRAWS_BY_TEXTURE);
    
    for (int i = 0; i < MAX_DRAWS_BY_TEXTURE; i++)
    {
        draws[i].texId = 0;
        draws[i].firstVertex = 0;
        draws[i].vCount = 0;
    }
    
    drawsCounter = 1;
    
    // Default white texture for plain colors (required by shader)   
    CreateDefaultTexture();
    
    draws[drawsCounter - 1].texId = whiteTexture;
}

// Vertex Buffer Object deinitialization (memory free)
void CloseRlLegacy()
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

void DrawRlLegacy()
{
    glUseProgram(shaderProgram);        // Use our shader

    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    
    glUniformMatrix4fv(projectionMatrixLoc, 1, false, GetMatrixVector(projection));
    glUniformMatrix4fv(modelviewMatrixLoc, 1, false, GetMatrixVector(modelview));
    glUniform1i(textureLoc, 0);

    UpdateBuffers();

    if (lines.vCounter > 0)
    {
        glBindVertexArray(vaoLines);
        glDrawArrays(GL_LINES, 0, lines.vCounter);
    }
    
    if (triangles.vCounter > 0)
    {
        glBindVertexArray(vaoTriangles);
        glDrawArrays(GL_TRIANGLES, 0, triangles.vCounter);
    }
    
    if (quads.vCounter > 0)
    {
        int numQuads = 0;
 
        // Depending on useBufferB, use Buffer A or Buffer B
        if (useBufferB) glBindVertexArray(vaoQuadsB);
        else glBindVertexArray(vaoQuads);
        
        for (int i = 0; i < drawsCounter; i++)
        {
            numQuads += draws[i].vCount/4;

            glBindTexture(GL_TEXTURE_2D, draws[i].texId);
            rlDrawRangeElements(GL_TRIANGLES, draws[i].firstVertex, draws[i].firstVertex + draws[i].vCount - 1, 6*numQuads, GL_UNSIGNED_INT, 0);
        }
    }
  
    glBindTexture(GL_TEXTURE_2D, whiteTexture);

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
    triangles.vCounter = 0;
    
    quads.vCounter = 0;
    quads.tcCounter = 0;
    quads.cCounter = 0;
    
    // TODO: Review double buffer performance -> no improvement!
    //useBufferB = !useBufferB;   // Change buffers usage!
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Load shader (GLSL 150)
static GLuint LoadShaders(char *vertexFileName, char *fragmentFileName)
{
    char *vs = NULL;
    char *fs = NULL;
 
    GLuint p,v,f;
 
    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
 
    vs = TextFileRead(vertexFileName);
    fs = TextFileRead(fragmentFileName);
 
    const char *vv = vs;
    const char *ff = fs;
 
    glShaderSource(v, 1, &vv, NULL);
    glShaderSource(f, 1, &ff, NULL);
 
    free(vs);
    free(fs);
 
    glCompileShader(v);
    glCompileShader(f);
 
    p = glCreateProgram();
    
    glAttachShader(p,v);
    glAttachShader(p,f);
 
    glLinkProgram(p);
 
    glDeleteShader(v);
	glDeleteShader(f);
 
    return(p);
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
    
    printf("All buffers initialized - lines, triangles, quads(indexed).\n");
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*3*MAX_TRIANGLES_BATCH, triangles.vertices, GL_DYNAMIC_DRAW);

    // Triangles - colors buffer
    glBindBuffer(GL_ARRAY_BUFFER, trianglesBuffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*3*MAX_TRIANGLES_BATCH, triangles.colors, GL_DYNAMIC_DRAW);
    
    //--------------------------------------------------------------

    // Depending on useBufferB, update Buffer A or Buffer B
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
    
    //--------------------------------------------------------------
    
    // Unbind the current VAO
    glBindVertexArray(0);
}

// Create default white texture to be used on plain colors (required by default shader)
static void CreateDefaultTexture()
{       
    unsigned char *imgData = (unsigned char *)malloc(4);   // 1 pixel RGBA (4 bytes)
    
    for (int i = 0; i < 4; i++) imgData[i] = 255;        // Initialize array

    // Convert data to OpenGL texture
    //--------------------------------
    GLuint id;
    glGenTextures(1, &id);                    // Generate pointer to the texture
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);            // Set texture to clamp on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);            // Set texture to clamp on y-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);      // Filter for pixel-perfect drawing, alternative: GL_LINEAR 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);      // Filter for pixel-perfect drawing, alternative: GL_LINEAR 
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
    
    // At this point we have the image converted to texture and uploaded to GPU
    
    free(imgData);                    // Now we can free loaded data from RAM memory
    
    whiteTexture = id;
    
    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Replacement for glDrawRangeElements (OpenGL 3.3+)
// NOTE: glDrawRangeElement is not supported by OpenGL ES 2.0 so, we use glDrawElements instead
static void rlDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, void *indices)
{
    glDrawElements(mode, count, type, indices);
}