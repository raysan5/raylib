/*******************************************************************************************
*
*   rPBR [shader] - Bidirectional reflectance distribution function fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330
#define         MAX_SAMPLES        1024u

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Constant values
const float PI = 3.14159265359;

// Output fragment color
out vec4 finalColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec2 IntegrateBRDF(float NdotV, float roughness);

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2*(a2 - 1.0) + 1.0);
    denom = PI*denom*denom;

    return nom/denom;
}

float RadicalInverse_VdC(uint bits)
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

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
    vec3 V = vec3(sqrt(1.0 - NdotV*NdotV), 0.0, NdotV);
    float A = 0.0;
    float B = 0.0; 
    vec3 N = vec3(0.0, 0.0, 1.0);

    for(uint i = 0u; i < MAX_SAMPLES; i++)
    {
        // Generate a sample vector that's biased towards the preferred alignment direction (importance sampling)
        vec2 Xi = Hammersley(i, MAX_SAMPLES);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0*dot(V, H)*H - V);
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G*VdotH)/(NdotH*NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc)*G_Vis;
            B += Fc*G_Vis;
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
