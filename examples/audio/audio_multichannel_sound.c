
/*
* This example was coded to demonstrate multi channel audio changes added to raylib
*
*   This example has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   This example Copyright (c) 2018 Chris Camacho (codifies) http://bedroomcoders.co.uk/captcha/
*
* THIS EXAMPLE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* This example may be freely redistributed.
*/

#include "raylib.h"
#include <stdio.h> // sprintf

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [audio] abuse!");

    InitAudioDevice();      // Initialize audio device

    Sound fxWav = LoadSound("resources/sound.wav");         // Load WAV audio file
    Sound fxOgg = LoadSound("resources/tanatana.ogg");      // Load OGG audio file


    //InitPlayBufferPool();



    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    int frame = 0;
    // Main game loop

    // old system still works alongside
    SetSoundVolume(fxWav, 0.2); // effects all mutltiplay replay (really annoying sound!!!)
    PlaySound(fxOgg);

    bool inhibitWav = false;
    bool inhibitOgg = false;
    int maxFrame = 60;
    int numberPlaying = 0;

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        frame++;

        if (IsKeyDown(KEY_ENTER)) { inhibitWav=true; } else { inhibitWav=false; }
        if (IsKeyDown(KEY_SPACE)) { inhibitOgg=true; } else { inhibitOgg=false; }

        // deliberatly hammer the play pool to see what dropping old
        // pool entries sounds like....
        if (frame % 5==0) {
           if (!inhibitWav) PlaySoundEx(fxWav);
        }
        if (frame==maxFrame) {
            if (!inhibitOgg) PlaySoundEx(fxOgg);
            frame=0;
            maxFrame = GetRandomValue(6,12);
        }

        numberPlaying = ConcurrentPlayChannels();

        //----------------------------------------------------------------------------------


        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            char msg[1024];
            DrawText("multichannel sound abuse!", 200, 180, 20, LIGHTGRAY);
            DrawText("Space to inhibit new ogg triggering", 200, 200, 20, LIGHTGRAY);
            DrawText("Enter to inhibit new wav triggering", 200, 220, 20, LIGHTGRAY);

            sprintf(msg,"concurrently playing %i", numberPlaying);
            DrawText(msg, 200, 280, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------


    // you MUST stop the buffer pool before unloading
    // any Sounds it might have used...
    StopPlayBufferPool();

    UnloadSound(fxWav);     // Unload sound data
    UnloadSound(fxOgg);     // Unload sound data

    CloseAudioDevice();     // Close audio device

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
