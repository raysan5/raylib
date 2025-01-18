#version 300 es

precision highp float;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main() {
    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(fragPosition,1.0);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(fragNormal),1.0);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(diffuseTexture, fragTexCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(specularTexture, fragTexCoord).r;
}
