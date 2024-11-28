#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
//out vec4 finalColor;

// NOTE: Add here your custom variables

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    gl_FragColor = texelColor*colDiffuse*fragColor;
	gl_FragDepth = gl_FragCoord.z;
}