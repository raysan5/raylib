/*******************************************************************************************
*
*   raylib [others] example - embedded files loading
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Example originally created with raylib 3.0, last time updated with raylib 3.5
*
*   Example contributed by Kristian Holmgren (@defutura) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2020-2025 Kristian Holmgren (@defutura) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "resources/audio_data.h"   // Wave file exported with ExportWaveAsCode()
#include "resources/image_data.h"   // Image file exported with ExportImageAsCode()

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [others] example - embedded files loading");

    InitAudioDevice();              // Initialize audio device

    // Loaded in CPU memory (RAM) from header file (audio_data.h)
    // Same as: Wave wave = LoadWave("sound.wav");
    Wave wave = {
        .data = AUDIO_DATA,
        .frameCount = AUDIO_FRAME_COUNT,
        .sampleRate = AUDIO_SAMPLE_RATE,
        .sampleSize = AUDIO_SAMPLE_SIZE,
        .channels = AUDIO_CHANNELS
    };

    // Wave converted to Sound to be played
    Sound sound = LoadSoundFromWave(wave);

    // With a Wave loaded from file, after Sound is loaded, we can unload Wave
    // but in our case, Wave is embedded in executable, in program .data segment
    // we can not (and should not) try to free that private memory region
    //UnloadWave(wave);             // Do not unload wave data!

    // Loaded in CPU memory (RAM) from header file (image_data.h)
    // Same as: Image image = LoadImage("raylib_logo.png");
    Image image = {
        .data = IMAGE_DATA,
        .width = IMAGE_WIDTH,
        .height = IMAGE_HEIGHT,
        .format = IMAGE_FORMAT,
        .mipmaps = 1
    };

    // Image converted to Texture (VRAM) to be drawn
    Texture2D texture = LoadTextureFromImage(image);

    // With an Image loaded from file, after Texture is loaded, we can unload Image
    // but in our case, Image is embedded in executable, in program .data segment
    // we can not (and should not) try to free that private memory region
    //UnloadImage(image);           // Do not unload image data!

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_SPACE)) PlaySound(sound);      // Play sound
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexture(texture, screenWidth/2 - texture.width/2, 40, WHITE);

            DrawText("raylib logo and sound loaded from header files", 150, 320, 20, LIGHTGRAY);
            DrawText("Press SPACE to PLAY the sound!", 220, 370, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSound(sound);     // Unload sound from VRAM
    UnloadTexture(texture); // Unload texture from VRAM

    CloseAudioDevice();     // Close audio device

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}