#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

void main()
{
    vec3 color = texture2D(texture0, fragTexCoord).rgb;
    vec3 colors[3];
    colors[0] = vec3(0.0, 0.0, 1.0);
    colors[1] = vec3(1.0, 1.0, 0.0);
    colors[2] = vec3(1.0, 0.0, 0.0);
    
    float lum = (color.r + color.g + color.b)/3.0;
    
    int ix = (lum < 0.5)? 0:1;
    
    vec3 tc = mix(colors[ix], colors[ix + 1], (lum - float(ix)*0.5)/0.5);
    
    fragColor = vec4(tc, 1.0);
}