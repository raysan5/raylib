# version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

float hatchOffsetY = 5.0;
float lumThreshold01 = 0.9;
float lumThreshold02 = 0.7;
float lumThreshold03 = 0.5;
float lumThreshold04 = 0.3;

void main()
{
    vec3 tc = vec3(1.0, 1.0, 1.0);
    float lum = length(texture2D(texture0, fragTexCoord).rgb);

    if (lum < lumThreshold01)
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold02)
    {
        if (mod(gl_FragCoord .x - gl_FragCoord .y, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold03)
    {
        if (mod(gl_FragCoord .x + gl_FragCoord .y - hatchOffsetY, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    if (lum < lumThreshold04)
    {
        if (mod(gl_FragCoord .x - gl_FragCoord .y - hatchOffsetY, 10.0) == 0.0) tc = vec3(0.0, 0.0, 0.0);
    }

    gl_FragColor = vec4(tc, 1.0);
}