/*******************************************************************************************
 *
 *   raylib [network] example - Network Test
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

void test_network_initialise()
{
    assert(InitNetwork() == true);
}

void test_socket_result()
{
    SocketResult *result = AllocSocketResult();
    assert(result != NULL);
    FreeSocketResult(&result);
    assert(result == NULL);
}

void test_socket()
{
    Socket *socket = AllocSocket();
    assert(socket != NULL);
    FreeSocket(&socket);
    assert(socket == NULL);
}

void test_resolve_ip()
{
    const char *host = "8.8.8.8";
    const char *port = "8080";
    char        ip[ADDRESS_IPV6_ADDRSTRLEN];
    char        service[ADDRESS_MAXSERV];

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
    const char *        address = "localhost";
    const char *        port    = "80";
    AddressInformation *addr    = AllocAddressList(3);
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
    SocketConfig  server_cfg = {.host = "127.0.0.1", .port = "8080", .server = true, .nonblocking = true};
    Socket *      socket     = AllocSocket();
    SocketResult *server_res = AllocSocketResult();
    SocketSet *   socket_set = AllocSocketSet(1);
    assert(SocketCreate(&server_cfg, server_res));
    assert(AddSocket(socket_set, server_res->socket));
    assert(SocketListen(&server_cfg, server_res));
}

int main()
{
    int screenWidth  = 800;
    int screenHeight = 450;
    InitWindow(
        screenWidth, screenHeight, "raylib [network] example - network test");
    SetTargetFPS(60);

    // Run the tests
    test_network_initialise();
    test_resolve_host();
    // test_socket_create();

    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}