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
    vec3 color = texture2D(texture0, fragTexCoord).rgb;
    vec3 colors[3];
    colors[0] = vec3(0.0, 0.0, 1.0);
    colors[1] = vec3(1.0, 1.0, 0.0);
    colors[2] = vec3(1.0, 0.0, 0.0);
    
    float lum = (color.r + color.g + color.b)/3.0;
    
    int ix = (lum < 0.5)? 0:1;
    
    vec3 tc = mix(colors[ix], colors[ix+1], (lum-float(ix)*0.5)/0.5);
    
    gl_FragColor = vec4(tc, 1.0);
}