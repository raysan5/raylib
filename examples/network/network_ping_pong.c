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
#include "rnet.c"

#include <assert.h>
#include <stdio.h>
#include <string.h>

char         recvBuffer[512];
float        elapsed   = 0.0f;
float        delay     = 1.0f;
bool         ping      = false;
bool         pong      = false;
bool         connected = false;
const char*  pingmsg   = "Ping!";
const char*  pongmsg   = "Pong!";
int          msglen    = 0;
SocketConfig server_cfg = {.host = "127.0.0.1", .port = "8080", .server = true, .nonblocking = true};
SocketConfig client_cfg = {.host = "127.0.0.1", .port = "8080", .nonblocking = true};
SocketConfig          connection_cfg = {.nonblocking = true};
SocketResult*         server_res     = NULL;
SocketResult*         client_res     = NULL;
SocketSet*            socket_set     = NULL;
Socket*               connection     = NULL;  

/*
** packshort() -- store a 16-bit int into a char buffer (like htons())
*/
void packshort(unsigned short s)
{
	unsigned char* sptr = ((unsigned char*) &(s));
	printf("before: %02X %02X\n", sptr[0], sptr[1]);
	unsigned short ns = htons(s);
	memcpy(packbuf + packlen, (unsigned char*) &(ns), sizeof(short));
	printf("after: %02X %02X\n", (packbuf + packlen)[0], (packbuf + packlen)[1]);
	packlen += sizeof(s);
}

void NetworkConnect()
{
	if (server_cfg.datagram) {
		ping      = true;
		connected = true;
	} else {
		if ((connection = SocketAccept(server_res->socket, &connection_cfg)) != NULL) {
			AddSocket(socket_set, connection);
			ping      = true;
			connected = true;
		}
	}
}

void NetworkUpdate()
{
	int active = CheckSockets(socket_set, 0);
	if (active != 0) {
		TraceLog(LOG_DEBUG,
				 "There are currently %d socket(s) with data to be processed.", active);
	}

	int bytesRecv = 0;
	if (server_cfg.datagram) {
		if (IsSocketReady(server_res->socket)) {
			bytesRecv = SocketReceive(server_res->socket, recvBuffer, msglen, 0);
		}
	} else {
		if (IsSocketReady(connection)) {
			bytesRecv = SocketReceive(connection, recvBuffer, msglen, 0);
		}
	}
	if (bytesRecv > 0) {
		if (strcmp(recvBuffer, pingmsg) == 0) { pong = true; }
		if (strcmp(recvBuffer, pongmsg) == 0) { ping = true; }
	}

	elapsed += GetFrameTime();
	if (elapsed > delay) {
		if (ping) {
			ping = false;
			SocketSend(client_res->socket, pingmsg, msglen);
		} else if (pong) {
			pong = false;
			SocketSend(client_res->socket, pongmsg, msglen);
		}
		elapsed = 0.0f;
	}
}
  
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "=" VALUE(var) 

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
	server_res = AllocSocketResult();
	if (!SocketOpen(&server_cfg, server_res)) {
		TraceLog(LOG_WARNING, "Failed to open server: status %d, errno %d\n",
				 server_res->status, server_res->socket->status);
	}

	// Create the client
	client_res = AllocSocketResult();
	if (!SocketOpen(&client_cfg, client_res)) {
		TraceLog(LOG_WARNING, "Failed to open client: status %d, errno %d\n",
				 client_res->status, client_res->socket->status);
	}

	socket_set = CreateSocketSet(3);
	msglen     = strlen(pingmsg) + 1;
	memset(recvBuffer, '\0', sizeof(recvBuffer));
	AddSocket(socket_set, server_res->socket);
	AddSocket(socket_set, client_res->socket);

	// Main game loop
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE);
		if (connected) {
			NetworkUpdate();
		} else {
			NetworkConnect();
		}
		EndDrawing();
	}

	// Cleanup
	CloseWindow();
	return 0;
}