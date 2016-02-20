#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 base = texture(texture0, fragTexCoord)*fragTintColor;
    
    // Convert to grayscale using NTSC conversion weights
    float gray = dot(base.rgb, vec3(0.299, 0.587, 0.114));
    
    fragColor = vec4(gray, gray, gray, fragTintColor.a);
}