/*******************************************************************************************
*
*   raylib game - Dr. Turtle & Mr. Gamera
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
*   This game has been created using raylib 1.1 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_ENEMIES 10

typedef enum { TITLE, GAMEPLAY, ENDING } GameScreen;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    // Init window
    InitWindow(screenWidth, screenHeight, "Dr. Turtle & Mr. GAMERA");
    
    // Define current screen
    GameScreen currentScreen = TITLE;
    
    SetTargetFPS(60);       // Setup game frames per second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Game screens management
        switch (currentScreen)
        {
            case TITLE:
            {
                // Press enter to change to gameplay screen
                if (IsKeyPressed(KEY_ENTER))
                {
                    currentScreen = GAMEPLAY;
                }
                
            } break;
            case GAMEPLAY:
            {
                // Press enter to change to ending screen
                if (IsKeyPressed(KEY_ENTER))
                {
                    currentScreen = ENDING;
                }
            
            } break;
            case ENDING:
            {
                // Press enter to change to title screen
                if (IsKeyPressed(KEY_ENTER))
                {
                    currentScreen = TITLE;
                }
      
            } break;
            default: break;
        }
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            switch (currentScreen)
            {
                case TITLE:
                {
                    // Draw title screen
                    DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
                    DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
                
                } break;
                case GAMEPLAY:
                {
                    // Draw gameplay screen
                    DrawRectangle(0, 0, screenWidth, screenHeight, RED);
                    DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);
                
                } break;
                case ENDING:
                {
                    // Draw ending screen
                    DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
                    DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
                    
                } break;
                default: break;
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}