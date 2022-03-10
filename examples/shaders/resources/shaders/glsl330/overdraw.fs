#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    // To show overdraw, we just render all the fragments
    // with a solid color and some transparency

    // NOTE: This is not a postpro render,
    // it will only render all screen texture in a plain color

    finalColor = vec4(1.0, 0.0, 0.0, 0.2);
}

