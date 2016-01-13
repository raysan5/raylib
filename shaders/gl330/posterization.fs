#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

float gamma = 0.6f;
float numColors = 8.0f;

void main()
{
    vec3 color = texture2D(texture0, fragTexCoord.xy).rgb;
    
    color = pow(color, vec3(gamma, gamma, gamma));
    color = color*numColors;
    color = floor(color);
    color = color/numColors;
    color = pow(color, vec3(1.0/gamma));
    
    fragColor = vec4(color, 1.0);
}