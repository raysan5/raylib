#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexTangent;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform vec3 lightPos;
uniform vec4 difColor;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out mat3 TBN;

const float normalOffset = 0.1;

void main()
{
    // Compute binormal from vertex normal and tangent
    vec3 vertexBinormal = cross(vertexNormal, vertexTangent.xyz)*vertexTangent.w;

    // Compute fragment normal based on normal transformations
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));

    // Compute fragment position based on model transformations
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));

    fragTexCoord = vertexTexCoord*2.0;
    fragNormal = normalize(normalMatrix*vertexNormal);
    vec3 fragTangent = normalize(normalMatrix*vertexTangent.xyz);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal)*fragNormal);
    vec3 fragBinormal = normalize(normalMatrix*vertexBinormal);
    fragBinormal = cross(fragNormal, fragTangent);

    TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
