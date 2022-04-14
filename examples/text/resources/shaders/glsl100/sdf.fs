#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables
const float smoothing = 1.0/16.0;

void main()
{
    // Texel color fetching from texture sampler
    // NOTE: Calculate alpha using signed distance field (SDF)
    float distance = texture2D(texture0, fragTexCoord).a;
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);

    // Calculate final fragment color
    gl_FragColor = vec4(fragColor.rgb, fragColor.a*alpha);
}
