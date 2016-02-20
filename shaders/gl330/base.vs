#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

out vec2 fragTexCoord;

uniform mat4 mvpMatrix;

// NOTE: Add here your custom variables 

void main()
{
    fragTexCoord = vertexTexCoord;
    
    gl_Position = mvpMatrix*vec4(vertexPosition, 1.0);
}