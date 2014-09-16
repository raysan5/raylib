/*******************************************************************************************
*
*   raylib example 07c - Load and draw a 3d model (OBJ)
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

    Vector3 position = { 0.0, 0.0, 0.0 };
    
    // Define the camera to look into our 3d world
    Camera camera = {{ 10.0, 8.0, 10.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    InitWindow(screenWidth, screenHeight, "raylib example 07c - 3d models");
    
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    
    Texture2D texture = LoadTexture("resources/catsham.png");
    Model cat = LoadModel("resources/cat.obj");
    SetModelTexture(&cat, texture); // Link texture to model
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_LEFT)) position.x -= 0.2;
        if (IsKeyDown(KEY_RIGHT)) position.x += 0.2;
        if (IsKeyDown(KEY_UP)) position.z -= 0.2;
        if (IsKeyDown(KEY_DOWN)) position.z += 0.2;
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            Begin3dMode(camera);
            
                DrawModel(cat, position, 0.1f, WHITE);   // Draw 3d model with texture

                DrawGrid(10.0, 1.0);        // Draw a grid
                
                DrawGizmo(position); 
                
            End3dMode();
            
            DrawFPS(10, 10);
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture);     // Unload texture
    UnloadModel(cat);           // Unload model
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}