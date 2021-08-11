#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 texScale;

// Function for drawing outlines on alpha-blended textures
vec4 DrawOutline(sampler2D tex, vec2 uv, vec2 lineScale, vec3 lineCol)
{
	vec2 texelScale = 1.0 / lineScale;
	vec4 center = texture2D(tex, uv); // We sample the center texel, (with all color data)
	// Next we sample four corner texels, but only for the alpha channel (this is for the outline)
	vec4 corners;
	corners.x = texture2D(tex, uv+vec2( texelScale.x, texelScale.y)).a;
	corners.y = texture2D(tex, uv+vec2( texelScale.x,-texelScale.y)).a;
	corners.z = texture2D(tex, uv+vec2(-texelScale.x, texelScale.y)).a;
	corners.w = texture2D(tex, uv+vec2(-texelScale.x,-texelScale.y)).a;
	
	float outline = min(dot(corners, vec4(1.0)), 1.0);
	vec4 col = mix(vec4(0.0), vec4(lineCol, 1.0), outline);
	col = mix(col, center, center.a);
	return col;
}

void main()
{
	gl_FragColor = DrawOutline(texture0, fragTexCoord, texScale, vec3(0.0));
}