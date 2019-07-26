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

vec3 BlendOverlay(vec3 base, vec3 blend)
{
    float red;
    float green;
    float blue;

    if (base.r < 0.5) red = 2.0*base.r*blend.r;
    else red = 1.0 - 2.0*(1.0 - base.r)*(1.0 - blend.r);

    if (base.g < 0.5) green = 2.0*base.g*blend.g;
    else green = 1.0 - 2.0 *(1.0 - base.g)*(1.0 - blend.g);

    if (base.b < 0.5) blue = 2.0*base.b*blend.b;
    else blue = 1.0 - 2.0 *(1.0 - base.b)*(1.0 - blend.b);

    return vec3(red, green, blue);
}

void main()
{
    // Blending Overlay
    vec4 base = texture2D(texture0, fragTexCoord);

    // No blending shader -> 64 FPS (1280x720)
    //gl_FragColor = base*tintColor;

    // Option01 -> 50 FPS (1280x720), 200 FPS (640x360)
    vec3 final = BlendOverlay(base.rgb, fragColor.rgb);
    finalColor = vec4(final.rgb, base.a*fragColor.a);

    // Option02 (Doesn't work) -> 63 FPS (1280x720)
    //float luminance = (base.r*0.2126) + (base.g*0.7152) + (base.b*0.0722);
    //gl_FragColor = vec4(tintColor.rgb*luminance, base.a);

    // Option03 (no branches, precalculated ifs) -> 28 FPS (1280x720)
    /*
    vec4 blend = tintColor;
    //if (base.a == 0.0) discard;   // No improvement
    vec3 br = clamp(sign(base.rgb - vec3(0.5)), vec3(0.0), vec3(1.0));
    vec3 multiply = 2.0 * base.rgb * blend.rgb;
    vec3 screen = vec3(1.0) - 2.0 * (vec3(1.0) - base.rgb)*(vec3(1.0) - blend.rgb);
    vec3 overlay = mix(multiply, screen, br);
    vec3 finalColor = mix(base.rgb, overlay, blend.a);
    gl_FragColor = vec4(finalColor, base.a);
    */

    // Option04 (no branches, using built-in functions) -> 38 FPS (1280x720)
    //gl_FragColor = vec4(mix(1 - 2*(1 - base.rgb)*(1 - tintColor.rgb), 2*base.rgb*tintColor.rgb, step(0.5, base.rgb)), base.a);
}
