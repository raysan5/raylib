#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec3 fragNormal; //used for when normal mapping is toggled off
varying vec4 fragColor;
varying mat3 TBN;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D normalMap;
uniform vec4 colDiffuse;
uniform vec3 viewPos;

// NOTE: Add your custom variables here

uniform vec3 lightPos;
uniform bool useNormalMap;
uniform float specularExponent;

void main()
{
    vec4 texelColor = texture(texture0, vec2(fragTexCoord.x, fragTexCoord.y));
    vec3 specular = vec3(0.0);
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 lightDir = normalize(lightPos - fragPosition);

    vec3 normal;
    if (useNormalMap)
    {
        normal = texture(normalMap, vec2(fragTexCoord.x, fragTexCoord.y)).rgb;

        //Transform normal values to the range -1.0 ... 1.0
        normal = normalize(normal*2.0 - 1.0);

        //Transform the normal from tangent-space to world-space for lighting calculation
        normal = normalize(normal*TBN);
    }
    else
    {
        normal = normalize(fragNormal);
    }

    vec4 tint = colDiffuse*fragColor;

    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 lightDot = lightColor*NdotL;

    float specCo = 0.0;

    if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewDir, reflect(-lightDir, normal))), specularExponent); // 16 refers to shine

    specular += specCo;

    finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(vec4(1.0, 1.0, 1.0, 1.0)/40.0)*tint;

    // Gamma correction
    gl_FragColor = pow(finalColor, vec4(1.0/2.2));
}
