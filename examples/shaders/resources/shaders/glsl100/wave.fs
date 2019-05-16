#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float secondes;

uniform vec2 size;

uniform float freqX;
uniform float freqY;
uniform float ampX;
uniform float ampY;
uniform float speedX;
uniform float speedY;

void main() {
	float pixelWidth = 1.0 / size.x;
	float pixelHeight = 1.0 / size.y;
	float aspect = pixelHeight / pixelWidth;
	float boxLeft = 0.0;
	float boxTop = 0.0;

	vec2 p = fragTexCoord;
	p.x += cos((fragTexCoord.y - boxTop) * freqX / ( pixelWidth * 750.0) + (secondes * speedX)) * ampX * pixelWidth;
	p.y += sin((fragTexCoord.x - boxLeft) * freqY * aspect / ( pixelHeight * 750.0) + (secondes * speedY)) * ampY * pixelHeight;

	gl_FragColor = texture2D(texture0, p)*colDiffuse*fragColor;
}
