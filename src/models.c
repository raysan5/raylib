/**********************************************************************************************
*
*   raylib.models - Basic functions to deal with 3d shapes and 3d models
*
*   CONFIGURATION:
*
*   #define SUPPORT_FILEFORMAT_OBJ
*   #define SUPPORT_FILEFORMAT_MTL
*   #define SUPPORT_FILEFORMAT_IQM
*   #define SUPPORT_FILEFORMAT_GLTF
*   #define SUPPORT_FILEFORMAT_VOX
* 
*       Selected desired fileformats to be supported for model data loading.
*
*   #define SUPPORT_MESH_GENERATION
*       Support procedural mesh generation functions, uses external par_shapes.h library
*       NOTE: Some generated meshes DO NOT include generated texture coordinates
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2021 Ramon Santamaria (@raysan5)
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

#include "raylib.h"         // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"     // Defines module configuration flags
#endif

#include "utils.h"          // Required for: TRACELOG(), LoadFileData(), LoadFileText(), SaveFileText()
#include "rlgl.h"           // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2
#include "raymath.h"        // Required for: Vector3, Quaternion and Matrix functionality

#include <stdio.h>          // Required for: sprintf()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: memcmp(), strlen()
#include <math.h>           // Required for: sinf(), cosf(), sqrtf(), fabsf()

#if defined(SUPPORT_FILEFORMAT_OBJ) || defined(SUPPORT_FILEFORMAT_MTL)
    #define TINYOBJ_MALLOC RL_MALLOC
    #define TINYOBJ_CALLOC RL_CALLOC
    #define TINYOBJ_REALLOC RL_REALLOC
    #define TINYOBJ_FREE RL_FREE

    #define TINYOBJ_LOADER_C_IMPLEMENTATION
    #include "external/tinyobj_loader_c.h"      // OBJ/MTL file formats loading
#endif

#if defined(SUPPORT_FILEFORMAT_GLTF)
    #define CGLTF_MALLOC RL_MALLOC
    #define CGLTF_FREE RL_FREE

    #define CGLTF_IMPLEMENTATION
    #include "external/cgltf.h"         // glTF file format loading
    #include "external/stb_image.h"     // glTF texture images loading
#endif

#if defined(SUPPORT_FILEFORMAT_VOX)
    // TODO: Support custom memory allocators

    #define VOX_LOADER_IMPLEMENTATION
    #include "external/vox_loader.h"    // vox file format loading (MagikaVoxel)
#endif

#if defined(SUPPORT_MESH_GENERATION)
    #define PAR_MALLOC(T, N) ((T*)RL_MALLOC(N*sizeof(T)))
    #define PAR_CALLOC(T, N) ((T*)RL_CALLOC(N*sizeof(T), 1))
    #define PAR_REALLOC(T, BUF, N) ((T*)RL_REALLOC(BUF, sizeof(T)*(N)))
    #define PAR_FREE RL_FREE

    #define PAR_SHAPES_IMPLEMENTATION
    #include "external/par_shapes.h"    // Shapes 3d parametric generation
#endif

#if defined(_WIN32)
    #include <direct.h>     // Required for: _chdir() [Used in LoadOBJ()]
    #define CHDIR _chdir
#else
    #include <unistd.h>     // Required for: chdir() (POSIX) [Used in LoadOBJ()]
    #define CHDIR chdir
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef MAX_MATERIAL_MAPS
    #define MAX_MATERIAL_MAPS       12    // Maximum number of maps supported
#endif
#ifndef MAX_MESH_VERTEX_BUFFERS
    #define MAX_MESH_VERTEX_BUFFERS  7    // Maximum vertex buffers (VBO) per mesh
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_OBJ)
static Model LoadOBJ(const char *fileName);     // Load OBJ mesh data
#endif
#if defined(SUPPORT_FILEFORMAT_IQM)
static Model LoadIQM(const char *fileName);     // Load IQM mesh data
static ModelAnimation *LoadIQMModelAnimations(const char *fileName, int *animCount);    // Load IQM animation data
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
static Model LoadGLTF(const char *fileName);    // Load GLTF mesh data
static ModelAnimation *LoadGLTFModelAnimations(const char *fileName, int *animCount);    // Load GLTF animation data
static void LoadGLTFMaterial(Model *model, const char *fileName, const cgltf_data *data);
static void LoadGLTFMesh(cgltf_data *data, cgltf_mesh *mesh, Model *outModel, Matrix currentTransform, int *primitiveIndex, const char *fileName);
static void LoadGLTFNode(cgltf_data *data, cgltf_node *node, Model *outModel, Matrix currentTransform, int *primitiveIndex, const char *fileName);
static void InitGLTFBones(Model *model, const cgltf_data *data);
static void BindGLTFPrimitiveToBones(Model *model, const cgltf_data *data, int primitiveIndex);
static void GetGLTFPrimitiveCount(cgltf_node *node, int *outCount);
static bool ReadGLTFValue(cgltf_accessor *acc, unsigned int index, void *variable);
static void *ReadGLTFValuesAs(cgltf_accessor *acc, cgltf_component_type type, bool adjustOnDownCasting);
#endif
#if defined(SUPPORT_FILEFORMAT_VOX)
static Model LoadVOX(const char *filename);     // Load VOX mesh data
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Draw a line in 3D world space
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color)
{
    // WARNING: Be careful with internal buffer vertex alignment
    // when using RL_LINES or RL_TRIANGLES, data is aligned to fit
    // lines-triangles-quads in the same indexed buffers!!!
    rlCheckRenderBatchLimit(8);

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex3f(startPos.x, startPos.y, startPos.z);
        rlVertex3f(endPos.x, endPos.y, endPos.z);
    rlEnd();
}

// Draw a point in 3D space, actually a small line
void DrawPoint3D(Vector3 position, Color color)
{
    rlCheckRenderBatchLimit(8);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);
        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex3f(0.0f, 0.0f, 0.0f);
            rlVertex3f(0.0f, 0.0f, 0.1f);
        rlEnd();
    rlPopMatrix();
}

// Draw a circle in 3D world space
void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color)
{
    rlCheckRenderBatchLimit(2*36);

    rlPushMatrix();
        rlTranslatef(center.x, center.y, center.z);
        rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);

        rlBegin(RL_LINES);
            for (int i = 0; i < 360; i += 10)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex3f(sinf(DEG2RAD*i)*radius, cosf(DEG2RAD*i)*radius, 0.0f);
                rlVertex3f(sinf(DEG2RAD*(i + 10))*radius, cosf(DEG2RAD*(i + 10))*radius, 0.0f);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a color-filled triangle (vertex in counter-clockwise order!)
void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color)
{
    rlCheckRenderBatchLimit(3);

    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex3f(v1.x, v1.y, v1.z);
        rlVertex3f(v2.x, v2.y, v2.z);
        rlVertex3f(v3.x, v3.y, v3.z);
    rlEnd();
}

// Draw a triangle strip defined by points
void DrawTriangleStrip3D(Vector3 *points, int pointCount, Color color)
{
    if (pointCount >= 3)
    {
        rlCheckRenderBatchLimit(3*(pointCount - 2));

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 2; i < pointCount; i++)
            {
                if ((i%2) == 0)
                {
                    rlVertex3f(points[i].x, points[i].y, points[i].z);
                    rlVertex3f(points[i - 2].x, points[i - 2].y, points[i - 2].z);
                    rlVertex3f(points[i - 1].x, points[i - 1].y, points[i - 1].z);
                }
                else
                {
                    rlVertex3f(points[i].x, points[i].y, points[i].z);
                    rlVertex3f(points[i - 1].x, points[i - 1].y, points[i - 1].z);
                    rlVertex3f(points[i - 2].x, points[i - 2].y, points[i - 2].z);
                }
            }
        rlEnd();
    }
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlCheckRenderBatchLimit(36);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front face
            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left

            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right

            // Back face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right

            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left

            // Top face
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

            // Bottom face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left

            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left

            // Right face
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

            // Left face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right

            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
        rlEnd();
    rlPopMatrix();
}

// Draw cube (Vector version)
void DrawCubeV(Vector3 position, Vector3 size, Color color)
{
    DrawCube(position, size.x, size.y, size.z, color);
}

// Draw cube wires
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlCheckRenderBatchLimit(36);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            // Back Face ------------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left

            // Top Face -------------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Back

            // Bottom Face  ---------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right Back
        rlEnd();
    rlPopMatrix();
}

// Draw cube wires (vector version)
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color)
{
    DrawCubeWires(position, size.x, size.y, size.z, color);
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCubeTexture(Texture2D texture, Vector3 position, float width, float height, float length, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;

    rlCheckRenderBatchLimit(36);

    rlSetTexture(texture.id);

    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            // Front Face
            rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            // Back Face
            rlNormal3f(0.0f, 0.0f, - 1.0f);                  // Normal Pointing Away From Viewer
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            // Top Face
            rlNormal3f(0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            // Bottom Face
            rlNormal3f(0.0f, - 1.0f, 0.0f);                  // Normal Pointing Down
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            // Left Face
            rlNormal3f( - 1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
        rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}

// Draw sphere
void DrawSphere(Vector3 centerPos, float radius, Color color)
{
    DrawSphereEx(centerPos, radius, 16, 16, color);
}

// Draw sphere with extended parameters
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    int numVertex = (rings + 2)*slices*6;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> translate)
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*i)),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*cosf(DEG2RAD*(360.0f*j/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*j/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*i)),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*cosf(DEG2RAD*(360.0f*j/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}

// Draw sphere wires
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    int numVertex = (rings + 2)*slices*6;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> translate)
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*i)),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*cosf(DEG2RAD*(360.0f*j/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*(j + 1)/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*(j + 1)/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*j/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1))),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*(i + 1)))*cosf(DEG2RAD*(360.0f*j/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*sinf(DEG2RAD*(360.0f*j/slices)),
                               sinf(DEG2RAD*(270 + (180.0f/(rings + 1))*i)),
                               cosf(DEG2RAD*(270 + (180.0f/(rings + 1))*i))*cosf(DEG2RAD*(360.0f*j/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a cylinder
// NOTE: It could be also used for pyramid and cone
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
    if (sides < 3) sides = 3;

    int numVertex = sides*6;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            if (radiusTop > 0)
            {
                // Draw Body -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); //Bottom Left
                    rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360.0f/sides))*radiusBottom); //Bottom Right
                    rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360.0f/sides))*radiusTop); //Top Right

                    rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop); //Top Left
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); //Bottom Left
                    rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360.0f/sides))*radiusTop); //Top Right
                }

                // Draw Cap --------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
                    rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360.0f/sides))*radiusTop);
                }
            }
            else
            {
                // Draw Cone -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
                    rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360.0f/sides))*radiusBottom);
                }
            }

            // Draw Base -----------------------------------------------------------------------------------------
            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(0, 0, 0);
                rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360.0f/sides))*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a wired cylinder
// NOTE: It could be also used for pyramid and cone
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
    if (sides < 3) sides = 3;

    int numVertex = sides*8;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360.0f/sides))*radiusBottom);

                rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360.0f/sides))*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360.0f/sides))*radiusTop);

                rlVertex3f(sinf(DEG2RAD*(i + 360.0f/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360.0f/sides))*radiusTop);
                rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);

                rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a plane
void DrawPlane(Vector3 centerPos, Vector2 size, Color color)
{
    rlCheckRenderBatchLimit(4);

    // NOTE: Plane is always created on XZ ground
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(size.x, 1.0f, size.y);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(0.0f, 1.0f, 0.0f);

            rlVertex3f(-0.5f, 0.0f, -0.5f);
            rlVertex3f(-0.5f, 0.0f, 0.5f);
            rlVertex3f(0.5f, 0.0f, 0.5f);
            rlVertex3f(0.5f, 0.0f, -0.5f);
        rlEnd();
    rlPopMatrix();
}

// Draw a ray line
void DrawRay(Ray ray, Color color)
{
    float scale = 10000;

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex3f(ray.position.x, ray.position.y, ray.position.z);
        rlVertex3f(ray.position.x + ray.direction.x*scale, ray.position.y + ray.direction.y*scale, ray.position.z + ray.direction.z*scale);
    rlEnd();
}

// Draw a grid centered at (0, 0, 0)
void DrawGrid(int slices, float spacing)
{
    int halfSlices = slices/2;

    rlCheckRenderBatchLimit((slices + 2)*4);

    rlBegin(RL_LINES);
        for (int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
    rlEnd();
}

// Load model from files (mesh and material)
Model LoadModel(const char *fileName)
{
    Model model = { 0 };

#if defined(SUPPORT_FILEFORMAT_OBJ)
    if (IsFileExtension(fileName, ".obj")) model = LoadOBJ(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_IQM)
    if (IsFileExtension(fileName, ".iqm")) model = LoadIQM(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
    if (IsFileExtension(fileName, ".gltf;.glb")) model = LoadGLTF(fileName);
#endif
#if defined(SUPPORT_FILEFORMAT_VOX)
    if (IsFileExtension(fileName, ".vox")) model = LoadVOX(fileName);
#endif

    // Make sure model transform is set to identity matrix!
    model.transform = MatrixIdentity();

    if (model.meshCount == 0)
    {
        model.meshCount = 1;
        model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));
#if defined(SUPPORT_MESH_GENERATION)
        TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load mesh data, default to cube mesh", fileName);
        model.meshes[0] = GenMeshCube(1.0f, 1.0f, 1.0f);
#else
        TRACELOG(LOG_WARNING, "MESH: [%s] Failed to load mesh data", fileName);
#endif
    }
    else
    {
        // Upload vertex data to GPU (static mesh)
        for (int i = 0; i < model.meshCount; i++) UploadMesh(&model.meshes[i], false);
    }

    if (model.materialCount == 0)
    {
        TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to load material data, default to white material", fileName);

        model.materialCount = 1;
        model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
        model.materials[0] = LoadMaterialDefault();

        if (model.meshMaterial == NULL) model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
    }

    return model;
}

// Load model from generated mesh
// WARNING: A shallow copy of mesh is generated, passed by value,
// as long as struct contains pointers to data and some values, we get a copy
// of mesh pointing to same data as original version... be careful!
Model LoadModelFromMesh(Mesh mesh)
{
    Model model = { 0 };

    model.transform = MatrixIdentity();

    model.meshCount = 1;
    model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));
    model.meshes[0] = mesh;

    model.materialCount = 1;
    model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
    model.materials[0] = LoadMaterialDefault();

    model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));
    model.meshMaterial[0] = 0;  // First material index

    return model;
}

// Unload model (meshes/materials) from memory (RAM and/or VRAM)
// NOTE: This function takes care of all model elements, for a detailed control
// over them, use UnloadMesh() and UnloadMaterial()
void UnloadModel(Model model)
{
    // Unload meshes
    for (int i = 0; i < model.meshCount; i++) UnloadMesh(model.meshes[i]);

    // Unload materials maps
    // NOTE: As the user could be sharing shaders and textures between models,
    // we don't unload the material but just free it's maps,
    // the user is responsible for freeing models shaders and textures
    for (int i = 0; i < model.materialCount; i++) RL_FREE(model.materials[i].maps);

    // Unload arrays
    RL_FREE(model.meshes);
    RL_FREE(model.materials);
    RL_FREE(model.meshMaterial);

    // Unload animation data
    RL_FREE(model.bones);
    RL_FREE(model.bindPose);

    TRACELOG(LOG_INFO, "MODEL: Unloaded model (and meshes) from RAM and VRAM");
}

// Unload model (but not meshes) from memory (RAM and/or VRAM)
void UnloadModelKeepMeshes(Model model)
{
    // Unload materials maps
    // NOTE: As the user could be sharing shaders and textures between models,
    // we don't unload the material but just free it's maps,
    // the user is responsible for freeing models shaders and textures
    for (int i = 0; i < model.materialCount; i++) RL_FREE(model.materials[i].maps);

    // Unload arrays
    RL_FREE(model.meshes);
    RL_FREE(model.materials);
    RL_FREE(model.meshMaterial);

    // Unload animation data
    RL_FREE(model.bones);
    RL_FREE(model.bindPose);

    TRACELOG(LOG_INFO, "MODEL: Unloaded model (but not meshes) from RAM and VRAM");
}

// Compute model bounding box limits (considers all meshes)
BoundingBox GetModelBoundingBox(Model model)
{
    BoundingBox bounds = { 0 };

    if (model.meshCount > 0)
    {
        Vector3 temp = { 0 };
        bounds = GetMeshBoundingBox(model.meshes[0]);

        for (int i = 1; i < model.meshCount; i++)
        {
            BoundingBox tempBounds = GetMeshBoundingBox(model.meshes[i]);

            temp.x = (bounds.min.x < tempBounds.min.x)? bounds.min.x : tempBounds.min.x;
            temp.y = (bounds.min.y < tempBounds.min.y)? bounds.min.y : tempBounds.min.y;
            temp.z = (bounds.min.z < tempBounds.min.z)? bounds.min.z : tempBounds.min.z;
            bounds.min = temp;

            temp.x = (bounds.max.x > tempBounds.max.x)? bounds.max.x : tempBounds.max.x;
            temp.y = (bounds.max.y > tempBounds.max.y)? bounds.max.y : tempBounds.max.y;
            temp.z = (bounds.max.z > tempBounds.max.z)? bounds.max.z : tempBounds.max.z;
            bounds.max = temp;
        }
    }

    return bounds;
}

// Upload vertex data into a VAO (if supported) and VBO
void UploadMesh(Mesh *mesh, bool dynamic)
{
    if (mesh->vaoId > 0)
    {
        // Check if mesh has already been loaded in GPU
        TRACELOG(LOG_WARNING, "VAO: [ID %i] Trying to re-load an already loaded mesh", mesh->vaoId);
        return;
    }

    mesh->vboId = (unsigned int *)RL_CALLOC(MAX_MESH_VERTEX_BUFFERS, sizeof(unsigned int));

    mesh->vaoId = 0;        // Vertex Array Object
    mesh->vboId[0] = 0;     // Vertex buffer: positions
    mesh->vboId[1] = 0;     // Vertex buffer: texcoords
    mesh->vboId[2] = 0;     // Vertex buffer: normals
    mesh->vboId[3] = 0;     // Vertex buffer: colors
    mesh->vboId[4] = 0;     // Vertex buffer: tangents
    mesh->vboId[5] = 0;     // Vertex buffer: texcoords2
    mesh->vboId[6] = 0;     // Vertex buffer: indices

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mesh->vaoId = rlLoadVertexArray();
    rlEnableVertexArray(mesh->vaoId);

    // NOTE: Attributes must be uploaded considering default locations points

    // Enable vertex attributes: position (shader-location = 0)
    void *vertices = mesh->animVertices != NULL ? mesh->animVertices : mesh->vertices;
    mesh->vboId[0] = rlLoadVertexBuffer(vertices, mesh->vertexCount*3*sizeof(float), dynamic);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(0);

    // Enable vertex attributes: texcoords (shader-location = 1)
    mesh->vboId[1] = rlLoadVertexBuffer(mesh->texcoords, mesh->vertexCount*2*sizeof(float), dynamic);
    rlSetVertexAttribute(1, 2, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(1);

    if (mesh->normals != NULL)
    {
        // Enable vertex attributes: normals (shader-location = 2)
        void *normals = mesh->animNormals != NULL ? mesh->animNormals : mesh->normals;
        mesh->vboId[2] = rlLoadVertexBuffer(normals, mesh->vertexCount*3*sizeof(float), dynamic);
        rlSetVertexAttribute(2, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(2);
    }
    else
    {
        // Default color vertex attribute set to WHITE
        float value[3] = { 1.0f, 1.0f, 1.0f };
        rlSetVertexAttributeDefault(2, value, SHADER_ATTRIB_VEC3, 3);
        rlDisableVertexAttribute(2);
    }

    if (mesh->colors != NULL)
    {
        // Enable vertex attribute: color (shader-location = 3)
        mesh->vboId[3] = rlLoadVertexBuffer(mesh->colors, mesh->vertexCount*4*sizeof(unsigned char), dynamic);
        rlSetVertexAttribute(3, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
        rlEnableVertexAttribute(3);
    }
    else
    {
        // Default color vertex attribute set to WHITE
        float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        rlSetVertexAttributeDefault(3, value, SHADER_ATTRIB_VEC4, 4);
        rlDisableVertexAttribute(3);
    }

    if (mesh->tangents != NULL)
    {
        // Enable vertex attribute: tangent (shader-location = 4)
        mesh->vboId[4] = rlLoadVertexBuffer(mesh->tangents, mesh->vertexCount*4*sizeof(float), dynamic);
        rlSetVertexAttribute(4, 4, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(4);
    }
    else
    {
        // Default tangents vertex attribute
        float value[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        rlSetVertexAttributeDefault(4, value, SHADER_ATTRIB_VEC4, 4);
        rlDisableVertexAttribute(4);
    }

    if (mesh->texcoords2 != NULL)
    {
        // Enable vertex attribute: texcoord2 (shader-location = 5)
        mesh->vboId[5] = rlLoadVertexBuffer(mesh->texcoords2, mesh->vertexCount*2*sizeof(float), dynamic);
        rlSetVertexAttribute(5, 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(5);
    }
    else
    {
        // Default texcoord2 vertex attribute
        float value[2] = { 0.0f, 0.0f };
        rlSetVertexAttributeDefault(5, value, SHADER_ATTRIB_VEC2, 2);
        rlDisableVertexAttribute(5);
    }

    if (mesh->indices != NULL)
    {
        mesh->vboId[6] = rlLoadVertexBufferElement(mesh->indices, mesh->triangleCount*3*sizeof(unsigned short), dynamic);
    }

    if (mesh->vaoId > 0) TRACELOG(LOG_INFO, "VAO: [ID %i] Mesh uploaded successfully to VRAM (GPU)", mesh->vaoId);
    else TRACELOG(LOG_INFO, "VBO: Mesh uploaded successfully to VRAM (GPU)");

    rlDisableVertexArray();
#endif
}

// Update mesh vertex data in GPU for a specific buffer index
void UpdateMeshBuffer(Mesh mesh, int index, void *data, int dataSize, int offset)
{
    rlUpdateVertexBuffer(mesh.vboId[index], data, dataSize, offset);
}

// Draw a 3d mesh with material and transform
void DrawMesh(Mesh mesh, Material material, Matrix transform)
{
#if defined(GRAPHICS_API_OPENGL_11)
    #define GL_VERTEX_ARRAY         0x8074
    #define GL_NORMAL_ARRAY         0x8075
    #define GL_COLOR_ARRAY          0x8076
    #define GL_TEXTURE_COORD_ARRAY  0x8078

    rlEnableTexture(material.maps[MATERIAL_MAP_DIFFUSE].texture.id);

    rlEnableStatePointer(GL_VERTEX_ARRAY, mesh.vertices);
    rlEnableStatePointer(GL_TEXTURE_COORD_ARRAY, mesh.texcoords);
    rlEnableStatePointer(GL_NORMAL_ARRAY, mesh.normals);
    rlEnableStatePointer(GL_COLOR_ARRAY, mesh.colors);

    rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(transform));
        rlColor4ub(material.maps[MATERIAL_MAP_DIFFUSE].color.r,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.g,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.b,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.a);

        if (mesh.indices != NULL) rlDrawVertexArrayElements(0, mesh.triangleCount*3, mesh.indices);
        else rlDrawVertexArray(0, mesh.vertexCount);
    rlPopMatrix();

    rlDisableStatePointer(GL_VERTEX_ARRAY);
    rlDisableStatePointer(GL_TEXTURE_COORD_ARRAY);
    rlDisableStatePointer(GL_NORMAL_ARRAY);
    rlDisableStatePointer(GL_COLOR_ARRAY);

    rlDisableTexture();
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Bind shader program
    rlEnableShader(material.shader.id);

    // Send required data to shader (matrices, values)
    //-----------------------------------------------------
    // Upload to shader material.colDiffuse
    if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
    {
        float values[4] = {
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.r/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.g/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.b/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.colSpecular (if location available)
    if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1)
    {
        float values[4] = {
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.r/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.g/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.b/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Get a copy of current matrices to work with,
    // just in case stereo render is required and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Model transformation matrix is send to shader uniform location: SHADER_LOC_MATRIX_MODEL
    if (material.shader.locs[SHADER_LOC_MATRIX_MODEL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MODEL], transform);

    // Accumulate several model transformations:
    //    transform: model transformation provided (includes DrawModel() params combined with model.transform)
    //    rlGetMatrixTransform(): rlgl internal transform matrix due to push/pop matrix stack
    matModel = MatrixMultiply(transform, rlGetMatrixTransform());

    // Get model-view matrix
    matModelView = MatrixMultiply(matModel, matView);

    // Upload model normal matrix (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Enable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
            else rlEnableTexture(material.maps[i].texture.id);

            rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
        }
    }

    // Try binding vertex array objects (VAO)
    // or use VBOs if not possible
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[1]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[2]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
        {
            if (mesh.vboId[3] != 0)
            {
                rlEnableVertexBuffer(mesh.vboId[3]);
                rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC2, 4);
                rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[4]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[5]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh
        if (mesh.indices != NULL) rlDrawVertexArrayElements(0, mesh.triangleCount*3, 0);
        else rlDrawVertexArray(0, mesh.vertexCount);
    }

    // Unbind all binded texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        // Select current shader texture slot
        rlActiveTextureSlot(i);

        // Disable texture for active slot
        if ((i == MATERIAL_MAP_IRRADIANCE) ||
            (i == MATERIAL_MAP_PREFILTER) ||
            (i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
        else rlDisableTexture();
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
#endif
}

// Draw multiple mesh instances with material and different transforms
void DrawMeshInstanced(Mesh mesh, Material material, Matrix *transforms, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Instancing required variables
    float16 *instanceTransforms = NULL;
    unsigned int instancesVboId = 0;

    // Bind shader program
    rlEnableShader(material.shader.id);

    // Send required data to shader (matrices, values)
    //-----------------------------------------------------
    // Upload to shader material.colDiffuse
    if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
    {
        float values[4] = {
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.r/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.g/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.b/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.colSpecular (if location available)
    if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1)
    {
        float values[4] = {
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.r/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.g/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.b/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Get a copy of current matrices to work with,
    // just in case stereo render is required and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Create instances buffer
    instanceTransforms = (float16 *)RL_MALLOC(instances*sizeof(float16));

    // Fill buffer with instances transformations as float16 arrays
    for (int i = 0; i < instances; i++) instanceTransforms[i] = MatrixToFloatV(transforms[i]);

    // Enable mesh VAO to attach new buffer
    rlEnableVertexArray(mesh.vaoId);

    // This could alternatively use a static VBO and either glMapBuffer() or glBufferSubData().
    // It isn't clear which would be reliably faster in all cases and on all platforms,
    // anecdotally glMapBuffer() seems very slow (syncs) while glBufferSubData() seems
    // no faster, since we're transferring all the transform matrices anyway
    instancesVboId = rlLoadVertexBuffer(instanceTransforms, instances*sizeof(float16), false);

    // Instances transformation matrices are send to shader attribute location: SHADER_LOC_MATRIX_MODEL
    for (unsigned int i = 0; i < 4; i++)
    {
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 4, RL_FLOAT, 0, sizeof(Matrix), (void *)(i*sizeof(Vector4)));
        rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 1);
    }

    rlDisableVertexBuffer();
    rlDisableVertexArray();

    // Accumulate internal matrix transform (push/pop) and view matrix
    // NOTE: In this case, model instance transformation must be computed in the shader
    matModelView = MatrixMultiply(rlGetMatrixTransform(), matView);

    // Upload model normal matrix (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Enable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
            else rlEnableTexture(material.maps[i].texture.id);

            rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
        }
    }

    // Try binding vertex array objects (VAO)
    // or use VBOs if not possible
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[1]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[2]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
        {
            if (mesh.vboId[3] != 0)
            {
                rlEnableVertexBuffer(mesh.vboId[3]);
                rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC2, 4);
                rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[4]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[5]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh instanced
        if (mesh.indices != NULL) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount*3, 0, instances);
        else rlDrawVertexArrayInstanced(0, mesh.vertexCount, instances);
    }

    // Unbind all binded texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        // Select current shader texture slot
        rlActiveTextureSlot(i);

        // Disable texture for active slot
        if ((i == MATERIAL_MAP_IRRADIANCE) ||
            (i == MATERIAL_MAP_PREFILTER) ||
            (i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
        else rlDisableTexture();
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Remove instance transforms buffer
    rlUnloadVertexBuffer(instancesVboId);
    RL_FREE(instanceTransforms);
#endif
}

// Unload mesh from memory (RAM and VRAM)
void UnloadMesh(Mesh mesh)
{
    // Unload rlgl mesh vboId data
    rlUnloadVertexArray(mesh.vaoId);

    for (int i = 0; i < MAX_MESH_VERTEX_BUFFERS; i++) rlUnloadVertexBuffer(mesh.vboId[i]);
    RL_FREE(mesh.vboId);

    RL_FREE(mesh.vertices);
    RL_FREE(mesh.texcoords);
    RL_FREE(mesh.normals);
    RL_FREE(mesh.colors);
    RL_FREE(mesh.tangents);
    RL_FREE(mesh.texcoords2);
    RL_FREE(mesh.indices);

    RL_FREE(mesh.animVertices);
    RL_FREE(mesh.animNormals);
    RL_FREE(mesh.boneWeights);
    RL_FREE(mesh.boneIds);
}

// Export mesh data to file
bool ExportMesh(Mesh mesh, const char *fileName)
{
    bool success = false;

    if (IsFileExtension(fileName, ".obj"))
    {
        // Estimated data size, it should be enough...
        int dataSize = mesh.vertexCount/3* (int)strlen("v 0000.00f 0000.00f 0000.00f") +
                       mesh.vertexCount/2* (int)strlen("vt 0.000f 0.00f") +
                       mesh.vertexCount/3* (int)strlen("vn 0.000f 0.00f 0.00f") +
                       mesh.triangleCount/3* (int)strlen("f 00000/00000/00000 00000/00000/00000 00000/00000/00000");

        // NOTE: Text data buffer size is estimated considering mesh data size
        char *txtData = (char *)RL_CALLOC(dataSize + 2000, sizeof(char));

        int byteCount = 0;
        byteCount += sprintf(txtData + byteCount, "# //////////////////////////////////////////////////////////////////////////////////\n");
        byteCount += sprintf(txtData + byteCount, "# //                                                                              //\n");
        byteCount += sprintf(txtData + byteCount, "# // rMeshOBJ exporter v1.0 - Mesh exported as triangle faces and not optimized   //\n");
        byteCount += sprintf(txtData + byteCount, "# //                                                                              //\n");
        byteCount += sprintf(txtData + byteCount, "# // more info and bugs-report:  github.com/raysan5/raylib                        //\n");
        byteCount += sprintf(txtData + byteCount, "# // feedback and support:       ray[at]raylib.com                                //\n");
        byteCount += sprintf(txtData + byteCount, "# //                                                                              //\n");
        byteCount += sprintf(txtData + byteCount, "# // Copyright (c) 2018 Ramon Santamaria (@raysan5)                               //\n");
        byteCount += sprintf(txtData + byteCount, "# //                                                                              //\n");
        byteCount += sprintf(txtData + byteCount, "# //////////////////////////////////////////////////////////////////////////////////\n\n");
        byteCount += sprintf(txtData + byteCount, "# Vertex Count:     %i\n", mesh.vertexCount);
        byteCount += sprintf(txtData + byteCount, "# Triangle Count:   %i\n\n", mesh.triangleCount);

        byteCount += sprintf(txtData + byteCount, "g mesh\n");

        for (int i = 0, v = 0; i < mesh.vertexCount; i++, v += 3)
        {
            byteCount += sprintf(txtData + byteCount, "v %.2f %.2f %.2f\n", mesh.vertices[v], mesh.vertices[v + 1], mesh.vertices[v + 2]);
        }

        for (int i = 0, v = 0; i < mesh.vertexCount; i++, v += 2)
        {
            byteCount += sprintf(txtData + byteCount, "vt %.3f %.3f\n", mesh.texcoords[v], mesh.texcoords[v + 1]);
        }

        for (int i = 0, v = 0; i < mesh.vertexCount; i++, v += 3)
        {
            byteCount += sprintf(txtData + byteCount, "vn %.3f %.3f %.3f\n", mesh.normals[v], mesh.normals[v + 1], mesh.normals[v + 2]);
        }

        for (int i = 0; i < mesh.triangleCount; i += 3)
        {
            byteCount += sprintf(txtData + byteCount, "f %i/%i/%i %i/%i/%i %i/%i/%i\n", i, i, i, i + 1, i + 1, i + 1, i + 2, i + 2, i + 2);
        }

        byteCount += sprintf(txtData + byteCount, "\n");

        // NOTE: Text data length exported is determined by '\0' (NULL) character
        success = SaveFileText(fileName, txtData);

        RL_FREE(txtData);
    }
    else if (IsFileExtension(fileName, ".raw"))
    {
        // TODO: Support additional file formats to export mesh vertex data
    }

    return success;
}


// Load materials from model file
Material *LoadMaterials(const char *fileName, int *materialCount)
{
    Material *materials = NULL;
    unsigned int count = 0;

    // TODO: Support IQM and GLTF for materials parsing

#if defined(SUPPORT_FILEFORMAT_MTL)
    if (IsFileExtension(fileName, ".mtl"))
    {
        tinyobj_material_t *mats = NULL;

        int result = tinyobj_parse_mtl_file(&mats, &count, fileName);
        if (result != TINYOBJ_SUCCESS) TRACELOG(LOG_WARNING, "MATERIAL: [%s] Failed to parse materials file", fileName);

        // TODO: Process materials to return

        tinyobj_materials_free(mats, count);
    }
#else
    TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to load material file", fileName);
#endif

    // Set materials shader to default (DIFFUSE, SPECULAR, NORMAL)
    if (materials != NULL)
    {
        for (unsigned int i = 0; i < count; i++)
        {
            materials[i].shader.id = rlGetShaderIdDefault();
            materials[i].shader.locs = rlGetShaderLocsDefault();
        }
    }

    *materialCount = count;
    return materials;
}

// Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
Material LoadMaterialDefault(void)
{
    Material material = { 0 };
    material.maps = (MaterialMap *)RL_CALLOC(MAX_MATERIAL_MAPS, sizeof(MaterialMap));

    // Using rlgl default shader
    material.shader.id = rlGetShaderIdDefault();
    material.shader.locs = rlGetShaderLocsDefault();

    // Using rlgl default texture (1x1 pixel, UNCOMPRESSED_R8G8B8A8, 1 mipmap)
    material.maps[MATERIAL_MAP_DIFFUSE].texture = (Texture2D){ rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    //material.maps[MATERIAL_MAP_NORMAL].texture;         // NOTE: By default, not set
    //material.maps[MATERIAL_MAP_SPECULAR].texture;       // NOTE: By default, not set

    material.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;    // Diffuse color
    material.maps[MATERIAL_MAP_SPECULAR].color = WHITE;   // Specular color

    return material;
}

// Unload material from memory
void UnloadMaterial(Material material)
{
    // Unload material shader (avoid unloading default shader, managed by raylib)
    if (material.shader.id != rlGetShaderIdDefault()) UnloadShader(material.shader);

    // Unload loaded texture maps (avoid unloading default texture, managed by raylib)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id != rlGetTextureIdDefault()) rlUnloadTexture(material.maps[i].texture.id);
    }

    RL_FREE(material.maps);
}

// Set texture for a material map type (MATERIAL_MAP_DIFFUSE, MATERIAL_MAP_SPECULAR...)
// NOTE: Previous texture should be manually unloaded
void SetMaterialTexture(Material *material, int mapType, Texture2D texture)
{
    material->maps[mapType].texture = texture;
}

// Set the material for a mesh
void SetModelMeshMaterial(Model *model, int meshId, int materialId)
{
    if (meshId >= model->meshCount) TRACELOG(LOG_WARNING, "MESH: Id greater than mesh count");
    else if (materialId >= model->materialCount) TRACELOG(LOG_WARNING, "MATERIAL: Id greater than material count");
    else  model->meshMaterial[meshId] = materialId;
}

// Load model animations from file
ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount)
{
    ModelAnimation *animations = NULL;

#if defined(SUPPORT_FILEFORMAT_IQM)
    if (IsFileExtension(fileName, ".iqm")) animations = LoadIQMModelAnimations(fileName, animCount);
#endif
#if defined(SUPPORT_FILEFORMAT_GLTF)
    if (IsFileExtension(fileName, ".gltf;.glb")) animations = LoadGLTFModelAnimations(fileName, animCount);
#endif

    return animations;
}

// Update model animated vertex data (positions and normals) for a given frame
// NOTE: Updated data is uploaded to GPU
void UpdateModelAnimation(Model model, ModelAnimation anim, int frame)
{
    if ((anim.frameCount > 0) && (anim.bones != NULL) && (anim.framePoses != NULL))
    {
        if (frame >= anim.frameCount) frame = frame%anim.frameCount;

        for (int m = 0; m < model.meshCount; m++)
        {
            Vector3 animVertex = { 0 };
            Vector3 animNormal = { 0 };

            Vector3 inTranslation = { 0 };
            Quaternion inRotation = { 0 };
            //Vector3 inScale = { 0 };      // Not used...

            Vector3 outTranslation = { 0 };
            Quaternion outRotation = { 0 };
            Vector3 outScale = { 0 };

            int vCounter = 0;
            int boneCounter = 0;
            int boneId = 0;
            float boneWeight = 0.0;

            for (int i = 0; i < model.meshes[m].vertexCount; i++)
            {
                model.meshes[m].animVertices[vCounter] = 0;
                model.meshes[m].animVertices[vCounter + 1] = 0;
                model.meshes[m].animVertices[vCounter + 2] = 0;

                model.meshes[m].animNormals[vCounter] = 0;
                model.meshes[m].animNormals[vCounter + 1] = 0;
                model.meshes[m].animNormals[vCounter + 2] = 0;

                for (int j = 0; j < 4; j++)
                {
                    boneId = model.meshes[m].boneIds[boneCounter];
                    boneWeight = model.meshes[m].boneWeights[boneCounter];
                    inTranslation = model.bindPose[boneId].translation;
                    inRotation = model.bindPose[boneId].rotation;
                    //inScale = model.bindPose[boneId].scale;
                    outTranslation = anim.framePoses[frame][boneId].translation;
                    outRotation = anim.framePoses[frame][boneId].rotation;
                    outScale = anim.framePoses[frame][boneId].scale;

                    // Vertices processing
                    // NOTE: We use meshes.vertices (default vertex position) to calculate meshes.animVertices (animated vertex position)
                    animVertex = (Vector3){ model.meshes[m].vertices[vCounter], model.meshes[m].vertices[vCounter + 1], model.meshes[m].vertices[vCounter + 2] };
                    animVertex = Vector3Multiply(animVertex, outScale);
                    animVertex = Vector3Subtract(animVertex, inTranslation);
                    animVertex = Vector3RotateByQuaternion(animVertex, QuaternionMultiply(outRotation, QuaternionInvert(inRotation)));
                    animVertex = Vector3Add(animVertex, outTranslation);
                    model.meshes[m].animVertices[vCounter] += animVertex.x*boneWeight;
                    model.meshes[m].animVertices[vCounter + 1] += animVertex.y*boneWeight;
                    model.meshes[m].animVertices[vCounter + 2] += animVertex.z*boneWeight;

                    // Normals processing
                    // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
                    if (model.meshes[m].normals != NULL)
                    {
                        animNormal = (Vector3){ model.meshes[m].normals[vCounter], model.meshes[m].normals[vCounter + 1], model.meshes[m].normals[vCounter + 2] };
                        animNormal = Vector3RotateByQuaternion(animNormal, QuaternionMultiply(outRotation, QuaternionInvert(inRotation)));
                        model.meshes[m].animNormals[vCounter] += animNormal.x*boneWeight;
                        model.meshes[m].animNormals[vCounter + 1] += animNormal.y*boneWeight;
                        model.meshes[m].animNormals[vCounter + 2] += animNormal.z*boneWeight;
                    }
                    boneCounter += 1;
                }
                vCounter += 3;
            }

            // Upload new vertex data to GPU for model drawing
            rlUpdateVertexBuffer(model.meshes[m].vboId[0], model.meshes[m].animVertices, model.meshes[m].vertexCount*3*sizeof(float), 0);    // Update vertex position
            rlUpdateVertexBuffer(model.meshes[m].vboId[2], model.meshes[m].animNormals, model.meshes[m].vertexCount*3*sizeof(float), 0);     // Update vertex normals
        }
    }
}

// Unload animation array data
void UnloadModelAnimations(ModelAnimation* animations, unsigned int count)
{
    for (unsigned int i = 0; i < count; i++) UnloadModelAnimation(animations[i]);
    RL_FREE(animations);
}

// Unload animation data
void UnloadModelAnimation(ModelAnimation anim)
{
    for (int i = 0; i < anim.frameCount; i++) RL_FREE(anim.framePoses[i]);

    RL_FREE(anim.bones);
    RL_FREE(anim.framePoses);
}

// Check model animation skeleton match
// NOTE: Only number of bones and parent connections are checked
bool IsModelAnimationValid(Model model, ModelAnimation anim)
{
    int result = true;

    if (model.boneCount != anim.boneCount) result = false;
    else
    {
        for (int i = 0; i < model.boneCount; i++)
        {
            if (model.bones[i].parent != anim.bones[i].parent) { result = false; break; }
        }
    }

    return result;
}

#if defined(SUPPORT_MESH_GENERATION)
// Generate polygonal mesh
Mesh GenMeshPoly(int sides, float radius)
{
    Mesh mesh = { 0 };

    if (sides < 3) return mesh;

    int vertexCount = sides*3;

    // Vertices definition
    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));

    float d = 0.0f, dStep = 360.0f/sides;
    for (int v = 0; v < vertexCount; v += 3)
    {
        vertices[v] = (Vector3){ 0.0f, 0.0f, 0.0f };
        vertices[v + 1] = (Vector3){ sinf(DEG2RAD*d)*radius, 0.0f, cosf(DEG2RAD*d)*radius };
        vertices[v + 2] = (Vector3){sinf(DEG2RAD*(d+dStep))*radius, 0.0f, cosf(DEG2RAD*(d+dStep))*radius };
        d += dStep;
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++) normals[n] = (Vector3){ 0.0f, 1.0f, 0.0f };   // Vector3.up;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int n = 0; n < vertexCount; n++) texcoords[n] = (Vector2){ 0.0f, 0.0f };

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = sides;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);

    // Upload vertex data to GPU (static mesh)
    // NOTE: mesh.vboId array is allocated inside UploadMesh()
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
Mesh GenMeshPlane(float width, float length, int resX, int resZ)
{
    Mesh mesh = { 0 };

#define CUSTOM_MESH_GEN_PLANE
#if defined(CUSTOM_MESH_GEN_PLANE)
    resX++;
    resZ++;

    // Vertices definition
    int vertexCount = resX*resZ; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*length;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;
            vertices[x + z*resX] = (Vector3){ xPos, 0.0f, zPos };
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++) normals[n] = (Vector3){ 0.0f, 1.0f, 0.0f };   // Vector3.up;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resZ - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resZ - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face % (resX - 1) + (face/(resZ - 1)*resX);

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resX;
        triangles[t++] = i + resX + 1;
        triangles[t++] = i + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++) mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

#else       // Use par_shapes library to generate plane mesh

    par_shapes_mesh *plane = par_shapes_create_plane(resX, resZ);   // No normals/texcoords generated!!!
    par_shapes_scale(plane, width, length, 1.0f);
    par_shapes_rotate(plane, -PI/2.0f, (float[]){ 1, 0, 0 });
    par_shapes_translate(plane, -width/2, 0.0f, length/2);

    mesh.vertices = (float *)RL_MALLOC(plane->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(plane->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(plane->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = plane->ntriangles*3;
    mesh.triangleCount = plane->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = plane->points[plane->triangles[k]*3];
        mesh.vertices[k*3 + 1] = plane->points[plane->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = plane->points[plane->triangles[k]*3 + 2];

        mesh.normals[k*3] = plane->normals[plane->triangles[k]*3];
        mesh.normals[k*3 + 1] = plane->normals[plane->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = plane->normals[plane->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = plane->tcoords[plane->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = plane->tcoords[plane->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(plane);
#endif

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generated cuboid mesh
Mesh GenMeshCube(float width, float height, float length)
{
    Mesh mesh = { 0 };

#define CUSTOM_MESH_GEN_CUBE
#if defined(CUSTOM_MESH_GEN_CUBE)
    float vertices[] = {
        -width/2, -height/2, length/2,
        width/2, -height/2, length/2,
        width/2, height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, -height/2, -length/2,
        -width/2, height/2, -length/2,
        -width/2, height/2, length/2,
        width/2, height/2, length/2,
        width/2, height/2, -length/2,
        -width/2, -height/2, -length/2,
        width/2, -height/2, -length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, length/2,
        width/2, -height/2, -length/2,
        width/2, height/2, -length/2,
        width/2, height/2, length/2,
        width/2, -height/2, length/2,
        -width/2, -height/2, -length/2,
        -width/2, -height/2, length/2,
        -width/2, height/2, length/2,
        -width/2, height/2, -length/2
    };

    float texcoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };

    mesh.vertices = (float *)RL_MALLOC(24*3*sizeof(float));
    memcpy(mesh.vertices, vertices, 24*3*sizeof(float));

    mesh.texcoords = (float *)RL_MALLOC(24*2*sizeof(float));
    memcpy(mesh.texcoords, texcoords, 24*2*sizeof(float));

    mesh.normals = (float *)RL_MALLOC(24*3*sizeof(float));
    memcpy(mesh.normals, normals, 24*3*sizeof(float));

    mesh.indices = (unsigned short *)RL_MALLOC(36*sizeof(unsigned short));

    int k = 0;

    // Indices can be initialized right now
    for (int i = 0; i < 36; i+=6)
    {
        mesh.indices[i] = 4*k;
        mesh.indices[i+1] = 4*k+1;
        mesh.indices[i+2] = 4*k+2;
        mesh.indices[i+3] = 4*k;
        mesh.indices[i+4] = 4*k+2;
        mesh.indices[i+5] = 4*k+3;

        k++;
    }

    mesh.vertexCount = 24;
    mesh.triangleCount = 12;

#else               // Use par_shapes library to generate cube mesh
/*
// Platonic solids:
par_shapes_mesh* par_shapes_create_tetrahedron();       // 4 sides polyhedron (pyramid)
par_shapes_mesh* par_shapes_create_cube();              // 6 sides polyhedron (cube)
par_shapes_mesh* par_shapes_create_octahedron();        // 8 sides polyhedron (dyamond)
par_shapes_mesh* par_shapes_create_dodecahedron();      // 12 sides polyhedron
par_shapes_mesh* par_shapes_create_icosahedron();       // 20 sides polyhedron
*/
    // Platonic solid generation: cube (6 sides)
    // NOTE: No normals/texcoords generated by default
    par_shapes_mesh *cube = par_shapes_create_cube();
    cube->tcoords = PAR_MALLOC(float, 2*cube->npoints);
    for (int i = 0; i < 2*cube->npoints; i++) cube->tcoords[i] = 0.0f;
    par_shapes_scale(cube, width, height, length);
    par_shapes_translate(cube, -width/2, 0.0f, -length/2);
    par_shapes_compute_normals(cube);

    mesh.vertices = (float *)RL_MALLOC(cube->ntriangles*3*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(cube->ntriangles*3*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(cube->ntriangles*3*3*sizeof(float));

    mesh.vertexCount = cube->ntriangles*3;
    mesh.triangleCount = cube->ntriangles;

    for (int k = 0; k < mesh.vertexCount; k++)
    {
        mesh.vertices[k*3] = cube->points[cube->triangles[k]*3];
        mesh.vertices[k*3 + 1] = cube->points[cube->triangles[k]*3 + 1];
        mesh.vertices[k*3 + 2] = cube->points[cube->triangles[k]*3 + 2];

        mesh.normals[k*3] = cube->normals[cube->triangles[k]*3];
        mesh.normals[k*3 + 1] = cube->normals[cube->triangles[k]*3 + 1];
        mesh.normals[k*3 + 2] = cube->normals[cube->triangles[k]*3 + 2];

        mesh.texcoords[k*2] = cube->tcoords[cube->triangles[k]*2];
        mesh.texcoords[k*2 + 1] = cube->tcoords[cube->triangles[k]*2 + 1];
    }

    par_shapes_free_mesh(cube);
#endif

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate sphere mesh (standard sphere)
Mesh GenMeshSphere(float radius, int rings, int slices)
{
    Mesh mesh = { 0 };

    if ((rings >= 3) && (slices >= 3))
    {
        par_shapes_mesh *sphere = par_shapes_create_parametric_sphere(slices, rings);
        par_shapes_scale(sphere, radius, radius, radius);
        // NOTE: Soft normals are computed internally

        mesh.vertices = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));
        mesh.texcoords = (float *)RL_MALLOC(sphere->ntriangles*3*2*sizeof(float));
        mesh.normals = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));

        mesh.vertexCount = sphere->ntriangles*3;
        mesh.triangleCount = sphere->ntriangles;

        for (int k = 0; k < mesh.vertexCount; k++)
        {
            mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
            mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
            mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];

            mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
            mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
            mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];

            mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
            mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
        }

        par_shapes_free_mesh(sphere);

        // Upload vertex data to GPU (static mesh)
        UploadMesh(&mesh, false);
    }
    else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: sphere");

    return mesh;
}

// Generate hemi-sphere mesh (half sphere, no bottom cap)
Mesh GenMeshHemiSphere(float radius, int rings, int slices)
{
    Mesh mesh = { 0 };

    if ((rings >= 3) && (slices >= 3))
    {
        if (radius < 0.0f) radius = 0.0f;

        par_shapes_mesh *sphere = par_shapes_create_hemisphere(slices, rings);
        par_shapes_scale(sphere, radius, radius, radius);
        // NOTE: Soft normals are computed internally

        mesh.vertices = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));
        mesh.texcoords = (float *)RL_MALLOC(sphere->ntriangles*3*2*sizeof(float));
        mesh.normals = (float *)RL_MALLOC(sphere->ntriangles*3*3*sizeof(float));

        mesh.vertexCount = sphere->ntriangles*3;
        mesh.triangleCount = sphere->ntriangles;

        for (int k = 0; k < mesh.vertexCount; k++)
        {
            mesh.vertices[k*3] = sphere->points[sphere->triangles[k]*3];
            mesh.vertices[k*3 + 1] = sphere->points[sphere->triangles[k]*3 + 1];
            mesh.vertices[k*3 + 2] = sphere->points[sphere->triangles[k]*3 + 2];

            mesh.normals[k*3] = sphere->normals[sphere->triangles[k]*3];
            mesh.normals[k*3 + 1] = sphere->normals[sphere->triangles[k]*3 + 1];
            mesh.normals[k*3 + 2] = sphere->normals[sphere->triangles[k]*3 + 2];

            mesh.texcoords[k*2] = sphere->tcoords[sphere->triangles[k]*2];
            mesh.texcoords[k*2 + 1] = sphere->tcoords[sphere->triangles[k]*2 + 1];
        }

        par_shapes_free_mesh(sphere);

        // Upload vertex data to GPU (static mesh)
        UploadMesh(&mesh, false);
    }
    else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: hemisphere");

    return mesh;
}

// Generate cylinder mesh
Mesh GenMeshCylinder(float radius, float height, int slices)
{
    Mesh mesh = { 0 };

    if (slices >= 3)
    {
        // Instance a cylinder that sits on the Z=0 plane using the given tessellation
        // levels across the UV domain.  Think of "slices" like a number of pizza
        // slices, and "stacks" like a number of stacked rings.
        // Height and radius are both 1.0, but they can easily be changed with par_shapes_scale
        par_shapes_mesh *cylinder = par_shapes_create_cylinder(slices, 8);
        par_shapes_scale(cylinder, radius, radius, height);
        par_shapes_rotate(cylinder, -PI/2.0f, (float[]){ 1, 0, 0 });
        par_shapes_rotate(cylinder, PI/2.0f, (float[]){ 0, 1, 0 });

        // Generate an orientable disk shape (top cap)
        par_shapes_mesh *capTop = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, 1 });
        capTop->tcoords = PAR_MALLOC(float, 2*capTop->npoints);
        for (int i = 0; i < 2*capTop->npoints; i++) capTop->tcoords[i] = 0.0f;
        par_shapes_rotate(capTop, -PI/2.0f, (float[]){ 1, 0, 0 });
        par_shapes_translate(capTop, 0, height, 0);

        // Generate an orientable disk shape (bottom cap)
        par_shapes_mesh *capBottom = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, -1 });
        capBottom->tcoords = PAR_MALLOC(float, 2*capBottom->npoints);
        for (int i = 0; i < 2*capBottom->npoints; i++) capBottom->tcoords[i] = 0.95f;
        par_shapes_rotate(capBottom, PI/2.0f, (float[]){ 1, 0, 0 });

        par_shapes_merge_and_free(cylinder, capTop);
        par_shapes_merge_and_free(cylinder, capBottom);

        mesh.vertices = (float *)RL_MALLOC(cylinder->ntriangles*3*3*sizeof(float));
        mesh.texcoords = (float *)RL_MALLOC(cylinder->ntriangles*3*2*sizeof(float));
        mesh.normals = (float *)RL_MALLOC(cylinder->ntriangles*3*3*sizeof(float));

        mesh.vertexCount = cylinder->ntriangles*3;
        mesh.triangleCount = cylinder->ntriangles;

        for (int k = 0; k < mesh.vertexCount; k++)
        {
            mesh.vertices[k*3] = cylinder->points[cylinder->triangles[k]*3];
            mesh.vertices[k*3 + 1] = cylinder->points[cylinder->triangles[k]*3 + 1];
            mesh.vertices[k*3 + 2] = cylinder->points[cylinder->triangles[k]*3 + 2];

            mesh.normals[k*3] = cylinder->normals[cylinder->triangles[k]*3];
            mesh.normals[k*3 + 1] = cylinder->normals[cylinder->triangles[k]*3 + 1];
            mesh.normals[k*3 + 2] = cylinder->normals[cylinder->triangles[k]*3 + 2];

            mesh.texcoords[k*2] = cylinder->tcoords[cylinder->triangles[k]*2];
            mesh.texcoords[k*2 + 1] = cylinder->tcoords[cylinder->triangles[k]*2 + 1];
        }

        par_shapes_free_mesh(cylinder);

        // Upload vertex data to GPU (static mesh)
        UploadMesh(&mesh, false);
    }
    else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: cylinder");

    return mesh;
}

// Generate cone/pyramid mesh
Mesh GenMeshCone(float radius, float height, int slices)
{
    Mesh mesh = { 0 };

    if (slices >= 3)
    {
        // Instance a cone that sits on the Z=0 plane using the given tessellation
        // levels across the UV domain.  Think of "slices" like a number of pizza
        // slices, and "stacks" like a number of stacked rings.
        // Height and radius are both 1.0, but they can easily be changed with par_shapes_scale
        par_shapes_mesh *cone = par_shapes_create_cone(slices, 8);
        par_shapes_scale(cone, radius, radius, height);
        par_shapes_rotate(cone, -PI/2.0f, (float[]){ 1, 0, 0 });
        par_shapes_rotate(cone, PI/2.0f, (float[]){ 0, 1, 0 });

        // Generate an orientable disk shape (bottom cap)
        par_shapes_mesh *capBottom = par_shapes_create_disk(radius, slices, (float[]){ 0, 0, 0 }, (float[]){ 0, 0, -1 });
        capBottom->tcoords = PAR_MALLOC(float, 2*capBottom->npoints);
        for (int i = 0; i < 2*capBottom->npoints; i++) capBottom->tcoords[i] = 0.95f;
        par_shapes_rotate(capBottom, PI/2.0f, (float[]){ 1, 0, 0 });

        par_shapes_merge_and_free(cone, capBottom);

        mesh.vertices = (float *)RL_MALLOC(cone->ntriangles*3*3*sizeof(float));
        mesh.texcoords = (float *)RL_MALLOC(cone->ntriangles*3*2*sizeof(float));
        mesh.normals = (float *)RL_MALLOC(cone->ntriangles*3*3*sizeof(float));

        mesh.vertexCount = cone->ntriangles*3;
        mesh.triangleCount = cone->ntriangles;

        for (int k = 0; k < mesh.vertexCount; k++)
        {
            mesh.vertices[k*3] = cone->points[cone->triangles[k]*3];
            mesh.vertices[k*3 + 1] = cone->points[cone->triangles[k]*3 + 1];
            mesh.vertices[k*3 + 2] = cone->points[cone->triangles[k]*3 + 2];

            mesh.normals[k*3] = cone->normals[cone->triangles[k]*3];
            mesh.normals[k*3 + 1] = cone->normals[cone->triangles[k]*3 + 1];
            mesh.normals[k*3 + 2] = cone->normals[cone->triangles[k]*3 + 2];

            mesh.texcoords[k*2] = cone->tcoords[cone->triangles[k]*2];
            mesh.texcoords[k*2 + 1] = cone->tcoords[cone->triangles[k]*2 + 1];
        }

        par_shapes_free_mesh(cone);

        // Upload vertex data to GPU (static mesh)
        UploadMesh(&mesh, false);
    }
    else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: cone");

    return mesh;
}

// Generate torus mesh
Mesh GenMeshTorus(float radius, float size, int radSeg, int sides)
{
    Mesh mesh = { 0 };

    if ((sides >= 3) && (radSeg >= 3))
    {
        if (radius > 1.0f) radius = 1.0f;
        else if (radius < 0.1f) radius = 0.1f;

        // Create a donut that sits on the Z=0 plane with the specified inner radius
        // The outer radius can be controlled with par_shapes_scale
        par_shapes_mesh *torus = par_shapes_create_torus(radSeg, sides, radius);
        par_shapes_scale(torus, size/2, size/2, size/2);

        mesh.vertices = (float *)RL_MALLOC(torus->ntriangles*3*3*sizeof(float));
        mesh.texcoords = (float *)RL_MALLOC(torus->ntriangles*3*2*sizeof(float));
        mesh.normals = (float *)RL_MALLOC(torus->ntriangles*3*3*sizeof(float));

        mesh.vertexCount = torus->ntriangles*3;
        mesh.triangleCount = torus->ntriangles;

        for (int k = 0; k < mesh.vertexCount; k++)
        {
            mesh.vertices[k*3] = torus->points[torus->triangles[k]*3];
            mesh.vertices[k*3 + 1] = torus->points[torus->triangles[k]*3 + 1];
            mesh.vertices[k*3 + 2] = torus->points[torus->triangles[k]*3 + 2];

            mesh.normals[k*3] = torus->normals[torus->triangles[k]*3];
            mesh.normals[k*3 + 1] = torus->normals[torus->triangles[k]*3 + 1];
            mesh.normals[k*3 + 2] = torus->normals[torus->triangles[k]*3 + 2];

            mesh.texcoords[k*2] = torus->tcoords[torus->triangles[k]*2];
            mesh.texcoords[k*2 + 1] = torus->tcoords[torus->triangles[k]*2 + 1];
        }

        par_shapes_free_mesh(torus);

        // Upload vertex data to GPU (static mesh)
        UploadMesh(&mesh, false);
    }
    else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: torus");

    return mesh;
}

// Generate trefoil knot mesh
Mesh GenMeshKnot(float radius, float size, int radSeg, int sides)
{
    Mesh mesh = { 0 };

    if ((sides >= 3) && (radSeg >= 3))
    {
        if (radius > 3.0f) radius = 3.0f;
        else if (radius < 0.5f) radius = 0.5f;

        par_shapes_mesh *knot = par_shapes_create_trefoil_knot(radSeg, sides, radius);
        par_shapes_scale(knot, size, size, size);

        mesh.vertices = (float *)RL_MALLOC(knot->ntriangles*3*3*sizeof(float));
        mesh.texcoords = (float *)RL_MALLOC(knot->ntriangles*3*2*sizeof(float));
        mesh.normals = (float *)RL_MALLOC(knot->ntriangles*3*3*sizeof(float));

        mesh.vertexCount = knot->ntriangles*3;
        mesh.triangleCount = knot->ntriangles;

        for (int k = 0; k < mesh.vertexCount; k++)
        {
            mesh.vertices[k*3] = knot->points[knot->triangles[k]*3];
            mesh.vertices[k*3 + 1] = knot->points[knot->triangles[k]*3 + 1];
            mesh.vertices[k*3 + 2] = knot->points[knot->triangles[k]*3 + 2];

            mesh.normals[k*3] = knot->normals[knot->triangles[k]*3];
            mesh.normals[k*3 + 1] = knot->normals[knot->triangles[k]*3 + 1];
            mesh.normals[k*3 + 2] = knot->normals[knot->triangles[k]*3 + 2];

            mesh.texcoords[k*2] = knot->tcoords[knot->triangles[k]*2];
            mesh.texcoords[k*2 + 1] = knot->tcoords[knot->triangles[k]*2 + 1];
        }

        par_shapes_free_mesh(knot);

        // Upload vertex data to GPU (static mesh)
        UploadMesh(&mesh, false);
    }
    else TRACELOG(LOG_WARNING, "MESH: Failed to generate mesh: knot");

    return mesh;
}

// Generate a mesh from heightmap
// NOTE: Vertex data is uploaded to GPU
Mesh GenMeshHeightmap(Image heightmap, Vector3 size)
{
    #define GRAY_VALUE(c) ((c.r+c.g+c.b)/3)

    Mesh mesh = { 0 };

    int mapX = heightmap.width;
    int mapZ = heightmap.height;

    Color *pixels = LoadImageColors(heightmap);

    // NOTE: One vertex per pixel
    mesh.triangleCount = (mapX-1)*(mapZ-1)*2;    // One quad every four pixels

    mesh.vertexCount = mesh.triangleCount*3;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.colors = NULL;

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float
    
    int trisCounter = 0;

    Vector3 scaleFactor = { size.x/mapX, size.y/255.0f, size.z/mapZ };

    Vector3 vA = { 0 };
    Vector3 vB = { 0 };
    Vector3 vC = { 0 };
    Vector3 vN = { 0 };

    for (int z = 0; z < mapZ-1; z++)
    {
        for (int x = 0; x < mapX-1; x++)
        {
            // Fill vertices array with data
            //----------------------------------------------------------

            // one triangle - 3 vertex
            mesh.vertices[vCounter] = (float)x*scaleFactor.x;
            mesh.vertices[vCounter + 1] = (float)GRAY_VALUE(pixels[x + z*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 2] = (float)z*scaleFactor.z;

            mesh.vertices[vCounter + 3] = (float)x*scaleFactor.x;
            mesh.vertices[vCounter + 4] = (float)GRAY_VALUE(pixels[x + (z + 1)*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 5] = (float)(z + 1)*scaleFactor.z;

            mesh.vertices[vCounter + 6] = (float)(x + 1)*scaleFactor.x;
            mesh.vertices[vCounter + 7] = (float)GRAY_VALUE(pixels[(x + 1) + z*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 8] = (float)z*scaleFactor.z;

            // another triangle - 3 vertex
            mesh.vertices[vCounter + 9] = mesh.vertices[vCounter + 6];
            mesh.vertices[vCounter + 10] = mesh.vertices[vCounter + 7];
            mesh.vertices[vCounter + 11] = mesh.vertices[vCounter + 8];

            mesh.vertices[vCounter + 12] = mesh.vertices[vCounter + 3];
            mesh.vertices[vCounter + 13] = mesh.vertices[vCounter + 4];
            mesh.vertices[vCounter + 14] = mesh.vertices[vCounter + 5];

            mesh.vertices[vCounter + 15] = (float)(x + 1)*scaleFactor.x;
            mesh.vertices[vCounter + 16] = (float)GRAY_VALUE(pixels[(x + 1) + (z + 1)*mapX])*scaleFactor.y;
            mesh.vertices[vCounter + 17] = (float)(z + 1)*scaleFactor.z;
            vCounter += 18;     // 6 vertex, 18 floats

            // Fill texcoords array with data
            //--------------------------------------------------------------
            mesh.texcoords[tcCounter] = (float)x/(mapX - 1);
            mesh.texcoords[tcCounter + 1] = (float)z/(mapZ - 1);

            mesh.texcoords[tcCounter + 2] = (float)x/(mapX - 1);
            mesh.texcoords[tcCounter + 3] = (float)(z + 1)/(mapZ - 1);

            mesh.texcoords[tcCounter + 4] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[tcCounter + 5] = (float)z/(mapZ - 1);

            mesh.texcoords[tcCounter + 6] = mesh.texcoords[tcCounter + 4];
            mesh.texcoords[tcCounter + 7] = mesh.texcoords[tcCounter + 5];

            mesh.texcoords[tcCounter + 8] = mesh.texcoords[tcCounter + 2];
            mesh.texcoords[tcCounter + 9] = mesh.texcoords[tcCounter + 3];

            mesh.texcoords[tcCounter + 10] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[tcCounter + 11] = (float)(z + 1)/(mapZ - 1);
            tcCounter += 12;    // 6 texcoords, 12 floats

            // Fill normals array with data
            //--------------------------------------------------------------
            for (int i = 0; i < 18; i += 9)
            {
                vA.x = mesh.vertices[nCounter + i];
                vA.y = mesh.vertices[nCounter + i + 1];
                vA.z = mesh.vertices[nCounter + i + 2];

                vB.x = mesh.vertices[nCounter + i + 3];
                vB.y = mesh.vertices[nCounter + i + 4];
                vB.z = mesh.vertices[nCounter + i + 5];

                vC.x = mesh.vertices[nCounter + i + 6];
                vC.y = mesh.vertices[nCounter + i + 7];
                vC.z = mesh.vertices[nCounter + i + 8];

                vN = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(vB, vA), Vector3Subtract(vC, vA)));

                mesh.normals[nCounter + i] = vN.x;
                mesh.normals[nCounter + i + 1] = vN.y;
                mesh.normals[nCounter + i + 2] = vN.z;

                mesh.normals[nCounter + i + 3] = vN.x;
                mesh.normals[nCounter + i + 4] = vN.y;
                mesh.normals[nCounter + i + 5] = vN.z;

                mesh.normals[nCounter + i + 6] = vN.x;
                mesh.normals[nCounter + i + 7] = vN.y;
                mesh.normals[nCounter + i + 8] = vN.z;
            }

            nCounter += 18;     // 6 vertex, 18 floats
            trisCounter += 2;
        }
    }

    UnloadImageColors(pixels);  // Unload pixels color data

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate a cubes mesh from pixel data
// NOTE: Vertex data is uploaded to GPU
Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize)
{
    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    Mesh mesh = { 0 };

    Color *pixels = LoadImageColors(cubicmap);

    int mapWidth = cubicmap.width;
    int mapHeight = cubicmap.height;

    // NOTE: Max possible number of triangles numCubes*(12 triangles by cube)
    int maxTriangles = cubicmap.width*cubicmap.height*12;

    int vCounter = 0;       // Used to count vertices
    int tcCounter = 0;      // Used to count texcoords
    int nCounter = 0;       // Used to count normals

    float w = cubeSize.x;
    float h = cubeSize.z;
    float h2 = cubeSize.y;

    Vector3 *mapVertices = (Vector3 *)RL_MALLOC(maxTriangles*3*sizeof(Vector3));
    Vector2 *mapTexcoords = (Vector2 *)RL_MALLOC(maxTriangles*3*sizeof(Vector2));
    Vector3 *mapNormals = (Vector3 *)RL_MALLOC(maxTriangles*3*sizeof(Vector3));

    // Define the 6 normals of the cube, we will combine them accordingly later...
    Vector3 n1 = { 1.0f, 0.0f, 0.0f };
    Vector3 n2 = { -1.0f, 0.0f, 0.0f };
    Vector3 n3 = { 0.0f, 1.0f, 0.0f };
    Vector3 n4 = { 0.0f, -1.0f, 0.0f };
    Vector3 n5 = { 0.0f, 0.0f, -1.0f };
    Vector3 n6 = { 0.0f, 0.0f, 1.0f };

    // NOTE: We use texture rectangles to define different textures for top-bottom-front-back-right-left (6)
    typedef struct RectangleF {
        float x;
        float y;
        float width;
        float height;
    } RectangleF;

    RectangleF rightTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    RectangleF leftTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    RectangleF frontTexUV = { 0.0f, 0.0f, 0.5f, 0.5f };
    RectangleF backTexUV = { 0.5f, 0.0f, 0.5f, 0.5f };
    RectangleF topTexUV = { 0.0f, 0.5f, 0.5f, 0.5f };
    RectangleF bottomTexUV = { 0.5f, 0.5f, 0.5f, 0.5f };

    for (int z = 0; z < mapHeight; ++z)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            // Define the 8 vertex of the cube, we will combine them accordingly later...
            Vector3 v1 = { w*(x - 0.5f), h2, h*(z - 0.5f) };
            Vector3 v2 = { w*(x - 0.5f), h2, h*(z + 0.5f) };
            Vector3 v3 = { w*(x + 0.5f), h2, h*(z + 0.5f) };
            Vector3 v4 = { w*(x + 0.5f), h2, h*(z - 0.5f) };
            Vector3 v5 = { w*(x + 0.5f), 0, h*(z - 0.5f) };
            Vector3 v6 = { w*(x - 0.5f), 0, h*(z - 0.5f) };
            Vector3 v7 = { w*(x - 0.5f), 0, h*(z + 0.5f) };
            Vector3 v8 = { w*(x + 0.5f), 0, h*(z + 0.5f) };

            // We check pixel color to be WHITE -> draw full cube
            if (COLOR_EQUAL(pixels[z*cubicmap.width + x], WHITE))
            {
                // Define triangles and checking collateral cubes
                //------------------------------------------------

                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                // WARNING: Not required for a WHITE cubes, created to allow seeing the map from outside
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v2;
                mapVertices[vCounter + 2] = v3;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v3;
                mapVertices[vCounter + 5] = v4;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v8;
                mapVertices[vCounter + 2] = v7;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v5;
                mapVertices[vCounter + 5] = v8;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                tcCounter += 6;

                // Checking cube on bottom of current cube
                if (((z < cubicmap.height - 1) && COLOR_EQUAL(pixels[(z + 1)*cubicmap.width + x], BLACK)) || (z == cubicmap.height - 1))
                {
                    // Define front triangles (2 tris, 6 vertex) --> v2 v7 v3, v3 v7 v8
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v2;
                    mapVertices[vCounter + 1] = v7;
                    mapVertices[vCounter + 2] = v3;
                    mapVertices[vCounter + 3] = v3;
                    mapVertices[vCounter + 4] = v7;
                    mapVertices[vCounter + 5] = v8;
                    vCounter += 6;

                    mapNormals[nCounter] = n6;
                    mapNormals[nCounter + 1] = n6;
                    mapNormals[nCounter + 2] = n6;
                    mapNormals[nCounter + 3] = n6;
                    mapNormals[nCounter + 4] = n6;
                    mapNormals[nCounter + 5] = n6;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ frontTexUV.x, frontTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ frontTexUV.x, frontTexUV.y + frontTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ frontTexUV.x + frontTexUV.width, frontTexUV.y + frontTexUV.height };
                    tcCounter += 6;
                }

                // Checking cube on top of current cube
                if (((z > 0) && COLOR_EQUAL(pixels[(z - 1)*cubicmap.width + x], BLACK)) || (z == 0))
                {
                    // Define back triangles (2 tris, 6 vertex) --> v1 v5 v6, v1 v4 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1;
                    mapVertices[vCounter + 1] = v5;
                    mapVertices[vCounter + 2] = v6;
                    mapVertices[vCounter + 3] = v1;
                    mapVertices[vCounter + 4] = v4;
                    mapVertices[vCounter + 5] = v5;
                    vCounter += 6;

                    mapNormals[nCounter] = n5;
                    mapNormals[nCounter + 1] = n5;
                    mapNormals[nCounter + 2] = n5;
                    mapNormals[nCounter + 3] = n5;
                    mapNormals[nCounter + 4] = n5;
                    mapNormals[nCounter + 5] = n5;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y + backTexUV.height };
                    mapTexcoords[tcCounter + 3] = (Vector2){ backTexUV.x + backTexUV.width, backTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ backTexUV.x, backTexUV.y };
                    mapTexcoords[tcCounter + 5] = (Vector2){ backTexUV.x, backTexUV.y + backTexUV.height };
                    tcCounter += 6;
                }

                // Checking cube on right of current cube
                if (((x < cubicmap.width - 1) && COLOR_EQUAL(pixels[z*cubicmap.width + (x + 1)], BLACK)) || (x == cubicmap.width - 1))
                {
                    // Define right triangles (2 tris, 6 vertex) --> v3 v8 v4, v4 v8 v5
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v3;
                    mapVertices[vCounter + 1] = v8;
                    mapVertices[vCounter + 2] = v4;
                    mapVertices[vCounter + 3] = v4;
                    mapVertices[vCounter + 4] = v8;
                    mapVertices[vCounter + 5] = v5;
                    vCounter += 6;

                    mapNormals[nCounter] = n1;
                    mapNormals[nCounter + 1] = n1;
                    mapNormals[nCounter + 2] = n1;
                    mapNormals[nCounter + 3] = n1;
                    mapNormals[nCounter + 4] = n1;
                    mapNormals[nCounter + 5] = n1;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ rightTexUV.x, rightTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ rightTexUV.x, rightTexUV.y + rightTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ rightTexUV.x + rightTexUV.width, rightTexUV.y + rightTexUV.height };
                    tcCounter += 6;
                }

                // Checking cube on left of current cube
                if (((x > 0) && COLOR_EQUAL(pixels[z*cubicmap.width + (x - 1)], BLACK)) || (x == 0))
                {
                    // Define left triangles (2 tris, 6 vertex) --> v1 v7 v2, v1 v6 v7
                    // NOTE: Collateral occluded faces are not generated
                    mapVertices[vCounter] = v1;
                    mapVertices[vCounter + 1] = v7;
                    mapVertices[vCounter + 2] = v2;
                    mapVertices[vCounter + 3] = v1;
                    mapVertices[vCounter + 4] = v6;
                    mapVertices[vCounter + 5] = v7;
                    vCounter += 6;

                    mapNormals[nCounter] = n2;
                    mapNormals[nCounter + 1] = n2;
                    mapNormals[nCounter + 2] = n2;
                    mapNormals[nCounter + 3] = n2;
                    mapNormals[nCounter + 4] = n2;
                    mapNormals[nCounter + 5] = n2;
                    nCounter += 6;

                    mapTexcoords[tcCounter] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 1] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 2] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y };
                    mapTexcoords[tcCounter + 3] = (Vector2){ leftTexUV.x, leftTexUV.y };
                    mapTexcoords[tcCounter + 4] = (Vector2){ leftTexUV.x, leftTexUV.y + leftTexUV.height };
                    mapTexcoords[tcCounter + 5] = (Vector2){ leftTexUV.x + leftTexUV.width, leftTexUV.y + leftTexUV.height };
                    tcCounter += 6;
                }
            }
            // We check pixel color to be BLACK, we will only draw floor and roof
            else if (COLOR_EQUAL(pixels[z*cubicmap.width + x], BLACK))
            {
                // Define top triangles (2 tris, 6 vertex --> v1-v2-v3, v1-v3-v4)
                mapVertices[vCounter] = v1;
                mapVertices[vCounter + 1] = v3;
                mapVertices[vCounter + 2] = v2;
                mapVertices[vCounter + 3] = v1;
                mapVertices[vCounter + 4] = v4;
                mapVertices[vCounter + 5] = v3;
                vCounter += 6;

                mapNormals[nCounter] = n4;
                mapNormals[nCounter + 1] = n4;
                mapNormals[nCounter + 2] = n4;
                mapNormals[nCounter + 3] = n4;
                mapNormals[nCounter + 4] = n4;
                mapNormals[nCounter + 5] = n4;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ topTexUV.x, topTexUV.y + topTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ topTexUV.x, topTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y };
                mapTexcoords[tcCounter + 5] = (Vector2){ topTexUV.x + topTexUV.width, topTexUV.y + topTexUV.height };
                tcCounter += 6;

                // Define bottom triangles (2 tris, 6 vertex --> v6-v8-v7, v6-v5-v8)
                mapVertices[vCounter] = v6;
                mapVertices[vCounter + 1] = v7;
                mapVertices[vCounter + 2] = v8;
                mapVertices[vCounter + 3] = v6;
                mapVertices[vCounter + 4] = v8;
                mapVertices[vCounter + 5] = v5;
                vCounter += 6;

                mapNormals[nCounter] = n3;
                mapNormals[nCounter + 1] = n3;
                mapNormals[nCounter + 2] = n3;
                mapNormals[nCounter + 3] = n3;
                mapNormals[nCounter + 4] = n3;
                mapNormals[nCounter + 5] = n3;
                nCounter += 6;

                mapTexcoords[tcCounter] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 1] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 2] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 3] = (Vector2){ bottomTexUV.x + bottomTexUV.width, bottomTexUV.y };
                mapTexcoords[tcCounter + 4] = (Vector2){ bottomTexUV.x, bottomTexUV.y + bottomTexUV.height };
                mapTexcoords[tcCounter + 5] = (Vector2){ bottomTexUV.x, bottomTexUV.y };
                tcCounter += 6;
            }
        }
    }

    // Move data from mapVertices temp arays to vertices float array
    mesh.vertexCount = vCounter;
    mesh.triangleCount = vCounter/3;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.colors = NULL;

    int fCounter = 0;

    // Move vertices data
    for (int i = 0; i < vCounter; i++)
    {
        mesh.vertices[fCounter] = mapVertices[i].x;
        mesh.vertices[fCounter + 1] = mapVertices[i].y;
        mesh.vertices[fCounter + 2] = mapVertices[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move normals data
    for (int i = 0; i < nCounter; i++)
    {
        mesh.normals[fCounter] = mapNormals[i].x;
        mesh.normals[fCounter + 1] = mapNormals[i].y;
        mesh.normals[fCounter + 2] = mapNormals[i].z;
        fCounter += 3;
    }

    fCounter = 0;

    // Move texcoords data
    for (int i = 0; i < tcCounter; i++)
    {
        mesh.texcoords[fCounter] = mapTexcoords[i].x;
        mesh.texcoords[fCounter + 1] = mapTexcoords[i].y;
        fCounter += 2;
    }

    RL_FREE(mapVertices);
    RL_FREE(mapNormals);
    RL_FREE(mapTexcoords);

    UnloadImageColors(pixels);   // Unload pixels color data

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}
#endif      // SUPPORT_MESH_GENERATION

// Compute mesh bounding box limits
// NOTE: minVertex and maxVertex should be transformed by model transform matrix
BoundingBox GetMeshBoundingBox(Mesh mesh)
{
    // Get min and max vertex to construct bounds (AABB)
    Vector3 minVertex = { 0 };
    Vector3 maxVertex = { 0 };

    if (mesh.vertices != NULL)
    {
        minVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
        maxVertex = (Vector3){ mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };

        for (int i = 1; i < mesh.vertexCount; i++)
        {
            minVertex = Vector3Min(minVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
            maxVertex = Vector3Max(maxVertex, (Vector3){ mesh.vertices[i*3], mesh.vertices[i*3 + 1], mesh.vertices[i*3 + 2] });
        }
    }

    // Create the bounding box
    BoundingBox box = { 0 };
    box.min = minVertex;
    box.max = maxVertex;

    return box;
}

// Compute mesh tangents
// NOTE: To calculate mesh tangents and binormals we need mesh vertex positions and texture coordinates
// Implementation base don: https://answers.unity.com/questions/7789/calculating-tangents-vector4.html
void GenMeshTangents(Mesh *mesh)
{
    if (mesh->tangents == NULL) mesh->tangents = (float *)RL_MALLOC(mesh->vertexCount*4*sizeof(float));
    else
    {
        RL_FREE(mesh->tangents);
        mesh->tangents = (float *)RL_MALLOC(mesh->vertexCount*4*sizeof(float));
    }

    Vector3 *tan1 = (Vector3 *)RL_MALLOC(mesh->vertexCount*sizeof(Vector3));
    Vector3 *tan2 = (Vector3 *)RL_MALLOC(mesh->vertexCount*sizeof(Vector3));

    for (int i = 0; i < mesh->vertexCount; i += 3)
    {
        // Get triangle vertices
        Vector3 v1 = { mesh->vertices[(i + 0)*3 + 0], mesh->vertices[(i + 0)*3 + 1], mesh->vertices[(i + 0)*3 + 2] };
        Vector3 v2 = { mesh->vertices[(i + 1)*3 + 0], mesh->vertices[(i + 1)*3 + 1], mesh->vertices[(i + 1)*3 + 2] };
        Vector3 v3 = { mesh->vertices[(i + 2)*3 + 0], mesh->vertices[(i + 2)*3 + 1], mesh->vertices[(i + 2)*3 + 2] };

        // Get triangle texcoords
        Vector2 uv1 = { mesh->texcoords[(i + 0)*2 + 0], mesh->texcoords[(i + 0)*2 + 1] };
        Vector2 uv2 = { mesh->texcoords[(i + 1)*2 + 0], mesh->texcoords[(i + 1)*2 + 1] };
        Vector2 uv3 = { mesh->texcoords[(i + 2)*2 + 0], mesh->texcoords[(i + 2)*2 + 1] };

        float x1 = v2.x - v1.x;
        float y1 = v2.y - v1.y;
        float z1 = v2.z - v1.z;
        float x2 = v3.x - v1.x;
        float y2 = v3.y - v1.y;
        float z2 = v3.z - v1.z;

        float s1 = uv2.x - uv1.x;
        float t1 = uv2.y - uv1.y;
        float s2 = uv3.x - uv1.x;
        float t2 = uv3.y - uv1.y;

        float div = s1*t2 - s2*t1;
        float r = (div == 0.0f)? 0.0f : 1.0f/div;

        Vector3 sdir = { (t2*x1 - t1*x2)*r, (t2*y1 - t1*y2)*r, (t2*z1 - t1*z2)*r };
        Vector3 tdir = { (s1*x2 - s2*x1)*r, (s1*y2 - s2*y1)*r, (s1*z2 - s2*z1)*r };

        tan1[i + 0] = sdir;
        tan1[i + 1] = sdir;
        tan1[i + 2] = sdir;

        tan2[i + 0] = tdir;
        tan2[i + 1] = tdir;
        tan2[i + 2] = tdir;
    }

    // Compute tangents considering normals
    for (int i = 0; i < mesh->vertexCount; i++)
    {
        Vector3 normal = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };
        Vector3 tangent = tan1[i];

        // TODO: Review, not sure if tangent computation is right, just used reference proposed maths...
#if defined(COMPUTE_TANGENTS_METHOD_01)
        Vector3 tmp = Vector3Subtract(tangent, Vector3Scale(normal, Vector3DotProduct(normal, tangent)));
        tmp = Vector3Normalize(tmp);
        mesh->tangents[i*4 + 0] = tmp.x;
        mesh->tangents[i*4 + 1] = tmp.y;
        mesh->tangents[i*4 + 2] = tmp.z;
        mesh->tangents[i*4 + 3] = 1.0f;
#else
        Vector3OrthoNormalize(&normal, &tangent);
        mesh->tangents[i*4 + 0] = tangent.x;
        mesh->tangents[i*4 + 1] = tangent.y;
        mesh->tangents[i*4 + 2] = tangent.z;
        mesh->tangents[i*4 + 3] = (Vector3DotProduct(Vector3CrossProduct(normal, tangent), tan2[i]) < 0.0f)? -1.0f : 1.0f;
#endif
    }

    RL_FREE(tan1);
    RL_FREE(tan2);

    if (mesh->vboId != NULL)
    {
        if (mesh->vboId[SHADER_LOC_VERTEX_TANGENT] != 0)
        {
            // Upate existing vertex buffer
            rlUpdateVertexBuffer(mesh->vboId[SHADER_LOC_VERTEX_TANGENT], mesh->tangents, mesh->vertexCount*4*sizeof(float), 0);
        }
        else
        {
            // Load a new tangent attributes buffer
            mesh->vboId[SHADER_LOC_VERTEX_TANGENT] = rlLoadVertexBuffer(mesh->tangents, mesh->vertexCount*4*sizeof(float), false);
        }

        rlEnableVertexArray(mesh->vaoId);
        rlSetVertexAttribute(4, 4, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(4);
        rlDisableVertexArray();
    }

    TRACELOG(LOG_INFO, "MESH: Tangents data computed and uploaded for provided mesh");
}

// Compute mesh binormals (aka bitangent)
void GenMeshBinormals(Mesh *mesh)
{
    for (int i = 0; i < mesh->vertexCount; i++)
    {
        //Vector3 normal = { mesh->normals[i*3 + 0], mesh->normals[i*3 + 1], mesh->normals[i*3 + 2] };
        //Vector3 tangent = { mesh->tangents[i*4 + 0], mesh->tangents[i*4 + 1], mesh->tangents[i*4 + 2] };
        //Vector3 binormal = Vector3Scale(Vector3CrossProduct(normal, tangent), mesh->tangents[i*4 + 3]);

        // TODO: Register computed binormal in mesh->binormal?
    }
}

// Draw a model (with texture if set)
void DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };

    DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}

// Draw a model with extended parameters
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)((((float)color.r/255.0)*((float)tint.r/255.0))*255.0f);
        colorTint.g = (unsigned char)((((float)color.g/255.0)*((float)tint.g/255.0))*255.0f);
        colorTint.b = (unsigned char)((((float)color.b/255.0)*((float)tint.b/255.0))*255.0f);
        colorTint.a = (unsigned char)((((float)color.a/255.0)*((float)tint.a/255.0))*255.0f);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}

// Draw a model wires (with texture if set)
void DrawModelWires(Model model, Vector3 position, float scale, Color tint)
{
    rlEnableWireMode();

    DrawModel(model, position, scale, tint);

    rlDisableWireMode();
}

// Draw a model wires (with texture if set) with extended parameters
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    rlEnableWireMode();

    DrawModelEx(model, position, rotationAxis, rotationAngle, scale, tint);

    rlDisableWireMode();
}

// Draw a billboard
void DrawBillboard(Camera camera, Texture2D texture, Vector3 position, float size, Color tint)
{
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };

    DrawBillboardRec(camera, texture, source, position, (Vector2){ size, size }, tint);
}

// Draw a billboard (part of a texture defined by a rectangle)
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint)
{
    DrawBillboardPro(camera, texture, source, position, size, Vector2Zero(), 0.0f, tint);
}

void DrawBillboardPro(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Vector2 origin, float rotation, Color tint)
{
    // NOTE: Billboard size will maintain source rectangle aspect ratio, size will represent billboard width
    Vector2 sizeRatio = { size.y, size.x*(float)source.height/source.width };

    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    Vector3 right = { matView.m0, matView.m4, matView.m8 };
    //Vector3 up = { matView.m1, matView.m5, matView.m9 };

    // NOTE: Billboard locked on axis-Y
    Vector3 up = { 0.0f, 1.0f, 0.0f };

    Vector3 rightScaled = Vector3Scale(right, sizeRatio.x/2);
    Vector3 upScaled = Vector3Scale(up, sizeRatio.y/2);

    Vector3 p1 = Vector3Add(rightScaled, upScaled);
    Vector3 p2 = Vector3Subtract(rightScaled, upScaled);

    Vector3 topLeft = Vector3Scale(p2, -1);
    Vector3 topRight = p1;
    Vector3 bottomRight = p2;
    Vector3 bottomLeft = Vector3Scale(p1, -1);

    if (rotation != 0.0f)
    {
        float sinRotation = sinf(rotation*DEG2RAD);
        float cosRotation = cosf(rotation*DEG2RAD);

        // NOTE: (-1, 1) is the range where origin.x, origin.y is inside the texture
        float rotateAboutX = sizeRatio.x*origin.x/2;
        float rotateAboutY = sizeRatio.y*origin.y/2;

        float xtvalue, ytvalue;
        float rotatedX, rotatedY;

        xtvalue = Vector3DotProduct(right, topLeft) - rotateAboutX; // Project points to x and y coordinates on the billboard plane
        ytvalue = Vector3DotProduct(up, topLeft) - rotateAboutY;
        rotatedX = xtvalue*cosRotation - ytvalue*sinRotation + rotateAboutX; // Rotate about the point origin
        rotatedY = xtvalue*sinRotation + ytvalue*cosRotation + rotateAboutY;
        topLeft = Vector3Add(Vector3Scale(up, rotatedY), Vector3Scale(right, rotatedX)); // Translate back to cartesian coordinates

        xtvalue = Vector3DotProduct(right, topRight) - rotateAboutX;
        ytvalue = Vector3DotProduct(up, topRight) - rotateAboutY;
        rotatedX = xtvalue*cosRotation - ytvalue*sinRotation + rotateAboutX;
        rotatedY = xtvalue*sinRotation + ytvalue*cosRotation + rotateAboutY;
        topRight = Vector3Add(Vector3Scale(up, rotatedY), Vector3Scale(right, rotatedX));

        xtvalue = Vector3DotProduct(right, bottomRight) - rotateAboutX;
        ytvalue = Vector3DotProduct(up, bottomRight) - rotateAboutY;
        rotatedX = xtvalue*cosRotation - ytvalue*sinRotation + rotateAboutX;
        rotatedY = xtvalue*sinRotation + ytvalue*cosRotation + rotateAboutY;
        bottomRight = Vector3Add(Vector3Scale(up, rotatedY), Vector3Scale(right, rotatedX));

        xtvalue = Vector3DotProduct(right, bottomLeft)-rotateAboutX;
        ytvalue = Vector3DotProduct(up, bottomLeft)-rotateAboutY;
        rotatedX = xtvalue*cosRotation - ytvalue*sinRotation + rotateAboutX;
        rotatedY = xtvalue*sinRotation + ytvalue*cosRotation + rotateAboutY;
        bottomLeft = Vector3Add(Vector3Scale(up, rotatedY), Vector3Scale(right, rotatedX));
    }

    // Translate points to the draw center (position)
    topLeft = Vector3Add(topLeft, position);
    topRight = Vector3Add(topRight, position);
    bottomRight = Vector3Add(bottomRight, position);
    bottomLeft = Vector3Add(bottomLeft, position);

    rlCheckRenderBatchLimit(4);

    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        // Bottom-left corner for texture and quad
        rlTexCoord2f((float)source.x/texture.width, (float)source.y/texture.height);
        rlVertex3f(topLeft.x, topLeft.y, topLeft.z);

        // Top-left corner for texture and quad
        rlTexCoord2f((float)source.x/texture.width, (float)(source.y + source.height)/texture.height);
        rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);

        // Top-right corner for texture and quad
        rlTexCoord2f((float)(source.x + source.width)/texture.width, (float)(source.y + source.height)/texture.height);
        rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

        // Bottom-right corner for texture and quad
        rlTexCoord2f((float)(source.x + source.width)/texture.width, (float)source.y/texture.height);
        rlVertex3f(topRight.x, topRight.y, topRight.z);
    rlEnd();

    rlSetTexture(0);
}

// Draw a bounding box with wires
void DrawBoundingBox(BoundingBox box, Color color)
{
    Vector3 size = { 0 };

    size.x = fabsf(box.max.x - box.min.x);
    size.y = fabsf(box.max.y - box.min.y);
    size.z = fabsf(box.max.z - box.min.z);

    Vector3 center = { box.min.x + size.x/2.0f, box.min.y + size.y/2.0f, box.min.z + size.z/2.0f };

    DrawCubeWires(center, size.x, size.y, size.z, color);
}

// Check collision between two spheres
bool CheckCollisionSpheres(Vector3 center1, float radius1, Vector3 center2, float radius2)
{
    bool collision = false;

    // Simple way to check for collision, just checking distance between two points
    // Unfortunately, sqrtf() is a costly operation, so we avoid it with following solution
    /*
    float dx = center1.x - center2.x;      // X distance between centers
    float dy = center1.y - center2.y;      // Y distance between centers
    float dz = center1.z - center2.z;      // Z distance between centers

    float distance = sqrtf(dx*dx + dy*dy + dz*dz);  // Distance between centers

    if (distance <= (radius1 + radius2)) collision = true;
    */

    // Check for distances squared to avoid sqrtf()
    if (Vector3DotProduct(Vector3Subtract(center2, center1), Vector3Subtract(center2, center1)) <= (radius1 + radius2)*(radius1 + radius2)) collision = true;

    return collision;
}

// Check collision between two boxes
// NOTE: Boxes are defined by two points minimum and maximum
bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2)
{
    bool collision = true;

    if ((box1.max.x >= box2.min.x) && (box1.min.x <= box2.max.x))
    {
        if ((box1.max.y < box2.min.y) || (box1.min.y > box2.max.y)) collision = false;
        if ((box1.max.z < box2.min.z) || (box1.min.z > box2.max.z)) collision = false;
    }
    else collision = false;

    return collision;
}

// Check collision between box and sphere
bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius)
{
    bool collision = false;

    float dmin = 0;

    if (center.x < box.min.x) dmin += powf(center.x - box.min.x, 2);
    else if (center.x > box.max.x) dmin += powf(center.x - box.max.x, 2);

    if (center.y < box.min.y) dmin += powf(center.y - box.min.y, 2);
    else if (center.y > box.max.y) dmin += powf(center.y - box.max.y, 2);

    if (center.z < box.min.z) dmin += powf(center.z - box.min.z, 2);
    else if (center.z > box.max.z) dmin += powf(center.z - box.max.z, 2);

    if (dmin <= (radius*radius)) collision = true;

    return collision;
}

// Get collision info between ray and sphere
RayCollision GetRayCollisionSphere(Ray ray, Vector3 center, float radius)
{
    RayCollision collision = { 0 };

    Vector3 raySpherePos = Vector3Subtract(center, ray.position);
    float vector = Vector3DotProduct(raySpherePos, ray.direction);
    float distance = Vector3Length(raySpherePos);
    float d = radius*radius - (distance*distance - vector*vector);

    collision.hit = d >= 0.0f;

    // Check if ray origin is inside the sphere to calculate the correct collision point
    if (distance < radius)
    {
        collision.distance = vector + sqrtf(d);

        // Calculate collision point
        collision.point = Vector3Add(ray.position, Vector3Scale(ray.direction, collision.distance));

        // Calculate collision normal (pointing outwards)
        collision.normal = Vector3Negate(Vector3Normalize(Vector3Subtract(collision.point, center)));
    }
    else
    {
        collision.distance = vector - sqrtf(d);

        // Calculate collision point
        collision.point = Vector3Add(ray.position, Vector3Scale(ray.direction, collision.distance));

        // Calculate collision normal (pointing inwards)
        collision.normal = Vector3Normalize(Vector3Subtract(collision.point, center));
    }

    return collision;
}

// Get collision info between ray and box
RayCollision GetRayCollisionBox(Ray ray, BoundingBox box)
{
    RayCollision collision = { 0 };

    // Note: If ray.position is inside the box, the distance is negative (as if the ray was reversed)
    // Reversing ray.direction will give use the correct result.
    bool insideBox = (ray.position.x > box.min.x) && (ray.position.x < box.max.x) &&
                     (ray.position.y > box.min.y) && (ray.position.y < box.max.y) &&
                     (ray.position.z > box.min.z) && (ray.position.z < box.max.z);

    if (insideBox) ray.direction = Vector3Negate(ray.direction);

    float t[11] = { 0 };

    t[8] = 1.0f/ray.direction.x;
    t[9] = 1.0f/ray.direction.y;
    t[10] = 1.0f/ray.direction.z;

    t[0] = (box.min.x - ray.position.x)*t[8];
    t[1] = (box.max.x - ray.position.x)*t[8];
    t[2] = (box.min.y - ray.position.y)*t[9];
    t[3] = (box.max.y - ray.position.y)*t[9];
    t[4] = (box.min.z - ray.position.z)*t[10];
    t[5] = (box.max.z - ray.position.z)*t[10];
    t[6] = (float)fmax(fmax(fmin(t[0], t[1]), fmin(t[2], t[3])), fmin(t[4], t[5]));
    t[7] = (float)fmin(fmin(fmax(t[0], t[1]), fmax(t[2], t[3])), fmax(t[4], t[5]));

    collision.hit = !((t[7] < 0) || (t[6] > t[7]));
    collision.distance = t[6];
    collision.point = Vector3Add(ray.position, Vector3Scale(ray.direction, collision.distance));

    // Get box center point
    collision.normal = Vector3Lerp(box.min, box.max, 0.5f);
    // Get vector center point->hit point
    collision.normal = Vector3Subtract(collision.point, collision.normal);
    // Scale vector to unit cube
    // NOTE: We use an additional .01 to fix numerical errors
    collision.normal = Vector3Scale(collision.normal, 2.01f);
    collision.normal = Vector3Divide(collision.normal, Vector3Subtract(box.max, box.min));
    // The relevant elemets of the vector are now slightly larger than 1.0f (or smaller than -1.0f)
    // and the others are somewhere between -1.0 and 1.0 casting to int is exactly our wanted normal!
    collision.normal.x = (float)((int)collision.normal.x);
    collision.normal.y = (float)((int)collision.normal.y);
    collision.normal.z = (float)((int)collision.normal.z);

    collision.normal = Vector3Normalize(collision.normal);

    if (insideBox)
    {
        // Reset ray.direction
        ray.direction = Vector3Negate(ray.direction);
        // Fix result
        collision.distance *= -1.0f;
        collision.normal = Vector3Negate(collision.normal);
    }

    return collision;
}

// Get collision info between ray and mesh
RayCollision GetRayCollisionMesh(Ray ray, Mesh mesh, Matrix transform)
{
    RayCollision collision = { 0 };

    // Check if mesh vertex data on CPU for testing
    if (mesh.vertices != NULL)
    {
        int triangleCount = mesh.triangleCount;

        // Test against all triangles in mesh
        for (int i = 0; i < triangleCount; i++)
        {
            Vector3 a, b, c;
            Vector3* vertdata = (Vector3*)mesh.vertices;

            if (mesh.indices)
            {
                a = vertdata[mesh.indices[i*3 + 0]];
                b = vertdata[mesh.indices[i*3 + 1]];
                c = vertdata[mesh.indices[i*3 + 2]];
            }
            else
            {
                a = vertdata[i*3 + 0];
                b = vertdata[i*3 + 1];
                c = vertdata[i*3 + 2];
            }

            a = Vector3Transform(a, transform);
            b = Vector3Transform(b, transform);
            c = Vector3Transform(c, transform);

            RayCollision triHitInfo = GetRayCollisionTriangle(ray, a, b, c);

            if (triHitInfo.hit)
            {
                // Save the closest hit triangle
                if ((!collision.hit) || (collision.distance > triHitInfo.distance)) collision = triHitInfo;
            }
        }
    }

    return collision;
}

// Get collision info between ray and model
RayCollision GetRayCollisionModel(Ray ray, Model model)
{
    RayCollision collision = { 0 };

    for (int m = 0; m < model.meshCount; m++)
    {
        RayCollision meshHitInfo = GetRayCollisionMesh(ray, model.meshes[m], model.transform);

        if (meshHitInfo.hit)
        {
            // Save the closest hit mesh
            if ((!collision.hit) || (collision.distance > meshHitInfo.distance)) collision = meshHitInfo;
        }
    }

    return collision;
}

// Get collision info between ray and triangle
// NOTE: The points are expected to be in counter-clockwise winding
// NOTE: Based on https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3)
{
    #define EPSILON 0.000001        // A small number

    RayCollision collision = { 0 };
    Vector3 edge1 = { 0 };
    Vector3 edge2 = { 0 };
    Vector3 p, q, tv;
    float det, invDet, u, v, t;

    // Find vectors for two edges sharing V1
    edge1 = Vector3Subtract(p2, p1);
    edge2 = Vector3Subtract(p3, p1);

    // Begin calculating determinant - also used to calculate u parameter
    p = Vector3CrossProduct(ray.direction, edge2);

    // If determinant is near zero, ray lies in plane of triangle or ray is parallel to plane of triangle
    det = Vector3DotProduct(edge1, p);

    // Avoid culling!
    if ((det > -EPSILON) && (det < EPSILON)) return collision;

    invDet = 1.0f/det;

    // Calculate distance from V1 to ray origin
    tv = Vector3Subtract(ray.position, p1);

    // Calculate u parameter and test bound
    u = Vector3DotProduct(tv, p)*invDet;

    // The intersection lies outside of the triangle
    if ((u < 0.0f) || (u > 1.0f)) return collision;

    // Prepare to test v parameter
    q = Vector3CrossProduct(tv, edge1);

    // Calculate V parameter and test bound
    v = Vector3DotProduct(ray.direction, q)*invDet;

    // The intersection lies outside of the triangle
    if ((v < 0.0f) || ((u + v) > 1.0f)) return collision;

    t = Vector3DotProduct(edge2, q)*invDet;

    if (t > EPSILON)
    {
        // Ray hit, get hit point and normal
        collision.hit = true;
        collision.distance = t;
        collision.normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));
        collision.point = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
    }

    return collision;
}

// Get collision info between ray and quad
// NOTE: The points are expected to be in counter-clockwise winding
RayCollision GetRayCollisionQuad(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4)
{
    RayCollision collision = { 0 };

    collision = GetRayCollisionTriangle(ray, p1, p2, p4);

    if (!collision.hit) collision = GetRayCollisionTriangle(ray, p2, p3, p4);

    return collision;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(SUPPORT_FILEFORMAT_OBJ)
// Load OBJ mesh data
//
// Keep the following information in mind when reading this
//  - A mesh is created for every material present in the obj file
//  - the model.meshCount is therefore the materialCount returned from tinyobj
//  - the mesh is automatically triangulated by tinyobj
static Model LoadOBJ(const char *fileName)
{
    Model model = { 0 };

    tinyobj_attrib_t attrib = { 0 };
    tinyobj_shape_t *meshes = NULL;
    unsigned int meshCount = 0;

    tinyobj_material_t *materials = NULL;
    unsigned int materialCount = 0;

    char *fileText = LoadFileText(fileName);

    if (fileText != NULL)
    {
        unsigned int dataSize = (unsigned int)strlen(fileText);
        char currentDir[1024] = { 0 };
        strcpy(currentDir, GetWorkingDirectory());
        const char *workingDir = GetDirectoryPath(fileName);
        if (CHDIR(workingDir) != 0)
        {
            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", workingDir);
        }

        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        int ret = tinyobj_parse_obj(&attrib, &meshes, &meshCount, &materials, &materialCount, fileText, dataSize, flags);

        if (ret != TINYOBJ_SUCCESS) TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load OBJ data", fileName);
        else TRACELOG(LOG_INFO, "MODEL: [%s] OBJ data loaded successfully: %i meshes/%i materials", fileName, meshCount, materialCount);

        model.meshCount = materialCount;

        // Init model materials array
        if (materialCount > 0)
        {
            model.materialCount = materialCount;
            model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
            TraceLog(LOG_INFO, "MODEL: model has %i material meshes", materialCount);
        }
        else
        {
            model.meshCount = 1;
            TraceLog(LOG_INFO, "MODEL: No materials, putting all meshes in a default material");
        }

        model.meshes = (Mesh *)RL_CALLOC(model.meshCount, sizeof(Mesh));
        model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));

        // Count the faces for each material
        int *matFaces = RL_CALLOC(materialCount, sizeof(int));

        for (int fi = 0; fi< attrib.num_faces; fi++)
        {
            //tinyobj_vertex_index_t face = attrib.faces[fi];
            int idx = attrib.material_ids[fi];
            matFaces[idx]++;
        }

        //--------------------------------------
        // Create the material meshes

        // Running counts/indexes for each material mesh as we are
        // building them at the same time
        int *vCount = RL_CALLOC(model.meshCount, sizeof(int));
        int *vtCount = RL_CALLOC(model.meshCount, sizeof(int));
        int *vnCount = RL_CALLOC(model.meshCount, sizeof(int));
        int *faceCount = RL_CALLOC(model.meshCount, sizeof(int));

        // Allocate space for each of the material meshes
        for (int mi = 0; mi < model.meshCount; mi++)
        {
            model.meshes[mi].vertexCount = matFaces[mi]*3;
            model.meshes[mi].triangleCount = matFaces[mi];
            model.meshes[mi].vertices = (float *)RL_CALLOC(model.meshes[mi].vertexCount*3, sizeof(float));
            model.meshes[mi].texcoords = (float *)RL_CALLOC(model.meshes[mi].vertexCount*2, sizeof(float));
            model.meshes[mi].normals = (float *)RL_CALLOC(model.meshes[mi].vertexCount*3, sizeof(float));
            model.meshMaterial[mi] = mi;
        }

        // Scan through the combined sub meshes and pick out each material mesh
        for (unsigned int af = 0; af < attrib.num_faces; af++)
        {
            int mm = attrib.material_ids[af];   // mesh material for this face
            if (mm == -1) { mm = 0; }           // no material object..

            // Get indices for the face
            tinyobj_vertex_index_t idx0 = attrib.faces[3*af + 0];
            tinyobj_vertex_index_t idx1 = attrib.faces[3*af + 1];
            tinyobj_vertex_index_t idx2 = attrib.faces[3*af + 2];

            // Fill vertices buffer (float) using vertex index of the face
            for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx0.v_idx*3 + v]; } vCount[mm] +=3;
            for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx1.v_idx*3 + v]; } vCount[mm] +=3;
            for (int v = 0; v < 3; v++) { model.meshes[mm].vertices[vCount[mm] + v] = attrib.vertices[idx2.v_idx*3 + v]; } vCount[mm] +=3;

            if (attrib.num_texcoords > 0)
            {
                // Fill texcoords buffer (float) using vertex index of the face
                // NOTE: Y-coordinate must be flipped upside-down to account for
                // raylib's upside down textures...
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx0.vt_idx*2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx0.vt_idx*2 + 1]; vtCount[mm] += 2;
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx1.vt_idx*2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx1.vt_idx*2 + 1]; vtCount[mm] += 2;
                model.meshes[mm].texcoords[vtCount[mm] + 0] = attrib.texcoords[idx2.vt_idx*2 + 0];
                model.meshes[mm].texcoords[vtCount[mm] + 1] = 1.0f - attrib.texcoords[idx2.vt_idx*2 + 1]; vtCount[mm] += 2;
            }

            if (attrib.num_normals > 0)
            {
                // Fill normals buffer (float) using vertex index of the face
                for (int v = 0; v < 3; v++) { model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx0.vn_idx*3 + v]; } vnCount[mm] +=3;
                for (int v = 0; v < 3; v++) { model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx1.vn_idx*3 + v]; } vnCount[mm] +=3;
                for (int v = 0; v < 3; v++) { model.meshes[mm].normals[vnCount[mm] + v] = attrib.normals[idx2.vn_idx*3 + v]; } vnCount[mm] +=3;
            }
        }

        // Init model materials
        for (unsigned int m = 0; m < materialCount; m++)
        {
            // Init material to default
            // NOTE: Uses default shader, which only supports MATERIAL_MAP_DIFFUSE
            model.materials[m] = LoadMaterialDefault();

            // Get default texture, in case no texture is defined
            // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
            model.materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = (Texture2D){ rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

            if (materials[m].diffuse_texname != NULL) model.materials[m].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(materials[m].diffuse_texname);  //char *diffuse_texname; // map_Kd

            model.materials[m].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ (unsigned char)(materials[m].diffuse[0]*255.0f), (unsigned char)(materials[m].diffuse[1]*255.0f), (unsigned char)(materials[m].diffuse[2]*255.0f), 255 }; //float diffuse[3];
            model.materials[m].maps[MATERIAL_MAP_DIFFUSE].value = 0.0f;

            if (materials[m].specular_texname != NULL) model.materials[m].maps[MATERIAL_MAP_SPECULAR].texture = LoadTexture(materials[m].specular_texname);  //char *specular_texname; // map_Ks
            model.materials[m].maps[MATERIAL_MAP_SPECULAR].color = (Color){ (unsigned char)(materials[m].specular[0]*255.0f), (unsigned char)(materials[m].specular[1]*255.0f), (unsigned char)(materials[m].specular[2]*255.0f), 255 }; //float specular[3];
            model.materials[m].maps[MATERIAL_MAP_SPECULAR].value = 0.0f;

            if (materials[m].bump_texname != NULL) model.materials[m].maps[MATERIAL_MAP_NORMAL].texture = LoadTexture(materials[m].bump_texname);  //char *bump_texname; // map_bump, bump
            model.materials[m].maps[MATERIAL_MAP_NORMAL].color = WHITE;
            model.materials[m].maps[MATERIAL_MAP_NORMAL].value = materials[m].shininess;

            model.materials[m].maps[MATERIAL_MAP_EMISSION].color = (Color){ (unsigned char)(materials[m].emission[0]*255.0f), (unsigned char)(materials[m].emission[1]*255.0f), (unsigned char)(materials[m].emission[2]*255.0f), 255 }; //float emission[3];

            if (materials[m].displacement_texname != NULL) model.materials[m].maps[MATERIAL_MAP_HEIGHT].texture = LoadTexture(materials[m].displacement_texname);  //char *displacement_texname; // disp
        }

        tinyobj_attrib_free(&attrib);
        tinyobj_shapes_free(meshes, meshCount);
        tinyobj_materials_free(materials, materialCount);

        UnloadFileText(fileText);

        RL_FREE(matFaces);
        RL_FREE(vCount);
        RL_FREE(vtCount);
        RL_FREE(vnCount);
        RL_FREE(faceCount);

        if (CHDIR(currentDir) != 0)
        {
            TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to change working directory", currentDir);
        }
    }

    return model;
}
#endif

#if defined(SUPPORT_FILEFORMAT_IQM)
// Load IQM mesh data
static Model LoadIQM(const char *fileName)
{
    #define IQM_MAGIC     "INTERQUAKEMODEL" // IQM file magic number
    #define IQM_VERSION          2          // only IQM version 2 supported

    #define BONE_NAME_LENGTH    32          // BoneInfo name string length
    #define MESH_NAME_LENGTH    32          // Mesh name string length
    #define MATERIAL_NAME_LENGTH 32         // Material name string length

    unsigned int fileSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &fileSize);
    unsigned char *fileDataPtr = fileData;

    // IQM file structs
    //-----------------------------------------------------------------------------------
    typedef struct IQMHeader {
        char magic[16];
        unsigned int version;
        unsigned int filesize;
        unsigned int flags;
        unsigned int num_text, ofs_text;
        unsigned int num_meshes, ofs_meshes;
        unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
        unsigned int num_triangles, ofs_triangles, ofs_adjacency;
        unsigned int num_joints, ofs_joints;
        unsigned int num_poses, ofs_poses;
        unsigned int num_anims, ofs_anims;
        unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
        unsigned int num_comment, ofs_comment;
        unsigned int num_extensions, ofs_extensions;
    } IQMHeader;

    typedef struct IQMMesh {
        unsigned int name;
        unsigned int material;
        unsigned int first_vertex, num_vertexes;
        unsigned int first_triangle, num_triangles;
    } IQMMesh;

    typedef struct IQMTriangle {
        unsigned int vertex[3];
    } IQMTriangle;

    typedef struct IQMJoint {
        unsigned int name;
        int parent;
        float translate[3], rotate[4], scale[3];
    } IQMJoint;

    typedef struct IQMVertexArray {
        unsigned int type;
        unsigned int flags;
        unsigned int format;
        unsigned int size;
        unsigned int offset;
    } IQMVertexArray;

    // NOTE: Below IQM structures are not used but listed for reference
    /*
    typedef struct IQMAdjacency {
        unsigned int triangle[3];
    } IQMAdjacency;

    typedef struct IQMPose {
        int parent;
        unsigned int mask;
        float channeloffset[10];
        float channelscale[10];
    } IQMPose;

    typedef struct IQMAnim {
        unsigned int name;
        unsigned int first_frame, num_frames;
        float framerate;
        unsigned int flags;
    } IQMAnim;

    typedef struct IQMBounds {
        float bbmin[3], bbmax[3];
        float xyradius, radius;
    } IQMBounds;
    */
    //-----------------------------------------------------------------------------------

    // IQM vertex data types
    enum {
        IQM_POSITION     = 0,
        IQM_TEXCOORD     = 1,
        IQM_NORMAL       = 2,
        IQM_TANGENT      = 3,       // NOTE: Tangents unused by default
        IQM_BLENDINDEXES = 4,
        IQM_BLENDWEIGHTS = 5,
        IQM_COLOR        = 6,
        IQM_CUSTOM       = 0x10     // NOTE: Custom vertex values unused by default
    };

    Model model = { 0 };

    IQMMesh *imesh = NULL;
    IQMTriangle *tri = NULL;
    IQMVertexArray *va = NULL;
    IQMJoint *ijoint = NULL;

    float *vertex = NULL;
    float *normal = NULL;
    float *text = NULL;
    char *blendi = NULL;
    unsigned char *blendw = NULL;
    unsigned char *color = NULL;

    // In case file can not be read, return an empty model
    if (fileDataPtr == NULL) return model;

    // Read IQM header
    IQMHeader *iqmHeader = (IQMHeader *)fileDataPtr;

    if (memcmp(iqmHeader->magic, IQM_MAGIC, sizeof(IQM_MAGIC)) != 0)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file is not a valid model", fileName);
        return model;
    }

    if (iqmHeader->version != IQM_VERSION)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file version not supported (%i)", fileName, iqmHeader->version);
        return model;
    }

    //fileDataPtr += sizeof(IQMHeader);       // Move file data pointer

    // Meshes data processing
    imesh = RL_MALLOC(iqmHeader->num_meshes*sizeof(IQMMesh));
    //fseek(iqmFile, iqmHeader->ofs_meshes, SEEK_SET);
    //fread(imesh, sizeof(IQMMesh)*iqmHeader->num_meshes, 1, iqmFile);
    memcpy(imesh, fileDataPtr + iqmHeader->ofs_meshes, iqmHeader->num_meshes*sizeof(IQMMesh));

    model.meshCount = iqmHeader->num_meshes;
    model.meshes = RL_CALLOC(model.meshCount, sizeof(Mesh));

    model.materialCount = model.meshCount;
    model.materials = (Material *)RL_CALLOC(model.materialCount, sizeof(Material));
    model.meshMaterial = (int *)RL_CALLOC(model.meshCount, sizeof(int));

    char name[MESH_NAME_LENGTH] = { 0 };
    char material[MATERIAL_NAME_LENGTH] = { 0 };

    for (int i = 0; i < model.meshCount; i++)
    {
        //fseek(iqmFile, iqmHeader->ofs_text + imesh[i].name, SEEK_SET);
        //fread(name, sizeof(char)*MESH_NAME_LENGTH, 1, iqmFile);
        memcpy(name, fileDataPtr + iqmHeader->ofs_text + imesh[i].name, MESH_NAME_LENGTH*sizeof(char));

        //fseek(iqmFile, iqmHeader->ofs_text + imesh[i].material, SEEK_SET);
        //fread(material, sizeof(char)*MATERIAL_NAME_LENGTH, 1, iqmFile);
        memcpy(material, fileDataPtr + iqmHeader->ofs_text + imesh[i].material, MATERIAL_NAME_LENGTH*sizeof(char));

        model.materials[i] = LoadMaterialDefault();

        TRACELOG(LOG_DEBUG, "MODEL: [%s] mesh name (%s), material (%s)", fileName, name, material);

        model.meshes[i].vertexCount = imesh[i].num_vertexes;

        model.meshes[i].vertices = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float));       // Default vertex positions
        model.meshes[i].normals = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float));        // Default vertex normals
        model.meshes[i].texcoords = RL_CALLOC(model.meshes[i].vertexCount*2, sizeof(float));      // Default vertex texcoords

        model.meshes[i].boneIds = RL_CALLOC(model.meshes[i].vertexCount*4, sizeof(float));        // Up-to 4 bones supported!
        model.meshes[i].boneWeights = RL_CALLOC(model.meshes[i].vertexCount*4, sizeof(float));    // Up-to 4 bones supported!

        model.meshes[i].triangleCount = imesh[i].num_triangles;
        model.meshes[i].indices = RL_CALLOC(model.meshes[i].triangleCount*3, sizeof(unsigned short));

        // Animated verted data, what we actually process for rendering
        // NOTE: Animated vertex should be re-uploaded to GPU (if not using GPU skinning)
        model.meshes[i].animVertices = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float));
        model.meshes[i].animNormals = RL_CALLOC(model.meshes[i].vertexCount*3, sizeof(float));
    }

    // Triangles data processing
    tri = RL_MALLOC(iqmHeader->num_triangles*sizeof(IQMTriangle));
    //fseek(iqmFile, iqmHeader->ofs_triangles, SEEK_SET);
    //fread(tri, iqmHeader->num_triangles*sizeof(IQMTriangle), 1, iqmFile);
    memcpy(tri, fileDataPtr + iqmHeader->ofs_triangles, iqmHeader->num_triangles*sizeof(IQMTriangle));

    for (int m = 0; m < model.meshCount; m++)
    {
        int tcounter = 0;

        for (unsigned int i = imesh[m].first_triangle; i < (imesh[m].first_triangle + imesh[m].num_triangles); i++)
        {
            // IQM triangles indexes are stored in counter-clockwise, but raylib processes the index in linear order,
            // expecting they point to the counter-clockwise vertex triangle, so we need to reverse triangle indexes
            // NOTE: raylib renders vertex data in counter-clockwise order (standard convention) by default
            model.meshes[m].indices[tcounter + 2] = tri[i].vertex[0] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter + 1] = tri[i].vertex[1] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter] = tri[i].vertex[2] - imesh[m].first_vertex;
            tcounter += 3;
        }
    }

    // Vertex arrays data processing
    va = RL_MALLOC(iqmHeader->num_vertexarrays*sizeof(IQMVertexArray));
    //fseek(iqmFile, iqmHeader->ofs_vertexarrays, SEEK_SET);
    //fread(va, iqmHeader->num_vertexarrays*sizeof(IQMVertexArray), 1, iqmFile);
    memcpy(va, fileDataPtr + iqmHeader->ofs_vertexarrays, iqmHeader->num_vertexarrays*sizeof(IQMVertexArray));

    for (unsigned int i = 0; i < iqmHeader->num_vertexarrays; i++)
    {
        switch (va[i].type)
        {
            case IQM_POSITION:
            {
                vertex = RL_MALLOC(iqmHeader->num_vertexes*3*sizeof(float));
                //fseek(iqmFile, va[i].offset, SEEK_SET);
                //fread(vertex, iqmHeader->num_vertexes*3*sizeof(float), 1, iqmFile);
                memcpy(vertex, fileDataPtr + va[i].offset, iqmHeader->num_vertexes*3*sizeof(float));

                for (unsigned int m = 0; m < iqmHeader->num_meshes; m++)
                {
                    int vCounter = 0;
                    for (unsigned int i = imesh[m].first_vertex*3; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*3; i++)
                    {
                        model.meshes[m].vertices[vCounter] = vertex[i];
                        model.meshes[m].animVertices[vCounter] = vertex[i];
                        vCounter++;
                    }
                }
            } break;
            case IQM_NORMAL:
            {
                normal = RL_MALLOC(iqmHeader->num_vertexes*3*sizeof(float));
                //fseek(iqmFile, va[i].offset, SEEK_SET);
                //fread(normal, iqmHeader->num_vertexes*3*sizeof(float), 1, iqmFile);
                memcpy(normal, fileDataPtr + va[i].offset, iqmHeader->num_vertexes*3*sizeof(float));

                for (unsigned int m = 0; m < iqmHeader->num_meshes; m++)
                {
                    int vCounter = 0;
                    for (unsigned int i = imesh[m].first_vertex*3; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*3; i++)
                    {
                        model.meshes[m].normals[vCounter] = normal[i];
                        model.meshes[m].animNormals[vCounter] = normal[i];
                        vCounter++;
                    }
                }
            } break;
            case IQM_TEXCOORD:
            {
                text = RL_MALLOC(iqmHeader->num_vertexes*2*sizeof(float));
                //fseek(iqmFile, va[i].offset, SEEK_SET);
                //fread(text, iqmHeader->num_vertexes*2*sizeof(float), 1, iqmFile);
                memcpy(text, fileDataPtr + va[i].offset, iqmHeader->num_vertexes*2*sizeof(float));

                for (unsigned int m = 0; m < iqmHeader->num_meshes; m++)
                {
                    int vCounter = 0;
                    for (unsigned int i = imesh[m].first_vertex*2; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*2; i++)
                    {
                        model.meshes[m].texcoords[vCounter] = text[i];
                        vCounter++;
                    }
                }
            } break;
            case IQM_BLENDINDEXES:
            {
                blendi = RL_MALLOC(iqmHeader->num_vertexes*4*sizeof(char));
                //fseek(iqmFile, va[i].offset, SEEK_SET);
                //fread(blendi, iqmHeader->num_vertexes*4*sizeof(char), 1, iqmFile);
                memcpy(blendi, fileDataPtr + va[i].offset, iqmHeader->num_vertexes*4*sizeof(char));

                for (unsigned int m = 0; m < iqmHeader->num_meshes; m++)
                {
                    int boneCounter = 0;
                    for (unsigned int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
                    {
                        model.meshes[m].boneIds[boneCounter] = blendi[i];
                        boneCounter++;
                    }
                }
            } break;
            case IQM_BLENDWEIGHTS:
            {
                blendw = RL_MALLOC(iqmHeader->num_vertexes*4*sizeof(unsigned char));
                //fseek(iqmFile, va[i].offset, SEEK_SET);
                //fread(blendw, iqmHeader->num_vertexes*4*sizeof(unsigned char), 1, iqmFile);
                memcpy(blendw, fileDataPtr + va[i].offset, iqmHeader->num_vertexes*4*sizeof(unsigned char));

                for (unsigned int m = 0; m < iqmHeader->num_meshes; m++)
                {
                    int boneCounter = 0;
                    for (unsigned int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
                    {
                        model.meshes[m].boneWeights[boneCounter] = blendw[i]/255.0f;
                        boneCounter++;
                    }
                }
            } break;
            case IQM_COLOR:
            {
                color = RL_MALLOC(iqmHeader->num_vertexes*4*sizeof(unsigned char));
                //fseek(iqmFile, va[i].offset, SEEK_SET);
                //fread(blendw, iqmHeader->num_vertexes*4*sizeof(unsigned char), 1, iqmFile);
                memcpy(color, fileDataPtr + va[i].offset, iqmHeader->num_vertexes*4*sizeof(unsigned char));

                for (unsigned int m = 0; m < iqmHeader->num_meshes; m++)
                {
                    model.meshes[m].colors = RL_CALLOC(model.meshes[m].vertexCount*4, sizeof(unsigned char));

                    int vCounter = 0;
                    for (unsigned int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
                    {
                        model.meshes[m].colors[vCounter] = color[i];
                        vCounter++;
                    }
                }
            } break;
        }
    }

    // Bones (joints) data processing
    ijoint = RL_MALLOC(iqmHeader->num_joints*sizeof(IQMJoint));
    //fseek(iqmFile, iqmHeader->ofs_joints, SEEK_SET);
    //fread(ijoint, iqmHeader->num_joints*sizeof(IQMJoint), 1, iqmFile);
    memcpy(ijoint, fileDataPtr + iqmHeader->ofs_joints, iqmHeader->num_joints*sizeof(IQMJoint));

    model.boneCount = iqmHeader->num_joints;
    model.bones = RL_MALLOC(iqmHeader->num_joints*sizeof(BoneInfo));
    model.bindPose = RL_MALLOC(iqmHeader->num_joints*sizeof(Transform));

    for (unsigned int i = 0; i < iqmHeader->num_joints; i++)
    {
        // Bones
        model.bones[i].parent = ijoint[i].parent;
        //fseek(iqmFile, iqmHeader->ofs_text + ijoint[i].name, SEEK_SET);
        //fread(model.bones[i].name, BONE_NAME_LENGTH*sizeof(char), 1, iqmFile);
        memcpy(model.bones[i].name, fileDataPtr + iqmHeader->ofs_text + ijoint[i].name, BONE_NAME_LENGTH*sizeof(char));

        // Bind pose (base pose)
        model.bindPose[i].translation.x = ijoint[i].translate[0];
        model.bindPose[i].translation.y = ijoint[i].translate[1];
        model.bindPose[i].translation.z = ijoint[i].translate[2];

        model.bindPose[i].rotation.x = ijoint[i].rotate[0];
        model.bindPose[i].rotation.y = ijoint[i].rotate[1];
        model.bindPose[i].rotation.z = ijoint[i].rotate[2];
        model.bindPose[i].rotation.w = ijoint[i].rotate[3];

        model.bindPose[i].scale.x = ijoint[i].scale[0];
        model.bindPose[i].scale.y = ijoint[i].scale[1];
        model.bindPose[i].scale.z = ijoint[i].scale[2];
    }

    // Build bind pose from parent joints
    for (int i = 0; i < model.boneCount; i++)
    {
        if (model.bones[i].parent >= 0)
        {
            model.bindPose[i].rotation = QuaternionMultiply(model.bindPose[model.bones[i].parent].rotation, model.bindPose[i].rotation);
            model.bindPose[i].translation = Vector3RotateByQuaternion(model.bindPose[i].translation, model.bindPose[model.bones[i].parent].rotation);
            model.bindPose[i].translation = Vector3Add(model.bindPose[i].translation, model.bindPose[model.bones[i].parent].translation);
            model.bindPose[i].scale = Vector3Multiply(model.bindPose[i].scale, model.bindPose[model.bones[i].parent].scale);
        }
    }

    RL_FREE(fileData);

    RL_FREE(imesh);
    RL_FREE(tri);
    RL_FREE(va);
    RL_FREE(vertex);
    RL_FREE(normal);
    RL_FREE(text);
    RL_FREE(blendi);
    RL_FREE(blendw);
    RL_FREE(ijoint);

    return model;
}

// Load IQM animation data
static ModelAnimation* LoadIQMModelAnimations(const char *fileName, int *animCount)
{
#define IQM_MAGIC       "INTERQUAKEMODEL"   // IQM file magic number
#define IQM_VERSION     2                   // only IQM version 2 supported

    unsigned int fileSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &fileSize);
    unsigned char *fileDataPtr = fileData;

    typedef struct IQMHeader {
        char magic[16];
        unsigned int version;
        unsigned int filesize;
        unsigned int flags;
        unsigned int num_text, ofs_text;
        unsigned int num_meshes, ofs_meshes;
        unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
        unsigned int num_triangles, ofs_triangles, ofs_adjacency;
        unsigned int num_joints, ofs_joints;
        unsigned int num_poses, ofs_poses;
        unsigned int num_anims, ofs_anims;
        unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
        unsigned int num_comment, ofs_comment;
        unsigned int num_extensions, ofs_extensions;
    } IQMHeader;

    typedef struct IQMPose {
        int parent;
        unsigned int mask;
        float channeloffset[10];
        float channelscale[10];
    } IQMPose;

    typedef struct IQMAnim {
        unsigned int name;
        unsigned int first_frame, num_frames;
        float framerate;
        unsigned int flags;
    } IQMAnim;

    // In case file can not be read, return an empty model
    if (fileDataPtr == NULL) return NULL;

    // Read IQM header
    IQMHeader *iqmHeader = (IQMHeader *)fileDataPtr;

    if (memcmp(iqmHeader->magic, IQM_MAGIC, sizeof(IQM_MAGIC)) != 0)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file is not a valid model", fileName);
        return NULL;
    }

    if (iqmHeader->version != IQM_VERSION)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] IQM file version not supported (%i)", fileName, iqmHeader->version);
        return NULL;
    }

    // Get bones data
    IQMPose *poses = RL_MALLOC(iqmHeader->num_poses*sizeof(IQMPose));
    //fseek(iqmFile, iqmHeader->ofs_poses, SEEK_SET);
    //fread(poses, iqmHeader->num_poses*sizeof(IQMPose), 1, iqmFile);
    memcpy(poses, fileDataPtr + iqmHeader->ofs_poses, iqmHeader->num_poses*sizeof(IQMPose));

    // Get animations data
    *animCount = iqmHeader->num_anims;
    IQMAnim *anim = RL_MALLOC(iqmHeader->num_anims*sizeof(IQMAnim));
    //fseek(iqmFile, iqmHeader->ofs_anims, SEEK_SET);
    //fread(anim, iqmHeader->num_anims*sizeof(IQMAnim), 1, iqmFile);
    memcpy(anim, fileDataPtr + iqmHeader->ofs_anims, iqmHeader->num_anims*sizeof(IQMAnim));

    ModelAnimation *animations = RL_MALLOC(iqmHeader->num_anims*sizeof(ModelAnimation));

    // frameposes
    unsigned short *framedata = RL_MALLOC(iqmHeader->num_frames*iqmHeader->num_framechannels*sizeof(unsigned short));
    //fseek(iqmFile, iqmHeader->ofs_frames, SEEK_SET);
    //fread(framedata, iqmHeader->num_frames*iqmHeader->num_framechannels*sizeof(unsigned short), 1, iqmFile);
    memcpy(framedata, fileDataPtr + iqmHeader->ofs_frames, iqmHeader->num_frames*iqmHeader->num_framechannels*sizeof(unsigned short));

    for (unsigned int a = 0; a < iqmHeader->num_anims; a++)
    {
        animations[a].frameCount = anim[a].num_frames;
        animations[a].boneCount = iqmHeader->num_poses;
        animations[a].bones = RL_MALLOC(iqmHeader->num_poses*sizeof(BoneInfo));
        animations[a].framePoses = RL_MALLOC(anim[a].num_frames*sizeof(Transform *));
        // animations[a].framerate = anim.framerate;     // TODO: Use framerate?

        for (unsigned int j = 0; j < iqmHeader->num_poses; j++)
        {
            strcpy(animations[a].bones[j].name, "ANIMJOINTNAME");
            animations[a].bones[j].parent = poses[j].parent;
        }

        for (unsigned int j = 0; j < anim[a].num_frames; j++) animations[a].framePoses[j] = RL_MALLOC(iqmHeader->num_poses*sizeof(Transform));

        int dcounter = anim[a].first_frame*iqmHeader->num_framechannels;

        for (unsigned int frame = 0; frame < anim[a].num_frames; frame++)
        {
            for (unsigned int i = 0; i < iqmHeader->num_poses; i++)
            {
                animations[a].framePoses[frame][i].translation.x = poses[i].channeloffset[0];

                if (poses[i].mask & 0x01)
                {
                    animations[a].framePoses[frame][i].translation.x += framedata[dcounter]*poses[i].channelscale[0];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].translation.y = poses[i].channeloffset[1];

                if (poses[i].mask & 0x02)
                {
                    animations[a].framePoses[frame][i].translation.y += framedata[dcounter]*poses[i].channelscale[1];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].translation.z = poses[i].channeloffset[2];

                if (poses[i].mask & 0x04)
                {
                    animations[a].framePoses[frame][i].translation.z += framedata[dcounter]*poses[i].channelscale[2];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].rotation.x = poses[i].channeloffset[3];

                if (poses[i].mask & 0x08)
                {
                    animations[a].framePoses[frame][i].rotation.x += framedata[dcounter]*poses[i].channelscale[3];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].rotation.y = poses[i].channeloffset[4];

                if (poses[i].mask & 0x10)
                {
                    animations[a].framePoses[frame][i].rotation.y += framedata[dcounter]*poses[i].channelscale[4];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].rotation.z = poses[i].channeloffset[5];

                if (poses[i].mask & 0x20)
                {
                    animations[a].framePoses[frame][i].rotation.z += framedata[dcounter]*poses[i].channelscale[5];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].rotation.w = poses[i].channeloffset[6];

                if (poses[i].mask & 0x40)
                {
                    animations[a].framePoses[frame][i].rotation.w += framedata[dcounter]*poses[i].channelscale[6];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].scale.x = poses[i].channeloffset[7];

                if (poses[i].mask & 0x80)
                {
                    animations[a].framePoses[frame][i].scale.x += framedata[dcounter]*poses[i].channelscale[7];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].scale.y = poses[i].channeloffset[8];

                if (poses[i].mask & 0x100)
                {
                    animations[a].framePoses[frame][i].scale.y += framedata[dcounter]*poses[i].channelscale[8];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].scale.z = poses[i].channeloffset[9];

                if (poses[i].mask & 0x200)
                {
                    animations[a].framePoses[frame][i].scale.z += framedata[dcounter]*poses[i].channelscale[9];
                    dcounter++;
                }

                animations[a].framePoses[frame][i].rotation = QuaternionNormalize(animations[a].framePoses[frame][i].rotation);
            }
        }

        // Build frameposes
        for (unsigned int frame = 0; frame < anim[a].num_frames; frame++)
        {
            for (int i = 0; i < animations[a].boneCount; i++)
            {
                if (animations[a].bones[i].parent >= 0)
                {
                    animations[a].framePoses[frame][i].rotation = QuaternionMultiply(animations[a].framePoses[frame][animations[a].bones[i].parent].rotation, animations[a].framePoses[frame][i].rotation);
                    animations[a].framePoses[frame][i].translation = Vector3RotateByQuaternion(animations[a].framePoses[frame][i].translation, animations[a].framePoses[frame][animations[a].bones[i].parent].rotation);
                    animations[a].framePoses[frame][i].translation = Vector3Add(animations[a].framePoses[frame][i].translation, animations[a].framePoses[frame][animations[a].bones[i].parent].translation);
                    animations[a].framePoses[frame][i].scale = Vector3Multiply(animations[a].framePoses[frame][i].scale, animations[a].framePoses[frame][animations[a].bones[i].parent].scale);
                }
            }
        }
    }

    RL_FREE(fileData);

    RL_FREE(framedata);
    RL_FREE(poses);
    RL_FREE(anim);

    return animations;
}

#endif

#if defined(SUPPORT_FILEFORMAT_GLTF)

static const unsigned char base64Table[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 62, 0, 0, 0, 63, 52, 53,
    54, 55, 56, 57, 58, 59, 60, 61, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 2, 3, 4,
    5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0, 0, 0, 0, 0, 0, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51
};

static int GetSizeBase64(char *input)
{
    int size = 0;

    for (int i = 0; input[4*i] != 0; i++)
    {
        if (input[4*i + 3] == '=')
        {
            if (input[4*i + 2] == '=') size += 1;
            else size += 2;
        }
        else size += 3;
    }

    return size;
}

static unsigned char *DecodeBase64(char *input, int *size)
{
    *size = GetSizeBase64(input);

    unsigned char *buf = (unsigned char *)RL_MALLOC(*size);
    for (int i = 0; i < *size/3; i++)
    {
        unsigned char a = base64Table[(int)input[4*i]];
        unsigned char b = base64Table[(int)input[4*i + 1]];
        unsigned char c = base64Table[(int)input[4*i + 2]];
        unsigned char d = base64Table[(int)input[4*i + 3]];

        buf[3*i] = (a << 2) | (b >> 4);
        buf[3*i + 1] = (b << 4) | (c >> 2);
        buf[3*i + 2] = (c << 6) | d;
    }

    if (*size%3 == 1)
    {
        int n = *size/3;
        unsigned char a = base64Table[(int)input[4*n]];
        unsigned char b = base64Table[(int)input[4*n + 1]];
        buf[*size - 1] = (a << 2) | (b >> 4);
    }
    else if (*size%3 == 2)
    {
        int n = *size/3;
        unsigned char a = base64Table[(int)input[4*n]];
        unsigned char b = base64Table[(int)input[4*n + 1]];
        unsigned char c = base64Table[(int)input[4*n + 2]];
        buf[*size - 2] = (a << 2) | (b >> 4);
        buf[*size - 1] = (b << 4) | (c >> 2);
    }
    return buf;
}

// Load texture from cgltf_image
static Image LoadImageFromCgltfImage(cgltf_image *image, const char *texPath, Color tint)
{
    Image rimage = { 0 };

    if (image->uri)
    {
        if ((strlen(image->uri) > 5) &&
            (image->uri[0] == 'd') &&
            (image->uri[1] == 'a') &&
            (image->uri[2] == 't') &&
            (image->uri[3] == 'a') &&
            (image->uri[4] == ':'))
        {
            // Data URI
            // Format: data:<mediatype>;base64,<data>

            // Find the comma
            int i = 0;
            while ((image->uri[i] != ',') && (image->uri[i] != 0)) i++;

            if (image->uri[i] == 0) TRACELOG(LOG_WARNING, "IMAGE: glTF data URI is not a valid image");
            else
            {
                int size = 0;
                unsigned char *data = DecodeBase64(image->uri + i + 1, &size);

                int width, height;
                unsigned char *raw = stbi_load_from_memory(data, size, &width, &height, NULL, 4);
                RL_FREE(data);

                rimage.data = raw;
                rimage.width = width;
                rimage.height = height;
                rimage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
                rimage.mipmaps = 1;

                // TODO: Tint shouldn't be applied here!
                ImageColorTint(&rimage, tint);
            }
        }
        else
        {
            rimage = LoadImage(TextFormat("%s/%s", texPath, image->uri));

            // TODO: Tint shouldn't be applied here!
            ImageColorTint(&rimage, tint);
        }
    }
    else if (image->buffer_view)
    {
        unsigned char *data = RL_MALLOC(image->buffer_view->size);
        int n = (int)image->buffer_view->offset;
        int stride = (int)image->buffer_view->stride ? (int)image->buffer_view->stride : 1;

        for (unsigned int i = 0; i < image->buffer_view->size; i++)
        {
            data[i] = ((unsigned char *)image->buffer_view->buffer->data)[n];
            n += stride;
        }

        int width, height;
        unsigned char *raw = stbi_load_from_memory(data, (int)image->buffer_view->size, &width, &height, NULL, 4);
        RL_FREE(data);

        rimage.data = raw;
        rimage.width = width;
        rimage.height = height;
        rimage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        rimage.mipmaps = 1;

        // TODO: Tint shouldn't be applied here!
        ImageColorTint(&rimage, tint);
    }
    else rimage = GenImageColor(1, 1, tint);

    return rimage;
}

//
static bool ReadGLTFValue(cgltf_accessor *acc, unsigned int index, void *variable)
{
    unsigned int typeElements = 0;

    switch (acc->type)
    {
        case cgltf_type_scalar: typeElements = 1; break;
        case cgltf_type_vec2: typeElements = 2; break;
        case cgltf_type_vec3: typeElements = 3; break;
        case cgltf_type_vec4:
        case cgltf_type_mat2: typeElements = 4; break;
        case cgltf_type_mat3: typeElements = 9; break;
        case cgltf_type_mat4: typeElements = 16; break;
        case cgltf_type_invalid: typeElements = 0; break;
        default: break;
    }

    unsigned int typeSize = 0;

    switch (acc->component_type)
    {
        case cgltf_component_type_r_8u:
        case cgltf_component_type_r_8: typeSize = 1; break;
        case cgltf_component_type_r_16u:
        case cgltf_component_type_r_16: typeSize = 2; break;
        case cgltf_component_type_r_32f:
        case cgltf_component_type_r_32u: typeSize = 4; break;
        case cgltf_component_type_invalid: typeSize = 0; break;
        default: break;
    }

    unsigned int singleElementSize = typeSize*typeElements;

    if (acc->count == 2)
    {
        if (index > 1) return false;

        memcpy(variable, index == 0 ? acc->min : acc->max, singleElementSize);
        return true;
    }

    memset(variable, 0, singleElementSize);

    if (acc->buffer_view == NULL || acc->buffer_view->buffer == NULL || acc->buffer_view->buffer->data == NULL) return false;

    if (!acc->buffer_view->stride)
    {
        void *readPosition = ((char *)acc->buffer_view->buffer->data) + (index*singleElementSize) + acc->buffer_view->offset + acc->offset;
        memcpy(variable, readPosition, singleElementSize);
    }
    else
    {
        void *readPosition = ((char *)acc->buffer_view->buffer->data) + (index*acc->buffer_view->stride) + acc->buffer_view->offset + acc->offset;
        memcpy(variable, readPosition, singleElementSize);
    }

    return true;
}

static void *ReadGLTFValuesAs(cgltf_accessor* acc, cgltf_component_type type, bool adjustOnDownCasting)
{
    unsigned int count = acc->count;
    unsigned int typeSize = 0;
    switch (type)
    {
        case cgltf_component_type_r_8u:
        case cgltf_component_type_r_8: typeSize = 1; break;
        case cgltf_component_type_r_16u:
        case cgltf_component_type_r_16: typeSize = 2; break;
        case cgltf_component_type_r_32f:
        case cgltf_component_type_r_32u: typeSize = 4; break;
        case cgltf_component_type_invalid: typeSize = 0; break;
        default: break;
    }

    unsigned int typeElements = 0;
    switch (acc->type)
    {
        case cgltf_type_scalar: typeElements = 1; break;
        case cgltf_type_vec2: typeElements = 2; break;
        case cgltf_type_vec3: typeElements = 3; break;
        case cgltf_type_vec4:
        case cgltf_type_mat2: typeElements = 4; break;
        case cgltf_type_mat3: typeElements = 9; break;
        case cgltf_type_mat4: typeElements = 16; break;
        case cgltf_type_invalid: typeElements = 0; break;
        default: break;
    }

    if (acc->component_type == type)
    {
        void *array = RL_MALLOC(count*typeElements*typeSize);

        for (unsigned int i = 0; i < count; i++) ReadGLTFValue(acc, i, (char *)array + i*typeElements*typeSize);

        return array;

    }
    else
    {
        unsigned int accTypeSize = 0;
        switch (acc->component_type)
        {
            case cgltf_component_type_r_8u:
            case cgltf_component_type_r_8: accTypeSize = 1; break;
            case cgltf_component_type_r_16u:
            case cgltf_component_type_r_16: accTypeSize = 2; break;
            case cgltf_component_type_r_32f:
            case cgltf_component_type_r_32u: accTypeSize = 4; break;
            case cgltf_component_type_invalid: accTypeSize = 0; break;
            default: break;
        }

        void *array = RL_MALLOC(count*typeElements*typeSize);
        void *additionalArray = RL_MALLOC(count*typeElements*accTypeSize);

        for (unsigned int i = 0; i < count; i++)
        {
            ReadGLTFValue(acc, i, (char *)additionalArray + i*typeElements*accTypeSize);
        }

        switch (acc->component_type)
        {
            case cgltf_component_type_r_8u:
            {
                unsigned char *typedAdditionalArray = (unsigned char *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (char)(typedAdditionalArray[i]/(UCHAR_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }

                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        unsigned short *typedArray = (unsigned short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        unsigned int *typedArray = (unsigned int *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned int)typedAdditionalArray[i];
                    } break;
                    default:
                    {
                        RL_FREE(array);
                        RL_FREE(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_8:
            {
                char *typedAdditionalArray = (char *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        unsigned char *typedArray = (unsigned char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned char)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        unsigned short *typedArray = (unsigned short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        unsigned int *typedArray = (unsigned int *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned int)typedAdditionalArray[i];
                    } break;
                    default:
                    {
                        RL_FREE(array);
                        RL_FREE(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_16u:
            {
                unsigned short *typedAdditionalArray = (unsigned short *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        unsigned char *typedArray = (unsigned char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (unsigned char)(typedAdditionalArray[i]/(USHRT_MAX/UCHAR_MAX));
                            else typedArray[i] = (unsigned char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *) array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (char)(typedAdditionalArray[i]/(USHRT_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (short)(typedAdditionalArray[i]/(USHRT_MAX/SHRT_MAX));
                            else typedArray[i] = (short)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        unsigned int *typedArray = (unsigned int *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned int)typedAdditionalArray[i];
                    } break;
                    default:
                    {
                        RL_FREE(array);
                        RL_FREE(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_16:
            {
                short *typedAdditionalArray = (short *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        unsigned char *typedArray = (unsigned char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (unsigned char)(typedAdditionalArray[i]/(SHRT_MAX/UCHAR_MAX));
                            else typedArray[i] = (unsigned char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (char)(typedAdditionalArray[i]/(SHRT_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        unsigned short *typedArray = (unsigned short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        unsigned int *typedArray = (unsigned int *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned int)typedAdditionalArray[i];
                    } break;
                    default:
                    {
                        RL_FREE(array);
                        RL_FREE(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_32f:
            {
                float *typedAdditionalArray = (float *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        unsigned char *typedArray = (unsigned char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned char)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (char)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        unsigned short *typedArray = (unsigned short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (short)typedAdditionalArray[i];
                    } break;
                    case cgltf_component_type_r_32u:
                    {
                        unsigned int *typedArray = (unsigned int *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (unsigned int)typedAdditionalArray[i];
                    } break;
                    default:
                    {
                        RL_FREE(array);
                        RL_FREE(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            case cgltf_component_type_r_32u:
            {
                unsigned int *typedAdditionalArray = (unsigned int *)additionalArray;
                switch (type)
                {
                    case cgltf_component_type_r_8u:
                    {
                        unsigned char *typedArray = (unsigned char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (unsigned char)(typedAdditionalArray[i]/(UINT_MAX/UCHAR_MAX));
                            else typedArray[i] = (unsigned char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_8:
                    {
                        char *typedArray = (char *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (char)(typedAdditionalArray[i]/(UINT_MAX/CHAR_MAX));
                            else typedArray[i] = (char)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16u:
                    {
                        unsigned short *typedArray = (unsigned short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (unsigned short)(typedAdditionalArray[i]/(UINT_MAX/USHRT_MAX));
                            else typedArray[i] = (unsigned short)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_16:
                    {
                        short *typedArray = (short *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++)
                        {
                            if (adjustOnDownCasting) typedArray[i] = (short)(typedAdditionalArray[i]/(UINT_MAX/SHRT_MAX));
                            else typedArray[i] = (short)typedAdditionalArray[i];
                        }
                    } break;
                    case cgltf_component_type_r_32f:
                    {
                        float *typedArray = (float *)array;
                        for (unsigned int i = 0; i < count*typeElements; i++) typedArray[i] = (float)typedAdditionalArray[i];
                    } break;
                    default:
                    {
                        RL_FREE(array);
                        RL_FREE(additionalArray);
                        return NULL;
                    } break;
                }
            } break;
            default:
            {
                RL_FREE(array);
                RL_FREE(additionalArray);
                return NULL;
            } break;
        }

        RL_FREE(additionalArray);
        return array;
    }
}

// LoadGLTF loads in model data from given filename, supporting both .gltf and .glb
static Model LoadGLTF(const char *fileName)
{
    /***********************************************************************************

        Function implemented by Wilhem Barbier(@wbrbr), with modifications by Tyler Bezera(@gamerfiend) and Hristo Stamenov(@object71)

        Features:
          - Supports .gltf and .glb files
          - Supports embedded (base64) or external textures
          - Loads all raylib supported material textures, values and colors
          - Supports multiple mesh per model and multiple primitives per model

        Some restrictions (not exhaustive):
          - Triangle-only meshes
          - Not supported node hierarchies or transforms
          - Only supports unsigned short indices (no byte/unsigned int)
          - Only supports float for texture coordinates (no byte/unsigned short)

    *************************************************************************************/

    Model model = { 0 };

    // glTF file loading
    unsigned int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);

    if (fileData == NULL) return model;

    // glTF data loading
    cgltf_options options = { 0 };
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

    if (result == cgltf_result_success)
    {
        TRACELOG(LOG_INFO, "MODEL: [%s] glTF meshes (%s) count: %i", fileName, (data->file_type == 2)? "glb" : "gltf", data->meshes_count);
        TRACELOG(LOG_INFO, "MODEL: [%s] glTF materials (%s) count: %i", fileName, (data->file_type == 2)? "glb" : "gltf", data->materials_count);

        // Read data buffers
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load mesh/material buffers", fileName);

        if (data->scenes_count > 1) TRACELOG(LOG_INFO, "MODEL: [%s] Has multiple scenes but only the first one will be loaded", fileName);

        int primitiveCount = 0;
        for (unsigned int i = 0; i < data->scene->nodes_count; i++)
        {
            GetGLTFPrimitiveCount(data->scene->nodes[i], &primitiveCount);
        }

        // Process glTF data and map to model
        model.meshCount = primitiveCount;
        model.meshes = RL_CALLOC(model.meshCount, sizeof(Mesh));
        model.materialCount = (int)data->materials_count + 1;
        model.materials = RL_MALLOC(model.materialCount*sizeof(Material));
        model.meshMaterial = RL_MALLOC(model.meshCount*sizeof(int));
        model.boneCount = (int)data->nodes_count;
        model.bones = RL_CALLOC(model.boneCount, sizeof(BoneInfo));
        model.bindPose = RL_CALLOC(model.boneCount, sizeof(Transform));

        InitGLTFBones(&model, data);
        LoadGLTFMaterial(&model, fileName, data);

        int primitiveIndex = 0;
        for (unsigned int i = 0; i < data->scene->nodes_count; i++)
        {
            Matrix staticTransform = MatrixIdentity();
            LoadGLTFNode(data, data->scene->nodes[i], &model, staticTransform, &primitiveIndex, fileName);
        }

        cgltf_free(data);
    }
    else TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);

    RL_FREE(fileData);

    return model;
}

static void InitGLTFBones(Model *model, const cgltf_data *data)
{
    for (unsigned int j = 0; j < data->nodes_count; j++)
    {
        strcpy(model->bones[j].name, data->nodes[j].name == 0 ? "ANIMJOINT" : data->nodes[j].name);
        model->bones[j].parent = (data->nodes[j].parent != NULL) ? (int)(data->nodes[j].parent - data->nodes) : -1;
    }

    for (unsigned int i = 0; i < data->nodes_count; i++)
    {
        if (data->nodes[i].has_translation) memcpy(&model->bindPose[i].translation, data->nodes[i].translation, 3*sizeof(float));
        else model->bindPose[i].translation = Vector3Zero();

        if (data->nodes[i].has_rotation) memcpy(&model->bindPose[i].rotation, data->nodes[i].rotation, 4*sizeof(float));
        else model->bindPose[i].rotation = QuaternionIdentity();

        model->bindPose[i].rotation = QuaternionNormalize(model->bindPose[i].rotation);

        if (data->nodes[i].has_scale) memcpy(&model->bindPose[i].scale, data->nodes[i].scale, 3*sizeof(float));
        else model->bindPose[i].scale = Vector3One();
    }

    {
        bool *completedBones = RL_CALLOC(model->boneCount, sizeof(bool));
        int numberCompletedBones = 0;

        while (numberCompletedBones < model->boneCount)
        {
            for (int i = 0; i < model->boneCount; i++)
            {
                if (completedBones[i]) continue;

                if (model->bones[i].parent < 0)
                {
                    completedBones[i] = true;
                    numberCompletedBones++;
                    continue;
                }

                if (!completedBones[model->bones[i].parent]) continue;

                Transform* currentTransform = &model->bindPose[i];
                BoneInfo* currentBone = &model->bones[i];
                int root = currentBone->parent;
                if (root >= model->boneCount) root = 0;
                Transform* parentTransform = &model->bindPose[root];

                currentTransform->rotation = QuaternionMultiply(parentTransform->rotation, currentTransform->rotation);
                currentTransform->translation = Vector3RotateByQuaternion(currentTransform->translation, parentTransform->rotation);
                currentTransform->translation = Vector3Add(currentTransform->translation, parentTransform->translation);
                currentTransform->scale = Vector3Multiply(currentTransform->scale, parentTransform->scale);
                completedBones[i] = true;
                numberCompletedBones++;
            }
        }

        RL_FREE(completedBones);
    }
}

static void LoadGLTFMaterial(Model *model, const char *fileName, const cgltf_data *data)
{
    for (int i = 0; i < model->materialCount - 1; i++)
    {
        model->materials[i] = LoadMaterialDefault();
        Color tint = (Color){ 255, 255, 255, 255 };
        const char *texPath = GetDirectoryPath(fileName);

        // Ensure material follows raylib support for PBR (metallic/roughness flow)
        if (data->materials[i].has_pbr_metallic_roughness)
        {
            tint.r = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[0]*255);
            tint.g = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[1]*255);
            tint.b = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[2]*255);
            tint.a = (unsigned char)(data->materials[i].pbr_metallic_roughness.base_color_factor[3]*255);

            model->materials[i].maps[MATERIAL_MAP_ALBEDO].color = tint;

            if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
            {
                Image albedo = LoadImageFromCgltfImage(data->materials[i].pbr_metallic_roughness.base_color_texture.texture->image, texPath, tint);
                model->materials[i].maps[MATERIAL_MAP_ALBEDO].texture = LoadTextureFromImage(albedo);
                UnloadImage(albedo);
            }

            tint = WHITE;   // Set tint to white after it's been used by Albedo

            if (data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture)
            {
                Image metallicRoughness = LoadImageFromCgltfImage(data->materials[i].pbr_metallic_roughness.metallic_roughness_texture.texture->image, texPath, tint);
                model->materials[i].maps[MATERIAL_MAP_ROUGHNESS].texture = LoadTextureFromImage(metallicRoughness);

                float roughness = data->materials[i].pbr_metallic_roughness.roughness_factor;
                model->materials[i].maps[MATERIAL_MAP_ROUGHNESS].value = roughness;

                float metallic = data->materials[i].pbr_metallic_roughness.metallic_factor;
                model->materials[i].maps[MATERIAL_MAP_METALNESS].value = metallic;

                UnloadImage(metallicRoughness);
            }

            if (data->materials[i].normal_texture.texture)
            {
                Image normalImage = LoadImageFromCgltfImage(data->materials[i].normal_texture.texture->image, texPath, tint);
                model->materials[i].maps[MATERIAL_MAP_NORMAL].texture = LoadTextureFromImage(normalImage);
                UnloadImage(normalImage);
            }

            if (data->materials[i].occlusion_texture.texture)
            {
                Image occulsionImage = LoadImageFromCgltfImage(data->materials[i].occlusion_texture.texture->image, texPath, tint);
                model->materials[i].maps[MATERIAL_MAP_OCCLUSION].texture = LoadTextureFromImage(occulsionImage);
                UnloadImage(occulsionImage);
            }

            if (data->materials[i].emissive_texture.texture)
            {
                Image emissiveImage = LoadImageFromCgltfImage(data->materials[i].emissive_texture.texture->image, texPath, tint);
                model->materials[i].maps[MATERIAL_MAP_EMISSION].texture = LoadTextureFromImage(emissiveImage);
                tint.r = (unsigned char)(data->materials[i].emissive_factor[0]*255);
                tint.g = (unsigned char)(data->materials[i].emissive_factor[1]*255);
                tint.b = (unsigned char)(data->materials[i].emissive_factor[2]*255);
                model->materials[i].maps[MATERIAL_MAP_EMISSION].color = tint;
                UnloadImage(emissiveImage);
            }
        }
    }

    model->materials[model->materialCount - 1] = LoadMaterialDefault();
}

static void BindGLTFPrimitiveToBones(Model *model, const cgltf_data *data, int primitiveIndex)
{
    for (unsigned int nodeId = 0; nodeId < data->nodes_count; nodeId++)
    {
        if (data->nodes[nodeId].mesh == &(data->meshes[primitiveIndex]))
        {
            if (model->meshes[primitiveIndex].boneIds == NULL)
            {
                model->meshes[primitiveIndex].boneIds = RL_CALLOC(model->meshes[primitiveIndex].vertexCount*4, sizeof(int));
                model->meshes[primitiveIndex].boneWeights = RL_CALLOC(model->meshes[primitiveIndex].vertexCount*4, sizeof(float));

                for (int b = 0; b < model->meshes[primitiveIndex].vertexCount*4; b++)
                {
                    if (b%4 == 0)
                    {
                        model->meshes[primitiveIndex].boneIds[b] = nodeId;
                        model->meshes[primitiveIndex].boneWeights[b] = 1.0f;
                    }
                    else
                    {
                        model->meshes[primitiveIndex].boneIds[b] = 0;
                        model->meshes[primitiveIndex].boneWeights[b] = 0.0f;
                    }
                }
            }
        }
    }
}

// LoadGLTF loads in animation data from given filename
static ModelAnimation *LoadGLTFModelAnimations(const char *fileName, int *animCount)
{
    /***********************************************************************************

        Function implemented by Hristo Stamenov (@object71)

        Features:
          - Supports .gltf and .glb files

        Some restrictions (not exhaustive):
          - ...

    *************************************************************************************/

    // glTF file loading
    unsigned int dataSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &dataSize);

    ModelAnimation *animations = NULL;

    if (fileData == NULL) return animations;

    // glTF data loading
    cgltf_options options = { 0 };
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);

    if (result == cgltf_result_success)
    {
        TRACELOG(LOG_INFO, "MODEL: [%s] glTF animations (%s) count: %i", fileName, (data->file_type == 2)? "glb" :
        "gltf", data->animations_count);

        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TRACELOG(LOG_WARNING, "MODEL: [%s] unable to load glTF animations data", fileName);
        animations = RL_MALLOC(data->animations_count*sizeof(ModelAnimation));
        *animCount = (int)data->animations_count;

        for (unsigned int a = 0; a < data->animations_count; a++)
        {
            // gltf animation consists of the following structures:
            // - nodes - bones are part of the node system (the whole node system is animatable)
            // - channels - single transformation type on a single bone
            //     - node - animatable node
            //     - transformation type (path) - translation, rotation, scale
            //     - sampler - animation samples
            //         - input - points in time this transformation happens
            //         - output - the transformation amount at the given input points in time
            //         - interpolation - the type of interpolation to use between the frames

            cgltf_animation *animation = data->animations + a;

            ModelAnimation *output = animations + a;

            // 30 frames sampled per second
            const float timeStep = (1.0f/60.0f);
            float animationDuration = 0.0f;

            // Getting the max animation time to consider for animation duration
            for (unsigned int i = 0; i < animation->channels_count; i++)
            {
                cgltf_animation_channel *channel = animation->channels + i;
                int frameCounts = (int)channel->sampler->input->count;
                float lastFrameTime = 0.0f;

                if (ReadGLTFValue(channel->sampler->input, frameCounts - 1, &lastFrameTime))
                {
                    animationDuration = fmaxf(lastFrameTime, animationDuration);
                }
            }

            output->frameCount = (int)(animationDuration/timeStep);
            output->boneCount = (int)data->nodes_count;
            output->bones = RL_MALLOC(output->boneCount*sizeof(BoneInfo));
            output->framePoses = RL_MALLOC(output->frameCount*sizeof(Transform *));
            // output->framerate = // TODO: Use framerate instead of const timestep

            // Name and parent bones
            for (int j = 0; j < output->boneCount; j++)
            {
                strcpy(output->bones[j].name, data->nodes[j].name == 0 ? "ANIMJOINT" : data->nodes[j].name);
                output->bones[j].parent = (data->nodes[j].parent != NULL) ? (int)(data->nodes[j].parent - data->nodes) : -1;
            }

            // Allocate data for frames
            // Initiate with zero bone translations
            for (int frame = 0; frame < output->frameCount; frame++)
            {
                output->framePoses[frame] = RL_MALLOC(output->boneCount*sizeof(Transform));

                for (int i = 0; i < output->boneCount; i++)
                {
                    if (data->nodes[i].has_translation) memcpy(&output->framePoses[frame][i].translation, data->nodes[i].translation, 3*sizeof(float));
                    else output->framePoses[frame][i].translation = Vector3Zero();

                    if (data->nodes[i].has_rotation) memcpy(&output->framePoses[frame][i], data->nodes[i].rotation, 4*sizeof(float));
                    else output->framePoses[frame][i].rotation = QuaternionIdentity();

                    output->framePoses[frame][i].rotation = QuaternionNormalize(output->framePoses[frame][i].rotation);

                    if (data->nodes[i].has_scale) memcpy(&output->framePoses[frame][i].scale, data->nodes[i].scale, 3*sizeof(float));
                    else output->framePoses[frame][i].scale = Vector3One();
                }
            }

            // for each single transformation type on single bone
            for (unsigned int channelId = 0; channelId < animation->channels_count; channelId++)
            {
                cgltf_animation_channel *channel = animation->channels + channelId;
                cgltf_animation_sampler *sampler = channel->sampler;

                int boneId = (int)(channel->target_node - data->nodes);

                for (int frame = 0; frame < output->frameCount; frame++)
                {
                    bool shouldSkipFurtherTransformation = true;
                    int outputMin = 0;
                    int outputMax = 0;
                    float frameTime = frame*timeStep;
                    float lerpPercent = 0.0f;

                    // For this transformation:
                    // getting between which input values the current frame time position
                    // and also what is the percent to use in the linear interpolation later
                    for (unsigned int j = 0; j < sampler->input->count; j++)
                    {
                        float inputFrameTime;
                        if (ReadGLTFValue(sampler->input, j, &inputFrameTime))
                        {
                            if (frameTime < inputFrameTime)
                            {
                                shouldSkipFurtherTransformation = false;
                                outputMin = (j == 0) ? 0 : j - 1;
                                outputMax = j;

                                float previousInputTime = 0.0f;
                                if (ReadGLTFValue(sampler->input, outputMin, &previousInputTime))
                                {
                                    if ((inputFrameTime - previousInputTime) != 0)
                                    {
                                        lerpPercent = (frameTime - previousInputTime)/(inputFrameTime - previousInputTime);
                                    }
                                }

                                break;
                            }
                        }
                        else break;
                    }

                    // If the current transformation has no information for the current frame time point
                    if (shouldSkipFurtherTransformation) continue;

                    if (channel->target_path == cgltf_animation_path_type_translation)
                    {
                        Vector3 translationStart;
                        Vector3 translationEnd;

                        float values[3];

                        bool success = ReadGLTFValue(sampler->output, outputMin, values);

                        translationStart.x = values[0];
                        translationStart.y = values[1];
                        translationStart.z = values[2];

                        success = ReadGLTFValue(sampler->output, outputMax, values) || success;

                        translationEnd.x = values[0];
                        translationEnd.y = values[1];
                        translationEnd.z = values[2];

                        if (success) output->framePoses[frame][boneId].translation = Vector3Lerp(translationStart, translationEnd, lerpPercent);
                    }
                    if (channel->target_path == cgltf_animation_path_type_rotation)
                    {
                        Vector4 rotationStart;
                        Vector4 rotationEnd;

                        float values[4];

                        bool success = ReadGLTFValue(sampler->output, outputMin, &values);

                        rotationStart.x = values[0];
                        rotationStart.y = values[1];
                        rotationStart.z = values[2];
                        rotationStart.w = values[3];

                        success = ReadGLTFValue(sampler->output, outputMax, &values) || success;

                        rotationEnd.x = values[0];
                        rotationEnd.y = values[1];
                        rotationEnd.z = values[2];
                        rotationEnd.w = values[3];

                        if (success)
                        {
                            output->framePoses[frame][boneId].rotation = QuaternionNlerp(rotationStart, rotationEnd, lerpPercent);
                        }
                    }
                    if (channel->target_path == cgltf_animation_path_type_scale)
                    {
                        Vector3 scaleStart;
                        Vector3 scaleEnd;

                        float values[3];

                        bool success = ReadGLTFValue(sampler->output, outputMin, &values);

                        scaleStart.x = values[0];
                        scaleStart.y = values[1];
                        scaleStart.z = values[2];

                        success = ReadGLTFValue(sampler->output, outputMax, &values) || success;

                        scaleEnd.x = values[0];
                        scaleEnd.y = values[1];
                        scaleEnd.z = values[2];

                        if (success) output->framePoses[frame][boneId].scale = Vector3Lerp(scaleStart, scaleEnd, lerpPercent);
                    }
                }
            }

            // Build frameposes
            for (int frame = 0; frame < output->frameCount; frame++)
            {
                bool *completedBones = RL_CALLOC(output->boneCount, sizeof(bool));
                int numberCompletedBones = 0;

                while (numberCompletedBones < output->boneCount)
                {
                    for (int i = 0; i < output->boneCount; i++)
                    {
                        if (completedBones[i]) continue;

                        if (output->bones[i].parent < 0)
                        {
                            completedBones[i] = true;
                            numberCompletedBones++;
                            continue;
                        }

                        if (!completedBones[output->bones[i].parent]) continue;

                        output->framePoses[frame][i].rotation = QuaternionMultiply(output->framePoses[frame][output->bones[i].parent].rotation, output->framePoses[frame][i].rotation);
                        output->framePoses[frame][i].translation = Vector3RotateByQuaternion(output->framePoses[frame][i].translation, output->framePoses[frame][output->bones[i].parent].rotation);
                        output->framePoses[frame][i].translation = Vector3Add(output->framePoses[frame][i].translation, output->framePoses[frame][output->bones[i].parent].translation);
                        output->framePoses[frame][i].scale = Vector3Multiply(output->framePoses[frame][i].scale, output->framePoses[frame][output->bones[i].parent].scale);
                        completedBones[i] = true;
                        numberCompletedBones++;
                    }
                }

                RL_FREE(completedBones);
            }
        }

        cgltf_free(data);
    }
    else TRACELOG(LOG_WARNING, ": [%s] Failed to load glTF data", fileName);

    RL_FREE(fileData);

    return animations;
}

void LoadGLTFMesh(cgltf_data *data, cgltf_mesh *mesh, Model *outModel, Matrix currentTransform, int *primitiveIndex, const char *fileName)
{
    for (unsigned int p = 0; p < mesh->primitives_count; p++)
    {
        for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
        {
            if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position)
            {
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                outModel->meshes[(*primitiveIndex)].vertexCount = (int)acc->count;
                int bufferSize = outModel->meshes[(*primitiveIndex)].vertexCount*3*sizeof(float);
                outModel->meshes[(*primitiveIndex)].animVertices = RL_MALLOC(bufferSize);

                outModel->meshes[(*primitiveIndex)].vertices = ReadGLTFValuesAs(acc, cgltf_component_type_r_32f, false);

                // Transform using the nodes matrix attributes
                for (int v = 0; v < outModel->meshes[(*primitiveIndex)].vertexCount; v++)
                {
                    Vector3 vertex = {
                            outModel->meshes[(*primitiveIndex)].vertices[(v*3 + 0)],
                            outModel->meshes[(*primitiveIndex)].vertices[(v*3 + 1)],
                            outModel->meshes[(*primitiveIndex)].vertices[(v*3 + 2)] };

                    vertex = Vector3Transform(vertex, currentTransform);

                    outModel->meshes[(*primitiveIndex)].vertices[(v*3 + 0)] = vertex.x;
                    outModel->meshes[(*primitiveIndex)].vertices[(v*3 + 1)] = vertex.y;
                    outModel->meshes[(*primitiveIndex)].vertices[(v*3 + 2)] = vertex.z;
                }

                memcpy(outModel->meshes[(*primitiveIndex)].animVertices, outModel->meshes[(*primitiveIndex)].vertices, bufferSize);
            }
            else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_normal)
            {
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;

                int bufferSize = (int)(acc->count*3*sizeof(float));
                outModel->meshes[(*primitiveIndex)].animNormals = RL_MALLOC(bufferSize);

                outModel->meshes[(*primitiveIndex)].normals = ReadGLTFValuesAs(acc, cgltf_component_type_r_32f, false);

                // Transform using the nodes matrix attributes
                for (int v = 0; v < outModel->meshes[(*primitiveIndex)].vertexCount; v++)
                {
                    Vector3 normal = {
                            outModel->meshes[(*primitiveIndex)].normals[(v*3 + 0)],
                            outModel->meshes[(*primitiveIndex)].normals[(v*3 + 1)],
                            outModel->meshes[(*primitiveIndex)].normals[(v*3 + 2)] };

                    normal = Vector3Transform(normal, currentTransform);

                    outModel->meshes[(*primitiveIndex)].normals[(v*3 + 0)] = normal.x;
                    outModel->meshes[(*primitiveIndex)].normals[(v*3 + 1)] = normal.y;
                    outModel->meshes[(*primitiveIndex)].normals[(v*3 + 2)] = normal.z;
                }

                memcpy(outModel->meshes[(*primitiveIndex)].animNormals, outModel->meshes[(*primitiveIndex)].normals, bufferSize);
            }
            else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_texcoord)
            {
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                outModel->meshes[(*primitiveIndex)].texcoords = ReadGLTFValuesAs(acc, cgltf_component_type_r_32f, false);
            }
            else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_joints)
            {
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                unsigned int boneCount = acc->count;
                unsigned int totalBoneWeights = boneCount*4;

                outModel->meshes[(*primitiveIndex)].boneIds = RL_MALLOC(totalBoneWeights*sizeof(int));
                short *bones = ReadGLTFValuesAs(acc, cgltf_component_type_r_16, false);
                for (unsigned int a = 0; a < totalBoneWeights; a++)
                {
                    outModel->meshes[(*primitiveIndex)].boneIds[a] = 0;
                    if (bones[a] < 0) continue;

                    cgltf_node* skinJoint = data->skins->joints[bones[a]];
                    for (unsigned int k = 0; k < data->nodes_count; k++)
                    {
                        if (data->nodes + k == skinJoint)
                        {
                            outModel->meshes[(*primitiveIndex)].boneIds[a] = k;
                            break;
                        }
                    }
                }
                RL_FREE(bones);
            }
            else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_weights)
            {
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                outModel->meshes[(*primitiveIndex)].boneWeights = ReadGLTFValuesAs(acc, cgltf_component_type_r_32f, false);
            }
            else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_color)
            {
                cgltf_accessor *acc = mesh->primitives[p].attributes[j].data;
                outModel->meshes[(*primitiveIndex)].colors = ReadGLTFValuesAs(acc, cgltf_component_type_r_8u, true);
            }
        }

        cgltf_accessor *acc = mesh->primitives[p].indices;
        if (acc)
        {
            outModel->meshes[(*primitiveIndex)].triangleCount = acc->count/3;
            outModel->meshes[(*primitiveIndex)].indices = ReadGLTFValuesAs(acc, cgltf_component_type_r_16u, false);
        }
        else
        {
            // Unindexed mesh
            outModel->meshes[(*primitiveIndex)].triangleCount = outModel->meshes[(*primitiveIndex)].vertexCount/3;
        }

        if (mesh->primitives[p].material)
        {
            // Compute the offset
            outModel->meshMaterial[(*primitiveIndex)] = (int)(mesh->primitives[p].material - data->materials);
        }
        else outModel->meshMaterial[(*primitiveIndex)] = outModel->materialCount - 1;

        BindGLTFPrimitiveToBones(outModel, data, *primitiveIndex);

        (*primitiveIndex) = (*primitiveIndex) + 1;
    }
}

static Matrix GetNodeTransformationMatrix(cgltf_node *node, Matrix current)
{
    if (node->has_matrix)
    {
        Matrix nodeTransform = {
        node->matrix[0], node->matrix[4], node->matrix[8], node->matrix[12],
        node->matrix[1], node->matrix[5], node->matrix[9], node->matrix[13],
        node->matrix[2], node->matrix[6], node->matrix[10], node->matrix[14],
        node->matrix[3], node->matrix[7], node->matrix[11], node->matrix[15] };
        current= MatrixMultiply(nodeTransform, current);
    }
    if (node->has_translation)
    {
        Matrix tl = MatrixTranslate(node->translation[0],node->translation[1],node->translation[2]);
        current = MatrixMultiply(tl, current);
    }
    if (node->has_rotation)
    {
        Matrix rot = QuaternionToMatrix((Quaternion){node->rotation[0],node->rotation[1],node->rotation[2],node->rotation[3]});
        current = MatrixMultiply(rot, current);
    }
    if (node->has_scale)
    {
        Matrix scale = MatrixScale(node->scale[0],node->scale[1],node->scale[2]);
        current = MatrixMultiply(scale, current);
    }
    return current;
}

void LoadGLTFNode(cgltf_data *data, cgltf_node *node, Model *outModel, Matrix currentTransform, int *primitiveIndex, const char *fileName)
{
    // Apply the transforms if they exist (Will still be applied even if no mesh is present to support emptys and bone structures)
    Matrix localTransform = GetNodeTransformationMatrix(node, MatrixIdentity());
    currentTransform = MatrixMultiply(localTransform, currentTransform);
    // Load mesh if it exists
    if (node->mesh != NULL)
    {
        // Check if skinning is enabled and load Mesh accordingly
        Matrix vertexTransform = currentTransform;
        if((node->skin != NULL) && (node->parent != NULL))
        {
            vertexTransform = localTransform;
            TRACELOG(LOG_WARNING,"MODEL: GLTF Node %s is skinned but not root node! Parent transformations will be ignored (NODE_SKINNED_MESH_NON_ROOT)",node->name);
        }
        LoadGLTFMesh(data, node->mesh, outModel, vertexTransform, primitiveIndex, fileName);
    }
    for (unsigned int i = 0; i < node->children_count; i++) LoadGLTFNode(data, node->children[i], outModel, currentTransform, primitiveIndex, fileName);
}

static void GetGLTFPrimitiveCount(cgltf_node *node, int *outCount)
{
    if (node->mesh != NULL) *outCount += node->mesh->primitives_count;

    for (unsigned int i = 0; i < node->children_count; i++) GetGLTFPrimitiveCount(node->children[i], outCount);
}

#endif

#if defined(SUPPORT_FILEFORMAT_VOX)
// Load VOX (MagikaVoxel) mesh data
static Model LoadVOX(const char *fileName)
{
    Model model = { 0 };
    int nbvertices = 0;
    int meshescount = 0;
    
    VoxArray3D voxarray = { 0 };
    int ret = Vox_LoadFileName(fileName, &voxarray);

    if (ret != VOX_SUCCESS)
    {
        TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load VOX data", fileName);
        return model;
    }
    else
    {
        // Compute meshes count
        nbvertices = voxarray.vertices.used;
        meshescount = 1 + (nbvertices/65536);

        TRACELOG(LOG_INFO, "MODEL: [%s] VOX data loaded successfully : %i vertices/%i meshes", fileName, nbvertices, meshescount);
    }

    // Build models from meshes
    model.transform = MatrixIdentity();

    model.meshCount = meshescount;
    model.meshes = (Mesh *)MemAlloc(model.meshCount*sizeof(Mesh));

    model.meshMaterial = (int *)MemAlloc(model.meshCount*sizeof(int));

    model.materialCount = 1;
    model.materials = (Material *)MemAlloc(model.materialCount*sizeof(Material));
    model.materials[0] = LoadMaterialDefault();

    // Init model meshes
    int verticesRemain = voxarray.vertices.used;
    int verticesMax = 65532; // 5461 voxels x 12 vertices per voxel -> 65532 (must be inf 65536)

    Vector3 *pvertices = voxarray.vertices.array;	    // 6*4 = 12 vertices per voxel
    Color *pcolors = voxarray.colors.array;
    unsigned short *pindices = voxarray.indices.array;	// 5461*6*6 = 196596 indices max per mesh

    int size = 0;

    for (int idxMesh = 0; idxMesh < meshescount; idxMesh++)
    {
        Mesh *pmesh = &model.meshes[idxMesh];
        memset(pmesh, 0, sizeof(Mesh));

        // Copy vertices
        pmesh->vertexCount = (int)fmin(verticesMax, verticesRemain);

        size = pmesh->vertexCount*sizeof(float)*3;
        pmesh->vertices = MemAlloc(size);
        memcpy(pmesh->vertices, pvertices, size);

        // Copy indices 
        // TODO: compute globals indices array
        size = voxarray.indices.used * sizeof(unsigned short);
        pmesh->indices = MemAlloc(size);
        memcpy(pmesh->indices, pindices, size);

        pmesh->triangleCount = (pmesh->vertexCount/4)*2;

        // Copy colors
        size = pmesh->vertexCount*sizeof(Color);
        pmesh->colors = MemAlloc(size);
        memcpy(pmesh->colors, pcolors, size);

        // First material index
        model.meshMaterial[idxMesh] = 0;

        // Upload mesh data to GPU
        UploadMesh(pmesh, false);

        verticesRemain -= verticesMax;
        pvertices += verticesMax;
        pcolors += verticesMax;
    }

    Vox_FreeArrays(&voxarray);

    return model;
}
#endif