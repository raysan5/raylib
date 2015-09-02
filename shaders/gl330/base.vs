#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

out vec2 fragTexCoord;

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;

// NOTE: Add here your custom variables 

void main()
{
    fragTexCoord = vertexTexCoord;
    
    gl_Position = projectionMatrix*modelviewMatrix*vec4(vertexPosition, 1.0);
}