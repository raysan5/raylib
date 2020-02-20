/*******************************************************************************************
*
*   raylib [network] example - Network Test
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

void test_network_initialise()
{
    assert(InitNetworkDevice() == true);
}

void test_socket_result()
{
    SocketResult *result = LoadSocketResult();
    assert(result != NULL);
    UnloadSocketResult(&result);
    assert(result == NULL);
}

void test_socket()
{
    Socket *socket = LoadSocket();
    assert(socket != NULL);
    UnloadSocket(&socket);
    assert(socket == NULL);
}

void test_resolve_ip()
{
    const char *host = "8.8.8.8";
    const char *port = "8080";
    char ip[ADDRESS_IPV6_ADDRSTRLEN];
    char service[ADDRESS_MAXSERV];

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_NUMERICHOST, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "8.8.8.8") == 0);

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_DEFAULT, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "google-public-dns-a.google.com") == 0);

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_NOFQDN, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "google-public-dns-a") == 0);

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_NUMERICHOST, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "8.8.8.8") == 0);

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_NAMEREQD, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "google-public-dns-a.google.com") == 0);

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_NUMERICSERV, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "google-public-dns-a.google.com") == 0);

    memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
    ResolveIP(host, port, NAME_INFO_DGRAM, ip, service);
    TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
    assert(strcmp(ip, "google-public-dns-a.google.com") == 0);
}

void test_resolve_host()
{
    const char *address = "localhost";
    const char *port = "80";
    AddressInformation *addr = LoadAddressList(3);
    int count = ResolveHost(address, port, ADDRESS_TYPE_ANY, 0, addr); 

    assert(GetAddressFamily(addr[0]) == ADDRESS_TYPE_IPV6);
    assert(GetAddressFamily(addr[1]) == ADDRESS_TYPE_IPV4);
    assert(GetAddressSocketType(addr[0]) == 0);
    assert(GetAddressProtocol(addr[0]) == 0);
    // for (size_t i = 0; i < count; i++) { PrintAddressInfo(addr[i]); }
}

void test_address()
{
}

void test_address_list()
{
}

void test_socket_create()
{
    SocketConfig server_cfg = { .host = "127.0.0.1", .port = "8080", .server = true, .nonblocking = true };
    Socket *socket = LoadSocket();
    SocketResult *server_res = LoadSocketResult();
    SocketSet *socket_set = LoadSocketSet(1);

    assert(SocketCreate(&server_cfg, server_res));
    assert(AddSocket(socket_set, server_res->socket));
    assert(SocketListen(&server_cfg, server_res));
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [network] example - network test");
    
    InitNetworkDevice();    // Init network communications
    
    // Run some tests
    test_resolve_host();
    //test_socket_create();
    //test_resolve_ip();

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