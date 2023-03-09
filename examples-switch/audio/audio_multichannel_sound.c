/*******************************************************************************************
*
*   raylib [audio] example - Multichannel sound playing
*
*   Example originally created with raylib 3.0, last time updated with raylib 3.5
*
*   Example contributed by Chris Camacho (@chriscamacho) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2022 Chris Camacho (@chriscamacho) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <switch.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Initialize resource directory
    romfsInit();
    InitWindow(screenWidth, screenHeight, "raylib [audio] example - Multichannel sound playing");

    InitAudioDevice();      // Initialize audio device

    Sound fxWav = LoadSound("romfs:/resources/sound.wav");         // Load WAV audio file
    Sound fxOgg = LoadSound("romfs:/resources/target.ogg");        // Load OGG audio file

    SetSoundVolume(fxWav, 0.2f);

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) PlaySoundMulti(fxWav);     // Play a new wav sound instance
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) PlaySoundMulti(fxOgg);     // Play a new ogg sound instance
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("MULTICHANNEL SOUND PLAYING", 20, 20, 20, GRAY);
            DrawText("Press A button to play new ogg instance!", 200, 120, 20, LIGHTGRAY);
            DrawText("Press B button to play new wav instance!", 200, 180, 20, LIGHTGRAY);

            DrawText(TextFormat("CONCURRENT SOUNDS PLAYING: %02i", GetSoundsPlaying()), 220, 280, 20, RED);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    StopSoundMulti();       // We must stop the buffer pool before unloading

    UnloadSound(fxWav);     // Unload sound data
    UnloadSound(fxOgg);     // Unload sound data

    CloseAudioDevice();     // Close audio device

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    romfsExit();
    return 0;
}
