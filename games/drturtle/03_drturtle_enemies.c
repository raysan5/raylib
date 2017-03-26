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
    Texture2D shark = LoadTexture("resources/shark.png");
    Texture2D orca = LoadTexture("resources/orca.png");
    Texture2D swhale = LoadTexture("resources/swhale.png");
    Texture2D fish = LoadTexture("resources/fish.png");

    // Define scrolling variables
    int backScrolling = 0;
    int seaScrolling = 0;
    
    // Define current screen
    GameScreen currentScreen = TITLE;
    
    // Define player variables
    int playerRail = 1;
    Rectangle playerBounds = { 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
    bool gameraMode = false;
    
    // Define enemies variables
    Rectangle enemyBounds[MAX_ENEMIES];
    int enemyRail[MAX_ENEMIES];
    int enemyType[MAX_ENEMIES];
    bool enemyActive[MAX_ENEMIES];
    float enemySpeed = 10;
    
    // Init enemies variables
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        // Define enemy type (all same probability)
        enemyType[i] = GetRandomValue(0, 3);

        // Define enemy rail
        enemyRail[i] = GetRandomValue(0, 4);
        
        // Define enemy bounding box
        enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
        enemyActive[i] = false;
    }
    
    // Define additional game variables
    int foodBar = 0;
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
                seaScrolling -= (enemySpeed - 2);
                if (seaScrolling <= -screenWidth) seaScrolling = 0; 
            
                // Player movement logic
                if (IsKeyPressed(KEY_DOWN)) playerRail++;
                else if (IsKeyPressed(KEY_UP)) playerRail--;
                
                // Check player not out of rails
                if (playerRail > 4) playerRail = 4;
                else if (playerRail < 0) playerRail = 0;
            
                // Update player bounds
                playerBounds = (Rectangle){ 30 + 14, playerRail*120 + 90 + 14, 100, 100 };
                
                // Enemies activation logic (every 40 frames)        
                if (framesCounter > 40)
                {
                    for (int i = 0; i < MAX_ENEMIES; i++)
                    {
                        if (enemyActive[i] == false)
                        {
                            enemyActive[i] = true;
                            i = MAX_ENEMIES;
                        }
                    }
                    
                    framesCounter = 0;
                }
                
                // Enemies logic
                for (int i = 0; i < MAX_ENEMIES; i++)
                {
                    if (enemyActive[i])
                    {
                        enemyBounds[i].x -= enemySpeed;
                    }
                    
                    // Check enemies out of screen
                    if (enemyBounds[i].x <= 0 - 128)
                    {
                        enemyType[i] = GetRandomValue(0, 3);
                        enemyRail[i] = GetRandomValue(0, 4);    
                        
                        enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
                        enemyActive[i] = false;
                    }
                }
                
                // Enemies speed increase every frame
                if (!gameraMode) enemySpeed += 0.005;
                
                // Check collision player vs enemies
                for (int i = 0; i < MAX_ENEMIES; i++)
                {
                    if (enemyActive[i])
                    {
                        if (CheckCollisionRecs(playerBounds, enemyBounds[i]))
                        {                       
                            if (enemyType[i] < 3)   // Bad enemies
                            {
                                if (gameraMode)
                                {
                                    foodBar += 15;
                                    
                                    // After enemy deactivation, reset enemy parameters to be reused
                                    enemyType[i] = GetRandomValue(0, 3);
                                    enemyRail[i] = GetRandomValue(0, 4);
                                    
                                    enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
                                    enemyActive[i] = false;
                                }
                                else
                                {
                                    // Player die logic
                                    currentScreen = ENDING;
                                    framesCounter = 0;
                                }
                            }
                            else    // Sweet fish
                            {
                                enemyActive[i] = false;
                                enemyType[i] = GetRandomValue(0, 3);
                                enemyRail[i] = GetRandomValue(0, 4);
                                
                                enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
                                
                                if (!gameraMode) foodBar += 80;
                                else foodBar += 25;
                                
                                if (foodBar == 400)
                                {
                                    gameraMode = true;
                                }
                            }
                        }
                    }
                }
                
                // Gamera mode logic
                if (gameraMode)
                {
                    foodBar--;
                    
                    if (foodBar <= 0) 
                    {
                        gameraMode = false;
                        enemySpeed -= 2;
                        if (enemySpeed < 10) enemySpeed = 10;
                    }
                }
            
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
                    
                    // Reset enemies data
                    for (int i = 0; i < MAX_ENEMIES; i++)
                    {                       
                        enemyType[i] = GetRandomValue(0, 3);
                        enemyRail[i] = GetRandomValue(0, 4);
                        
                        enemyBounds[i] = (Rectangle){ screenWidth + 14, 120*enemyRail[i] + 90 + 14, 100, 100 };
                        enemyActive[i] = false;
                    }
                    
                    enemySpeed = 10;
                    
                    // Reset game variables
                    foodBar = 0;
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
                    DrawTexture(title, screenWidth/2 - title.width/2, screenHeight/2 - title.height/2 - 80, WHITE);
                    
                    // Draw blinking text
                    if ((framesCounter/30) % 2) DrawText("PRESS ENTER", 480, 480, 40, BLACK);
                
                } break;
                case GAMEPLAY:
                {
                    // Draw water lines
                    for (int i = 0; i < 5; i++) DrawRectangle(0, i*120 + 120, screenWidth, 110, Fade(SKYBLUE, 0.1f));
                    
                    // Draw player
                    if (!gameraMode) DrawTexture(turtle, playerBounds.x - 14, playerBounds.y - 14, WHITE);
                    else DrawTexture(gamera, playerBounds.x - 64, playerBounds.y - 64, WHITE);
                    
                    // Draw player bounding box
                    //if (!gameraMode) DrawRectangleRec(playerBounds, Fade(GREEN, 0.4f));
                    //else DrawRectangleRec(playerBounds, Fade(ORANGE, 0.4f));
                    
                    // Draw enemies
                    for (int i = 0; i < MAX_ENEMIES; i++)
                    {
                        if (enemyActive[i]) 
                        {
                            // Draw enemies
                            /*
                            switch(enemyType[i])
                            {
                                case 0: DrawTexture(shark, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
                                case 1: DrawTexture(orca, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
                                case 2: DrawTexture(swhale, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
                                case 3: DrawTexture(fish, enemyBounds[i].x - 14, enemyBounds[i].y - 14, WHITE); break;
                                default: break;
                            }
                            */
                            
                            // Draw enemies bounding boxes
                            switch(enemyType[i])
                            {
                                case 0: DrawRectangleRec(enemyBounds[i], RED); break;
                                case 1: DrawRectangleRec(enemyBounds[i], RED); break;
                                case 2: DrawRectangleRec(enemyBounds[i], RED); break;
                                case 3: DrawRectangleRec(enemyBounds[i], GREEN); break;
                                default: break;
                            }
                        }
                    }
                    
                    // Draw gameplay interface
                    
                    // Draw food bar
                    DrawRectangle(20, 20, 400, 40, Fade(GRAY, 0.4f));
                    DrawRectangle(20, 20, foodBar, 40, ORANGE);
                    DrawRectangleLines(20, 20, 400, 40, BLACK);
                    
                    if (gameraMode)
                    {
                        DrawText("GAMERA MODE", 60, 22, 40, GRAY);
                    }
            
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
    UnloadTexture(shark);
    UnloadTexture(orca);
    UnloadTexture(swhale);
    UnloadTexture(fish);
    
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}