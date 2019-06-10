/*******************************************************************************************
*
*   rPBR [shader] - Bidirectional reflectance distribution function vertex shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;

void main()
{
    // Calculate fragment position based on model transformations
    fragTexCoord = vertexTexCoord;

    // Calculate final vertex position
    gl_Position = vec4(vertexPosition, 1.0);
}