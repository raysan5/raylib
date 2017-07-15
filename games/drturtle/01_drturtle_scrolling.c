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
    
    // Load game resources: textures
    Texture2D sky = LoadTexture("resources/sky.png");
    Texture2D mountains = LoadTexture("resources/mountains.png");
    Texture2D sea = LoadTexture("resources/sea.png");

    // Define scrolling variables
    int backScrolling = 0;
    int seaScrolling = 0;
    
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
                // Sea scrolling
                seaScrolling -= 2;
                if (seaScrolling <= -screenWidth) seaScrolling = 0; 
            
                // Press enter to change to gameplay screen
                if (IsKeyPressed(KEY_ENTER))
                {
                    currentScreen = GAMEPLAY;
                }
                
            } break;
            case GAMEPLAY:
            {
                // Background scrolling logic
                backScrolling--;
                if (backScrolling <= -screenWidth) backScrolling = 0; 
                
                // Sea scrolling logic
                seaScrolling -= 8;
                if (seaScrolling <= -screenWidth) seaScrolling = 0;
                
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
            
            // Draw background (common to all screens)
            DrawTexture(sky, 0, 0, WHITE);
            
            DrawTexture(mountains, backScrolling, 0, WHITE);
            DrawTexture(mountains, screenWidth + backScrolling, 0, WHITE);
            
            DrawTexture(sea, seaScrolling, 0, BLUE);
            DrawTexture(sea, screenWidth + seaScrolling, 0, BLUE);
            
            switch (currentScreen)
            {
                case TITLE:
                {
                    // Draw title screen
                    DrawText("PRESS ENTER", 450, 420, 40, BLACK);
                
                } break;
                case GAMEPLAY:
                {
                    // Draw gameplay screen
                    DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);
                
                } break;
                case ENDING:
                {
                    // Draw ending screen
                    
                    // Draw a transparent black rectangle that covers all screen
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f));
                    
                    DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
                    
                } break;
                default: break;
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    
    // Unload textures
    UnloadTexture(sky);
    UnloadTexture(mountains);
    UnloadTexture(sea);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}