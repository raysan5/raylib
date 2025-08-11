#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add your custom variables here
uniform vec2 tiling;

void main()
{
    vec2 texCoord = fragTexCoord*tiling;

    gl_FragColor = texture2D(texture0, texCoord)*colDiffuse;
}
