#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

// Output values to fragment shader
out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

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
