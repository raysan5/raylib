#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;

void main()
{
    vec4 color = texture(texture0, fragTexCoord);

    // High-pass threshold blur: only pixels brighter than 0.75 in any channel contribute
    vec4 bloom = vec4(0.0);
    vec2 texel = vec2(1.0/1280.0, 1.0/720.0)*2.5;

    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; y++)
        {
            vec4 smp = texture(texture0, fragTexCoord + vec2(float(x), float(y))*texel);
            float brightness = max(smp.r, max(smp.g, smp.b));
            if (brightness > 0.75) bloom += smp;
        }
    }
    bloom /= 25.0;

    // Combine original with the bloom glow, then Reinhard tone-map back to [0, 1]
    vec3 hdr = color.rgb + bloom.rgb*1.2;
    vec3 ldr = hdr/(hdr + vec3(1.0));

    finalColor = vec4(ldr, color.a);
}
