/*******************************************************************************************
*
*   raylib [text] example - font scaling metrics
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like open-source license that allows free use in commercial applications.
*
********************************************************************************************/

#include "raylib.h"
#include <stddef.h>

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 960;
    const int screenHeight = 540;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - font scaling metrics");

    // Some fonts need to be loaded using the Em Box scaling metric.
    // Loading pixel fonts with default metric can cause blurry rendering when drawn to render texture.
    SetFontSizeMetric(FONT_SIZE_METRIC_EM_BOX);

    Font fontM5x7 = LoadFontEx("resources/m5x7.ttf", 16, NULL, 0);
    Font fontM6x11 = LoadFontEx("resources/m6x11.ttf", 16, NULL, 0);
    Font fontMonogram = LoadFontEx("resources/monogram.ttf", 16, NULL, 0);

    // Other fonts can use the default Ascent scaling metric.
    SetFontSizeMetric(FONT_SIZE_METRIC_ASCENT);
    Font fontAutography = LoadFontEx("resources/autography.ttf", 64, NULL, 0);
    Font fontCreatorGenius = LoadFontEx("resources/creator_genius.ttf", 32, NULL, 0);
    Font fontPixelSimpel = LoadFontEx("resources/pixel_simpel.ttf", 32, NULL, 0);

    // Set up Render Texture
    RenderTexture2D target = LoadRenderTexture(480, 270);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(target);
            ClearBackground(BLACK);

            // Render fonts with Em Box scaling metric
            DrawTextEx(fontM5x7, "m5x7", (Vector2){ 50, 50 }, 32.0f, 2, WHITE);
            DrawTextEx(fontM6x11, "m6x11", (Vector2){ 50, 100 }, 32.0f, 2, WHITE);
            DrawTextEx(fontMonogram, "monogram", (Vector2){ 50, 150 }, 32.0f, 2, WHITE);

            // Render fonts with Ascent scaling metric
            DrawTextEx(fontAutography, "Autography", (Vector2){ 200, 50 }, 64.0f, 2, WHITE);
            DrawTextEx(fontCreatorGenius, "Creator Genius", (Vector2){ 200, 100 }, 32.0f, 2, WHITE);
            DrawTextEx(fontPixelSimpel, "Pixel Simpel", (Vector2){ 200, 150 }, 32.0f, 2, WHITE);
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE);

            DrawLine(180, 0, 180, 250, WHITE);
            DrawLine(50, 30, 450, 30, WHITE);

            DrawText("Em Box", 50, 0, 32, WHITE);
            DrawText("Ascent", 200, 0, 32, WHITE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(fontM5x7);
    UnloadFont(fontM6x11);
    UnloadFont(fontMonogram);

    UnloadFont(fontAutography);
    UnloadFont(fontCreatorGenius);
    UnloadFont(fontPixelSimpel);

    UnloadRenderTexture(target);

    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
