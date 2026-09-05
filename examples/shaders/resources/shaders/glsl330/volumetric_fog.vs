#version 330

// Input vertex attributes
in vec3 vertexPosition; // local XYZ-position
in vec2 vertexTexCoord; // UV coordinates
in vec3 vertexNormal; // direction of the surface at that vertex
in vec4 vertexColor;

// equivalent to matProjection * matView * matModel
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
