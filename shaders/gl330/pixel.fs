#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables 

const float renderWidth = 1280;
const float renderHeight = 720;

uniform float pixelWidth = 5.0f;
uniform float pixelHeight = 5.0f;

void main()
{
    float dx = pixelWidth*(1.0/renderWidth);
    float dy = pixelHeight*(1.0/renderHeight);

    vec2 coord = vec2(dx*floor(fragTexCoord.x/dx), dy*floor(fragTexCoord.y/dy));

    vec3 tc = texture2D(texture0, coord).rgb;

    fragColor = vec4(tc, 1.0);
}