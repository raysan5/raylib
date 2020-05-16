#version 100

precision mediump float;

#define MAX_SPOTS   3

struct Spot {
    vec2 pos;		// window coords of spot
    float inner;	// inner fully transparent centre radius
    float radius;	// alpha fades out to this radius
};

uniform Spot spots[MAX_SPOTS];  // Spotlight positions array
uniform float screenWidth;      // Width of the screen

void main()
{
	float alpha = 1.0;
    
	// Get the position of the current fragment (screen coordinates!)
	vec2 pos = vec2(gl_FragCoord.x, gl_FragCoord.y);
	
	// Find out which spotlight is nearest
	float d = 65000.0;  // some high value
	int fi = -1;        // found index

    for (int i = 0; i < MAX_SPOTS; i++)
    {
		for (int j = 0; j < MAX_SPOTS; j++)
		{
			float dj = distance(pos, spots[j].pos) - spots[j].radius + spots[i].radius;
            
			if (d > dj) 
			{
				d = dj;
				fi = i;
			}
		}
    }
    
    // d now equals distance to nearest spot...
    // allowing for the different radii of all spotlights
    if (fi == 0) 
    {
		if (d > spots[0].radius) alpha = 1.0;
		else
		{
			if (d < spots[0].inner) alpha = 0.0;
			else alpha = (d - spots[0].inner)/(spots[0].radius - spots[0].inner);
		}
	}
    else if (fi == 1) 
    {
		if (d > spots[1].radius) alpha = 1.0;
		else
		{
			if (d < spots[1].inner) alpha = 0.0;
			else alpha = (d - spots[1].inner)/(spots[1].radius - spots[1].inner);
		}
	}
    else if (fi == 2) 
    {
		if (d > spots[2].radius) alpha = 1.0;
		else
		{
			if (d < spots[2].inner) alpha = 0.0;
			else alpha = (d - spots[2].inner)/(spots[2].radius - spots[2].inner);
		}
	}
	
	// Right hand side of screen is dimly lit, 
    // could make the threshold value user definable
	if ((pos.x > screenWidth/2.0) && (alpha > 0.9)) alpha = 0.9;

	// could make the black out colour user definable...
    gl_FragColor = vec4(0, 0, 0, alpha);
}
