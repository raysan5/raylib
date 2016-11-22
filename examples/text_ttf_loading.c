/*******************************************************************************************
*
*   raylib [text] example - TTF loading and usage
*
*   This example has been created using raylib 1.3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - ttf loading");
    
    const char msg[50] = "TTF SpriteFont";

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)
    
    // TTF SpriteFont loading with custom generation parameters
    SpriteFont font = LoadSpriteFontTTF("resources/fonts/KAISG.ttf", 96, 0, 0);
    
    // Generate mipmap levels to use trilinear filtering
    // NOTE: On 2D drawing it won't be noticeable, it looks like FILTER_BILINEAR
    GenTextureMipmaps(&font.texture);

    float fontSize = font.size;
    Vector2 fontPosition = { 40, screenHeight/2 + 50 };
    Vector2 textSize;

    SetTextureFilter(font.texture, FILTER_POINT);
    int currentFontFilter = 0;      // FILTER_POINT
    
    int count = 0;
    char **droppedFiles;
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
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
        
        // Load a dropped TTF file dynamically (at current fontSize)
        if (IsFileDropped())
        {
            droppedFiles = GetDroppedFiles(&count);
            
            if (count == 1) // Only support one ttf file dropped
            {
                UnloadSpriteFont(font);
                font = LoadSpriteFontTTF(droppedFiles[0], fontSize, 0, 0);
                ClearDroppedFiles();
            }
        }
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

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(font);     // SpriteFont unloading
    
    ClearDroppedFiles();        // Clear internal buffers

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}