#version 330

// Vertex input data
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

// Projection and model data
uniform mat4 mvpMatrix;

uniform mat4 modelMatrix;
//uniform mat4 viewMatrix;  // Not used

// Attributes to fragment shader
out vec2 fragTexCoord;
out vec3 fragNormal;

void main()
{
    // Send texture coord to fragment shader
    fragTexCoord = vertexTexCoord;
    
    // Calculate view vector normal from model
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    fragNormal = normalize(normalMatrix*vertexNormal);
    
    // Calculate final vertex position
    gl_Position = mvpMatrix*vec4(vertexPosition, 1.0);
}