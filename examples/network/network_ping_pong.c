/*******************************************************************************************
*
*   raylib [network] example - Client/Server ping-pong
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

float elapsed = 0.0f;
float delay = 1.0f;
bool ping = false;
bool pong = false;
bool connected = false;
bool clientConnected = false;
const char *pingmsg = "Ping!";
const char *pongmsg = "Pong!";
int msglen = 0;
SocketConfig serverConfig = { .host = "127.0.0.1", .port = "4950", .type = SOCKET_TCP, .server = true, .nonblocking = true };
SocketConfig clientConfig = { .host = "127.0.0.1", .port = "4950", .type = SOCKET_TCP, .nonblocking = true };
SocketConfig connectionConfig = { .nonblocking = true };
SocketResult *serverResult = NULL;
SocketResult *clientResult = NULL;
SocketSet *socketSet = NULL;
Socket *connection = NULL;
char receiveBuffer[512] = { 0 };

// Attempt to connect to the network (Either TCP, or UDP)
static void NetworkConnect(void)
{
    // If the server is configured as UDP, ignore connection requests
    if ((serverConfig.type == SOCKET_UDP) && (clientConfig.type == SOCKET_UDP))
    {
        ping = true;
        connected = true;
    }
    else
    {
        // If the client is connected, run the server code to check for a connection
        if (clientConnected)
        {
            int active = CheckSockets(socketSet, 0);
            if (active != 0) TraceLog(LOG_INFO, "There are currently %d socket(s) with data to be processed.", active);

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
        else
        {
            // Check if we're connected every _delay_ seconds
            elapsed += GetFrameTime();
            if (elapsed > delay)
            {
                if (IsSocketConnected(clientResult->socket)) clientConnected = true;

                elapsed = 0.0f;
            }
        }
    }
}

// Once connected to the network, check the sockets for pending information
// and when information is ready, send either a Ping or a Pong.
static void UpdateNetwork(void)
{
    // CheckSockets, if any of the sockets in the socketSet are pending (received data, or requests)
    // then mark the socket as being ready. You can check this with IsSocketReady(clientResult->socket)
    int active = CheckSockets(socketSet, 0);
    if (active != 0) TraceLog(LOG_DEBUG, "There are currently %d socket(s) with data to be processed.", active);

    // IsSocketReady, if the socket is ready, attempt to receive data from the socket
    int bytesRecv = 0;
    if ((serverConfig.type == SOCKET_UDP) && (clientConfig.type == SOCKET_UDP))
    {
        if (IsSocketReady(clientResult->socket)) bytesRecv = SocketReceive(clientResult->socket, receiveBuffer, msglen);
        if (IsSocketReady(serverResult->socket)) bytesRecv = SocketReceive(serverResult->socket, receiveBuffer, msglen);
    }
    else if (IsSocketReady(connection)) bytesRecv = SocketReceive(connection, receiveBuffer, msglen);

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
            if (serverConfig.type == SOCKET_UDP && clientConfig.type == SOCKET_UDP) SocketSend(clientResult->socket, pingmsg, msglen);
            else SocketSend(clientResult->socket, pingmsg, msglen);
        }
        else if (pong)
        {
            pong = false;
            if (serverConfig.type == SOCKET_UDP && clientConfig.type == SOCKET_UDP) SocketSend(clientResult->socket, pongmsg, msglen);
            else SocketSend(clientResult->socket, pongmsg, msglen);
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

    InitWindow(screenWidth, screenHeight, "raylib [network] example - ping pong");

    InitNetworkDevice();    // Init network communications

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
                if (!SocketListen(&serverConfig, serverResult))
                {
                    TraceLog(LOG_WARNING, "Failed to start listen server: status %d, errno %d", serverResult->status, serverResult->socket->status);
                }
            }
        }
    }

    // Create the client: getaddrinfo + socket + setsockopt + connect (TCP only)
    clientResult = LoadSocketResult();
    if (!SocketCreate(&clientConfig, clientResult))
    {
        TraceLog(LOG_WARNING, "Failed to open client: status %d, errno %d", clientResult->status, clientResult->socket->status);
    }
    else
    {
        if (!(clientConfig.type == SOCKET_UDP))
        {
            if (!SocketConnect(&clientConfig, clientResult))
            {
                TraceLog(LOG_WARNING, "Failed to connect to server: status %d, errno %d", clientResult->status, clientResult->socket->status);
            }
        }
    }

    // Create and add sockets to the socket set
    socketSet = LoadSocketSet(3);

    AddSocket(socketSet, serverResult->socket);
    AddSocket(socketSet, clientResult->socket);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (connected) UpdateNetwork();
        //else NetworkConnect();
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