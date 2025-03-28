#version 100

in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D depthTexture;

float linearizeDepth(float depth)
{
    float n = 0.1;    // near plane
    float f = 100.0;  // far plane
    return (2.0 * n) / (f + n - depth * (f - n));
}

void main() {
    vec2 flippedTexCoord = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);
    float depth = texture(depthTexture, flippedTexCoord).r;
    float linearDepth = linearizeDepth(depth);
    finalColor = vec4(vec3(linearDepth), 1.0);
}
