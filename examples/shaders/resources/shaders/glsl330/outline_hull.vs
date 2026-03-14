#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform float outlineThickness;

void main() {
    // Extrude vertex along its normal to create the hull.
    vec3 extruded = vertexPosition + vertexNormal * outlineThickness;
    gl_Position = mvp * vec4(extruded, 1.0);
}
