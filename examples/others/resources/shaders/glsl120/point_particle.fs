#version 120

// Input uniform values
uniform vec4 color;

// NOTE: Add your custom variables here

void main()
{
    // Each point is drawn as a screen space square of gl_PointSize size. gl_PointCoord contains where we are inside of
    // it. (0, 0) is the top left, (1, 1) the bottom right corner
    // Draw each point as a colored circle with alpha 1.0 in the center and 0.0 at the outer edges
    gl_FragColor = vec4(color.rgb, color.a*(1.0 - length(gl_PointCoord.xy - vec2(0.5))*2.0));
}