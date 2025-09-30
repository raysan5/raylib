#version 120

// Input from the vertex shader
varying vec2 fragTexCoord;

// Output color for the screen
varying vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 resolution;

// Fontsize less then 9 may be not complete
uniform float fontSize;

float GreyScale(in vec3 col)
{
    return dot(col, vec3(0.2126, 0.7152, 0.0722));
}

float GetCharacter(float n, vec2 p)
{
    p = floor(p*vec2(-4.0, 4.0) + 2.5);

    // Check if the calculated coordinate is inside the 5x5 grid (from 0.0 to 4.0)
    if (clamp(p.x, 0.0, 4.0) == p.x && clamp(p.y, 0.0, 4.0) == p.y)
    {
        float a = floor(p.x + 0.5) + 5.0*floor(p.y + 0.5);

        // This checked if the 'a'-th bit of 'n' was set
        float shiftedN = floor(n/pow(2.0, a));

        if (mod(shiftedN, 2.0) == 1.0)
        {
            return 1.0; // The bit is on
        }
    }

    return 0.0; // The bit is off, or we are outside the grid
}

// -----------------------------------------------------------------------------
// Main shader logic
// -----------------------------------------------------------------------------

void main() 
{
    vec2 charPixelSize = vec2(fontSize, fontSize);
    vec2 uvCellSize = charPixelSize / resolution;

    // The cell size is based on the fontSize set by application
    vec2 cellUV = floor(fragTexCoord / uvCellSize)*uvCellSize;

    vec3 cellColor = texture2D(texture0, cellUV).rgb;

    // Gray is used to define what character will be selected to draw
    float gray = GreyScale(cellColor);

	float n =  4096.0;
    
    // Character set from https://www.shadertoy.com/view/lssGDj
    // Create new bitmaps https://thrill-project.com/archiv/coding/bitmap/
    if (gray > 0.2) n = 65600.0;    // :
	if (gray > 0.3) n = 18725316.0; // v
	if (gray > 0.4) n = 15255086.0; // o 
	if (gray > 0.5) n = 13121101.0; // &
	if (gray > 0.6) n = 15252014.0; // 8
	if (gray > 0.7) n = 13195790.0; // @
	if (gray > 0.8) n = 11512810.0; // #

    vec2 localUV = (fragTexCoord - cellUV)/uvCellSize; // Range [0.0, 1.0]

    vec2 p = localUV*2.0 - 1.0; // Range [-1.0, 1.0]

    // cellColor and charShape will define the color of the char
    vec3 color = cellColor*GetCharacter(n, p);

    gl_FragColor = vec4(color, 1.0);
}