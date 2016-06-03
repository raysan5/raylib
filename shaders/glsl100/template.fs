#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    // NOTE: Implement here your fragment shader code
    
    gl_FragColor = texelColor*colDiffuse;
}