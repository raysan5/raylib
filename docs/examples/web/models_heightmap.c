/*******************************************************************************************
*
*   raylib [models] example - Heightmap loading and drawing  (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
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
int screenWidth = 800;
int screenHeight = 450;

// Define our custom camera to look into our 3d world
Camera camera = {{ 18.0f, 16.0f, 18.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f };

Texture2D texture;
Model map;

Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };	// Set model position (depends on model scaling!)

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
    InitWindow(screenWidth, screenHeight, "raylib [models] example - heightmap loading and drawing");

    Image image = LoadImage("resources/heightmap.png");     // Load heightmap image (RAM)
    texture = LoadTextureFromImage(image);                  // Convert image to texture (VRAM)
    map = LoadHeightmap(image, (Vector3){ 16, 8, 16 });     // Load heightmap model with defined size
    map.material.texDiffuse = texture;                      // Set map diffuse texture

    UnloadImage(image);                 // Unload heightmap image from RAM, already uploaded to VRAM
    
    SetCameraMode(CAMERA_ORBITAL);      // Set an orbital camera mode
    SetCameraPosition(camera.position); // Set internal camera position to match our custom camera position

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
    UnloadTexture(texture);     // Unload texture
    UnloadModel(map);           // Unload model

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
    UpdateCamera(&camera);          // Update internal camera and our camera
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        Begin3dMode(camera);

            // NOTE: Model is scaled to 1/4 of its original size (128x128 units)
            DrawModel(map, mapPosition, 1.0f, RED);

        	DrawGrid(20, 1.0f);

            End3dMode();
            
            DrawTexture(texture, screenWidth - texture.width - 20, 20, WHITE);
            DrawRectangleLines(screenWidth - texture.width - 20, 20, texture.width, texture.height, GREEN);

        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}