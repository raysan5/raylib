#version 120

// Input vertex attributes (from fragment shader)
varying vec2 fragTexCoord;
varying float height;

void main()
{
    vec4 darkblue = vec4(0.0, 0.13, 0.18, 1.0);
    vec4 lightblue = vec4(1.0, 1.0, 1.0, 1.0);
    // Interpolate between two colors based on height
    vec4 finalColor = mix(darkblue, lightblue, height);

    gl_FragColor = finalColor;
}