#version 330

const int MAX_INDEXED_COLORS = 8;

// Input fragment attributes (from fragment shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform ivec3 palette[MAX_INDEXED_COLORS];
//uniform sampler2D palette; // Alternative to ivec3, palette provided as a 256x1 texture

// Output fragment color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture sampler
    // NOTE: The texel is actually the a GRAYSCALE index color
    vec4 texelColor = texture(texture0, fragTexCoord)*fragColor;

    // Convert the (normalized) texel color RED component (GB would work, too)
    // to the palette index by scaling up from [0..1] to [0..255]
    int index = int(texelColor.r*255.0);
    ivec3 color = palette[index];
    
    //finalColor = texture(palette, texelColor.xy); // Alternative to ivec3

    // Calculate final fragment color. Note that the palette color components
    // are defined in the range [0..255] and need to be normalized to [0..1]
    finalColor = vec4(color/255.0, texelColor.a);
}
