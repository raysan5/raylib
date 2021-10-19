#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 colDiffuse;

uniform float divider;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor0 = texture2D(texture0, fragTexCoord);
    vec4 texelColor1 = texture2D(texture1, fragTexCoord);

    float x = fract(fragTexCoord.s);
    float final = smoothstep(divider - 0.1, divider + 0.1, x);

    gl_FragColor = mix(texelColor0, texelColor1, final);
}
