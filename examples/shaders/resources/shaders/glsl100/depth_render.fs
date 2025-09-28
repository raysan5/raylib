#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;

// Input uniform values
uniform sampler2D depthTexture;
uniform bool flipY;

float nearPlane = 0.1;
float farPlane = 100.0;

void main()
{
    // Handle potential Y-flipping
    vec2 texCoord = fragTexCoord;
    if (flipY) texCoord.y = 1.0 - texCoord.y;

    // Sample depth texture
    float depth = texture2D(depthTexture, texCoord).r;

    // Linearize depth
    float linearDepth = (2.0*nearPlane)/(farPlane + nearPlane - depth*(farPlane - nearPlane));

    // Output final color
    gl_FragColor = vec4(vec3(linearDepth), 1.0);
}
