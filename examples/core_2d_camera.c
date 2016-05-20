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
    
    camera.offset = (Vector2){ 0, 0 };
    camera.target = (Vector2){ 400, 200 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    Rectangle player = { 400, 200, 40, 40 };
    camera.target = (Vector2){ player.x + 20, player.y + 20 };
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) player.x -= 2;
        else if (IsKeyDown(KEY_LEFT)) player.x += 2;
        else if (IsKeyDown(KEY_UP)) player.y -= 2;
        else if (IsKeyDown(KEY_DOWN)) player.y += 2;
        
        // Camera target follows player
        camera.target = (Vector2){ player.x + 20, player.y + 20 };
        
        if (IsKeyDown(KEY_R)) camera.rotation--;
        else if (IsKeyDown(KEY_F)) camera.rotation++;
        
        // Camera controls
        if (IsKeyDown(KEY_R)) camera.rotation--;
        else if (IsKeyDown(KEY_F)) camera.rotation++;
        
        camera.zoom += ((float)GetMouseWheelMove()*0.05f);
        
        if (IsKeyPressed(KEY_Z)) 
        {
            camera.zoom = 1.0f;
            camera.rotation = 0.0f;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawingEx(camera);

            ClearBackground(RAYWHITE);

            DrawText("2D CAMERA TEST", 20, 20, 20, GRAY);
            
            DrawRectangle(0, 300, screenWidth, 50, GRAY);
            DrawRectangleRec(player, RED);
            
            DrawRectangle(camera.origin.x, 0, 1, screenHeight, GREEN);
            DrawRectangle(0, camera.origin.y, screenWidth, 1, GREEN);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------   
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}