/**********************************************************************************************
*
*   riqm - InterQuake Model format (IQM) loader for animated meshes
*
*   CONFIGURATION:
*
*   #define RIQM_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2018 Jonas Daeyaert (@culacant) and Ramon Santamaria (@raysan5)
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

#ifndef RIQM_H
#define RIQM_H

//#define RIQM_STATIC
#ifdef RIQM_STATIC
    #define RIQMDEF static              // Functions just visible to module including this file
#else
    #ifdef __cplusplus
        #define RIQMDEF extern "C"      // Functions visible from other files (no name mangling of functions in C++)
    #else
        #define RIQMDEF extern          // Functions visible from other files
    #endif
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

#define JOINT_NAME_LENGTH    32          // Joint name string length
#define MESH_NAME_LENGTH     32          // Mesh name string length

typedef struct Joint {
    char name[JOINT_NAME_LENGTH];
    int parent;
} Joint;

typedef struct Pose {
    Vector3 translation;
    Quaternion rotation;
    Vector3 scale;
} Pose;

typedef struct Animation {
    int jointCount;
    Joint *joints;      // NOTE: Joints in anims do not have names

    int frameCount;
    float framerate;

    Pose **framepose;
} Animation;

typedef struct AnimatedMesh {

    //Mesh mesh;

    // Mesh struct defines:
    //-------------------------
    int vertexCount;
    int triangleCount;

    float *vertices;
    float *normals;
    float *texcoords;
    unsigned short *triangles;    //equivalent to mes.indices
    
    unsigned int vaoId;
    unsigned int vboId[7];
    //-------------------------
    
    char name[MESH_NAME_LENGTH];
    
    float *animVertices;
    float *animNormals;
    float *weightBias;
    int *weightId;

} AnimatedMesh;

typedef struct AnimatedModel {
    int meshCount;
    AnimatedMesh *mesh;

    int materialCount;
    int *meshMaterialId;
    Material *materials;

    int jointCount;
    Joint *joints;

    Pose *basepose;
    Matrix transform;
} AnimatedModel;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Loading/Unloading functions
RIQMDEF AnimatedModel LoadAnimatedModel(const char *filename);
RIQMDEF void UnloadAnimatedModel(AnimatedModel model);
RIQMDEF Animation LoadAnimation(const char *filename);
RIQMDEF void UnloadAnimation(Animation anim);

RIQMDEF AnimatedModel AnimatedModelAddTexture(AnimatedModel model,const char *filename);    // GENERIC!
RIQMDEF AnimatedModel SetMeshMaterial(AnimatedModel model,int meshid, int textureid);       // GENERIC!


// Usage functionality
RIQMDEF bool CheckSkeletonsMatch(AnimatedModel model, Animation anim);
RIQMDEF void AnimateModel(AnimatedModel model, Animation anim, int frame);
RIQMDEF void DrawAnimatedModel(AnimatedModel model,Vector3 position,float scale,Color tint);
RIQMDEF void DrawAnimatedModelEx(AnimatedModel model,Vector3 position,Vector3 rotationAxis,float rotationAngle, Vector3 scale,Color tint);

#endif // RIQM_H


/***********************************************************************************
*
*  RIQM IMPLEMENTATION
*
************************************************************************************/

#if defined(RIQM_IMPLEMENTATION)

//#include "utils.h"          // Required for: fopen() Android mapping

#include <stdio.h>          // Required for: FILE, fopen(), fclose(), feof(), fseek(), fread()
#include <stdlib.h>         // Required for: malloc(), free()
#include <string.h>         // Required for: strncmp(),strcpy()

#include "raymath.h"        // Required for: Vector3, Quaternion functions
#include "rlgl.h"           // raylib OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2

#include "glad.h"           // Required for OpenGL functions > TO BE REMOVED!


//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define IQM_MAGIC       "INTERQUAKEMODEL"   // IQM file magic number
#define IQM_VERSION     2                   // only IQM version 2 supported
#define ANIMJOINTNAME   "ANIMJOINT"         // default joint name (used in Animation)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// iqm file structs
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

typedef struct IQMAdjacency {                 // adjacency unused by default
    unsigned int triangle[3];
} IQMAdjacency;

typedef struct IQMJoint {
    unsigned int name;
    int parent;
    float translate[3], rotate[4], scale[3];
} IQMJoint;

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

typedef struct IQMVertexArray {
    unsigned int type;
    unsigned int flags;
    unsigned int format;
    unsigned int size;
    unsigned int offset;
} IQMVertexArray;

typedef struct IQMBounds {                    // bounds unused by default
    float bbmin[3], bbmax[3];
    float xyradius, radius;
} IQMBounds;


typedef enum {
    IQM_POSITION     = 0,
    IQM_TEXCOORD     = 1,
    IQM_NORMAL       = 2,
    IQM_TANGENT      = 3,                    // tangents unused by default
    IQM_BLENDINDEXES = 4,
    IQM_BLENDWEIGHTS = 5,
    IQM_COLOR        = 6,                    // vertex colors unused by default
    IQM_CUSTOM       = 0x10                  // custom vertex values unused by default
} IQMVertexType;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
void rlLoadAnimatedMesh(AnimatedMesh *amesh, bool dynamic);
void rlUnloadAnimatedMesh(AnimatedMesh *amesh);
void rlUpdateAnimatedMesh(AnimatedMesh *amesh);
void rlDrawAnimatedMesh(AnimatedMesh amesh, Material material, Matrix transform);

static AnimatedModel LoadIQM(const char *filename);

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void rlLoadAnimatedMesh(AnimatedMesh *amesh, bool dynamic)
{
    amesh->vaoId = 0;        // Vertex Array Object
    amesh->vboId[0] = 0;     // Vertex positions VBO  << these are the animated vertices in animVertices
    amesh->vboId[1] = 0;     // Vertex texcoords VBO
    amesh->vboId[2] = 0;     // Vertex normals VBO    << these are the animated normals in animNormals
    amesh->vboId[3] = 0;     // Vertex colors VBO
    amesh->vboId[4] = 0;     // Vertex tangents VBO       UNUSED
    amesh->vboId[5] = 0;     // Vertex texcoords2 VBO     UNUSED
    amesh->vboId[6] = 0;     // Vertex indices VBO

#if defined(GRAPHICS_API_OPENGL_11)
TraceLog(LOG_WARNING, "OGL 11");
#endif
#if defined(GRAPHICS_API_OPENGL_21)
TraceLog(LOG_WARNING, "OGL 21");
#endif
#if defined(GRAPHICS_API_OPENGL_33)
TraceLog(LOG_WARNING, "OGL 33");
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
TraceLog(LOG_WARNING, "OGL ES2");
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    int drawHint = GL_STATIC_DRAW;
    if (dynamic) drawHint = GL_DYNAMIC_DRAW;

    //if (vaoSupported)
    {
        // Initialize Quads VAO (Buffer A)
        glGenVertexArrays(1, &amesh->vaoId);
        glBindVertexArray(amesh->vaoId);
    }

    // NOTE: Attributes must be uploaded considering default locations points

    // Enable vertex attributes: position (shader-location = 0)
    glGenBuffers(1, &amesh->vboId[0]);
    glBindBuffer(GL_ARRAY_BUFFER, amesh->vboId[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*amesh->vertexCount, amesh->animVertices, drawHint);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(0);

    // Enable vertex attributes: texcoords (shader-location = 1)
    glGenBuffers(1, &amesh->vboId[1]);
    glBindBuffer(GL_ARRAY_BUFFER, amesh->vboId[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*amesh->vertexCount, amesh->texcoords, drawHint);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(1);

    // Enable vertex attributes: normals (shader-location = 2)
    if (amesh->animNormals != NULL)
    {
        glGenBuffers(1, &amesh->vboId[2]);
        glBindBuffer(GL_ARRAY_BUFFER, amesh->vboId[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*amesh->vertexCount, amesh->animNormals, drawHint);
        glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(2);
    }
    else
    {
        // Default color vertex attribute set to WHITE
        glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f);
        glDisableVertexAttribArray(2);
    }
// colors UNUSED
/*
    // Default color vertex attribute (shader-location = 3)
    if (mesh->colors != NULL)
    {
        glGenBuffers(1, &amesh->vboId[3]);
        glBindBuffer(GL_ARRAY_BUFFER, amesh->vboId[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned char)*4*mesh->vertexCount, mesh->colors, drawHint);
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(3);
    }
    else
    {
        // Default color vertex attribute set to WHITE
        glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
        glDisableVertexAttribArray(3);
    }
*/
// colors to default
    glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
    glDisableVertexAttribArray(3);

// tangents UNUSED
/*
    // Default tangent vertex attribute (shader-location = 4)
    if (mesh->tangents != NULL)
    {
        glGenBuffers(1, &mesh->vboId[4]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[4]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*mesh->vertexCount, mesh->tangents, drawHint);
        glVertexAttribPointer(4, 4, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(4);
    }
    else
    {
        // Default tangents vertex attribute
        glVertexAttrib4f(4, 0.0f, 0.0f, 0.0f, 0.0f);
        glDisableVertexAttribArray(4);
    }
*/
// tangents to default
    glVertexAttrib4f(4, 0.0f, 0.0f, 0.0f, 0.0f);
    glDisableVertexAttribArray(4);

// texcoords2 UNUSED
/*
    // Default texcoord2 vertex attribute (shader-location = 5)
    if (mesh->texcoords2 != NULL)
    {
        glGenBuffers(1, &mesh->vboId[5]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId[5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->vertexCount, mesh->texcoords2, drawHint);
        glVertexAttribPointer(5, 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(5);
    }
    else
    {
        // Default texcoord2 vertex attribute
        glVertexAttrib2f(5, 0.0f, 0.0f);
        glDisableVertexAttribArray(5);
    }
*/
// texcoords2  to default
    glVertexAttrib2f(5, 0.0f, 0.0f);
    glDisableVertexAttribArray(5);

    if (amesh->triangles != NULL)
    {
        glGenBuffers(1, &amesh->vboId[6]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, amesh->vboId[6]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*amesh->triangleCount*3, amesh->triangles, GL_STATIC_DRAW);
    }

    if (amesh->vaoId > 0) TraceLog(LOG_INFO, "[VAO ID %i] Mesh uploaded successfully to VRAM (GPU)", amesh->vaoId);
    else TraceLog(LOG_WARNING, "Mesh could not be uploaded to VRAM (GPU)");
#endif
}

// Unload mesh data from CPU and GPU
void rlUnloadAnimatedMesh(AnimatedMesh *amesh)
{
    if (amesh->vertices != NULL) free(amesh->vertices);
    if (amesh->animVertices != NULL) free(amesh->animVertices);
    if (amesh->texcoords != NULL) free(amesh->texcoords);
    if (amesh->normals != NULL) free(amesh->normals);
    if (amesh->animNormals != NULL) free(amesh->animNormals);
//    if (mesh->colors != NULL) free(mesh->colors);
//    if (mesh->tangents != NULL) free(mesh->tangents);
//    if (mesh->texcoords2 != NULL) free(mesh->texcoords2);
    if (amesh->triangles != NULL) free(amesh->triangles);
    if (amesh->weightId != NULL) free(amesh->weightId);
    if (amesh->weightBias != NULL) free(amesh->weightBias);

    rlDeleteBuffers(amesh->vboId[0]);   // vertex
    rlDeleteBuffers(amesh->vboId[1]);   // texcoords
    rlDeleteBuffers(amesh->vboId[2]);   // normals
    rlDeleteBuffers(amesh->vboId[3]);   // colors
    rlDeleteBuffers(amesh->vboId[4]);   // tangents
    rlDeleteBuffers(amesh->vboId[5]);   // texcoords2
    rlDeleteBuffers(amesh->vboId[6]);   // indices

    rlDeleteVertexArrays(amesh->vaoId);
}

// Update vertex and normal data into GPU
void rlUpdateAnimatedMesh(AnimatedMesh *amesh)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Activate mesh VAO
    glBindVertexArray(amesh->vaoId);

    // Update positions data
    glBindBuffer(GL_ARRAY_BUFFER, amesh->vboId[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*amesh->vertexCount, amesh->animVertices);

    // Update normals data
    glBindBuffer(GL_ARRAY_BUFFER, amesh->vboId[2]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*amesh->vertexCount, amesh->animNormals);

    // Unbind the current VAO
    glBindVertexArray(0);

    //mesh.vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    // Now we can modify vertices
    //glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
}

// Draw a 3d mesh with material and transform
void rlDrawAnimatedMesh(AnimatedMesh amesh, Material material, Matrix transform)
{
#if defined(GRAPHICS_API_OPENGL_11)
/*
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, material.maps[MAP_DIFFUSE].texture.id);

    // NOTE: On OpenGL 1.1 we use Vertex Arrays to draw model
    glEnableClientState(GL_VERTEX_ARRAY);                   // Enable vertex array
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);            // Enable texture coords array

    //if (amesh.normals != NULL) glEnableClientState(GL_NORMAL_ARRAY);     // Enable normals array
    //if (amesh.colors != NULL) glEnableClientState(GL_COLOR_ARRAY);       // Enable colors array

    glVertexPointer(3, GL_FLOAT, 0, amesh.animVertices);         // Pointer to vertex coords array
    glTexCoordPointer(2, GL_FLOAT, 0, amesh.texcoords);      // Pointer to texture coords array
    if (amesh.animNormals != NULL) glNormalPointer(GL_FLOAT, 0, amesh.animNormals);           // Pointer to normals array
    //if (mesh.colors != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, mesh.colors);   // Pointer to colors array

    rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(transform));
        rlColor4ub(material.maps[MAP_DIFFUSE].color.r, material.maps[MAP_DIFFUSE].color.g, material.maps[MAP_DIFFUSE].color.b, material.maps[MAP_DIFFUSE].color.a);

        if (amesh.triangles != NULL) glDrawElements(GL_TRIANGLES, amesh.triangleCount*3, GL_UNSIGNED_SHORT, amesh.triangles);
        else glDrawArrays(GL_TRIANGLES, 0, amesh.vertexCount);
    rlPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);                  // Disable vertex array
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);           // Disable texture coords array
    if (amesh.animNormals != NULL) glDisableClientState(GL_NORMAL_ARRAY);    // Disable normals array
    //if (mesh.colors != NULL) glDisableClientState(GL_NORMAL_ARRAY);     // Disable colors array

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
*/
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Bind shader program
    glUseProgram(material.shader.id);

    // Matrices and other values required by shader
    //-----------------------------------------------------
    // Calculate and send to shader model matrix (used by PBR shader)
    if (material.shader.locs[LOC_MATRIX_MODEL] != -1) SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_MODEL], transform);

    // Upload to shader material.colDiffuse
    if (material.shader.locs[LOC_COLOR_DIFFUSE] != -1)
        glUniform4f(material.shader.locs[LOC_COLOR_DIFFUSE], (float)material.maps[MAP_DIFFUSE].color.r/255.0f,
                                                           (float)material.maps[MAP_DIFFUSE].color.g/255.0f,
                                                           (float)material.maps[MAP_DIFFUSE].color.b/255.0f,
                                                           (float)material.maps[MAP_DIFFUSE].color.a/255.0f);

    // Upload to shader material.colSpecular (if available)
    if (material.shader.locs[LOC_COLOR_SPECULAR] != -1)
        glUniform4f(material.shader.locs[LOC_COLOR_SPECULAR], (float)material.maps[MAP_SPECULAR].color.r/255.0f,
                                                               (float)material.maps[MAP_SPECULAR].color.g/255.0f,
                                                               (float)material.maps[MAP_SPECULAR].color.b/255.0f,
                                                               (float)material.maps[MAP_SPECULAR].color.a/255.0f);

    if (material.shader.locs[LOC_MATRIX_VIEW] != -1) SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_VIEW], GetMatrixModelview());
    if (material.shader.locs[LOC_MATRIX_PROJECTION] != -1) SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_PROJECTION], projection);

    // At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it an no model-drawing function modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matView = GetMatrixModelview();         // View matrix (camera)
    Matrix matProjection = projection;  // Projection matrix (perspective)

    // Calculate model-view matrix combining matModel and matView
    Matrix matModelView = MatrixMultiply(transform, matView);           // Transform to camera-space coordinates
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, material.maps[i].texture.id);
            else glBindTexture(GL_TEXTURE_2D, material.maps[i].texture.id);

            glUniform1i(material.shader.locs[LOC_MAP_DIFFUSE + i], i);
        }
    }
    
    glBindVertexArray(amesh.vaoId);

    /*
    // Bind vertex array objects (or VBOs)
    if (vaoSupported) glBindVertexArray(amesh.vaoId);
    else
    {
        // TODO: Simplify VBO binding into a for loop

        // Bind mesh VBO data: vertex position (shader-location = 0)
        glBindBuffer(GL_ARRAY_BUFFER, amesh.vboId[0]);
        glVertexAttribPointer(material.shader.locs[LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        glBindBuffer(GL_ARRAY_BUFFER, amesh.vboId[1]);
        glVertexAttribPointer(material.shader.locs[LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_TEXCOORD01]);

        // Bind mesh VBO data: vertex normals (shader-location = 2, if available)
        if (material.shader.locs[LOC_VERTEX_NORMAL] != -1)
        {
            glBindBuffer(GL_ARRAY_BUFFER, amesh.vboId[2]);
            glVertexAttribPointer(material.shader.locs[LOC_VERTEX_NORMAL], 3, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[LOC_VERTEX_COLOR] != -1)
        {
            if (amesh.vboId[3] != 0)
            {
                glBindBuffer(GL_ARRAY_BUFFER, amesh.vboId[3]);
                glVertexAttribPointer(material.shader.locs[LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
                glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                glVertexAttrib4f(material.shader.locs[LOC_VERTEX_COLOR], 1.0f, 1.0f, 1.0f, 1.0f);
                glDisableVertexAttribArray(material.shader.locs[LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[LOC_VERTEX_TANGENT] != -1)
        {
            glBindBuffer(GL_ARRAY_BUFFER, amesh.vboId[4]);
            glVertexAttribPointer(material.shader.locs[LOC_VERTEX_TANGENT], 4, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[LOC_VERTEX_TEXCOORD02] != -1)
        {
            glBindBuffer(GL_ARRAY_BUFFER, amesh.vboId[5]);
            glVertexAttribPointer(material.shader.locs[LOC_VERTEX_TEXCOORD02], 2, GL_FLOAT, 0, 0, 0);
            glEnableVertexAttribArray(material.shader.locs[LOC_VERTEX_TEXCOORD02]);
        }

        if (amesh.triangles != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, amesh.vboId[6]);
    }
    */

    int eyesCount = 1;
#if defined(SUPPORT_VR_SIMULATOR)
    if (vrStereoRender) eyesCount = 2;
#endif

    for (int eye = 0; eye < eyesCount; eye++)
    {
        if (eyesCount == 1) modelview = matModelView;
        #if defined(SUPPORT_VR_SIMULATOR)
        else SetStereoView(eye, matProjection, matModelView);
        #endif

        // Calculate model-view-projection matrix (MVP)
        Matrix matMVP = MatrixMultiply(modelview, projection);        // Transform to screen-space coordinates

        // Send combined model-view-projection matrix to shader
        glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_MVP], 1, false, MatrixToFloat(matMVP));

        // Draw call!
        if (amesh.triangles != NULL) glDrawElements(GL_TRIANGLES, amesh.triangleCount*3, GL_UNSIGNED_SHORT, 0); // Indexed vertices draw
        else glDrawArrays(GL_TRIANGLES, 0, amesh.vertexCount);
    }

    // Unbind all binded texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);       // Set shader active texture
        if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        else glBindTexture(GL_TEXTURE_2D, 0);   // Unbind current active texture
    }

    glBindVertexArray(0);
    
    /*
    // Unind vertex array objects (or VBOs)
    if (vaoSupported) glBindVertexArray(0);
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        if (amesh.triangles != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    */
    // Unbind shader program
    glUseProgram(0);

    // Restore projection/modelview matrices
    // NOTE: In stereo rendering matrices are being modified to fit every eye
    projection = matProjection;
    modelview = matView;
#endif
}

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

// Load .iqm file and initialize animated model
AnimatedModel LoadAnimatedModel(const char *filename)
{
    AnimatedModel out = LoadIQM(filename);

    for (int i = 0; i < out.meshCount; i++) rlLoadAnimatedMesh(&out.mesh[i], false);

    out.transform = MatrixIdentity();
    out.meshMaterialId = malloc(sizeof(int)*out.meshCount);
    out.materials = NULL;
    out.materialCount = 0;

    for (int i = 0; i < out.meshCount; i++) out.meshMaterialId[i] = -1;

    return out;
}

// Add a texture to an animated model
AnimatedModel AnimatedModelAddTexture(AnimatedModel model, const char *filename)
{
    Texture2D texture = LoadTexture(filename);

    model.materials = realloc(model.materials, sizeof(Material)*(model.materialCount + 1));
    model.materials[model.materialCount] = LoadMaterialDefault();
    model.materials[model.materialCount].maps[MAP_DIFFUSE].texture = texture;
    model.materialCount++;

    return model;
}

// Set the material for a mesh
AnimatedModel SetMeshMaterial(AnimatedModel model, int meshid, int textureid)
{
    if (meshid > model.meshCount)
    {
        TraceLog(LOG_WARNING, "MeshId greater than meshCount\n");
        return model;
    }

    if (textureid > model.materialCount)
    {
        TraceLog(LOG_WARNING,"textureid greater than materialCount\n");
        return model;
    }

    model.meshMaterialId[meshid] = textureid;

    return model;
}

// Load animations from a .iqm file
Animation LoadAnimationFromIQM(const char *filename)
{
    Animation animation = { 0 };

    FILE *iqmFile;
    IQMHeader iqm;

    iqmFile = fopen(filename,"rb");

    if (!iqmFile)
    {
        TraceLog(LOG_ERROR, "[%s] Unable to open file", filename);
        return animation;
    }

    // header
    fread(&iqm, sizeof(IQMHeader), 1, iqmFile);

    if (strncmp(iqm.magic, IQM_MAGIC, sizeof(IQM_MAGIC)))
    {
        TraceLog(LOG_ERROR, "Magic Number \"%s\"does not match.", iqm.magic);
        fclose(iqmFile);
        return animation;
    }

    if (iqm.version != IQM_VERSION)
    {
        TraceLog(LOG_ERROR, "IQM version %i is incorrect.", iqm.version);
        fclose(iqmFile);
        return animation;
    }

    // header
    if (iqm.num_anims > 1) TraceLog(LOG_WARNING, "More than 1 animation in file, only the first one will get loaded");

    // joints
    IQMPose *poses;
    poses = malloc(sizeof(IQMPose)*iqm.num_poses);
    fseek(iqmFile, iqm.ofs_poses, SEEK_SET);
    fread(poses, sizeof(IQMPose)*iqm.num_poses, 1, iqmFile);

    animation.jointCount = iqm.num_poses;
    animation.joints = malloc(sizeof(Joint)*iqm.num_poses);

    for (int j = 0; j < iqm.num_poses; j++)
    {
        strcpy(animation.joints[j].name, ANIMJOINTNAME);
        animation.joints[j].parent = poses[j].parent;
    }

    // animations
    IQMAnim anim = {0};
    fseek(iqmFile, iqm.ofs_anims, SEEK_SET);
    fread(&anim, sizeof(IQMAnim), 1, iqmFile);

    animation.frameCount = anim.num_frames;
    animation.framerate = anim.framerate;

    // frameposes
    unsigned short *framedata = malloc(sizeof(unsigned short)*iqm.num_frames*iqm.num_framechannels);
    fseek(iqmFile, iqm.ofs_frames, SEEK_SET);
    fread(framedata, sizeof(unsigned short)*iqm.num_frames*iqm.num_framechannels, 1, iqmFile);

    animation.framepose = malloc(sizeof(Pose*)*anim.num_frames);
    for (int j = 0; j < anim.num_frames; j++) animation.framepose[j] = malloc(sizeof(Pose)*iqm.num_poses);

    int dcounter = anim.first_frame*iqm.num_framechannels;

    for (int frame = 0; frame < anim.num_frames; frame++)
    {
        for (int i = 0; i < iqm.num_poses; i++)
        {
            animation.framepose[frame][i].translation.x = poses[i].channeloffset[0];

            if (poses[i].mask & 0x01)
            {
                animation.framepose[frame][i].translation.x += framedata[dcounter]*poses[i].channelscale[0];
                dcounter++;
            }

            animation.framepose[frame][i].translation.y = poses[i].channeloffset[1];

            if (poses[i].mask & 0x02)
            {
                animation.framepose[frame][i].translation.y += framedata[dcounter]*poses[i].channelscale[1];
                dcounter++;
            }

            animation.framepose[frame][i].translation.z = poses[i].channeloffset[2];

            if (poses[i].mask & 0x04)
            {
                animation.framepose[frame][i].translation.z += framedata[dcounter]*poses[i].channelscale[2];
                dcounter++;
            }

            animation.framepose[frame][i].rotation.x = poses[i].channeloffset[3];

            if (poses[i].mask & 0x08)
            {
                animation.framepose[frame][i].rotation.x += framedata[dcounter]*poses[i].channelscale[3];
                dcounter++;
            }

            animation.framepose[frame][i].rotation.y = poses[i].channeloffset[4];

            if (poses[i].mask & 0x10)
            {
                animation.framepose[frame][i].rotation.y += framedata[dcounter]*poses[i].channelscale[4];
                dcounter++;
            }

            animation.framepose[frame][i].rotation.z = poses[i].channeloffset[5];

            if (poses[i].mask & 0x20)
            {
                animation.framepose[frame][i].rotation.z += framedata[dcounter]*poses[i].channelscale[5];
                dcounter++;
            }

            animation.framepose[frame][i].rotation.w = poses[i].channeloffset[6];

            if (poses[i].mask & 0x40)
            {
                animation.framepose[frame][i].rotation.w += framedata[dcounter]*poses[i].channelscale[6];
                dcounter++;
            }

            animation.framepose[frame][i].scale.x = poses[i].channeloffset[7];

            if (poses[i].mask & 0x80)
            {
                animation.framepose[frame][i].scale.x += framedata[dcounter]*poses[i].channelscale[7];
                dcounter++;
            }

            animation.framepose[frame][i].scale.y = poses[i].channeloffset[8];

            if (poses[i].mask & 0x100)
            {
                animation.framepose[frame][i].scale.y += framedata[dcounter]*poses[i].channelscale[8];
                dcounter++;
            }

            animation.framepose[frame][i].scale.z = poses[i].channeloffset[9];

            if (poses[i].mask & 0x200)
            {
                animation.framepose[frame][i].scale.z += framedata[dcounter]*poses[i].channelscale[9];
                dcounter++;
            }

            animation.framepose[frame][i].rotation = QuaternionNormalize(animation.framepose[frame][i].rotation);
        }
    }

    // Build frameposes
    for (int frame = 0; frame < anim.num_frames; frame++)
    {
        for (int i = 0; i < animation.jointCount; i++)
        {
            if (animation.joints[i].parent >= 0)
            {
                animation.framepose[frame][i].rotation = QuaternionMultiply(animation.framepose[frame][animation.joints[i].parent].rotation, animation.framepose[frame][i].rotation);
                animation.framepose[frame][i].translation = Vector3RotateByQuaternion(animation.framepose[frame][i].translation, animation.framepose[frame][animation.joints[i].parent].rotation);
                animation.framepose[frame][i].translation = Vector3Add(animation.framepose[frame][i].translation, animation.framepose[frame][animation.joints[i].parent].translation);
                animation.framepose[frame][i].scale = Vector3MultiplyV(animation.framepose[frame][i].scale, animation.framepose[frame][animation.joints[i].parent].scale);
            }
        }
    }

    free(framedata);
    free(poses);

    fclose(iqmFile);

    return animation;
}

// Unload animated model
void UnloadAnimatedModel(AnimatedModel model)
{
    free(model.materials);
    free(model.meshMaterialId);
    free(model.joints);
    free(model.basepose);

    for (int i = 0; i < model.meshCount; i++) rlUnloadAnimatedMesh(&model.mesh[i]);

    free(model.mesh);
}

// Unload animation
void UnloadAnimation(Animation anim)
{
    free(anim.joints);
    free(anim.framepose);

    for (int i = 0; i < anim.frameCount; i++) free(anim.framepose[i]);
}

// Check if skeletons match, only parents and jointCount are checked
bool CheckSkeletonsMatch(AnimatedModel model, Animation anim)
{
    if (model.jointCount != anim.jointCount) return 0;

    for (int i = 0; i < model.jointCount; i++)
    {
        if (model.joints[i].parent != anim.joints[i].parent) return 0;
    }

    return 1;
}

// Calculate the animated vertex positions and normals based on an animation at a given frame
void AnimateModel(AnimatedModel model, Animation anim, int frame)
{
    if (frame >= anim.frameCount) frame = frame%anim.frameCount;

    for (int m = 0; m < model.meshCount; m++)
    {
        Vector3 outv = {0};
        Vector3 outn = {0};

        Vector3 baset = {0};
        Quaternion baser = {0};
        Vector3 bases = {0};

        Vector3 outt = {0};
        Quaternion outr = {0};
        Vector3 outs = {0};

        int vcounter = 0;
        int wcounter = 0;
        int weightId = 0;

        for (int i = 0; i < model.mesh[m].vertexCount; i++)
        {
            weightId = model.mesh[m].weightId[wcounter];
            baset = model.basepose[weightId].translation;
            baser = model.basepose[weightId].rotation;
            bases = model.basepose[weightId].scale;
            outt = anim.framepose[frame][weightId].translation;
            outr = anim.framepose[frame][weightId].rotation;
            outs = anim.framepose[frame][weightId].scale;

            // vertices
            outv = (Vector3){model.mesh[m].vertices[vcounter],model.mesh[m].vertices[vcounter + 1],model.mesh[m].vertices[vcounter + 2]};
            outv = Vector3MultiplyV(outv,outs);
            outv = Vector3Subtract(outv,baset);
            outv = Vector3RotateByQuaternion(outv,QuaternionMultiply(outr,QuaternionInvert(baser)));
            outv = Vector3Add(outv,outt);
            model.mesh[m].animVertices[vcounter] = outv.x;
            model.mesh[m].animVertices[vcounter + 1] = outv.y;
            model.mesh[m].animVertices[vcounter + 2] = outv.z;

            // normals
            outn = (Vector3){model.mesh[m].normals[vcounter],model.mesh[m].normals[vcounter + 1],model.mesh[m].normals[vcounter + 2]};
            outn = Vector3RotateByQuaternion(outn,QuaternionMultiply(outr,QuaternionInvert(baser)));
            model.mesh[m].animNormals[vcounter] = outn.x;
            model.mesh[m].animNormals[vcounter + 1] = outn.y;
            model.mesh[m].animNormals[vcounter + 2] = outn.z;
            vcounter += 3;
            wcounter += 4;
        }
    }
}

// Draw an animated model
void DrawAnimatedModel(AnimatedModel model,Vector3 position,float scale,Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f,0.0f,0.0f };

    DrawAnimatedModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}

// Draw an animated model with extended parameters
void DrawAnimatedModelEx(AnimatedModel model,Vector3 position,Vector3 rotationAxis,float rotationAngle, Vector3 scale,Color tint)
{
    if (model.materialCount == 0)
    {
        TraceLog(LOG_WARNING,"No materials set, can't draw animated mesh\n");
        return;
    }

    Matrix matScale = MatrixScale(scale.x,scale.y,scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis,rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x,position.y,position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale,matRotation),matTranslation);
    model.transform = MatrixMultiply(model.transform,matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        rlUpdateAnimatedMesh(&model.mesh[i]);
        rlDrawAnimatedMesh(model.mesh[i],model.materials[model.meshMaterialId[i]],MatrixIdentity());
    }
}






// Load animated model meshes from IQM file
static AnimatedModel LoadIQM(const char *filename)
{
    AnimatedModel model = { 0 };

    FILE *iqmFile;
    IQMHeader iqm;

    IQMMesh *imesh;
    IQMTriangle *tri;
    IQMVertexArray *va;
    IQMJoint *ijoint;

    float *vertex;
    float *normal;
    float *text;
    char *blendi;
    unsigned char *blendw;

    iqmFile = fopen(filename, "rb");

    if (!iqmFile)
    {
        TraceLog(LOG_ERROR, "[%s] Unable to open file", filename);
        return model;
    }

    // header
    fread(&iqm,sizeof(IQMHeader), 1, iqmFile);

    if (strncmp(iqm.magic, IQM_MAGIC, sizeof(IQM_MAGIC)))
    {
        TraceLog(LOG_ERROR, "Magic Number \"%s\"does not match.", iqm.magic);
        fclose(iqmFile);
        return model;
    }

    if(iqm.version != IQM_VERSION)
    {
        TraceLog(LOG_ERROR, "IQM version %i is incorrect.", iqm.version);
        fclose(iqmFile);
        return model;
    }

    // meshes
    imesh = malloc(sizeof(IQMMesh)*iqm.num_meshes);
    fseek(iqmFile, iqm.ofs_meshes, SEEK_SET);
    fread(imesh, sizeof(IQMMesh)*iqm.num_meshes, 1, iqmFile);

    model.meshCount = iqm.num_meshes;
    model.mesh = malloc(sizeof(AnimatedMesh)*iqm.num_meshes);

    for (int i = 0; i < iqm.num_meshes; i++)
    {
        fseek(iqmFile,iqm.ofs_text+imesh[i].name,SEEK_SET);
        fread(model.mesh[i].name, sizeof(char)*MESH_NAME_LENGTH, 1, iqmFile);
        model.mesh[i].vertexCount = imesh[i].num_vertexes;
        model.mesh[i].vertices = malloc(sizeof(float)*imesh[i].num_vertexes*3);
        model.mesh[i].normals = malloc(sizeof(float)*imesh[i].num_vertexes*3);
        model.mesh[i].texcoords = malloc(sizeof(float)*imesh[i].num_vertexes*2);
        model.mesh[i].weightId = malloc(sizeof(int)*imesh[i].num_vertexes*4);
        model.mesh[i].weightBias = malloc(sizeof(float)*imesh[i].num_vertexes*4);
        model.mesh[i].triangleCount = imesh[i].num_triangles;
        model.mesh[i].triangles = malloc(sizeof(unsigned short)*imesh[i].num_triangles*3);
        model.mesh[i].animVertices = malloc(sizeof(float)*imesh[i].num_vertexes*3);
        model.mesh[i].animNormals = malloc(sizeof(float)*imesh[i].num_vertexes*3);
    }

    // tris
    tri = malloc(sizeof(IQMTriangle)*iqm.num_triangles);
    fseek(iqmFile, iqm.ofs_triangles, SEEK_SET);
    fread(tri, sizeof(IQMTriangle)*iqm.num_triangles, 1, iqmFile);

    for (int m = 0; m < iqm.num_meshes; m++)
    {
        int tcounter = 0;

        for (int i=imesh[m].first_triangle; i < imesh[m].first_triangle+imesh[m].num_triangles; i++)
        {
            // IQM triangles are stored counter clockwise, but raylib sets opengl to clockwise drawing, so we swap them around
            model.mesh[m].triangles[tcounter+2] = tri[i].vertex[0] - imesh[m].first_vertex;
            model.mesh[m].triangles[tcounter+1] = tri[i].vertex[1] - imesh[m].first_vertex;
            model.mesh[m].triangles[tcounter] = tri[i].vertex[2] - imesh[m].first_vertex;
            tcounter += 3;
        }
    }

    // vertarrays
    va = malloc(sizeof(IQMVertexArray)*iqm.num_vertexarrays);
    fseek(iqmFile, iqm.ofs_vertexarrays, SEEK_SET);
    fread(va, sizeof(IQMVertexArray)*iqm.num_vertexarrays, 1, iqmFile);

    for (int i = 0; i < iqm.num_vertexarrays; i++)
    {
        switch (va[i].type)
        {
            case IQM_POSITION:
            {
                vertex = malloc(sizeof(float)*iqm.num_vertexes*3);
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(vertex, sizeof(float)*iqm.num_vertexes*3, 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vcounter = 0;
                    for (int i = imesh[m].first_vertex*3; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*3; i++)
                    {
                        model.mesh[m].vertices[vcounter] = vertex[i];
                        model.mesh[m].animVertices[vcounter] = vertex[i];
                        vcounter++;
                    }
                }
            } break;
            case IQM_NORMAL:
            {
                normal = malloc(sizeof(float)*iqm.num_vertexes*3);
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(normal, sizeof(float)*iqm.num_vertexes*3, 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vcounter = 0;
                    for (int i = imesh[m].first_vertex*3; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*3; i++)
                    {
                        model.mesh[m].normals[vcounter] = normal[i];
                        model.mesh[m].animNormals[vcounter] = normal[i];
                        vcounter++;
                    }
                }
            } break;
            case IQM_TEXCOORD:
            {
                text = malloc(sizeof(float)*iqm.num_vertexes*2);
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(text, sizeof(float)*iqm.num_vertexes*2, 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vcounter = 0;
                    for (int i = imesh[m].first_vertex*2; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*2; i++)
                    {
                        model.mesh[m].texcoords[vcounter] = text[i];
                        vcounter++;
                    }
                }
            } break;
            case IQM_BLENDINDEXES:
            {
                blendi = malloc(sizeof(char)*iqm.num_vertexes*4);
                fseek(iqmFile, va[i].offset, SEEK_SET);
                fread(blendi, sizeof(char)*iqm.num_vertexes*4, 1, iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vcounter = 0;
                    for (int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
                    {
                        model.mesh[m].weightId[vcounter] = blendi[i];
                        vcounter++;
                    }
                }
            } break;
            case IQM_BLENDWEIGHTS:
            {
                blendw = malloc(sizeof(unsigned char)*iqm.num_vertexes*4);
                fseek(iqmFile,va[i].offset,SEEK_SET);
                fread(blendw,sizeof(unsigned char)*iqm.num_vertexes*4,1,iqmFile);

                for (int m = 0; m < iqm.num_meshes; m++)
                {
                    int vcounter = 0;
                    for (int i = imesh[m].first_vertex*4; i < (imesh[m].first_vertex + imesh[m].num_vertexes)*4; i++)
                    {
                        model.mesh[m].weightBias[vcounter] = blendw[i]/255.0f;
                        vcounter++;
                    }
                }
            } break;
        }
    }

    // joints, include base poses
    ijoint = malloc(sizeof(IQMJoint)*iqm.num_joints);
    fseek(iqmFile, iqm.ofs_joints, SEEK_SET);
    fread(ijoint, sizeof(IQMJoint)*iqm.num_joints, 1, iqmFile);

    model.jointCount = iqm.num_joints;
    model.joints = malloc(sizeof(Joint)*iqm.num_joints);
    model.basepose = malloc(sizeof(Pose)*iqm.num_joints);

    for (int i = 0; i < iqm.num_joints; i++)
    {
        // joints
        model.joints[i].parent = ijoint[i].parent;
        fseek(iqmFile, iqm.ofs_text + ijoint[i].name, SEEK_SET);
        fread(model.joints[i].name,sizeof(char)*JOINT_NAME_LENGTH, 1, iqmFile);

        // basepose
        model.basepose[i].translation.x = ijoint[i].translate[0];
        model.basepose[i].translation.y = ijoint[i].translate[1];
        model.basepose[i].translation.z = ijoint[i].translate[2];

        model.basepose[i].rotation.x = ijoint[i].rotate[0];
        model.basepose[i].rotation.y = ijoint[i].rotate[1];
        model.basepose[i].rotation.z = ijoint[i].rotate[2];
        model.basepose[i].rotation.w = ijoint[i].rotate[3];

        model.basepose[i].scale.x = ijoint[i].scale[0];
        model.basepose[i].scale.y = ijoint[i].scale[1];
        model.basepose[i].scale.z = ijoint[i].scale[2];
    }

    // build base pose
    for (int i = 0; i < model.jointCount; i++)
    {
        if (model.joints[i].parent >= 0)
        {
            model.basepose[i].rotation = QuaternionMultiply(model.basepose[model.joints[i].parent].rotation, model.basepose[i].rotation);
            model.basepose[i].translation = Vector3RotateByQuaternion(model.basepose[i].translation, model.basepose[model.joints[i].parent].rotation);
            model.basepose[i].translation = Vector3Add(model.basepose[i].translation, model.basepose[model.joints[i].parent].translation);
            model.basepose[i].scale = Vector3MultiplyV(model.basepose[i].scale, model.basepose[model.joints[i].parent].scale);
        }
    }

    fclose(iqmFile);
    free(imesh);
    free(tri);
    free(va);
    free(vertex);
    free(normal);
    free(text);
    free(blendi);
    free(blendw);
    free(ijoint);

    return model;
}

#endif
