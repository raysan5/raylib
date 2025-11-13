#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform vec2 iResolution;
uniform sampler2D iChannel0;

const vec4  BLACK = vec4(0.0, 0.0, 0.0, 1.0);
const vec4  WHITE = vec4(1.0, 1.0, 1.0, 1.0);
const float FFT_ROW = 0.0;
const float NUM_OF_BINS = 512.0;

void main() {
    vec2  fragCoord = fragTexCoord*iResolution;
    float cell_width = iResolution.x/NUM_OF_BINS;
    float bin_index = floor(fragCoord.x/cell_width);
    float local_x = mod(fragCoord.x, cell_width);
    float bar_width  = cell_width - 1.0;
    vec4  color = BLACK;
    if (local_x <= bar_width) {
        float sample_x = (bin_index + 0.5)/NUM_OF_BINS;
        vec2  sample_coord = vec2(sample_x, FFT_ROW);
        float amplitude = texture(iChannel0, sample_coord).r; // only filled the red channel, all channels left open for alternative use
        if (fragTexCoord.y < amplitude) {
            color = WHITE;
        }
    }
    finalColor = color;
}
