#version 330

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;

uniform mat4 projectionMatrix;
uniform mat4 modelviewMatrix;

varying vec2 fragTexCoord;

void main()
{
    fragTexCoord = vertexTexCoord;
    gl_Position = projectionMatrix*modelviewMatrix*vec4(vertexPosition, 1.0);
}