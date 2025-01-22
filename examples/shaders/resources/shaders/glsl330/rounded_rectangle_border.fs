// Note: SDF by Iñigo Quilez is licensed under MIT License

#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

uniform vec4 rectangle; // Rectangle dimensions (x, y, width, height)
uniform vec4 radius; // Corner radius (top-left, top-right, bottom-left, bottom-right)
uniform float borderThickness;

// TODO: Remove anti-aliasing?

// Anti-alias using easing function for smmoth edges
uniform float aaPower;
uniform float aaDistance;

// Ease in-out
float ease(float x, float p)
{
    if (x < 0.5)
    {
        return 1.0/pow(0.5, p - 1.0)*pow(x, p);
    }
    else
    {
        return 1.0 - 1.0/pow(0.5, p - 1.0)*pow(1.0 - x, p);
    }
}

// Smoothstep with easing
float easestep(float edge0, float edge1, float x, float p)
{
    float t = clamp( (x - edge0)/(edge1 - edge0), 0.0, 1.0 );
    return ease(t, p);
}

// Anti-alias on edge for x
float antiAlias(float edge, float x)
{
    return easestep(edge + aaDistance*0.5, edge - aaDistance*0.5, x, aaPower);
}

// Create a rounded rectangle using signed distance field
// Thanks to Iñigo Quilez (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
// And thanks to inobelar (https://www.shadertoy.com/view/fsdyzB) for shader
// MIT License
float roundedRectangleSDF(
    vec2 fragCoord,
    vec2 center,
    vec2 halfSize,
    vec4 radius
)
{
    vec2 fragFromCenter = fragCoord - center;

    // Determine which corner radius to use
    radius.xy = (fragFromCenter.y > 0.0) ? radius.xy : radius.zw;
    radius.x  = (fragFromCenter.x < 0.0) ? radius.x  : radius.y;

    // Calculate signed distance field
    vec2 dist = abs(fragFromCenter) - halfSize + radius.x;
    return min(max(dist.x, dist.y), 0.0) + length(max(dist, 0.0)) - radius.x;
}

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // Get fragment coordinate in pixels
    vec2 fragCoord = gl_FragCoord.xy;

    // Calculate signed distance field for rounded rectangle's border
    vec2 halfSize = rectangle.zw*0.5;
    vec2 center = rectangle.xy + halfSize;
    float sdf = roundedRectangleSDF(fragCoord, center, halfSize, radius);

    // Calculate anti-aliased factor
    float aa = antiAlias(borderThickness, -sdf) * (1.0 - antiAlias(0.0, -sdf));

    finalColor = texelColor*colDiffuse*fragColor
                 *vec4(1.0, 1.0, 1.0, aa);
}