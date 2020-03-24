#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_SPOTS			2
#define		RADIUS				128
#define		INNER				96

uniform vec2 spots[MAX_SPOTS];      // Spotlight positions array
uniform float screenWidth;          // Width of the screen

void main()
{
	float alpha = 0.0;
    
	// Get the position of the current fragment (screen coordinates!)
	vec2 pos = vec2(gl_FragCoord.x, gl_FragCoord.y);
	
	// Find out which spotlight is nearest
	float d = 65000; // some high value
	float di = 0;

    for (int i = 0; i < MAX_SPOTS; i++)
    {
		di = distance(pos, spots[i]);
		if (d > di) d = di;
    }
    
    // d now equals distance to nearest spot...
    if (d > RADIUS) alpha = 1.0;
	else
    {
		if (d < INNER) alpha = 0.0;
		else alpha = (d - INNER)/(RADIUS - INNER);
	}
	
	// Right hand side of screen is dimly lit, 
    // could make the threshold value user definable
	if ((pos.x > screenWidth/2.0) && (alpha > 0.9)) alpha = 0.9;

    finalColor = vec4(0, 0, 0, alpha);
}
