#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;     // Depth texture
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    float zNear = 0.01; // camera z near
    float zFar = 10.0;  // camera z far
    float z = texture(texture0, fragTexCoord).x;

    // Linearize depth value
    float depth = (2.0*zNear)/(zFar + zNear - z*(zFar - zNear));

    // Calculate final fragment color
    finalColor = vec4(depth, depth, depth, 1.0f);
}