/*******************************************************************************************
*
*   raylib example 04b - Texture loading and drawing a part defined by a rectangle 
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
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    const char textLine1[] = "Lena image is a standard test image which has been in use since 1973.";
    const char textLine2[] = "It comprises 512x512 pixels, and was originally cropped from the centerfold";
    const char textLine3[] = "of November 1972 issue of Playboy magazine. The image is probably the most";
    const char textLine4[] = "widely used test image for all sorts of image processing algorithms.";

    InitWindow(screenWidth, screenHeight, "raylib example 04b - texture rectangle");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Texture2D texture = LoadTexture("resources/lena.png");        // Texture loading
    
    Color halfTrans = WHITE;
    halfTrans.a = 30;
    
    Rectangle eyesRec = { 225, 240, 155, 50 };
    Vector2 position = { 369, 241 };
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawText("LENA", 220, 100, 20, PINK);
            
            DrawTexture(texture, screenWidth/2 - 256, 0, halfTrans); // Draw background image
            
            DrawTextureRec(texture, eyesRec, position, WHITE);  // Draw eyes part of image

            DrawText(textLine1, 220, 140, 10, DARKGRAY);
            DrawText(textLine2, 220, 160, 10, DARKGRAY);  
            DrawText(textLine3, 220, 180, 10, DARKGRAY);  
            DrawText(textLine4, 220, 200, 10, DARKGRAY);  
       
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);       // Texture unloading
    
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}
