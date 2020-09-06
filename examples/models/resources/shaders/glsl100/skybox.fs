/*******************************************************************************************
*
*   rPBR [shader] - Background skybox fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
*	19-Jun-2020 - modified by Giuseppe Mastrangelo (@peppemas) - VFlip Support
*
**********************************************************************************************/

#version 110

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;
uniform bool vflipped;

void main()
{
    // Fetch color from texture map
    vec3 color = { 0.0 };

    if (vflipped) color = texture2D(environmentMap, vec3(fragPosition.x, -fragPosition.y, fragPosition.z)).rgb;
    else color = texture2D(environmentMap, fragPosition).rgb;

    // Apply gamma correction
    color = color/(color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    // Calculate final fragment color
    gl_FragColor = vec4(color, 1.0);
}
