/*******************************************************************************************
*
*   rPBR [shader] - Background skybox fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Fetch color from texture map
    vec3 color = texture(environmentMap, fragPosition).rgb;

    // Apply gamma correction
    color = color/(color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    // Calculate final fragment color
    finalColor = vec4(color, 1.0);
}
