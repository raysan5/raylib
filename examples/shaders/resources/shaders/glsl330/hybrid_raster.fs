#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add your custom variables here

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    finalColor = texelColor*colDiffuse*fragColor;
    gl_FragDepth = finalColor.z;
}