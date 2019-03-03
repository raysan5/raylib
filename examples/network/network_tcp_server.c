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

bool          connected = false;
bool          sent      = false;
float         elapsed   = 0.0f;
float         delay     = 1.0f;
SocketConfig  server_cfg = {.host = "127.0.0.1", .port = "8080", .server = true, .nonblocking = true};
SocketConfig  connection_cfg = {.nonblocking = true};
SocketResult* server_res     = NULL;
Socket*       connection     = NULL;
SocketSet*    socket_set     = NULL;

void NetworkSend()
{
	if (!sent) {
		sent = true;
		SocketSend(connection, "Hello, world!", 13);
	}
}

void NetworkListen()
{
	int active = CheckSockets(socket_set, 0);
	if (active != 0) {
		TraceLog(LOG_DEBUG,
				 "There are currently %d socket(s) with data to be processed.", active);
	}

	if (active > 0) {
		connection = SocketAccept(server_res->socket, &connection_cfg);
		if (connection != NULL) {
			AddSocket(socket_set, connection);
			connected = true;
		} else {
			TraceLog(LOG_WARNING, "Failed to accept socket: status %d, errno %d",
					 server_res->status, server_res->socket->status);
		}
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
	server_res = AllocSocketResult();
	socket_set = AllocSocketSet(2);
	if (!SocketCreate(&server_cfg, server_res)) {
		TraceLog(LOG_WARNING, "Failed to create socket: status %d, errno %d",
				 server_res->status, server_res->socket->status);
	} else {
		AddSocket(socket_set, server_res->socket);
		if (!SocketListen(&server_cfg, server_res)) {
			TraceLog(LOG_WARNING,
					 "Failed to listen on socket: status %d, errno %d",
					 server_res->status, server_res->socket->status);
		}
	}

	// Main game loop
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE);
		if (connected) {
			NetworkSend();
		} else {
			elapsed += GetFrameTime();
			if (elapsed > delay) {
				NetworkListen();
				elapsed = 0.0f;
			}
		}
		EndDrawing();
	}
	CloseWindow();

	return 0;
}