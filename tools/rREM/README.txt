/**********************************************************************************************
*
*   rREM - raylib Resource Embedder 1.0.0 (alpha)
*
*   Tool to embed resources (images, text, sounds, models...) into a rRES file.
*
*   Copyright 2014 Ramon Santamaria. All rights reserved.
*
***********************************************************************************************/

rrem creates a .rres resource with embedded files and a .h header to access embedded data

Usage example: 

1) Create 'resources.rres' and 'resources.h' including 3 files:
	
    rrem image01.png image02.jpg sound03.wav

2) In your raylib program, just add at top:

	#include "resources.h"

3) When a resource is required, just load it using:

	Texture2D mytex = LoadTextureFromRES("resources.rres", RES_image01);
	Sound mysound = LoadSoundFromRES("resources.rres", RES_sound03);

Note that you can check resources id names in resources.h file

Have fun! :)