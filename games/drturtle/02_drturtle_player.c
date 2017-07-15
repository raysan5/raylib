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
    Texture2D title = LoadTexture("resources/title.png");
    Texture2D turtle = LoadTexture("resources/turtle.png");
    Texture2D gamera = LoadTexture("resources/gamera.png");
    
    // Define scrolling variables
    int backScrolling = 0;
    int seaScrolling = 0;
    
    // Define current screen
    GameScreen currentScreen = TITLE;
    
    // Define player variables
    int playerRail = 1;
    Rectangle playerBounds = { 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
    bool gameraMode = false;
    
    // Define additional game variables
    int framesCounter = 0;
    
    SetTargetFPS(60);       // Setup game frames per second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        framesCounter++;

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
                    framesCounter = 0;
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
            
                // Player movement logic
                if (IsKeyPressed(KEY_DOWN)) playerRail++;
                else if (IsKeyPressed(KEY_UP)) playerRail--;
                
                // Check player not out of rails
                if (playerRail > 4) playerRail = 4;
                else if (playerRail < 0) playerRail = 0;
            
                // Update player bounds
                playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
                
                if (IsKeyPressed(KEY_SPACE)) gameraMode = !gameraMode;
                if (IsKeyPressed(KEY_ENTER)) currentScreen = ENDING;
                
            } break;
            case ENDING:
            {
                // Press enter to play again
                if (IsKeyPressed(KEY_ENTER))
                {
                    currentScreen = GAMEPLAY;
                    
                    // Reset player
                    playerRail = 1;
                    playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
                    
                    gameraMode = false;
                    framesCounter = 0;
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
            
            if (!gameraMode)
            {
                DrawTexture(sea, seaScrolling, 0, BLUE);
                DrawTexture(sea, screenWidth + seaScrolling, 0, BLUE);
            }
            else
            {
                DrawTexture(sea, seaScrolling, 0, RED);
                DrawTexture(sea, screenWidth + seaScrolling, 0, RED);
            }
            
            switch (currentScreen)
            {
                case TITLE:
                {
                    // Draw title
                    //DrawTexture(title, screenWidth/2 - title.width/2, screenHeight/2 - title.height/2 - 80, WHITE);
                    DrawRectangle(380, 140, 500, 300, GRAY);
                    
                    // Draw blinking text
                    if ((framesCounter/30) % 2) DrawText("PRESS ENTER", 480, 480, 40, BLACK);
                
                } break;
                case GAMEPLAY:
                {                    
                    // Draw player
                    //if (!gameraMode) DrawTexture(turtle, playerBounds.x - 14, playerBounds.y - 14, WHITE);
                    //else DrawTexture(gamera, playerBounds.x - 64, playerBounds.y - 64, WHITE);
                    
                    // Draw player bounding box
                    if (!gameraMode) DrawRectangleRec(playerBounds, GREEN);
                    else DrawRectangleRec(playerBounds, ORANGE);
            
                } break;
                case ENDING:
                {
                    // Draw a transparent black rectangle that covers all screen
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f));
                
                    DrawText("GAME OVER", 300, 200, 100, MAROON);
                    
                    // Draw blinking text
                    if ((framesCounter/30) % 2) DrawText("PRESS ENTER to REPLAY", 400, 420, 30, LIGHTGRAY);
                    
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
    UnloadTexture(title);
    UnloadTexture(turtle);
    UnloadTexture(gamera);
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}