#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform vec2 iResolution;
uniform sampler2D iChannel0;

const vec4 BLACK = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 WHITE = vec4(1.0, 1.0, 1.0, 1.0);
const float FFT_ROW = 0.0;
const float NUM_OF_BINS = 512.0;

void main()
{
    vec2 fragCoord = fragTexCoord*iResolution;
    float cellWidth = iResolution.x/NUM_OF_BINS;
    float binIndex = floor(fragCoord.x/cellWidth);
    float localX = mod(fragCoord.x, cellWidth);
    float barWidth  = cellWidth - 1.0;
    vec4 color = WHITE;
    
    if (localX <= barWidth)
    {
        float sampleX = (binIndex + 0.5)/NUM_OF_BINS;
        vec2  sampleCoord = vec2(sampleX, FFT_ROW);
        float amplitude = texture(iChannel0, sampleCoord).r; // Only filled the red channel, all channels left open for alternative use
        
        if (fragTexCoord.y < amplitude) color = BLACK;
    }
    
    finalColor = color;
}
