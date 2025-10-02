/*******************************************************************************************
 *
 *   raylib [shapes] example - interactive pie chart
 *
 *   Example complexity rating: [★★☆☆] 2/4
 *
 *   This example has been created using raylib 5.5 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
 *
 *   Copyright (c) 2025 Ramon Santamaria (@raysan5) and Gideon Serfontein (@GideonSerf)
 *
 ********************************************************************************************/

#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    const int MAX_SLICES = 10;

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - interactive pie chart");

#define MAX_SLICES 10
    int sliceCount = 7;
    float values[MAX_SLICES] = {300.0f, 100.0f, 450.0f, 350.0f, 600.0f, 380.0f, 750.0f}; //initial slice values
    char labels[MAX_SLICES][32];
    bool editingLabel[MAX_SLICES] = {false};

    for (int i = 0; i < MAX_SLICES; i++)
        snprintf(labels[i], 32, "Slice %i", i + 1);

    bool showValues = true;
    bool showPercentages = false;
    int hoveredSlice = -1;
    Rectangle scrollPanelBounds = {0};
    Vector2 scrollContentOffset = {0};
    Rectangle view = {0};

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        //UI layout parameters
        const int panelWidth = 270;
        const int panelMargin = 5;

        // UI Panel top-left anchor
        const Vector2 panelPos = {
            (float)screenWidth  - panelMargin - panelWidth,
            (float)panelMargin
        };

        // UI Panel rectangle
        const Rectangle panelRect = {
            panelPos.x, panelPos.y,
            (float)panelWidth,
            (float)screenHeight - 2.0f*panelMargin
        };

        // Pie chart geometry
        const Rectangle canvas = { 0, 0, panelPos.x, (float)screenHeight };
        const Vector2 center = {canvas.width / 2.0f, canvas.height / 2.0f};
        const float radius = 205.0f;

        // Calculate total value for percentage calculations
        float totalValue = 0.0f;
        for (int i = 0; i < sliceCount; i++)
            totalValue += values[i];

        // Check for mouse hover over slices
        hoveredSlice = -1; // Reset hovered slice
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, canvas)) // Only check if mouse is inside the canvas
        {
            float dx = mousePos.x - center.x;
            float dy = mousePos.y - center.y;
            float distance = sqrtf(dx * dx + dy * dy);

            if (distance <= radius) // Inside the pie radius
            {
                float angle = atan2f(dy, dx) * RAD2DEG;
                if (angle < 0)
                    angle += 360;

                float currentAngle = 0.0f;
                for (int i = 0; i < sliceCount; i++)
                {
                    float sweep = (totalValue > 0) ? (values[i] / totalValue) * 360.0f : 0.0f;
                    if (angle >= currentAngle && angle < (currentAngle + sweep))
                    {
                        hoveredSlice = i;
                        break;
                    }
                    currentAngle += sweep;
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw the pie chart on the canvas
        //------------------------------------------------------------------------------
        float startAngle = 0.0f;
        for (int i = 0; i < sliceCount; i++)
        {
            float sweepAngle = (totalValue > 0) ? (values[i] / totalValue) * 360.0f : 0.0f;
            float midAngle = startAngle + sweepAngle / 2.0f; // Middle angle for label positioning

            Color color = ColorFromHSV((float)i / sliceCount * 360.0f, 0.75f, 0.9f);
            float currentRadius = radius;

            // Make the hovered slice pop out by adding 5 pixels to its radius
            if (i == hoveredSlice)
                currentRadius += 5.0f;

            // Draw the pie slice using raylib's DrawCircleSector function
            DrawCircleSector(center, currentRadius, startAngle, startAngle + sweepAngle, 120, color);

            // Draw the label for the current slice
            if (values[i] > 0)
            {
                char labelText[64];
                if (showValues && showPercentages)
                    snprintf(labelText, 64, "%.1f (%.0f%%)", values[i], (values[i] / totalValue) * 100.0f);
                else if (showValues)
                    snprintf(labelText, 64, "%.1f", values[i]);
                else if (showPercentages)
                    snprintf(labelText, 64, "%.0f%%", (values[i] / totalValue) * 100.0f);
                else
                    labelText[0] = '\0';

                Vector2 textSize = MeasureTextEx(GetFontDefault(), labelText, 18, 1);
                float labelRadius = radius * 0.7f;
                Vector2 labelPos = {
                    center.x + cosf(midAngle * DEG2RAD) * labelRadius - textSize.x / 2,
                    center.y + sinf(midAngle * DEG2RAD) * labelRadius - textSize.y / 2};
                DrawText(labelText, (int)labelPos.x, (int)labelPos.y, 18, WHITE);
            }

            startAngle += sweepAngle;
        }
        //------------------------------------------------------------------------------

        // UI control panel
        //------------------------------------------------------------------------------
        DrawRectangleRec(panelRect, Fade(LIGHTGRAY, 0.5f));
        DrawRectangleLinesEx(panelRect, 1.0f, GRAY);

        int currentY = (int)panelPos.y + 12; // Start a bit lower for margin

        GuiSpinner((Rectangle){ panelPos.x + 95, (float)currentY, 125, 25 }, "Slices ", &sliceCount, 1, MAX_SLICES, false);
        currentY += 40;

        GuiCheckBox((Rectangle){ panelPos.x + 20, (float)currentY, 20, 20 }, "Show Values", &showValues);
        currentY += 30;

        GuiCheckBox((Rectangle){ panelPos.x + 20, (float)currentY, 20, 20 }, "Show Percentages", &showPercentages);
        currentY += 40;

        GuiLine((Rectangle){ panelPos.x + 10, (float)currentY, panelRect.width - 20, 1 }, NULL);
        currentY += 20;

        // Scrollable area for slice editors
        scrollPanelBounds = (Rectangle){ panelPos.x+panelMargin, (float)currentY, panelRect.width-panelMargin*2, panelRect.y + panelRect.height - currentY - panelMargin };
        int contentHeight = sliceCount * 35;

        GuiScrollPanel(scrollPanelBounds, NULL,
                       (Rectangle){ 0, 0, panelRect.width - 20, (float)contentHeight },
                       &scrollContentOffset, &view);

        const float contentX = view.x + scrollContentOffset.x; // left of content
        const float contentY = view.y + scrollContentOffset.y; // top of content

        BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
        for (int i = 0; i < sliceCount; i++)
        {
            const int rowY = (int)(contentY + 5 + i * 35);

            // Color indicator
            Color color = ColorFromHSV((float)i / sliceCount * 360.0f, 0.75f, 0.9f);
            DrawRectangle((int)(contentX + 15), rowY + 5, 20, 20, color);

            // Label textbox
            if (GuiTextBox((Rectangle){contentX + 45, (float)rowY, 75, 30}, labels[i], 32, editingLabel[i]))
                editingLabel[i] = !editingLabel[i];

            GuiSliderBar((Rectangle){contentX + 130, (float)rowY, 110, 30},
                         NULL, NULL, &values[i], 0.0f, 1000.0f);
        }
        EndScissorMode();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}