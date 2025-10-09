/*******************************************************************************************
 *
 *   raylib [shapes] example - math sine cosine
 *
 *   Port of the p5.js "sine and cosine" demo to raylib + raygui
 *   Sources: https://p5js.org/examples/angles-and-motion-sine-cosine/
 *
 *   Demonstrates uniform circular motion and the correspondence between
 *   the unit circle and sine / cosine graphs. Right panel contains a
 *   play/pause toggle implemented with raygui.
 * 
 *   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
 *
 *   Copyright (c) 2025 Olivier (@oqu)
 *
 ********************************************************************************************/

#include "raylib.h"

// Required for GUI controls
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <math.h>

int main(void)
{
    // Window initialization
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - math sine cosine");
    const char* playText = "Play";
    const char* pauseText = "Pause";

    // Scene layout (mimic p5 example positioning)
    const float circleX = 200.0f;
    const float circleY = 150.0f;
    const float circleRadius = 75.0f;

    const float graphX = 50.0f;
    const float graphY = 300.0f;
    const float graphAmplitude = 50.0f;
    const float graphPeriod = 300.0f; // width of the graph in pixels

    // Animation / UI state
    bool playing = true;     // play / pause toggle
    int frameCountLocal = 0; // local frame counter (used when playing)

    SetTargetFPS(60);

    // Main loop
    while (!WindowShouldClose())
    {
        // Handle GUI and input first
        BeginDrawing();

        ClearBackground(BLACK);

        // Right control panel
        float panelX = (float)GetScreenWidth() - 270.0f;
        Rectangle panel = {panelX, 10, 260, (float)GetScreenHeight() - 20};
        DrawRectangleRec(panel, Fade(LIGHTGRAY, 0.6f));
        DrawRectangleLinesEx(panel, 1, GRAY);

        int y = 20;
        int px = (int)panelX + 10;
        DrawText("Sine / Cosine Demo", px, y, 20, DARKGRAY);
        y += 34;

        // Play / Pause toggle
        DrawText("Animation:", px, y, 14, DARKGRAY);
        y += 18;
        GuiToggle((Rectangle){(float)px, y, 220, 30}, playing ? pauseText : playText, &playing);
        y += 40;

        // Small description
        DrawText("Animated demonstration of a point moving", px, y, 10, DARKGRAY);
        y += 14;
        DrawText("around the unit circle and the corresponding", px, y, 10, DARKGRAY);
        y += 14;
        DrawText("sine (red) and cosine (orange) graphs.", px, y, 10, DARKGRAY);
        y += 30;

        DrawFPS(px, GetScreenHeight() - 30);

        // Update frameCount if playing
        if (playing)
            frameCountLocal++;

        // Angle in degrees (like the p5 example)
        int angleDeg = frameCountLocal % 360;

        // Draw angle label
        Color labelCol = WHITE;
        DrawText(TextFormat("angle: %d", angleDeg), 25, 25, 20, labelCol);

        // --- Draw circle and diameters ---
        // circle outer
        DrawCircleLines((int)circleX, (int)circleY, circleRadius, Fade(GRAY, 0.6f));
        // diameters
        DrawLine((int)circleX, (int)(circleY - circleRadius), (int)circleX, (int)(circleY + circleRadius), Fade(GRAY, 0.6f));
        DrawLine((int)(circleX - circleRadius), (int)circleY, (int)(circleX + circleRadius), (int)circleY, Fade(GRAY, 0.6f));

        // Compute moving point on circle (cos for x, sin for y). Note: cos/sin in C expect radians
        float angRad = angleDeg * (PI / 180.0f);
        float pointX = circleX + circleRadius * cosf(angRad);
        float pointY = circleY - circleRadius * sinf(angRad); // minus to match p5 y-axis direction

        // line from center to point
        DrawLine((int)circleX, (int)circleY, (int)pointX, (int)pointY, Fade(GRAY, 0.6f));

        // moving points
        DrawCircleV((Vector2){pointX, pointY}, 6.0f, WHITE);
        DrawCircle((int)pointX, (int)circleY, 6.0f, ORANGE);
        DrawCircle((int)circleX, (int)pointY, 6.0f, RED);

        // --- Draw graph area axes ---
        // axes lines
        DrawLine((int)graphX, (int)graphY, (int)(graphX + graphPeriod), (int)graphY, Fade(GRAY, 0.6f));
        DrawLine((int)graphX, (int)(graphY - graphAmplitude), (int)graphX, (int)(graphY + graphAmplitude), Fade(GRAY, 0.6f));
        DrawLine((int)(graphX + graphPeriod), (int)(graphY - graphAmplitude), (int)(graphX + graphPeriod), (int)(graphY + graphAmplitude), Fade(GRAY, 0.6f));

        // labels
        DrawText("0", (int)graphX - 6, (int)(graphY + graphAmplitude + 6), 14, Fade(GRAY, 0.9f));
        DrawText("360", (int)(graphX + graphPeriod) - 12, (int)(graphY + graphAmplitude + 6), 14, Fade(GRAY, 0.9f));
        DrawText("1", (int)(graphX / 2) - 6, (int)(graphY - graphAmplitude) - 6, 14, Fade(GRAY, 0.9f));
        DrawText("0", (int)(graphX / 2) - 6, (int)(graphY)-6, 14, Fade(GRAY, 0.9f));
        DrawText("-1", (int)(graphX / 2) - 10, (int)(graphY + graphAmplitude) - 6, 14, Fade(GRAY, 0.9f));

        DrawText("cos", (int)(graphX + graphPeriod + graphX / 2) - 18, (int)(graphY - graphAmplitude) - 10, 20, ORANGE);
        DrawText("sin", (int)(graphX + graphPeriod + graphX / 2) - 18, (int)(graphY) - 10, 20, RED);

        // --- Draw cosine curve (orange) ---
        for (int t = 0; t <= 360; t++)
        {
            float x = ((float)t / 360.0f) * graphPeriod + graphX;
            float y = graphY - graphAmplitude * cosf(t * (PI / 180.0f));
            // draw small segments between consecutive points for smoother curve
            if (t > 0)
            {
                int prevT = t - 1;
                float px = ((float)prevT / 360.0f) * graphPeriod + graphX;
                float py = graphY - graphAmplitude * cosf(prevT * (PI / 180.0f));
                DrawLineEx((Vector2){px, py}, (Vector2){x, y}, 2.5f, ORANGE);
            }
        }

        // --- Draw sine curve (red) ---
        for (int t = 0; t <= 360; t++)
        {
            float x = ((float)t / 360.0f) * graphPeriod + graphX;
            float y = graphY - graphAmplitude * sinf(t * (PI / 180.0f));
            if (t > 0)
            {
                int prevT = t - 1;
                float px = ((float)prevT / 360.0f) * graphPeriod + graphX;
                float py = graphY - graphAmplitude * sinf(prevT * (PI / 180.0f));
                DrawLineEx((Vector2){px, py}, (Vector2){x, y}, 2.5f, RED);
            }
        }

        // --- Draw moving vertical line on the graph corresponding to the angle ---
        float lineX = ((float)angleDeg / 360.0f) * graphPeriod + graphX;
        DrawLine((int)lineX, (int)(graphY - graphAmplitude), (int)lineX, (int)(graphY + graphAmplitude), Fade(GRAY, 0.6f));

        // moving points on graph
        float orangeY = graphY - graphAmplitude * cosf(angRad);
        float redY = graphY - graphAmplitude * sinf(angRad);

        DrawCircle((int)lineX, (int)orangeY, 6.0f, ORANGE);
        DrawCircle((int)lineX, (int)redY, 6.0f, RED);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}