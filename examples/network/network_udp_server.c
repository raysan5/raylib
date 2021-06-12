/*******************************************************************************************
*
*   raylib [network] example - UDP Server
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

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [network] example - udp server");

    InitNetworkDevice();    // Init network communications

    const char *pingmsg = "Ping!";
    const char *pongmsg = "Pong!";

    bool ping = false;
    bool pong = false;
    float elapsed = 0.0f;
    float delay = 1.0f;

    SocketConfig serverConfig = {
        .host = "127.0.0.1",
        .port = "4950",
        .server = true,
        .type = SOCKET_UDP,
        .nonblocking = true
    };

    SocketResult *serverResult = NULL;
    SocketSet *socketSet = NULL;
    char receiveBuffer[512] = { 0 };

    //  Create the server: getaddrinfo + socket + setsockopt + bind + listen
    serverResult = LoadSocketResult();

    if (!SocketCreate(&serverConfig, serverResult)) TraceLog(LOG_WARNING, "Failed to open server: status %d, errno %d", serverResult->status, serverResult->socket->status);
    else if (!SocketBind(&serverConfig, serverResult)) TraceLog(LOG_WARNING, "Failed to bind server: status %d, errno %d", serverResult->status, serverResult->socket->status);

    //  Create and add sockets to the socket set
    socketSet = LoadSocketSet(1);
    AddSocket(socketSet, serverResult->socket);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Once connected to the network, check the sockets for pending information
        // and when information is ready, send either a Ping or a Pong.

        // CheckSockets, if any of the sockets in the set are pending (received data, or requests)
        // then mark the socket as being ready. You can check this with IsSocketReady(client_res->socket)
        int active = CheckSockets(socketSet, 0);
        if (active != 0) TraceLog(LOG_INFO, "There are currently %d socket(s) with data to be processed.", active);

        // IsSocketReady, if the socket is ready, attempt to receive data from the socket
        //  int bytesRecv = 0;
        //  if (IsSocketReady(serverResult->socket)) {
        //      bytesRecv = SocketReceive(serverResult->socket, receiveBuffer, msglen);
        //  }
        int bytesRecv = SocketReceive(serverResult->socket, receiveBuffer, (int)strlen(pingmsg) + 1);

        // If we received data, is that data a "Ping!" or a "Pong!"?
        if (bytesRecv > 0)
        {
            if (strcmp(receiveBuffer, pingmsg) == 0) { pong = true; }
            if (strcmp(receiveBuffer, pongmsg) == 0) { ping = true; }
        }

        // After each delay has expired, send a response "Ping!" for a "Pong!" and vice-versa
        elapsed += GetFrameTime();

        if (elapsed > delay)
        {
            if (ping)
            {
                ping = false;
                SocketSend(serverResult->socket, pingmsg, (int)strlen(pingmsg) + 1);
            }
            else if (pong)
            {
                pong = false;
                SocketSend(serverResult->socket, pongmsg, (int)strlen(pongmsg) + 1);
            }

            elapsed = 0.0f;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            // TODO: Draw relevant connection info

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseNetworkDevice();   // Close network communication

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}