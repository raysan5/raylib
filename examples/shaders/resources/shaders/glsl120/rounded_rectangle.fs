// Note: SDF by Iñigo Quilez is licensed under MIT License

#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec4 rectangle; // Rectangle dimensions (x, y, width, height)
uniform vec4 radius; // Corner radius (top-left, top-right, bottom-left, bottom-right)
uniform vec4 color;

// Shadow parameters
uniform float shadowRadius;
uniform vec2 shadowOffset;
uniform float shadowScale;
uniform vec4 shadowColor;

// Border parameters
uniform float borderThickness;
uniform vec4 borderColor;

// Create a rounded rectangle using signed distance field
// Thanks to Iñigo Quilez (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
// And thanks to inobelar (https://www.shadertoy.com/view/fsdyzB) for shader
// MIT License
float RoundedRectangleSDF(vec2 fragCoord, vec2 center, vec2 halfSize, vec4 radius)
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
    vec4 texelColor = texture2D(texture0, fragTexCoord);

    // Requires fragment coordinate varying pixels
    vec2 fragCoord = gl_FragCoord.xy;

    // Calculate signed distance field for rounded rectangle
    vec2 halfSize = rectangle.zw*0.5;
    vec2 center = rectangle.xy + halfSize;
    float recSDF = RoundedRectangleSDF(fragCoord, center, halfSize, radius);

    // Calculate signed distance field for rectangle shadow
    vec2 shadowHalfSize = halfSize*shadowScale;
    vec2 shadowCenter = center + shadowOffset;
    float shadowSDF = RoundedRectangleSDF(fragCoord, shadowCenter, shadowHalfSize, radius);

    // Caculate alpha factors
    float recFactor = smoothstep(1.0, 0.0, recSDF);
    float shadowFactor = smoothstep(shadowRadius, 0.0, shadowSDF);
    float borderFactor = smoothstep(0.0, 1.0, recSDF + borderThickness)*recFactor;

    // Multiply each color by its respective alpha factor
    vec4 recColor = vec4(color.rgb, color.a*recFactor);
    vec4 shadowCol = vec4(shadowColor.rgb, shadowColor.a*shadowFactor);
    vec4 borderCol = vec4(borderColor.rgb, borderColor.a*borderFactor);

    // Combine the colors varying the order (shadow, rectangle, border)
    gl_FragColor = mix(mix(shadowCol, recColor, recColor.a), borderCol, borderCol.a);
}