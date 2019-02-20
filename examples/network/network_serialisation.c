/*******************************************************************************************
 *
 *   raylib [network] example - Resolve host
 *
 *   Welcome to raylib!
 *
 *   To test examples, just press F6 and execute raylib_compile_execute script
 *   Note that compiled executable is placed in the same folder as .c file
 *
 *   You can find all basic examples on C:\raylib\raylib\examples folder or
 *   raylib official webpage: www.raylib.com
 *
 *   Enjoy using raylib. :)
 *
 *   This example has been created using raylib 2.0 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h
 *for details)
 *
 *   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/


#include "raylib.h" 
#include <string.h> 

int main()
{
	// Setup
	int screenWidth  = 800;
	int screenHeight = 450;
	InitWindow(
		screenWidth, screenHeight, "raylib [network] example - ping pong");
	SetTargetFPS(60);

	SetTraceLogLevel(LOG_DEBUG);

	// Networking
	InitNetwork();

	unsigned char buf[1024];
	unsigned char magic;
	int           monkeycount;
	long          altitude;
	double        absurdityfactor;
	char*         s = "Great unmitigated Zot!  You've found the Runestaff!";
	char          s2[96];
	unsigned int  packetsize, ps2;

	packetsize = PackData(buf, "CHhlsd", 'B', 0, 37, -5, s, -3490.5);
	packi16(buf + 1, packetsize); // store packet size in packet for kicks

	printf("packet is %u bytes\n", packetsize);
	UnpackData(buf, "CHhl96sd", &magic, &ps2, &monkeycount, &altitude, s2, &absurdityfactor);
	printf("'%c' %hhu %u %ld \"%s\" %f\n", magic, ps2, monkeycount, altitude, s2, absurdityfactor);


	// Main game loop
	while (!WindowShouldClose())
	{
		// Draw
		BeginDrawing();

		// Clear
		ClearBackground(RAYWHITE);

		// End draw
		EndDrawing();
	}

	// Cleanup
	CloseWindow();
	return 0;
}