#include "raylib.h"

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

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_NUMERICHOST, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "8.8.8.8") == 0);

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_DEFAULT, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "google-public-dns-a.google.com") == 0);

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_NOFQDN, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "google-public-dns-a") == 0);

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_NUMERICHOST, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "8.8.8.8") == 0);

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_NAMEREQD, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "google-public-dns-a.google.com") == 0);

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_NUMERICSERV, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "google-public-dns-a.google.com") == 0);

	memset(ip, '\0', ADDRESS_IPV6_ADDRSTRLEN);
	ResolveIP(host, port, NAME_INFO_DGRAM, ip);
	TraceLog(LOG_INFO, "Resolved %s to %s", host, ip);
	assert(strcmp(ip, "google-public-dns-a.google.com") == 0);
}

void test_resolve_host()
{
	const char *                 address = "localhost";
	const char *                 port    = "80";
	struct _AddressInformation **addr    = AllocAddressList(3);
	int                          count   = ResolveHost(address, port, addr); 
	assert(GetAddressFamily(addr[0]) == ADDRESS_TYPE_IPV6);
	assert(GetAddressFamily(addr[1]) == ADDRESS_TYPE_IPV4);
	for (size_t i = 0; i < count; i++) { PrintAddressInfo(addr[i]); }
}

void test_address()
{

}

void test_address_list()
{

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
	// test_socket_result();
	// test_socket();
	// test_resolve_ip();
	test_resolve_host();

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