#version 330 core
out vec4 finalColor;

in vec2 texCoord;
in vec2 texCoord2;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    int enabled;
    int type;       // Unused in this demo
    vec3 position;
    vec3 target;    // Unused in this demo
    vec4 color;
};

const int NR_LIGHTS = 4;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPosition;

const float QUADRATIC = 0.032;
const float LINEAR = 0.09;

void main() {
    vec3 fragPosition = texture(gPosition, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 albedo = texture(gAlbedoSpec, texCoord).rgb;
    float specular = texture(gAlbedoSpec, texCoord).a;

    vec3 ambient = albedo*vec3(0.1f);
    vec3 viewDirection = normalize(viewPosition - fragPosition);

    for (int i = 0; i < NR_LIGHTS; ++i)
    {
        if (lights[i].enabled == 0) continue;
        vec3 lightDirection = lights[i].position - fragPosition;
        vec3 diffuse = max(dot(normal, lightDirection), 0.0)*albedo*lights[i].color.xyz;

        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        float spec = pow(max(dot(normal, halfwayDirection), 0.0), 32.0);
        vec3 specular = specular*spec*lights[i].color.xyz;

        // Attenuation
        float distance = length(lights[i].position - fragPosition);
        float attenuation = 1.0/(1.0 + LINEAR*distance + QUADRATIC*distance*distance);
        diffuse *= attenuation;
        specular *= attenuation;
        ambient += diffuse + specular;
    }

    finalColor = vec4(ambient, 1.0);
}

