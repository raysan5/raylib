/*******************************************************************************************
*
*   raylib [textures] example - cellular automata
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 5.6, last time updated with raylib 5.6
*
*   Example contributed by Jordi Santonja (@JordSant) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Jordi Santonja (@JordSant)
*
********************************************************************************************/

#include "raylib.h"

// Initialization constants
//--------------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;
const int imageWidth = 800;
const int imageHeight = 800/2;

// Rule button sizes and positions
const int drawRuleStartX       = 585;
const int drawRuleStartY       = 10;
const int drawRuleSpacing      = 15;
const int drawRuleGroupSpacing = 50;
const int drawRuleSize         = 14;
const int drawRuleInnerSize    = 10;

// Preset button sizes
const int presetsSizeX = 42;
const int presetsSizeY = 22;

const int linesUpdatedPerFrame = 4;

//----------------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------------
void ComputeLine(Image *image, int line, int rule)
{
    // Compute next line pixels. Boundaries are not computed, always 0
    for (int i = 1; i < imageWidth - 1; i++)
    {
        // Get, from the previous line, the 3 pixels states as a binary value
        const int prevValue = ((GetImageColor(*image, i - 1, line - 1).r < 5)? 4 : 0) +     // Left pixel
                              ((GetImageColor(*image, i,     line - 1).r < 5)? 2 : 0) +     // Center pixel
                              ((GetImageColor(*image, i + 1, line - 1).r < 5)? 1 : 0);      // Right pixel
        // Get next value from rule bitmask
        const bool currValue = (rule & (1 << prevValue));
        // Update pixel color
        ImageDrawPixel(image, i, line, (currValue)? BLACK : RAYWHITE);
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [textures] example - cellular automata");

    // Image that contains the cellular automaton
    Image image = GenImageColor(imageWidth, imageHeight, RAYWHITE);
    // The top central pixel set as black
    ImageDrawPixel(&image, imageWidth/2, 0, BLACK);

    Texture2D texture = LoadTextureFromImage(image);

    // Some interesting rules
    const int presetValues[] = { 18, 30, 60, 86, 102, 124, 126, 150, 182, 225 };
    const int presetsCount = sizeof(presetValues)/sizeof(presetValues[0]);

    // Variables
    int rule = 30;  // Starting rule
    int line = 1;   // Line to compute, starting from line 1. One point in line 0 is already set

    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Handle mouse
        const Vector2 mouse = GetMousePosition();
        int mouseInCell = -1;   // -1: outside any button; 0-7: rule cells; 8+: preset cells

        // Check mouse on rule cells
        for (int i = 0; i < 8; i++)
        {
            const int cellX = drawRuleStartX - drawRuleGroupSpacing*i + drawRuleSpacing;
            const int cellY = drawRuleStartY + drawRuleSpacing;
            if ((mouse.x >= cellX) && (mouse.x <= cellX + drawRuleSize) &&
                (mouse.y >= cellY) && (mouse.y <= cellY + drawRuleSize))
            {
                mouseInCell = i;    // 0-7: rule cells
                break;
            }
        }

        // Check mouse on preset cells
        if (mouseInCell < 0)
        {
            for (int i = 0; i < presetsCount; i++)
            {
                const int cellX = 4 + (presetsSizeX + 2)*(i/2);
                const int cellY = 2 + (presetsSizeY + 2)*(i%2);
                if ((mouse.x >= cellX) && (mouse.x <= cellX + presetsSizeX) &&
                    (mouse.y >= cellY) && (mouse.y <= cellY + presetsSizeY))
                {
                    mouseInCell = i + 8;    // 8+: preset cells
                    break;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (mouseInCell >= 0))
        {
            // Rule changed both by selecting a preset or toggling a bit
            if (mouseInCell < 8)
                rule ^= (1 << mouseInCell);
            else
                rule = presetValues[mouseInCell - 8];

            // Reset image
            ImageClearBackground(&image, RAYWHITE);
            ImageDrawPixel(&image, imageWidth/2, 0, BLACK);
            line = 1;
        }

        // Compute next lines
        //----------------------------------------------------------------------------------
        if (line < imageHeight)
        {
            for (int i = 0; (i < linesUpdatedPerFrame) && (line + i < imageHeight); i++)
                ComputeLine(&image, line + i, rule);
            line += linesUpdatedPerFrame;

            UpdateTexture(texture, image.data);
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw cellular automaton texture
            DrawTexture(texture, 0, screenHeight - imageHeight, WHITE);

            // Draw preset values
            for (int i = 0; i < presetsCount; i++)
            {
                DrawText(TextFormat("%i", presetValues[i]), 8 + (presetsSizeX + 2)*(i/2), 4 + (presetsSizeY + 2)*(i%2), 20, GRAY);
                DrawRectangleLines(4 + (presetsSizeX + 2)*(i/2), 2 + (presetsSizeY + 2)*(i%2), presetsSizeX, presetsSizeY, BLUE);

                // If the mouse is on this preset, highlight it
                if (mouseInCell == i + 8)
                    DrawRectangleLinesEx((Rectangle) { 2 + (presetsSizeX + 2.0f)*(i/2),
                                                       (presetsSizeY + 2.0f)*(i%2),
                                                       presetsSizeX + 4.0f, presetsSizeY + 4.0f }, 3, RED);
            }

            // Draw rule bits
            for (int i = 0; i < 8; i++)
            {
                // The three input bits
                for (int j = 0; j < 3; j++)
                {
                    DrawRectangleLines(drawRuleStartX - drawRuleGroupSpacing*i + drawRuleSpacing*j, drawRuleStartY, drawRuleSize, drawRuleSize, GRAY);
                    if (i & (4 >> j))
                        DrawRectangle(drawRuleStartX + 2 - drawRuleGroupSpacing*i + drawRuleSpacing*j, drawRuleStartY + 2, drawRuleInnerSize, drawRuleInnerSize, BLACK);
                }

                // The output bit
                DrawRectangleLines(drawRuleStartX - drawRuleGroupSpacing*i + drawRuleSpacing, drawRuleStartY + drawRuleSpacing, drawRuleSize, drawRuleSize, BLUE);
                if (rule & (1 << i))
                    DrawRectangle(drawRuleStartX + 2 - drawRuleGroupSpacing*i + drawRuleSpacing, drawRuleStartY + 2 + drawRuleSpacing, drawRuleInnerSize, drawRuleInnerSize, BLACK);

                // If the mouse is on this rule bit, highlight it
                if (mouseInCell == i)
                    DrawRectangleLinesEx((Rectangle){ drawRuleStartX - drawRuleGroupSpacing*i + drawRuleSpacing - 2.0f,
                                                      drawRuleStartY + drawRuleSpacing - 2.0f,
                                                      drawRuleSize + 4.0f, drawRuleSize + 4.0f }, 3, RED);
            }

            DrawText(TextFormat("RULE: %i", rule), drawRuleStartX + drawRuleSpacing*4, drawRuleStartY + 1, 30, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadImage(image);
    UnloadTexture(texture);

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

