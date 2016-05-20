#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

// Input uniform values
uniform mat4 mvpMatrix;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec3 fragNormal;

// NOTE: Add here your custom variables
uniform mat4 modelMatrix;

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;

    // Calculate view vector normal from model
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    fragNormal = normalize(normalMatrix*vertexNormal);
    
    // Calculate final vertex position
    gl_Position = mvpMatrix*vec4(vertexPosition, 1.0);
}