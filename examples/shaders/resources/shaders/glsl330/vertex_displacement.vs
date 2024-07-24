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

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

float perlinNoise(vec2 p);

void main()
{
    float height = perlinNoise(vertexTexCoord +  vec2(time, time) * 0.01);
    vec3 displacedPosition = vertexPosition + vec3(0.0, height*2.0, 0.0);

    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(displacedPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvp*vec4(displacedPosition, 1.0);
}

float perlinNoise(vec2 p) {
    // Implement or call your Perlin noise function here
    // This is a placeholder function
    return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}
