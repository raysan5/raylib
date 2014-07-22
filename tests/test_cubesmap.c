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

#include "../raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    Vector3 position = { 0.5, 0.0, 0.5 };
    
    // Define the camera to look into our 3d world
    Camera camera = {{ 7.0, 6.0, 7.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }};
    
    InitWindow(screenWidth, screenHeight, "raylib test - Heightmap loading and drawing");
    
    Image img = LoadImage("resources/cubesmap.png");
    Model map = LoadCubesmap(img);
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
        if (IsKeyDown(KEY_UP)) camera.position.y += 0.2f;
        else if (IsKeyDown(KEY_DOWN)) camera.position.y -= 0.2f;
        
        if (IsKeyDown(KEY_RIGHT)) camera.position.z += 0.2f;
        else if (IsKeyDown(KEY_LEFT)) camera.position.z -= 0.2f;
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            Begin3dMode(camera);
            
                //DrawCube(position, 1.0f, 1.0f, 1.0f, RED);
            
                DrawModel(map, position, 1.0f, MAROON);
                
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
    UnloadModel(map);       // Unload model
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}