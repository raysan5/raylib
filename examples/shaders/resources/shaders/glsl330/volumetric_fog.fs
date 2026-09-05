#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int fogEnabled;
uniform float fogDensity;

// Output fragment color
out vec4 finalColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

const vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0);

// Fog Parameters
#define FOG_STEPS 32    // Number of raymarching steps
#define FOG_HEIGHT_FALLOFF 0.5 // Controls how fast fog thins with height

// Exponantial decay in y
float getFogDensity(vec3 pos) {
    return fogDensity * exp(-pos.y * FOG_HEIGHT_FALLOFF);
}

// Raymarch from camera to surface to compute integrated volumetric fog
vec4 calculateVolumetricFog(vec3 rayStart, vec3 rayEnd, vec3 surfaceColor) {
    vec3 rayDir = rayEnd - rayStart;
    vec3 rayDirNor = normalize(rayDir);
    float rayLength = length(rayDir);
    float stepSize = rayLength / float(FOG_STEPS);
    vec3 stepVec = rayDirNor * stepSize;
    vec3 currentPos = rayStart;

    float totalAccumulatedFog = 0.0;
    vec3 inScatteredLight = vec3(0.0);

    for (int i = 0; i < FOG_STEPS; i++) {
        float currentDensity = getFogDensity(currentPos);
        totalAccumulatedFog += currentDensity * stepSize;

        // Light direction from current fog particle to light source
        vec3 lightDir = normalize(lightPos - currentPos);
        float lightScattering = max(0.0, dot(rayDirNor, lightDir));
        float phase = pow(lightScattering, 8.0);

        // Accumulate light bouncing off fog particles toward camera
        inScatteredLight += lightColor.rgb * phase * currentDensity * stepSize;
        currentPos += stepVec;
    }
    float transmittance = exp(-totalAccumulatedFog);
    vec3 finalRGB = surfaceColor * transmittance + inScatteredLight;

    return vec4(finalRGB, 1.0);
}

void main()
{
    // Surface lighting
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 lightDir = normalize(lightPos - fragPosition);
    vec4 tint = colDiffuse * fragColor;

    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 lightDot = lightColor.rgb * NdotL;

    float specCo = 0.0;
    if (NdotL > 0.0) {
        specCo = pow(max(0.0, dot(viewD, reflect(-lightDir, normal))), 16.0);
    }
    vec3 specular = vec3(specCo);

    vec3 surfaceColor = (texelColor * ((tint + vec4(specular, 1.0)) * vec4(lightDot, 1.0))).rgb;
    surfaceColor += (texelColor * tint * ambient).rgb;

    // Add fog to surface
    vec4 foggedColor;
    if (fogEnabled == 1) {
        foggedColor = calculateVolumetricFog(viewPos, fragPosition, surfaceColor);

        finalColor = pow(foggedColor, vec4(1.0 / 2.2));
    } else {
        finalColor = vec4(surfaceColor, 1.0);
    }
}
