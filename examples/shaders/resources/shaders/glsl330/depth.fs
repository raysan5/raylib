#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Input uniform values
uniform sampler2D depthTexture;
uniform bool flipY;

const float nearPlane = 0.1;
const float farPlane = 100.0;

// Output fragment color
out vec4 finalColor;

// Linearizes the depth buffer value
float linearizeDepth(float depth)
{
    return (2.0 * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

void main()
{
    // Handle potential Y-flipping
    vec2 texCoord = fragTexCoord;
    if (flipY)
        texCoord.y = 1.0 - texCoord.y;

    // Sample depth
    float depth = texture(depthTexture, texCoord).r;

    // Linearize depth value
    float linearDepth = linearizeDepth(depth);

    // Output final color
    finalColor = vec4(vec3(linearDepth), 1.0);
}