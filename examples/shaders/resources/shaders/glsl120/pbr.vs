#version 120

// Input vertex attributes
attribute  vec3 vertexPosition;
attribute  vec2 vertexTexCoord;
attribute  vec3 vertexNormal;
attribute  vec3 vertexTangent;
attribute  vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform vec3 lightPos;
uniform vec4 difColor;

// Output vertex attributes (to fragment shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;
varying mat3 TBN;

const float normalOffset = 0.1;

// https://github.com/glslify/glsl-inverse
mat3 inverse(mat3 m)
{
  float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
  float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
  float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

  float b01 = a22*a11 - a12*a21;
  float b11 = -a22*a10 + a12*a20;
  float b21 = a21*a10 - a11*a20;

  float det = a00*b01 + a01*b11 + a02*b21;

  return mat3(b01, (-a22*a01 + a02*a21), (a12*a01 - a02*a11),
              b11, (a22*a00 - a02*a20), (-a12*a00 + a02*a10),
              b21, (-a21*a00 + a01*a20), (a11*a00 - a01*a10))/det;
}

// https://github.com/glslify/glsl-transpose
mat3 transpose(mat3 m)
{
  return mat3(m[0][0], m[1][0], m[2][0],
              m[0][1], m[1][1], m[2][1],
              m[0][2], m[1][2], m[2][2]);
}

void main()
{

    // calc binormal from vertex normal and tangent
    vec3 vertexBinormal = cross(vertexNormal, vertexTangent);
    // calc fragment normal based on normal transformations
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    // calc fragment position based on model transformations

    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));

    fragTexCoord = vertexTexCoord*2.0;

    fragNormal = normalize(normalMatrix*vertexNormal);
    vec3 fragTangent = normalize(normalMatrix*vertexTangent);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal)*fragNormal);
    vec3 fragBinormal = normalize(normalMatrix*vertexBinormal);
    fragBinormal = cross(fragNormal, fragTangent);

    TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}