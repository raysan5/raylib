#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec2 fragTexCoord2;
varying vec3 fragPosition;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    vec4 texelColor2 = texture2D(texture1, fragTexCoord2);

    gl_FragColor = texelColor*texelColor2;
}
