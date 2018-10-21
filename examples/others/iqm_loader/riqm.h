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
    int jointCount;         // Number of joints (bones)
    Joint *joints;          // Joints array
                            // NOTE: Joints in anims do not have names

    int frameCount;         // Number of animation frames
    float framerate;        // Frame change speed

    Pose **framepose;       // Poses array by frame (and one pose by joint)
} Animation;

// Animated Model type
typedef struct AnimatedModel {
    Matrix transform;       // Local transform matrix
    
    int meshCount;          // Number of meshes
    Mesh *meshes;           // Meshes array

    int materialCount;      // Number of materials
    Material *materials;    // Materials array
    
    int *meshMaterialId;    // Mesh materials ids

    // Animation required data
    int jointCount;         // Number of joints (and keyposes)
    Joint *joints;          // Mesh joints (bones)
    Pose *basepose;         // Mesh base-poses by joint
} AnimatedModel;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Loading/Unloading functions
RIQMDEF AnimatedModel LoadAnimatedModel(const char *filename);
RIQMDEF void UnloadAnimatedModel(AnimatedModel model);
RIQMDEF Animation LoadAnimation(const char *filename);
RIQMDEF void UnloadAnimation(Animation anim);

RIQMDEF AnimatedModel AnimatedModelAddTexture(AnimatedModel model, const char *filename);    // GENERIC!
RIQMDEF AnimatedModel SetMeshMaterial(AnimatedModel model, int meshid, int textureid);       // GENERIC!

// Usage functionality
RIQMDEF bool CheckSkeletonsMatch(AnimatedModel model, Animation anim);
RIQMDEF void AnimateModel(AnimatedModel model, Animation anim, int frame);
RIQMDEF void DrawAnimatedModel(AnimatedModel model, Vector3 position, float scale, Color tint);
RIQMDEF void DrawAnimatedModelEx(AnimatedModel model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);

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
static AnimatedModel LoadIQM(const char *filename);

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

// Load .iqm file and initialize animated model
AnimatedModel LoadAnimatedModel(const char *filename)
{
    AnimatedModel out = LoadIQM(filename);

    for (int i = 0; i < out.meshCount; i++) rlLoadMesh(&out.meshes[i], false);

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

// Set the material for a meshes
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

    for (int i = 0; i < model.meshCount; i++) rlUnloadMesh(&model.meshes[i]);

    free(model.meshes);
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

        for (int i = 0; i < model.meshes[m].vertexCount; i++)
        {
            weightId = model.meshes[m].weightId[wcounter];
            baset = model.basepose[weightId].translation;
            baser = model.basepose[weightId].rotation;
            bases = model.basepose[weightId].scale;
            outt = anim.framepose[frame][weightId].translation;
            outr = anim.framepose[frame][weightId].rotation;
            outs = anim.framepose[frame][weightId].scale;

            // vertices
            // NOTE: We use meshes.baseVertices (default position) to calculate meshes.vertices (animated position)
            outv = (Vector3){ model.meshes[m].baseVertices[vcounter], model.meshes[m].baseVertices[vcounter + 1], model.meshes[m].baseVertices[vcounter + 2] };
            outv = Vector3MultiplyV(outv, outs);
            outv = Vector3Subtract(outv, baset);
            outv = Vector3RotateByQuaternion(outv, QuaternionMultiply(outr, QuaternionInvert(baser)));
            outv = Vector3Add(outv, outt);
            model.meshes[m].vertices[vcounter] = outv.x;
            model.meshes[m].vertices[vcounter + 1] = outv.y;
            model.meshes[m].vertices[vcounter + 2] = outv.z;

            // normals
            // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
            outn = (Vector3){ model.meshes[m].baseNormals[vcounter], model.meshes[m].baseNormals[vcounter + 1], model.meshes[m].baseNormals[vcounter + 2] };
            outn = Vector3RotateByQuaternion(outn, QuaternionMultiply(outr, QuaternionInvert(baser)));
            model.meshes[m].normals[vcounter] = outn.x;
            model.meshes[m].normals[vcounter + 1] = outn.y;
            model.meshes[m].normals[vcounter + 2] = outn.z;
            vcounter += 3;
            wcounter += 4;
        }
    }
}

// Draw an animated model
void DrawAnimatedModel(AnimatedModel model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 1.0f, 0.0f,0.0f };

    DrawAnimatedModelEx(model, position, rotationAxis, -90.0f, vScale, tint);
}

// Draw an animated model with extended parameters
void DrawAnimatedModelEx(AnimatedModel model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    if (model.materialCount == 0)
    {
        TraceLog(LOG_WARNING,"No materials set, can't draw animated meshes\n");
        return;
    }

    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        rlUpdateMesh(model.meshes[i], 0, model.meshes[i].vertexCount);      // Update vertex position
        rlUpdateMesh(model.meshes[i], 2, model.meshes[i].vertexCount);      // Update vertex normals
        rlDrawMesh(model.meshes[i], model.materials[model.meshMaterialId[i]], model.transform);   // Draw meshes
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
    model.meshes = malloc(sizeof(Mesh)*iqm.num_meshes);
    
    char name[MESH_NAME_LENGTH];

    for (int i = 0; i < iqm.num_meshes; i++)
    {
        fseek(iqmFile,iqm.ofs_text+imesh[i].name,SEEK_SET);
        fread(name, sizeof(char)*MESH_NAME_LENGTH, 1, iqmFile);         // Mesh name not used...
        model.meshes[i].vertexCount = imesh[i].num_vertexes;
        
        model.meshes[i].baseVertices = malloc(sizeof(float)*imesh[i].num_vertexes*3);     // Default IQM base position
        model.meshes[i].baseNormals = malloc(sizeof(float)*imesh[i].num_vertexes*3);      // Default IQM base normal
        
        model.meshes[i].texcoords = malloc(sizeof(float)*imesh[i].num_vertexes*2);
        model.meshes[i].weightId = malloc(sizeof(int)*imesh[i].num_vertexes*4);
        model.meshes[i].weightBias = malloc(sizeof(float)*imesh[i].num_vertexes*4);
        
        model.meshes[i].triangleCount = imesh[i].num_triangles;
        model.meshes[i].indices = malloc(sizeof(unsigned short)*imesh[i].num_triangles*3);
        
        // What we actually process for rendering, should be updated transforming meshes.vertices and meshes.normals
        model.meshes[i].vertices = malloc(sizeof(float)*imesh[i].num_vertexes*3);     
        model.meshes[i].normals = malloc(sizeof(float)*imesh[i].num_vertexes*3);
    }

    // tris
    tri = malloc(sizeof(IQMTriangle)*iqm.num_triangles);
    fseek(iqmFile, iqm.ofs_triangles, SEEK_SET);
    fread(tri, sizeof(IQMTriangle)*iqm.num_triangles, 1, iqmFile);

    for (int m = 0; m < iqm.num_meshes; m++)
    {
        int tcounter = 0;

        for (int i = imesh[m].first_triangle; i < imesh[m].first_triangle+imesh[m].num_triangles; i++)
        {
            // IQM triangles are stored counter clockwise, but raylib sets opengl to clockwise drawing, so we swap them around
            model.meshes[m].indices[tcounter+2] = tri[i].vertex[0] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter+1] = tri[i].vertex[1] - imesh[m].first_vertex;
            model.meshes[m].indices[tcounter] = tri[i].vertex[2] - imesh[m].first_vertex;
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
                        model.meshes[m].vertices[vcounter] = vertex[i];
                        model.meshes[m].baseVertices[vcounter] = vertex[i];
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
                        model.meshes[m].normals[vcounter] = normal[i];
                        model.meshes[m].baseNormals[vcounter] = normal[i];
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
                        model.meshes[m].texcoords[vcounter] = text[i];
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
                        model.meshes[m].weightId[vcounter] = blendi[i];
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
                        model.meshes[m].weightBias[vcounter] = blendw[i]/255.0f;
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
