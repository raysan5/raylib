#version 100

precision mediump float;

const int colors = 8;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform ivec3 palette[colors];

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord)*fragColor;

    // Convert the (normalized) texel color RED component (GB would work, too)
    // to the palette index by scaling up from [0, 1] to [0, 255].
    int index = int(texelColor.r*255.0);

    ivec3 color = ivec3(0);

    // NOTE: On GLSL 100 we are not allowed to index a uniform array by a variable value,
    // a constantmust be used, so this logic...
    if (index == 0) color = palette[0];
    else if (index == 1) color = palette[1];
    else if (index == 2) color = palette[2];
    else if (index == 3) color = palette[3];
    else if (index == 4) color = palette[4];
    else if (index == 5) color = palette[5];
    else if (index == 6) color = palette[6];
    else if (index == 7) color = palette[7];

    // Calculate final fragment color. Note that the palette color components
    // are defined in the range [0, 255] and need to be normalized to [0, 1]
    // for OpenGL to work.
    gl_FragColor = vec4(float(color.x)/255.0, float(color.y)/255.0, float(color.z)/255.0, texelColor.a);
}
