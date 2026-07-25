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

    Font fontPixantiquaEmBox = LoadFontEx("resources/pixantiqua.ttf", 16, NULL, 0);
    Font fontAnonymousEmBox = LoadFontEx("resources/anonymous_pro_bold.ttf", 16, NULL, 0);
    Font fontDotGothicEmBox = LoadFontEx("resources/DotGothic16-Regular.ttf", 16, NULL, 0);

    // Other fonts can use the default Ascent scaling metric.
    SetFontSizeMetric(FONT_SIZE_METRIC_ASCENT);
    Font fontPixantiquaAscent = LoadFontEx("resources/pixantiqua.ttf", 64, NULL, 0);
    Font fontAnonymousAscent = LoadFontEx("resources/anonymous_pro_bold.ttf", 32, NULL, 0);
    Font fontDotGothicAscent = LoadFontEx("resources/DotGothic16-Regular.ttf", 32, NULL, 0);

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

            // Headers inside render texture
            DrawText("Em Box", 30, 10, 20, LIGHTGRAY);
            DrawText("Ascent", 270, 10, 20, LIGHTGRAY);

            DrawLine(240, 0, 240, 270, GRAY);
            DrawLine(10, 35, 470, 35, GRAY);

            // Render fonts with Em Box scaling metric (Left column)
            DrawTextEx(fontPixantiquaEmBox, "Pixantiqua 16px", (Vector2){ 30, 50 }, 16.0f, 1, WHITE);
            DrawTextEx(fontAnonymousEmBox, "Anonymous Pro", (Vector2){ 30, 110 }, 16.0f, 1, WHITE);
            DrawTextEx(fontDotGothicEmBox, "DotGothic16", (Vector2){ 30, 170 }, 16.0f, 1, WHITE);

            // Render fonts with Ascent scaling metric (Right column)
            DrawTextEx(fontPixantiquaAscent, "Pixantiqua 16px", (Vector2){ 270, 50 }, 16.0f, 1, WHITE);
            DrawTextEx(fontAnonymousAscent, "Anonymous Pro", (Vector2){ 270, 110 }, 16.0f, 1, WHITE);
            DrawTextEx(fontDotGothicAscent, "DotGothic16", (Vector2){ 270, 170 }, 16.0f, 1, WHITE);
        EndTextureMode();

        // Render target texture stretched to the main window
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(target.texture,
                           (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                           (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                           (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(fontPixantiquaEmBox);
    UnloadFont(fontAnonymousEmBox);
    UnloadFont(fontDotGothicEmBox);

    UnloadFont(fontPixantiquaAscent);
    UnloadFont(fontAnonymousAscent);
    UnloadFont(fontDotGothicAscent);

    UnloadRenderTexture(target);

    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
