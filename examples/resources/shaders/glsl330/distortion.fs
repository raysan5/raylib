#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables
const vec2 LeftLensCenter = vec2(0.2863248, 0.5);
const vec2 RightLensCenter = vec2(0.7136753, 0.5);
const vec2 LeftScreenCenter = vec2(0.25, 0.5);
const vec2 RightScreenCenter = vec2(0.75, 0.5);
const vec2 Scale = vec2(0.25, 0.45);    //vec2(0.1469278, 0.2350845);
const vec2 ScaleIn = vec2(4, 2.2222);
const vec4 HmdWarpParam = vec4(1, 0.22, 0.24, 0);
const vec4 ChromaAbParam = vec4(0.996, -0.004, 1.014, 0.0);

/*
// Another set of default values
ChromaAbCorrection = {1.0, 0.0, 1.0, 0}
DistortionK = {1.0, 0.22, 0.24, 0}
Scale = {0.25, 0.5*AspectRatio, 0, 0}
ScaleIn = {4.0, 2/AspectRatio, 0, 0}
Left Screen Center = {0.25, 0.5, 0, 0}
Left Lens Center = {0.287994117, 0.5, 0, 0}
Right Screen Center = {0.75, 0.5, 0, 0}
Right Lens Center = {0.712005913, 0.5, 0, 0}
*/

void main()
{
    // The following two variables need to be set per eye
    vec2 LensCenter = fragTexCoord.x < 0.5 ? LeftLensCenter : RightLensCenter;
    vec2 ScreenCenter = fragTexCoord.x < 0.5 ? LeftScreenCenter : RightScreenCenter;
    
    // Scales input texture coordinates for distortion: vec2 HmdWarp(vec2 fragTexCoord, vec2 LensCenter)
    vec2 theta = (fragTexCoord - LensCenter)*ScaleIn;   // Scales to [-1, 1]
    float rSq = theta.x*theta.x + theta.y*theta.y;
    vec2 theta1 = theta*(HmdWarpParam.x + HmdWarpParam.y*rSq + HmdWarpParam.z*rSq*rSq + HmdWarpParam.w*rSq*rSq*rSq);
    //vec2 tc = LensCenter + Scale*theta1;
    
    // Detect whether blue texture coordinates are out of range since these will scaled out the furthest
    vec2 thetaBlue = theta1*(ChromaAbParam.z + ChromaAbParam.w*rSq);
    vec2 tcBlue = LensCenter + Scale*thetaBlue;

    if (any(bvec2(clamp(tcBlue, ScreenCenter - vec2(0.25, 0.5), ScreenCenter + vec2(0.25, 0.5)) - tcBlue))) finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    else
    {
        // Do blue texture lookup
        float blue = texture(texture0, tcBlue).b;

        // Do green lookup (no scaling)
        vec2 tcGreen = LensCenter + Scale*theta1;
        float green = texture(texture0, tcGreen).g;

        // Do red scale and lookup
        vec2 thetaRed = theta1*(ChromaAbParam.x + ChromaAbParam.y*rSq);
        vec2 tcRed = LensCenter + Scale*thetaRed;
        float red = texture(texture0, tcRed).r;

        finalColor = vec4(red, green, blue, 1.0);
    }
}

