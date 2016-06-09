#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 sum = vec4(0);
    vec4 tc = vec4(0);

    for (int i = -4; i < 4; i++)
    {
        for (int j = -3; j < 3; j++)
        {
            sum += texture2D(texture0, fragTexCoord + vec2(j, i)*0.004) * 0.25;
        }
    }
    
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    // Calculate final fragment color
    if (texelColor.r < 0.3) tc = sum*sum*0.012 + texelColor;
    else if (texelColor.r < 0.5) tc = sum*sum*0.009 + texelColor;
    else tc = sum*sum*0.0075 + texelColor;
    
    gl_FragColor = tc;
}