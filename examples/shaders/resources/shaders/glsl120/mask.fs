#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D mask;
uniform vec4 colDiffuse;
uniform int frame;

// NOTE: Add your custom variables here

void main()
{
    vec4 maskColour = texture2D(mask, fragTexCoord + vec2(sin(-float(frame)/150.0)/10.0, cos(-float(frame)/170.0)/10.0));
    if (maskColour.r < 0.25) discard;
    vec4 texelColor = texture2D(texture0, fragTexCoord + vec2(sin(float(frame)/90.0)/8.0, cos(float(frame)/60.0)/8.0));

    gl_FragColor = texelColor*maskColour;
}
