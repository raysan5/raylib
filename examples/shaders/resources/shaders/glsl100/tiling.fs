#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D diffuseMap;
uniform vec4 tiling;

// NOTE: Add here your custom variables

void main()
{
    vec2 texCoord = fragTexCoord*tiling;
    fragColor = texture2D(diffuseMap, texCoord);
    
    gl_FragColor = fragColor;
}
