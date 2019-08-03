/*******************************************************************************************
 *
 *   raylib [network] example - UDP Server
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
#include "rnet.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

float         elapsed    = 0.0f;
float         delay      = 1.0f;
bool          ping       = false;
bool          pong       = false;
const char *  pingmsg    = "Ping!";
const char *  pongmsg    = "Pong!";
int           msglen     = 0;
SocketConfig  server_cfg = {.host = "127.0.0.1", .port = "4950", .server = true, .type = SOCKET_UDP, .nonblocking = true};
SocketResult *server_res = NULL;
SocketSet *   socket_set = NULL;
char          recvBuffer[512];

// Once connected to the network, check the sockets for pending information
// and when information is ready, send either a Ping or a Pong.
void NetworkUpdate()
{
    // CheckSockets
    //
    // If any of the sockets in the socket_set are pending (received data, or requests)
    // then mark the socket as being ready. You can check this with IsSocketReady(client_res->socket)
    int active = CheckSockets(socket_set, 0);
    if (active != 0) {
        TraceLog(LOG_DEBUG,
                 "There are currently %d socket(s) with data to be processed.", active);
    }

    // IsSocketReady
    //
    // If the socket is ready, attempt to receive data from the socket
    //  int bytesRecv = 0;
    //  if (IsSocketReady(server_res->socket)) {
    //      bytesRecv = SocketReceive(server_res->socket, recvBuffer, msglen);
    //  }
    int bytesRecv = SocketReceive(server_res->socket, recvBuffer, msglen);

    // If we received data, was that data a "Ping!" or a "Pong!"
    if (bytesRecv > 0) {
        if (strcmp(recvBuffer, pingmsg) == 0) { pong = true; }
        if (strcmp(recvBuffer, pongmsg) == 0) { ping = true; }
    }

    // After each delay has expired, send a response "Ping!" for a "Pong!" and vice versa
    elapsed += GetFrameTime();
    if (elapsed > delay) {
        if (ping) {
            ping = false;
            SocketSend(server_res->socket, pingmsg, msglen);
        } else if (pong) {
            pong = false;
            SocketSend(server_res->socket, pongmsg, msglen);
        }
        elapsed = 0.0f;
    }
}

int main()
{
    // Setup
    int screenWidth  = 800;
    int screenHeight = 450;
    InitWindow(
        screenWidth, screenHeight, "raylib [network] example - udp server");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_DEBUG);

    // Networking
    InitNetwork();

    //  Create the server
    //
    //  Performs
    //      getaddrinfo
    //      socket
    //      setsockopt
    //      bind
    //      listen
    server_res = AllocSocketResult();
    if (!SocketCreate(&server_cfg, server_res)) {
        TraceLog(LOG_WARNING, "Failed to open server: status %d, errno %d",
                 server_res->status, server_res->socket->status);
    } else {
        if (!SocketBind(&server_cfg, server_res)) {
            TraceLog(LOG_WARNING, "Failed to bind server: status %d, errno %d",
                     server_res->status, server_res->socket->status);
        }
    }

    //  Create & Add sockets to the socket set
    socket_set = AllocSocketSet(1);
    msglen     = strlen(pingmsg) + 1;
    memset(recvBuffer, '\0', sizeof(recvBuffer));
    AddSocket(socket_set, server_res->socket);

    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        NetworkUpdate();
        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    return 0;
}