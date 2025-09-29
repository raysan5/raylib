#version 120

// Input from the vertex shader
varying vec2 fragTexCoord;

// Output color for the screen
varying vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 resolution;
uniform float fontSize;

float greyScale(in vec3 col) {
    return dot(col, vec3(0.2126, 0.7152, 0.0722));
}

float character(float n, vec2 p)
{
	p = floor(p * vec2(4.0, -4.0) + 2.5);
	
	// Check if the coordinate is inside the 5x5 grid (0 to 4).
	if (clamp(p.x, 0.0, 4.0) == p.x && clamp(p.y, 0.0, 4.0) == p.y) {
        
		if (int(mod(n / exp2(p.x + 5.0 * p.y), 2.0)) == 1) {
            return 1.0; // The bit is on, so draw this part of the character.
        }
    }

	return 0.0; // The bit is off, or we are outside the grid.
}

// -----------------------------------------------------------------------------
// Main shader logic
// -----------------------------------------------------------------------------
void main() 
{
    vec2 charPixelSize = vec2(fontSize, fontSize * 1.8);
    vec2 uvCellSize = charPixelSize / resolution;

    vec2 cellUV = floor(fragTexCoord / uvCellSize) * uvCellSize;

    vec3 cellColor = texture2D(texture0, cellUV).rgb;
    float gray = greyScale(cellColor);

	float n =  4096;
    
    // limited character set
    if (gray > 0.2) n = 65600.0;    // :
	if (gray > 0.3) n = 163153.0;   // *
	if (gray > 0.4) n = 15255086.0; // o 
	if (gray > 0.5) n = 13121101.0; // &
	if (gray > 0.6) n = 15252014.0; // 8
	if (gray > 0.7) n = 13195790.0; // @
	if (gray > 0.8) n = 11512810.0; // #

    vec2 localUV = (fragTexCoord - cellUV) / uvCellSize; // Range [0.0, 1.0]

    vec2 p = localUV * 2.0 - 1.0;

    float charShape = character(n, p);

    vec3 final_col = cellColor * charShape;

    gl_FragColor = vec4(final_col, 1.0);
}