/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        float dx = 600 - 100;
        float dy = 105 - 405;
        
        float d = sqrtf(dx*dx + dy*dy);
        float angle = asinf(dy/d);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            //DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
            
            DrawRectangle(100, 400, 1, 10, BLACK);
            DrawRectangle(96, 404, 10, 1, BLACK);

            DrawRectangle(600, 100, 1, 10, BLACK);
            DrawRectangle(596, 104, 10, 1, BLACK);
            
            DrawLine(100, 405, 600, 105, RED);
            
            // Draw lines using textures
            /*
            DrawTexturePro(GetDefaultTexture(), (Rectangle){ 0, 0, GetDefaultTexture().width, GetDefaultTexture().height }, 
                           (Rectangle){ 100, 405, (float)GetDefaultTexture().width*d, 1 },
                           (Vector2){ 0, (float)GetDefaultTexture().height/2 }, -RAD2DEG*angle, BLUE);
            */

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------   
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}