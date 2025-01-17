/*******************************************************************************************
*
*   raylib [audio] example - Playing sound multiple times
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 4.6, last time updated with raylib 4.6
*
*   Example contributed by Jeffery Myers (@JeffM2501) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023-2025 Jeffery Myers (@JeffM2501)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_SOUNDS 10
Sound soundArray[MAX_SOUNDS] = { 0 };
int currentSound;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - playing sound multiple times");

    InitAudioDevice();      // Initialize audio device

    // load the sound list
    soundArray[0] = LoadSound("resources/sound.wav");         // Load WAV audio file into the first slot as the 'source' sound
                                                              // this sound owns the sample data
    for (int i = 1; i < MAX_SOUNDS; i++)
    {
        soundArray[i] = LoadSoundAlias(soundArray[0]);        // Load an alias of the sound into slots 1-9. These do not own the sound data, but can be played
    }
    currentSound = 0;                                         // set the sound list to the start

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_SPACE))
        {
            PlaySound(soundArray[currentSound]);            // play the next open sound slot
            currentSound++;                                 // increment the sound slot
            if (currentSound >= MAX_SOUNDS)                 // if the sound slot is out of bounds, go back to 0
                currentSound = 0;

            // Note: a better way would be to look at the list for the first sound that is not playing and use that slot
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Press SPACE to PLAY a WAV sound!", 200, 180, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (int i = 1; i < MAX_SOUNDS; i++)
        UnloadSoundAlias(soundArray[i]);     // Unload sound aliases
    UnloadSound(soundArray[0]);              // Unload source sound data

    CloseAudioDevice();     // Close audio device

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}