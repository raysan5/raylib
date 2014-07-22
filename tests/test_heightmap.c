/*******************************************************************************************
*
*   raylib test - Testing Heightmap Loading and Drawing
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
    Camera camera = {{ 12.0, 10.0, 12.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    InitWindow(screenWidth, screenHeight, "raylib test - Heightmap loading and drawing");
    
    Image img = LoadImage("resources/heightmap.png");
    Model map = LoadHeightmap(img, 4);
    Texture2D texture = CreateTexture(img, false);
    UnloadImage(img);
    
    SetModelTexture(&map, texture);
    
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            Begin3dMode(camera);
            
                DrawModel(map, position, 0.5f, MAROON);
                
                DrawGrid(10.0, 1.0);        // Draw a grid
                
                DrawGizmo(position); 
                
            End3dMode();
            
            DrawFPS(10, 10);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Unload texture
    UnloadModel(map);       // Unload model
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}