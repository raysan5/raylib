#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

uniform float seconds;

uniform vec2 size;

uniform float freqX;
uniform float freqY;
uniform float ampX;
uniform float ampY;
uniform float speedX;
uniform float speedY;

void main() {
    float pixelWidth = 1.0/size.x;
    float pixelHeight = 1.0/size.y;
    float aspect = pixelHeight/pixelWidth;
    float boxLeft = 0.0;
    float boxTop = 0.0;

    vec2 p = fragTexCoord;
    p.x += cos((fragTexCoord.y - boxTop)*freqX/(pixelWidth*750.0) + (seconds*speedX))*ampX*pixelWidth;
    p.y += sin((fragTexCoord.x - boxLeft)*freqY*aspect/(pixelHeight*750.0) + (seconds*speedY))*ampY*pixelHeight;

    finalColor = texture(texture0, p)*colDiffuse*fragColor;
}
