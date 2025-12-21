/*******************************************************************************************
*
*   raylib [textures] example - Test DDS loading with mipmaps
*
********************************************************************************************/

#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [textures] - DDS mipmaps test");

    // Load DDS texture with mipmaps (create one with a distinct color pattern)
    Texture2D texture = LoadTexture("resources/test_rgba_mipmaps.dds");
    
    if (texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load DDS texture");
    }
    else
    {
        TraceLog(LOG_INFO, "DDS texture loaded: %dx%d, mipmaps: %d", 
                 texture.width, texture.height, texture.mipmaps);
    }

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Draw texture at different scales to see mipmap levels
            DrawTexturePro(texture, 
                          (Rectangle){0, 0, texture.width, texture.height},
                          (Rectangle){50, 50, 256, 256}, 
                          (Vector2){0, 0}, 0.0f, WHITE);
            
            DrawTexturePro(texture,
                          (Rectangle){0, 0, texture.width, texture.height},
                          (Rectangle){350, 50, 128, 128},
                          (Vector2){0, 0}, 0.0f, WHITE);
            
            DrawTexturePro(texture,
                          (Rectangle){0, 0, texture.width, texture.height},
                          (Rectangle){520, 50, 64, 64},
                          (Vector2){0, 0}, 0.0f, WHITE);
            
            DrawText("256x256", 50, 320, 20, DARKGRAY);
            DrawText("128x128", 350, 190, 20, DARKGRAY);
            DrawText("64x64", 520, 125, 20, DARKGRAY);
            
            DrawText("If colors look wrong (blue/red swapped), mipmaps have swizzling bug", 
                     10, 400, 10, RED);
            
        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();

    return 0;
}