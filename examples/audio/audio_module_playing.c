/*******************************************************************************************
*
*   raylib [audio] example - Module playing (streaming)
*
*   NOTE: This example requires OpenAL Soft library installed
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_CIRCLES  64

typedef struct {
    Vector2 position;
    float radius;
    float alpha;
    float speed;
    Color color;
} CircleWave;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);      // NOTE: Try to enable MSAA 4X
    
    InitWindow(screenWidth, screenHeight, "raylib [audio] example - module playing (streaming)");

    InitAudioDevice();              // Initialize audio device
    
    Color colors[14] = { RL_ORANGE, RL_RED, RL_GOLD, RL_LIME, RL_BLUE, RL_VIOLET, RL_BROWN, RL_LIGHTGRAY, RL_PINK,
                         RL_YELLOW, RL_GREEN, RL_SKYBLUE, RL_PURPLE, RL_BEIGE };
    
    // Creates ome circles for visual effect
    CircleWave circles[MAX_CIRCLES];
    
    for (int i = MAX_CIRCLES - 1; i >= 0; i--)
    {
        circles[i].alpha = 0.0f;
        circles[i].radius = GetRandomValue(10, 40);
        circles[i].position.x = GetRandomValue(circles[i].radius, screenWidth - circles[i].radius);
        circles[i].position.y = GetRandomValue(circles[i].radius, screenHeight - circles[i].radius);
        circles[i].speed = (float)GetRandomValue(1, 100)/20000.0f;
        circles[i].color = colors[GetRandomValue(0, 13)];
    }

    Music xm = LoadMusicStream("resources/mini1111.xm");
    
    PlayMusicStream(xm);

    float timePlayed = 0.0f;
    bool pause = false;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateMusicStream(xm);        // Update music buffer with new stream data
        
        // Restart music playing (stop and play)
        if (IsKeyPressed(KEY_SPACE)) 
        {
            StopMusicStream(xm);
            PlayMusicStream(xm);
        }
        
        // Pause/Resume music playing 
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
            
            if (pause) PauseMusicStream(xm);
            else ResumeMusicStream(xm);
        }
        
        // Get timePlayed scaled to bar dimensions
        timePlayed = GetMusicTimePlayed(xm)/GetMusicTimeLength(xm)*(screenWidth - 40);
        
        // Color circles animation
        for (int i = MAX_CIRCLES - 1; (i >= 0) && !pause; i--)
        {
            circles[i].alpha += circles[i].speed;
            circles[i].radius += circles[i].speed*10.0f;
            
            if (circles[i].alpha > 1.0f) circles[i].speed *= -1;
            
            if (circles[i].alpha <= 0.0f)
            {
                circles[i].alpha = 0.0f;
                circles[i].radius = GetRandomValue(10, 40);
                circles[i].position.x = GetRandomValue(circles[i].radius, screenWidth - circles[i].radius);
                circles[i].position.y = GetRandomValue(circles[i].radius, screenHeight - circles[i].radius);
                circles[i].color = colors[GetRandomValue(0, 13)];
                circles[i].speed = (float)GetRandomValue(1, 100)/20000.0f;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RL_RAYWHITE);
            
            for (int i = MAX_CIRCLES - 1; i >= 0; i--)
            {
                DrawCircleV(circles[i].position, circles[i].radius, Fade(circles[i].color, circles[i].alpha));
            }
            
            // Draw time bar
            DrawRectangle(20, screenHeight - 20 - 12, screenWidth - 40, 12, RL_LIGHTGRAY);
            DrawRectangle(20, screenHeight - 20 - 12, (int)timePlayed, 12, RL_MAROON);
            DrawRectangleLines(20, screenHeight - 20 - 12, screenWidth - 40, 12, RL_GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadMusicStream(xm);          // Unload music stream buffers from RAM
    
    CloseAudioDevice();     // Close audio device (music streaming is automatically stopped)

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}