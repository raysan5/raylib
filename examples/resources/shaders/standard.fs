#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

out vec4 finalColor;

uniform sampler2D texture0;

uniform vec4 colAmbient;
uniform vec4 colDiffuse;
uniform vec4 colSpecular;
uniform float glossiness;

uniform mat4 modelMatrix;
uniform vec3 viewDir;

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 direction;
    vec4 diffuse;
    float intensity;
    float attenuation;
    float coneAngle;
};

const int maxLights = 8;
uniform int lightsCount;
uniform Light lights[maxLights];

vec3 CalcPointLight(Light l, vec3 n, vec3 v)
{
    vec3 surfacePos = vec3(modelMatrix*vec4(fragPosition, 1));
    vec3 surfaceToLight = l.position - surfacePos;
    
    // Diffuse shading
    float brightness = clamp(dot(n, surfaceToLight)/(length(surfaceToLight)*length(n)), 0, 1);
    float diff = 1.0/dot(surfaceToLight/l.attenuation, surfaceToLight/l.attenuation)*brightness*l.intensity;
    
    // Specular shading
    float spec = 0.0;
    if(diff > 0.0)
    {
        vec3 h = normalize(-l.direction + v);
        spec = pow(dot(n, h), 3 + glossiness);
    }
    
    return (diff*l.diffuse.rgb*colDiffuse.rgb + spec*colSpecular.rgb);
}

vec3 CalcDirectionalLight(Light l, vec3 n, vec3 v)
{
    vec3 lightDir = normalize(-l.direction);
    
    // Diffuse shading
    float diff = clamp(dot(n, lightDir), 0.0, 1.0)*l.intensity;
    
    // Specular shading
    float spec = 0.0;
    if(diff > 0.0)
    {
        vec3 h = normalize(lightDir + v);
        spec = pow(dot(n, h), 3 + glossiness);
    }
    
    // Combine results
    return (diff*l.intensity*l.diffuse.rgb*colDiffuse.rgb + spec*colSpecular.rgb);
}

vec3 CalcSpotLight(Light l, vec3 n, vec3 v)
{
    vec3 surfacePos = vec3(modelMatrix*vec4(fragPosition, 1));
    vec3 lightToSurface = normalize(surfacePos - l.position);
    vec3 lightDir = normalize(-l.direction);
    
    // Diffuse shading
    float diff = clamp(dot(n, lightDir), 0.0, 1.0)*l.intensity;
    
    // Spot attenuation
    float attenuation = clamp(dot(n, lightToSurface), 0.0, 1.0);
    attenuation = dot(lightToSurface, -lightDir);
    float lightToSurfaceAngle = degrees(acos(attenuation));
    if(lightToSurfaceAngle > l.coneAngle) attenuation = 0.0;
    float falloff = (l.coneAngle - lightToSurfaceAngle)/l.coneAngle;
    
    // Combine diffuse and attenuation
    float diffAttenuation = diff*attenuation;
    
    // Specular shading
    float spec = 0.0;
    if(diffAttenuation > 0.0)
    {
        vec3 h = normalize(lightDir + v);
        spec = pow(dot(n, h), 3 + glossiness);
    }
    
    return falloff*(diffAttenuation*l.diffuse.rgb + spec*colSpecular.rgb);
}

void main()
{
    // Calculate fragment normal in screen space
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 normal = normalize(normalMatrix*fragNormal);
   
    // Normalize normal and view direction vectors
    vec3 n = normalize(normal);
    vec3 v = normalize(viewDir);

    // Calculate diffuse texture color fetching
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lighting = colAmbient.rgb;
    
    for(int i = 0; i < lightsCount; i++)
    {
        // Check if light is enabled
        if(lights[i].enabled == 1)
        {
            // Calculate lighting based on light type
            switch(lights[i].type)
            {
                case 0: lighting += CalcPointLight(lights[i], n, v); break;
                case 1: lighting += CalcDirectionalLight(lights[i], n, v); break;
                case 2: lighting += CalcSpotLight(lights[i], n, v); break;
                default: break;
            }
        }
    }
    
    // Calculate final fragment color
    finalColor = vec4(texelColor.rgb*lighting, texelColor.a);
}
