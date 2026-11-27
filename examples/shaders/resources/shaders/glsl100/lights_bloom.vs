#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;

// Output values to fragment shader
varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec2 fragTexCoord;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;

void main()
{
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragNormal = normalize(vec3(matModel*vec4(vertexNormal, 0.0)));
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
