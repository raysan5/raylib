/*******************************************************************************************
*
*   rPBR [shader] - Prefiltered environment for reflections fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330
#define     MAX_SAMPLES             1024u
#define     CUBEMAP_RESOLUTION      1024.0

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;
uniform float roughness;

// Constant values
const float PI = 3.14159265359f;

// Output fragment color
out vec4 finalColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float RadicalInverse_VdC(uint bits);
vec2 Hammersley(uint i, uint N);
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

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

void main()
{
    // Make the simplyfying assumption that V equals R equals the normal 
    vec3 N = normalize(fragPosition);
    vec3 R = N;
    vec3 V = R;

    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < MAX_SAMPLES; i++)
    {
        // Generate a sample vector that's biased towards the preferred alignment direction (importance sampling)
        vec2 Xi = Hammersley(i, MAX_SAMPLES);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0*dot(V, H)*H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // Sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D*NdotH/(4.0*HdotV) + 0.0001; 

            float resolution = CUBEMAP_RESOLUTION;
            float saTexel  = 4.0*PI/(6.0*resolution*resolution);
            float saSample = 1.0/(float(MAX_SAMPLES)*pdf + 0.0001);
            float mipLevel = ((roughness == 0.0) ? 0.0 : 0.5*log2(saSample/saTexel)); 

            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb*NdotL;
            totalWeight += NdotL;
        }
    }

    // Calculate prefilter average color
    prefilteredColor = prefilteredColor/totalWeight;

    // Calculate final fragment color
    finalColor = vec4(prefilteredColor, 1.0);
}
