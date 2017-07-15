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
*       gcc -c audio.c stb_vorbis.c -Wall -std=c99 -DAUDIO_STANDALONE -DAL_LIBTYPE_STATIC
*
*   Compile example using:
*       gcc -o audio_standalone.exe audio_standalone.c audio.o stb_vorbis.o -lopenal32 -lwinmm /
*           -Wall -std=c99 -Wl,-allow-multiple-definition
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <stdio.h>
#if defined(_WIN32)
#include <conio.h>          // Windows only, no stardard library
#endif

#include "audio.h"

#if defined(__linux__)

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

static int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

static char getch()
{
	return getchar();
}

#endif

#define KEY_ESCAPE  27

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    unsigned char key;
    
    InitAudioDevice();
    
    Sound fxWav = LoadSound("resources/audio/weird.wav");         // Load WAV audio file
    Sound fxOgg = LoadSound("resources/audio/tanatana.ogg");      // Load OGG audio file
    
    Music music = LoadMusicStream("resources/audio/guitar_noodling.ogg");
    PlayMusicStream(music);

    printf("\nPress s or d to play sounds...\n");
    //--------------------------------------------------------------------------------------

    // Main loop
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
        
        UpdateMusicStream(music);
    }
    
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSound(fxWav);         // Unload sound data
    UnloadSound(fxOgg);         // Unload sound data
    
    UnloadMusicStream(music);   // Unload music stream data
    
    CloseAudioDevice();
    //--------------------------------------------------------------------------------------

    return 0;
}
