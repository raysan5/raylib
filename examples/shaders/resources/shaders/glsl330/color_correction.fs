#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float contrast;
uniform float saturation;
uniform float brightness;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);   // Get texel color

    // Apply contrast
    texel.rgb = (texel.rgb - 0.5f)*(contrast/100.0f + 1.0f) + 0.5f;

    // Apply brightness
    texel.rgb = texel.rgb + brightness/100.0f;

    // Apply saturation
    float intensity = dot(texel.rgb, vec3(0.299f, 0.587f, 0.114f));
    texel.rgb = (texel.rgb - intensity)*saturation/100.0f + texel.rgb;

    // Output resulting color
    finalColor = texel;
}