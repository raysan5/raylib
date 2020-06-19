/*******************************************************************************************
*
*   rPBR [shader] - Background skybox fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
*	19-Jun-2020 - modified by Giuseppe Mastrangelo (@peppemas) - VFlip Support
*
**********************************************************************************************/

#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;
uniform bool vflipped;

// Output fragment color
out vec4 finalColor;

vec4 flipTextureCube(samplerCube sampler, vec3 texCoord) {
	return texture(sampler, vec3(texCoord.x,-texCoord.y,texCoord.z));
}

void main()
{
    // Fetch color from texture map
    vec3 color;

    if (vflipped )
    	color = flipTextureCube(environmentMap, fragPosition).rgb;
    else 
    	color = texture(environmentMap, fragPosition).rgb;

    // Apply gamma correction
    color = color/(color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    // Calculate final fragment color
    finalColor = vec4(color, 1.0);
}
