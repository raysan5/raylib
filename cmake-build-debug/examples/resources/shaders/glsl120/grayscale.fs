#version 120

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
    vec4 texelColor = texture2D(texture0, fragTexCoord)*colDiffuse*fragColor;
    
    // Convert texel color to grayscale using NTSC conversion weights
    float gray = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
    
    // Calculate final fragment color
    gl_FragColor = vec4(gray, gray, gray, texelColor.a);
}