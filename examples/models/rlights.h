/**********************************************************************************************
*
*   raylib.lights - Some useful functions to deal with lights data
*
*   CONFIGURATION:
*
*   #define RLIGHTS_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers 
*       or source files without problems. But only ONE file should hold the implementation.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2017 Victor Fisac and Ramon Santamaria
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

#ifndef RLIGHTS_H
#define RLIGHTS_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define         MAX_LIGHTS            4         // Max lights supported by shader
#define         LIGHT_DISTANCE        3.5f      // Light distance from world center
#define         LIGHT_HEIGHT          1.0f      // Light height position

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT
} LightType;

typedef struct {
    bool enabled;
    LightType type;
    Vector3 position;
    Vector3 target;
    Color color;
    int enabledLoc;
    int typeLoc;
    int posLoc;
    int targetLoc;
    int colorLoc;
} Light;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int lightsCount = 0;                     // Current amount of created lights

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
Light CreateLight(int type, Vector3 pos, Vector3 targ, Color color, Shader shader);         // Defines a light and get locations from PBR shader
void UpdateLightValues(Shader shader, Light light);                                         // Send to PBR shader light values

#ifdef __cplusplus
}
#endif

#endif // RLIGHTS_H


/***********************************************************************************
*
*   RLIGHTS IMPLEMENTATION
*
************************************************************************************/

#if defined(RLIGHTS_IMPLEMENTATION)

#include "raylib.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

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
// ...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Defines a light and get locations from PBR shader
Light CreateLight(int type, Vector3 pos, Vector3 targ, Color color, Shader shader)
{
    Light light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = true;
        light.type = type;
        light.position = pos;
        light.target = targ;
        light.color = color;

        char enabledName[32] = "lights[x].enabled\0";
        char typeName[32] = "lights[x].type\0";
        char posName[32] = "lights[x].position\0";
        char targetName[32] = "lights[x].target\0";
        char colorName[32] = "lights[x].color\0";
        enabledName[7] = '0' + lightsCount;
        typeName[7] = '0' + lightsCount;
        posName[7] = '0' + lightsCount;
        targetName[7] = '0' + lightsCount;
        colorName[7] = '0' + lightsCount;

        light.enabledLoc = GetShaderLocation(shader, enabledName);
        light.typeLoc = GetShaderLocation(shader, typeName);
        light.posLoc = GetShaderLocation(shader, posName);
        light.targetLoc = GetShaderLocation(shader, targetName);
        light.colorLoc = GetShaderLocation(shader, colorName);

        UpdateLightValues(shader, light);
        lightsCount++;
    }

    return light;
}

// Send to PBR shader light values
void UpdateLightValues(Shader shader, Light light)
{
    // Send to shader light enabled state and type
    SetShaderValue(shader, light.enabledLoc, &light.enabled, UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, UNIFORM_INT);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.posLoc, position, UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.target.x, light.target.y, light.target.z };
    SetShaderValue(shader, light.targetLoc, target, UNIFORM_VEC3);

    // Send to shader light color values
    float diff[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255, (float)light.color.b/(float)255, (float)light.color.a/(float)255 };
    SetShaderValue(shader, light.colorLoc, diff, UNIFORM_VEC4);
}

#endif // RLIGHTS_IMPLEMENTATION