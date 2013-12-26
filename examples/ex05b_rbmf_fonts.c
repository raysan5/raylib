/*******************************************************************************************
*
*   raylib example 05b - raylib bitmap font (rbmf) loading and using
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
    int screenWidth = 560;
    int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "raylib example 05b - rBMF fonts");
    
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    SpriteFont font1 = LoadSpriteFont("resources/fonts/alagard.rbmf");       // SpriteFont loading
    SpriteFont font2 = LoadSpriteFont("resources/fonts/pixelplay.rbmf");     // SpriteFont loading
    SpriteFont font3 = LoadSpriteFont("resources/fonts/mecha.rbmf");         // SpriteFont loading
    SpriteFont font4 = LoadSpriteFont("resources/fonts/setback.rbmf");       // SpriteFont loading
    SpriteFont font5 = LoadSpriteFont("resources/fonts/romulus.rbmf");       // SpriteFont loading
    SpriteFont font6 = LoadSpriteFont("resources/fonts/pixantiqua.rbmf");    // SpriteFont loading
    SpriteFont font7 = LoadSpriteFont("resources/fonts/alpha_beta.rbmf");    // SpriteFont loading
    SpriteFont font8 = LoadSpriteFont("resources/fonts/jupiter_crash.rbmf"); // SpriteFont loading
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
            
            DrawTextEx(font1, "TESTING ALAGARD FONT", (Vector2){ 100, 100 }, GetFontBaseSize(font1)*2, 2, MAROON);
            DrawTextEx(font2, "TESTING PIXELPLAY FONT", (Vector2){ 100, 180 }, GetFontBaseSize(font2)*2, 4, ORANGE);  
            DrawTextEx(font3, "TESTING MECHA FONT", (Vector2){ 100, 260 }, GetFontBaseSize(font3)*2, 8, DARKGREEN);  
            DrawTextEx(font4, "TESTING SETBACK FONT", (Vector2){ 100, 350 }, GetFontBaseSize(font4)*2, 4, DARKBLUE);  
            DrawTextEx(font5, "TESTING ROMULUS FONT", (Vector2){ 100, 430 }, GetFontBaseSize(font5)*2, 3, DARKPURPLE);
            DrawTextEx(font6, "TESTING PIXANTIQUA FONT", (Vector2){ 100, 510 }, GetFontBaseSize(font6)*2, 4, LIME);  
            DrawTextEx(font7, "TESTING ALPHA_BETA FONT", (Vector2){ 100, 590 }, GetFontBaseSize(font7)*2, 4, GOLD);  
            DrawTextEx(font8, "TESTING JUPITER_CRASH FONT", (Vector2){ 100, 660 }, GetFontBaseSize(font8)*2, 1, RED);               
       
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSpriteFont(font1);       // SpriteFont unloading
    UnloadSpriteFont(font2);       // SpriteFont unloading
    UnloadSpriteFont(font3);       // SpriteFont unloading
    UnloadSpriteFont(font4);       // SpriteFont unloading
    UnloadSpriteFont(font5);       // SpriteFont unloading
    UnloadSpriteFont(font6);       // SpriteFont unloading
    UnloadSpriteFont(font7);       // SpriteFont unloading
    UnloadSpriteFont(font8);       // SpriteFont unloading
    
    CloseWindow();                 // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}