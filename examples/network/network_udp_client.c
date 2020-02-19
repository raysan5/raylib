/*******************************************************************************************
 *
 *   raylib [network] example - UDP Client
*
*   This example has been created using raylib 3.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2019-2020 Jak Barnes (@syphonx) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#define RNET_IMPLEMENTATION
#include "rnet.h"


#include <assert.h>
#include <stdio.h>
#include <string.h>

float         elapsed = 0.0f;
float         delay   = 1.0f;
bool          ping    = false;
bool          pong    = false;
const char *  pingmsg = "Ping!";
const char *  pongmsg = "Pong!";
int           msglen  = 0;
SocketConfig  client_cfg = {.host = "127.0.0.1", .port = "4950",  .type = SOCKET_UDP, .nonblocking = true};
SocketResult *client_res = NULL;
SocketSet *   socket_set = NULL;
char          recvBuffer[512];

// Once connected to the network, check the sockets for pending information
// and when information is ready, send either a Ping or a Pong.
void UpdateNetwork()
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
    int bytesRecv = 0;
    if (IsSocketReady(client_res->socket)) {
        bytesRecv = SocketReceive(client_res->socket, recvBuffer, msglen);
    }

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
            SocketSend(client_res->socket, pingmsg, msglen);
        } else if (pong) {
            pong = false;
            SocketSend(client_res->socket, pongmsg, msglen);
        }
        elapsed = 0.0f;
    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [network] example - udp client");

    InitNetworkDevice();

    // Create the client
    //
    //  Performs
    //      getaddrinfo
    //      socket
    //      setsockopt
    //      connect (TCP only)
    client_res = AllocSocketResult();
    if (!SocketCreate(&client_cfg, client_res)) {
        TraceLog(LOG_WARNING, "Failed to open client: status %d, errno %d",
                 client_res->status, client_res->socket->status);
    }

    //  Create & Add sockets to the socket set
    socket_set = AllocSocketSet(1);
    msglen     = strlen(pingmsg) + 1;
    ping       = true;
    memset(recvBuffer, '\0', sizeof(recvBuffer));
    AddSocket(socket_set, client_res->socket);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateNetwork();
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseNetworkDevice(); // Close network
    
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}