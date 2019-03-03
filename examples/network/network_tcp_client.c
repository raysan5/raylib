/*******************************************************************************************
*
*   raylib [core] example - Basic window
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
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define MAX_BUFFER_SIZE 512

char         buffer[MAX_BUFFER_SIZE];
float        elapsed   = 0.0f;
float        delay     = 1.0f;
bool         connected = false;
SocketConfig client_cfg = {.host = "127.0.0.1", .port = "8080", .nonblocking = true};
SocketResult* client_res = NULL;
SocketSet*    socket_set = NULL;

void NetworkConnect()
{
	if (!SocketConnect(&client_cfg, client_res)) {
		TraceLog(LOG_WARNING,
				 "Failed to connect socket to server: status %d, errno %d",
				 client_res->status, client_res->socket->status);
	} else {
		connected = true;
	}
}

void NetworkUpdate()
{
	int active = CheckSockets(socket_set, 0);
	if (active != 0) {
		TraceLog(LOG_DEBUG,
				 "There are currently %d socket(s) with data to be processed.", active);
	}

	if (active > 0) {
		int bytesRecv = 0;
		if (IsSocketReady(client_res->socket)) {
			bytesRecv = SocketReceive(client_res->socket, buffer, MAX_BUFFER_SIZE, 0);
		}
		if (bytesRecv > 0) { TraceLog(LOG_INFO, "%s", buffer); }
	}
}

int main()
{
	int screenWidth  = 800;
	int screenHeight = 450;
	InitWindow(
		screenWidth, screenHeight, "raylib [core] example - basic window");
	SetTargetFPS(60);

	// Set log type
	SetTraceLogLevel(LOG_DEBUG);

	// Init the network layer
	InitNetwork();

	// Create the server
	client_res = AllocSocketResult();
	socket_set = AllocSocketSet(1);
	if (!SocketCreate(&client_cfg, client_res)) {
		TraceLog(LOG_WARNING, "Failed to create socket: status %d, errno %d",
				 client_res->status, client_res->socket->status);
	} else {
		AddSocket(socket_set, client_res->socket); 
	}

	// Main game loop
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE); 
		if (connected) {
			NetworkUpdate();
		} else {
			elapsed += GetFrameTime();
			if (elapsed > delay) {
				NetworkConnect();
				elapsed = 0.0f;
			}
		}
		EndDrawing();
	}
	CloseWindow();

	return 0;
}