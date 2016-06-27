#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;

// Input uniform values
uniform sampler2D texture0;

// NOTE: Add here your custom variables
const vec2 LeftLensCenter = vec2(0.2863248, 0.5);
const vec2 RightLensCenter = vec2(0.7136753, 0.5);
const vec2 LeftScreenCenter = vec2(0.25, 0.5);
const vec2 RightScreenCenter = vec2(0.75, 0.5);
const vec2 Scale = vec2(0.25, 0.45);    //vec2(0.1469278, 0.2350845);
const vec2 ScaleIn = vec2(4, 2.2222);
const vec4 HmdWarpParam = vec4(1, 0.22, 0.24, 0);

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

// Scales input texture coordinates for distortion.
vec2 HmdWarp(vec2 in01, vec2 LensCenter)
{
    vec2 theta = (in01 - LensCenter)*ScaleIn; // Scales to [-1, 1]
    float rSq = theta.x*theta.x + theta.y*theta.y;
    vec2 rvector = theta*(HmdWarpParam.x + HmdWarpParam.y*rSq + HmdWarpParam.z*rSq*rSq + HmdWarpParam.w*rSq*rSq*rSq);

    return LensCenter + Scale*rvector;
}

void main()
{
    // SOURCE: http://www.mtbs3d.com/phpbb/viewtopic.php?f=140&t=17081
    
    // The following two variables need to be set per eye
    vec2 LensCenter = fragTexCoord.x < 0.5 ? LeftLensCenter : RightLensCenter;
    vec2 ScreenCenter = fragTexCoord.x < 0.5 ? LeftScreenCenter : RightScreenCenter;

    vec2 tc = HmdWarp(fragTexCoord, LensCenter);

    if (any(bvec2(clamp(tc,ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc))) gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    else gl_FragColor = texture2D(texture0, tc);
    
    /*
    // Chromatic aberration is caused when a lens can't focus every color to the same focal point. 
    // A simple way to fake this effect, and render it as a quick full-screen post-process, 
    // is to apply an offset to each color channel in a fragment shader.
    vec4 rValue = texture2D(texture0, fragTexCoord - rOffset);  
    vec4 gValue = texture2D(texture0, fragTexCoord - gOffset);
    vec4 bValue = texture2D(texture0, fragTexCoord - bOffset);
    finalColor = vec4(rValue.r, gValue.g, bValue.b, 1.0);
    */
}
