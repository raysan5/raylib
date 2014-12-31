/*******************************************************************************************
*
*   raylib game - Floppy Bird
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
*   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_TUBES 100

int main()
{    
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Floppy Bird");
    
    InitAudioDevice();      // Initialize audio device
    
    Sound coin = LoadSound("resources/coin.wav");
    Sound jump = LoadSound("resources/jump.wav");
    
    Texture2D background = LoadTexture("resources/background.png");
    Texture2D tubes = LoadTexture("resources/tubes.png");
    Texture2D floppy = LoadTexture("resources/floppy.png");
    
    Vector2 floppyPos = { 80, screenHeight/2 - floppy.height/2 };
    
    Vector2 tubesPos[MAX_TUBES];
    int tubesSpeedX = 2;
    
    for (int i = 0; i < MAX_TUBES; i++)
    {
        tubesPos[i].x = 400 + 280*i;
        tubesPos[i].y = -GetRandomValue(0, 120);
    }
    
    Rectangle tubesRecs[MAX_TUBES*2];
    bool tubesActive[MAX_TUBES];
    
    for (int i = 0; i < MAX_TUBES*2; i += 2)
    {
        tubesRecs[i].x = tubesPos[i/2].x;
        tubesRecs[i].y = tubesPos[i/2].y;
        tubesRecs[i].width = tubes.width;
        tubesRecs[i].height = 255;
        
        tubesRecs[i+1].x = tubesPos[i/2].x;
        tubesRecs[i+1].y = 600 + tubesPos[i/2].y - 255;
        tubesRecs[i+1].width = tubes.width;
        tubesRecs[i+1].height = 255;
        
        tubesActive[i/2] = true;
    }
 
    int backScroll = 0;
    
    int score = 0;
    int hiscore = 0;
    
    bool gameover = false;
    bool superfx = false;
    
    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        backScroll--;
        
        if (backScroll <= -800) backScroll = 0; 
        
        for (int i = 0; i < MAX_TUBES; i++) tubesPos[i].x -= tubesSpeedX;
        
        for (int i = 0; i < MAX_TUBES*2; i += 2)
        {
            tubesRecs[i].x = tubesPos[i/2].x;
            tubesRecs[i+1].x = tubesPos[i/2].x;
        }

        if (IsKeyDown(KEY_SPACE) && !gameover) floppyPos.y -= 3;
        else floppyPos.y += 1;
        
        if (IsKeyPressed(KEY_SPACE) && !gameover) PlaySound(jump);
        
        // Check Collisions
        for (int i = 0; i < MAX_TUBES*2; i++)
        {
            if (CheckCollisionCircleRec((Vector2){ floppyPos.x + floppy.width/2, floppyPos.y + floppy.height/2 }, floppy.width/2, tubesRecs[i])) 
            {
                gameover = true;
            }
            else if ((tubesPos[i/2].x < floppyPos.x) && tubesActive[i/2] && !gameover)
            {
                score += 100;
                tubesActive[i/2] = false;
                PlaySound(coin);
                
                superfx = true;
                
                if (score > hiscore) hiscore = score;
            }
        }
        
        if (gameover && IsKeyPressed(KEY_ENTER))
        {
            for (int i = 0; i < MAX_TUBES; i++)
            {
                tubesPos[i].x = 400 + 280*i;
                tubesPos[i].y = -GetRandomValue(0, 120);
            }
    
            for (int i = 0; i < MAX_TUBES*2; i += 2)
            {
                tubesRecs[i].x = tubesPos[i/2].x;
                tubesRecs[i].y = tubesPos[i/2].y;
                
                tubesRecs[i+1].x = tubesPos[i/2].x;
                tubesRecs[i+1].y = 600 + tubesPos[i/2].y - 255;
                
                tubesActive[i/2] = true;
            }
        
            floppyPos.x = 80;
            floppyPos.y = screenHeight/2 - floppy.height/2;
            
            gameover = false;
            score = 0;
        }
        
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawTexture(background, backScroll, 0, WHITE);
            DrawTexture(background, screenWidth + backScroll, 0, WHITE);
            
            if (!gameover)
            {
                DrawTextureEx(floppy, floppyPos, 0, 1.0, WHITE);
                //DrawCircleLines(floppyPos.x + floppy.width/2, floppyPos.y + floppy.height/2, floppy.width/2, RED);
            }
            
            for (int i = 0; i < MAX_TUBES; i++)
            {
                if (tubesPos[i].x <= 800) DrawTextureEx(tubes, tubesPos[i], 0, 1.0, WHITE);
            
                //DrawRectangleLines(tubesRecs[i*2].x, tubesRecs[i*2].y, tubesRecs[i*2].width, tubesRecs[i*2].height, RED);
                //DrawRectangleLines(tubesRecs[i*2 + 1].x, tubesRecs[i*2 + 1].y, tubesRecs[i*2 + 1].width, tubesRecs[i*2 + 1].height, RED);
            }
            
            DrawText(FormatText("%04i", score), 20, 20, 40, PINK);
            DrawText(FormatText("HI-SCORE: %04i", hiscore), 20, 70, 20, VIOLET); 
            
            if (gameover)
            {
                DrawText("GAME OVER", 100, 180, 100, MAROON);
                DrawText("PRESS ENTER to RETRY!", 280, 280, 20, RED);    
            }
            
            if (superfx)
            {
                DrawRectangle(0, 0, screenWidth, screenHeight, GOLD);
                superfx = false;
            }
        
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(background);  // Texture unloading
    UnloadTexture(tubes);       // Texture unloading
    UnloadTexture(floppy);      // Texture unloading
    
    UnloadSound(coin);          // Unload sound data
    UnloadSound(jump);          // Unload sound data
    
    CloseAudioDevice();         // Close audio device
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    
    return 0;
}