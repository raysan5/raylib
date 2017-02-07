/*******************************************************************************************
*
*   raylib [audio] example - Music playing (streaming) (adapted for HTML5 platform)
*
*   NOTE: This example requires OpenAL Soft library installed
*
*   This example has been created using raylib 1.3 (www.raylib.com)
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

    PlayMusicStream(0, "resources/audio/guitar_noodling.ogg");         // Play music stream
    
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
    framesCounter++;
    
    // Testing music fading from one file to another
/*
        if (framesCounter > 600)    // Wait for 10 seconds (600 frames)
        {
            volume -= 0.01;         // Decrement music volume level

            // When music volume level equal or lower than 0,
            // restore volume level and init another music file
            if (volume <= 0)
            {
                volume = 1.0;
                framesCounter = 0;
                PlayMusicStream(1, "resources/audio/another_file.ogg");
            }

            SetMusicVolume(volume);
        }
*/
    
    if (IsWindowMinimized()) PauseMusicStream(0);
    else ResumeMusicStream(0);

    timePlayed = GetMusicTimePlayed(0)/GetMusicTimeLength(0)*100*4; // We scale by 4 to fit 400 pixels
    
    UpdateMusicStream(0);        // Update music buffer with new stream data
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("MUSIC SHOULD BE PLAYING!", 255, 200, 20, LIGHTGRAY);

        DrawRectangle(200, 250, 400, 12, LIGHTGRAY);
        DrawRectangle(200, 250, (int)timePlayed, 12, MAROON); 
        
        
    EndDrawing();
    //----------------------------------------------------------------------------------
}