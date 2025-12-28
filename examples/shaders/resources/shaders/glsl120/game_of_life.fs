#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Input size in pixels of the textures
uniform vec2 resolution;

void main()
{
    // Size of one pixel in texture coordinates (from 0.0 to 1.0)
    float x = 1.0/resolution.x;
    float y = 1.0/resolution.y;

    // Status of the current cell (1 = alive, 0 = dead)
    int origValue = (texture2D(texture0, fragTexCoord).r < 0.1)? 1 : 0;

    // Sum of alive neighbors
    int sumValue = (texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y - y)).r < 0.1)? 1 : 0;  // Top-left
    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y    )).r < 0.1)? 1 : 0;  // Top
    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x - x, fragTexCoord.y + y)).r < 0.1)? 1 : 0;  // Top-right

    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x,     fragTexCoord.y - y)).r < 0.1)? 1 : 0;  // Left
    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x,     fragTexCoord.y + y)).r < 0.1)? 1 : 0;  // Right

    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y - y)).r < 0.1)? 1 : 0;  // Bottom-left
    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y    )).r < 0.1)? 1 : 0;  // Bottom
    sumValue    += (texture2D(texture0, vec2(fragTexCoord.x + x, fragTexCoord.y + y)).r < 0.1)? 1 : 0;  // Bottom-right

    // Game of life rules:
    // Current cell remains alive when 2 or 3 neighbors are alive, dies otherwise
    // Current cell goes from dead to alive when exactly 3 neighbors are alive
    if (((origValue == 1) && (sumValue == 2)) || sumValue == 3)
        gl_FragColor = vec4(0.0, 0.0, 0.0, 255.0);  // Alive: draw the pixel black
    else
        gl_FragColor = fragColor;                   // Dead: draw the pixel with the background color, RAYWHITE
}
