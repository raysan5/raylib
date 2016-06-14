/*******************************************************************************************
*
*   raylib [core] example - Oculus Rift CV1
*
*   Compile example using:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) -L. -L..\src\external\OculusSDK\LibOVR -lLibOVRRT32_1 -lraylib -lglfw3 -lopengl32 -lgdi32 -std=c99
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 1080;
    int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "raylib [core] example - oculus rift");
    
    InitOculusDevice();
    
    // Define the camera to look into our 3d world
    Camera camera;
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    //SetTargetFPS(90);                   // Set our game to run at 90 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateOculusTracking();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            BeginOculusDrawing();
            
                for (int eye = 0; eye < 2; eye++)
                {
                    // TODO: Probably projection and view matrices could be created here...
                    // ...without the need to create it internally through Begin3dMode()
                    Begin3dMode(camera);
                
                        SetOculusMatrix(eye);
                        
                        DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                        DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
                        
                        DrawGrid(10, 1.0f);
                    
                    // TODO: Call internal buffers drawing directly (rlglDraw()) and...
                    // ...reset internal matrices, instead of letting End3dMode() do that
                    End3dMode();
                }
            
            EndOculusDrawing();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseOculusDevice();    // Close Oculus Rift device
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
