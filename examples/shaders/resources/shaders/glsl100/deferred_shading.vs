#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;

// Output vertex attributes (to fragment shader)
varying vec2 fragTexCoord;

void main()
{
    fragTexCoord = vertexTexCoord;

    // Calculate final vertex position
    gl_Position = vec4(vertexPosition, 1.0);    
}
