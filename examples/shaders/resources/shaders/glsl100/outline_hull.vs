#version 100

attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;

uniform mat4 mvp;
uniform float outlineThickness;

void main()
{
    vec3 extruded = vertexPosition + vertexNormal * outlineThickness;
    gl_Position = mvp * vec4(extruded, 1.0);
}