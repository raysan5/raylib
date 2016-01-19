#version 100

precision mediump float;

varying vec2 fragTexCoord;
uniform sampler2D texture0;
uniform vec4 fragTintColor;

// NOTE: Add here your custom variables

const float renderWidth = 1280;
const float renderHeight = 720;

float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

void main() 
{ 
    vec3 tc = texture2D(texture0, fragTexCoord).rgb*weight[0];

    for (int i = 1; i < 3; i++) 
    {
        tc += texture2D(texture0, fragTexCoord + vec2(offset[i])/renderWidth, 0.0).rgb*weight[i];
        tc += texture2D(texture0, fragTexCoord - vec2(offset[i])/renderWidth, 0.0).rgb*weight[i];
    }

    gl_FragColor = vec4(tc, 1.0);
}