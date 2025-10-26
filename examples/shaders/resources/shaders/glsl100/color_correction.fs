#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float contrast;
uniform float saturation;
uniform float brightness;

void main()
{
    // Get texel color
    vec4 texel = texture2D(texture0, fragTexCoord);

    // Apply contrast
    texel.rgb = (texel.rgb - 0.5)*(contrast/100.0 + 1.0) + 0.5;

    // Apply brightness
    texel.rgb = texel.rgb + brightness/100.0;

    // Apply saturation
    float intensity = dot(texel.rgb, vec3(0.299, 0.587, 0.114));
    texel.rgb = (texel.rgb - intensity)*saturation/100.0 + texel.rgb;

    // Output resulting color
    gl_FragColor = texel;
}
