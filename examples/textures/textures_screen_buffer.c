/*******************************************************************************************
*
*   raylib [textures] example - screen buffer
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by Agnis Aldiņš (@nezvers) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Agnis Aldiņš (@nezvers)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: calloc(), free()

#define MAX_COLORS      256
#define SCALE_FACTOR      2

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] example - screen buffer");

    int imageWidth = screenWidth/SCALE_FACTOR;
    int imageHeight = screenHeight/SCALE_FACTOR;
    int flameWidth = screenWidth/SCALE_FACTOR;

    Color palette[MAX_COLORS] = { 0 };
    unsigned char *indexBuffer = RL_CALLOC(imageWidth*imageWidth, sizeof(unsigned char));
    unsigned char *flameRootBuffer = RL_CALLOC(flameWidth, sizeof(unsigned char));

    Image screenImage = GenImageColor(imageWidth, imageHeight, BLACK);
    Texture screenTexture = LoadTextureFromImage(screenImage);

    // Generate flame color palette
    for (int i = 0; i < MAX_COLORS; i++)
    {
        float t = (float)i/(float)(MAX_COLORS - 1);
        float hue = t*t;
        float saturation = t;
        float value = t;
        palette[i] = ColorFromHSV(250.0f + 150.0f*hue, saturation, value);
    }

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Grow flameRoot
        for (int x = 2; x < flameWidth; x++)
        {
            unsigned char flame = flameRootBuffer[x];
            if (flame == 255) continue;
            flame += GetRandomValue(0, 2);
            if (flame > 255) flame = 255;
            flameRootBuffer[x] = flame;
        }

        // Transfer flameRoot to indexBuffer
        for (int x = 0; x < flameWidth; x++)
        {
            int i = x + (imageHeight - 1)*imageWidth;
            indexBuffer[i] = flameRootBuffer[x];
        }

        // Clear top row, because it can't move any higher
        for (int x = 0; x < imageWidth; x++)
        {
            if (indexBuffer[x] == 0) continue;
            indexBuffer[x] = 0;
        }

        // Skip top row, it is already cleared
        for (int y = 1; y < imageHeight; y++)
        {
            for (int x = 0; x < imageWidth; x++)
            {
                unsigned int i = x + y*imageWidth;
                unsigned char colorIndex = indexBuffer[i];
                if (colorIndex == 0) continue;

                // Move pixel a row above
                indexBuffer[i] = 0;
                int moveX = GetRandomValue(0, 2) - 1;
                int newX = x + moveX;
                if (newX < 0 || newX >= imageWidth) continue;

                unsigned int iabove = i - imageWidth + moveX;
                int decay = GetRandomValue(0, 3);
                colorIndex -= (decay < colorIndex)? decay : colorIndex;
                indexBuffer[iabove] = colorIndex;
            }
        }

        // Update screenImage with palette colors
        for (int y = 1; y < imageHeight; y++)
        {
            for (int x = 0; x < imageWidth; x++)
            {
                unsigned int i = x + y*imageWidth;
                unsigned char colorIndex = indexBuffer[i];
                Color col = palette[colorIndex];
                ImageDrawPixel(&screenImage, x, y, col);
            }
        }

        UpdateTexture(screenTexture, screenImage.data);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            
            ClearBackground(RAYWHITE);

            DrawTextureEx(screenTexture, (Vector2){ 0, 0 }, 0.0f, 2.0f, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    RL_FREE(indexBuffer);
    RL_FREE(flameRootBuffer);
    UnloadTexture(screenTexture);
    UnloadImage(screenImage);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
