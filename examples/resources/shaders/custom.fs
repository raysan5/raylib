#version 330

uniform sampler2D texture0;
varying vec2 fragTexCoord;

uniform vec4 tintColor;

void main()
{
    vec4 base = texture2D(texture0, fragTexCoord)*tintColor;
    
    // Convert to grayscale using NTSC conversion weights
    float gray = dot(base.rgb, vec3(0.299, 0.587, 0.114));
    
    gl_FragColor = vec4(gray, gray, gray, tintColor.a);
}