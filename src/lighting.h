/*******************************************************************************************
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

#ifndef LIGHTING_H
#define LIGHTING_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for LIGHTING_STANDALONE usage
//----------------------------------------------------------------------------------
// Vector3 type
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

// Light type
typedef struct Light {
    float position[3];
    float rotation[3];
    float intensity[1];
    float ambientColor[3];
    float diffuseColor[3];
    float specularColor[3];
    float specularIntensity[1];
} Light;

// Material type
typedef struct Material {
    float ambientColor[3];
    float diffuseColor[3];
    float specularColor[3];
    float glossiness[1];
    float normalDepth[1];
} Material;

//----------------------------------------------------------------------------------
// Module Functions Declaration 
// NOTE: light and material structs uses float pointers instead of vectors to be compatible with SetShaderValue()
//----------------------------------------------------------------------------------
// Lights functions
void SetLightPosition(Light *light, Vector3 position);                  // Set light position converting position vector to float pointer
void SetLightRotation(Light *light, Vector3 rotation);                  // Set light rotation converting rotation vector to float pointer
void SetLightIntensity(Light *light, float intensity);                  // Set light intensity value 
void SetLightAmbientColor(Light *light, Vector3 color);                 // Set light ambient color value (it will be multiplied by material ambient color)
void SetLightDiffuseColor(Light *light, Vector3 color);                 // Set light diffuse color (light color)
void SetLightSpecularColor(Light *light, Vector3 color);                // Set light specular color (it will be multiplied by material specular color)
void SetLightSpecIntensity(Light *light, float specIntensity);          // Set light specular intensity (specular color scalar multiplier)

// Materials functions
void SetMaterialAmbientColor(Material *material, Vector3 color);        // Set material ambient color value (it will be multiplied by light ambient color)
void SetMaterialDiffuseColor(Material *material, Vector3 color);        // Set material diffuse color (material color, should use DrawModel() tint parameter)
void SetMaterialSpecularColor(Material *material, Vector3 color);       // Set material specular color (it will be multiplied by light specular color)
void SetMaterialGlossiness(Material *material, float glossiness);       // Set material glossiness value (recommended values: 0 - 100)
void SetMaterialNormalDepth(Material *material, float depth);           // Set normal map depth (B component from RGB type map scalar multiplier)

#ifdef __cplusplus
}
#endif

#endif // LIGHTING_H