#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec3 texelColor = texture(texture0, fragTexCoord).rgb;
    vec3 colors[3];
    colors[0] = vec3(0.0, 0.0, 1.0);
    colors[1] = vec3(1.0, 1.0, 0.0);
    colors[2] = vec3(1.0, 0.0, 0.0);

    float lum = (texelColor.r + texelColor.g + texelColor.b)/3.0;

    int ix = (lum < 0.5)? 0:1;

    vec3 tc = mix(colors[ix], colors[ix + 1], (lum - float(ix)*0.5)/0.5);

    finalColor = vec4(tc, 1.0);
}