#version 330 core

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 tiling;

out vec4 finalColor;

void main()
{
    vec2 texCoord = fragTexCoord*tiling;

    finalColor = texture(texture0, texCoord)*colDiffuse;
}
