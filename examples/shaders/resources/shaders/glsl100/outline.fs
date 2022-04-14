#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 textureSize;
uniform float outlineSize;
uniform vec4 outlineColor;

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord);   // Get texel color
    vec2 texelScale = vec2(0.0);
    texelScale.x = outlineSize/textureSize.x;
    texelScale.y = outlineSize/textureSize.y;

    // We sample four corner texels, but only for the alpha channel (this is for the outline)
    vec4 corners = vec4(0.0);
    corners.x = texture2D(texture0, fragTexCoord + vec2(texelScale.x, texelScale.y)).a;
    corners.y = texture2D(texture0, fragTexCoord + vec2(texelScale.x, -texelScale.y)).a;
    corners.z = texture2D(texture0, fragTexCoord + vec2(-texelScale.x, texelScale.y)).a;
    corners.w = texture2D(texture0, fragTexCoord + vec2(-texelScale.x, -texelScale.y)).a;

    float outline = min(dot(corners, vec4(1.0)), 1.0);
    vec4 color = mix(vec4(0.0), outlineColor, outline);
    gl_FragColor = mix(color, texel, texel.a);
}