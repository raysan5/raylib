/*******************************************************************************************
*
*   raylib [rlgl] example - compute shader - Conway's Game of Life
*
*   NOTE: This example requires raylib OpenGL 4.3 versions for compute shaders support,
*         shaders used in this example are #version 430 (OpenGL 4.3)
*
*   This example has been created using raylib 4.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Example contributed by Teddy Astie (@tsnake41) and reviewed by Ramon Santamaria (@raysan5)
*
*   Copyright (c) 2021 Teddy Astie (@tsnake41)
*
********************************************************************************************/

#include "raylib.h"
#include "rlgl.h"

#include <stdlib.h>

// IMPORTANT: This must match gol*.glsl GOL_WIDTH constant.
// This must be a multiple of 16 (check golLogic compute dispatch).
#define GOL_WIDTH 768

// Maximum amount of queued draw commands (squares draw from mouse down events).
#define MAX_BUFFERED_TRANSFERTS 48

// Game Of Life Update Command
typedef struct GolUpdateCmd {
    unsigned int x;         // x coordinate of the gol command
    unsigned int y;         // y coordinate of the gol command
    unsigned int w;         // width of the filled zone
    unsigned int enabled;   // whether to enable or disable zone
} GolUpdateCmd;

// Game Of Life Update Commands SSBO
typedef struct GolUpdateSSBO {
    unsigned int count;
    GolUpdateCmd commands[MAX_BUFFERED_TRANSFERTS];
} GolUpdateSSBO;

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(GOL_WIDTH, GOL_WIDTH, "raylib [rlgl] example - compute shader - game of life");

    const Vector2 resolution = { GOL_WIDTH, GOL_WIDTH };
    unsigned int brushSize = 8;

    // Game of Life logic compute shader
    char *golLogicCode = LoadFileText("resources/shaders/glsl430/gol.glsl");
    unsigned int golLogicShader = rlCompileShader(golLogicCode, RL_COMPUTE_SHADER);
    unsigned int golLogicProgram = rlLoadComputeShaderProgram(golLogicShader);
    UnloadFileText(golLogicCode);

    // Game of Life logic compute shader
    Shader golRenderShader = LoadShader(NULL, "resources/shaders/glsl430/gol_render.glsl");
    int resUniformLoc = GetShaderLocation(golRenderShader, "resolution");

    // Game of Life transfert shader
    char *golTransfertCode = LoadFileText("resources/shaders/glsl430/gol_transfert.glsl");
    unsigned int golTransfertShader = rlCompileShader(golTransfertCode, RL_COMPUTE_SHADER);
    unsigned int golTransfertProgram = rlLoadComputeShaderProgram(golTransfertShader);
    UnloadFileText(golTransfertCode);

    // SSBOs
    unsigned int ssboA = rlLoadShaderBuffer(GOL_WIDTH*GOL_WIDTH*sizeof(unsigned int), NULL, RL_DYNAMIC_COPY);
    unsigned int ssboB = rlLoadShaderBuffer(GOL_WIDTH*GOL_WIDTH*sizeof(unsigned int), NULL, RL_DYNAMIC_COPY);

    struct GolUpdateSSBO transfertBuffer;
    transfertBuffer.count = 0;

    int transfertSSBO = rlLoadShaderBuffer(sizeof(struct GolUpdateSSBO), NULL, RL_DYNAMIC_COPY);

    // Create a white texture of the size of the window to update 
    // each pixel of the window using the fragment shader
    Image whiteImage = GenImageColor(GOL_WIDTH, GOL_WIDTH, WHITE);
    Texture whiteTex = LoadTextureFromImage(whiteImage);
    UnloadImage(whiteImage);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        brushSize += (int)GetMouseWheelMove();

        if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            && (transfertBuffer.count < MAX_BUFFERED_TRANSFERTS))
        {
            // Buffer a new command
            transfertBuffer.commands[transfertBuffer.count].x = GetMouseX() - brushSize/2;
            transfertBuffer.commands[transfertBuffer.count].y = GetMouseY() - brushSize/2;
            transfertBuffer.commands[transfertBuffer.count].w = brushSize;
            transfertBuffer.commands[transfertBuffer.count].enabled = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            transfertBuffer.count++;
        }
        else if (transfertBuffer.count > 0)
        {
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
        }
        else
        {
            // Process game of life logic
            rlEnableShader(golLogicProgram);
            rlBindShaderBuffer(ssboA, 1);
            rlBindShaderBuffer(ssboB, 2);
            rlComputeShaderDispatch(GOL_WIDTH/16, GOL_WIDTH/16, 1);
            rlDisableShader();

            // ssboA <-> ssboB
            int temp = ssboA;
            ssboA = ssboB;
            ssboB = temp;
        }

        rlBindShaderBuffer(ssboA, 1);
        SetShaderValue(golRenderShader, resUniformLoc, &resolution, SHADER_UNIFORM_VEC2);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLANK);

            BeginShaderMode(golRenderShader);
                DrawTexture(whiteTex, 0, 0, WHITE);
            EndShaderMode();
            
            DrawRectangleLines(GetMouseX() - brushSize/2, GetMouseY() - brushSize/2, brushSize, brushSize, RED);

            DrawText("Use Mouse wheel to increase/decrease brush size", 10, 10, 20, WHITE);
            DrawFPS(GetScreenWidth() - 100, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload shader buffers objects.
    rlUnloadShaderBuffer(ssboA);
    rlUnloadShaderBuffer(ssboB);
    rlUnloadShaderBuffer(transfertSSBO);

    // Unload compute shader programs
    rlUnloadShaderProgram(golTransfertProgram);
    rlUnloadShaderProgram(golLogicProgram);

    UnloadTexture(whiteTex);            // Unload white texture
    UnloadShader(golRenderShader);      // Unload rendering fragment shader

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
