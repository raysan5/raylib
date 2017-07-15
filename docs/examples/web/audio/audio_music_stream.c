/*******************************************************************************************
*
*   raylib [audio] example - Music playing (streaming) (adapted for HTML5 platform)
*
*   NOTE: This example requires OpenAL Soft library installed
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

int framesCounter = 0;
float timePlayed = 0.0f;
static bool pause = false;

Music music;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [audio] example - music playing (streaming)");
    
    InitAudioDevice();              // Initialize audio device

    music = LoadMusicStream("resources/guitar_noodling.ogg");
    
    PlayMusicStream(music);
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadMusicStream(music);   // Unload music stream buffers from RAM

    CloseAudioDevice();     // Close audio device (music streaming is automatically stopped)
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateMusicStream(music);        // Update music buffer with new stream data
    
    // Restart music playing (stop and play)
    if (IsKeyPressed(KEY_SPACE)) 
    {
        StopMusicStream(music);
        PlayMusicStream(music);
    }
    
    // Pause/Resume music playing 
    if (IsKeyPressed(KEY_P))
    {
        pause = !pause;
        
        if (pause) PauseMusicStream(music);
        else ResumeMusicStream(music);
    }
    
    // Get timePlayed scaled to bar dimensions (400 pixels)
    timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music)*400;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("MUSIC SHOULD BE PLAYING!", 255, 150, 20, LIGHTGRAY);

        DrawRectangle(200, 200, 400, 12, LIGHTGRAY);
        DrawRectangle(200, 200, (int)timePlayed, 12, MAROON);
        DrawRectangleLines(200, 200, 400, 12, GRAY);
        
        DrawText("PRESS SPACE TO RESTART MUSIC", 215, 250, 20, LIGHTGRAY);
        DrawText("PRESS P TO PAUSE/RESUME MUSIC", 208, 280, 20, LIGHTGRAY);
        
    EndDrawing();
    //----------------------------------------------------------------------------------
}