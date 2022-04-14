#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables
uniform vec2 leftLensCenter = vec2(0.288, 0.5);
uniform vec2 rightLensCenter = vec2(0.712, 0.5);
uniform vec2 leftScreenCenter = vec2(0.25, 0.5);
uniform vec2 rightScreenCenter = vec2(0.75, 0.5);
uniform vec2 scale = vec2(0.25, 0.45);
uniform vec2 scaleIn = vec2(4, 2.2222);
uniform vec4 deviceWarpParam = vec4(1, 0.22, 0.24, 0);
uniform vec4 chromaAbParam = vec4(0.996, -0.004, 1.014, 0.0);

void main()
{
    // Compute lens distortion
    vec2 lensCenter = fragTexCoord.x < 0.5? leftLensCenter : rightLensCenter;
    vec2 screenCenter = fragTexCoord.x < 0.5? leftScreenCenter : rightScreenCenter;
    vec2 theta = (fragTexCoord - lensCenter)*scaleIn;
    float rSq = theta.x*theta.x + theta.y*theta.y;
    vec2 theta1 = theta*(deviceWarpParam.x + deviceWarpParam.y*rSq + deviceWarpParam.z*rSq*rSq + deviceWarpParam.w*rSq*rSq*rSq);
    vec2 thetaBlue = theta1*(chromaAbParam.z + chromaAbParam.w*rSq);
    vec2 tcBlue = lensCenter + scale*thetaBlue;

    if (any(bvec2(clamp(tcBlue, screenCenter - vec2(0.25, 0.5), screenCenter + vec2(0.25, 0.5)) - tcBlue)))
    {
        // Set black fragment for everything outside the lens border
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        // Compute color chroma aberration
        float blue = texture(texture0, tcBlue).b;
        vec2 tcGreen = lensCenter + scale*theta1;
        float green = texture(texture0, tcGreen).g;

        vec2 thetaRed = theta1*(chromaAbParam.x + chromaAbParam.y*rSq);
        vec2 tcRed = lensCenter + scale*thetaRed;

        float red = texture(texture0, tcRed).r;
        finalColor = vec4(red, green, blue, 1.0);
    }
}
