#version 330

// Raylib standard attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Raylib standard uniforms
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
