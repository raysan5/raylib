#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

float hatchOffsetY = 5.0;
float lumThreshold01 = 0.9;
float lumThreshold02 = 0.7;
float lumThreshold03 = 0.5;
float lumThreshold04 = 0.3;

void main()
{
    vec3 tc = vec3(1.0, 1.0, 1.0);
    float lum = length(texture(texture0, fragTexCoord).rgb);

    if (lum < lumThreshold01)
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold02)
    {
        if (mod(gl_FragCoord.x - gl_FragCoord.y, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold03)
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y - hatchOffsetY, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold04)
    {
        if (mod(gl_FragCoord.x - gl_FragCoord.y - hatchOffsetY, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    finalColor = vec4(tc, 1.0);
}