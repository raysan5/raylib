#version 100

precision mediump float;

// Input values from vertex shader
varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec2 fragTexCoord;

// Input uniform values
uniform sampler2D texture0;
uniform vec3 viewPos;
uniform vec3 lightPositions[8];
uniform vec3 lightColors[8];

void main()
{
    vec4 texColor = texture2D(texture0, fragTexCoord);
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);

    vec3 ambient = 0.05*texColor.rgb;
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    for (int i = 0; i < 8; i++)
    {
        vec3 lightDir = normalize(lightPositions[i] - fragPosition);
        float dist = length(lightPositions[i] - fragPosition);
        float attenuation = 1.0/(1.0 + 0.8*dist + 0.4*dist*dist);

        // Diffuse shading
        float diff = max(dot(norm, lightDir), 0.0);
        totalDiffuse += diff*lightColors[i]*attenuation*1.2;

        // Blinn-Phong specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
        totalSpecular += spec*lightColors[i]*attenuation*0.8;
    }

    vec3 result = ambient + (totalDiffuse*texColor.rgb) + totalSpecular;
    gl_FragColor = vec4(result, texColor.a);
}
