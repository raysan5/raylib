/*******************************************************************************************
*
*   raylib test - Testing DrawBillboard() and DrawBillboardRec()
*
*   This test has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    Vector3 position = { 0.0, 0.0, 0.0 };
    
    // Define the camera to look into our 3d world
    Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    InitWindow(screenWidth, screenHeight, "raylib test - Billboards");
    
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    
    Texture2D texture = LoadTexture("resources/raylib_logo.png");
    Texture2D lena = LoadTexture("resources/lena.png");
    
    Rectangle eyesRec = { 225, 240, 155, 50 };
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT)) camera.position.x -= 0.2;
        if (IsKeyDown(KEY_RIGHT)) camera.position.x += 0.2;
        if (IsKeyDown(KEY_UP)) camera.position.y -= 0.2;
        if (IsKeyDown(KEY_DOWN)) camera.position.y += 0.2;
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            Begin3dMode(camera);
            
                //DrawBillboard(camera, texture, position, 2.0, WHITE);
                DrawBillboardRec(camera, lena, eyesRec, position, 4.0, WHITE);
            
                DrawGrid(10.0, 1.0);        // Draw a grid
                
            End3dMode();
            
            DrawFPS(10, 10);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadTexture(lena);        // Unload texture
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}