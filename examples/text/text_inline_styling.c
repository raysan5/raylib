/*******************************************************************************************
*
*   raylib [text] example - inline styling
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 5.6-dev, last time updated with raylib 5.6-dev
*
*   Example contributed by Wagner Barongello (@SultansOfCode) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 Wagner Barongello (@SultansOfCode) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: strtoul(), NULL

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void DrawTextStyled(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color color);
static Vector2 MeasureTextStyled(Font font, const char *text, float fontSize, float spacing);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - inline styling");

    Vector2 textSize = { 0 };   // Measure text box for provided font and text
    Color colRandom = RED;      // Random color used on text
    int frameCounter = 0;       // Used to generate a new random color every certain frames

    SetTargetFPS(60);           // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frameCounter++;

        if ((frameCounter%20) == 0)
        {
            colRandom.r = (unsigned char)GetRandomValue(0, 255);
            colRandom.g = (unsigned char)GetRandomValue(0, 255);
            colRandom.b = (unsigned char)GetRandomValue(0, 255);
            colRandom.a = 255;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Text inline styling strategy used: [ ] delimiters for format
            // - Define foreground color:      [cRRGGBBAA]
            // - Define background color:      [bRRGGBBAA]
            // - Reset formating:              [r]
            // Colors defined with [cRRGGBBAA] or [bRRGGBBAA] are multiplied by the base color alpha
            // This allows global transparency control while keeping per-section styling (ex. text fade effects)
            // Example: [bAA00AAFF][cFF0000FF]red text on gray background[r] normal text

            DrawTextStyled(GetFontDefault(), "This changes the [cFF0000FF]foreground color[r] of provided text!!!",
                (Vector2){ 100, 80 }, 20.0f, 2.0f, BLACK);

            DrawTextStyled(GetFontDefault(), "This changes the [bFF00FFFF]background color[r] of provided text!!!",
                (Vector2){ 100, 120 }, 20.0f, 2.0f, BLACK);

            DrawTextStyled(GetFontDefault(), "This changes the [c00ff00ff][bff0000ff]foreground and background colors[r]!!!",
                (Vector2){ 100, 160 }, 20.0f, 2.0f, BLACK);

            DrawTextStyled(GetFontDefault(), "This changes the [c00ff00ff]alpha[r] relative [cffffffff][b000000ff]from source[r] [cff000088]color[r]!!!",
                (Vector2){ 100, 200 }, 20.0f, 2.0f, (Color){ 0, 0, 0, 100 });

            // Get pointer to formated text
            const char *text = TextFormat("Let's be [c%02x%02x%02xFF]CREATIVE[r] !!!", colRandom.r, colRandom.g, colRandom.b);
            DrawTextStyled(GetFontDefault(), text, (Vector2){ 100, 240 }, 40.0f, 2.0f, BLACK);

            textSize = MeasureTextStyled(GetFontDefault(), text, 40.0f, 2.0f);
            DrawRectangleLines(100, 240, (int)textSize.x, (int)textSize.y, GREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Draw text using inline styling
// PARAM: color is the default text color, background color is BLANK by default
// NOTE: Using input color as the base alpha multiplied to inline styles
static void DrawTextStyled(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color color)
{
    // Text inline styling strategy used: [ ] delimiters for format
    // - Define foreground color:      [cRRGGBBAA]
    // - Define background color:      [bRRGGBBAA]
    // - Reset formating:              [r]
    // Example: [bAA00AAFF][cFF0000FF]red text on gray background[r] normal text

    if (font.texture.id == 0) font = GetFontDefault();

    int textLen = TextLength(text);

    Color colFront = color;
    Color colBack = BLANK;
    int backRecPadding = 4; // Background rectangle padding

    float textOffsetY = 0.0f;
    float textOffsetX = 0.0f;
    float textLineSpacing = 0.0f;
    float scaleFactor = fontSize/font.baseSize;

    for (int i = 0; i < textLen;)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);

        if (codepoint == '\n')
        {
            textOffsetY += (fontSize + textLineSpacing);
            textOffsetX = 0.0f;
        }
        else
        {
            if (codepoint == '[') // Process pipe styling
            {
                if (((i + 2) < textLen) && (text[i + 1] == 'r') && (text[i + 2] == ']')) // Reset styling
                {
                    colFront = color;
                    colBack = BLANK;

                    i += 3;     // Skip "[r]"
                    continue;   // Do not draw characters
                }
                else if (((i + 1) < textLen) && ((text[i + 1] == 'c') || (text[i + 1] == 'b')))
                {
                    i += 2;     // Skip "[c" or "[b" to start parsing color

                    // Parse following color
                    char colHexText[9] = { 0 };
                    const char *textPtr = &text[i]; // Color should start here, let's see...

                    int colHexCount = 0;
                    while ((textPtr != NULL) && (textPtr[colHexCount] != '\0') && (textPtr[colHexCount] != ']'))
                    {
                        if (((textPtr[colHexCount] >= '0') && (textPtr[colHexCount] <= '9')) ||
                            ((textPtr[colHexCount] >= 'A') && (textPtr[colHexCount] <= 'F')) ||
                            ((textPtr[colHexCount] >= 'a') && (textPtr[colHexCount] <= 'f')))
                        {
                            colHexText[colHexCount] = textPtr[colHexCount];
                            colHexCount++;
                        }
                        else break; // Only affects while loop
                    }

                    // Convert hex color text into actual Color
                    unsigned int colHexValue = strtoul(colHexText, NULL, 16);
                    if (text[i - 1] == 'c')
                    {
						colFront = GetColor(colHexValue);
						colFront.a = (unsigned char)(colFront.a * (float)color.a/255.0f);
					}
                    else if (text[i - 1] == 'b')
					{
						colBack = GetColor(colHexValue);
						colBack.a *= (unsigned char)(colFront.a * (float)color.a / 255.0f);
					}

                    i += (colHexCount + 1); // Skip color value retrieved and ']'
                    continue;   // Do not draw characters
                }
            }

            int index = GetGlyphIndex(font, codepoint);
            float increaseX = 0.0f;

            if (font.glyphs[index].advanceX == 0) increaseX = ((float)font.recs[index].width*scaleFactor + spacing);
            else increaseX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);

            // Draw background rectangle color (if required)
            if (colBack.a > 0) DrawRectangleRec((Rectangle) { position.x + textOffsetX, position.y + textOffsetY - backRecPadding, increaseX, fontSize + 2*backRecPadding }, colBack);

            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font, codepoint, (Vector2){ position.x + textOffsetX, position.y + textOffsetY }, fontSize, colFront);
            }

            textOffsetX += increaseX;
        }

        i += codepointByteCount;
    }
}

// Measure inline styled text
// NOTE: Measuring styled text requires skipping styling data
// WARNING: Not considering line breaks
static Vector2 MeasureTextStyled(Font font, const char *text, float fontSize, float spacing)
{
    Vector2 textSize = { 0 };

    if ((font.texture.id == 0) || (text == NULL) || (text[0] == '\0')) return textSize; // Security check

    int textLen = TextLength(text); // Get size in bytes of text
    //float textLineSpacing = fontSize*1.5f; // Not used...

    float textWidth = 0.0f;
    float textHeight = fontSize;
    float scaleFactor = fontSize/(float)font.baseSize;

    int codepoint = 0;              // Current character
    int index = 0;                  // Index position in sprite font
    int validCodepointCounter = 0;

    for (int i = 0; i < textLen;)
    {
        int codepointByteCount = 0;
        codepoint = GetCodepointNext(&text[i], &codepointByteCount);

        if (codepoint == '[') // Ignore pipe inline styling
        {
            if (((i + 2) < textLen) && (text[i + 1] == 'r') && (text[i + 2] == ']')) // Reset styling
            {
                i += 3;     // Skip "[r]"
                continue;   // Do not measure characters
            }
            else if (((i + 1) < textLen) && ((text[i + 1] == 'c') || (text[i + 1] == 'b')))
            {
                i += 2;     // Skip "[c" or "[b" to start parsing color

                const char *textPtr = &text[i]; // Color should start here, let's see...

                int colHexCount = 0;
                while ((textPtr != NULL) && (textPtr[colHexCount] != '\0') && (textPtr[colHexCount] != ']'))
                {
                    if (((textPtr[colHexCount] >= '0') && (textPtr[colHexCount] <= '9')) ||
                        ((textPtr[colHexCount] >= 'A') && (textPtr[colHexCount] <= 'F')) ||
                        ((textPtr[colHexCount] >= 'a') && (textPtr[colHexCount] <= 'f')))
                    {
                        colHexCount++;
                    }
                    else break; // Only affects while loop
                }

                i += (colHexCount + 1); // Skip color value retrieved and ']'
                continue;   // Do not measure characters
            }
        }
        else if (codepoint != '\n')
        {
            index = GetGlyphIndex(font, codepoint);

            if (font.glyphs[index].advanceX > 0) textWidth += font.glyphs[index].advanceX;
            else textWidth += (font.recs[index].width + font.glyphs[index].offsetX);

            validCodepointCounter++;
            i += codepointByteCount;
        }
    }

    textSize.x = textWidth*scaleFactor + (validCodepointCounter - 1)*spacing;
    textSize.y = textHeight;

    return textSize;
}
