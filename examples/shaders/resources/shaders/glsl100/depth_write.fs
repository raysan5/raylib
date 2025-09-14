#version 100

#extension GL_EXT_frag_depth : enable          
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    gl_FragColor = texelColor*colDiffuse*fragColor;
    gl_FragDepthEXT = 1.0 - gl_FragCoord.z;
}