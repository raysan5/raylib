/*******************************************************************************************
*
*   raylib [audio] example - sound loading
*
*   Example complexity rating: [★☆☆☆] 1/4
*
*   Example originally created with raylib 5.5
*   Example contributed by Torphedo (@torphedo)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2026 Torphedo (@torphedo)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    Wave wave = LoadWave("resources/country.mp3");
    if (wave.data) {
        ExportWave(wave, "country.wav");
        UnloadWave(wave);
    }

    return 0;
}