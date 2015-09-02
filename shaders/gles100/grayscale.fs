#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec4 tintColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 base = texture2D(texture0, fragTexCoord)*tintColor;
    
    // Convert to grayscale using NTSC conversion weights
    float gray = dot(base.rgb, vec3(0.299, 0.587, 0.114));
    
    gl_FragColor = vec4(gray, gray, gray, tintColor.a);
}