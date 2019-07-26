/*******************************************************************************************
*
*   rPBR [shader] - Equirectangular to cubemap vertex shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330

// Input vertex attributes
in vec3 vertexPosition;

// Input uniform values
uniform mat4 projection;
uniform mat4 view;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;

void main()
{
    // Calculate fragment position based on model transformations
    fragPosition = vertexPosition;

    // Calculate final vertex position
    gl_Position = projection*view*vec4(vertexPosition, 1.0);
}
