/*******************************************************************************************
*
*   BRDF LUT Generation - Bidirectional reflectance distribution function fragment shader
*
*   REF: https://github.com/HectorMF/BRDFGenerator
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Constant values
const float PI = 3.14159265359;
const uint MAX_SAMPLES = 1024u;

// Output fragment color
out vec4 finalColor;

vec2 Hammersley(uint i, uint N);
float RadicalInverseVdC(uint bits);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
vec2 IntegrateBRDF(float NdotV, float roughness);

float RadicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Compute Hammersley coordinates
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverseVdC(i));
}

// Integrate number of importance samples for (roughness and NoV)
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y)/(1.0 + (a*a - 1.0)*Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

	// Transform from spherical coordinates to cartesian coordinates (halfway vector)
	vec3 H = vec3(cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta);

	// Transform from tangent space H vector to world space sample vector
	vec3 up = ((abs(N.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0));
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	vec3 sampleVec = tangent*H.x + bitangent*H.y + N*H.z;

	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // For IBL k is calculated different
    float k = (roughness*roughness)/2.0;

    float nom = NdotV;
    float denom = NdotV*(1.0 - k) + k;

    return nom/denom;
}

// Compute the geometry term for the BRDF given roughness squared, NoV, NoL
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1*ggx2;
}

vec2 IntegrateBRDF(float NdotV, float roughness)
{
    float A = 0.0;
    float B = 0.0;
    vec3 V = vec3(sqrt(1.0 - NdotV*NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);

    for (uint i = 0u; i < MAX_SAMPLES; i++)
    {
        // Generate a sample vector that's biased towards the preferred alignment direction (importance sampling)
        
        vec2 Xi = Hammersley(i, MAX_SAMPLES);       // Compute a Hammersely coordinate
        vec3 H = ImportanceSampleGGX(Xi, N, roughness); // Integrate number of importance samples for (roughness and NoV)
        vec3 L = normalize(2.0*dot(V, H)*H - V);    // Compute reflection vector L
        
        float NdotL = max(L.z, 0.0);                // Compute normal dot light
        float NdotH = max(H.z, 0.0);                // Compute normal dot half
        float VdotH = max(dot(V, H), 0.0);          // Compute view dot half

        if (NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);    // Compute the geometry term for the BRDF given roughness squared, NoV, NoL
            float GVis = (G*VdotH)/(NdotH*NdotV);   // Compute the visibility term given G, VoH, NoH, NoV, NoL
            float Fc = pow(1.0 - VdotH, 5.0);       // Compute the fresnel term given VoH

            A += (1.0 - Fc)*GVis;                   // Sum the result given fresnel, geometry, visibility
            B += Fc*GVis;
        }
    }

    // Calculate brdf average sample
    A /= float(MAX_SAMPLES);
    B /= float(MAX_SAMPLES);

    return vec2(A, B);
}

void main()
{
    // Calculate brdf based on texture coordinates
    vec2 brdf = IntegrateBRDF(fragTexCoord.x, fragTexCoord.y);

    // Calculate final fragment color
    finalColor = vec4(brdf.r, brdf.g, 0.0, 1.0);
}
