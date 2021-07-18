#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 texScale;

// Output fragment color
out vec4 finalColor;

// Function for drawing outlines on alpha-blended textures
vec4 DrawOutline(sampler2D tex, vec2 uv, vec2 lineScale, vec3 lineCol)
{
	vec2 texelScale = 1.0 / lineScale;
	vec4 center = texture(tex, uv); // We sample the center texel, (with all color data)
	// Next we sample four corner texels, but only for the alpha channel (this is for the outline)
	vec4 corners;
	corners.x = texture(tex, uv+vec2( texelScale.x, texelScale.y)).a;
	corners.y = texture(tex, uv+vec2( texelScale.x,-texelScale.y)).a;
	corners.z = texture(tex, uv+vec2(-texelScale.x, texelScale.y)).a;
	corners.w = texture(tex, uv+vec2(-texelScale.x,-texelScale.y)).a;
	
	float outline = min(dot(corners, vec4(1.0)), 1.0);
	vec4 col = mix(vec4(0.0), vec4(lineCol, 1.0), outline);
	col = mix(col, center, center.a);
	return col;
}

void main()
{
	finalColor = DrawOutline(texture0, fragTexCoord, texScale, vec3(0.0));
}