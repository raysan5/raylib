#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

uniform float time; 

uniform sampler2D perlinNoiseMap;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
out float height;

void main()
{
    // Calculate animated texture coordinates based on time and vertex position
    vec2 animatedTexCoord = sin(vertexTexCoord + vec2(sin(time + vertexPosition.x * 0.1), cos(time + vertexPosition.z * 0.1)) * 0.3);

    // Normalize animated texture coordinates to range [0, 1]
    animatedTexCoord = animatedTexCoord * 0.5 + 0.5;

    // Fetch displacement from the perlin noise map
    float displacement = texture(perlinNoiseMap, animatedTexCoord).r * 7; // Amplified displacement

    // Displace vertex position
    vec3 displacedPosition = vertexPosition + vec3(0.0, displacement, 0.0);

    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(displacedPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
    height = displacedPosition.y * 0.2; // send height to fragment shader for coloring

    // Calculate final vertex position
    gl_Position = mvp*vec4(displacedPosition , 1.0);
}
