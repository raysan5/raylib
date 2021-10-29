// Game of life transfert shader.
#version 430
#define GOL_WIDTH 768

struct GolUpdateCmd {
  uint x; // x coordinate of the gol command
  uint y; // y coordinate of the gol command
  uint w; // width of the filled zone
  uint enabled; // whether to enable or disable zone
};

layout (local_size_x = 1, local_size_y = 1, local_size_x = 1) in;

layout(std430, binding = 1) writeonly restrict buffer golBufferLayout {
  uint golBuffer[]; // golBuffer[x, y] = golBuffer[x + GOL_WIDTH * y]
};

layout(std430, binding = 3) readonly restrict buffer golUpdateLayout {
  uint count;
  GolUpdateCmd commands[];
};

#define isInside(x, y) (((x) >= 0) && ((y) >= 0) && ((x) < GOL_WIDTH) && ((y) < GOL_WIDTH))
#define getBufferIndex(x, y) ((x) + GOL_WIDTH * (y))

void main()
{
  uint cmd_index = gl_GlobalInvocationID.x;
  struct GolUpdateCmd cmd = commands[cmd_index];

  for (uint x = cmd.x; x < (cmd.x + cmd.w); x++) {
    for (uint y = cmd.y; y < (cmd.y + cmd.w); y++) {
      if (isInside(x, y)) {
        if (cmd.enabled != 0) {
          atomicOr(golBuffer[getBufferIndex(x, y)], 1);
        } else {
          atomicAnd(golBuffer[getBufferIndex(x, y)], 0);
        }
      }
    }
  }
}