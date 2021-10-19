#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 textureSize;
uniform float outlineSize;
uniform vec4 outlineColor;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord);   // Get texel color
    vec2 texelScale = vec2(0.0);
    texelScale.x = outlineSize/textureSize.x;
    texelScale.y = outlineSize/textureSize.y;

    // We sample four corner texels, but only for the alpha channel (this is for the outline)
    vec4 corners = vec4(0.0);
    corners.x = texture(texture0, fragTexCoord + vec2(texelScale.x, texelScale.y)).a;
    corners.y = texture(texture0, fragTexCoord + vec2(texelScale.x, -texelScale.y)).a;
    corners.z = texture(texture0, fragTexCoord + vec2(-texelScale.x, texelScale.y)).a;
    corners.w = texture(texture0, fragTexCoord + vec2(-texelScale.x, -texelScale.y)).a;

    float outline = min(dot(corners, vec4(1.0)), 1.0);
    vec4 color = mix(vec4(0.0), outlineColor, outline);
    finalColor = mix(color, texel, texel.a);
}