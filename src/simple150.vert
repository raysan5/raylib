#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // Pass some variables to the fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    // Apply all matrix transformations to vertex
    gl_Position = projectionMatrix * modelviewMatrix * vec4(vertexPosition, 1.0);
}