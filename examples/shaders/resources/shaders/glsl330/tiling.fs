#version 330 core

uniform sampler2D diffuseMap;
uniform vec2 tiling;

in vec2 fragTexCoord;

out vec4 fragColor;

void main()
{
    vec2 texCoord = fragTexCoord * tiling;
    fragColor = texture(diffuseMap, texCoord);
}
