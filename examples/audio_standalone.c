/*******************************************************************************************
*
*   raylib [audio] example - Using audio module as standalone module
*
*   NOTE: This example does not require any graphic device, it can run directly on console.
*
*   [audio] module requires some external libs:
*       OpenAL Soft - Audio device management lib (http://kcat.strangesoft.net/openal.html)
*       stb_vorbis - Ogg audio files loading (http://www.nothings.org/stb_vorbis/)
*       jar_xm - XM module file loading
*       jar_mod - MOD audio file loading
*
*   Compile audio module using:
*   gcc -c audio.c stb_vorbis.c -Wall -std=c99 -DAUDIO_STANDALONE
*
*   Compile example using:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) audio.o stb_vorbis.o -lopenal32 -std=c99
*
*   This example has been created using raylib 1.5 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <stdio.h>
#include <conio.h>      // Windows only, no stardard library

#include "audio.h"

#define KEY_ESCAPE  27

int main()
{
    unsigned char key;
    
    InitAudioDevice();
    
    Sound fxWav = LoadSound("resources/audio/weird.wav");         // Load WAV audio file
    Sound fxOgg = LoadSound("resources/audio/tanatana.ogg");      // Load OGG audio file
    
    PlayMusicStream(0, "resources/audio/guitar_noodling.ogg");

    printf("\nPress s or d to play sounds...\n");
    
    while (key != KEY_ESCAPE)
    {
        if (kbhit()) key = getch();

        if (key == 's')
        {
            PlaySound(fxWav);
            key = 0;
        }
        
        if (key == 'd')
        {
            PlaySound(fxOgg);
            key = 0;
        }
        
        UpdateMusicStream(0);
    }
    
    UnloadSound(fxWav);     // Unload sound data
    UnloadSound(fxOgg);     // Unload sound data
    
    CloseAudioDevice();
    
    printf("\n\nPress ENTER to close...");
    getchar();

    return 0;
}