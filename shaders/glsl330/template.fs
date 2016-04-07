#version 330

in vec2 fragTexCoord;

out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    // NOTE: Implement here your fragment shader code
    
    fragColor = texelColor*fragTintColor;
}
