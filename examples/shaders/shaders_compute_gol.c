/*******************************************************************************************
*
*   raylib [shaders] example - Compute shaders Conway's Game of Life
*
*   NOTE: This example requires raylib OpenGL 4.3 versions for compute shaders support,
*
*   NOTE: Shaders used in this example are #version 430 (OpenGL 4.3).
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Teddy Astie (@tsnake41)
*
*   Copyright (c) 2021 Teddy Astie (@tsnake41)
*
********************************************************************************************/

#include <stdlib.h>

#include "raylib.h"
#include "rlgl.h"

// IMPORTANT: This must match gol*.glsl GOL_WIDTH constant.
//            This must be a multiple of 16 (check golLogic compute dispatch).
#define GOL_WIDTH 768

#define SSBO_SIZE (sizeof(unsigned int) * GOL_WIDTH * GOL_WIDTH)

// Maximum amount of queued draw commands (squares draw from mouse down events).
#define MAX_BUFFERED_TRANSFERTS 48

struct GolUpdateCmd {
  unsigned int x; // x coordinate of the gol command
  unsigned int y; // y coordinate of the gol command
  unsigned int w; // width of the filled zone
  unsigned int enabled; // whether to enable or disable zone
};

struct GolUpdateSSBO {
  unsigned int count;
  struct GolUpdateCmd commands[MAX_BUFFERED_TRANSFERTS];
};

int main(void)
{
  InitWindow(GOL_WIDTH, GOL_WIDTH, "raylib [shaders] example - compute shader gol");

  const Vector2 resolution = { GOL_WIDTH, GOL_WIDTH };
  unsigned int brush_size = 1;

  // Game of Life logic compute shader
  char *golLogicCode = LoadFileText("resources/shaders/glsl430/gol.glsl");
  unsigned int golLogicShader = rlCompileShader(golLogicCode, RL_COMPUTE_SHADER);
  unsigned int golLogicProgram = rlLoadComputeShaderProgram(golLogicShader);
  MemFree(golLogicCode);

  // Game of Life logic compute shader
  Shader golRenderShader = LoadShader(NULL, "resources/shaders/glsl430/golRender.glsl");
  int resUniformLoc = GetShaderLocation(golRenderShader, "res");

  // Game of Life transfert shader
  char *golTransfertCode = LoadFileText("resources/shaders/glsl430/golTransfert.glsl");
  unsigned int golTransfertShader = rlCompileShader(golTransfertCode, RL_COMPUTE_SHADER);
  unsigned int golTransfertProgram = rlLoadComputeShaderProgram(golTransfertShader);
  MemFree(golTransfertCode);

  // SSBOs
  unsigned int ssboA = rlLoadShaderBuffer(SSBO_SIZE, NULL, RL_DYNAMIC_COPY);
  unsigned int ssboB = rlLoadShaderBuffer(SSBO_SIZE, NULL, RL_DYNAMIC_COPY);

  struct GolUpdateSSBO transfertBuffer;
  transfertBuffer.count = 0;

  int transfertSSBO = rlLoadShaderBuffer(sizeof(struct GolUpdateSSBO), NULL, RL_DYNAMIC_COPY);

  Image whiteImage = GenImageColor(GOL_WIDTH, GOL_WIDTH, WHITE);
  Texture whiteTex = LoadTextureFromImage(whiteImage);
  UnloadImage(whiteImage);

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_UP))
      brush_size *= 2;
    else if (IsKeyPressed(KEY_DOWN) && brush_size != 1)
      brush_size /= 2;

    if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
      && transfertBuffer.count < MAX_BUFFERED_TRANSFERTS) {
      // Buffer a new command
      transfertBuffer.commands[transfertBuffer.count].x = GetMouseX();
      transfertBuffer.commands[transfertBuffer.count].y = GetMouseY();
      transfertBuffer.commands[transfertBuffer.count].w = brush_size;
      transfertBuffer.commands[transfertBuffer.count].enabled = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
      transfertBuffer.count++;
    } else if (transfertBuffer.count > 0) {
      // Process transfert buffer

      // Send SSBO buffer to GPU
      rlUpdateShaderBufferElements(transfertSSBO, &transfertBuffer, sizeof(struct GolUpdateSSBO), 0);
      // Process ssbo command
      rlEnableShader(golTransfertProgram);
      rlBindShaderBuffer(ssboA, 1);
      rlBindShaderBuffer(transfertSSBO, 3);
      rlComputeShaderDispatch(transfertBuffer.count, 1, 1); // each GPU unit will process a command
      rlDisableShader();

      transfertBuffer.count = 0;
    } else {
      // Process game of life logic
      rlEnableShader(golLogicProgram);
      rlBindShaderBuffer(ssboA, 1);
      rlBindShaderBuffer(ssboB, 2);
      rlComputeShaderDispatch(GOL_WIDTH / 16, GOL_WIDTH / 16, 1);
      rlDisableShader();

      // ssboA <-> ssboB
      int temp = ssboA;
      ssboA = ssboB;
      ssboB = temp;
    }

    rlBindShaderBuffer(ssboA, 1);

    BeginDrawing();

    ClearBackground(BLANK);
    SetShaderValue(golRenderShader, resUniformLoc, &resolution, SHADER_UNIFORM_VEC2);

    BeginShaderMode(golRenderShader);
    DrawTexture(whiteTex, 0, 0, WHITE);
    EndShaderMode();

    DrawFPS(0, 0);

    EndDrawing();
  }

  rlUnloadShaderBuffer(ssboA);
  rlUnloadShaderBuffer(ssboB);
  rlUnloadShaderBuffer(transfertSSBO);
  rlUnloadShaderProgram(golTransfertProgram);
  rlUnloadShaderProgram(golLogicProgram);

  UnloadTexture(whiteTex);
  UnloadShader(golRenderShader);

  CloseWindow();
}