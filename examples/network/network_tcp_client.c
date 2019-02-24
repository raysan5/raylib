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

#define PORT "3490"     // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

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

	int             numbytes;
	char            buf[MAXDATASIZE];   

	// Create the client
	SocketConfig client_cfg = {
		.host = "127.0.0.1", 
		.port = "8080"
	};
	SocketResult client_res;
	memset(&client_res, 0, sizeof(SocketResult));
	{
		bool ok = SocketOpen(&client_cfg, &client_res);
		if (!ok)
		{
			printf("Failed to open: status %d, errno %d\n", client_res.status,
				   client_res.socket.error);
			return false;
		}
	}

	// Main game loop
	while (!WindowShouldClose())
	{
		// Draw
		BeginDrawing();

		// Clear
		ClearBackground(RAYWHITE);

		// Receive bytes from the server
		numbytes = SocketReceive(client_res.socket.handle, buf, MAXDATASIZE - 1);
		if (numbytes == -1)
		{
			printf("Client: error recv '%s'\n", buf);
			break;
		}
		buf[numbytes] = '\0';
		printf("Client: received '%s'\n", buf);
		break;

		// End draw
		EndDrawing();
	}

	// Cleanup
	SocketClose(client_res.socket.handle);
	CloseNetwork();
	CloseWindow();
	return 0;
}