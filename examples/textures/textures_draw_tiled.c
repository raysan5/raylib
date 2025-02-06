/*******************************************************************************************
*
*   raylib [textures] example - Draw part of the texture tiled
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 3.0, last time updated with raylib 4.2
*
*   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2020-2025 Vlad Adrian (@demizdor) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define SIZEOF(A) (sizeof(A)/sizeof(A[0]))
#define OPT_WIDTH       220       // Max width for the options container
#define MARGIN_SIZE       8       // Size for the margins
#define COLOR_SIZE       16       // Size of the color select buttons

// Draw part of a texture (defined by a rectangle) with rotation and scale tiled into dest.
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE); // Make the window resizable
    InitWindow(screenWidth, screenHeight, "raylib [textures] example - Draw part of a texture tiled");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture texPattern = LoadTexture("resources/patterns.png");
    SetTextureFilter(texPattern, TEXTURE_FILTER_TRILINEAR); // Makes the texture smoother when upscaled

    // Coordinates for all patterns inside the texture
    const Rectangle recPattern[] = {
        (Rectangle){ 3, 3, 66, 66 },
        (Rectangle){ 75, 3, 100, 100 },
        (Rectangle){ 3, 75, 66, 66 },
        (Rectangle){ 7, 156, 50, 50 },
        (Rectangle){ 85, 106, 90, 45 },
        (Rectangle){ 75, 154, 100, 60}
    };

    // Setup colors
    const Color colors[] = { BLACK, MAROON, ORANGE, BLUE, PURPLE, BEIGE, LIME, RED, DARKGRAY, SKYBLUE };
    enum { MAX_COLORS = SIZEOF(colors) };
    Rectangle colorRec[MAX_COLORS] = { 0 };

    // Calculate rectangle for each color
    for (int i = 0, x = 0, y = 0; i < MAX_COLORS; i++)
    {
        colorRec[i].x = 2.0f + MARGIN_SIZE + x;
        colorRec[i].y = 22.0f + 256.0f + MARGIN_SIZE + y;
        colorRec[i].width = COLOR_SIZE*2.0f;
        colorRec[i].height = (float)COLOR_SIZE;

        if (i == (MAX_COLORS/2 - 1))
        {
            x = 0;
            y += COLOR_SIZE + MARGIN_SIZE;
        }
        else x += (COLOR_SIZE*2 + MARGIN_SIZE);
    }

    int activePattern = 0, activeCol = 0;
    float scale = 1.0f, rotation = 0.0f;

    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Handle mouse
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            const Vector2 mouse = GetMousePosition();

            // Check which pattern was clicked and set it as the active pattern
            for (int i = 0; i < SIZEOF(recPattern); i++)
            {
                if (CheckCollisionPointRec(mouse, (Rectangle){ 2 + MARGIN_SIZE + recPattern[i].x, 40 + MARGIN_SIZE + recPattern[i].y, recPattern[i].width, recPattern[i].height }))
                {
                    activePattern = i;
                    break;
                }
            }

            // Check to see which color was clicked and set it as the active color
            for (int i = 0; i < MAX_COLORS; ++i)
            {
                if (CheckCollisionPointRec(mouse, colorRec[i]))
                {
                    activeCol = i;
                    break;
                }
            }
        }

        // Handle keys

        // Change scale
        if (IsKeyPressed(KEY_UP)) scale += 0.25f;
        if (IsKeyPressed(KEY_DOWN)) scale -= 0.25f;
        if (scale > 10.0f) scale = 10.0f;
        else if ( scale <= 0.0f) scale = 0.25f;

        // Change rotation
        if (IsKeyPressed(KEY_LEFT)) rotation -= 25.0f;
        if (IsKeyPressed(KEY_RIGHT)) rotation += 25.0f;

        // Reset
        if (IsKeyPressed(KEY_SPACE)) { rotation = 0.0f; scale = 1.0f; }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw the tiled area
            DrawTextureTiled(texPattern, recPattern[activePattern], (Rectangle){(float)OPT_WIDTH+MARGIN_SIZE, (float)MARGIN_SIZE, GetScreenWidth() - OPT_WIDTH - 2.0f*MARGIN_SIZE, GetScreenHeight() - 2.0f*MARGIN_SIZE},
                (Vector2){0.0f, 0.0f}, rotation, scale, colors[activeCol]);

            // Draw options
            DrawRectangle(MARGIN_SIZE, MARGIN_SIZE, OPT_WIDTH - MARGIN_SIZE, GetScreenHeight() - 2*MARGIN_SIZE, ColorAlpha(LIGHTGRAY, 0.5f));

            DrawText("Select Pattern", 2 + MARGIN_SIZE, 30 + MARGIN_SIZE, 10, BLACK);
            DrawTexture(texPattern, 2 + MARGIN_SIZE, 40 + MARGIN_SIZE, BLACK);
            DrawRectangle(2 + MARGIN_SIZE + (int)recPattern[activePattern].x, 40 + MARGIN_SIZE + (int)recPattern[activePattern].y, (int)recPattern[activePattern].width, (int)recPattern[activePattern].height, ColorAlpha(DARKBLUE, 0.3f));

            DrawText("Select Color", 2+MARGIN_SIZE, 10+256+MARGIN_SIZE, 10, BLACK);
            for (int i = 0; i < MAX_COLORS; i++)
            {
                DrawRectangleRec(colorRec[i], colors[i]);
                if (activeCol == i) DrawRectangleLinesEx(colorRec[i], 3, ColorAlpha(WHITE, 0.5f));
            }

            DrawText("Scale (UP/DOWN to change)", 2 + MARGIN_SIZE, 80 + 256 + MARGIN_SIZE, 10, BLACK);
            DrawText(TextFormat("%.2fx", scale), 2 + MARGIN_SIZE, 92 + 256 + MARGIN_SIZE, 20, BLACK);

            DrawText("Rotation (LEFT/RIGHT to change)", 2 + MARGIN_SIZE, 122 + 256 + MARGIN_SIZE, 10, BLACK);
            DrawText(TextFormat("%.0f degrees", rotation), 2 + MARGIN_SIZE, 134 + 256 + MARGIN_SIZE, 20, BLACK);

            DrawText("Press [SPACE] to reset", 2 + MARGIN_SIZE, 164 + 256 + MARGIN_SIZE, 10, DARKBLUE);

            // Draw FPS
            DrawText(TextFormat("%i FPS", GetFPS()), 2 + MARGIN_SIZE, 2 + MARGIN_SIZE, 20, BLACK);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texPattern);        // Unload texture

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Draw part of a texture (defined by a rectangle) with rotation and scale tiled into dest.
void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if ((texture.id <= 0) || (scale <= 0.0f)) return;  // Wanna see a infinite loop?!...just delete this line!
    if ((source.width == 0) || (source.height == 0)) return;

    int tileWidth = (int)(source.width*scale), tileHeight = (int)(source.height*scale);
    if ((dest.width < tileWidth) && (dest.height < tileHeight))
    {
        // Can fit only one tile
        DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                    (Rectangle){dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tileWidth)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tileHeight < dest.height; dy += tileHeight)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, source.height}, (Rectangle){dest.x, dest.y + dy, dest.width, (float)tileHeight}, origin, rotation, tint);
        }

        // Fit last tile
        if (dy < dest.height)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                        (Rectangle){dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);
        }
    }
    else if (dest.height <= tileHeight)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)dest.height/tileHeight)*source.height}, (Rectangle){dest.x + dx, dest.y, (float)tileWidth, dest.height}, origin, rotation, tint);
        }

        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)dest.height/tileHeight)*source.height},
                        (Rectangle){dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tileWidth < dest.width; dx += tileWidth)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, source, (Rectangle){dest.x + dx, dest.y + dy, (float)tileWidth, (float)tileHeight}, origin, rotation, tint);
            }

            if (dy < dest.height)
            {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, (float)tileWidth, dest.height - dy}, origin, rotation, tint);
            }
        }

        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tileHeight < dest.height; dy += tileHeight)
            {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, source.height},
                        (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, (float)tileHeight}, origin, rotation, tint);
            }

            // Draw final tile in the bottom right corner
            if (dy < dest.height)
            {
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tileWidth)*source.width, ((float)(dest.height - dy)/tileHeight)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
            }
        }
    }
}
