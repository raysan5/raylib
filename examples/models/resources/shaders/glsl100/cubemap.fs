#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;

// Input uniform values
uniform sampler2D equirectangularMap;

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183);
    uv += 0.5;
    return uv;
}

void main()
{
    // Normalize local position
    vec2 uv = SampleSphericalMap(normalize(fragPosition));

    // Fetch color from texture map
    vec3 color = texture2D(equirectangularMap, uv).rgb;

    // Calculate final fragment color
    gl_FragColor = vec4(color, 1.0);
}
