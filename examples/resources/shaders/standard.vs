#version 330 

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

uniform mat4 mvpMatrix;

void main()
{
    fragPosition = vertexPosition;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;

    gl_Position = mvpMatrix*vec4(vertexPosition, 1.0);
}