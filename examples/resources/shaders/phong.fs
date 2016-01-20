#version 330

// Vertex shader input data
in vec2 fragTexCoord;
in vec3 fragNormal;

// Diffuse data
uniform sampler2D texture0;
uniform vec4 fragTintColor;

// Light attributes
uniform vec3 light_ambientColor = vec3(0.6, 0.3, 0.0);
uniform vec3 light_diffuseColor = vec3(1.0, 0.5, 0.0);
uniform vec3 light_specularColor = vec3(0.0, 1.0, 0.0);
uniform float light_intensity = 1.0;
uniform float light_specIntensity = 1.0;

// Material attributes
uniform vec3 mat_ambientColor = vec3(1.0, 1.0, 1.0);
uniform vec3 mat_specularColor = vec3(1.0, 1.0, 1.0);
uniform float mat_glossiness = 50.0;

// World attributes
uniform vec3 lightPos;
uniform vec3 cameraPos;

// Fragment shader output data
out vec4 fragColor;

vec3 AmbientLighting()
{
   return (mat_ambientColor*light_ambientColor);
}

vec3 DiffuseLighting(in vec3 N, in vec3 L)
{
   // Lambertian reflection calculation
   float diffuse = clamp(dot(N, L), 0, 1);
   
   return (fragTintColor.xyz*light_diffuseColor*light_intensity*diffuse);
}

vec3 SpecularLighting(in vec3 N, in vec3 L, in vec3 V)
{
   float specular = 0.0;

   // Calculate specular reflection only if the surface is oriented to the light source
   if (dot(N, L) > 0)
   {
      // Calculate half vector
      vec3 H = normalize(L + V);
      
      // Calculate specular intensity
      specular = pow(dot(N, H), 3 + mat_glossiness);
   }
   
   return (mat_specularColor*light_specularColor*light_specIntensity*specular);
}

void main()
{
    // Normalize input vectors
    vec3 L = normalize(lightPos);
    vec3 V = normalize(cameraPos);
    vec3 N = normalize(fragNormal);
    
    vec3 ambient = AmbientLighting();
    vec3 diffuse = DiffuseLighting(N, L);
    vec3 specular = SpecularLighting(N, L, V);
    
    // Get base color from texture
    vec4 textureColor = texture(texture0, fragTexCoord);
    vec3 finalColor = textureColor.rgb;
    
    fragColor = vec4(finalColor * (ambient + diffuse + specular), textureColor.a);
}