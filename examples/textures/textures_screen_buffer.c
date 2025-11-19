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

#define MAX_COLORS 256
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define SCALE_FACTOR 2
// buffer size at least for screenImage pixel count
#define INDEX_BUFFER_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT) / SCALE_FACTOR)
#define FLAME_WIDTH (SCREEN_WIDTH / SCALE_FACTOR)

static void GeneretePalette(Color *palette);
static void ClearIndexBuffer(unsigned char *buffer, int count);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = SCREEN_WIDTH;
    const int screenHeight = SCREEN_HEIGHT;
    const int pixelScale = SCALE_FACTOR;
    const int imageWidth = screenWidth / pixelScale;
    const int imageHeight = screenHeight / pixelScale;
    InitWindow(screenWidth, screenHeight, "raylib [textures] example - screen buffer");

    Color palette[MAX_COLORS] = {0};
    unsigned char indexBuffer[INDEX_BUFFER_SIZE] = {0};
    unsigned char flameRootBuffer[FLAME_WIDTH] = {0};

    Image screenImage = GenImageColor(imageWidth, imageHeight, BLACK);
    Texture screenTexture = LoadTextureFromImage(screenImage);
    GeneretePalette(palette);
    ClearIndexBuffer(indexBuffer, INDEX_BUFFER_SIZE);
    ClearIndexBuffer(flameRootBuffer, FLAME_WIDTH);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Grow flameRoot
        for (int x = 2; x < FLAME_WIDTH; ++x)
        {
            unsigned short flame = flameRootBuffer[x];
            if (flame == 255) continue;
            flame += GetRandomValue(0, 2);
            if (flame > 255) flame = 255;
            flameRootBuffer[x] = flame;
        }

        // transfer flameRoot to indexBuffer
        for (int x = 0; x < FLAME_WIDTH; ++x)
        {
            int i = x + (imageHeight - 1) * imageWidth;
            indexBuffer[i] = flameRootBuffer[x];
        }

        // Clear top row, because it can't move any higher
        for (int x = 0; x < imageWidth; ++x)
        {
            if (indexBuffer[x] == 0) continue;
            indexBuffer[x] = 0;
        }

        // Skip top row, it is already cleared
        for (int y = 1; y < imageHeight; ++y)
        {
            for (int x = 0; x < imageWidth; ++x)
            {
                unsigned i = x + y * imageWidth;
                unsigned char colorIndex = indexBuffer[i];
                if (colorIndex == 0) continue;

                // Move pixel a row above
                indexBuffer[i] = 0;
                int moveX = GetRandomValue(0, 2) - 1;
                int newX = x + moveX;
                if (newX < 0 || newX >= imageWidth) continue;

                unsigned i_above = i - imageWidth + moveX;
                int decay = GetRandomValue(0, 3);
                colorIndex -= (decay < colorIndex) ? decay : colorIndex;
                indexBuffer[i_above] = colorIndex;
            }
        }

        // Update screenImage with palette colors
        for (int y = 1; y < imageHeight; ++y)
        {
            for (int x = 0; x < imageWidth; ++x)
            {
                unsigned i = x + y * imageWidth;
                unsigned char colorIndex = indexBuffer[i];
                Color col = palette[colorIndex];
                ImageDrawPixel(&screenImage, x, y, col);
            }
        }

        UpdateTexture(screenTexture, screenImage.data);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            const Vector2 origin = (Vector2){0, 0};
            const float rotation = 0.f;
            DrawTextureEx(screenTexture, origin, rotation, pixelScale, WHITE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    UnloadTexture(screenTexture);
    UnloadImage(screenImage);

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static void GeneretePalette(Color *palette)
{
    for (int i = 0; i < MAX_COLORS; ++i)
    {
        float t = (float)i/(float)(MAX_COLORS - 1);
        float hue = t * t;
        float saturation = t;
        float value = t;
        palette[i] = ColorFromHSV(250.f + 150.f * hue, saturation, value);
    }
}

static void ClearIndexBuffer(unsigned char *buffer, int count)
{
    // Use memset to set to ZERO, but for demonstration a plain for loop is used
    for (int i = 0; i < count; ++i)
    {
        buffer[i] = 0;
    }
}