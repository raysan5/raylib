#version 430

// Game of Life logic shader

#define GOL_WIDTH 768

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(std430, binding = 1) readonly restrict buffer golLayout {
    uint golBuffer[];       // golBuffer[x, y] = golBuffer[x + gl_NumWorkGroups.x * y]
};

layout(std430, binding = 2) writeonly restrict buffer golLayout2 {
    uint golBufferDest[];   // golBufferDest[x, y] = golBufferDest[x + gl_NumWorkGroups.x * y]
};

#define fetchGol(x, y) ((((x) < 0) || ((y) < 0) || ((x) > GOL_WIDTH) || ((y) > GOL_WIDTH)) \
    ? (0) \
    : golBuffer[(x) + GOL_WIDTH * (y)])

#define setGol(x, y, value) golBufferDest[(x) + GOL_WIDTH*(y)] = value

void main()
{
    uint neighbourCount = 0;
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;

    neighbourCount += fetchGol(x - 1, y - 1);   // Top left
    neighbourCount += fetchGol(x, y - 1);       // Top middle
    neighbourCount += fetchGol(x + 1, y - 1);   // Top right
    neighbourCount += fetchGol(x - 1, y);       // Left
    neighbourCount += fetchGol(x + 1, y);       // Right
    neighbourCount += fetchGol(x - 1, y + 1);   // Bottom left
    neighbourCount += fetchGol(x, y + 1);       // Bottom middle   
    neighbourCount += fetchGol(x + 1, y + 1);   // Bottom right

    if (neighbourCount == 3) setGol(x, y, 1);
    else if (neighbourCount == 2) setGol(x, y, fetchGol(x, y));
    else setGol(x, y, 0);
}
