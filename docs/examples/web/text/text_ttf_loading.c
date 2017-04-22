/*******************************************************************************************
*
*   raylib [text] example - TTF loading and usage
*
*   This example has been created using raylib 1.3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;

const char msg[50] = "TTF SpriteFont";

SpriteFont font;
float fontSize;
Vector2 fontPosition = { 40, screenHeight/2 + 50 };
Vector2 textSize;

int currentFontFilter = 0;      // FILTER_POINT

#if !defined(PLATFORM_WEB)
int count = 0;
char **droppedFiles;
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [text] example - ttf loading");

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
    
    // TTF SpriteFont loading with custom generation parameters
    font = LoadSpriteFontEx("resources/KAISG.ttf", 96, 0, 0);
    
    // Generate mipmap levels to use trilinear filtering
    // NOTE: On 2D drawing it won't be noticeable, it looks like FILTER_BILINEAR
    GenTextureMipmaps(&font.texture);

    fontSize = font.baseSize;

    SetTextureFilter(font.texture, FILTER_POINT);
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(font);     // SpriteFont unloading
    
#if !defined(PLATFORM_WEB) 
    ClearDroppedFiles();        // Clear internal buffers
#endif

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    fontSize += GetMouseWheelMove()*4.0f;
    
    // Choose font texture filter method
    if (IsKeyPressed(KEY_ONE))
    {
        SetTextureFilter(font.texture, FILTER_POINT);
        currentFontFilter = 0;
    }
    else if (IsKeyPressed(KEY_TWO))
    {
        SetTextureFilter(font.texture, FILTER_BILINEAR);
        currentFontFilter = 1;
    }
    else if (IsKeyPressed(KEY_THREE))
    {
        // NOTE: Trilinear filter won't be noticed on 2D drawing
        SetTextureFilter(font.texture, FILTER_TRILINEAR);
        currentFontFilter = 2;
    }
    
    textSize = MeasureTextEx(font, msg, fontSize, 0);
    
    if (IsKeyDown(KEY_LEFT)) fontPosition.x -= 10;
    else if (IsKeyDown(KEY_RIGHT)) fontPosition.x += 10;
    
#if !defined(PLATFORM_WEB)
    // Load a dropped TTF file dynamically (at current fontSize)
    if (IsFileDropped())
    {
        droppedFiles = GetDroppedFiles(&count);
        
        if (count == 1) // Only support one ttf file dropped
        {
            UnloadSpriteFont(font);
            font = LoadSpriteFontEx(droppedFiles[0], fontSize, 0, 0);
            ClearDroppedFiles();
        }
    }
#endif
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    
        ClearBackground(RAYWHITE);
        
        DrawText("Use mouse wheel to change font size", 20, 20, 10, GRAY);
        DrawText("Use KEY_RIGHT and KEY_LEFT to move text", 20, 40, 10, GRAY);
        DrawText("Use 1, 2, 3 to change texture filter", 20, 60, 10, GRAY);
        DrawText("Drop a new TTF font for dynamic loading", 20, 80, 10, DARKGRAY);

        DrawTextEx(font, msg, fontPosition, fontSize, 0, BLACK);
        
        // TODO: It seems texSize measurement is not accurate due to chars offsets...
        //DrawRectangleLines(fontPosition.x, fontPosition.y, textSize.x, textSize.y, RED);
        
        DrawRectangle(0, screenHeight - 80, screenWidth, 80, LIGHTGRAY);
        DrawText(FormatText("Font size: %02.02f", fontSize), 20, screenHeight - 50, 10, DARKGRAY);
        DrawText(FormatText("Text size: [%02.02f, %02.02f]", textSize.x, textSize.y), 20, screenHeight - 30, 10, DARKGRAY);
        DrawText("CURRENT TEXTURE FILTER:", 250, 400, 20, GRAY);
        
        if (currentFontFilter == 0) DrawText("POINT", 570, 400, 20, BLACK);
        else if (currentFontFilter == 1) DrawText("BILINEAR", 570, 400, 20, BLACK);
        else if (currentFontFilter == 2) DrawText("TRILINEAR", 570, 400, 20, BLACK);
  
    EndDrawing();
    //----------------------------------------------------------------------------------
}