/*******************************************************************************************
*
*   rPBR [shader] - Irradiance cubemap fragment shader
*
*   Copyright (c) 2017 Victor Fisac
*
**********************************************************************************************/

#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;

// Input uniform values
uniform samplerCube environmentMap;

// Constant values
const float PI = 3.14159265359f;

// Output fragment color
out vec4 finalColor;

void main()
{
    // The sample direction equals the hemisphere's orientation
    vec3 normal = normalize(fragPosition);

    vec3 irradiance = vec3(0.0);  

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up = cross(normal, right);

    float sampleDelta = 0.025f;
    float nrSamples = 0.0f; 

    for (float phi = 0.0; phi < 2.0*PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5*PI; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
            
            // tangent space to world
            vec3 sampleVec = tangentSample.x*right + tangentSample.y*up + tangentSample.z*normal; 

            // Fetch color from environment cubemap
            irradiance += texture(environmentMap, sampleVec).rgb*cos(theta)*sin(theta);
            nrSamples++;
        }
    }

    // Calculate irradiance average value from samples
    irradiance = PI*irradiance*(1.0/float(nrSamples));

    // Calculate final fragment color
    finalColor = vec4(irradiance, 1.0);
}
