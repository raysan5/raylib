/*******************************************************************************************
*
*   rPBR [shader] - Physically based rendering fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330

#define     MAX_REFLECTION_LOD      4.0
#define     MAX_DEPTH_LAYER         20
#define     MIN_DEPTH_LAYER         10

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct MaterialProperty {
    vec3 color;
    int useSampler;
    sampler2D sampler;
};

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBinormal;

// Input material values
uniform MaterialProperty albedo;
uniform MaterialProperty normals;
uniform MaterialProperty metalness;
uniform MaterialProperty roughness;
uniform MaterialProperty occlusion;
uniform MaterialProperty emission;
uniform MaterialProperty height;

// Input uniform values
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// Input lighting values
uniform Light lights[MAX_LIGHTS];

// Other uniform values
uniform int renderMode;
uniform vec3 viewPos;
vec2 texCoord;

// Constant values
const float PI = 3.14159265359;

// Output fragment color
out vec4 finalColor;

vec3 ComputeMaterialProperty(MaterialProperty property);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

vec3 ComputeMaterialProperty(MaterialProperty property)
{
    vec3 result = vec3(0.0, 0.0, 0.0);

    if (property.useSampler == 1) result = texture(property.sampler, texCoord).rgb;
    else result = property.color;

    return result;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom = a2;
    float denom = (NdotH2*(a2 - 1.0) + 1.0);
    denom = PI*denom*denom;

    return nom/denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = r*r/8.0;

    float nom = NdotV;
    float denom = NdotV*(1.0 - k) + k;

    return nom/denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1*ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0)*pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)*pow(1.0 - cosTheta, 5.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    // Calculate the number of depth layers and calculate the size of each layer
    float numLayers = mix(MAX_DEPTH_LAYER, MIN_DEPTH_LAYER, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    float layerDepth = 1.0/numLayers;

    // Calculate depth of current layer
    float currentLayerDepth = 0.0;

    // Calculate the amount to shift the texture coordinates per layer (from vector P)
    // Note: height amount is stored in height material attribute color R channel (sampler use is independent)
    vec2 P = viewDir.xy*height.color.r; 
    vec2 deltaTexCoords = P/numLayers;

    // Store initial texture coordinates and depth values
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(height.sampler, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        // Shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;

        // Get depth map value at current texture coordinates
        currentDepthMapValue = texture(height.sampler, currentTexCoords).r;

        // Get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    // Get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // Get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(height.sampler, prevTexCoords).r - currentLayerDepth + layerDepth;

    // Interpolation of texture coordinates
    float weight = afterDepth/(afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords*weight + currentTexCoords*(1.0 - weight);

    return finalTexCoords;
}

void main()
{
    // Calculate TBN and RM matrices
    mat3 TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));

    // Calculate lighting required attributes
    vec3 normal = normalize(fragNormal);
    vec3 view = normalize(viewPos - fragPosition);
    vec3 refl = reflect(-view, normal);

    // Check if parallax mapping is enabled and calculate texture coordinates to use based on height map
    // NOTE: remember that 'texCoord' variable must be assigned before calling any ComputeMaterialProperty() function
    if (height.useSampler == 1) texCoord = ParallaxMapping(fragTexCoord, view);
    else texCoord = fragTexCoord;   // Use default texture coordinates

    // Fetch material values from texture sampler or color attributes
    vec3 color = ComputeMaterialProperty(albedo);
    vec3 metal = ComputeMaterialProperty(metalness);
    vec3 rough = ComputeMaterialProperty(roughness);
    vec3 emiss = ComputeMaterialProperty(emission);
    vec3 ao = ComputeMaterialProperty(occlusion);

    // Check if normal mapping is enabled
    if (normals.useSampler == 1)
    {
        // Fetch normal map color and transform lighting values to tangent space
        normal = ComputeMaterialProperty(normals);
        normal = normalize(normal*2.0 - 1.0);
        normal = normalize(normal*TBN);

        // Convert tangent space normal to world space due to cubemap reflection calculations
        refl = normalize(reflect(-view, normal));
    }

    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, color, metal.r);

    // Calculate lighting for all lights
    vec3 Lo = vec3(0.0);
    vec3 lightDot = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            // Calculate per-light radiance
            vec3 light = vec3(0.0);
            vec3 radiance = lights[i].color.rgb;
            if (lights[i].type == LIGHT_DIRECTIONAL) light = -normalize(lights[i].target - lights[i].position);
            else if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
                float distance = length(lights[i].position - fragPosition);
                float attenuation = 1.0/(distance*distance);
                radiance *= attenuation;
            }

            // Cook-torrance BRDF
            vec3 high = normalize(view + light);
            float NDF = DistributionGGX(normal, high, rough.r);
            float G = GeometrySmith(normal, view, light, rough.r);
            vec3 F = fresnelSchlick(max(dot(high, view), 0.0), F0);
            vec3 nominator = NDF*G*F;
            float denominator = 4*max(dot(normal, view), 0.0)*max(dot(normal, light), 0.0) + 0.001;
            vec3 brdf = nominator/denominator;

            // Store to kS the fresnel value and calculate energy conservation
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;

            // Multiply kD by the inverse metalness such that only non-metals have diffuse lighting
            kD *= 1.0 - metal.r;

            // Scale light by dot product between normal and light direction
            float NdotL = max(dot(normal, light), 0.0);

            // Add to outgoing radiance Lo
            // Note: BRDF is already multiplied by the Fresnel so it doesn't need to be multiplied again
            Lo += (kD*color/PI + brdf)*radiance*NdotL*lights[i].color.a;
            lightDot += radiance*NdotL + brdf*lights[i].color.a;
        }
    }

    // Calculate ambient lighting using IBL
    vec3 F = fresnelSchlickRoughness(max(dot(normal, view), 0.0), F0, rough.r);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metal.r;

    // Calculate indirect diffuse
    vec3 irradiance = texture(irradianceMap, fragNormal).rgb;
    vec3 diffuse = color*irradiance;

    // Sample both the prefilter map and the BRDF lut and combine them together as per the Split-Sum approximation
    vec3 prefilterColor = textureLod(prefilterMap, refl, rough.r*MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, view), 0.0), rough.r)).rg;
    vec3 reflection = prefilterColor*(F*brdf.x + brdf.y);

    // Calculate final lighting
    vec3 ambient = (kD*diffuse + reflection)*ao;

    // Calculate fragment color based on render mode
    vec3 fragmentColor = ambient + Lo + emiss;                              // Physically Based Rendering

    if (renderMode == 1) fragmentColor = color;                             // Albedo
    else if (renderMode == 2) fragmentColor = normal;                       // Normals
    else if (renderMode == 3) fragmentColor = metal;                        // Metalness
    else if (renderMode == 4) fragmentColor = rough;                        // Roughness
    else if (renderMode == 5) fragmentColor = ao;                           // Ambient Occlusion
    else if (renderMode == 6) fragmentColor = emiss;                        // Emission
    else if (renderMode == 7) fragmentColor = lightDot;                     // Lighting
    else if (renderMode == 8) fragmentColor = kS;                           // Fresnel
    else if (renderMode == 9) fragmentColor = irradiance;                   // Irradiance
    else if (renderMode == 10) fragmentColor = reflection;                  // Reflection

    // Apply HDR tonemapping
    fragmentColor = fragmentColor/(fragmentColor + vec3(1.0));

    // Apply gamma correction
    fragmentColor = pow(fragmentColor, vec3(1.0/2.2));

    // Calculate final fragment color
    finalColor = vec4(fragmentColor, 1.0);
}
