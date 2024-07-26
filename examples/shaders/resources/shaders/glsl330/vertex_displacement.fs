#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in float height;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 darkblue = vec4(0.0, 0.13, 0.18, 1.0);
    vec4 lightblue = vec4(1.0, 1.0, 1.0, 1.0);
    // interplate between two colors based on height
    finalColor = mix(darkblue, lightblue, height);
}