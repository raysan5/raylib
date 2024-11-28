#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
    // Fetch color from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    // Calculate final fragment color
    gl_FragColor = texelColor*colDiffuse*fragColor;
}
