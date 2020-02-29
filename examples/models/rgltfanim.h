/**********************************************************************************************
*
*   raylib.gltfanimation - Simple module to have animation support with GLTF models, based on the 
*   implementation work for Google's Filament engine.
*
*   CONFIGURATION:
*
*   #define RGLTFANIMATION_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers 
*       or source files without problems. But only ONE file should hold the implementation.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2020 Tyler Bezera and Ramon Santamaria
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

#ifndef RGLTFANIM_H
#define RGLTFANIM_H

#include "raylib.h"
#include "../../src/external/cgltf.h"

typedef enum
{
    TYPE_LINEAR,
    TYPE_STEP,
    TYPE_CUBICSPLINE
} AnimationGLTFInterpolationType;

typedef enum
{
    TYPE_TRANSLATION,
    TYPE_ROTATION,
    TYPE_SCALE,
    TYPE_WEIGHTS
} AnimationGLTFPathType;

typedef struct ModelGLTFAnimationSampler
{
    AnimationGLTFInterpolationType interpolationType;
    float *sourceValues;
    int sourceValuesCount;
} ModelGLTFAnimationSampler;

typedef struct ModelGLTFAnimationChannel
{
    AnimationGLTFPathType pathType;
    ModelGLTFAnimationSampler *sourceData;
    Model targetModel;
} ModelGLTFAnimationChannel;

typedef struct ModelAnimationGLTF
{
    char animationName[50];

    ModelGLTFAnimationSampler *samplers;
    int samplersCount;

    ModelGLTFAnimationChannel *channels;
    int channelsCount;

    float duration;
    float start;
    float end;
} ModelAnimationGLTF;

typedef struct ModelAnimationsGLTF 
{
    ModelAnimationGLTF *animations;
    int animationsCount;

    Matrix *boneMatrices;
    int boneMatricesCount;
} ModelAnimationsGLTF;

#ifdef __cplusplus
extern "C"
{ // Prevents name mangling of functions
#endif

    //----------------------------------------------------------------------------------
    // Module Functions Declaration
    //----------------------------------------------------------------------------------
    ModelAnimationsGLTF LoadModelGLTFAnimations(const char *filename);
    void UpdateModelAnimationGLTF(Model model, int animationIndex, float time);
    void createSampler(cgltf_animation_sampler *src, ModelGLTFAnimationSampler *dst);

#ifdef __cplusplus
}
#endif

#endif //RGLTFANIM_H

/***********************************************************************************
*
*   RGLTFANIM IMPLEMENTATION
*
************************************************************************************/

#if defined(RGLTFANIMATION_IMPLEMENTATION)
#include "raylib.h"
#include <stdio.h>
//#define CGLTF_IMPLEMENTATION
#include "../../src/external/cgltf.h"

void createSampler(cgltf_animation_sampler *src, ModelGLTFAnimationSampler *dst) {
    //uint8_t *tileLineBlob = (uint8_t*)src->input->buffer_view->buffer->data;
    //float *tileLineFloats = (float*)(tileLineBlob + src->input->offset + src->input->buffer_view->offset);

    for (int i = 0, len = src->input->count; i < len; ++i) {
        
        //TODO: Need to support map
        //dst->times

        switch (src->output->type) {
            case cgltf_type_scalar:
                dst->sourceValues = RL_CALLOC(src->output->count, sizeof(float));
                cgltf_accessor_unpack_floats(src->output, &dst->sourceValues[0], src->output->count);
                break;

            case cgltf_type_vec3:
                dst->sourceValues = RL_CALLOC(src->output->count * 3, sizeof(float));
                cgltf_accessor_unpack_floats(src->output, &dst->sourceValues[0], src->output->count * 3);
                break;

            case cgltf_type_vec4:
                dst->sourceValues = RL_CALLOC(src->output->count * 3, sizeof(float));
                cgltf_accessor_unpack_floats(src->output, &dst->sourceValues[0], src->output->count * 3);
                break;
            default:
                break;
        }

        switch (src->interpolation) {
            case cgltf_interpolation_type_linear:
                dst->interpolationType = TYPE_LINEAR;
                break;
            
            case cgltf_interpolation_type_step:
                dst->interpolationType = TYPE_STEP;
                break;
            
            case cgltf_interpolation_type_cubic_spline:
                dst->interpolationType = TYPE_CUBICSPLINE;
                break;
        }
    }
}

void setTransformType(cgltf_animation_channel *src, ModelGLTFAnimationChannel *dst) {
    switch (src->target_path) {
        case cgltf_animation_path_type_translation:
            dst->pathType = TYPE_TRANSLATION;
            break;
        
        case cgltf_animation_path_type_rotation:
            dst->pathType = TYPE_ROTATION;
            break;
        
        case cgltf_animation_path_type_scale:
            dst->pathType = TYPE_SCALE;
            break;

        case cgltf_animation_path_type_weights:
            dst->pathType = TYPE_WEIGHTS;
            break;
    }
}

ModelAnimationsGLTF LoadModelGLTFAnimations(const char *fileName) {
    
    ModelAnimationsGLTF animationsGLTF = { 0 };
    // glTF file loading
    FILE *gltfFile = fopen(fileName, "rb");

    if (gltfFile == NULL)
    {
        TraceLog(LOG_WARNING, "[%s] glTF file could not be opened", fileName);
        return animationsGLTF;
    }

    fseek(gltfFile, 0, SEEK_END);
    int size = ftell(gltfFile);
    fseek(gltfFile, 0, SEEK_SET);

    void *buffer = RL_MALLOC(size);
    fread(buffer, size, 1, gltfFile);

    fclose(gltfFile);

    // glTF data loading
    cgltf_options options = { 0 };
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse(&options, buffer, size, &data);

     if (result == cgltf_result_success)
    {
        // Read data buffers
        result = cgltf_load_buffers(&options, data, fileName);
        if (result != cgltf_result_success) TraceLog(LOG_INFO, "[%s][%s] Error loading mesh/material buffers", fileName, (data->file_type == 2)? "glb" : "gltf");

        //Animation count
        int animationsCount = data->animations_count;

        //Initialize our animations array
        animationsGLTF.animationsCount = animationsCount;
        animationsGLTF.animations = RL_CALLOC(animationsCount, sizeof(ModelAnimationGLTF));

        //Loop through each Animation
        for (int i = 0; i < animationsCount; i++) {
            //Copy Animation name
            if(data->animations[i].name) TextCopy(animationsGLTF.animations[i].animationName, data->animations[i].name);

            int samplerCount = data->animations[i].samplers_count;
            animationsGLTF.animations[i].samplersCount = samplerCount;
            animationsGLTF.animations[i].samplers = RL_CALLOC(samplerCount, sizeof(ModelGLTFAnimationSampler));
            for (int j = 0; j < samplerCount; j++) {
                createSampler(&data->animations[i].samplers[j], &animationsGLTF.animations[i].samplers[j]);
                //TODO handle map times
            }

            int channelCount = data->animations[i].channels_count;
            animationsGLTF.animations[i].channelsCount = channelCount;
            animationsGLTF.animations[i].channels = RL_CALLOC(channelCount, sizeof(ModelGLTFAnimationChannel));
            for (int j = 0; j < channelCount; j++) {
                //TODO Model stuff? Also... strange pointer arthmetic? data->animations[i].channels[j].sampler - data->animations[i].samplers??
                animationsGLTF.animations[i].channels[j].sourceData = &animationsGLTF.animations[i].samplers[data->animations[i].channels[j].sampler - data->animations[i].samplers];
                setTransformType(&data->animations[i].channels[j], &animationsGLTF.animations[i].channels[j]);
            }
        }
    }

    return animationsGLTF;
}

void UpdateModelAnimationGLTF(Model model, int animationIndex, float time) {
    
}

#endif