#version 100

#extension GL_EXT_frag_depth : enable   // Extension required for writing depth         
precision mediump float;                // Precision required for OpenGL ES2 (WebGL)

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord);

    gl_FragColor = texelColor*colDiffuse*fragColor;
    gl_FragDepthEXT = gl_FragCoord.z;
}