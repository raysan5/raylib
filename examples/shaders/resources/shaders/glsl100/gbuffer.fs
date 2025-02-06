#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec4 fragColor;

// TODO: Is there some alternative for GLSL100
//layout (location = 0) out vec3 gPosition;
//layout (location = 1) out vec3 gNormal;
//layout (location = 2) out vec4 gAlbedoSpec;
//uniform vec3 gPosition;
//uniform vec3 gNormal;
//uniform vec4 gAlbedoSpec;

// Input uniform values
uniform sampler2D texture0;  // Diffuse texture
uniform sampler2D specularTexture;

void main()
{
    // Store the fragment position vector in the first gbuffer texture
    //gPosition = fragPosition;
    
    // Store the per-fragment normals into the gbuffer
    //gNormal = normalize(fragNormal);
    
    // Store the diffuse per-fragment color
    gl_FragColor.rgb = texture2D(texture0, fragTexCoord).rgb;
    
    // Store specular intensity in gAlbedoSpec's alpha component
    gl_FragColor.a = texture2D(specularTexture, fragTexCoord).r;
}
