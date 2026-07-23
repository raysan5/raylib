#include "raylib.h"
#include <stddef.h>

int main(void) {

    InitWindow(960, 540, "raylib [text] example - font scaling metrics");

    // Some fonts need to be loaded using the Em Box scaling metric.
    // Loading this font with the default font size metric and then drawing it to a render texture with a point filter can cause blurry rendering
    SetFontSizeMetric(FONT_SIZE_METRIC_EM_BOX);

    Font fontm5x7     = LoadFontEx("text/resources/m5x7.ttf", 16, NULL, 0);
    Font fontm6x11    = LoadFontEx("text/resources/m6x11.ttf", 16, NULL, 0);
    Font fontMonogram = LoadFontEx("text/resources/monogram.ttf", 16, NULL, 0);

    // Other fonts can use the default Ascent scaling metric.
    // (this was the default metric, but didn't work with some pixel fonts, so now you have the option to choose which metric to use for your font)
    SetFontSizeMetric(FONT_SIZE_METRIC_ASCENT);
    Font fontCenturyGothic = LoadFontEx("text/resources/centurygothic.ttf", 32, NULL, 0);
    Font fontComicSansMS   = LoadFontEx("text/resources/Comic Sans MS.ttf", 32, NULL, 0);
    Font fontAutography    = LoadFontEx("text/resources/Autography.ttf", 64, NULL, 0);

    // Set up Render Texture
    RenderTexture2D target = LoadRenderTexture(480, 270);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

    // Application loop
    SetTargetFPS(60);
    
    while(!WindowShouldClose()) {

        BeginTextureMode(target);
        ClearBackground(BLACK);

        // Render fonts with Em Box scaling metric
        DrawTextEx(fontm5x7, "m5x7", (Vector2) { 50, 50 }, 32.0f, 2, WHITE);
        DrawTextEx(fontm6x11, "m6x11", (Vector2) { 50, 100 }, 32.0f, 2, WHITE);
        DrawTextEx(fontMonogram, "monogram", (Vector2) { 50, 150 }, 32.0f, 2, WHITE);

	// Render fonts with Ascent scaling metric
        DrawTextEx(fontCenturyGothic, "Century Gothic", (Vector2) { 200, 50 }, 32.0f, 2, WHITE);
        DrawTextEx(fontComicSansMS, "Comic Sans MS", (Vector2) { 200, 100 }, 32.0f, 2, WHITE);
        DrawTextEx(fontAutography, "Autography", (Vector2) { 200, 150 }, 64.0f, 2, WHITE);

        EndTextureMode();
        
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureRec(target.texture, (Rectangle){ 0, 0, target.texture.width, -target.texture.height }, (Vector2){ 0, 0 }, WHITE);

	DrawLine(180, 0, 180, 250, WHITE);
	DrawLine(50, 30, 450, 30, WHITE);

	DrawText("Em Box", 50, 0, 32, WHITE);
	DrawText("Ascent", 200, 0, 32, WHITE);

        EndDrawing();
    }

    // Cleanup
    UnloadFont(fontm5x7);
    UnloadFont(fontm6x11);
    UnloadFont(fontMonogram);

    UnloadFont(fontCenturyGothic);
    UnloadFont(fontComicSansMS);
    UnloadFont(fontAutography);

    UnloadRenderTexture(target);

    CloseWindow();

    return 0;
}
