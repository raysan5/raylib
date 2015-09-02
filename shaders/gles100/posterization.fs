#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 tintColor;

// NOTE: Add here your custom variables

float gamma = 0.6;
float numColors = 8.0;

void main() 
{ 
    vec3 color = texture2D(texture0, fragTexCoord.xy).rgb;
    
    color = pow(color, vec3(gamma, gamma, gamma));
    color = color*numColors;
    color = floor(color);
    color = color/numColors;
    color = pow(color, vec3(1.0/gamma));
    
    gl_FragColor = vec4(color, 1.0);
}