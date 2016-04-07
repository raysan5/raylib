#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;

varying vec2 fragTexCoord;

uniform mat4 mvpMatrix;

// NOTE: Add here your custom variables 

void main()
{
    vec3 normal = vertexNormal;
    
    fragTexCoord = vertexTexCoord;
    
    gl_Position = mvpMatrix*vec4(vertexPosition, 1.0);
}