#version 110

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;

varying vec2 fragTexCoord;
varying vec4 fragColor;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    
    gl_Position = projectionMatrix*modelviewMatrix*vec4(vertexPosition, 1.0);
}