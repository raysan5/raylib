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

float gamma = 0.6;
float numColors = 8.0;

void main()
{
    // Texel color fetching from texture sampler
    vec3 texelColor = texture(texture0, fragTexCoord.xy).rgb;

    texelColor = pow(texelColor, vec3(gamma, gamma, gamma));
    texelColor = texelColor*numColors;
    texelColor = floor(texelColor);
    texelColor = texelColor/numColors;
    texelColor = pow(texelColor, vec3(1.0/gamma));

    finalColor = vec4(texelColor, 1.0);
}