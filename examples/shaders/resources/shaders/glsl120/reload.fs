#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;           // Texture coordinates (sampler2D)
varying vec4 fragColor;              // Tint color

// Uniform inputs
uniform vec2 resolution;        // Viewport resolution (in pixels)
uniform vec2 mouse;             // Mouse pixel xy coordinates
uniform float time;             // Total run time (in secods)

// Draw circle
vec4 DrawCircle(vec2 fragCoord, vec2 position, float radius, vec3 color)
{
    float d = length(position - fragCoord) - radius;
    float t = clamp(d, 0.0, 1.0);
    return vec4(color, 1.0 - t);
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 position = vec2(mouse.x, resolution.y - mouse.y);
    float radius = 40.0;

    // Draw background layer
    vec4 colorA = vec4(0.2,0.2,0.8, 1.0);
    vec4 colorB = vec4(1.0,0.7,0.2, 1.0);
    vec4 layer1 = mix(colorA, colorB, abs(sin(time*0.1)));

    // Draw circle layer
    vec3 color = vec3(0.9, 0.16, 0.21);
    vec4 layer2 = DrawCircle(fragCoord, position, radius, color);

    // Blend the two layers
    gl_FragColor = mix(layer1, layer2, layer2.a);
}
