/*******************************************************************************************
*
*   raylib [audio] example - Music playing (streaming)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"


#define AUDIO_THREAD_MUSIC_UPDATE true

// a simple lowpass filter applied to the music stream
static void audioEffectDemo(float* buffer, unsigned int nframes)
{
    static float low[2] = { 0.0f, 0.0f };
    static const float cutoff = 70.0f / 44100.0f; // 70 Hz lowpass filter
    const float k = cutoff / (cutoff + 0.1591549431f); // RC filter formula

    for (unsigned int i = 0; i < nframes*2; i+=2)
    {
        float l = buffer[i], r = buffer[i + 1];
        low[0] += k * (l - low[0]);
        low[1] += k * (r - low[1]);
        buffer[i] = low[0];
        buffer[i + 1] = low[1];
    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - music playing (streaming)");

    InitAudioDevice();              // Initialize audio device

    Music music = LoadMusicStream("resources/country.mp3");
    music.background = AUDIO_THREAD_MUSIC_UPDATE;

    PlayMusicStream(music);

    float timePlayed = 0.0f;
    bool pause = false;
    bool hasfx = false;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (!AUDIO_THREAD_MUSIC_UPDATE)
        {
            UpdateMusicStream(music);   // Update music buffer with new stream data
        }

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

        // Add/Remove effect
        if (IsKeyPressed(KEY_F))
        {
            hasfx = !hasfx;
            if (hasfx)
            {
                AddAudioStreamProcessor(music.stream, &audioEffectDemo);
            }
            else
            {
                RemoveAudioStreamProcessor(music.stream, &audioEffectDemo);
            }
        }

        // Get timePlayed scaled to bar dimensions (400 pixels)
        timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music)*400;

        if (timePlayed > 400) StopMusicStream(music);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("MUSIC SHOULD BE PLAYING!", 255, 150, 20, LIGHTGRAY);

            DrawRectangle(200, 200, 400, 12, LIGHTGRAY);
            DrawRectangle(200, 200, (int)timePlayed, 12, MAROON);
            DrawRectangleLines(200, 200, 400, 12, GRAY);
            DrawRectangleLines(200, 200, 400, 12, GRAY);

            DrawText("PRESS SPACE TO RESTART MUSIC", 215, 250, 20, LIGHTGRAY);
            DrawText("PRESS P TO PAUSE/RESUME MUSIC", 208, 280, 20, LIGHTGRAY);
            DrawText("PRESS F TO ADD/REMOVE EFFECT", 208, 310, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadMusicStream(music);   // Unload music stream buffers from RAM

    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}