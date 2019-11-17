#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables
uniform vec2 leftLensCenter;
uniform vec2 rightLensCenter;
uniform vec2 leftScreenCenter;
uniform vec2 rightScreenCenter;
uniform vec2 scale;
uniform vec2 scaleIn;
uniform vec4 hmdWarpParam;
uniform vec4 chromaAbParam;

void main()
{
    // Compute lens distortion
    vec2 lensCenter = fragTexCoord.x < 0.5? leftLensCenter : rightLensCenter;
    vec2 screenCenter = fragTexCoord.x < 0.5? leftScreenCenter : rightScreenCenter;
    vec2 theta = (fragTexCoord - lensCenter)*scaleIn;
    float rSq = theta.x*theta.x + theta.y*theta.y;
    vec2 theta1 = theta*(hmdWarpParam.x + hmdWarpParam.y*rSq + hmdWarpParam.z*rSq*rSq + hmdWarpParam.w*rSq*rSq*rSq);
    vec2 thetaBlue = theta1*(chromaAbParam.z + chromaAbParam.w*rSq);
    vec2 tcBlue = lensCenter + scale*thetaBlue;

    if (any(bvec2(clamp(tcBlue, screenCenter - vec2(0.25, 0.5), screenCenter + vec2(0.25, 0.5)) - tcBlue)))
    {
        // Set black fragment for everything outside the lens border
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        // Compute color chroma aberration
        float blue = texture2D(texture0, tcBlue).b;
        vec2 tcGreen = lensCenter + scale*theta1;
        float green = texture2D(texture0, tcGreen).g;

        vec2 thetaRed = theta1*(chromaAbParam.x + chromaAbParam.y*rSq);
        vec2 tcRed = lensCenter + scale*thetaRed;

        float red = texture2D(texture0, tcRed).r;
        gl_FragColor = vec4(red, green, blue, 1.0);
    }
}
