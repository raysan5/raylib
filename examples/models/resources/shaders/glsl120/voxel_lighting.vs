#version 120
// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Uniforms
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output to fragment shader
varying vec3 fragPosition;
varying vec4 fragColor;
varying vec3 fragNormal;

void main()
{
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
