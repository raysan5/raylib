/**********************************************************************************************
*
*   rnet - A simple and easy-to-use network module for raylib
*
*   FEATURES:
*       - Manage network stuff
*
*   DEPENDENCIES:
*       core.h    - core stuff
*
*   CONTRIBUTORS:
*       Jak Barnes (github: @syphonx) (Feb. 2019):
*           - Initial version
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

//----------------------------------------------------------------------------------
// Check if config flags have been externally provided on compilation line
//----------------------------------------------------------------------------------
#if !defined(EXTERNAL_CONFIG_FLAGS)
#	include "config.h" // Defines module configuration flags
#endif

#include "raylib.h"
#include "sysnet.h"

#include <errno.h>
#include <stdlib.h>

bool InitNetwork()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == NO_ERROR) {
		TraceLog(LOG_INFO, "WinSock initialised.");
		return true;
	} else {
		TraceLog(LOG_WARNING, "WinSock failed to initialise.");
		return false;
	}
#else
	return true;
#endif
}

void CloseNetwork()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

void CreateTCPListenServer(TCPSocket* tcpsock, const char* address, const char* port)
{
	// Variables
	int              status;  // Status value to return (0) is success
	struct addrinfo  hints;   // Address flags (IPV4, IPV6, UDP?)
	struct addrinfo* results; // A pointer to the resulting address list

	// Set the hints
	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_INET;     // Either IPv4 or IPv6 (AF_INET, AF_INET6)
	hints.ai_socktype = SOCK_STREAM; // TCP (SOCK_STREAM), UDP (SOCK_DGRAM)

	// Populate address information
	status = getaddrinfo(address, // e.g. "www.example.com" or IP
						 port,    // e.g. "http" or port number
						 &hints,  // e.g. SOCK_STREAM/SOCK_DGRAM
						 &results // The struct to populate
	);

	// Did we succeed?
	if (status == -1) {
		TraceLog(LOG_WARNING, "Failed to get resolve host %s:%s: %s", address, port, strerror(status));
	} else {
		TraceLog(LOG_INFO, "Successfully resolved host %s:%s", address, port);
	}

	// Create our server socket
	int sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

	// Bind it to the port we passed in to getaddrinfo():
	status = bind(sockfd, results->ai_addr, results->ai_addrlen);

	// Did we succeed?
	if (status == -1) {
		TraceLog(LOG_WARNING, "Failed to get bind socket to port (%s): %s", port, strerror(errno));
	} else {
		TraceLog(LOG_INFO, "Successfully bound %s to port (%s)", address, port);
	}

	// Listen on the bound port
	status = listen(sockfd, 5);

	// Did we succeed?
	if (status == -1) {
		TraceLog(LOG_WARNING, "Failed to listen to socket: %s", strerror(errno));
	} else {
		TraceLog(LOG_INFO, "Successfully started listen server.");
	}

	DWORD nonBlocking = 1;
	if (ioctlsocket(sockfd, FIONBIO, &nonBlocking) == -1) {
		TraceLog(LOG_WARNING, "Failed to set socket to non-blocking.");
	} else {
		TraceLog(LOG_INFO, "Successfully set socket to non-blocking.");
	}

	// Free the linked-list, we're not using it anymore
	freeaddrinfo(results);

	// Finally, return our socket descriptor
	tcpsock->sockfd = sockfd;
	tcpsock->server = 1;
	tcpsock->ready  = true;

	return;
}

void CreateTCPClient(TCPSocket* tcpsock, char* address, char* port)
{
	int              status;
	int              sockfd;
	struct addrinfo  hints;
	struct addrinfo* results; // Will point to the results

	memset(&hints, 0, sizeof hints); // Make sure the struct is empty
	hints.ai_family   = AF_UNSPEC;   // Don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// Get ready to connect
	status = getaddrinfo(address, port, &hints, &results);

	// Did we succeed?
	if (status != 0) {
		TraceLog(LOG_WARNING, "Failed to get address information: %s", strerror(errno));
	} else {
		TraceLog(LOG_INFO, "Successfully created TCP client on port (%s)", port);
	}

	// Create our socket
	sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

	// Did it succeed?
	if (sockfd == -1) {
		TraceLog(LOG_WARNING, "Failed to create socket: %s", strerror(errno));
	} else {
		TraceLog(LOG_INFO, "Successfully created socket");
	}

	// Connect to the server
	status = connect(sockfd, results->ai_addr, results->ai_addrlen);

	if (status == -1) {
		TraceLog(LOG_WARNING, "Failed to connect to server %s:%s", address, port);
	} else {
		TraceLog(LOG_INFO, "Successfully connected to %s:%s", address, port);
	}

	freeaddrinfo(results);

	// Finally, return our socket descriptor
	tcpsock->sockfd = sockfd;
	tcpsock->server = 0;
	tcpsock->ready  = true;
}

void AcceptIncomingConnections(TCPSocket* tcpsock, int sockfd)
{
	struct sockaddr_storage their_addr;
	socklen_t               addr_size;
	int                     new_fd;
	addr_size = sizeof their_addr;
	new_fd    = accept(sockfd, (struct sockaddr*) &their_addr, &addr_size);

	if (new_fd == -1) {
		TraceLog(LOG_DEBUG, "Failed to accept incoming connection: %s", strerror(errno));
	} else {
		tcpsock->sockfd = new_fd;
		tcpsock->server = false;
		tcpsock->ready  = true;
		TraceLog(LOG_INFO, "Successfully accepted a new connection.");
	}
}

int SendTCP(int sockfd, const char* data, int len)
{
	int sentBytes = send(sockfd, data, len, 0);
	if (sentBytes == -1) {
		TraceLog(LOG_WARNING, "Failed to send data: %s", strerror(errno));
	} else {
		TraceLog(LOG_DEBUG, "Successfully sent %d bytes.", sentBytes);
	}
}

int ReceiveTCP(int sockfd, const char* data, int len)
{
	int receiveBytes = recv(sockfd, data, len, 0);
	if (receiveBytes == -1) {
		TraceLog(LOG_DEBUG, "Failed to receive data: %s", strerror(errno));
	} else if (receiveBytes == 0) {
		TraceLog(LOG_INFO, "Connection closed.");
	} else {
		TraceLog(LOG_DEBUG, "Successfully received %d bytes.", receiveBytes);
	}
	return receiveBytes;
}

void ResetSocket(TCPSocket* socket)
{
	socket->ready  = false;
	socket->server = 0;
	socket->sockfd = -1;
};

void ResolveHost(const char* hostname)
{
	struct addrinfo hints, *res, *p;
	int             status;
	char            ipstr[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_INET; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
		TraceLog(LOG_WARNING, "getaddrinfo: %s", strerror(status));
		return 2;
	}

	TraceLog(LOG_INFO, "IP addresses for %s:", hostname);

	for (p = res; p != NULL; p = p->ai_next) {
		void* addr;
		char* ipver;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in* ipv4 = (struct sockaddr_in*) p->ai_addr;
			addr                     = &(ipv4->sin_addr);
			ipver                    = "IPv4";
		} else { // IPv6
			struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) p->ai_addr;
			addr                      = &(ipv6->sin6_addr);
			ipver                     = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		TraceLog(LOG_INFO, "%s: %s", ipver, ipstr);
	}

	freeaddrinfo(res); // free the linked list
}
