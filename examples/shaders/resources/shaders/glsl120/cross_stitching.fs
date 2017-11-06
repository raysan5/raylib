# version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables

// NOTE: Render size values must be passed from code
const float renderWidth = 800.0;
const float renderHeight = 450.0;

float stitchingSize = 6.0;
int invert = 0;

vec4 PostFX(sampler2D tex, vec2 uv)
{
    vec4 c = vec4(0.0);
    float size = stitchingSize;
    vec2 cPos = uv * vec2(renderWidth, renderHeight);
    vec2 tlPos = floor(cPos / vec2(size, size));
    tlPos *= size;

    int remX = int(mod(cPos.x, size));
    int remY = int(mod(cPos.y, size));
    
    if (remX == 0 && remY == 0) tlPos = cPos;
    
    vec2 blPos = tlPos;
    blPos.y += (size - 1.0);
    
    if ((remX == remY) || (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y)))))
    {
        if (invert == 1) c = vec4(0.2, 0.15, 0.05, 1.0);
        else c = texture2D(tex, tlPos * vec2(1.0/renderWidth, 1.0/renderHeight)) * 1.4;
    }
    else
    {
        if (invert == 1) c = texture2D(tex, tlPos * vec2(1.0/renderWidth, 1.0/renderHeight)) * 1.4;
        else c = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    return c;
}

void main()
{
    vec3 tc = PostFX(texture0, fragTexCoord).rgb;

    gl_FragColor = vec4(tc, 1.0);
}