#version 300 es

precision highp float;

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 texCoord;

void main() {
    gl_Position = vec4(vertexPosition, 1.0);
    texCoord = vertexTexCoord;
}
