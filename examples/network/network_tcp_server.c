/*******************************************************************************************
*
*   raylib [network] example - TCP Server
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

    InitWindow(screenWidth, screenHeight, "raylib [network] example - tcp server");

    InitNetworkDevice();    // Init network communications
    
    const char *pingmsg = "Ping!";
    const char *pongmsg = "Pong!";
    
    bool ping = false;
    bool pong = false;
    float elapsed = 0.0f;
    float delay = 1.0f;
    bool connected = false;

    SocketConfig serverConfig = {
        .host = "127.0.0.1", 
        .port = "4950", 
        .type = SOCKET_TCP,
        .server = true,
        .nonblocking = true
    };
    
    SocketConfig connectionConfig = { .nonblocking = true };
    
    Socket *connection = NULL;
    SocketSet *socketSet = NULL;
    SocketResult *serverResult = NULL;
    char receiveBuffer[512] = { 0 };
    
    //  Create the server: getaddrinfo + socket + setsockopt + bind + listen
    serverResult = LoadSocketResult();
    if (!SocketCreate(&serverConfig, serverResult)) 
    {
        TraceLog(LOG_WARNING, "Failed to open server: status %d, errno %d", serverResult->status, serverResult->socket->status);
    } 
    else
    {
        if (!SocketBind(&serverConfig, serverResult))
        {
            TraceLog(LOG_WARNING, "Failed to bind server: status %d, errno %d", serverResult->status, serverResult->socket->status);
        } 
        else
        {
            if (!(serverConfig.type == SOCKET_UDP))
            {
                if (!SocketListen(&serverConfig, serverResult)) TraceLog(LOG_WARNING, "Failed to start listen server: status %d, errno %d", serverResult->status, serverResult->socket->status);
            }
        }
    }

    // Create and add sockets to the socket set
    socketSet = LoadSocketSet(2);
    AddSocket(socketSet, serverResult->socket);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (connected)
        {
            // Once connected to the network, check the sockets for pending information
            // and when information is ready, send either a Ping or a Pong.

            // CheckSockets, if any of the sockets in the socketSet are pending (received data, or requests)
            // then mark the socket as being ready. You can check this with IsSocketReady(client_res->socket)
            int active = CheckSockets(socketSet, 0);
            if (active != 0) TraceLog(LOG_DEBUG, "There are currently %d socket(s) with data to be processed.", active);

            // IsSocketReady, if the socket is ready, attempt to receive data from the socket
            int bytesRecv = 0;
            if (IsSocketReady(connection)) bytesRecv = SocketReceive(connection, receiveBuffer, strlen(pingmsg) + 1);

            // If we received data, was that data a "Ping!" or a "Pong!"
            if (bytesRecv > 0)
            {
                if (strcmp(receiveBuffer, pingmsg) == 0) { pong = true; }
                if (strcmp(receiveBuffer, pongmsg) == 0) { ping = true; }
            }

            // After each delay has expired, send a response "Ping!" for a "Pong!" and vice versa
            elapsed += GetFrameTime();
            if (elapsed > delay)
            {
                if (ping) 
                {
                    ping = false;
                    SocketSend(connection, pingmsg, strlen(pingmsg) + 1);
                } 
                else if (pong)
                {
                    pong = false;
                    SocketSend(connection, pongmsg, strlen(pingmsg) + 1);
                }
                
                elapsed = 0.0f;
            }
        }
        else
        {
            // Attempt to connect to the network (Either TCP, or UDP)
            int active = CheckSockets(socketSet, 0);
            if (active != 0) TraceLog(LOG_DEBUG, "There are currently %d socket(s) with data to be processed.", active);

            if (active > 0) 
            {
                if ((connection = SocketAccept(serverResult->socket, &connectionConfig)) != NULL) 
                {
                    AddSocket(socketSet, connection);
                    connected = true;
                    ping = true;
                }
            }
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