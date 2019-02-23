/*******************************************************************************************
 *
 *   raylib [network] example - Client/Server ping-pong
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

int main()
{
	// Setup
	int screenWidth = 800;
	int screenHeight = 450;
	InitWindow(
		screenWidth, screenHeight, "raylib [network] example - ping pong");
	SetTargetFPS(60);

	SetTraceLogLevel(LOG_INFO);

	// Networking
	InitNetwork();

	// Create the server
	SocketConfig server_cfg = {
		.host = "127.0.0.1",
		.port = 8080,
		.server = true,
		.nonblocking = true,
	};

	SocketResult server_res;
	memset(&server_res, 0, sizeof(SocketResult));
	{
		bool ok = SocketOpen(&server_cfg, &server_res);
		if (!ok) { return false; }
	}

	// Create the client
	SocketConfig client_cfg = {
		.host = "127.0.0.1",
		.port = 8080,
	};

	SocketResult client_res;
	memset(&client_res, 0, sizeof(SocketResult));
	{
		bool ok = SocketOpen(&client_cfg, &client_res);
		if (!ok)
		{
			printf("failed to open: status %d, errno %d\n",
				client_res.status, client_res.saved_errno);
			return false;
		}
	}

	SocketResult connection;
	memset(&connection, 0, sizeof(SocketResult));
	float elapsed = 0.0f, delay = 1.0f; // ms
	bool  ping = false, pong = false;
	char  recvBuffer[512];
	bool  connected = false;
	memset(&recvBuffer, 0, 8);

	// Main game loop
	while (!WindowShouldClose())
	{
		// Draw
		BeginDrawing();

		// Clear
		ClearBackground(RAYWHITE);

		// A valid connection will != -1
		if (!connected)
		{
			if (SocketAccept(server_res.socket.handle, &connection))
			{
				ping = true;
				connected = true;
			}
		}

		// Connected
		if (connected)
		{
			int bytesRecv = SocketReceive(&connection.socket, recvBuffer, 5);
			if (bytesRecv > 0)
			{
				if (strcmp(recvBuffer, "Ping!") == 0)
				{
					pong = true;
					printf("Ping!\n");
				}
				if (strcmp(recvBuffer, "Pong!") == 0)
				{
					ping = true;
					printf("Pong!\n");
				}
			}

			elapsed += GetFrameTime();
			if (elapsed > delay)
			{
				if (ping)
				{
					ping = false;
					SocketSend(&client_res.socket, "Ping!", 5);
				}
				else if (pong)
				{
					pong = false;
					SocketSend(&client_res.socket, "Pong!", 5);
				}
				elapsed = 0.0f;
			}
		}

		// End draw
		EndDrawing();
	}

	// Cleanup
	CloseWindow();
	return 0;
}