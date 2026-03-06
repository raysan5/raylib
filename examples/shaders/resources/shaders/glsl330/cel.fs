#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Raylib standard uniforms
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// View position for future specular / fresnel use.
uniform vec3 viewPos;

// Number of discrete toon bands (2 = hard binary, 10 = default, 20 = near-smooth).
uniform float numBands;

// rlights.h compatible light block.
struct Light {
    int  enabled;
    int  type; // 0 = directional, 1 = point
    vec3 position;
    vec3 target;
    vec4 color;
    float attenuation;
};
uniform Light lights[4];

out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    vec3 baseColor = texColor.rgb * fragColor.rgb * colDiffuse.rgb;
    vec3 norm = normalize(fragNormal);

    float lightAccum = 0.08; // ambient floor

    for (int i = 0; i < 4; i++) {
        if (lights[i].enabled == 0) continue;

        vec3 lightDir;
        if (lights[i].type == 0) {
            // Directional: direction is from position toward target.
            lightDir = normalize(lights[i].position - lights[i].target);
        } else {
            // Point: direction from surface to light.
            lightDir = normalize(lights[i].position - fragPosition);
        }

        float NdotL = max(dot(norm, lightDir), 0.0);

        // Quantize NdotL into numBands discrete steps.
        // min() guards against NdotL == 1.0 producing an out-of-range index.
        float quantized = min(floor(NdotL * numBands), numBands - 1.0) / (numBands - 1.0);
        lightAccum += quantized * lights[i].color.r;
    }

    lightAccum = clamp(lightAccum, 0.0, 1.0);
    finalColor = vec4(baseColor * lightAccum, texColor.a * colDiffuse.a);
}
