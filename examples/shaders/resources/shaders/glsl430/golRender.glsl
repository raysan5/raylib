// Game of Life rendering shader
// Just renders the content of the ssbo at binding 1 to screen.
#version 430

#define GOL_WIDTH 768

out vec4 finalColor;
in vec2 fragTexCoord;

layout(std430, binding = 1) readonly buffer golLayout {
  uint golBuffer[];
};

uniform vec2 res;

void main()
{
  ivec2 coords = ivec2(fragTexCoord * res);

  if (golBuffer[coords.x + coords.y * uvec2(res).x] == 1)
    finalColor = vec4(1.0);
  else
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
}
