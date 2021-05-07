/*******************************************************************************************
*
*   raylib [audio] example - Using raudio module as standalone module
*
*   NOTE: This example does not require any graphic device, it can run directly on console.
*
*   DEPENDENCIES:
*       miniaudio.h  - Audio device management lib (https://github.com/dr-soft/miniaudio)
*       stb_vorbis.h - Ogg audio files loading (http://www.nothings.org/stb_vorbis/)
*       dr_wav.h     - WAV audio file loading (https://github.com/mackron/dr_libs)
*       dr_mp3.h     - MP3 audio file loading (https://github.com/mackron/dr_libs)
*       dr_flac.h    - FLAC audio file loading (https://github.com/mackron/dr_libs)
*       jar_xm.h     - XM module file loading
*       jar_mod.h    - MOD audio file loading
*
*   COMPILATION:
*       gcc -o raudio_standalone.exe raudio_standalone.c ..\..\src\raudio.c /
*           -I..\..\src -I..\..\src\external -L. -Wall -std=c99 -DRAUDIO_STANDALONE /
*           -DSUPPORT_FILEFORMAT_WAV -DSUPPORT_FILEFORMAT_OGG -DSUPPORT_FILEFORMAT_MP3
*
*   LICENSE: zlib/libpng
*
*   This example is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2014-2020 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
********************************************************************************************/

#include "raudio.h"             // raylib audio library

#include <stdio.h>              // Required for: printf()

#if defined(_WIN32)
    #include <conio.h>          // Windows only, no stardard library
#else
    // Required for kbhit() function in non-Windows platforms
    #include <stdio.h>
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

#define KEY_ESCAPE  27

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
#if !defined(_WIN32)
static int kbhit(void);             // Check if a key has been pressed
static char getch();                // Get pressed character
#else
#define kbhit _kbhit
#define getch _getch
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    static unsigned char key = 0;

    InitAudioDevice();

    Sound fxWav = LoadSound("resources/audio/weird.wav");       // Load WAV audio file
    Sound fxOgg = LoadSound("resources/audio/target.ogg");      // Load OGG audio file

    Music music = LoadMusicStream("resources/audio/country.mp3");
    PlayMusicStream(music);

    printf("\nPress s or d to play sounds, ESC to stop...\n");
    //--------------------------------------------------------------------------------------

    // Main loop
    while (key != KEY_ESCAPE)
    {
        if (kbhit()) key = getch();

        if ((key == 's') || (key == 'S')) PlaySound(fxWav);
        if ((key == 'd') || (key == 'D')) PlaySound(fxOgg);

        key = 0;

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

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
#if !defined(_WIN32)
// Check if a key has been pressed
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

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// Get pressed character
static char getch() { return getchar(); }
#endif
