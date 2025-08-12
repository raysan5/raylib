#version 120

#define MAX_BONE_NUM 64

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec4 vertexColor;
attribute vec4 vertexBoneIds;
attribute vec4 vertexBoneWeights;

// Input uniform values
uniform mat4 mvp;
uniform mat4 boneMatrices[MAX_BONE_NUM];

// Output vertex attributes (to fragment shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

void main()
{
    int boneIndex0 = int(vertexBoneIds.x);
    int boneIndex1 = int(vertexBoneIds.y);
    int boneIndex2 = int(vertexBoneIds.z);
    int boneIndex3 = int(vertexBoneIds.w);
    
    // WARNING: OpenGL ES 2.0 does not support automatic matrix transposing, neither transpose() function
    mat4 boneMatrixTransposed0 = mat4(
        vec4(boneMatrices[boneIndex0][0].x, boneMatrices[boneIndex0][1].x, boneMatrices[boneIndex0][2].x, boneMatrices[boneIndex0][3].x),
        vec4(boneMatrices[boneIndex0][0].y, boneMatrices[boneIndex0][1].y, boneMatrices[boneIndex0][2].y, boneMatrices[boneIndex0][3].y),
        vec4(boneMatrices[boneIndex0][0].z, boneMatrices[boneIndex0][1].z, boneMatrices[boneIndex0][2].z, boneMatrices[boneIndex0][3].z),
        vec4(boneMatrices[boneIndex0][0].w, boneMatrices[boneIndex0][1].w, boneMatrices[boneIndex0][2].w, boneMatrices[boneIndex0][3].w));
    mat4 boneMatrixTransposed1 = mat4(
        vec4(boneMatrices[boneIndex1][0].x, boneMatrices[boneIndex1][1].x, boneMatrices[boneIndex1][2].x, boneMatrices[boneIndex1][3].x),
        vec4(boneMatrices[boneIndex1][0].y, boneMatrices[boneIndex1][1].y, boneMatrices[boneIndex1][2].y, boneMatrices[boneIndex1][3].y),
        vec4(boneMatrices[boneIndex1][0].z, boneMatrices[boneIndex1][1].z, boneMatrices[boneIndex1][2].z, boneMatrices[boneIndex1][3].z),
        vec4(boneMatrices[boneIndex1][0].w, boneMatrices[boneIndex1][1].w, boneMatrices[boneIndex1][2].w, boneMatrices[boneIndex1][3].w));
    mat4 boneMatrixTransposed2 = mat4(
        vec4(boneMatrices[boneIndex2][0].x, boneMatrices[boneIndex2][1].x, boneMatrices[boneIndex2][2].x, boneMatrices[boneIndex2][3].x),
        vec4(boneMatrices[boneIndex2][0].y, boneMatrices[boneIndex2][1].y, boneMatrices[boneIndex2][2].y, boneMatrices[boneIndex2][3].y),
        vec4(boneMatrices[boneIndex2][0].z, boneMatrices[boneIndex2][1].z, boneMatrices[boneIndex2][2].z, boneMatrices[boneIndex2][3].z),
        vec4(boneMatrices[boneIndex2][0].w, boneMatrices[boneIndex2][1].w, boneMatrices[boneIndex2][2].w, boneMatrices[boneIndex2][3].w));
    mat4 boneMatrixTransposed3 = mat4(
        vec4(boneMatrices[boneIndex3][0].x, boneMatrices[boneIndex3][1].x, boneMatrices[boneIndex3][2].x, boneMatrices[boneIndex3][3].x),
        vec4(boneMatrices[boneIndex3][0].y, boneMatrices[boneIndex3][1].y, boneMatrices[boneIndex3][2].y, boneMatrices[boneIndex3][3].y),
        vec4(boneMatrices[boneIndex3][0].z, boneMatrices[boneIndex3][1].z, boneMatrices[boneIndex3][2].z, boneMatrices[boneIndex3][3].z),
        vec4(boneMatrices[boneIndex3][0].w, boneMatrices[boneIndex3][1].w, boneMatrices[boneIndex3][2].w, boneMatrices[boneIndex3][3].w));
    
    vec4 skinnedPosition =
        vertexBoneWeights.x*(boneMatrixTransposed0*vec4(vertexPosition, 1.0)) +
        vertexBoneWeights.y*(boneMatrixTransposed1*vec4(vertexPosition, 1.0)) + 
        vertexBoneWeights.z*(boneMatrixTransposed2*vec4(vertexPosition, 1.0)) + 
        vertexBoneWeights.w*(boneMatrixTransposed3*vec4(vertexPosition, 1.0));
    
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp*skinnedPosition;
}