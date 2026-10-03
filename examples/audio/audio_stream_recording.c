/*******************************************************************************************
*
*   raylib [audio] example - stream recording
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 6.0, last time updated with raylib 6.0
*
*   Example created by Jairo Correa (@jn-jairo) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Jairo Correa (@jn-jairo)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h> // Required for: NULL
#include <string.h> // Required for: memcpy()

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static unsigned int maxFrameCount = 0;
static Wave wave = { 0 };

//------------------------------------------------------------------------------------
// Module Functions Declaration
//------------------------------------------------------------------------------------
static void RecordingCallback(const void *framesIn, unsigned int frameCount);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] example - stream recording");

    InitAudioDevice();
    InitAudioRecordingDevice();
    // InitAudioRecordingDeviceEx(24000, 16, 1);

    // Configure it so that RecordingCallback is called whenever new samples are read from the microphone
    SetAudioRecordingCallback(RecordingCallback);

    // Configure the wave to match the recording
    wave.sampleRate = GetAudioRecordingSampleRate();
    wave.sampleSize = GetAudioRecordingSampleSize();
    wave.channels = GetAudioRecordingChannels();

    // Allocate buffer for the audio recording
    maxFrameCount = wave.sampleRate*5; // 5 seconds
    wave.data = RL_CALLOC(maxFrameCount*wave.channels, wave.sampleSize/8);

    Sound sound = { 0 };

    float timeRecorded = 0.0f;

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------

        // Space key pressed or the recording buffer is full
        if (IsKeyPressed(KEY_SPACE) || (IsAudioRecording() && (wave.frameCount == maxFrameCount)))
        {
            if (IsAudioRecording())
            {
                StopAudioRecording();

                // Reload and play the sound
                UnloadSound(sound);
                sound = LoadSoundFromWave(wave);
                PlaySound(sound);
            }
            else
            {
                // Stop the sound and reset the recording frameCount
                StopSound(sound);
                wave.frameCount = 0;

                StartAudioRecording();
            }
        }

        // Get timeRecorded scaled to bar dimensions
        timeRecorded = ((float)wave.frameCount/(float)maxFrameCount)*(screenWidth - 40);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // Draw time bar
            DrawRectangle(20, screenHeight - 20 - 12, screenWidth - 40, 12, LIGHTGRAY);
            DrawRectangle(20, screenHeight - 20 - 12, (int)timeRecorded, 12, MAROON);
            DrawRectangleLines(20, screenHeight - 20 - 12, screenWidth - 40, 12, GRAY);

            // Draw help instructions
            DrawText("Press SPACE to START/STOP recording the sound!", 130, 180, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseAudioRecordingDevice(); // Close audio recording device (recording is automatically stopped)
    CloseAudioDevice();          // Close audio device (music streaming is automatically stopped)

    UnloadSound(sound);
    UnloadWave(wave);

    CloseWindow();               // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definition
//------------------------------------------------------------------------------------
static void RecordingCallback(const void *framesIn, unsigned int frameCount)
{
    // Check how many frames left in the buffer
    unsigned int framesLeft = maxFrameCount - wave.frameCount;

    // Do not overflow the buffer
    if (framesLeft == 0) return;
    unsigned int framesToCopy = (frameCount < framesLeft)? frameCount : framesLeft;

    // Append the new data
    memcpy((unsigned char *)wave.data + wave.frameCount*wave.channels*wave.sampleSize/8, framesIn, framesToCopy*wave.channels*wave.sampleSize/8);
    wave.frameCount += framesToCopy;
}
