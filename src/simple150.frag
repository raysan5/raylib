#version 150

uniform sampler2D texture0;

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 pixelColor;

void main()
{
    // Output pixel color
    pixelColor = texture(texture0, fragTexCoord) * fragColor;
}