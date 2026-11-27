#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// Input size in pixels of the textures
uniform vec2 resolution;

void main()
{
    // Size of one pixel in texture coordinates (from 0.0 to 1.0)
    float x = 1.0/resolution.x;
    float y = 1.0/resolution.y;

    // Status of the current cell (1 = alive, 0 = dead)
    int origValue = (texture(texture0, fragTexCoord).r < 0.1)? 1 : 0;

    // Sum of alive neighbors
    int sumValue = (texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y - y)).r < 0.1)? 1 : 0;    // Top-left
    sumValue    += (texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y    )).r < 0.1)? 1 : 0;    // Top
    sumValue    += (texture(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y + y)).r < 0.1)? 1 : 0;    // Top-right

    sumValue    += (texture(texture0, vec2(fragTexCoord.x,     fragTexCoord.y - y)).r < 0.1)? 1 : 0;    // Left
    sumValue    += (texture(texture0, vec2(fragTexCoord.x,     fragTexCoord.y + y)).r < 0.1)? 1 : 0;    // Right

    sumValue    += (texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y - y)).r < 0.1)? 1 : 0;    // Bottom-left
    sumValue    += (texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y    )).r < 0.1)? 1 : 0;    // Bottom
    sumValue    += (texture(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y + y)).r < 0.1)? 1 : 0;    // Bottom-right

    // Game of life rules:
    // Current cell remains alive when 2 or 3 neighbors are alive, dies otherwise
    // Current cell goes from dead to alive when exactly 3 neighbors are alive
    if (((origValue == 1) && (sumValue == 2)) || sumValue == 3)
        finalColor = vec4(0.0, 0.0, 0.0, 255.0);    // Alive: draw the pixel black
    else
        finalColor = fragColor;                     // Dead: draw the pixel with the background color, RAYWHITE
}
