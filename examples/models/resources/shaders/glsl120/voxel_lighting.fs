#version 120
// Input from vertex shader
varying vec3 fragPosition;
varying vec4 fragColor;
varying vec3 fragNormal;

// Uniforms
uniform vec4 colDiffuse;
uniform vec4 ambient;
uniform vec3 viewPos;

#define MAX_LIGHTS 4
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

uniform Light lights[MAX_LIGHTS];

void main()
{
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
                light = -normalize(lights[i].target - lights[i].position);

            if (lights[i].type == LIGHT_POINT)
                light = normalize(lights[i].position - fragPosition);

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL;

            if (NdotL > 0.0)
            {
                float specCo = pow(max(0.0, dot(viewD, reflect(-light, normal))), 16.0);
                specular += specCo;
            }
        }
    }

    vec4 finalColor = (fragColor*((colDiffuse + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += fragColor*(ambient/10.0)*colDiffuse;

    finalColor = pow(finalColor, vec4(1.0/2.2)); // gamma correction

    gl_FragColor = finalColor;
}
