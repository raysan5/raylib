/*******************************************************************************************
*
*   raylib [core] example - 2d camera
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
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");
    
    Camera2D camera;
    
    camera.position = (Vector2){ 0, 0 };
    camera.origin = (Vector2){ 100, 100 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) camera.position.x--;
        else if (IsKeyDown(KEY_LEFT)) camera.position.x++;
        else if (IsKeyDown(KEY_UP)) camera.position.y++;
        else if (IsKeyDown(KEY_DOWN)) camera.position.y--;
        
        if (IsKeyDown(KEY_R)) camera.rotation--;
        else if (IsKeyDown(KEY_F)) camera.rotation++;
        
        if (IsKeyDown(KEY_W)) camera.zoom += 0.005f;
        if (IsKeyDown(KEY_S)) camera.zoom -= 0.005f;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawingEx(camera);

            ClearBackground(RAYWHITE);

            DrawText("2D CAMERA TEST", 20, 20, 20, GRAY);
            
            DrawRectangle(0, 300, screenWidth, 50, GRAY);
            DrawRectangle(400, 250, 40, 40, RED);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------   
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}