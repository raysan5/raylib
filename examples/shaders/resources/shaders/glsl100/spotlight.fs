#version 100

precision mediump float;

#define     MAX_SPOTS			4
#define		RADIUS				256.0
#define		INNER				200.0

// Inputs
// array of spotlight positions
uniform vec2 spots[MAX_SPOTS];

uniform float screenWidth; // width of the screen

void main()
{

	float alpha;
	// get the position of the current fragment (screen coordinates!)
	vec2 pos = vec2(gl_FragCoord.x, gl_FragCoord.y);

	
	// find out which spotlight is nearest
	float d = 65000.0; // some high value
	float di = 0.0;

    for (int i = 0; i < MAX_SPOTS; i++)
    {
		di = distance(pos, spots[i]);
		if (d > di) d = di;
    }
    
    // d now equals distance to nearest spot...
    if (d > RADIUS) {
		alpha = 1.0;
	} else {
		if (d < INNER) {
			alpha = 0.0;
		} else {
			alpha = (d - INNER) / (RADIUS - INNER);
		}
	}
	
	// right hand side of screen is dimly lit, could make the
	// threshold value user definable.
	if (pos.x > screenWidth/2.0 && alpha > 0.9) {
		alpha = 0.9;
	}

	// could make the black out colour user definable...
    gl_FragColor = vec4( 0, 0, 0, alpha);
}
