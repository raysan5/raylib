#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables 

const float renderWidth = 1280;
const float renderHeight = 720;

uniform float pixelWidth = 5.0f;
uniform float pixelHeight = 5.0f;

void main() 
{ 
    float dx = pixelWidth*(1.0/renderWidth);
    float dy = pixelHeight*(1.0/renderHeight);
    
    vec2 coord = vec2(dx*floor(fragTexCoord.x/dx), dy*floor(fragTexCoord.y/dy));
    
    vec3 tc = texture2D(texture0, coord).rgb;

    gl_FragColor = vec4(tc, 1.0);
}