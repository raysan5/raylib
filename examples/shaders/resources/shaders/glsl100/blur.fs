#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add your custom variables here

// NOTE: Render size values must be passed from code
const float renderWidth = 800.0;
const float renderHeight = 450.0;

vec3 offset = vec3(0.0, 1.3846153846, 3.2307692308);
vec3 weight = vec3(0.2270270270, 0.3162162162, 0.0702702703);

void main()
{
    // Texel color fetching from texture sampler
    vec3 tc = texture2D(texture0, fragTexCoord).rgb*weight.x;

    tc += texture2D(texture0, fragTexCoord + vec2(offset.y)/renderWidth, 0.0).rgb*weight.y;
    tc += texture2D(texture0, fragTexCoord - vec2(offset.y)/renderWidth, 0.0).rgb*weight.y;

    tc += texture2D(texture0, fragTexCoord + vec2(offset.z)/renderWidth, 0.0).rgb*weight.z;
    tc += texture2D(texture0, fragTexCoord - vec2(offset.z)/renderWidth, 0.0).rgb*weight.z;

    gl_FragColor = vec4(tc, 1.0);
}