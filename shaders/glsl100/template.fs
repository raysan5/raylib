#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    // NOTE: Implement here your fragment shader code
    
    gl_FragColor = texelColor*fragTintColor;
}