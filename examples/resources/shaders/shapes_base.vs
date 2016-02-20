#version 330

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

uniform mat4 mvpMatrix;

varying vec2 fragTexCoord;
varying vec4 fragTintColor;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragTintColor = vertexColor;
    
    gl_Position = mvpMatrix*vec4(vertexPosition, 1.0);
}