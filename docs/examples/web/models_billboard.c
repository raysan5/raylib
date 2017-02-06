/*******************************************************************************************
*
*   raylib [models] example - Drawing billboards  (adapted for HTML5 platform)
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

// Define the camera to look into our 3d world
Camera camera = {{ 5.0f, 4.0f, 5.0f }, { 0.0f, 2.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }};

Texture2D bill;                                 // Our texture billboard
Vector3 billPosition = { 0.0f, 2.0f, 0.0f };    // Position where draw billboard
    
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
    InitWindow(screenWidth, screenHeight, "raylib [models] example - drawing billboards");

    bill = LoadTexture("resources/billboard.png");     // Our texture billboard
    
    SetCameraMode(CAMERA_ORBITAL);      // Set an orbital camera mode
    SetCameraPosition(camera.position); // Set internal camera position to match our camera position
    SetCameraTarget(camera.target);     // Set internal camera target to match our camera target

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
    UnloadTexture(bill);        // Unload texture

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

            DrawBillboard(camera, bill, billPosition, 2.0f, WHITE);

            DrawGrid(10, 1.0f);        // Draw a grid

        End3dMode();

        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}