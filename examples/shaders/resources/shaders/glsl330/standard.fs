#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec4 colAmbient;
uniform vec4 colDiffuse;
uniform vec4 colSpecular;
uniform float glossiness;

uniform int useNormal;
uniform int useSpecular;

uniform mat4 modelMatrix;
uniform vec3 viewDir;

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 direction;
    vec4 diffuse;
    float intensity;
    float radius;
    float coneAngle;
};

const int maxLights = 8;
uniform Light lights[maxLights];

vec3 ComputeLightPoint(Light l, vec3 n, vec3 v, float s)
{
    vec3 surfacePos = vec3(modelMatrix*vec4(fragPosition, 1));
    vec3 surfaceToLight = l.position - surfacePos;
    
    // Diffuse shading
    float brightness = clamp(float(dot(n, surfaceToLight)/(length(surfaceToLight)*length(n))), 0.0, 1.0);
    float diff = 1.0/dot(surfaceToLight/l.radius, surfaceToLight/l.radius)*brightness*l.intensity;
    
    // Specular shading
    float spec = 0.0;
    if (diff > 0.0)
    {
        vec3 h = normalize(-l.direction + v);
        spec = pow(abs(dot(n, h)), 3.0 + glossiness)*s;
    }
    
    return (diff*l.diffuse.rgb + spec*colSpecular.rgb);
}

vec3 ComputeLightDirectional(Light l, vec3 n, vec3 v, float s)
{
    vec3 lightDir = normalize(-l.direction);
    
    // Diffuse shading
    float diff = clamp(float(dot(n, lightDir)), 0.0, 1.0)*l.intensity;
    
    // Specular shading
    float spec = 0.0;
    if (diff > 0.0)
    {
        vec3 h = normalize(lightDir + v);
        spec = pow(abs(dot(n, h)), 3.0 + glossiness)*s;
    }
    
    // Combine results
    return (diff*l.intensity*l.diffuse.rgb + spec*colSpecular.rgb);
}

vec3 ComputeLightSpot(Light l, vec3 n, vec3 v, float s)
{
    vec3 surfacePos = vec3(modelMatrix*vec4(fragPosition, 1));
    vec3 lightToSurface = normalize(surfacePos - l.position);
    vec3 lightDir = normalize(-l.direction);
    
    // Diffuse shading
    float diff = clamp(float(dot(n, lightDir)), 0.0, 1.0)*l.intensity;
    
    // Spot attenuation
    float attenuation = clamp(float(dot(n, lightToSurface)), 0.0, 1.0);
    attenuation = dot(lightToSurface, -lightDir);
    
    float lightToSurfaceAngle = degrees(acos(attenuation));
    if (lightToSurfaceAngle > l.coneAngle) attenuation = 0.0;
    
    float falloff = (l.coneAngle - lightToSurfaceAngle)/l.coneAngle;
    
    // Combine diffuse and attenuation
    float diffAttenuation = diff*attenuation;
    
    // Specular shading
    float spec = 0.0;
    if (diffAttenuation > 0.0)
    {
        vec3 h = normalize(lightDir + v);
        spec = pow(abs(dot(n, h)), 3.0 + glossiness)*s;
    }
    
    return (falloff*(diffAttenuation*l.diffuse.rgb + spec*colSpecular.rgb));
}

void main()
{
    // Calculate fragment normal in screen space
    // NOTE: important to multiply model matrix by fragment normal to apply model transformation (rotation and scale)
    mat3 normalMatrix = mat3(modelMatrix);
    vec3 normal = normalize(normalMatrix*fragNormal);

    // Normalize normal and view direction vectors
    vec3 n = normalize(normal);
    vec3 v = normalize(viewDir);

    // Calculate diffuse texture color fetching
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lighting = colAmbient.rgb;
    
    // Calculate normal texture color fetching or set to maximum normal value by default
    if (useNormal == 1)
    {
        n *= texture(texture1, fragTexCoord).rgb;
        n = normalize(n);
    }
    
    // Calculate specular texture color fetching or set to maximum specular value by default
    float spec = 1.0;
    if (useSpecular == 1) spec = texture(texture2, fragTexCoord).r;
    
    for (int i = 0; i < maxLights; i++)
    {
        // Check if light is enabled
        if (lights[i].enabled == 1)
        {
            // Calculate lighting based on light type
            if (lights[i].type == 0) lighting += ComputeLightPoint(lights[i], n, v, spec);
            else if (lights[i].type == 1) lighting += ComputeLightDirectional(lights[i], n, v, spec);
            else if (lights[i].type == 2) lighting += ComputeLightSpot(lights[i], n, v, spec);
        }
    }
    
    // Calculate final fragment color
    finalColor = vec4(texelColor.rgb*lighting*colDiffuse.rgb, texelColor.a*colDiffuse.a);
}
