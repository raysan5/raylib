/*******************************************************************************************
*
*   raylib example 05a - SpriteFont loading and drawing some text with it 
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    int screenWidth = 800;
    int screenHeight = 450;
    
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example 05a - sprite fonts");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    SpriteFont font = LoadSpriteFont("resources/custom_font.png");        // SpriteFont loading
    //----------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
        // TODO: Update your variables here
        //-----------------------------------------------------
        
        // Draw
        //-----------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            // TODO: Comming soon...
            // TIP: Use DrawTextEx() function
/*
void DrawTextEx(SpriteFont spriteFont, const char* text, Vector2 position, int fontSize, int spacing, Color tint);        
*/        
        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    UnloadSpriteFont(font);       // SpriteFont unloading
    
    CloseWindow();                // Close window and OpenGL context
    //----------------------------------------------------------
    
    return 0;
}