#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Input uniform values
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

// Output vertex attributes (to fragment shader)
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
varying vec4 fragColor;


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
    // Calculate vertex attributes for fragment shader
    vec4 worldPos = matModel*vec4(vertexPosition, 1.0);
    fragPosition = worldPos.xyz; 
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    fragNormal = normalMatrix*vertexNormal;

    // Calculate final vertex position
    gl_Position = matProjection*matView*worldPos;
}
