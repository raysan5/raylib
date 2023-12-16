/**********************************************************************************************
*
*   raylib.pbr - Some useful functions to deal with pbr materials and lights
*
*   CONFIGURATION:
*
*   #define RPBR_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2023-2024 Afan OLOVCIC (@_DevDad) 2017-2020 Victor Fisac(@victorfisac),Ramon Santamaria (@raysan5)
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

#ifndef RPBR_H
#define RPBR_H
#include "raylib.h"


//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_LIGHTS  4        // Max dynamic lights supported by shader
#define SHADER_LOC_MAP_MRA SHADER_LOC_MAP_METALNESS  //METALLIC, ROUGHNESS and AO
#define SHADER_LOC_MAP_EMISSIVE  SHADER_LOC_MAP_HEIGHT     //EMISSIVE
#define MATERIAL_MAP_MRA MATERIAL_MAP_METALNESS
#define MATERIAL_MAP_EMISSIVE  MATERIAL_MAP_HEIGHT
#define NULL 0
#define COLOR_TO_ARRAY(c)

typedef struct {
    int enabled;
    int type;
    Vector3 position;
    Vector3 target;
    float color[4];
    float intensity;

    int enabledLoc;
    int typeLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int intensityLoc;
} PBRLight;

typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT,
    LIGHT_SPOT
} PBRLightType;

typedef struct{
    Shader pbrShader;
    Shader skyShader;
    unsigned int cubemap;
    unsigned int irradiance;
    unsigned int prefilter;
    unsigned int brdf;
    int modelMatrixLoc;
    int pbrViewLoc;
    int skyViewLoc;
    int skyResolutionLoc;
} PBREnvironment;

typedef enum{
    PBR_COLOR_ALBEDO = 0,
    PBR_COLOR_EMISSIVE
}PBRColorType;

typedef enum{
    PBR_VEC2_TILING = 0,
    PBR_VEC2_OFFSET
}PBRVec2Type;

typedef enum{
    PBR_PARAM_NORMAL =0,
    PBR_PARAM_METALLIC,
    PBR_PARAM_ROUGHNESS,
    PBR_PARAM_EMISSIVE,
    PBR_PARAM_AO
}PBRFloatType;

typedef enum{
    PBR_TEXTURE_ALBEDO = 0,
    PBR_TEXTURE_NORMAL,
    PBR_TEXTURE_MRA,
    PBR_TEXTURE_EMISSIVE
}PBRTexType;

// Textures are moved to material from params to pack better and use less textures on the end
// texture MRAE 4Channel R: Metallic G: Roughness B: A: Ambient Occlusion
// texEmissive use just one channel, so we have 3 channels still to use if we need
typedef struct {
    Shader pbrShader;
    float albedo[4];
    float normal;
    float metallic;
    float roughness;
    float ao;
    float emissive[4];
    float ambient[3];
    float emissivePower;

    Texture2D texAlbedo;
    Texture2D texNormal;
    Texture2D texMRA;//r: Metallic  g: Roughness b: AO a:Empty
    Texture2D texEmissive; //Emissive Texture
    // Using float4 to store tilling at 1st and 2nd position and offset at 3rd and 4th
    float texTiling[2];
    float texOffset[2];

    int useTexAlbedo;
    int useTexNormal;
    int useTexMRA;
    int useTexEmissive;

    int albedoLoc;
    int normalLoc;
    int metallicLoc;
    int roughnessLoc;
    int aoLoc;
    int emissiveColorLoc;
    int emissivePowerLoc;

    int texTilingLoc;
    int texOffsetLoc;

    int useTexAlbedoLoc;
    int useTexNormalLoc;
    int useTexMRAELoc;
    int useTexEmissiveLoc;
} PBRMaterial;

typedef struct{
    Model model;
    PBRMaterial pbrMat;
}PBRModel;

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// Create a light and get shader locations
PBRLight PBRLightCreate(int type, Vector3 position, Vector3 target, Color color,float intensity, Shader shader);
// Send light properties to shader
void PBRLightUpdate(Shader shader, PBRLight light);

//For now until we do real skylight
void PBRSetAmbient(Shader shader, Color color, float intensity);

PBRModel PBRModelLoad(const char *fileName);
PBRModel PBRModelLoadFromMesh(Mesh mesh);

void PBRLoadTextures(PBRMaterial *pbrMat,PBRTexType pbrTexType,const char *fileName);
void UnloadPBRMaterial(PBRMaterial pbrMat);
void PBRSetColor(PBRMaterial *pbrMat,PBRColorType pbrColorType,Color color);
void PBRSetVec2(PBRMaterial *pbrMat,PBRVec2Type type,Vector2 value);
void PBRSetFloat(PBRMaterial *pbrMat, PBRFloatType pbrParamType, float value);

void PBRMaterialSetup( PBRMaterial *pbrMat,Shader pbrShader, PBREnvironment* environment);
void PBRSetMaterial(PBRModel* model,PBRMaterial* pbrMat,int matIndex);
void PBRDrawModel(PBRModel pbrModel, Vector3 position, float scale);

#ifdef __cplusplus
}
#endif

#endif //RPBR_H

/***********************************************************************************
*
*   RPBR IMPLEMENTATION
*
************************************************************************************/

#if defined(RPBR_IMPLEMENTATION)

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int lightsCount = 0;    // Current amount of created lights

// Create a light and get shader locations
PBRLight PBRLightCreate(int type, Vector3 position, Vector3 target, Color color,float intensity, Shader shader)
{
    PBRLight light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = 1;
        light.type = type;
        light.position = position;
        light.target = target;
        light.color[0] = (float)color.r/(float)255;
        light.color[1] = (float)color.g/(float)255;
        light.color[2] = (float)color.b/(float)255;
        light.color[3] = (float)color.a/(float)255;
        light.intensity = intensity;
        // NOTE: Lighting shader naming must be the provided ones
        light.enabledLoc =  GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
        light.typeLoc =     GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
        light.targetLoc =   GetShaderLocation(shader, TextFormat("lights[%i].target", lightsCount));
        light.colorLoc =    GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));
        light.intensityLoc =    GetShaderLocation(shader, TextFormat("lights[%i].intensity", lightsCount));
        PBRLightUpdate(shader, light);

        lightsCount++;
    }

    return light;
}

// Send light properties to shader
// NOTE: Light shader locations should be available
void PBRLightUpdate(Shader shader, PBRLight light)
{
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);
    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, light.colorLoc, light.color, SHADER_UNIFORM_VEC4);
    SetShaderValue(shader, light.intensityLoc, &light.intensity, SHADER_UNIFORM_FLOAT);
}

void PBRSetAmbient(Shader shader, Color color, float intensity){
    float col[3] = {color.r/255,color.g/255,color.b/255};
    SetShaderValue(shader, GetShaderLocation(shader, "ambientColor"), col, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, GetShaderLocation(shader, "ambient"), &intensity, SHADER_UNIFORM_FLOAT);
}

void PBRMaterialSetup(PBRMaterial *pbrMat, Shader pbrShader, PBREnvironment* environment){
    pbrMat->pbrShader = pbrShader;

    pbrMat->texAlbedo = (Texture2D){0};
    pbrMat->texNormal = (Texture2D){0};
    pbrMat->texMRA = (Texture2D){0};
    pbrMat->texEmissive = (Texture2D){0};

    //PBRParam
    pbrMat->albedo[0] = 1.0;
    pbrMat->albedo[1] = 1.0;
    pbrMat->albedo[2] = 1.0;
    pbrMat->albedo[3] = 1.0;
    pbrMat->metallic = 0;
    pbrMat->roughness = 0;
    pbrMat->ao = 1.0;
    pbrMat->normal = 1;
    pbrMat->emissive[0] = 0;
    pbrMat->emissive[1] = 0;
    pbrMat->emissive[2] = 0;
    pbrMat->emissive[3] = 0;

    pbrMat->texTiling[0] = 1.0;
    pbrMat->texTiling[1] = 1.0;
    pbrMat->texOffset[0] = 0.0;
    pbrMat->texOffset[1] = 0.0;
    pbrMat->emissivePower = 1.0;
    // Set up PBR shader material locations

    pbrMat->albedoLoc = GetShaderLocation(pbrMat->pbrShader, "albedoColor");
    pbrMat->normalLoc = GetShaderLocation(pbrMat->pbrShader, "normalValue");
    pbrMat->metallicLoc = GetShaderLocation(pbrMat->pbrShader, "metallicValue");
    pbrMat->roughnessLoc = GetShaderLocation(pbrMat->pbrShader, "roughnessValue");
    pbrMat->aoLoc = GetShaderLocation(pbrMat->pbrShader, "aoValue");
    pbrMat->emissiveColorLoc = GetShaderLocation(pbrMat->pbrShader, "emissiveColor");
    pbrMat->emissivePowerLoc = GetShaderLocation(pbrMat->pbrShader, "emissivePower");

    pbrMat->texTilingLoc = GetShaderLocation(pbrMat->pbrShader, "tiling");
    pbrMat->texOffsetLoc = GetShaderLocation(pbrMat->pbrShader, "offset");

    pbrMat->useTexAlbedoLoc = GetShaderLocation(pbrMat->pbrShader, "useTexAlbedo");
    pbrMat->useTexNormalLoc = GetShaderLocation(pbrMat->pbrShader, "useTexNormal");
    pbrMat->useTexMRAELoc = GetShaderLocation(pbrMat->pbrShader, "useTexMRA");
    pbrMat->useTexEmissiveLoc = GetShaderLocation(pbrMat->pbrShader, "useTexEmissive");

    SetShaderValue(pbrMat->pbrShader,pbrMat->albedoLoc,pbrMat->albedo,SHADER_UNIFORM_VEC4);
    SetShaderValue(pbrMat->pbrShader, pbrMat->emissiveColorLoc, pbrMat->emissive, SHADER_UNIFORM_VEC4);
    SetShaderValue(pbrMat->pbrShader, pbrMat->emissivePowerLoc, &pbrMat->emissivePower, SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->metallicLoc,&pbrMat->metallic,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->roughnessLoc,&pbrMat->roughness,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->aoLoc,&pbrMat->ao,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->normalLoc,&pbrMat->normal,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->texTilingLoc,pbrMat->texTiling,SHADER_UNIFORM_VEC2);
    SetShaderValue(pbrMat->pbrShader,pbrMat->texOffsetLoc,pbrMat->texOffset,SHADER_UNIFORM_VEC2);
}

void PBRLoadTextures(PBRMaterial *pbrMat,PBRTexType pbrTexType,const char *fileName){
    if(pbrMat == NULL) return;
    switch(pbrTexType){
        case PBR_TEXTURE_ALBEDO:
            pbrMat->texAlbedo = LoadTexture(fileName);
            pbrMat->useTexAlbedo = 1;
            break;
        case PBR_TEXTURE_MRA:
            pbrMat->texMRA = LoadTexture(fileName);
            pbrMat->useTexMRA = 1;
            break;
        case PBR_TEXTURE_NORMAL:
            pbrMat->texNormal = LoadTexture(fileName);
            pbrMat->useTexNormal = 1;
            break;
        case PBR_TEXTURE_EMISSIVE:
            pbrMat->texEmissive = LoadTexture(fileName);
            pbrMat->useTexEmissive = 1;
            break;
    }
}

void UnloadPBRMaterial(PBRMaterial pbrMat){
    if(pbrMat.useTexAlbedo == 1) UnloadTexture(pbrMat.texAlbedo);
    if(pbrMat.useTexNormal == 1) UnloadTexture(pbrMat.texNormal);
    if(pbrMat.useTexMRA == 1) UnloadTexture(pbrMat.texMRA);
    if(pbrMat.useTexEmissive == 1) UnloadTexture(pbrMat.texEmissive);
}

void PBRSetColor(PBRMaterial *pbrMat,PBRColorType pbrColorType,Color color){
    if(pbrMat == NULL) return;
    switch(pbrColorType){
        case PBR_COLOR_ALBEDO:
            pbrMat->albedo[0] = (float) color.r / 255;
            pbrMat->albedo[1] = (float) color.g / 255;
            pbrMat->albedo[2] = (float) color.b / 255;
            pbrMat->albedo[3] = (float) color.a / 255;
            SetShaderValue(pbrMat->pbrShader,pbrMat->albedoLoc,pbrMat->albedo,SHADER_UNIFORM_VEC4);
            break;
        case PBR_COLOR_EMISSIVE:
            pbrMat->emissive[0] = (float) color.r / 255;
            pbrMat->emissive[1] = (float) color.g / 255;
            pbrMat->emissive[2] = (float) color.b / 255;
            pbrMat->emissive[3] = (float) color.a / 255;
            SetShaderValue(pbrMat->pbrShader, pbrMat->emissiveColorLoc, pbrMat->emissive, SHADER_UNIFORM_VEC4);
            break;
    }
}

void PBRSetFloat(PBRMaterial *pbrMat, PBRFloatType pbrParamType, float value){
    if(pbrMat == NULL) return;
    switch(pbrParamType){
        case PBR_PARAM_METALLIC:
            pbrMat->metallic = value;
            SetShaderValue(pbrMat->pbrShader,pbrMat->metallicLoc,&pbrMat->metallic,SHADER_UNIFORM_FLOAT);
            break;
        case PBR_PARAM_ROUGHNESS:
            pbrMat->roughness = value;
            SetShaderValue(pbrMat->pbrShader,pbrMat->roughnessLoc,&pbrMat->roughness,SHADER_UNIFORM_FLOAT);
            break;
        case PBR_PARAM_NORMAL:
            pbrMat->normal = value;
            SetShaderValue(pbrMat->pbrShader,pbrMat->normalLoc,&pbrMat->normal,SHADER_UNIFORM_FLOAT);
            break;
        case PBR_PARAM_AO:
            pbrMat->ao = value;
            SetShaderValue(pbrMat->pbrShader,pbrMat->aoLoc,&pbrMat->ao,SHADER_UNIFORM_FLOAT);
            break;
        case PBR_PARAM_EMISSIVE:
            pbrMat->emissivePower = value;
            SetShaderValue(pbrMat->pbrShader,pbrMat->emissivePowerLoc,&pbrMat->emissivePower,SHADER_UNIFORM_FLOAT);
            break;
    }
}


void PBRSetVec2(PBRMaterial *pbrMat,PBRVec2Type type,Vector2 value){
    switch(type){
        case PBR_VEC2_TILING:
            pbrMat->texTiling[0] = value.x;
            pbrMat->texTiling[1] = value.y;
            SetShaderValue(pbrMat->pbrShader,pbrMat->texTilingLoc,&pbrMat->texTiling,SHADER_UNIFORM_VEC2);
            break;
        case PBR_VEC2_OFFSET:
            pbrMat->texOffset[0] = value.x;
            pbrMat->texOffset[1] = value.y;
            SetShaderValue(pbrMat->pbrShader,pbrMat->texOffsetLoc,&pbrMat->texOffset,SHADER_UNIFORM_VEC2);
            break;
    }
}

void PBRSetMaterial(PBRModel* model,PBRMaterial* pbrMat,int matIndex){


    model->pbrMat = *pbrMat;
    model->model.materials[matIndex].shader = model->pbrMat.pbrShader;
    pbrMat->pbrShader.locs[SHADER_LOC_MAP_MRA] = GetShaderLocation(pbrMat->pbrShader, "mraMap");
    pbrMat->pbrShader.locs[SHADER_LOC_MAP_EMISSIVE] = GetShaderLocation(pbrMat->pbrShader, "emissiveMap");
    pbrMat->pbrShader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(pbrMat->pbrShader, "normalMap");

    if(pbrMat->useTexAlbedo) {
        model->model.materials[matIndex].maps[MATERIAL_MAP_ALBEDO].texture = pbrMat->texAlbedo;
    }
    if(pbrMat->useTexMRA) {
        model->model.materials[matIndex].maps[MATERIAL_MAP_MRA].texture = pbrMat->texMRA;
    }
    if(pbrMat->useTexNormal) {
        model->model.materials[matIndex].maps[MATERIAL_MAP_NORMAL].texture = pbrMat->texNormal;
    }
    if(pbrMat->useTexEmissive) {
        model->model.materials[matIndex].maps[MATERIAL_MAP_EMISSIVE].texture = pbrMat->texEmissive;
    }

    SetShaderValue(pbrMat->pbrShader,pbrMat->useTexAlbedoLoc,&pbrMat->useTexAlbedo,SHADER_UNIFORM_INT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->useTexNormalLoc,&pbrMat->useTexNormal,SHADER_UNIFORM_INT);
    SetShaderValue(pbrMat->pbrShader, pbrMat->useTexMRAELoc, &pbrMat->useTexMRA, SHADER_UNIFORM_INT);
    SetShaderValue(pbrMat->pbrShader, pbrMat->useTexEmissiveLoc, &pbrMat->useTexEmissive, SHADER_UNIFORM_INT);
}

void PBRDrawModel(PBRModel pbrModel, Vector3 position, float scale){
    PBRMaterial *pbrMat = &pbrModel.pbrMat;
    SetShaderValue(pbrMat->pbrShader,pbrMat->albedoLoc,pbrMat->albedo,SHADER_UNIFORM_VEC4);
    SetShaderValue(pbrMat->pbrShader, pbrMat->emissiveColorLoc, pbrMat->emissive, SHADER_UNIFORM_VEC4);
    SetShaderValue(pbrMat->pbrShader,pbrMat->metallicLoc,&pbrMat->metallic,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->roughnessLoc,&pbrMat->roughness,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->aoLoc,&pbrMat->ao,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->normalLoc,&pbrMat->normal,SHADER_UNIFORM_FLOAT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->texTilingLoc,pbrMat->texTiling,SHADER_UNIFORM_VEC2);
    SetShaderValue(pbrMat->pbrShader,pbrMat->texOffsetLoc,pbrMat->texOffset,SHADER_UNIFORM_VEC2);

    SetShaderValue(pbrMat->pbrShader,pbrMat->useTexAlbedoLoc,&pbrMat->useTexAlbedo,SHADER_UNIFORM_INT);
    SetShaderValue(pbrMat->pbrShader,pbrMat->useTexNormalLoc,&pbrMat->useTexNormal,SHADER_UNIFORM_INT);
    SetShaderValue(pbrMat->pbrShader, pbrMat->useTexMRAELoc, &pbrMat->useTexMRA, SHADER_UNIFORM_INT);
    SetShaderValue(pbrMat->pbrShader, pbrMat->useTexEmissiveLoc, &pbrMat->useTexEmissive, SHADER_UNIFORM_INT);

    DrawModel(pbrModel.model,position,scale,WHITE);
}

PBRModel PBRModelLoad(const char *fileName){
    PBRModel pbrModel = (PBRModel){0};
    pbrModel.model = LoadModel(fileName);
    return pbrModel;
}

PBRModel PBRModelLoadFromMesh(Mesh mesh){
    PBRModel pbrModel = (PBRModel){0};
    pbrModel.model = LoadModelFromMesh(mesh);
    return pbrModel;
}
#endif // RPBR_IMPLEMENTATION
