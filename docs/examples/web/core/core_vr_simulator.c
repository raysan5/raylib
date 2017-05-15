/*******************************************************************************************
*
*   raylib [core] example - VR Simulator (Oculus Rift CV1 parameters)
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
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

// NOTE: screenWidth/screenHeight should match VR device aspect ratio

Camera camera;
 
Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
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
    InitWindow(screenWidth, screenHeight, "raylib [core] example - oculus rift");
    
    // NOTE: If device is not available, it fallbacks to default device (simulator)
    InitVrSimulator(HMD_OCULUS_RIFT_CV1);               // Init VR simulator (Oculus Rift CV1 parameters)
    
    // Define the camera to look into our 3d world
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    
    SetCameraMode(camera, CAMERA_FIRST_PERSON);         // Set first person camera mode
    
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
    CloseVrSimulator();     // Close VR simulator
    
    CloseWindow();          // Close window and OpenGL context
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
    UpdateCamera(&camera);          // Update camera (simulator mode)

    if (IsKeyPressed(KEY_SPACE)) ToggleVrMode();    // Toggle VR mode
    
    // Lock mouse cursor if mouse click on canvas
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) DisableCursor();
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        BeginVrDrawing();

            Begin3dMode(camera);

                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

                DrawGrid(40, 1.0f);

            End3dMode();
        
        EndVrDrawing();

        DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}