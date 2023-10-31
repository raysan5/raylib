#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

void main() {
    // store the fragment position vector in the first gbuffer texture
    gPosition = fragPosition;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(fragNormal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(diffuseTexture, fragTexCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(specularTexture, fragTexCoord).r;
}
