/**********************************************************************************************
*
*   raylib lighting engine module - Lighting and materials management functions
*
*   Copyright (c) 2015 Victor Fisac and Ramon Santamaria
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

//#define LIGHTING_STANDALONE     // NOTE: To use the lighting module as standalone lib, just uncomment this line

#if defined(LIGHTING_STANDALONE)
    #include "lighting.h"
#else
    #include "raylib.h"
#endif  
             
#include <string.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definitions
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Lights functions
void SetLightPosition(Light *light, Vector3 position)
{
    light->position[0] = position.x;
    light->position[1] = position.y;
    light->position[2] = position.z;
}

void SetLightRotation(Light *light, Vector3 rotation)
{
    light->rotation[0] = rotation.x;
    light->rotation[1] = rotation.y;
    light->rotation[2] = rotation.z;
}

void SetLightIntensity(Light *light, float intensity)
{
    light->intensity[0] = intensity;
}

void SetLightAmbientColor(Light *light, Vector3 color)
{
    light->ambientColor[0] = color.x;
    light->ambientColor[1] = color.y;
    light->ambientColor[2] = color.z;
}

void SetLightDiffuseColor(Light *light, Vector3 color)
{
    light->diffuseColor[0] = color.x;
    light->diffuseColor[1] = color.y;
    light->diffuseColor[2] = color.z;
}

void SetLightSpecularColor(Light *light, Vector3 color)
{
    light->specularColor[0] = color.x;
    light->specularColor[1] = color.y;
    light->specularColor[2] = color.z;
}

void SetLightSpecIntensity(Light *light, float specIntensity)
{
    light->specularIntensity[0] = specIntensity;
}

// Materials functions
void SetMaterialAmbientColor(Material *material, Vector3 color)
{
    material->ambientColor[0] = color.x;
    material->ambientColor[1] = color.y;
    material->ambientColor[2] = color.z;
}

void SetMaterialDiffuseColor(Material *material, Vector3 color)
{
    material->diffuseColor[0] = color.x;
    material->diffuseColor[1] = color.y;
    material->diffuseColor[2] = color.z;    
}

void SetMaterialSpecularColor(Material *material, Vector3 color)
{
    material->specularColor[0] = color.x;
    material->specularColor[1] = color.y;
    material->specularColor[2] = color.z;
}

void SetMaterialGlossiness(Material *material, float glossiness)
{
    material->glossiness[0] = glossiness;
}

void SetMaterialNormalDepth(Material *material, float depth)
{
    material->normalDepth[0] = depth;
}