/*******************************************************************************************
 *
 *   raylib [network] example - Resolve Host
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

    InitWindow(screenWidth, screenHeight, "raylib [network] example - ping pong");
    
    char buffer[ADDRESS_IPV6_ADDRSTRLEN];
    uint16_t port = 0;
    
    SetTraceLogLevel(LOG_DEBUG);


    // Networking
    InitNetworkDevice();
     
    AddressInformation* addr = AllocAddressList(1);
    int count = ResolveHost(
        NULL,
        "5210",
        ADDRESS_TYPE_IPV4,
        0                               // Uncomment any of these flags
        //  ADDRESS_INFO_NUMERICHOST    // or try them in conjunction to
        //  ADDRESS_INFO_NUMERICSERV    // specify custom behaviour from 
        //  ADDRESS_INFO_DNS_ONLY       // the function getaddrinfo()
        //  ADDRESS_INFO_ALL            //
        //  ADDRESS_INFO_FQDN           // e.g. ADDRESS_INFO_CANONNAME | ADDRESS_INFO_NUMERICSERV
        ,
        addr
    );

    if (count > 0)
    {
        GetAddressHostAndPort(addr[0], buffer, &port);
        TraceLog(LOG_INFO, "Resolved to ip %s::%d", buffer, port);
    }

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}