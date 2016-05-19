#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 fragTintColor;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

// Light uniform values
uniform vec3 lightAmbientColor = vec3(0.6, 0.3, 0.0);
uniform vec3 lightDiffuseColor = vec3(1.0, 0.5, 0.0);
uniform vec3 lightSpecularColor = vec3(0.0, 1.0, 0.0);
uniform float lightIntensity = 1.0;
uniform float lightSpecIntensity = 1.0;

// Material uniform values
uniform vec3 matAmbientColor = vec3(1.0, 1.0, 1.0);
uniform vec3 matSpecularColor = vec3(1.0, 1.0, 1.0);
uniform float matGlossiness = 50.0;

// World uniform values
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

// Fragment shader output data
out vec4 fragColor;

// Calculate ambient lighting component
vec3 AmbientLighting()
{
    return (matAmbientColor*lightAmbientColor);
}

// Calculate diffuse lighting component
vec3 DiffuseLighting(in vec3 N, in vec3 L)
{
    // Lambertian reflection calculation
    float diffuse = clamp(dot(N, L), 0, 1);

    return (fragTintColor.xyz*lightDiffuseColor*lightIntensity*diffuse);
}

// Calculate specular lighting component
vec3 SpecularLighting(in vec3 N, in vec3 L, in vec3 V)
{
    float specular = 0.0;

    // Calculate specular reflection only if the surface is oriented to the light source
    if (dot(N, L) > 0)
    {
        // Calculate half vector
        vec3 H = normalize(L + V);

        // Calculate specular intensity
        specular = pow(dot(N, H), 3 + matGlossiness);
    }
   
    return (matSpecularColor*lightSpecularColor*lightSpecIntensity*specular);
}

void main()
{
    // Normalize input vectors
    vec3 L = normalize(lightPosition);
    vec3 V = normalize(cameraPosition);
    vec3 N = normalize(fragNormal);
    
    // Calculate lighting components 
    vec3 ambient = AmbientLighting();
    vec3 diffuse = DiffuseLighting(N, L);
    vec3 specular = SpecularLighting(N, L, V);
    
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Calculate final fragment color
    finalColor = vec4(texelColor.rgb*(ambient + diffuse + specular), texelColor.a);
}