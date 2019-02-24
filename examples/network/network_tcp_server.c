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

	
	// Create the server
	SocketConfig server_cfg = {
		.host = "127.0.0.1", 
		.port = "8080", 
		.server = true, 
		.nonblocking = true
	};

	SocketResult server_res;
	memset(&server_res, 0, sizeof(SocketResult));
	{
		bool ok = SocketOpen(&server_cfg, &server_res);
		if (!ok)
		{
			return false;
		}
	}   

	
	SocketResult connection;
	memset(&connection, 0, sizeof(SocketResult));  

	char  recvBuffer[512];
	memset(&recvBuffer, 0, 8); 
	
	bool  connected = false;


	// Main game loop
	while (!WindowShouldClose())
	{
		// Draw
		BeginDrawing();

		// Clear
		ClearBackground(RAYWHITE);

		// Wait for a valid connection
		if (!connected)
		{
			if (SocketAccept(server_res.socket.handle, &connection))
			{ 
				connected = true;
			}
		}

		if (connected)
		{
			SocketSend(&connection, "Hello, world!", 13);
		} 

		// End draw
		EndDrawing();
	}

	// Cleanup
	CloseWindow();
	return 0;
}