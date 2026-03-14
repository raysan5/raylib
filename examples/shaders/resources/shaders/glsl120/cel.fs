#version 120

varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform float numBands;

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};
uniform Light lights[4];

void main()
{
    vec4 texColor = texture2D(texture0, fragTexCoord);
    vec3 baseColor = texColor.rgb * fragColor.rgb * colDiffuse.rgb;
    vec3 norm = normalize(fragNormal);

    float lightAccum = 0.08; // ambient floor

    for (int i = 0; i < 4; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 lightDir;
            if (lights[i].type == 0)
            {
                // Directional: direction is from position toward target.
                lightDir = normalize(lights[i].position - lights[i].target);
            }
            else
            {
                // Point: direction from surface to light.
                lightDir = normalize(lights[i].position - fragPosition);
            }

            float NdotL = max(dot(norm, lightDir), 0.0);

            // Quantize NdotL into numBands discrete steps.
            float quantized = min(floor(NdotL * numBands), numBands - 1.0) / (numBands - 1.0);
            lightAccum += quantized * lights[i].color.r;
        }
    }

    lightAccum = clamp(lightAccum, 0.0, 1.0);
    gl_FragColor = vec4(baseColor * lightAccum, texColor.a * colDiffuse.a);
}